#pragma once

#include <stdint.h>
#include <stdbool.h>


#define NumberOfPieces 7
#define GAME_WIDTH  10
#define GAME_HEIGHT 16

typedef struct {
    int x;
    int y;
    bool map[GAME_WIDTH * GAME_HEIGHT];
    int pieceIndex;
    int pieceWidth;
    int pieceHeight;
    int rotation; // 0 - 3
    bool columnLayout;
    int score;
    bool gameOver;
    int nextPieceIndex;
} GameState;

extern const uint8_t pieces[NumberOfPieces];
extern uint8_t rotations[NumberOfPieces * 3];

void initGame();
void initGameState(GameState *state);

bool isColumnLayout(int rotation);
int getWidthOfPiece(int pieceIndex, int rotation);
int getHeightOfPiece(int pieceIndex, int rotation);
uint8_t getPiece(const GameState *state);
uint8_t getSpecificPiece(int pieceIndex, int rotation);
void getPieceDrawInfo(int pieceIndex, int rotation, void (*callback)(void *ctx, int pieceIndex, int x, int y), void *ctx);

void moveLeft(GameState *state);
void moveRight(GameState *state);
void rotate(GameState *state);
void moveDown(GameState *state);
bool updateGame(GameState *state);
