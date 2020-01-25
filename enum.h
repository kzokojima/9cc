#ifndef NINECC_ENUM_H_
#define NINECC_ENUM_H_

// 列挙
typedef struct EnumDef EnumDef;
struct EnumDef {
  EnumDef *next;
  char *name;
  int name_len;
  int val;
};

// 列挙リスト
extern EnumDef *enum_def_list;

EnumDef *find_enum_def(char *str, int len);
EnumDef *new_enum_def(char *str, int len, int val);

#endif  // NINECC_ENUM_H_
