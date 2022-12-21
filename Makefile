cc ?= gcc
CFLAGS = -Wall -Wextra -pthread

OBJS = main.c
PROG = cpu_usage.out


$(PROG): $(OBJS)
	$(cc) $(CFLAGS) $(OBJS) -o $(PROG)


test: test.c
	$(cc) -Wall -Wextra -pthread -o test.out test.c

clean:
	rm -f *~ *.out $(PROG) core
