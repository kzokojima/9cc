CFLAGS=-g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): *.h

test: 9cc
	./9cc tests/test.c
	gcc -o tests/test tests/test.s fn.c
	./tests/test
	./test.sh

clean:
	rm -f 9cc *.o *~ try/tmp* tests/*.s tests/*.s tests/test tests/*/*.s tests/*/test

format:
	clang-format -i -style="{BasedOnStyle: Google}" *.c *.h

bash:
	docker build -t compilerbook .
	docker run --rm -it -v $(CURDIR):/home/user/work -w /home/user/work compilerbook

.PHONY: test clean format bash
