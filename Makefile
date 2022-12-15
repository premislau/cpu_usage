cc ?= gcc


compile: main.c tests.c
	$(cc) -Wall -Wextra -pthread -o main.out main.c

test: tests.c
	$(cc) -o tests.out tests.c