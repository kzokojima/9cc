/*
test.c
*/
#include <stdio.h>
#include <stdlib.h>

int assert_count = 0;
#define assert(expr) \
do { \
  if (!(expr)) { \
    printf("Assertion `"); \
    printf(#expr); \
    printf("' failed.\n"); \
    fflush(0); \
    abort(); \
  } \
  assert_count = assert_count + 1; \
} while (0)

void test_assert(void) {
  assert(42 == 42);
}

void test_basic(void) {
  assert(21 == 5+20-4);
  assert(41 == 12+34-5);
  assert(47 == 5+6*7);
  assert(15 == 5*(9-6));
  assert(4 == (3+5)/2);
  assert(10 == -10+20);
  assert(1 == 1==1);
  assert(0 == 1!=1);
  assert(1 == 1<2);
  assert(0 == 1<1);
  assert(1 == 1<=2);
  assert(0 == 1<=0);
  assert(1 == 2>1);
  assert(0 == 1>1);
  assert(1 == 2>=1);
  assert(0 == 0>=1);
  assert(1 == (1+2*3==3*2+1));
  assert(1 == (1+2*3!=(1+2)*3));
  assert(1 == !0);
  assert(0 == !1);
  assert(0 == !2);
  assert(0 == !!0);
  assert(1 == !!1);
  assert(1 == !!2);

  { int a; a=42; assert(42 == a); }
  { int a; int b; a=42;b=5+20-4; assert(63 == a+b); }
  { int foo; int bar; foo = 1;bar = 2 + 3; assert(6 == foo + bar); }
  { int _foo; int bar0; _foo = 1;bar0 = 2 + 3; assert(6 == _foo + bar0); }
  { int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;
    assert(27 == _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z); }
  { int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;
    assert(253 == _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u); }
  { int a; a=1000; assert(250 == a/4); }
  { int a; int b; a=1000;b=1000; assert(250 == (a+b)/8); }
  { int i1 = -12;int i2 = 4; assert(-3 == i1 / i2); }

  // overflow
  assert((add2(3000000000, 3000000000) - 3000000000) != 3000000000);
  assert((add2(1000000000, 1000000000) - 1000000000) == 1000000000);

  // assignment operators
  {
    int i = 1;
    i += 2;
    assert(3 == i);
  }
  {
    int i = 1;
    i -= 2;
    assert(-1 == i);
  }
  {
    int i = 1;
    i *= 1 + 1;
    assert(2 == i);
  }
  {
    int i = 1;
    i /= 1 + 1;
    assert(0 == i);
  }
}

int while_1 () {
  int n;
  int i = 1;
  int j = 1;
  n = 0;
  while (1) {
    while (1) {
      if (j == 5) {
        n = n + j;
        break;
      }
      j = j + 1;
    }
    if (i == 5) {
      n = n + i;
      break;
    }
    i = i + 1;
  }
  return n;
}

void test_while(void) {
  {
    int a; a=2; while (a<100) a=a*2;
    assert(128 == a);
  }
  {
    int n;
    int i = 1;
    n = 0;
    while (1) {
      n = n + i;
      if (i == 5) {
        break;
      }
      i = i + 1;
    }
    assert(15 == n);
  }
  {
    int n = 0;
    n = n + while_1();
    n = n + while_1();
    assert(60 == n);
  }
}

void test_do_while(void) {
  int max = 42;
  int i = 0;
  do {
    i = i + 1;
  } while (i < max);
  assert(42 == i);
}

int for_1(void) {
  int n;
  int i;
  int j;
  n = 0;
  for (i = 1; i <= 10; i = i + 1) {
    for (j = 1; j <= 10; j = j + 1) {
      if (j == 5) {
        n = n + j;
        break;
      }
    }
    if (i == 5) {
      n = n + i;
      break;
    }
  }
  return n;
}
void test_for(void) {
  {
    int n; int i; n = 0; for (i = 0; i <= 10; i = i + 1) n = n + i;
    assert(55 == n);
  }
  {
    int n;
    int i;
    n = 0;
    for (i = 1; i <= 10; i = i + 1) {
      n = n + i;
      if (i == 5) {
        break;
      }
    }
    assert(15 == n);
  }
  {
    int n = 0;
    n = n + for_1();
    n = n + for_1();
    assert(60 == n);
  }
}

void test_block(void) {
  { ; ; assert(42 == 42); }
  { int a; a=42; assert(42 == a); }
  { int a; int b; a = 2; { b = 40; { assert(42 == a+b); }}}
  { int a; int b; int c; a = 2; { b = 40; {c = 0;} { assert(42 == a+b+c); }}}
  { int a; int b; a=0; b=0; if (1) { a=1; b=2; } else { a=2; b=4; } assert(3 == a+b); }
  { int a; int b; a=0; b=0; if (0) { a=1; b=2; } else { a=2; b=4; } assert(6 == a+b); }
  { int a; int b; a=2; b=3; while (a<10) { a=a*2; b=b*2; } assert(40 == a+b); }
  { int a; int b; int n; int i; n = 0; for (i = 1; i <= 10; i = i + 1) { n = n + i; a = i; } assert(65 == n+a); }
}

int fn_1() { return 42; }
int fn_2() { int a; a=42; return a; }
int fn_3() { int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); return a; }
int fn_4(int x) { return x; }
int fn_5(int x, int y) {if (x + y <= 100) return fn_5(y, x + y); return x; }
void test_function(void) {
  { foo(); assert(0 == 0); }
  { int a; a=foo(); assert(42 == a); }
  { int a; a=add2(2, 40); assert(42 == a); }
  { int a; a=add2(1 + 1, 40); assert(42 == a); }
  { int a; a=add2(1 + 1, 2 * 20); assert(42 == a); }
  { int a; a=add6(1, 2, 3, 4, 5, 6); assert(21 == a); }
  { int a; a=add2(2, add2(20, 20)); assert(42 == a); }
  { int a; a=add2(2, add2(add2(10, 10), 20)); assert(42 == a); }
  { int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); assert(42 == a); }

  assert(42 == fn_1());
  assert(42 == fn_2());
  assert(42 == fn_3());
  assert(42 == fn_4(42));
  assert(55 == fn_5(1, 2));
}

int *g_p;
void pointer_1 (int *p) { *p = 42; }
void test_pointer(void) {
  // int
  { int i; int *p; i = 42; p = &i; assert(42 == *p); }
  { int i; int *p; p = &i; *p = 42; assert(42 == i); }
  { int i; int *p; p = &i; pointer_1(p); assert(42 == i); }

  // short
  { short s; short *ps; s = 42; ps = &s; assert(42 == *ps); }
  { short s; short *ps; ps = &s; *ps = 42; assert(42 == s); }

  // global
  { int i; g_p = &i; *g_p = 42; assert(42 == i); }
  { int i; g_p = &i; i = 42; assert(42 == *g_p); }
}

void test_sizeof(void) {
  int i;
  int *p;
  assert(4 == sizeof(i));
  assert(4 == sizeof(i + 1));
  assert(8 == sizeof(p));
  assert(8 == sizeof(p + 1));
  assert(4 == sizeof(*p));
  assert(4 == sizeof(1));
  assert(4 == sizeof(1 + 1));
  assert(8 == sizeof(sizeof(1)));
}

int array_1(int a[10]) { return sizeof(a); }
int array_2(int a[10]) { return *(a + 1); }
void test_array(void) {
  {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    assert(3 == *p + *(p + 1));
  }
  { int a[10]; assert(40 == sizeof(a)); }
  { int a[10]; assert(8 == array_1(a)); }
  { int a[10]; *(a + 1) = 2; assert(2 == array_2(a)); }
  { int a[10]; a[3] = 42; assert(42 == a[3]); }
}

int g_i;
int g_a[10];
int g_i2 = 21;
int global_variable(int a) { return a; }
void test_global_variable(void) {
  { g_i = 42; assert(42 == g_i); }
  { g_i = 42; assert(42 == global_variable(g_i)); }
  { int *p; p = &g_i; *p = 42; assert(42 == g_i); }
  { assert(4 == sizeof(g_i)); }
  { g_a[10] = 42; assert(42 == g_a[10]); }
  { g_i2 = g_i2 + 21; assert(42 == g_i2); }
}

int char1(char a[2]) { return *(a + 1); }
void test_char(void) {
  { int i; char c; i = 42; c = 1; assert(42 == i); }
  { char *p; char c; p = &c; c = 42; assert(42 == *p); }
  { char *p; char c; p = &c; *p = 42; assert(42 == c); }
  { char c; assert(1 == sizeof(c)); }
  { char c; assert(4 == sizeof(c + 1)); }
  { char *p; assert(8 == sizeof(p)); }
  { char *p; assert(8 == sizeof(p + 1)); }
  { char *p; assert(1 == sizeof(*p)); }
  { char a[2]; *(a + 1) = 2; assert(2 == char1(a)); }
}

void test_string(void) {
  { char *p; p = "hello, world"; assert(104 == *p); }
  { char *p; p = "hello, world"; assert(101 == *(p + 1)); }
  { assert(12 == printf("hello, world")); }
  { char *p; p = "hello, world"; assert(12 == printf(p)); }
  assert(15 == strlen("\"hello, world\\\""));
}

int initialization_i = 42;
int initialization_a[] = { 1,2,3,42 };
char initialization_a2[] = "hello, world";
char *initialization_p = "hello, world";
int initialization_i2 = 42; int *initialization_p2 = &initialization_i2;
void test_initialization(void) {
  { int i = 42; assert(42 == i); }
  { int i = 40 + 2; assert(42 == i); }
  { int i = add2(40, 2); assert(42 == i); }
  { char *p = "hello, world"; assert(104 == *p); }
  { int a[] = { 1,2,3,42 }; assert(42 == *(a + 3)); }
  { char a[] = "hello, world"; assert(104 == *a); }
  assert(42 == initialization_i);
  assert(42 == *(initialization_a + 3));
  assert(104 == *initialization_a2);
  assert(104 == *initialization_p);
  assert(42 == *initialization_p2);
  {
    int i = 1000000000; // 1 billion
    assert(1000000000 == i);
  }
  {
    int a[] = { 1000000000 }; // 1 billion
    assert(1000000000 == a[0]);
  }
}

char char_literal_c = 'h';
void test_char_literal(void) {
  { char c = 'h'; assert(104 == c); }
  { assert(104 == char_literal_c); }
  { assert(4 == sizeof('h')); }
}

void test_unsigned(void) {
  {
    unsigned u1 = 4000000000; // 4 billion
    assert(4000000000 == u1);
  }
  {
    unsigned u1 = 2000000000; // 2 billion
    unsigned u2 = 2000000000; // 2 billion
    assert(4000000000 == u1 + u2);
  }
  {
    unsigned u1 = 2000000000; // 2 billion
    unsigned u2 = 2000000000; // 2 billion
    assert(0 == u1 - u2);
  }
  {
    unsigned u1 = 2000000000; // 2 billion
    assert(4000000000 == u1 * 2);
  }
  {
    unsigned u1 = 3000000000; // 3 billion
    assert(1000000000 == u1 / 3);
  }
}

void test_long_long(void) {
  {
    long long ll = 9223372036854775807; // 0x7FFFFFFFFFFFFFFF
    assert(9223372036854775807 == ll);
  }
  {
    long long ll = -9223372036854775806; // 0xFFFFFFFFFFFFFFFF
    assert(-9223372036854775806 == ll);
  }
  {
    unsigned long long ull = 18446744073709551615; // 0xFFFFFFFFFFFFFFFF
    assert(18446744073709551615 == ull);
  }
  {
    unsigned long long ull1 = 9223372036854775807;
    unsigned long long ull2 = 9223372036854775807;
    assert(18446744073709551614 == ull1 + ull2);
  }
  {
    long long ll = 0777777777777777777777;
    assert(9223372036854775807 == ll);
  }
  {
    long long ll = 0x7FFFFFFFFFFFFFFF;
    assert(9223372036854775807 == ll);
  }
}

void test_short(void) {
  {
    short n1;
    n1 = -12;
    short n2 = 4;
    assert(-8 == n1 + n2);
  }
  {
    unsigned short n1;
    n1 = 30000;
    unsigned short n2 = 30000;
    assert(60000 == n1 + n2);
  }
  {
    unsigned short n1;
    n1 = 60000;
    assert(60000 == n1);
  }
  {
    unsigned short n1 = 60000;
    assert(60000 == n1);
  }
}

void test_binary(void) {
  assert(0 == 0b0);
  assert(0 == 0b00000000);
  assert(1 == 0b1);
  assert(1 == 0b01);
  assert(2 == 0b10);
  assert(3 == 0b11);
  assert(170 == 0b10101010);
}

void test_escape_sequences(void) {
  assert(0 == '\0');
  assert(10 == '\n');
  assert(13 == '\r');
}

void test_void(void) {
  void *vp;
  int *ip;
  int i = 1;
  vp = &i;
  ip = vp;
  assert(1 == *ip);
}

void test_switch(void) {
  switch (1) {
  case 0:
    assert(0);
    break;
  case 1:
    assert(1);
    break;
  case 2:
    assert(0);
    break;
  }
  {
    int n = 0;
    switch (n + 1) {
    case 0:
      n = 10;
      break;
    case 1:
      n = 11;
      break;
    case 2:
      n = 12;
      break;
    }
    assert(11 == n);
  }
  {
    int n = 0;
    switch (n + 1) {
    case 0:
      n = n + 1;
      n = n + 1;
    case 1:
      n = n + 1;
      n = n + 1;
    case 2:
      n = n + 1;
      n = n + 1;
    }
    assert(4 == n);
  }
  {
    int n = 0;
    switch (n + 1) {
    case 0:
      n = n + 1;
      n = n + 1;
    case 1:
      n = n + 1;
      n = n + 1;
    case 2:
      n = n + 1;
      n = n + 1;
    default:
      n = n + 1;
      n = n + 1;
    }
    assert(6 == n);
  }
}

void test_logical_or_and(void) {
  assert(0 == (0 || 0));
  assert(1 == (0 || 1));
  assert(1 == (1 || 0));
  assert(1 == (1 || 1));
  assert(1 == (0 || 2));
  assert(1 == (2 || 0));
  assert(1 == (2 || 2));
  assert(0 == (0 && 0));
  assert(0 == (0 && 1));
  assert(0 == (1 && 0));
  assert(1 == (1 && 1));
  assert(0 == (0 && 2));
  assert(0 == (2 && 0));
  assert(1 == (2 && 2));
  assert(1 == (0 == 0 || 0 == 1));
  assert(0 == (0 == 1 || 0 == 1));
  assert(1 == (0 == 0 || 0 == 0 && 0 == 0));
  assert(1 == (0 == 0 || 0 == 1 && 0 == 0));
  assert(1 == (0 == 0 || 0 == 1 && 0 == 1));
  assert(0 == (0 == 1 || 0 == 1 && 0 == 0));
  assert(0 == (0 == 1 || 0 == 1 && 0 == 1));
  assert(0 == (0 == 0 && 0 == 1));
  assert(0 == (0 == 1 && 0 == 1));
  assert(1 == (0 == 0 && 0 == 0 && 0 == 0));
  assert(0 == (0 == 0 && 0 == 1 && 0 == 0));
  assert(0 == (0 == 0 && 0 == 1 && 0 == 1));
  assert(0 == (0 == 1 && 0 == 1 && 0 == 0));
  assert(0 == (0 == 1 && 0 == 1 && 0 == 1));
}

void test_ternary_conditional(void) {
  assert(0 == (0 == 0 ? 0 : 1));
  assert(1 == (0 == 1 ? 0 : 1));
  assert(0 == (0 == 0 ? 0 == 0 ? 0 : 1 : 2));
  assert(1 == (0 == 0 ? 0 == 1 ? 0 : 1 : 2));
  assert(2 == (0 == 1 ? 0 == 0 ? 0 : 1 : 2));
  assert(2 == (0 == 1 ? 0 == 1 ? 0 : 1 : 2));
  assert(2 == (0 ? 0 : 1 ? 2 : 3));
  assert(0 == (1 ? 0 : 1 ? 2 : 3));
  assert(2 == ((0 ? 0 : 1) ? 2 : 3));
  assert(3 == ((1 ? 0 : 1) ? 2 : 3));
  assert(4 == (1 ? 2 ? 3 ? 4 : 5 : 6 : 7));
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

void test_define_macro(void) {
  #define FOO 42
  assert(42 == FOO);
  #undef FOO

  #define FOO 30 + 3
  assert(42 == FOO * 4);
  #undef FOO

  #define FOO 12+2+2
  #define BAR FOO * FOO
  assert(42 == BAR);
  #undef FOO
  #undef BAR

  #define FOO(v) v+v
  assert(42 == FOO(21));
  #undef FOO

  #define FOO(a, b) a+b
  assert(42 == FOO(40, 2));
  #undef FOO

  #define FOO(a, b) a+b
  assert(42 == FOO(3*7, 3*7));
  #undef FOO

  #define FOO(a, b) a*b
  assert(42 == FOO(12+2+2, 12+2+2));
  #undef FOO

  #define FOO(v) v+v
  {
    int v = 2;
    assert(42 == FOO(20) + v);
  }
  #undef FOO

  #define FOO(v) v+v
  #define BAR(v) FOO(v)
  {
    int v = 2;
    assert(42 == BAR(20) + v);
  }
  #undef FOO
  #undef BAR

  #define SWAP(x, y) \
  do { \
    tmp = x; \
    x = y; \
    y = tmp; \
  } while (0)
  {
    int tmp;
    int i = 0;
    int j = 42;
    SWAP(i, j);
    assert(42 == i);
  }
  #undef SWAP

  assert(1 == (1));
  assert(1 == ((1)));
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

void test_ifndef_macro(void) {
  #define FOO
  #ifndef FOO
  assert(0);
  #endif
  #undef FOO

  #ifndef FOO
  assert(1);
  #endif
}

int main() {
  test_assert();
  test_basic();
  test_while();
  test_do_while();
  test_for();
  test_block();
  test_function();
  test_pointer();
  test_sizeof();
  test_array();
  test_global_variable();
  test_char();
  test_string();
  test_initialization();
  test_char_literal();
  test_unsigned();
  test_long_long();
  test_short();
  test_binary();
  test_escape_sequences();
  test_void();
  test_switch();
  test_logical_or_and();
  test_ternary_conditional();
  test_block_var();
  test_if_statements();
  test_define_macro();
  test_empty_macro();
  test_ifndef_macro();

  printf("test.c: OK (%d assertions)\n", assert_count);
}
