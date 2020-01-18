#!/bin/bash
assert_exp() {
  expected="$1"
  input="$2"

  assert $expected "int main(){ $input }"
}

assert() {
  expected="$1"
  input="$2"
  gcc_opt="${3:-}"
  expect_error="${4:-}"

  echo "$input" > try/tmp.c
  if [[ "${USE_GCC}" = "1" ]]; then
    gcc -S -o try/tmp.s try/tmp.c
    ec=$?
  else
    ./9cc try/tmp.c
    ec=$?
  fi
  if [[ $expect_error = "1" ]]; then
    if [[ $ec = 0 ]]; then
      echo "error expected, but success"
      exit 1
    else
      echo "error expected"
      return 0
    fi
  fi
  gcc $gcc_opt -o try/tmp try/tmp.s fn.c
  ./try/tmp
  actual="$?"

  if [[ "$actual" != "$expected" ]]; then
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert_link_static() {
  assert "$1" "$2" "-static"
}

assert_output() {
  local ret=$(assert 0 "$2")
  if [[ "$ret" != "$1" ]]; then
    echo "$1 expected, but got $ret"
    exit 1
  fi
}

assert_error() {
  assert "" "$1" "" 1
}

mkdir -p try




# exit

assert_exp 0 ""
assert_exp 0 "return 0;"
assert_exp 42 "return 42;"
assert_exp 21 "return 5+20-4;"
assert_exp 41 "return  12 + 34 - 5 ;"
assert_exp 47 "return 5+6*7;"
assert_exp 15 "return 5*(9-6);"
assert_exp 4 "return (3+5)/2;"
assert_exp 10 "return -10+20;"
assert_exp 1 "return 1==1;"
assert_exp 0 "return 1!=1;"
assert_exp 1 "return 1<2;"
assert_exp 0 "return 1<1;"
assert_exp 1 "return 1<=2;"
assert_exp 0 "return 1<=0;"
assert_exp 1 "return 2>1;"
assert_exp 0 "return 1>1;"
assert_exp 1 "return 2>=1;"
assert_exp 0 "return 0>=1;"
assert_exp 1 "return 1+2*3==3*2+1;"
assert_exp 1 "return 1+2*3!=(1+2)*3;"
assert_exp 42 "int a; a=42;return a;"
assert_exp 63 "int a; int b; a=42;b=5+20-4;return a+b;"
assert_exp 6 "int foo; int bar; foo = 1;bar = 2 + 3;return foo + bar;"
assert_exp 6 "int _foo; int bar0; _foo = 1;bar0 = 2 + 3;return _foo + bar0;"
assert_exp 27 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z;"
assert_exp 253 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u;"
assert_exp 250 "int a; a=1000;return a/4;"
assert_exp 250 "int a; int b; a=1000;b=1000;return (a+b)/8;"
# overflow
assert_exp 20 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v;"
assert 0 'int main() { return (add2(3000000000, 3000000000) - 3000000000) == 3000000000; }'
assert 1 'int main() { return (add2(1000000000, 1000000000) - 1000000000) == 1000000000; }'
assert_exp 42 "int a; a=42;return a;0;"

# if
assert_exp 1  "int a; a=42;if(1)a=1;return a;"
assert_exp 42 "int a; a=42;if(0)a=1;return a;"
assert_exp 1  "int a; a=42;if(1)a=1;else a=2;return a;"
assert_exp 2  "int a; a=42;if(0)a=1;else a=2;return a;"
assert_exp 2  "int a; int b; a=42;if(1)a=1;else a=2;if(1)b=1;else b=2;return a+b;"
assert_exp 3  "int a; int b; a=42;if(1)a=1;else a=2;if(0)b=1;else b=2;return a+b;"
assert_exp 1  "int a; a=42;if(1)a=1;else if(1)a=2;else a=3;return a;"
assert_exp 2  "int a; a=42;if(0)a=1;else if(1)a=2;else a=3;return a;"
assert_exp 3  "int a; a=42;if(0)a=1;else if(0)a=2;else a=3;return a;"

# while
assert_exp 128  "int a; a=2; while (a<100) a=a*2; return a;"

# for
assert_exp 55  "int n; int i; n = 0; for (i = 0; i <= 10; i = i + 1) n = n + i; return n;"

# block
assert_exp 42 "{ return 42; }"
assert_exp 42 "{ int a; a=42; return a; }"
assert_exp 42 "{ int a; int b; a = 2; { b = 40; { return a+b; }}}"
assert_exp 42 "{ int a; int b; int c; a = 2; { b = 40; {c = 0;} { return a+b+c; }}}"
assert_exp 3  "int a; int b; a=0; b=0; if (1) { a=1; b=2; } else { a=2; b=4; } return a+b;"
assert_exp 6  "int a; int b; a=0; b=0; if (0) { a=1; b=2; } else { a=2; b=4; } return a+b;"
assert_exp 40 "int a; int b; a=2; b=3; while (a<10) { a=a*2; b=b*2; } return a+b;"
assert_exp 65 "int a; int b; int n; int i; n = 0; for (i = 1; i <= 10; i = i + 1) { n = n + i; a = i; } return n+a;"

# function
assert_exp 0 "foo(); return 0;"
assert_exp 42 "int a; a=foo(); return a;"
assert_exp 42 "int a; a=add2(2, 40); return a;"
assert_exp 42 "int a; a=add2(1 + 1, 40); return a;"
assert_exp 42 "int a; a=add2(1 + 1, 2 * 20); return a;"
assert_exp 21 "int a; a=add6(1, 2, 3, 4, 5, 6); return a;"
assert_exp 42 "int a; a=add2(2, add2(20, 20)); return a;"
assert_exp 42 "int a; a=add2(2, add2(add2(10, 10), 20)); return a;"
assert_exp 42 "int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); return a;"

assert 42 "int main() { return fn(); } int fn() { return 42; }"
assert 42 "int main() { return fn(); } int fn() { int a; a=42; return a; }"
assert 42 "int main() { return fn(); } int fn() { int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); return a; }"
assert 42 "int main() { return fn(42); } int fn(int x) { return x; }"
assert 0 'int main() {print_int(1);fn(1, 2);} int fn(int x, int y) {print_int(y);if (x + y <= 100)fn(y, x + y);}'
assert_output 1 'int main() {print_int(1);}'

# pointer
assert 42 'int main() { int i; int *p; i = 42; p = &i; return *p; }'
assert 42 'int main() { int i; int *p; p = &i; *p = 42; return i; }'
assert 42 'int main() { int i; int *p; p = &i; fn(p); return i; } int fn (int *p) { *p = 42; }'

# sizeof
assert 4 'int main() { int i; return sizeof(i); }'
assert 4 'int main() { int i; return sizeof(i + 1); }'
assert 8 'int main() { int *p; return sizeof(p); }'
assert 8 'int main() { int *p; return sizeof(p + 1); }'
assert 4 'int main() { int *p; return sizeof(*p); }'
assert 4 'int main() { return sizeof(1); }'
assert 4 'int main() { return sizeof(1 + 1); }'
assert 8 'int main() { return sizeof(sizeof(1)); }'

# array
assert 3 'int main() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  return *p + *(p + 1);
}'
assert 40 'int main() { int a[10]; return sizeof(a); }'
assert 8 "int main() { int a[10]; return fn(a); } int fn(int a[10]) { return sizeof(a); }"
assert 2 "int main() { int a[10]; *(a + 1) = 2; return fn(a); } int fn(int a[10]) { return *(a + 1); }"
assert 42 "int main() { int a[10]; a[3] = 42; return a[3]; }"

# global variable
assert 42 "int g_i; int main() { g_i = 42; return g_i; }"
assert 42 "int g_i; int main() { g_i = 42; return fn(g_i); } int fn(int a) { return a; }"
assert 42 "int g_i; int main() { int *p; p = &g_i; *p = 42; return g_i; }"
assert 4 "int g_i; int main() { return sizeof(g_i); }"
assert 42 "int *g_p; int main() { int i; g_p = &i; *g_p = 42; return i; }"
assert 42 "int *g_p; int main() { int i; g_p = &i; i = 42; return *g_p; }"
assert 42 "int g_a[10]; int main() { g_a[10] = 42; return g_a[10]; }"

# char
assert 42 'int main() { int i; char c; i = 42; c = 1; return i; }'
assert 42 'int main() { char *p; char c; p = &c; c = 42; return *p; }'
assert 42 'int main() { char *p; char c; p = &c; *p = 42; return c; }'
assert 1 'int main() { char c; return sizeof(c); }'
assert 4 'int main() { char c; return sizeof(c + 1); }'
assert 8 'int main() { char *p; return sizeof(p); }'
assert 8 'int main() { char *p; return sizeof(p + 1); }'
assert 1 'int main() { char *p; return sizeof(*p); }'
assert 2 "int main() { char a[2]; *(a + 1) = 2; return fn(a); } int fn(char a[2]) { return *(a + 1); }"

# string
assert 104 'int main() { char *p; p = "hello, world"; return *p; }'
assert 101 'int main() { char *p; p = "hello, world"; return *(p + 1); }'
assert_link_static 12 'int main() { return printf("hello, world"); }'
assert_link_static 12 'int main() { char *p; p = "hello, world"; return printf(p); }'

# comment
assert 42 'int main() {
  // ...
  return 42;
}'
assert 42 'int main() {
  /* ... */
  return 42;
}'

# initialization
assert 42 'int main() { int i = 42; return i; }'
assert 42 'int main() { int i = 40 + 2; return i; }'
assert 42 'int main() { int i = add2(40, 2); return i; }'
assert 104 'int main() { char *p = "hello, world"; return *p; }'
assert 42 'int main() { int a[] = { 1,2,3,42 }; return *(a + 3); }'
assert 104 'int main() { char a[] = "hello, world"; return *a; }'
assert 42 'int i = 42; int main() { return i; }'
assert 42 'int a[] = { 1,2,3,42 }; int main() { return *(a + 3); }'
assert 104 'char a[] = "hello, world"; int main() { return *a; }'
assert 104 'char *p = "hello, world"; int main() { return *p; }'
assert 42 'int i = 42; int *p = &i; int main() { return *p; }'

# 文字リテラル
assert 104 "int main() { char c = 'h'; return c; }"
assert 104 "char c = 'h'; int main() { return c; }"
assert 4 "int main() { return sizeof('h'); }"

# 構造体
assert 42 '
struct foo {
  int i;
};
int main() {
  struct foo val;
  val.i = 42;
  return val.i;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
int main() {
  struct foo val;
  val.i = 40;
  val.j = 2;
  print_int(val.i);
  print_int(val.j);
  return val.i + val.j;
}'
assert 42 '
struct foo {
  int i;
  int *p;
};
int main() {
  struct foo val;
  val.i = 42;
  val.p = &val.i;
  return *val.p;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
int main() {
  struct foo val;
  struct foo *p = &val;
  val.i = 40;
  p->j = 2;
  return p->i + p->j;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
int main() {
  struct foo val;
  int *p = &val.j;
  val.i = 40;
  *p = 2;
  return val.i + *p;
}'
# 構造体(グローバル変数)
assert 42 '
struct foo {
  int i;
};
struct foo val;
int main() {
  val.i = 42;
  return val.i;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
struct foo val;
int main() {
  val.i = 40;
  val.j = 2;
  print_int(val.i);
  print_int(val.j);
  return val.i + val.j;
}'
assert 42 '
struct foo {
  int i;
  int *p;
};
struct foo val;
int main() {
  val.i = 42;
  val.p = &val.i;
  return *val.p;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
struct foo val;
struct foo *p = &val;
int main() {
  val.i = 40;
  p->j = 2;
  return p->i + p->j;
}'
assert 42 '
struct foo {
  int i;
  int j;
};
struct foo val;
int *p = &val.j;
int main() {
  val.i = 40;
  *p = 2;
  return val.i + *p;
}'
# 構造体引数
assert 42 '
struct foo {
  int i;
};
int fn(struct foo *val) {
  val->i = val->i + 2;
}
int main() {
  struct foo val;
  val.i = 40;
  fn(&val);
  return val.i;
}'
# 構造体typedef
assert 42 '
typedef struct foo foo;
struct foo {
  int i;
};
int fn(foo *val) {
  val->i = val->i + 2;
}
int main() {
  foo val;
  val.i = 40;
  fn(&val);
  return val.i;
}'
# 構造体(リスト)
assert 42 '
typedef struct foo foo;
struct foo {
  int i;
  foo *next;
};
int main() {
  foo val1;
  foo val2;
  val1.i = 40;
  val1.next = &val2;
  val1.next->i = 2;
  return val1.i + val2.i;
}'
assert 42 '
typedef struct foo foo;
struct foo {
  int i;
  foo *next;
};
int main() {
  foo val1;
  foo val2;
  val1.i = 40;
  val1.next = &val2;
  val2.next = &val1;
  val1.next->next->next->i = 2;
  return val1.i + val2.i;
}'
# 列挙
assert_error '
enum { A, A };
int main() {
  return A;
}'
assert 1 '
enum { kTypeInt, kTypePtr, kTypeArray, kTypeChar, kTypeStruct };
int main() {
  return kTypePtr;
}'
assert 1 '
typedef enum {
  kNodeAdd,
  kNodeSub,
  kNodeMul,
  kNodeDiv,
} NodeKind;
int main() {
  NodeKind kind = kNodeSub;
  return kind;
}'

echo OK
