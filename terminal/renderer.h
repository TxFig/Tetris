#pragma once

#include <stdbool.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

#include "array.h"


typedef struct {
    int width;
    int height;
} Size;

#define TERM_HideCursor       "\033[?25l"
#define TERM_ShowCursor       "\033[?25h"
#define TERM_CursorTopLeft    "\033[H"
#define TERM_ClearAfterCursor "\033[J"
#define TERM_MoveCursor       "\033[%d;%dH" // 1 based

typedef enum {
    Color_Reset,
    Color_Black,
    Color_Red,
    Color_Green,
    Color_Yellow,
    Color_Blue,
    Color_Magenta,
    Color_Cyan,
    Color_White,
    Color_Bright_Black,
    Color_Bright_Red,
    Color_Bright_Green,
    Color_Bright_Yellow,
    Color_Bright_Blue,
    Color_Bright_Magenta,
    Color_Bright_Cyan,
    Color_Bright_White
} Renderer_Color;

const char* get_color_code(Renderer_Color color);

#define UTF8CharacterLength 4
typedef char CharacterNT[UTF8CharacterLength + 1]; // Null terminated utf-8 character

typedef struct {
    char data[UTF8CharacterLength];
    Renderer_Color foreground;
} Character;

typedef struct {
    int width;
    int height;
    Character *map;
    IntArray changed;
} Renderer;


int getIndex(const Renderer *r, int x, int y);
void drawAll(const Renderer *r);
int initRenderer(Renderer *r);
void deinitRenderer(Renderer *r);
void clear();
void setChar(Renderer *r, int x, int y, const char *chrNT, Renderer_Color color);
int setText(Renderer *r, int x, int y, char* text, Renderer_Color color);
void draw(Renderer *r);
