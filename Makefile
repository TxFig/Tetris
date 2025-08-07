ifeq ($(OS),Windows_NT)
    EXT := .exe
else
    EXT :=
endif

CC = gcc
CFLAGS = -Wall -g

terminal: main-terminal.c game.o
	$(MAKE) -C terminal

	$(CC) $(CFLAGS)         \
		game.o              \
		terminal/renderer.o \
		terminal/array.o    \
		terminal/input.o    \
		main-terminal.c     \
		-o tetris-terminal$(EXT)

game.o: game.c game.h
	$(CC) $(CFLAGS) -c game.c -o game.o
