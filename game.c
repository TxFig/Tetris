#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"


const uint8_t pieces[NumberOfPieces] = {
    0b11110000, // I
    0b11001100, // O
    0b11100100, // T
    0b10001110, // J
    0b11101000, // L
    0b01101100, // S
    0b11000110, // Z
};

// Row Layout   : 0 1 2 3
//                4 5 6 7
// Column Layout: 0 4
//                1 5
//                2 6
//                3 7
// 0: rotated 90º  (4x2)
// 1: rotated 180º (2x4)
// 2: rotated 270º (4x2)
uint8_t rotations[NumberOfPieces * 3];

// swaps the high and low nibbles of a byte
static uint8_t swap(uint8_t byte) {
    return (byte >> 4) | (byte << 4);
}

static const uint8_t reverse4Bits[16] = {
    0b0000, 0b1000, 0b0100, 0b1100,
    0b0010, 0b1010, 0b0110, 0b1110,
    0b0001, 0b1001, 0b0101, 0b1101,
    0b0011, 0b1011, 0b0111, 0b1111
};

static uint8_t reverseNibbles(uint8_t byte) {
    uint8_t high = byte >> 4;
    uint8_t low = byte & 0x0F;
    return reverse4Bits[high] << 4 | reverse4Bits[low];
}

static uint8_t alignTopLeft(uint8_t byte) {
    if (byte == 0) return byte;
    uint8_t result = byte;

    // Handle empty rows in 4x2 and empty columns in 2x4. Example:
    // 0000  01
    // 1111  01
    //       01
    //       01
    while ((result & 0b11110000) == 0) {
        result <<= 4;
    }

    // Handle empty columns in 4x2 and empty rows in 2x4
    // 0111 00
    // 0010 00
    //      11
    //      11
    while ((result & 0b10001000) == 0) {
        uint8_t high = result & 0xF0;
        uint8_t low  = result & 0x0F;
        result = (high << 1) | (low << 1);
    }

    return result;
}

// Rotations Examples
// 0110        |  1110        |  1111        |  1100
// 1100        |  0100        |  0000        |  1100
//             |              |              |
// 10    1100  |  01    0100  |  01    0000  |  11    1100
// 11    0110  |  11    1110  |  01    1111  |  11    1100
// 01          |  01          |  01          |  00
// 00          |  00          |  01          |  00
//             |              |              |
// 0011        |  0010        |  0000        |  0011
// 0110        |  0111        |  1111        |  0011
//             |              |              |
// 00    0110  |  00    0111  |  10    1111  |  00    0011
// 10    0011  |  10    0010  |  10    0000  |  00    0011
// 11          |  11          |  10          |  11
// 01          |  10          |  10          |  11

static void generateRotations() {
    for (int i = 0; i < NumberOfPieces; i++) {
        uint8_t byte = pieces[i];

        rotations[i * 3 + 0] = swap(byte);
        rotations[i * 3 + 0] = alignTopLeft(rotations[i * 3 + 0]);
        rotations[i * 3 + 1] = reverseNibbles(swap(byte));
        rotations[i * 3 + 1] = alignTopLeft(rotations[i * 3 + 1]);
        rotations[i * 3 + 2] = reverseNibbles(byte);
        rotations[i * 3 + 2] = alignTopLeft(rotations[i * 3 + 2]);
    }
}

static int getGameIndex(int x, int y) {
    return y * GAME_WIDTH + x;
}

static void byteIndexToRelativePosition(int index, bool columnLayout, int* outX, int* outY) {
    if (columnLayout) {
        *outX = index / 4;
        *outY = index % 4;
    } else {
        *outX = index % 4;
        *outY = index / 4;
    }
}

static bool collidesWithMap(const GameState *state) {
    uint8_t piece = getPiece(state);
    for (int i = 0; i < 8; i++) {
        bool bit = (piece << i) & 0b10000000;
        if (!bit) continue;
        int x, y;
        byteIndexToRelativePosition(i, state->columnLayout, &x, &y);
        x += state->x;
        y += state->y;
        int index = getGameIndex(x, y);
        if (state->map[index]) {
            return true;
        }
    }

    return false;
}

static void newPiece(GameState *state) {
    state->pieceIndex = state->nextPieceIndex;
    state->nextPieceIndex = rand() % NumberOfPieces;
    state->rotation = 0;
    state->pieceWidth = getWidthOfPiece(state->pieceIndex, state->rotation);
    state->pieceHeight = getHeightOfPiece(state->pieceIndex, state->rotation);
    state->columnLayout = false;
    state->x = (GAME_WIDTH - state->pieceWidth) / 2;
    state->y = 0;

    if (collidesWithMap(state)) {
        state->gameOver = true;
    }
}

// Checks if active piece collides with walls or placed pieces
static bool collide(GameState *state) {
    if (state->y + state->pieceHeight >= GAME_HEIGHT) {
        state->y = GAME_HEIGHT - state->pieceHeight;
        return true;
    }

    uint8_t piece = getPiece(state);
    for (int i = 0; i < 8; i++) {
        bool bit = (piece << i) & 0b10000000;
        if (!bit) continue;
        int x, y;
        byteIndexToRelativePosition(i, state->columnLayout, &x, &y);
        x += state->x;
        y += state->y;

        int belowIndex = getGameIndex(x, y + 1);
        if (state->map[belowIndex]) {
            return true;
        }
    }

    return false;
}

void initGame() {
    srand(time(NULL));
    generateRotations();
}

void initGameState(GameState *state) {
    for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
        state->map[i] = false;
    }

    state->nextPieceIndex = rand() % NumberOfPieces;
    newPiece(state);
    state->score = 0;
    state->gameOver = false;
}

bool isColumnLayout(int rotation) {
    return rotation == 1 || rotation == 3;
}

static const int byteLeftMostBit [16] = { -1, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int byteRightMostBit[16] = { -1, 3, 2, 3, 1, 3, 2, 3, 0, 3, 2, 3, 1, 3, 2, 3 };
int getWidthOfPiece(int pieceIndex, int rotation) {
    uint8_t piece = getSpecificPiece(pieceIndex, rotation);
    uint8_t high = piece >> 4;
    uint8_t low = piece & 0x0F;

    bool columnLayout = isColumnLayout(rotation);
    if (columnLayout) {
        return getHeightOfPiece(pieceIndex, 0);
    }

    uint8_t combined = high | low;
    int right = byteRightMostBit[combined];

    if (right == -1) return 0;
    return right + 1;
}

int getHeightOfPiece(int pieceIndex, int rotation) {
    uint8_t piece = getSpecificPiece(pieceIndex, rotation);
    uint8_t high = piece >> 4;
    uint8_t low = piece & 0x0F;

    bool columnLayout = isColumnLayout(rotation);
    if (columnLayout) {
        return getWidthOfPiece(pieceIndex, 0);
    }

    if (high == 0 && low == 0) return 0;
    if (high == 0 || low == 0) return 1;
    return 2;
}

uint8_t getPiece(const GameState *state) {
    return getSpecificPiece(state->pieceIndex, state->rotation);
}

uint8_t getSpecificPiece(int pieceIndex, int rotation) {
    if (rotation == 0) {
        return pieces[pieceIndex];
    }

    return rotations[pieceIndex * 3 + (rotation - 1)];
}

void getPieceDrawInfo(int pieceIndex, int rotation, void (*callback)(void *ctx, int pieceIndex, int x, int y), void *ctx) {
    uint8_t piece = getSpecificPiece(pieceIndex, rotation);
    bool columnLayout = isColumnLayout(rotation);

    for (int i = 0; i < 8; i++) {
        bool bit = (piece << i) & 0b10000000;
        if (!bit) continue;
        int x, y;
        byteIndexToRelativePosition(i, columnLayout, &x, &y);
        callback(ctx, pieceIndex, x, y);
    }
}

void moveLeft(GameState *state) {
    if (state->x == 0) return;
    uint8_t piece = getPiece(state);

    if (!state->columnLayout) {
        uint8_t top = piece >> 4;
        uint8_t bot = piece & 0x0F;

        int topLeftMostBit = byteLeftMostBit[top];
        if (topLeftMostBit != -1) {
            int indexBefore = getGameIndex(state->x + topLeftMostBit - 1, state->y);
            if (state->map[indexBefore]) return;
        }
        int botLeftMostBit = byteLeftMostBit[bot];
        if (botLeftMostBit != -1) {
            int indexBefore = getGameIndex(state->x + botLeftMostBit - 1, state->y + 1);
            if (state->map[indexBefore]) return;
        }
    } else {
        for (int y = 0; y < 4; y++) {
            uint8_t row = ((piece >> (7 - y) & 1) << 1) | (piece >> (3 - y) & 1);
            int leftMostBit = byteLeftMostBit[row];
            if (leftMostBit != -1) {
                leftMostBit -= 2; // from 4 bits to 2
                int indexBefore = getGameIndex(state->x + leftMostBit - 1, state->y + y);
                if (state->map[indexBefore])
                    return;
            }
        }
    }

    state->x -= 1;
}

void moveRight(GameState *state) {
    if (state->x + state->pieceWidth == GAME_WIDTH) return;
    uint8_t piece = getPiece(state);

    if (!state->columnLayout) {
        uint8_t top = piece >> 4;
        uint8_t bot = piece & 0x0F;

        int topRightMostBit = byteRightMostBit[top];
        if (topRightMostBit != -1) {
            int indexBefore = getGameIndex(state->x + topRightMostBit + 1, state->y);
            if (state->map[indexBefore]) return;
        }
        int botRightMostBit = byteRightMostBit[bot];
        if (botRightMostBit != -1) {
            int indexBefore = getGameIndex(state->x + botRightMostBit + 1, state->y + 1);
            if (state->map[indexBefore]) return;
        }
    } else {
        for (int y = 0; y < 4; y++) {
            uint8_t row = ((piece >> (7 - y) & 1) << 1) | (piece >> (3 - y) & 1);
            int rightMostBit = byteRightMostBit[row];
            if (rightMostBit != -1) {
                rightMostBit -= 2; // from 4 bits to 2
                int indexBefore = getGameIndex(state->x + rightMostBit + 1, state->y + y);
                if (state->map[indexBefore]) return;
            }
        }
    }

    state->x += 1;
}

void rotate(GameState *state) {
    GameState temp = *state;
    temp.rotation = (state->rotation + 1) % 4;
    temp.columnLayout = isColumnLayout(temp.rotation);
    if (collidesWithMap(&temp)) return;

    state->rotation = temp.rotation;
    state->columnLayout = temp.columnLayout;

    int oldWidth = state->pieceWidth;
    state->pieceWidth = getWidthOfPiece(state->pieceIndex, state->rotation);
    state->pieceHeight = getHeightOfPiece(state->pieceIndex, state->rotation);

    if (state->x + state->pieceWidth >= GAME_WIDTH) {
        state->x -= state->pieceWidth - oldWidth;
    }
}

void moveDown(GameState *state) {
    if (collide(state)) return;
    state->y += 1;
}

static void checkLines(GameState *state) {
    for (int y = 0; y < GAME_HEIGHT; y++) {
        bool emptyTile = false;
        for (int x = 0; x < GAME_WIDTH; x++) {
            int index = getGameIndex(x, y);
            if (!state->map[index]) {
                emptyTile = true;
                break;
            }
        }
        if (emptyTile) continue;
        for (int x = 0; x < GAME_WIDTH; x++) {
            int index = getGameIndex(x, y);
            state->map[index] = false;
        }

        for (int y2 = y - 1; y2 >= 0; y2--) {
            for (int x = 0; x < GAME_WIDTH; x++) {
                int index = getGameIndex(x, y2);
                int below = index + GAME_WIDTH;
                state->map[below] = state->map[index];
            }
        }
        state->score++;
    }
}

/* @return Piece placed */
bool updateGame(GameState *state) {
    if (state->gameOver) return false;

    if (collide(state)) {
        uint8_t piece = getPiece(state);

        for (int i = 0; i < 8; i++) {
            bool bit = (piece << i) & 0b10000000;
            if (!bit) continue;
            int x, y;
            byteIndexToRelativePosition(i, state->columnLayout, &x, &y);
            x += state->x;
            y += state->y;
            int index = getGameIndex(x, y);
            state->map[index] = true;
        }
        checkLines(state);
        newPiece(state);

        return true;
    }

    state->y += 1;
    return false;
}
