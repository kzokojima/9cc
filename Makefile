CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): *.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ try/tmp*

format:
	clang-format -i -style="{BasedOnStyle: Google}" *.c *.h

.PHONY: test clean format
