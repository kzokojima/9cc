#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,  // =
  ND_LVAR,    // ローカル変数
  ND_RETURN,  // return
  ND_IF,      // if
  ND_WHILE,   // while
  ND_FOR,     // for
  ND_BLOCK,   // { ... }
  ND_FN,      // foo(...)
  ND_FN_DEF,  // foo(...) { ... }
  ND_ADDR,    // &var
  ND_DEREF,   // *ptr
  ND_DEFVAR,  // int var
  ND_GVAR,    // グローバル変数
  ND_STRING,  // 文字列
} NodeKind;

typedef struct Type Type;

struct Type {
  enum { INT, PTR, ARRAY, CHAR } ty;
  struct Type *ptr_to;
  size_t array_size;
};

// 文字列定数
typedef struct StringConstant StringConstant;
struct StringConstant {
  StringConstant *next;   // 次の文字列定数かNULL
  char *str;              // 文字列
  int len;                // 長さ
  int index;              // インデックス
};
extern StringConstant *string_constants;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node *els;  // else
  Node *for_clause1;      // for
  Node *for_expression2;  // for
  Node *for_expression3;  // for
  Node *next;             // 次
  int val;
  int offset;
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int lvar_size;  // ローカル変数サイズ
  Type *type;
  StringConstant *string_constant;
};

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_RETURN,   // return
  TK_IF,       // if
  TK_ELSE,     // else
  TK_WHILE,    // while
  TK_FOR,      // for
  TK_SIZEOF,   // sizeof
  TK_EOF,      // 入力の終わりを表すトークン
  TK_STRING,   // 文字列
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
  Type *type; // データ型
};

// ローカル変数
extern LVar *locals;

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

extern Node *code[100];

extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
extern Token *tokenize();
extern Node *expr();
extern void program();
extern void gen_string_constants();
extern void gen(Node *node);
int get_type_size(int type);
