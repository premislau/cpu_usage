cc ?= gcc


compile: main.c tests.c
	$(cc) -o main.out main.c
	$(cc) -o tests.out tests.c

