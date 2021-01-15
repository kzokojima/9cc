#!/bin/bash
assert_exp() {
  expected="$1"
  input="$2"

  assert $expected "int main(){ $input }"
}

start=$(($(date +%s%N)/1000000))

set -e
./9cc tests/test.c
gcc -o tests/test tests/test.s fn.c
./tests/test
set +e

assert_count=0

assert() {
  let assert_count++
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
      echo "error expected, but success" 1>&2
      exit 1
    else
      echo "error expected" 1>&2
      return 0
    fi
  else
    if [[ $ec != 0 ]]; then
      echo "compile error" 1>&2
      exit 1
    fi
  fi
  gcc $gcc_opt -o try/tmp try/tmp.s fn.c
  ./try/tmp
  actual="$?"

  if [[ "$actual" != "$expected" ]]; then
    echo "$input => $expected expected, but got $actual" 1>&2
    exit 1
  fi
}

assert_output() {
  local ret=$(assert 0 "$2")
  if [[ "$ret" != "$1" ]]; then
    echo "$2 => $1 expected, but got $ret" 1>&2
    exit 1
  fi
}

assert_error() {
  assert "" "$1" "" 1
}

define_assert="
#define assert(expr) \
do { \
  if (!(expr)) { \
    print(\"Assertion \`\"); \
    print(#expr); \
    print(\"' failed.\\n\"); \
    fflush(0); \
    abort(); \
  } \
} while (0)
"

mkdir -p try

assert_exp 0 ""

# function
assert_output 1 'int main() {print_int(1);}'

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
assert 42 "int g_a[10]; int main() { g_a[10] = 42; return g_a[10]; }"
assert 42 "int g_i = 21; int main() { g_i = g_i + 21; return g_i; }"

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
assert 12 'int main() { return printf("hello, world"); }'
assert 12 'int main() { char *p; p = "hello, world"; return printf(p); }'
assert_output '"hello, world\"' 'int main() { print_str("\"hello, world\\\""); }'

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
assert_output 1000000000 "
int main() {
  int i = 1000000000; // 1 billion
  print_int(i);
}
"
assert_output 1000000000 "
int main() {
  int a[] = { 1000000000 }; // 1 billion
  print_int(a[0]);
}
"

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

# 符号なし整数(unsigned)
assert_output 4000000000 "
int main() {
  unsigned u1 = 4000000000; // 4 billion
  print_uint(u1);
}
"
assert_output 4000000000 "
int main() {
  unsigned u1 = 2000000000; // 2 billion
  unsigned u2 = 2000000000; // 2 billion
  print_uint(u1 + u2);
}
"
assert_output 0 "
int main() {
  unsigned u1 = 2000000000; // 2 billion
  unsigned u2 = 2000000000; // 2 billion
  print_uint(u1 - u2);
}
"
assert_output 4000000000 "
int main() {
  unsigned u1 = 2000000000; // 2 billion
  print_uint(u1 * 2);
}
"
assert_output 1000000000 "
int main() {
  unsigned u1 = 3000000000; // 3 billion
  print_uint(u1 / 3);
}
"

# long long int/unsigned long long int
assert_output 9223372036854775807 "
int main() {
  long long ll = 9223372036854775807; // 0x7FFFFFFFFFFFFFFF
  print_ll(ll);
}
"
assert_output -9223372036854775806 "
int main() {
  long long ll = -9223372036854775806; // 0xFFFFFFFFFFFFFFFF
  print_ll(ll);
}
"
assert_output 18446744073709551615 "
int main() {
  unsigned long long ull = 18446744073709551615; // 0xFFFFFFFFFFFFFFFF
  print_ull(ull);
}
"
assert_output 18446744073709551614 "
int main() {
  unsigned long long ull1 = 9223372036854775807;
  unsigned long long ull2 = 9223372036854775807;
  print_ull(ull1 + ull2);
}
"
assert_output 9223372036854775807 "
int main() {
  long long ll = 0777777777777777777777;
  print_ll(ll);
}
"
assert_output 9223372036854775807 "
int main() {
  long long ll = 0x7FFFFFFFFFFFFFFF;
  print_ll(ll);
}
"

# short/unsigned short
assert_output -8 "
int main() {
  short n1;
  n1 = -12;
  short n2 = 4;
  print_int(n1 + n2);
}
"
assert_output 60000 "
int main() {
  unsigned short n1;
  n1 = 30000;
  unsigned short n2 = 30000;
  print_int(n1 + n2);
}
"
assert_output 60000 "
int main() {
  unsigned short n1;
  n1 = 60000;
  print_int(n1);
}
"
assert_output 60000 "
int main() {
  unsigned short n1 = 60000;
  print_int(n1);
}
"

# binary
assert_exp 0 "return 0b0;"
assert_exp 0 "return 0b00000000;"
assert_exp 1 "return 0b1;"
assert_exp 1 "return 0b01;"
assert_exp 2 "return 0b10;"
assert_exp 3 "return 0b11;"
assert_exp 170 "return 0b10101010;"

# escape sequences
assert_exp 0 "return '\0';"
assert_exp 10 "return '\n';"
assert_exp 13 "return '\r';"

# void
assert_output 1 "
void fn() {
  print_int(1);
}
int main() {
  fn();
}
"
assert 0 "int main(void) {}"
assert 1 "
int main() {
  void *vp;
  int *ip;
  int i = 1;
  vp = &i;
  ip = vp;
  return *ip;
}
"

# switch
assert 11 "
int main() {
  switch (1) {
  case 0:
    return 10;
  case 1:
    return 11;
  case 2:
    return 12;
  }
}
"
assert 11 "
int main() {
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
  return n;
}
"
assert 4 "
int main() {
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
  return n;
}
"
assert 6 "
int main() {
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
  return n;
}
"

# logical OR/AND
assert_exp 0 "return 0 || 0;"
assert_exp 1 "return 0 || 1;"
assert_exp 1 "return 1 || 0;"
assert_exp 1 "return 1 || 1;"
assert_exp 1 "return 0 || 2;"
assert_exp 1 "return 2 || 0;"
assert_exp 1 "return 2 || 2;"
assert_exp 0 "return 0 && 0;"
assert_exp 0 "return 0 && 1;"
assert_exp 0 "return 1 && 0;"
assert_exp 1 "return 1 && 1;"
assert_exp 0 "return 0 && 2;"
assert_exp 0 "return 2 && 0;"
assert_exp 1 "return 2 && 2;"
assert_exp 1 "return 0 == 0 || 0 == 1;"
assert_exp 0 "return 0 == 1 || 0 == 1;"
assert_exp 1 "return 0 == 0 || 0 == 0 && 0 == 0;"
assert_exp 1 "return 0 == 0 || 0 == 1 && 0 == 0;"
assert_exp 1 "return 0 == 0 || 0 == 1 && 0 == 1;"
assert_exp 0 "return 0 == 1 || 0 == 1 && 0 == 0;"
assert_exp 0 "return 0 == 1 || 0 == 1 && 0 == 1;"
assert_exp 0 "return 0 == 0 && 0 == 1;"
assert_exp 0 "return 0 == 1 && 0 == 1;"
assert_exp 1 "return 0 == 0 && 0 == 0 && 0 == 0;"
assert_exp 0 "return 0 == 0 && 0 == 1 && 0 == 0;"
assert_exp 0 "return 0 == 0 && 0 == 1 && 0 == 1;"
assert_exp 0 "return 0 == 1 && 0 == 1 && 0 == 0;"
assert_exp 0 "return 0 == 1 && 0 == 1 && 0 == 1;"

# Ternary conditional
assert_exp 0 "return 0 == 0 ? 0 : 1;"
assert_exp 1 "return 0 == 1 ? 0 : 1;"
assert_exp 0 "return 0 == 0 ? 0 == 0 ? 0 : 1 : 2;"
assert_exp 1 "return 0 == 0 ? 0 == 1 ? 0 : 1 : 2;"
assert_exp 2 "return 0 == 1 ? 0 == 0 ? 0 : 1 : 2;"
assert_exp 2 "return 0 == 1 ? 0 == 1 ? 0 : 1 : 2;"
assert_exp 2 "return 0 ? 0 : 1 ? 2 : 3;"
assert_exp 0 "return 1 ? 0 : 1 ? 2 : 3;"
assert_exp 2 "return (0 ? 0 : 1) ? 2 : 3;"
assert_exp 3 "return (1 ? 0 : 1) ? 2 : 3;"
assert_exp 4 "return 1 ? 2 ? 3 ? 4 : 5 : 6 : 7;"

# __FILE__ / __LINE__
assert_output try/tmp.c  "
int main() {
  print_str(__FILE__);
}
"
assert 3  "
int main() {
  return __LINE__;
}
"

set -e
bash tests/plt/test.sh
bash tests/cpp/test.sh
bash tests/extern/test.sh
set +e

# マクロ
assert_error "
#define FOO(v) v+v
int main() {
  return FOO;
}
"

assert_output '0 == 1' '
#define pp(expr) print_str(#expr)
#define p(expr) pp(expr)
int main() {
  p(0 == 1);
}
'

assert 134 "
$define_assert
int main() {
  assert(0 == (1));
}
"

echo "$0: OK ($assert_count assertions)"

end=$(($(date +%s%N)/1000000))
time=$((end - start))
echo "Time: $time ms"
