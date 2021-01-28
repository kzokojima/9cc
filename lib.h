#ifndef NINECC_LIB_H_
#define NINECC_LIB_H_

#include <stdio.h>

typedef unsigned long long SIZE_T;

// 入力ファイル名
extern char *filename;

// 入力プログラム
extern char *user_input;

void emit(char *fmt, ...);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *read_file(char *filename, int buffer_size);
unsigned long long mystrtoull(const char *str, char **str_end);
int strcount(char *str, char *end, char c);
char *strescape(char *src, int src_len);

#endif  // NINECC_LIB_H_
