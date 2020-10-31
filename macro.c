#include "macro.h"

#include <stdlib.h>
#include <string.h>

// マクロリスト
MacroDef *macro_def_list;

// マクロを検索する
MacroDef *find_macro_def(char *str, int len, int depth) {
  if (depth == 0)
    depth = 2147483647;

  for (MacroDef *var = macro_def_list; var; var = var->next) {
    if (depth-- <= 0)
      return NULL;
    if (var->name_len == len && !memcmp(var->name, str, len)) return var;
  }
  return NULL;
}

// マクロを作成する
MacroDef *new_macro_def(char *str, int len, Token *tok) {
  MacroDef *macro_def = calloc(1, sizeof(MacroDef));
  macro_def->next = macro_def_list;
  macro_def->name = str;
  macro_def->name_len = len;
  macro_def->tok = tok;
  macro_def_list = macro_def;
  return macro_def;
}

void delete_macro_def(char *str, int len) {
  MacroDef **prev = &macro_def_list;
  for (MacroDef *var = macro_def_list; var; var = var->next) {
    if (var->name_len == len && !memcmp(var->name, str, len)) {
      *prev = var->next;
      break;
    }
    *prev = var;
  }
}

// マクロを巻き戻す
void rollback_macro_def(int macro_rollback_num) {
  while (macro_rollback_num--) {
    macro_def_list = macro_def_list->next;
  }
}
