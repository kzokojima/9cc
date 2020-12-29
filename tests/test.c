#include <stdio.h>
#include <stdlib.h>

#define assert(expr) \
do { \
  if (!(expr)) { \
    printf("Assertion `"); \
    printf(#expr); \
    printf("' failed.\n"); \
    fflush(0); \
    abort(); \
  } \
} while (0)

void test_assert(void) {
  assert(42 == 42);
}

void test_block_var(void) {
  int r = 0;
  {
    int i = 2;
    r = r + i;
  }
  {
    int i = 40;
    r = r + i;
  }
  assert(r == 42);
}

int main() {
  test_assert();
  test_block_var();
}
