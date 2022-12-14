cc ?= gcc


compile: main.c tests.c
	$(cc) -o main.out main.c

test: tests.c
	$(cc) -o tests.out tests.c