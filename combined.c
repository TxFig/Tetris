#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "game.h"

// windows.h and raylib.h use common names
#define Rectangle WinRectangle
#define CloseWindow WinCloseWindow
#define ShowCursor WinShowCursor
#define LoadImage WinLoadImage
#define DrawText WinDrawText
#define DrawTextEx WinDrawTextEx
#define PlaySound WinPlaySound
#include "terminal/renderer.h"
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#undef PlaySound

#include "terminal/input.h"
#include "raylib.h"


#define TileSize 20
#define WIDTH (GAME_WIDTH + 20) * TileSize
#define HEIGHT (GAME_HEIGHT + 6) * TileSize

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
const Renderer_Color Terminal_BoxColor = Color_White;
const Renderer_Color Terminal_PieceColor[NumberOfPieces] = {
    Color_Red,
    Color_Green,
    Color_Yellow,
    Color_Blue,
    Color_Magenta,
    Color_Cyan,
    Color_White
};

const Color GUI_BoxColor = WHITE;
const Color GUI_PieceColor[NumberOfPieces] = {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    SKYBLUE,
    RAYWHITE
};

typedef struct {
    Renderer *r;
    int offx;
    int offy;
} Terminal_DrawPieceContext;
void drawPieceCallbackTerminal(void *vCtx, int pieceIndex, int x, int y) {
    Terminal_DrawPieceContext *ctx = (Terminal_DrawPieceContext*)vCtx;
    setChar(ctx->r,
        ctx->offx + x,
        ctx->offy + y,
        C_Hash,
        Terminal_PieceColor[pieceIndex]
    );
}

typedef struct {
    int offx;
    int offy;
} GUI_DrawPieceContext;
void drawPieceCallbackGUI(void *vCtx, int pieceIndex, int x, int y) {
    GUI_DrawPieceContext *ctx = (GUI_DrawPieceContext*)vCtx;
    DrawRectangle(
        ctx->offx + x * TileSize,
        ctx->offy + y * TileSize,
        TileSize, TileSize,
        GUI_PieceColor[pieceIndex]
    );
}

void drawPieceTerminal(Renderer *r, int offx, int offy, int pieceIndex, int rotation) {
    Terminal_DrawPieceContext ctx = {
        .r = r,
        .offx = offx,
        .offy = offy
    };
    getPieceDrawInfo(pieceIndex, rotation, &drawPieceCallbackTerminal, &ctx);
}

void drawPieceGUI(int offx, int offy, int pieceIndex, int rotation) {
    GUI_DrawPieceContext ctx = {
        .offx = offx,
        .offy = offy
    };
    getPieceDrawInfo(pieceIndex, rotation, &drawPieceCallbackGUI, &ctx);
}

void drawBox(Renderer *r, int offx, int offy, int width, int height) {
    for (int y = 0; y < height; y++) {
        setChar(r, offx        , offy + y, C_Pipe, Terminal_BoxColor);
        setChar(r, offx + width, offy + y, C_Pipe, Terminal_BoxColor);
    }
    for (int x = 0; x < width; x++) {
        setChar(r, offx + x, offy         , C_Dash, Terminal_BoxColor);
        setChar(r, offx + x, offy + height, C_Dash, Terminal_BoxColor);
    }

    setChar(r, offx        , offy         , C_BoxTL, Terminal_BoxColor);
    setChar(r, offx + width, offy         , C_BoxTR, Terminal_BoxColor);
    setChar(r, offx        , offy + height, C_BoxBL, Terminal_BoxColor);
    setChar(r, offx + width, offy + height, C_BoxBR, Terminal_BoxColor);
}

void drawGameTerminal(Renderer *r, GameState *state) {
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
    drawPieceTerminal(r, gameX + state->x, gameY + state->y, state->pieceIndex, state->rotation);

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
    drawPieceTerminal(r, nextPieceX + 1, gameY + 1, state->nextPieceIndex, 0);
}

void drawGameGUI(GameState *state) {
    int gameX = (WIDTH - GAME_WIDTH * TileSize) / 2;
    int gameY = (HEIGHT - GAME_HEIGHT * TileSize) / 2;

    // Game Box
    DrawRectangleLines(
        gameX - 2,
        gameY - 2,
        GAME_WIDTH * TileSize + 4,
        GAME_HEIGHT * TileSize + 4,
        GUI_BoxColor
    );

    // Game Map
    for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
        int x = i % GAME_WIDTH;
        int y = i / GAME_WIDTH;
        DrawRectangle(
            gameX + x * TileSize,
            gameY + y * TileSize,
            TileSize, TileSize,
            state->map[i] ? GRAY : BLANK
        );
    }

    // Active piece
    drawPieceGUI(
        gameX + state->x * TileSize,
        gameY + state->y * TileSize,
        state->pieceIndex, state->rotation
    );

    int nextPieceX = gameX + (GAME_WIDTH + 1) * TileSize;
    // New Piece Box
    DrawRectangleLines(
        nextPieceX,
        gameY,
        (getWidthOfPiece(state->nextPieceIndex, 0) + 2) * TileSize,
        (getHeightOfPiece(state->nextPieceIndex, 0) + 2) * TileSize,
        GUI_BoxColor
    );

    // Next piece
    drawPieceGUI(nextPieceX + TileSize, gameY + TileSize, state->nextPieceIndex, 0);
}


#define LoopDelay    50
#define UpdateDelay  1000
#define DrawAllDelay 1000
#define KeyDelay     0.15

int main() {
    Renderer r;
    if (initRenderer(&r) != 0) {
        return 1;
    };
    initInput();

    SetTraceLogLevel(LOG_NONE);
    InitWindow(WIDTH, HEIGHT, "Tetris GUI");
    SetTargetFPS(60);

    initGame();
    GameState state;
    initGameState(&state);

    Clock loopClock;
    loopClock.start = clock();
    Clock updateClock;
    updateClock.start = clock();
    // Clock drawClock;
    // drawClock.start = clock();

    Clock keyClock;
    keyClock.start = clock();

    setText(&r, 0, 0, "Score: ", Color_White);
    char Terminal_scoreBuffer[12];
    char GUI_scoreBuffer[20];
    Key chr;
    while (!WindowShouldClose()) {
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

        if (clockTick(&keyClock, KeyDelay)) {
            if (IsKeyDown(KEY_LEFT))  moveLeft(&state);
            if (IsKeyDown(KEY_RIGHT)) moveRight(&state);
            if (IsKeyDown(KEY_UP))    rotate(&state);
            if (IsKeyDown(KEY_DOWN))  moveDown(&state);
        }

        if (clockTick(&updateClock, UpdateDelay)) {
            if (updateGame(&state)) {
                clearInputBuffer();
            }
        }

        ClearBackground(BLACK);

        // if (clockTick(&drawClock, DrawAllDelay)) {
        //     clear();
        //     drawAll(&r);
        // }

        sprintf(Terminal_scoreBuffer, "%d", state.score);
        setText(&r, 7, 0, Terminal_scoreBuffer, Color_Bright_White);

        if (state.gameOver) {
            setText(&r, 0, 1, "Game Over", Color_Bright_Red);
        }

        drawGameTerminal(&r, &state);
        draw(&r);

        BeginDrawing();
            sprintf(GUI_scoreBuffer, "Score: %d", state.score);
            DrawText(GUI_scoreBuffer, 0, 0, 20, WHITE);

            if (state.gameOver) {
                DrawText("Game Over", 0, 20, 20, RED);
            }

            drawGameGUI(&state);
        EndDrawing();
    }
    clear();

    deinitInput();
    deinitRenderer(&r);
    CloseWindow();
    return 0;
}
