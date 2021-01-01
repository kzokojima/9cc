#ifndef NINECC_MACRO_H_
#define NINECC_MACRO_H_

#include "parse.h"

typedef struct MacroParam MacroParam;
struct MacroParam {
  char *name;
  int name_len;
  MacroParam *next;
};

// マクロ
typedef struct MacroDef MacroDef;
struct MacroDef {
  MacroDef *next;
  char *name;
  int name_len;
  Token *tok;
  Token *end_tok;
  int is_function_like;
  MacroParam *param;
};

// マクロリスト
extern MacroDef *macro_def_list;

MacroDef *find_macro_def(char *str, int len, int depth);
MacroDef *new_macro_def(char *str, int len, Token *tok);
void delete_macro_def(char *str, int len);
void rollback_macro_def(int macro_rollback_num);

#endif  // NINECC_MACRO_H_
