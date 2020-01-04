#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  kNodeAdd,
  kNodeSub,
  kNodeMul,
  kNodeDiv,
  kNodeNum,
  kNodeEq,
  kNodeNe,
  kNodeLt,
  kNodeLe,
  kNodeAssign,  // =
  kNodeLocalVar,    // ローカル変数
  kNodeReturn,  // return
  kNodeIf,      // if
  kNodeWhile,   // while
  kNodeFor,     // for
  kNodeBlock,   // { ... }
  kNodeFuncCall,      // foo(...)
  kNodeFunc,  // foo(...) { ... }
  kNodeAddr,    // &var
  kNodeDeref,   // *ptr
  kNodeVarDef,  // int var
  kNodeGlobalVar,    // グローバル変数
  kNodeString,  // 文字列
  kNodeInitializerList,  // 初期化子リスト
} NodeKind;

typedef struct Type Type;

struct Type {
  enum { kTypeInt, kTypePtr, kTypeArray, kTypeChar } ty;
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
  char *name; // 名前
  int len;    // 名前の長さ
  int lvar_size;  // ローカル変数サイズ
  Type *type;
  StringConstant *string_constant;
};

// トークンの種類
typedef enum {
  kTokenReserved, // 記号
  kTokenIdent,    // 識別子
  kTokenNum,      // 整数トークン
  kTokenReturn,   // return
  kTokenIf,       // if
  kTokenElse,     // else
  kTokenWhile,    // while
  kTokenFor,      // for
  kTokenSizeof,   // sizeof
  kTokenEof,      // 入力の終わりを表すトークン
  kTokenString,   // 文字列
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがkTokenNumの場合、その数値
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
extern int get_type_size(int type);
