#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

#include "array.h"
#include "renderer.h"


const char* get_color_code(Color color) {
    switch (color) {
        case Color_Reset:          return "\033[0m";
        case Color_Black:          return "\033[30m";
        case Color_Red:            return "\033[31m";
        case Color_Green:          return "\033[32m";
        case Color_Yellow:         return "\033[33m";
        case Color_Blue:           return "\033[34m";
        case Color_Magenta:        return "\033[35m";
        case Color_Cyan:           return "\033[36m";
        case Color_White:          return "\033[37m";
        case Color_Bright_Black:   return "\033[90m";
        case Color_Bright_Red:     return "\033[91m";
        case Color_Bright_Green:   return "\033[92m";
        case Color_Bright_Yellow:  return "\033[93m";
        case Color_Bright_Blue:    return "\033[94m";
        case Color_Bright_Magenta: return "\033[95m";
        case Color_Bright_Cyan:    return "\033[96m";
        case Color_Bright_White:   return "\033[97m";
        default:                   return "\033[0m";
    }
}

static Size getTerminalSize() {
    Size t;
    t.width = -1;
    t.height = -1;
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return t;
        }

        t.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        t.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
            return t;
        }

        t.width = w.ws_col;
        t.height = w.ws_row;
    #endif
    return t;
}

int getIndex(const Renderer *r, int x, int y) {
    return y * r->width + x;
}

void drawAll(const Renderer *r) {
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            int i = getIndex(r, x, y);
            printf("%s", get_color_code(r->map[i].foreground));
            printf("%.*s", UTF8CharacterLength, r->map[i].data);
        }

        if (y != r->height - 1) {
            printf("\n");
        }
    }
    fflush(stdout);
}

int initRenderer(Renderer *r) {
    Size size = getTerminalSize();
    r->width = size.width;
    r->height = size.height;
    if (r->width == -1 || r->height == -1) {
        perror("Error getting terminal size");
        return 1;
    }

    r->map = calloc(r->width * r->height, sizeof(Character));
    for (int i = 0; i < r->width * r->height; i++) {
        r->map[i].data[0] = ' ';
    }
    initArray(&r->changed);

    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
    drawAll(r);
    printf(TERM_HideCursor);

    return 0;
}

void deinitRenderer(Renderer *r) {
    printf("%s", get_color_code(Color_Reset));
    printf(TERM_ShowCursor);
    free(r->map);
    freeArray(&r->changed);
}

void clear() {
    printf(TERM_CursorTopLeft TERM_ClearAfterCursor);
}

static bool pointInRect(int x, int y, int w, int h) {
    return x >= 0 && x < w && y >= 0 && y < h;
}

void setChar(Renderer *r, int x, int y, const char *chrNT, Color color) {
    if (!pointInRect(x, y, r->width, r->height)) {
        fprintf(stderr, "Error: tried to setChar outside renderer box\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(chrNT) > UTF8CharacterLength) {
        printf("Error: utf character too long (max 4 bytes)\n");
        return;
    }

    Character chr = {0};
    memcpy(chr.data, chrNT, strlen(chrNT));

    int index = getIndex(r, x, y);
    if (
        strcmp(r->map[index].data, chr.data) == 0 &&
        r->map[index].foreground == color
    ) return;
    strncpy(r->map[index].data, chr.data, UTF8CharacterLength);
    r->map[index].foreground = color;

    if (!containsArray(&r->changed, index)) {
        insertArray(&r->changed, index);
    }
}

static int utf8CharLength(unsigned char c) {
    if      ((c & 0x80) == 0x00) return 1;
    else if ((c & 0xE0) == 0xC0) return 2;
    else if ((c & 0xF0) == 0xE0) return 3;
    else if ((c & 0xF8) == 0xF0) return 4;
    else return -1;
}

int setText(Renderer *r, int x, int y, char* text, Color color) {
    unsigned char *p = (unsigned char*)text;
    while (*p) {
        int len = utf8CharLength(*p);
        if (len < 0) {
            fprintf(stderr, "Error: Invalid UTF-8\n");
            return 1;
        }
        CharacterNT chr = {0};
        memcpy(chr, p, len);
        chr[len] = '\0';
        setChar(r, x, y, chr, color);

        p += len;
        x += 1;
    }

    return 0;
}

void draw(Renderer *r) {
    if (r->changed.count == 0) return;

    for (int i = 0; i < r->changed.count; i++) {
        int index = r->changed.data[i];
        int x = index % r->width;
        int y = index / r->width;
        printf(TERM_MoveCursor, y + 1, x + 1);
        printf("%s", get_color_code(r->map[index].foreground));
        printf("%.*s", UTF8CharacterLength, r->map[index].data);
    }
    r->changed.count = 0;
    fflush(stdout);
}
