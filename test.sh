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

# definition error
assert_error '
typedef int t;
int main() {
  t t;
  t = 3;
  t u;  // error
}
'

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
