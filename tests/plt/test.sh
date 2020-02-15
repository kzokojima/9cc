set -e
./9cc tests/plt/lib.c
./9cc tests/plt/main.c
gcc -o tests/plt/test tests/plt/*.s
set +e
./tests/plt/test
if [[ "$?" != "42" ]]; then
  exit 1
fi
