#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 10 "-10+20;"
try 1 "1==1;"
try 0 "1!=1;"
try 1 "1<2;"
try 0 "1<1;"
try 1 "1<=2;"
try 0 "1<=0;"
try 1 "2>1;"
try 0 "1>1;"
try 1 "2>=1;"
try 0 "0>=1;"
try 1 "1+2*3==3*2+1;"
try 1 "1+2*3!=(1+2)*3;"
try 42 "a=42;a;"
try 63 "a=42;b=5+20-4;a+b;"
try 6 "foo = 1;bar = 2 + 3;foo + bar;"
try 6 "_foo = 1;bar0 = 2 + 3;_foo + bar0;"
try 27 "_=1;a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;_+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z;"
try 253 "_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;_+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u;"
# overflow
try 20 "_=1;a=2;b=3;c=4;d=5;e=6;f=7;g=8;h=9;i=10;j=11;k=12;l=13;m=14;n=15;o=16;p=17;q=18;r=19;s=20;t=21;u=22;v=23;w=24;x=25;y=26;z=27;_+a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v;"
try 42 "a=42;return a;0;"

# if
try 1  "a=42;if(1)a=1;return a;"
try 42 "a=42;if(0)a=1;return a;"
try 1  "a=42;if(1)a=1;else a=2;return a;"
try 2  "a=42;if(0)a=1;else a=2;return a;"
try 2  "a=42;if(1)a=1;else a=2;if(1)b=1;else b=2;return a+b;"
try 3  "a=42;if(1)a=1;else a=2;if(0)b=1;else b=2;return a+b;"
try 1  "a=42;if(1)a=1;else if(1)a=2;else a=3;return a;"
try 2  "a=42;if(0)a=1;else if(1)a=2;else a=3;return a;"
try 3  "a=42;if(0)a=1;else if(0)a=2;else a=3;return a;"

# while
try 128  "a=2; while (a<100) a=a*2; return a;"

# for
try 55  "n = 0; for (i = 0; i <= 10; i = i + 1) n = n + i; return n;"

echo OK
