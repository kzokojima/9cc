#ifndef NINECC_PARSE_H_
#define NINECC_PARSE_H_

#include <stddef.h>

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
  // logical NOT(!)
  //    lhs: Node
  kNodeLogicalNot,
  // logical OR(||)
  //    lhs: Node
  //    rhs: Node
  kNodeLogicalOr,
  // logical AND(&&)
  //    lhs: Node
  //    rhs: Node
  kNodeLogicalAnd,
  kNodeAssign,           // =
  kNodeLocalVar,         // ローカル変数
  kNodeReturn,           // return
  kNodeNOP,              // NOP
  kNodeIf,               // if
  kNodeWhile,            // while
  kNodeDoWhile,          // do-while
  kNodeFor,              // for
  kNodeBlock,            // { ... }
  kNodeFuncCall,         // foo(...)
  kNodeFunc,             // foo(...) { ... }
  kNodeAddr,             // &var
  kNodeDeref,            // *ptr
  kNodeVarDef,           // int var
  kNodeGlobalVar,        // グローバル変数
  kNodeString,           // 文字列
  kNodeInitializerList,  // 初期化子リスト
  kNodeStruct,           // 構造体
  // 構造体メンバ
  //    foo.i = ...
  //
  // kNodeStructMember
  //    lhs: Node(kNodeLocalVar, ...)
  //    name: メンバー名
  //    offset: オフセット
  kNodeStructMember,
  // 構造体ポインタメンバ
  //    foo->i = ...
  //
  // kNodeStructPointerMember
  //    lhs: Node(kNodeLocalVar, ...)
  //    name: メンバー名
  //    offset: オフセット
  kNodeStructPointerMember,
  kNodeBreak,
  // switch statement
  //    switch (expression) {
  //    case 0:
  //      i = 100;
  //      break;
  //    default:
  //      i = 1000;
  //      break;
  //    }
  //
  //    lhs: expression
  //    rhs: Node(kNodeSwitchCase)
  kNodeSwitch,
  // switch case
  //    lhs: Node(kNodeSwitchCase, kNodeSwitchDefault)
  //    rhs: statement Node
  //      next: statement Node
  //        ...
  //    val: constant_expression
  kNodeSwitchCase,
  // switch default
  //    lhs: Node(kNodeSwitchCase)
  //    rhs: statement Node
  //      next: statement Node
  //        ...
  kNodeSwitchDefault,
  // Ternary conditional
  //    lhs: Node
  //    rhs: true Node
  //      next: false Node
  kNodeTernaryConditional,
} NodeKind;

typedef enum {
  kTypeInt,
  kTypeUInt,
  kTypeLLong,
  kTypeULLong,
  kTypeShort,
  kTypeUShort,
  kTypePtr,
  kTypeArray,
  kTypeChar,
  kTypeStruct,
  kTypeVoid,
} TypeEnum;

typedef struct Type Type;

struct Type {
  TypeEnum ty;
  struct Type *ptr_to;
  size_t array_size;
  char *name;
  int name_len;
};

// 文字列定数
typedef struct StringConstant StringConstant;
struct StringConstant {
  StringConstant *next;  // 次の文字列定数かNULL
  char *str;             // 文字列
  int len;               // 長さ
  int index;             // インデックス
};
extern StringConstant *string_constants;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node *els;              // else
  Node *for_clause1;      // for
  Node *for_expression2;  // for
  Node *for_expression3;  // for
  Node *next;             // 次
  unsigned long long val;
  int offset;
  char *name;     // 名前
  int len;        // 名前の長さ
  int lvar_size;  // ローカル変数サイズ
  Type *type;
  StringConstant *string_constant;
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next;  // 次の変数かNULL
  char *name;  // 変数の名前
  int len;     // 名前の長さ
  int offset;  // RBPからのオフセット
  Type *type;  // データ型
};

// トークンの種類
typedef enum {
  kTokenReserved,       // 記号
  kTokenIdent,          // 識別子
  kTokenNum,            // 整数トークン
  kTokenReturn,         // return
  kTokenIf,             // if
  kTokenElse,           // else
  kTokenWhile,          // while
  kTokenFor,            // for
  kTokenSizeof,         // sizeof
  kTokenEof,            // 入力の終わりを表すトークン
  kTokenString,         // 文字列
  kTokenStruct,         // 構造体
  kTokenEnum,           // 列挙
  kTokenTypedef,        // typedef
  kTokenBreak,          // break
  kTokenSwitch,         // switch
  kTokenSwitchCase,     // switch case
  kTokenSwitchDefault,  // switch default
  kTokenNewline,
  kTokenMacro,          // マクロ
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind;          // トークンの型
  Token *next;             // 次の入力トークン
  unsigned long long val;  // kindがkTokenNumの場合、その数値
  char *str;               // トークン文字列
  int len;                 // トークンの長さ
};

// ローカル変数
extern LVar *locals;

// 現在着目しているトークン
extern Token *token;

extern Node *code[100];

Token *tokenize();
void program();
int get_type_size(int type);
int get_type_size_by_type(Type *type);

#endif  // NINECC_PARSE_H_
