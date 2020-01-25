#include "typedef.h"

#include <stdlib.h>
#include <string.h>

// typedefリスト
TypedefDef *typedef_def_list;

// typedefを検索する
TypedefDef *find_typedef_def(char *str, int len) {
  for (TypedefDef *var = typedef_def_list; var; var = var->next)
    if (var->name_len == len && !memcmp(var->name, str, len))
      return var;
  return NULL;
}

// typedefを作成する
TypedefDef *new_typedef_def(char *str, int len) {
  TypedefDef *typedef_def = calloc(1, sizeof(TypedefDef));
  typedef_def->next = typedef_def_list;
  typedef_def->name = str;
  typedef_def->name_len = len;
  typedef_def_list = typedef_def;
  return typedef_def;
}
