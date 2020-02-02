#include "enum.h"

#include <stdlib.h>
#include <string.h>

// 列挙リスト
EnumDef *enum_def_list;

// 列挙を検索する
EnumDef *find_enum_def(char *str, int len) {
  for (EnumDef *var = enum_def_list; var; var = var->next)
    if (var->name_len == len && !memcmp(var->name, str, len)) return var;
  return NULL;
}

// 列挙を作成する
EnumDef *new_enum_def(char *str, int len, int val) {
  EnumDef *enum_def = calloc(1, sizeof(EnumDef));
  enum_def->next = enum_def_list;
  enum_def->name = str;
  enum_def->name_len = len;
  enum_def->val = val;
  enum_def_list = enum_def;
  return enum_def;
}
