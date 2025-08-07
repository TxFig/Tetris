#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

#include "input.h"

#ifndef _WIN32
    struct termios oldt, newt;
#endif
void initInput() {
    #ifndef _WIN32
        // Enable raw mode
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // Set non blocking
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    #endif
}

void deinitInput() {
    #ifndef _WIN32
        // Disable raw mode
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
}

void getChar(Key *key) {
    #ifdef _WIN32
        if (!_kbhit()) {
            *key = NoKey;
            return;
        }
        int ch = _getch();
        if (ch == ESC) {
            *key = KESC;
            return;
        }

        if (!_kbhit() || (ch != ExtendedCodeA && ch != ExtendedCodeB)) {
            *key = NoKey;
            return;
        }

        ch = _getch();
        switch (ch) {
            case UP   : *key = KUP   ; return;
            case LEFT : *key = KLEFT ; return;
            case RIGHT: *key = KRIGHT; return;
            case DOWN : *key = KDOWN ; return;
            default   : *key = NoKey ; return;
        }
    #else
        char buffer[3];
        int bytesRead = read(STDIN_FILENO, buffer, 3);

        if (bytesRead < 1 || buffer[0] != ESCAPE) {
            *key = NoKey;
            return;
        }

        if (bytesRead < 2 || buffer[1] != CSI) {
            *key = KESC;
            return;
        }

        switch (buffer[2]) {
            case UP   : *key = KUP   ; return;
            case LEFT : *key = KLEFT ; return;
            case RIGHT: *key = KRIGHT; return;
            case DOWN : *key = KDOWN ; return;
            default   : *key = NoKey ; return;
        }
    #endif
}

void clearInputBuffer() {
    #ifdef _WIN32
        while (_kbhit()) _getch();
    #else
        char ch;
        while (read(STDIN_FILENO, &ch, 1) > 0) {}
    #endif
}
