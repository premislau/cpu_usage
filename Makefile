cc ?= gcc


compile: main.c
	$(cc) -Wall -Wextra -pthread -o main.out main.c

test: tests.c
	$(cc) -Wall -Wextra -o tests.out tests.c