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

void test_if_statements(void) {
  { int a; a=42;if(1)a=1; assert(a == 1); }
  { int a; a=42;if(0)a=1; assert(a == 42); }
  { int a; a=42;if(1)a=1;else a=2; assert(a == 1); }
  { int a; a=42;if(0)a=1;else a=2; assert(a == 2); }
  { int a; int b; a=42;if(1)a=1;else a=2;if(1)b=1;else b=2; assert(a+b == 2); }
  { int a; int b; a=42;if(1)a=1;else a=2;if(0)b=1;else b=2; assert(a+b == 3); }
  { int a; a=42;if(1)a=1;else if(1)a=2;else a=3; assert(a == 1); }
  { int a; a=42;if(0)a=1;else if(1)a=2;else a=3; assert(a == 2); }
  { int a; a=42;if(0)a=1;else if(0)a=2;else a=3; assert(a == 3); }
  { int i; for (i = 0; i < 42; i = i + 1); assert(i == 42); }

  int r = 0;
  int i = 2;
  if (i) {
    int i = 40;
    r = i;
  }
  r = r + i;
  assert(r == 42);
}

void test_empty_macro(void) {
  #define FOO
  assert(42 == FOO 42);
  #undef FOO

  #define FOO()
  assert(42 == FOO() 42);
  #undef FOO

  #define FOO(v)
  assert(42 == FOO(1) 42);
  #undef FOO
}

int main() {
  test_assert();
  test_block_var();
  test_if_statements();
  test_empty_macro();
}
