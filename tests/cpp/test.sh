set -e
./9cc tests/cpp/main.c
gcc -o tests/cpp/test tests/cpp/*.s
set +e
./tests/cpp/test
if [[ "$?" != "42" ]]; then
  echo "$0: error" >&2
  exit 1
fi
