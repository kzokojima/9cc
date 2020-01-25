#ifndef NINECC_LIB_H_
#define NINECC_LIB_H_

#include <stdio.h>

// 入力ファイル名
extern char *filename;

// 入力プログラム
extern char *user_input;

void emit(char *fmt, ...);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *read_file(char *filename, int buffer_size);

#endif  // NINECC_LIB_H_
