#include "parse.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "enum.h"
#include "lib.h"
#include "macro.h"
#include "struct.h"
#include "typedef.h"

// 現在着目しているトークン
Token *token;

Node *code[100];

// ローカル変数
VarTable *locals;
unsigned locals_offset;

// グローバル変数
VarTable *globals;

// 文字列定数
StringConstant *string_constants;
int string_constants_index = 0;

Node *expr();
bool macro_define();
bool macro_undef();
bool macro_ifndef();

bool in_preprocessor_directive = false;

// データ型のサイズ
int get_type_size(int type) {
  switch (type) {
    case kTypeShort:
    case kTypeUShort:
      return 2;
    case kTypeInt:
    case kTypeUInt:
      return 4;
    case kTypeLLong:
    case kTypeULLong:
    case kTypePtr:
      return 8;
    case kTypeChar:
      return 1;
    default:
      return -1;
  }
}

int get_type_size_by_type(Type *type) {
  switch (type->ty) {
    case kTypeShort:
    case kTypeUShort:
    case kTypeInt:
    case kTypeUInt:
    case kTypeLLong:
    case kTypeULLong:
    case kTypePtr:
    case kTypeChar:
      return get_type_size(type->ty);
    case kTypeStruct:
      return get_struct_size(type);
    default:
      return -1;
  }
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_var(Token *tok, VarTable *var_table) {
  for (LVar *var = var_table->vars; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

LVar *find_var_recursive(Token *tok, VarTable *var_table) {
  if (!var_table) {
    return NULL;
  }
  LVar *var = find_var(tok, var_table);
  if (var) {
    return var;
  }
  if (var_table->parent) {
    return find_var_recursive(tok, var_table->parent);
  } else {
    return NULL;
  }
}

// 文字列定数を検索する。見つからなかった場合はNULLを返す。
StringConstant *find_string_constant(Token *tok) {
  for (StringConstant *var = string_constants; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->str, var->len))
      return var;
  return NULL;
}

StringConstant *add_string_constant(Token *tok) {
  StringConstant *string_constant = calloc(1, sizeof(StringConstant));
  string_constant->next = string_constants;
  string_constant->str = tok->str;
  string_constant->len = tok->len;
  string_constant->index = string_constants_index;
  string_constants = string_constant;
  string_constants_index++;
  return string_constant;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != kTokenReserved || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_token(TokenKind kind) {
  if (token->kind != kind) return false;
  Token *current = token;
  token = token->next;
  return current;
}

Token *consume_ident(char *str) {
  if (token->kind != kTokenIdent) return false;
  if (memcmp(token->str, str, strlen(str))) return false;
  Token *current = token;
  token = token->next;
  return current;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != kTokenReserved || token->str[0] != op)
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
unsigned long long expect_number() {
  if (token->kind != kTokenNum) error_at(token->str, "数ではありません");
  unsigned long long val = token->val;
  token = token->next;
  return val;
}

Token *expect_ident(void) {
  Token *tok = consume_token(kTokenIdent);
  if (tok == NULL) {
    error_at(token->str, "識別子ではありません");
  }
  return tok;
}

bool at_eof() { return token->kind == kTokenEof; }

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  if (cur) {
    cur->next = tok;
  }
  return tok;
}

// トークン列をコピー
Token *copy_tokens(Token *src_begin, Token *src_end, Token *next) {
  Token *tok = calloc(1, sizeof(Token));
  memcpy(tok, src_begin, sizeof(Token));
  Token *head = tok;

  Token *cur = src_begin->next;
  while (cur != src_end->next) {
    tok->next = calloc(1, sizeof(Token));
    memcpy(tok->next, cur, sizeof(Token));
    tok = tok->next;

    cur = cur->next;
  }
  tok->next = next;

  return head;
}

int expect_keyword(char *p, char *keyword) {
  int len = strlen(keyword);
  if (strncmp(p, keyword, len) == 0 && !isalnum(p[len]) && p[len] != '_') {
    return len;
  }
  return 0;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int keyword_len;
  int beginning_of_line = 1;

  while (*p) {
    // プリプロセッサディレクティブ終端
    if (in_preprocessor_directive && *p == '\n') {
      cur = new_token(kTokenNewline, cur, p, 0);
      in_preprocessor_directive = false;
      p++;
      beginning_of_line = 1;
      continue;
    }

    // 空白文字をスキップ
    if (*p == '\n') {
      p++;
      beginning_of_line = 1;
      continue;
    } else if (isspace(*p)) {
      p++;
      continue;
    }

    // backslash
    if (*p == '\\') {
      p += 2;
      continue;
    }

    // マクロ
    if (keyword_len = expect_keyword(p, "#define")) {
      cur = new_token(kTokenMacroDefine, cur, p, 0);
      p += keyword_len;
      in_preprocessor_directive = true;
      continue;
    }
    if (keyword_len = expect_keyword(p, "#undef")) {
      cur = new_token(kTokenMacroUndef, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "#ifndef")) {
      cur = new_token(kTokenMacroIfndef, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "#endif")) {
      cur = new_token(kTokenMacroEndif, cur, p, 0);
      p += keyword_len;
      continue;
    }

    // #include
    if (keyword_len = expect_keyword(p, "#include")) {
      char *dest = p;
      p += keyword_len;
      size_t len = strspn(p, " \t");
      p += len;
      if (*p != '\"') {
        if (*p == '<') {
          len = strcspn(p, "\n");
          p += len;
          continue;
        }
        error_at(p, "'\"'ではありません");
      }
      p++;
      len = strcspn(p, "\"");
      char *buffer = read_file(strndup(p, len), 1024 * 1024);
      p += len + 2;  // `"` and `\n`
      memmove(dest + strlen(buffer), p, strlen(p) + 1);
      memmove(dest, buffer, strlen(buffer));
      p = dest;
      continue;
    }

    // 行コメントをスキップ
    if (strncmp(p, "//", 2) == 0) {
      p += 2;
      while (*p != '\n') p++;
      continue;
    }
    if (beginning_of_line && strncmp(p, "#", 1) == 0) {
      p += 1;
      while (*p != '\n') p++;
      continue;
    }

    beginning_of_line = 0;

    // ブロックコメントをスキップ
    if (strncmp(p, "/*", 2) == 0) {
      char *q = strstr(p + 2, "*/");
      if (!q) error_at(p, "コメントが閉じられていません");
      p = q + 2;
      continue;
    }

    if (!memcmp(p, "==", 2) || !memcmp(p, "!=", 2) || !memcmp(p, "<=", 2) ||
        !memcmp(p, ">=", 2) || !memcmp(p, "||", 2) || !memcmp(p, "&&", 2) ||
        !memcmp(p, "+=", 2) || !memcmp(p, "-=", 2) || !memcmp(p, "*=", 2) ||
        !memcmp(p, "/=", 2)) {
      cur = new_token(kTokenReserved, cur, p, 2);
      p += 2;
      continue;
    }

    if (!memcmp(p, "->", 2)) {
      cur = new_token(kTokenReserved, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' ||
        *p == '{' || *p == '}' || *p == ',' || *p == '&' || *p == '[' ||
        *p == ']' || *p == '.' || *p == ':' || *p == '!' || *p == '?' ||
        *p == '#') {
      cur = new_token(kTokenReserved, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(kTokenNum, cur, p, 0);
      cur->val = mystrtoull(p, &p);
      cur->len = p - cur->str;
      continue;
    }

    if (keyword_len = expect_keyword(p, "return")) {
      cur = new_token(kTokenReturn, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (keyword_len = expect_keyword(p, "break")) {
      cur = new_token(kTokenBreak, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (keyword_len = expect_keyword(p, "if")) {
      cur = new_token(kTokenIf, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "else")) {
      cur = new_token(kTokenElse, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (keyword_len = expect_keyword(p, "while")) {
      cur = new_token(kTokenWhile, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "do")) {
      cur = new_token(kTokenReserved, cur, p, keyword_len);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "for")) {
      cur = new_token(kTokenFor, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "sizeof")) {
      cur = new_token(kTokenSizeof, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "struct")) {
      cur = new_token(kTokenStruct, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "enum")) {
      cur = new_token(kTokenEnum, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "typedef")) {
      cur = new_token(kTokenTypedef, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "switch")) {
      cur = new_token(kTokenSwitch, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "case")) {
      cur = new_token(kTokenSwitchCase, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "default")) {
      cur = new_token(kTokenSwitchDefault, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "extern")) {
      cur = new_token(kTokenExtern, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (('A' <= *p && *p <= 'Z') || *p == '_' || ('a' <= *p && *p <= 'z')) {
      size_t len = strspn(
          p + 1,
          "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
      len++;
      cur = new_token(kTokenIdent, cur, p, len);
      p += len;
      continue;
    }

    if (*p == '"') {
      // 文字列
      p++;
      char *pos = p;
      for (;;) {
        if (*pos == '\"') {
          break;
        } else if (*pos == '\\') {
          pos += 2;
        } else if (*pos == '\0') {
          error_at(pos, "トークナイズできません");
        } else {
          pos++;
        }
      }
      cur = new_token(kTokenString, cur, p, pos - p);
      p = pos + 1;
      continue;
    }

    if (*p == '\'') {
      // 文字
      p++;
      char c = *p;
      if (c == '\\') {
        // escape sequences
        p++;
        switch (*p) {
          case 'a':
            c = '\a';
            break;
          case 'b':
            c = '\b';
            break;
          case 'f':
            c = '\f';
            break;
          case 'n':
            c = '\n';
            break;
          case 'r':
            c = '\r';
            break;
          case 't':
            c = '\t';
            break;
          case 'v':
            c = '\v';
            break;
          case '\\':
            c = '\\';
            break;
          case '\'':
            c = '\'';
            break;
          case '\"':
            c = '\"';
            break;
          case '?':
            c = '\?';
            break;
          case '0':
            c = '\0';
            break;
          default:
            error_at(p, "トークナイズできません");
        }
      }
      p++;
      if (*p != '\'') error_at(p, "トークナイズできません");
      cur = new_token(kTokenNum, cur, p, 0);
      cur->val = c;
      p++;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(kTokenEof, cur, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(unsigned long long val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kNodeNum;
  node->val = val;
  return node;
}

// 識別子ノード
Node *new_node_ident(NodeKind kind, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->name = token->str;
  node->len = token->len;
  return node;
}

// 型を検出
int detect_type(Node *node) {
  switch (node->kind) {
    case kNodeNum:
    case kNodeMul:
    case kNodeDiv:
    case kNodeEq:
    case kNodeNe:
    case kNodeLt:
    case kNodeLe:
      return kTypeInt;
    case kNodeDeref:
      return node->lhs->type->ptr_to->ty;
    case kNodeAddr:
      return kTypePtr;
    case kNodeLocalVar:
    case kNodeGlobalVar:
      return node->type->ty;
    case kNodeAdd:
    case kNodeSub: {
      int ty = detect_type(node->lhs);
      if (ty == kTypePtr) return ty;
      ty = detect_type(node->rhs);
      if (ty == kTypePtr) return ty;

      return kTypeInt;
    }
  }

  return -1;
}

Node *initializer_list(int *count) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kNodeInitializerList;
  Node *current = node;
  if (token->kind == kTokenNum) {
    for (;;) {
      (*count)++;
      current->next = new_node_num(expect_number());
      current = current->next;
      if (!consume(",")) {
        break;
      }
    }
  } else if (token->kind == kTokenString) {
    Token *tok = consume_token(kTokenString);
    int i;
    for (i = 0; i < tok->len; i++) {
      (*count)++;
      current->next = new_node_num(tok->str[i]);
      current = current->next;
    }
  }
  return node;
}

// マクロを展開する
Token *expand_macro(Token *tok, MacroDef *macro_def) {
  if (macro_def->is_function_like) {
    // 関数形式マクロ
    if (!consume("(")) {
      error_at(tok->str, "マクロ形式が違います");
    }
    // パラメータをマクロとして登録
    int macro_rollback_num = 0;
    MacroParam *param = macro_def->param;
    if (param == NULL) {
      expect(')');
      return tok->next->next;
    }
    for (; param != NULL;) {
      int parenthesis_level = 0;
      macro_rollback_num++;
      MacroDef *macro_def = new_macro_def(param->name, param->name_len, token);
      Token *tok;
      for (;;) {
        tok = token;
        if (consume("(")) {
          parenthesis_level++;
        } else if (consume(")")) {
          if (parenthesis_level == 0) {
            break;
          }
          parenthesis_level--;
        } else if (consume(",")) {
          if (parenthesis_level == 0) {
            break;
          }
        } else {
          tok = token;
          token = token->next;
        }
        macro_def->end_tok = tok;
      }
      param = param->next;
    }

    // トークン列をコピーし、パラメータを置き換える
    Token *head = copy_tokens(macro_def->tok, macro_def->end_tok, token);
    Token *cur = head, *prev = NULL;
    for (; cur != token; cur = cur->next) {
      if (cur->kind == kTokenIdent) {
        MacroDef *macro_def =
            find_macro_def(cur->str, cur->len, macro_rollback_num);
        if (macro_def) {
          Token *copy =
              copy_tokens(macro_def->tok, macro_def->end_tok, cur->next);
          if (prev) {
            prev->next = copy;
          } else {
            head = copy;
          }
        }
      } else if (cur->kind == kTokenReserved && !strncmp(cur->str, "#", 1)) {
        // # operator
        cur = cur->next;
        MacroDef *macro_def =
            find_macro_def(cur->str, cur->len, macro_rollback_num);
        if (macro_def) {
          char *s = strescape(macro_def->tok->str, macro_def->end_tok->str +
                                                       macro_def->end_tok->len -
                                                       macro_def->tok->str);
          Token *tok = new_token(kTokenString, NULL, s, strlen(s));
          tok->next = cur->next;
          if (prev) {
            prev->next = tok;
          } else {
            head = tok;
          }
        } else {
          error_at(cur->str, "未定義のパラメータです");
        }
      }
      prev = cur;
    }

    rollback_macro_def(macro_rollback_num);

    if (macro_def->tok->kind == kTokenNewline) {
      return token;
    }

    return head;
  } else {
    if (macro_def->tok->kind == kTokenNewline) {
      return tok;
    }
    macro_def->end_tok->next = token;
    return macro_def->tok;
  }
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(')');
    return node;
  }

  Token *tok = consume_token(kTokenIdent);
  if (tok) {
    // マクロ
    MacroDef *macro_def = find_macro_def(tok->str, tok->len, 0);
    if (macro_def) {
      token = expand_macro(token, macro_def);
      return primary();
    }

    // 関数呼び出し
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = kNodeFuncCall;
      node->name = tok->str;
      node->len = tok->len;
      if (consume(")")) return node;
      node->lhs = expr();
      Node *current = node->lhs;
      for (;;) {
        if (!consume(",")) {
          expect(')');
          return node;
        }
        current->next = expr();
        current = current->next;
      }
    }

    // __FILE__
    if (!strncmp(tok->str, "__FILE__", 8)) {
      Token tok;
      tok.str = filename;
      tok.len = strlen(filename);

      // 文字列
      StringConstant *string_constant = find_string_constant(&tok);
      if (string_constant == NULL) {
        string_constant = add_string_constant(&tok);
      }
      Node *node = calloc(1, sizeof(Node));
      node->kind = kNodeString;
      node->string_constant = string_constant;
      return node;
    }

    // __LINE__
    if (!strncmp(tok->str, "__LINE__", 8)) {
      return new_node_num(strcount(user_input, tok->str, '\n') + 1);
    }

    Node *node = calloc(1, sizeof(Node));
    LVar *lvar = find_var_recursive(tok, locals);
    EnumDef *enum_def = find_enum_def(tok->str, tok->len);
    if (lvar) {
      // ローカル変数
      node->kind = kNodeLocalVar;
      node->offset = lvar->offset;
      node->name = lvar->name;
      node->len = lvar->len;
      node->type = lvar->type;
    } else if (lvar = find_var(tok, globals)) {
      // グローバル変数
      node->kind = kNodeGlobalVar;
      node->name = lvar->name;
      node->len = lvar->len;
      node->type = lvar->type;
    } else if (enum_def) {
      // 列挙
      return new_node_num(enum_def->val);
    } else {
      error_at(tok->str, "未定義の変数です");
    }
    if (consume("[")) {
      // 配列添字
      node = new_node(kNodeDeref,
                      new_node(kNodeAdd, node, new_node_num(expect_number())),
                      NULL);
      expect(']');
    }

    // 構造体メンバ
    for (;;) {
      int node_kind = -1;
      if (consume(".")) {
        node_kind = kNodeStructMember;
      } else if (consume("->")) {
        node_kind = kNodeStructPointerMember;
      } else {
        break;
      }
      Type *type =
          node_kind == kNodeStructMember ? node->type : node->type->ptr_to;
      if (type == NULL || type->ty != kTypeStruct) {
        error_at(tok->str, "型が不明です");
      }
      StructDef *struct_def = find_struct_def(type->name, type->name_len);
      if (struct_def == NULL) {
        error_at(tok->str, "構造体ではありません");
      }
      Token *member_token = consume_token(kTokenIdent);
      if (member_token == NULL) {
        error_at(tok->str, "識別子ではありません");
      }
      StructMember *member =
          find_struct_member(struct_def, member_token->str, member_token->len);
      if (member == NULL) {
        error_at(member_token->str, "存在しないメンバーです");
      }
      node = new_node(node_kind, node, NULL);
      node->name = member->name;
      node->len = member->name_len;
      node->type = member->type;
      node->offset = member->offset;
    }

    return node;
  }

  tok = consume_token(kTokenString);
  if (tok) {
    // 文字列
    StringConstant *string_constant = find_string_constant(tok);
    if (string_constant == NULL) {
      string_constant = add_string_constant(tok);
    }
    Node *node = calloc(1, sizeof(Node));
    node->kind = kNodeString;
    node->string_constant = string_constant;
    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+")) return primary();
  if (consume("-")) return new_node(kNodeSub, new_node_num(0), primary());
  if (consume("&")) return new_node(kNodeAddr, primary(), NULL);
  if (consume("*")) return new_node(kNodeDeref, primary(), NULL);
  if (consume("!")) return new_node(kNodeLogicalNot, unary(), NULL);
  if (consume_token(kTokenSizeof)) {
    Token *tok = token;
    Node *node = unary();
    if ((node->kind == kNodeLocalVar || node->kind == kNodeGlobalVar) &&
        node->type->ty == kTypeArray) {
      return new_node_num(get_type_size(node->type->ptr_to->ty) *
                          node->type->array_size);
    } else if (tok->next->kind == kTokenSizeof) {
      // sizeof(sizeof(1))
      return new_node_num(8);
    }
    int ty = detect_type(node);
    if (ty != -1) return new_node_num(get_type_size(ty));
    error("データ型が不明です");
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(kNodeMul, node, unary());
    else if (consume("/"))
      node = new_node(kNodeDiv, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(kNodeAdd, node, mul());
    else if (consume("-"))
      node = new_node(kNodeSub, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<="))
      node = new_node(kNodeLe, node, add());
    else if (consume("<"))
      node = new_node(kNodeLt, node, add());
    else if (consume(">="))
      node = new_node(kNodeLe, add(), node);
    else if (consume(">"))
      node = new_node(kNodeLt, add(), node);
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(kNodeEq, node, relational());
    else if (consume("!="))
      node = new_node(kNodeNe, node, relational());
    else
      return node;
  }
}

Node *logical() {
  Node *node = equality();

  for (;;) {
    if (consume("||"))
      node = new_node(kNodeLogicalOr, node, logical());
    else if (consume("&&"))
      node = new_node(kNodeLogicalAnd, node, logical());
    else
      return node;
  }
}

Node *ternary_conditional() {
  Node *node = logical();
  if (consume("?")) {
    Node *t = ternary_conditional();
    if (!consume(":")) {
      error_at(token->str, "三項演算子が無効です");
    }
    t->next = ternary_conditional();
    node = new_node(kNodeTernaryConditional, node, t);
  }
  return node;
}

Node *assign() {
  Node *node = ternary_conditional();
  if (consume("=")) {
    node = new_node(kNodeAssign, node, assign());
  } else if (consume("+=")) {
    node = new_node(kNodeAssign, node, new_node(kNodeAdd, node, assign()));
  } else if (consume("-=")) {
    node = new_node(kNodeAssign, node, new_node(kNodeSub, node, assign()));
  } else if (consume("*=")) {
    node = new_node(kNodeAssign, node, new_node(kNodeMul, node, assign()));
  } else if (consume("/=")) {
    node = new_node(kNodeAssign, node, new_node(kNodeDiv, node, assign()));
  }
  return node;
}

Node *expr() { return assign(); }

// データ型
Type *parse_type() {
  Type *type = calloc(1, sizeof(Type));
  if (consume_ident("int")) {
    type->ty = kTypeInt;
  } else if (consume_ident("long")) {
    consume_ident("long");
    consume_ident("int");
    type->ty = kTypeLLong;
  } else if (consume_ident("short")) {
    type->ty = kTypeShort;
  } else if (consume_ident("unsigned")) {
    if (consume_ident("long")) {
      consume_ident("long");
      consume_ident("int");
      type->ty = kTypeULLong;
    } else if (consume_ident("short")) {
      type->ty = kTypeUShort;
    } else {
      consume_ident("int");
      type->ty = kTypeUInt;
    }
  } else if (consume_ident("char")) {
    type->ty = kTypeChar;
  } else if (consume_ident("void")) {
    type->ty = kTypeVoid;
  } else if (consume_token(kTokenStruct)) {
    Token *tok = consume_token(kTokenIdent);
    StructDef *struct_def = find_struct_def(tok->str, tok->len);
    if (struct_def == NULL) {
      error_at(tok->str, "構造体が定義されていません");
    }
    type->ty = kTypeStruct;
    type->name = tok->str;
    type->name_len = tok->len;
  } else {
    Token *tok = consume_token(kTokenIdent);
    if (tok) {
      TypedefDef *typedef_def = find_typedef_def(tok->str, tok->len);
      if (typedef_def == NULL) {
        token = tok;
        return NULL;
      }
      if (typedef_def->ty == kTypeStruct) {
        type->ty = kTypeStruct;
        type->name = typedef_def->type_name;
        type->name_len = typedef_def->type_name_len;
      } else {
        type->ty = typedef_def->ty;
      }
    } else {
      return NULL;
    }
  }
  while (consume("*")) {
    Type *t = calloc(1, sizeof(Type));
    t->ty = kTypePtr;
    t->ptr_to = type;
    type = t;
  }
  return type;
}

// 変数定義
Node *variable_definition(Type *type, VarTable *var_table) {
  Node *node;
  Token *tok = expect_ident();
  LVar *lvar = find_var(tok, var_table);
  if (lvar) {
    if (type->is_extern && lvar->type->is_extern) {
      return NULL;
    } else if (! lvar->type->is_extern) {
      error_at(tok->str, "定義済みの変数です");
    }
  }
  node = calloc(1, sizeof(Node));
  node->kind = kNodeVarDef;
  lvar = calloc(1, sizeof(LVar));
  lvar->next = var_table->vars;
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (var_table != globals)
    lvar->offset = locals_offset + get_type_size_by_type(type);
  lvar->type = type;
  if (consume("[")) {
    // 配列定義
    type = calloc(1, sizeof(Type));
    type->ty = kTypeArray;
    Token *tok_num = consume_token(kTokenNum);
    if (tok_num)
      type->array_size = tok_num->val;
    else
      type->array_size = 0;
    type->ptr_to = lvar->type;
    if (var_table != globals)
      lvar->offset =
          locals_offset + type->array_size * get_type_size_by_type(lvar->type);
    lvar->type = type;
    expect(']');
  }
  if (consume("=")) {
    // 初期化
    int is_init_str = type->ty == kTypeArray && type->ptr_to->ty == kTypeChar &&
                      token->kind == kTokenString;
    Node *node_lvar = calloc(1, sizeof(Node));
    if (consume("{") || is_init_str) {
      // 初期化子リスト
      int count = 0;
      node->lhs = new_node(kNodeAssign, node_lvar, initializer_list(&count));
      type->array_size = count;
      if (var_table != globals)
        lvar->offset = locals_offset +
                       type->array_size * get_type_size(lvar->type->ptr_to->ty);
      if (!is_init_str) expect('}');
    } else {
      node->lhs = new_node(kNodeAssign, node_lvar, expr());
    }
    node_lvar->kind = (var_table != globals) ? kNodeLocalVar : kNodeGlobalVar;
    node_lvar->offset = lvar->offset;
    node_lvar->name = lvar->name;
    node_lvar->len = lvar->len;
    node_lvar->type = lvar->type;
  }
  node->name = tok->str;
  node->len = tok->len;
  node->offset = lvar->offset;
  node->type = lvar->type;
  var_table->vars = lvar;
  locals_offset = lvar->offset;
  return node;
}

Node *stmt() {
  Node *node;
  Node *current;
  Type *type;

  Token *tok = consume_token(kTokenIdent);
  if (tok) {
    // マクロ
    MacroDef *macro_def = find_macro_def(tok->str, tok->len, 0);
    if (macro_def) {
      token = expand_macro(token, macro_def);
      return stmt();
    } else {
      token = tok;

      // variable
      LVar *lvar = find_var_recursive(tok, locals);
      if (lvar) {
        node = expr();
        expect(';');
        return node;
      }
    }
  }
  if (consume_token(kTokenReturn)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeReturn;
    node->lhs = expr();
  } else if (consume_token(kTokenBreak)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeBreak;
  } else if (consume_token(kTokenIf)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeIf;
    if (!consume("(")) error_at(token->str, "'('ではありません");
    node->lhs = expr();
    if (!consume(")")) error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    if (consume_token(kTokenElse)) {
      node->els = stmt();
    }
    return node;
  } else if (consume_token(kTokenWhile)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeWhile;
    if (!consume("(")) error_at(token->str, "'('ではありません");
    node->lhs = expr();
    if (!consume(")")) error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    return node;
  } else if (consume("do")) {  // do-while
    node = calloc(1, sizeof(Node));
    node->kind = kNodeDoWhile;
    node->rhs = stmt();
    if (!consume_token(kTokenWhile))
      error_at(token->str, "'while'ではありません");
    if (!consume("(")) error_at(token->str, "'('ではありません");
    node->lhs = expr();
    if (!consume(")")) error_at(token->str, "')'ではありません");
    if (!consume(";")) error_at(token->str, "';'ではありません");
    return node;
  } else if (consume_token(kTokenFor)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeFor;
    if (!consume("(")) error_at(token->str, "'('ではありません");
    node->for_clause1 = expr();
    expect(';');
    node->for_expression2 = expr();
    expect(';');
    node->for_expression3 = expr();
    if (!consume(")")) error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    return node;
  } else if (consume_token(kTokenSwitch)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeSwitch;
    expect('(');
    node->lhs = expr();
    expect(')');
    expect('{');
    Node *switch_case;
    Node **switch_case_tail = &node->rhs;
    while (1) {
      Token *tok;
      if (consume_token(kTokenSwitchCase)) {
        switch_case = calloc(1, sizeof(Node));
        switch_case->kind = kNodeSwitchCase;
        tok = consume_token(kTokenIdent);
        if (tok) {
          EnumDef *enum_def = find_enum_def(tok->str, tok->len);
          if (enum_def) {
            switch_case->val = enum_def->val;
          } else {
            error_at(tok->str, "定義されていない値です。");
          }
        } else {
          switch_case->val = expect_number();
        }
      } else if (consume_token(kTokenSwitchDefault)) {
        switch_case = calloc(1, sizeof(Node));
        switch_case->kind = kNodeSwitchDefault;
      } else {
        error_at(token->str, "'case'ではありません");
      }
      expect(':');
      *switch_case_tail = switch_case;
      switch_case_tail = &switch_case->lhs;
      Node *stmt_node;
      Node **tail = &switch_case->rhs;
      while (1) {
        if (tok = consume_token(kTokenSwitchCase)) {
          token = tok;
          break;
        } else if (tok = consume_token(kTokenSwitchDefault)) {
          token = tok;
          break;
        } else if (consume("}")) {
          return node;
        }
        stmt_node = stmt();
        *tail = stmt_node;
        tail = &stmt_node->next;
      }
    }
  } else if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeBlock;
    if (consume("}")) return node;
    VarTable *vt = calloc(1, sizeof(VarTable));
    vt->parent = locals;
    locals = vt;
    node->lhs = stmt();
    current = node->lhs;
    while (!consume("}")) {
      current->next = stmt();
      current = current->next;
    }
    locals = vt->parent;
    return node;
  } else if (type = parse_type()) {
    node = variable_definition(type, locals);
  } else if (consume(";")) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeNOP;
    return node;
  } else if (macro_define()) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeNOP;
    return node;
  } else if (macro_undef()) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeNOP;
    return node;
  } else if (macro_ifndef()) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeNOP;
    return node;
  } else if (consume_token(kTokenMacroEndif)) {
    node = calloc(1, sizeof(Node));
    node->kind = kNodeNOP;
    return node;
  } else {
    node = expr();
  }

  expect(';');
  return node;
}

// 構造体定義
Node *struct_definition() {
  if (!consume_token(kTokenStruct)) {
    return NULL;
  }
  Node *node = new_node_ident(kNodeStruct, expect_ident());
  if (!consume("{")) {
    return NULL;
  }
  StructDef *struct_def = new_struct_def(node);
  Type *type;
  StructMember *member;
  Node *cur = node;
  while (type = parse_type()) {
    cur->next = new_node_ident(kNodeVarDef, expect_ident());
    cur->next->type = type;
    new_struct_member(struct_def, cur->next);
    cur = cur->next;
    expect(';');
  }
  expect('}');

  return node;
}

// 列挙定義
bool enum_definition() {
  if (!consume_token(kTokenEnum)) {
    return false;
  }
  Token *tok = consume_token(kTokenIdent);
  if (tok != NULL) {
  }
  if (!consume("{")) {
    return false;
  }
  int val = 0;
  for (;;) {
    if (consume("}")) {
      return true;
    }
    tok = consume_token(kTokenIdent);
    if (find_enum_def(tok->str, tok->len)) {
      error_at(tok->str, "その列挙は定義済みです");
    }
    new_enum_def(tok->str, tok->len, val);
    val++;
    if (!consume(",")) {
      break;
    }
  }
  expect('}');
  return true;
}

// typedef定義
bool typedef_definition() {
  if (!consume_token(kTokenTypedef)) {
    return false;
  }
  Type *type;
  if (!enum_definition()) {
    if (consume_token(kTokenStruct)) {
      Token *tag_tok = expect_ident();
      Token *tok = expect_ident();
      TypedefDef *typedef_def = new_typedef_def(tok->str, tok->len);
      typedef_def->ty = kTypeStruct;
      typedef_def->type_name = tag_tok->str;
      typedef_def->type_name_len = tag_tok->len;
      return true;
    } else if (type = parse_type()) {
      Token *tok = expect_ident();
      TypedefDef *typedef_def = new_typedef_def(tok->str, tok->len);
      typedef_def->ty = type->ty;
      return true;
    } else {
      error("typedef定義エラーです");
    }
  }
  Token *tok = expect_ident();
  new_typedef_def(tok->str, tok->len);
  return true;
}

// マクロ
bool macro_define() {
  if (!consume_token(kTokenMacroDefine)) {
    return false;
  }
  Token *tok = expect_ident();
  if (find_macro_def(tok->str, tok->len, 0)) {
    error_at(tok->str, "そのマクロは定義済みです");
  }
  MacroDef *macro_def = new_macro_def(tok->str, tok->len, token);
  if (consume("(")) {
    // 関数形式マクロ
    macro_def->is_function_like = 1;
    if (!consume(")")) {
      MacroParam *param = calloc(1, sizeof(MacroParam));
      macro_def->param = param;
      for (;;) {
        tok = expect_ident();
        param->name = tok->str;
        param->name_len = tok->len;
        if (!consume(",")) {
          expect(')');
          break;
        }
        param->next = calloc(1, sizeof(MacroParam));
        param = param->next;
      }
    }
    macro_def->tok = token;
  }
  if (token->kind == kTokenNewline) {
    // 値なし
    macro_def->end_tok = token;
    token = token->next;
    return true;
  }
  do {
    macro_def->end_tok = token;
    token = token->next;
  } while (token->kind != kTokenNewline);
  token = token->next;
  return true;
}

bool macro_undef() {
  if (!consume_token(kTokenMacroUndef)) {
    return false;
  }
  Token *tok = expect_ident();
  delete_macro_def(tok->str, tok->len);
  return true;
}

bool macro_ifndef() {
  if (!consume_token(kTokenMacroIfndef)) {
    return false;
  }
  Token *tok = expect_ident();
  if (find_macro_def(tok->str, tok->len, 0)) {
    do {
      token = token->next;
    } while (token->kind != kTokenMacroEndif);
    token = token->next;
  }
  return true;
}

// グローバル定義
//
// - 関数
// - グローバル変数
// - 構造体
// - 列挙
// - マクロ
Node *global_definition() {
  Node *node;
  Type *type;
  Token *cur = token;
  int is_extern = 0;
  if (node = struct_definition()) {
    expect(';');
    return node;
  }
  if (enum_definition()) {
    expect(';');
    return NULL;
  }
  if (typedef_definition()) {
    expect(';');
    return NULL;
  }
  if (macro_define()) {
    return NULL;
  }
  if (macro_undef()) {
    return NULL;
  }
  if (macro_ifndef() || consume_token(kTokenMacroEndif)) {
    return NULL;
  }
  token = cur;
  if (consume_token(kTokenExtern)) {
    is_extern = 1;
  }
  if (!(type = parse_type())) {
    error_at(token->str, "定義ではありません");
  }
  type->is_extern = is_extern;
  Token *tok = expect_ident();
  if (consume("(")) {
    // 関数定義
    locals = calloc(1, sizeof(VarTable));
    locals_offset = 0;
    node = calloc(1, sizeof(Node));
    node->kind = kNodeFunc;
    node->name = tok->str;
    node->len = tok->len;
    if (!consume(")")) {
      // パラメーター
      Node *params = NULL;
      Node *current;
      for (;;) {
        if (!(type = parse_type())) {
          error_at(token->str, "定義ではありません");
        }
        if (type->ty == kTypeVoid) {
          expect(')');
          break;
        }
        if (params == NULL) {
          params = variable_definition(type, locals);
          current = params;
        } else {
          current->next = variable_definition(type, locals);
          current = current->next;
        }
        if (current->type->ty == kTypeArray) {
          locals->vars->type->ty = kTypePtr;
          locals->vars->offset = locals->vars->offset + get_type_size(kTypePtr);
          current->offset = locals->vars->offset;
          locals_offset = locals->vars->offset;
        }
        if (!consume(",")) {
          expect(')');
          break;
        }
      }
      node->lhs = params;
    }
    // declarations
    if (consume(";")) {
      return NULL;
    }
    // definition
    expect('{');
    Node *block_node = calloc(1, sizeof(Node));
    Node *current;
    block_node->kind = kNodeBlock;
    if (!consume("}")) {
      block_node->lhs = stmt();
      current = block_node->lhs;
      while (!consume("}")) {
        current->next = stmt();
        current = current->next;
      }
    }
    node->rhs = block_node;
    node->lvar_size = locals_offset;
    return node;
  } else {
    // グローバル変数定義
    token = tok;
    node = variable_definition(type, globals);
    expect(';');
    return is_extern ? NULL : node;
  }
}

void program() {
  int i = 0;
  Node *node;
  globals = calloc(1, sizeof(VarTable));
  while (!at_eof()) {
    node = global_definition();
    if (node) {
      code[i++] = node;
    }
  }
  code[i] = NULL;
}
