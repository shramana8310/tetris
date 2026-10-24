#pragma once
#include <SDL3/SDL.h>

#define BOARD_COLS 10
#define BOARD_ROWS 20

#define NO_BLOCKS_PER_TETROMINO 4

#define SCORE_PER_ROW 100
#define SCORE_PER_FIX 10
#define SCORE_PER_LEVEL 1000

#define LEVEL_MAX 9
#define LEVEL_MIN 1

#define MSPT_MIN 100 

#define STARTING_BLOCKS_MAX 20
#define STARTING_BLOCKS_MIN 0

typedef enum {
    TETROMINO_I = 0,
    TETROMINO_O,
    TETROMINO_T,
    TETROMINO_L,
    TETROMINO_J,
    TETROMINO_S,
    TETROMINO_Z,
    TETROMINO_COUNT,
} TetrominoType;

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

typedef struct {
    int row;
    int col;
} Coordinate;

typedef enum {
    ORIENTATION_NORTH,
    ORIENTATION_EAST,
    ORIENTATION_SOUTH,
    ORIENTATION_WEST,
} Orientation;

typedef struct {
    TetrominoType type;
    Coordinate coordinates[NO_BLOCKS_PER_TETROMINO];
    Orientation orientation;
    BlockColor color;
} Tetromino;

typedef struct {
    BlockColor board[BOARD_ROWS][BOARD_COLS];
    Tetromino current;
    TetrominoType next;
    bool moving;
    Uint64 lastTick;
    Uint8 level;
    Uint8 startingBlocks;
    Uint16 mspt;
    Uint16 score;
    bool gameOver;
    bool paused;
} TetrisContext;

void InitTetrisContext(TetrisContext *context, Uint8 level, Uint8 startingBlocks);
void UpdateGamePlay(TetrisContext *context);
void MoveDown(TetrisContext *context);
void HardDrop(TetrisContext *context);
void MoveLeft(TetrisContext *context);
void MoveRight(TetrisContext *context);
void Rotate(TetrisContext *context);
void TogglePause(TetrisContext *context);
void IncrementStartingBlocks(TetrisContext *context);
void DecrementStartingBlocks(TetrisContext *context);
void IncrementLevel(TetrisContext *context);
void DecrementLevel(TetrisContext *context);

Tetromino CreateTetrominoByType(TetrominoType tetrominoType);
Tetromino GetLandingTetromino(TetrisContext *context);
