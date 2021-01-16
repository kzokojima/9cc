CFLAGS=-g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): *.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ try/tmp* tests/*.s tests/*.s tests/test tests/*/*.s tests/*/test

format:
	find . -type f -iname '*.[ch]' | xargs clang-format -i -style="{BasedOnStyle: Google}"

bash:
	docker build -t compilerbook .
	docker run --rm -it -v $(CURDIR):/home/user/work -w /home/user/work compilerbook

.PHONY: test clean format bash
