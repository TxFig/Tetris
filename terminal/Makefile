CC = gcc
CFLAGS = -Wall -g

all: renderer.o array.o input.o

renderer.o: renderer.c renderer.h
	$(CC) $(CFLAGS) -c renderer.c -o renderer.o

array.o: array.c array.h
	$(CC) $(CFLAGS) -c array.c -o array.o

input.o: input.c input.h
	$(CC) $(CFLAGS) -c input.c -o input.o
