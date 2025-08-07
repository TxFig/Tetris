#pragma once

void initInput();
void deinitInput();

typedef enum {
    NoKey, KUP, KLEFT, KRIGHT, KDOWN, KESC
} Key;

#ifdef _WIN32
    #define ExtendedCodeA 0
    #define ExtendedCodeB 224
    #define ESC   27
    #define UP    72
    #define LEFT  75
    #define RIGHT 77
    #define DOWN  80
#else
    #define ESCAPE 27
    #define CSI    91 // Control Sequence Introducer
    #define UP    65
    #define LEFT  68
    #define RIGHT 67
    #define DOWN  66
#endif

void getChar(Key *key);
void clearInputBuffer();
