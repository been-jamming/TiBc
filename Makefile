CC = gcc

CFLAGS = -O3

default: linked_list.o dictionary.o parse.o compile.o translate.o
	$(CC) $(CFLAGS) linked_list.o dictionary.o parse.o compile.o translate.o -o TiBc

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
