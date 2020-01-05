#ifndef NINECC_STRUCT_H_
#define NINECC_STRUCT_H_

#include "9cc.h"

// 構造体メンバー
typedef struct StructMember StructMember;
struct StructMember {
  StructMember *next;
  Type *type;
  char *name;
  int name_len;
  int offset;
};

// 構造体
typedef struct StructDef StructDef;
struct StructDef {
  StructDef *next;
  char *name;
  int name_len;
  StructMember *member_list;
};

// 構造体リスト
extern StructDef *struct_def_list;

extern StructDef *find_struct_def(char *str, int len);
extern StructMember *find_struct_member(StructDef *struct_def, char *str, int len);
extern StructMember *new_struct_member(StructDef *struct_def, Node *node);
extern StructDef *new_struct_def(Node *node);
extern int get_struct_size(Type *type);

#endif  // NINECC_STRUCT_H_
