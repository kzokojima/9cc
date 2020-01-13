#include "struct.h"

#include "9cc.h"

// 構造体定義リスト
StructDef *struct_def_list;

// 構造体定義を検索する
StructDef *find_struct_def(char *str, int len) {
  for (StructDef *var = struct_def_list; var; var = var->next)
    if (var->name_len == len && !memcmp(var->name, str, len))
      return var;
  return NULL;
}

// 構造体定義からメンバを検索する
StructMember *find_struct_member(StructDef *struct_def, char *str, int len) {
  for (StructMember *member = struct_def->member_list; member; member = member->next) {
    if (member->name_len == len && !memcmp(member->name, str, len)) {
      return member;
    }
  }
  return NULL;
}

// メンバを作成する
StructMember *new_struct_member(StructDef *struct_def, Node *node) {
  StructMember *member = calloc(1, sizeof(StructMember));
  member->type = node->type;
  member->name = node->name;
  member->name_len = node->len;
  member->offset = (struct_def->member_list == NULL) ? 0 : struct_def->member_list->offset + get_type_size(struct_def->member_list->type->ty);
  member->next = struct_def->member_list;
  struct_def->member_list = member;
  return member;
}

// 構造体定義を作成する
StructDef *new_struct_def(Node *node) {
  StructDef *struct_def = calloc(1, sizeof(StructDef));
  struct_def->next = struct_def_list;
  struct_def->name = node->name;
  struct_def->name_len = node->len;
  struct_def_list = struct_def;
  return struct_def;
}

// 構造体のサイズ
int get_struct_size(Type *type) {
  StructDef *struct_def = find_struct_def(type->name, type->name_len);
  if (struct_def == NULL) {
    error("構造体が不明です");
  }
  int size = 0;
  StructMember *member = struct_def->member_list;
  while (member) {
    size += get_type_size_by_type(member->type);
    member = member->next;
  }
  return size;
}
