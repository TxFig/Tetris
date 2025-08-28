#include <stdio.h>
#include <math.h>

#include "game.h"
#include "raylib.h"


#define TileSize 20
#define WIDTH (GAME_WIDTH + 20) * TileSize
#define HEIGHT (GAME_HEIGHT + 6) * TileSize

const Color BoxColor = WHITE;
const Color PieceColor[NumberOfPieces] = {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    SKYBLUE,
    RAYWHITE
};

typedef struct {
    int offx;
    int offy;
} DrawPieceContext;

void drawPieceCallback(void *vCtx, int pieceIndex, int x, int y) {
    DrawPieceContext *ctx = (DrawPieceContext*)vCtx;
    DrawRectangle(
        ctx->offx + x * TileSize,
        ctx->offy + y * TileSize,
        TileSize, TileSize,
        PieceColor[pieceIndex]
    );
}

void drawPiece(int offx, int offy, int pieceIndex, int rotation) {
    DrawPieceContext ctx = {
        .offx = offx,
        .offy = offy
    };
    getPieceDrawInfo(pieceIndex, rotation, &drawPieceCallback, &ctx);
}


void drawGame(GameState *state) {
    int gameX = (WIDTH - GAME_WIDTH * TileSize) / 2;
    int gameY = (HEIGHT - GAME_HEIGHT * TileSize) / 2;

    // Game Box
    DrawRectangleLines(
        gameX - 2,
        gameY - 2,
        GAME_WIDTH * TileSize + 4,
        GAME_HEIGHT * TileSize + 4,
        BoxColor
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
    drawPiece(
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
        BoxColor
    );

    // Next piece
    drawPiece(nextPieceX + TileSize, gameY + TileSize, state->nextPieceIndex, 0);
}

#define KeyDelay     0.15
#define UpdateDelay  1

typedef struct {
    double limit;
    double last;
} Clock;
bool updateClock(Clock *clock) {
    double now = GetTime();
    if (now - clock->last >= clock->limit) {
        clock->last = now;
        return true;
    }
    return false;
}


int main() {
    InitWindow(WIDTH, HEIGHT, "Tetris GUI");
    SetTargetFPS(60);

    initGame();

    GameState state;
    initGameState(&state);

    Clock gameClock = { .limit = UpdateDelay, .last = GetTime() };
    Clock keyClock  = { .limit = KeyDelay   , .last = GetTime() };

    char scoreBuffer[20];
    while (!WindowShouldClose()) {
        ClearBackground(BLACK);

        if (updateClock(&keyClock)) {
            if (IsKeyDown(KEY_LEFT))  moveLeft(&state);
            if (IsKeyDown(KEY_RIGHT)) moveRight(&state);
            if (IsKeyDown(KEY_UP))    rotate(&state);
            if (IsKeyDown(KEY_DOWN))  moveDown(&state);
        }

        if (updateClock(&gameClock)) {
            updateGame(&state);
        }

        BeginDrawing();
            sprintf(scoreBuffer, "Score: %d", state.score);
            DrawText(scoreBuffer, 0, 0, 20, WHITE);

            if (state.gameOver) {
                DrawText("Game Over", 0, 20, 20, RED);
            }

            drawGame(&state);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
