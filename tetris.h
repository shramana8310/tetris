#pragma once
#include <SDL3/SDL.h>

#define BOARD_COLS 10
#define BOARD_ROWS 20

#define SCORE_PER_ROW 100
#define SCORE_PER_FIX 10
#define SCORE_PER_LEVEL 1000

#define LEVEL_MAX 9
#define LEVEL_MIN 1

#define MSPT_MIN 100 

#define RANDBLOCKS_MAX 20
#define RANDBLOCKS_MIN 0

typedef enum {
    TETROMINO_I = 0,
    TETROMINO_O,
    TETROMINO_T,
    TETROMINO_L,
    TETROMINO_J,
    TETROMINO_S,
    TETROMINO_Z,
    TETROMINO_COUNT,
} Tetromino;

typedef enum {
    BLOCK_CYAN,
    BLOCK_YELLOW,
    BLOCK_PURPLE,
    BLOCK_BLUE,
    BLOCK_ORANGE,
    BLOCK_GREEN,
    BLOCK_RED,
    BLOCK_GRAY,
    BLOCK_NONE,
} BlockColor;

typedef enum {
    BLOCK_EMPTY,
    BLOCK_MOVING,
    BLOCK_FIXED,
} BlockState;

typedef struct Block {
    BlockState state;
    BlockColor color;
} Block;

// TODO MAJOR REFACTORING Let's rethink how we represent the board and the current tetromino
typedef struct TetrisContext {
    Block board[BOARD_ROWS][BOARD_COLS];
    Tetromino next;
    Uint64 lasttick;
    Uint8 level;
    Uint8 randblocks;
    Uint16 mspt;
    Uint16 score;
    bool gameover;
    bool paused;
} TetrisContext;

void InitTetrisContext(TetrisContext *context, Uint8 level, Uint8 randblocks);
void UpdateGamePlay(TetrisContext *context);
void MoveDown(TetrisContext *context);
void MoveLeft(TetrisContext *context);
void MoveRight(TetrisContext *context);
void Rotate(TetrisContext *context);
void TogglePause(TetrisContext *context);
void IncrementRandBlocks(TetrisContext *context);
void DecrementRandBlocks(TetrisContext *context);
void IncrementLevel(TetrisContext *context);
void DecrementLevel(TetrisContext *context);
