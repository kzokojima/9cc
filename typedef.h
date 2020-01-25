#ifndef NINECC_TYPEDEF_H_
#define NINECC_TYPEDEF_H_

#include "parse.h"

// typedef
typedef struct TypedefDef TypedefDef;
struct TypedefDef {
  TypedefDef *next;
  char *name;
  int name_len;
  TypeEnum ty;
  char *type_name;
  int type_name_len;
};

// typedefリスト
extern TypedefDef *typedef_def_list;

TypedefDef *find_typedef_def(char *str, int len);
TypedefDef *new_typedef_def(char *str, int len);

#endif  // NINECC_TYPEDEF_H_
