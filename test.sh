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

echo OK
