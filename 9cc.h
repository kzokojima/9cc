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
} NodeKind;

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
  TK_EOF,      // 入力の終わりを表すトークン
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
extern void gen(Node *node);
