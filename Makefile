CC = gcc

CFLAGS = -O3

default: linked_list.o dictionary.o parse.o compile.o translate.o allocate.o include68k.o optimize1.o optimize2.o main.o
	$(CC) $(CFLAGS) linked_list.o dictionary.o parse.o compile.o translate.o allocate.o include68k.o optimize1.o optimize2.o main.o -o TiBc

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

optimize1.o: optimize1.c optimize1.h
	$(CC) $(CFLAGS) -c optimize1.c

optimize2.o: optimize2.c optimize2.h
	$(CC) $(CFLAGS) -c optimize2.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

