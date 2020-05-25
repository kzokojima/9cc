#ifndef NINECC_MACRO_H_
#define NINECC_MACRO_H_

#include "parse.h"

// マクロ
typedef struct MacroDef MacroDef;
struct MacroDef {
  MacroDef *next;
  char *name;
  int name_len;
  Token *tok;
  Token *end_tok;
};

// マクロリスト
extern MacroDef *macro_def_list;

MacroDef *find_macro_def(char *str, int len);
MacroDef *new_macro_def(char *str, int len, Token *tok);

#endif  // NINECC_MACRO_H_
