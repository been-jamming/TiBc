CC = gcc

CFLAGS = -O3

default: linked_list.o dictionary.o parse.o compile.o translate.o allocate.o include68k.o
	$(CC) $(CFLAGS) linked_list.o dictionary.o parse.o compile.o translate.o allocate.o include68k.o -o TiBc

linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) -c linked_list.c

dictionary.o: dictionary.c dictionary.h
	$(CC) $(CFLAGS) -c dictionary.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

compile.o: compile.c compile.h
	$(CC) $(CFLAGS) -c compile.c

translate.o: translate.c translate.h
	$(CC) $(CFLAGS) -c translate.c

allocate.o: allocate.c allocate.h
	$(CC) $(CFLAGS) -c allocate.c

include68k.o: include68k.c
	$(CC) $(CFLAGS) -c include68k.c
