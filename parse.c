#include "9cc.h"


// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

Node *code[100];

// ローカル変数
LVar *locals;

// データ型のサイズ
int get_type_size(int type) {
  switch (type) {
  case INT:
    return 4;
  case PTR:
    return 8;
  default:
    return -1;
  }
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_token(TokenKind kind) {
  if (token->kind != kind)
    return false;
  Token *current = token;
  token = token->next;
  return current;
}

Token *comsume_ident(char *str) {
  if (token->kind != TK_IDENT)
    return false;
  if (memcmp(token->str, str, strlen(str)))
    return false;
  Token *current = token;
  token = token->next;
  return current;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
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

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (! memcmp(p, "==", 2) ||
        ! memcmp(p, "!=", 2) ||
        ! memcmp(p, "<=", 2) ||
        ! memcmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '='
        || *p == '{' || *p == '}' || *p == ',' || *p == '&' || *p == '[' || *p == ']') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if (keyword_len = expect_keyword(p, "return")) {
      cur = new_token(TK_RETURN, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (keyword_len = expect_keyword(p, "if")) {
      cur = new_token(TK_IF, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "else")) {
      cur = new_token(TK_ELSE, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (keyword_len = expect_keyword(p, "while")) {
      cur = new_token(TK_WHILE, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "for")) {
      cur = new_token(TK_FOR, cur, p, 0);
      p += keyword_len;
      continue;
    }
    if (keyword_len = expect_keyword(p, "sizeof")) {
      cur = new_token(TK_SIZEOF, cur, p, 0);
      p += keyword_len;
      continue;
    }

    if (('A' <= *p && *p <= 'Z') || *p == '_' || ('a' <= *p && *p <= 'z')) {
      size_t len = strspn(p + 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
      len++;
      cur = new_token(TK_IDENT, cur, p, len);
      p += len;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 型を検出
int detect_type(Node *node) {
  switch (node->kind) {
  case ND_NUM:
  case ND_MUL:
  case ND_DIV:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_DEREF:
    return INT;
  case ND_ADDR:
    return PTR;
  case ND_LVAR:
    return node->type->ty;
  case ND_ADD:
  case ND_SUB:
    {
      int ty = detect_type(node->lhs);
      if (ty == PTR)
        return ty;
      ty = detect_type(node->rhs);
      if (ty == PTR)
        return ty;

      return INT;
    }
  }

  return -1;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(')');
    return node;
  }

  Token *tok = consume_token(TK_IDENT);
  if (tok) {
    // 関数呼び出し
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FN;
      node->name = tok->str;
      node->len = tok->len;
      if (consume(")"))
        return node;
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

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
      node->name = lvar->name;
      node->len = lvar->len;
      node->type = lvar->type;
      if (consume("[")) {
        // 配列添字
        node = new_node(ND_DEREF, new_node(ND_ADD, node, new_node_num(expect_number())), NULL);
        expect(']');
      }
    } else {
      error_at(tok->str, "未定義の変数です");
    }
    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  if (consume("&"))
    return new_node(ND_ADDR, primary(), NULL);
  if (consume("*"))
    return new_node(ND_DEREF, primary(), NULL);
  if (consume_token(TK_SIZEOF)) {
    Node *node = unary();
    if (node->kind == ND_LVAR && node->type->ty == ARRAY) {
      return new_node_num(get_type_size(node->type->ptr_to->ty) * node->type->array_size);
    }
    int ty = detect_type(node);
    if (ty != -1)
      return new_node_num(get_type_size(ty));
    error("データ型が不明です");
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else  if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr() {
  Node *node = assign();
}

// 変数定義
Node *defvar() {
  Node *node;
  Type *type = calloc(1, sizeof(Type));
  type->ty = INT;
  while (consume("*")) {
    Type *t = calloc(1, sizeof(Type));
    t->ty = PTR;
    t->ptr_to = type;
    type = t;
  }
  Token *tok = consume_token(TK_IDENT);
  if (!tok) {
    error_at(token->str, "識別子ではありません");
  }
  LVar *lvar = find_lvar(tok);
  if (lvar) {
    error_at(tok->str, "定義済みの変数です");
  }
  node = calloc(1, sizeof(Node));
  node->kind = ND_DEFVAR;
  lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->offset = (locals ? locals->offset : 0) + get_type_size(type->ty);
  lvar->type = type;
  if (consume("[")) {
    // 配列定義
    type = calloc(1, sizeof(Type));
    type->ty = ARRAY;
    type->array_size = expect_number();
    type->ptr_to = lvar->type;
    lvar->offset = (locals ? locals->offset : 0) + type->array_size * get_type_size(lvar->type->ty);
    lvar->type = type;
    expect(']');
  }
  node->name = tok->str;
  node->len = tok->len;
  node->offset = lvar->offset;
  node->type = lvar->type;
  locals = lvar;
  return node;
}

Node *stmt() {
  Node *node;
  Node *current;

  if (consume_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if (consume_token(TK_IF)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    if (!consume("("))
      error_at(token->str, "'('ではありません");
    node->lhs = expr();
    if (!consume(")"))
      error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    if (consume_token(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  } else if (consume_token(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    if (!consume("("))
      error_at(token->str, "'('ではありません");
    node->lhs = expr();
    if (!consume(")"))
      error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    return node;
  } else if (consume_token(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    if (!consume("("))
      error_at(token->str, "'('ではありません");
    node->for_clause1 = expr();
    expect(';');
    node->for_expression2 = expr();
    expect(';');
    node->for_expression3 = expr();
    if (!consume(")"))
      error_at(token->str, "')'ではありません");
    node->rhs = stmt();
    return node;
  } else if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    if (consume("}"))
      return node;
    node->lhs = stmt();
    current = node->lhs;
    while (!consume("}")) {
      current->next = stmt();
      current = current->next;
    }
    return node;
  } else if (comsume_ident("int")) {
    node = defvar();
  } else {
    node = expr();
  }

  expect(';');
  return node;
}

Node *function_definition() {
  Node *node;
  if (!comsume_ident("int")) {
    error_at(token->str, "定義ではありません");
  }
  Token *tok = consume_token(TK_IDENT);
  if (tok) {
    expect('(');
    locals = NULL;
    node = calloc(1, sizeof(Node));
    node->kind = ND_FN_DEF;
    node->name = tok->str;
    node->len = tok->len;
    if (!consume(")")) {
      // パラメーター
      Node *params = NULL;
      Node *current;
      for (;;) {
        if (!comsume_ident("int")) {
          error_at(token->str, "定義ではありません");
        }
        if (params == NULL) {
          params = defvar();
          current = params;
        } else {
          current->next = defvar();
          current = current->next;
        }
        if (current->type->ty == ARRAY) {
          locals->type->ty = PTR;
          locals->offset = locals->offset + get_type_size(PTR);
          current->offset = locals->offset;
        }
        if (!consume(",")) {
          expect(')');
          break;
        }
      }
      node->lhs = params;
    }
    node->rhs = stmt();
    if (locals) {
      node->lvar_size = locals->offset;
    }
    return node;
  } else {
    error_at(token->str, "識別子ではありません");
  }
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = function_definition();
  code[i] = NULL;
}
