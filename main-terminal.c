#include <time.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "terminal/renderer.h"
#include "terminal/input.h"


typedef struct {
    clock_t start;
    clock_t end;
} Clock;
bool clockTick(Clock *c, int limit) {
    c->end = clock();
    double elapsed_ms = (double)(c->end - c->start) * 1000 / CLOCKS_PER_SEC;
    if (elapsed_ms >= limit) {
        c->start = c->end;
        return true;
    }
    return false;
}

// https://en.wikipedia.org/wiki/List_of_Unicode_characters#Box_Drawing
const CharacterNT C_Hash = "▒"; // 	█ ▓ ▒ ░
const CharacterNT C_Space = " ";
const CharacterNT C_Pipe = "│";
const CharacterNT C_Dash = "─";
const CharacterNT C_BoxTL = "┌";
const CharacterNT C_BoxTR = "┐";
const CharacterNT C_BoxBL = "└";
const CharacterNT C_BoxBR = "┘";
const Color BoxColor = Color_White;
const Color PieceColor[NumberOfPieces] = {
    Color_Red,
    Color_Green,
    Color_Yellow,
    Color_Blue,
    Color_Magenta,
    Color_Cyan,
    Color_White
};

typedef struct {
    Renderer *r;
    int offx;
    int offy;
} DrawPieceContext;

void drawPieceCallback(void *vCtx, int pieceIndex, int x, int y) {
    DrawPieceContext *ctx = (DrawPieceContext*)vCtx;
    setChar(ctx->r,
        ctx->offx + x,
        ctx->offy + y,
        C_Hash,
        PieceColor[pieceIndex]
    );
}

void drawPiece(Renderer *r, int offx, int offy, int pieceIndex, int rotation) {
    DrawPieceContext ctx = {
        .r = r,
        .offx = offx,
        .offy = offy
    };
    getPieceDrawInfo(pieceIndex, rotation, &drawPieceCallback, &ctx);
}

void drawBox(Renderer *r, int offx, int offy, int width, int height) {
    for (int y = 0; y < height; y++) {
        setChar(r, offx        , offy + y, C_Pipe, BoxColor);
        setChar(r, offx + width, offy + y, C_Pipe, BoxColor);
    }
    for (int x = 0; x < width; x++) {
        setChar(r, offx + x, offy         , C_Dash, BoxColor);
        setChar(r, offx + x, offy + height, C_Dash, BoxColor);
    }

    setChar(r, offx        , offy         , C_BoxTL, BoxColor);
    setChar(r, offx + width, offy         , C_BoxTR, BoxColor);
    setChar(r, offx        , offy + height, C_BoxBL, BoxColor);
    setChar(r, offx + width, offy + height, C_BoxBR, BoxColor);
}

void drawGame(Renderer *r, GameState *state) {
    int gameX = (r->width - GAME_WIDTH) / 2;
    int gameY = (r->height - GAME_HEIGHT) / 2;

    // Game Map
    for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
        int x = i % GAME_WIDTH;
        int y = i / GAME_WIDTH;
        setChar(r,
            gameX + x,
            gameY + y,
            state->map[i] ? C_Hash : C_Space,
            Color_Bright_Black
        );
    }

    // Active piece
    drawPiece(r, gameX + state->x, gameY + state->y, state->pieceIndex, state->rotation);

    // Game Box
    drawBox(r, gameX - 1, gameY - 1, GAME_WIDTH + 1, GAME_HEIGHT + 1);

    int nextPieceX = gameX + GAME_WIDTH + 3;
    // Clear next piece area
    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 8; x++) {
            setChar(r, nextPieceX - 1 + x, gameY - 1 + y, " ", Color_Reset);
        }
    }

    // New Piece Box
    drawBox(r,
        nextPieceX - 1,
        gameY - 1,
        getWidthOfPiece(state->nextPieceIndex, 0) + 3,
        getHeightOfPiece(state->nextPieceIndex, 0) + 3
    );

    // Next piece
    drawPiece(r, nextPieceX + 1, gameY + 1, state->nextPieceIndex, 0);
}

#define LoopDelay    50
#define UpdateDelay  1000
#define DrawAllDelay 1000
int main() {
    Renderer r;
    if (initRenderer(&r) != 0) {
        return 1;
    };
    initInput();
    initGame();

    GameState state;
    initGameState(&state);

    Clock loopClock;
    loopClock.start = clock();
    Clock updateClock;
    updateClock.start = clock();
    // Clock drawClock;
    // drawClock.start = clock();

    setText(&r, 0, 0, "Score: ", Color_White);
    char scoreBuffer[12];

    Key chr;
    while (true) {
        if (!clockTick(&loopClock, LoopDelay)) continue;
        getChar(&chr);
        if (chr == KESC) break;
        switch (chr) {
            case KLEFT : moveLeft(&state); break;
            case KRIGHT: moveRight(&state); break;
            case KUP   : rotate(&state); break;
            case KDOWN : moveDown(&state); break;
            default: break;
        }

        if (clockTick(&updateClock, UpdateDelay)) {
            if (updateGame(&state)) {
                clearInputBuffer();
            }
        }

        // if (clockTick(&drawClock, DrawAllDelay)) {
        //     clear();
        //     drawAll(&r);
        // }

        sprintf(scoreBuffer, "%d", state.score);
        setText(&r, 7, 0, scoreBuffer, Color_Bright_White);

        if (state.gameOver) {
            setText(&r, 0, 1, "Game Over", Color_Bright_Red);
        }

        drawGame(&r, &state);
        draw(&r);
    }
    clear();

    deinitInput();
    deinitRenderer(&r);
    return 0;
}
