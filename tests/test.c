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

int main() {
  test_assert();
}
