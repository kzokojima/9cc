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

// 入力ファイル名
char *filename;

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
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
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

  filename = argv[1];
  if ((file = fopen(filename, "r")) == NULL) {
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

  // 文字列定数
  gen_string_constants();

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }

  return 0;
}
