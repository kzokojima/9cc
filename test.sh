#!/bin/bash
try() {
  expected="$1"
  input="$2"

  try2 $expected "int main(){ $input }"
}

try2() {
  expected="$1"
  input="$2"
  gcc_opt="${3:-}"

  echo "$input" > try/tmp.c
  if [[ "${USE_GCC}" = "1" ]]; then
    gcc -S -o try/tmp.s try/tmp.c
  else
    ./9cc try/tmp.c > try/tmp.s
  fi
  gcc $gcc_opt -o try/tmp try/tmp.s fn.c
  ./try/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try2s() {
  try2 "$1" "$2" "-static"
}

mkdir -p try

try 0 "return 0;"
try 42 "return 42;"
try 21 "return 5+20-4;"
try 41 "return  12 + 34 - 5 ;"
try 47 "return 5+6*7;"
try 15 "return 5*(9-6);"
try 4 "return (3+5)/2;"
try 10 "return -10+20;"
try 1 "return 1==1;"
try 0 "return 1!=1;"
try 1 "return 1<2;"
try 0 "return 1<1;"
try 1 "return 1<=2;"
try 0 "return 1<=0;"
try 1 "return 2>1;"
try 0 "return 1>1;"
try 1 "return 2>=1;"
try 0 "return 0>=1;"
try 1 "return 1+2*3==3*2+1;"
try 1 "return 1+2*3!=(1+2)*3;"
try 42 "int a; a=42;return a;"
try 63 "int a; int b; a=42;b=5+20-4;return a+b;"
try 6 "int foo; int bar; foo = 1;bar = 2 + 3;return foo + bar;"
try 6 "int _foo; int bar0; _foo = 1;bar0 = 2 + 3;return _foo + bar0;"
try 27 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z;"
try 253 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u;"
try 250 "int a; a=1000;return a/4;"
try 250 "int a; int b; a=1000;b=1000;return (a+b)/8;"
# overflow
try 20 "int _;int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z;_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;return _+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v;"
try2 0 'int main() { return (add2(3000000000, 3000000000) - 3000000000) == 3000000000; }'
try2 1 'int main() { return (add2(1000000000, 1000000000) - 1000000000) == 1000000000; }'
try 42 "int a; a=42;return a;0;"

# if
try 1  "int a; a=42;if(1)a=1;return a;"
try 42 "int a; a=42;if(0)a=1;return a;"
try 1  "int a; a=42;if(1)a=1;else a=2;return a;"
try 2  "int a; a=42;if(0)a=1;else a=2;return a;"
try 2  "int a; int b; a=42;if(1)a=1;else a=2;if(1)b=1;else b=2;return a+b;"
try 3  "int a; int b; a=42;if(1)a=1;else a=2;if(0)b=1;else b=2;return a+b;"
try 1  "int a; a=42;if(1)a=1;else if(1)a=2;else a=3;return a;"
try 2  "int a; a=42;if(0)a=1;else if(1)a=2;else a=3;return a;"
try 3  "int a; a=42;if(0)a=1;else if(0)a=2;else a=3;return a;"

# while
try 128  "int a; a=2; while (a<100) a=a*2; return a;"

# for
try 55  "int n; int i; n = 0; for (i = 0; i <= 10; i = i + 1) n = n + i; return n;"

# block
try 42 "{ return 42; }"
try 42 "{ int a; a=42; return a; }"
try 42 "{ int a; int b; a = 2; { b = 40; { return a+b; }}}"
try 42 "{ int a; int b; int c; a = 2; { b = 40; {c = 0;} { return a+b+c; }}}"
try 3  "int a; int b; a=0; b=0; if (1) { a=1; b=2; } else { a=2; b=4; } return a+b;"
try 6  "int a; int b; a=0; b=0; if (0) { a=1; b=2; } else { a=2; b=4; } return a+b;"
try 40 "int a; int b; a=2; b=3; while (a<10) { a=a*2; b=b*2; } return a+b;"
try 65 "int a; int b; int n; int i; n = 0; for (i = 1; i <= 10; i = i + 1) { n = n + i; a = i; } return n+a;"

# function
try 0 "foo(); return 0;"
try 42 "int a; a=foo(); return a;"
try 42 "int a; a=add2(2, 40); return a;"
try 42 "int a; a=add2(1 + 1, 40); return a;"
try 42 "int a; a=add2(1 + 1, 2 * 20); return a;"
try 21 "int a; a=add6(1, 2, 3, 4, 5, 6); return a;"
try 42 "int a; a=add2(2, add2(20, 20)); return a;"
try 42 "int a; a=add2(2, add2(add2(10, 10), 20)); return a;"
try 42 "int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); return a;"

try2 42 "int main() { return fn(); } int fn() { return 42; }"
try2 42 "int main() { return fn(); } int fn() { int a; a=42; return a; }"
try2 42 "int main() { return fn(); } int fn() { int a; a=add2(2, add2(add2(10, 10), add2(10, 10))); return a; }"
try2 42 "int main() { return fn(42); } int fn(int x) { return x; }"
try2 0 'int main() {print_int(1);fn(1, 2);} int fn(int x, int y) {print_int(y);if (x + y <= 100)fn(y, x + y);}'

# pointer
try2 42 'int main() { int i; int *p; i = 42; p = &i; return *p; }'
try2 42 'int main() { int i; int *p; p = &i; *p = 42; return i; }'
try2 42 'int main() { int i; int *p; p = &i; fn(p); return i; } int fn (int *p) { *p = 42; }'

# sizeof
try2 4 'int main() { int i; return sizeof(i); }'
try2 4 'int main() { int i; return sizeof(i + 1); }'
try2 8 'int main() { int *p; return sizeof(p); }'
try2 8 'int main() { int *p; return sizeof(p + 1); }'
try2 4 'int main() { int *p; return sizeof(*p); }'
try2 4 'int main() { return sizeof(1); }'
try2 4 'int main() { return sizeof(1 + 1); }'
try2 8 'int main() { return sizeof(sizeof(1)); }'

# array
try2 3 'int main() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  return *p + *(p + 1);
}'
try2 40 'int main() { int a[10]; return sizeof(a); }'
try2 8 "int main() { int a[10]; return fn(a); } int fn(int a[10]) { return sizeof(a); }"
try2 2 "int main() { int a[10]; *(a + 1) = 2; return fn(a); } int fn(int a[10]) { return *(a + 1); }"
try2 42 "int main() { int a[10]; a[3] = 42; return a[3]; }"

# global variable
try2 42 "int g_i; int main() { g_i = 42; return g_i; }"
try2 42 "int g_i; int main() { g_i = 42; return fn(g_i); } int fn(int a) { return a; }"
try2 42 "int g_i; int main() { int *p; p = &g_i; *p = 42; return g_i; }"
try2 4 "int g_i; int main() { return sizeof(g_i); }"
try2 42 "int *g_p; int main() { int i; g_p = &i; *g_p = 42; return i; }"
try2 42 "int *g_p; int main() { int i; g_p = &i; i = 42; return *g_p; }"
try2 42 "int g_a[10]; int main() { g_a[10] = 42; return g_a[10]; }"

# char
try2 42 'int main() { int i; char c; i = 42; c = 1; return i; }'
try2 42 'int main() { char *p; char c; p = &c; c = 42; return *p; }'
try2 42 'int main() { char *p; char c; p = &c; *p = 42; return c; }'
try2 1 'int main() { char c; return sizeof(c); }'
try2 4 'int main() { char c; return sizeof(c + 1); }'
try2 8 'int main() { char *p; return sizeof(p); }'
try2 8 'int main() { char *p; return sizeof(p + 1); }'
try2 1 'int main() { char *p; return sizeof(*p); }'
try2 2 "int main() { char a[2]; *(a + 1) = 2; return fn(a); } int fn(char a[2]) { return *(a + 1); }"

# string
try2 104 'int main() { char *p; p = "hello, world"; return *p; }'
try2 101 'int main() { char *p; p = "hello, world"; return *(p + 1); }'
try2s 12 'int main() { return printf("hello, world"); }'
try2s 12 'int main() { char *p; p = "hello, world"; return printf(p); }'

# comment
try2s 42 'int main() {
  // ...
  return 42;
}'
try2s 42 'int main() {
  /* ... */
  return 42;
}'

# initialization
try2 42 'int main() { int i = 42; return i; }'
try2 42 'int main() { int i = 40 + 2; return i; }'
try2 42 'int main() { int i = add2(40, 2); return i; }'
try2 104 'int main() { char *p = "hello, world"; return *p; }'
try2 42 'int main() { int a[] = { 1,2,3,42 }; return *(a + 3); }'
try2 104 'int main() { char a[] = "hello, world"; return *a; }'
try2 42 'int i = 42; int main() { return i; }'
try2 42 'int a[] = { 1,2,3,42 }; int main() { return *(a + 3); }'
try2 104 'char a[] = "hello, world"; int main() { return *a; }'
try2 104 'char *p = "hello, world"; int main() { return *p; }'
try2 42 'int i = 42; int *p = &i; int main() { return *p; }'


echo OK
