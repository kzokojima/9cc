set -e
./9cc tests/gvar/lib.c
./9cc tests/gvar/main.c
gcc -o tests/gvar/test tests/gvar/*.s
set +e
./tests/gvar/test
if [[ "$?" != "42" ]]; then
  exit 1
fi
