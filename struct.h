#ifndef NINECC_STRUCT_H_
#define NINECC_STRUCT_H_

#include "parse.h"

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

StructDef *find_struct_def(char *str, int len);
StructMember *find_struct_member(StructDef *struct_def, char *str, int len);
StructMember *new_struct_member(StructDef *struct_def, Node *node);
StructDef *new_struct_def(Node *node);
int get_struct_size(Type *type);

#endif  // NINECC_STRUCT_H_
