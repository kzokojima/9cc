#include <stdio.h>
int foo() {
  printf("OK\n");
  return 42;
}
int add2(int v1, int v2) { return v1 + v2; }
int add6(int v1, int v2, int v3, int v4, int v5, int v6) {
  return v1 + v2 + v3 + v4 + v5 + v6;
}
void print_int(int n) { printf("%d\n", n); }
void print_uint(unsigned n) { printf("%u\n", n); }
