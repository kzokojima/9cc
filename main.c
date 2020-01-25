#include "codegen.h"
#include "lib.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
  }
  filename = argv[1];

  // トークナイズする
  user_input = read_file(filename, 1024 * 1024);
  token = tokenize();
  program();

  // アセンブリの前半部分を出力
  emit(".intel_syntax noprefix");

  // 文字列定数
  gen_string_constants();

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }

  return 0;
}
