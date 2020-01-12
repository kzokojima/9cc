#ifndef NINECC_TYPEDEF_H_
#define NINECC_TYPEDEF_H_

// typedef
typedef struct TypedefDef TypedefDef;
struct TypedefDef {
  TypedefDef *next;
  char *name;
  int name_len;
};

// typedefリスト
extern TypedefDef *typedef_def_list;

extern TypedefDef *find_typedef_def(char *str, int len);
extern TypedefDef *new_typedef_def(char *str, int len);

#endif  // NINECC_TYPEDEF_H_
