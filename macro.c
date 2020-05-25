#include "macro.h"

#include <stdlib.h>
#include <string.h>

// マクロリスト
MacroDef *macro_def_list;

// マクロを検索する
MacroDef *find_macro_def(char *str, int len) {
  for (MacroDef *var = macro_def_list; var; var = var->next)
    if (var->name_len == len && !memcmp(var->name, str, len)) return var;
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
