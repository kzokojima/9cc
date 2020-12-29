set -e
./9cc tests/extern/lib.c
./9cc tests/extern/main.c
gcc -o tests/extern/test tests/extern/*.s
set +e
./tests/extern/test
if [[ "$?" != "42" ]]; then
  exit 1
fi
