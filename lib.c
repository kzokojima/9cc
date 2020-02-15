#include "lib.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 入力ファイル名
char *filename;

// 入力プログラム
char *user_input;

// 出力ファイル
FILE *output;

void emit(char *fmt, ...) {
  if (output == NULL) {
    char *output_filename = malloc(strlen(filename) + 1);
    strcpy(output_filename, filename);
    char *pos = strstr(output_filename, ".c");
    memcpy(pos, ".s", 2);
    if ((output = fopen(output_filename, "w")) == NULL) {
      error("ファイルが開けません");
    }
  }

  va_list ap;
  va_start(ap, fmt);
  vfprintf(output, fmt, ap);
  fprintf(output, "\n");
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (user_input < line && line[-1] != '\n') line--;

  char *end = loc;
  while (*end != '\n') end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n') line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *read_file(char *filename, int buffer_size) {
  FILE *file;
  size_t file_size;
  char *buffer = malloc(buffer_size);

  if ((file = fopen(filename, "r")) == NULL) {
    error("ファイルが開けません");
  }
  file_size = fread(buffer, 1, buffer_size, file);
  buffer[file_size] = '\0';
  fclose(file);

  return buffer;
}

// Interprets an integer value in a byte string
//  interpreting string:
//    100
//    0100
//    0x100
//    0b100
unsigned long long mystrtoull(const char *str, char **str_end) {
  if (!strncmp(str, "0b", 2)) {
    unsigned long long num = 0;
    str += 2;
    while (*str) {
      if (*str == '0') {
        num <<= 1;
      } else if (*str == '1') {
        num <<= 1;
        num += 1;
      } else {
        break;
      }
      str++;
    }
    *str_end = (char *)str;
    return num;
  } else {
    return strtoull(str, str_end, 0);
  }
}

int strcount(char *str, char *end, char c) {
  int no = 0;
  while (str != end) {
    if (*str == c) {
      no++;
    }
    str++;
  }
  return no;
}
