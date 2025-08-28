ifeq ($(OS),Windows_NT)
    EXT := .exe
    GUI_LIBS := -lgdi32 -lopengl32 -lwinmm
else
    EXT :=
    GUI_LIBS := -lm
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

gui: main-gui.c game.o
	$(MAKE) -C gui/raylib/src

	$(CC) $(CFLAGS)                \
		-I gui/raylib/src/         \
		game.o                     \
		main-gui.c                 \
		gui/raylib/src/libraylib.a \
		$(GUI_LIBS)                \
		-o tetris-gui$(EXT)

combined: combined.c game.o
	$(MAKE) -C terminal
	$(MAKE) -C gui/raylib/src

	$(CC) $(CFLAGS)                \
		-I gui/raylib/src/         \
		game.o                     \
		terminal/renderer.o        \
		terminal/array.o           \
		terminal/input.o           \
		combined.c                 \
		gui/raylib/src/libraylib.a \
		$(GUI_LIBS)                \
		-o tetris-combined$(EXT)

game.o: game.c game.h
	$(CC) $(CFLAGS) -c game.c -o game.o
