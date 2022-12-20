cc ?= gcc


compile: main.c
	$(cc) -Wall -Wextra -pthread -o main.out main.c

test: test.c
	$(cc) -Wall -Wextra -pthread -o test.out test.c