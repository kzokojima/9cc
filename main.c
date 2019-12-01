#include "9cc.h"

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー個所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int main(int argc, char **argv) {
  FILE *file;
  size_t file_size;
  char buffer[1024 * 1024];  // 1MB

  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  if ((file = fopen(argv[1], "r")) == NULL) {
    error("ファイルが開けません");
    return 1;
  }
  file_size = fread(buffer, 1, sizeof buffer, file);
  buffer[file_size] = '\0';
  fclose(file);

  // トークナイズする
  user_input = buffer;
  token = tokenize();
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }

  return 0;
}
