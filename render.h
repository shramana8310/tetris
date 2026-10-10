#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "tetris.h"

#define BLOCK_WIDTH 30
#define BLOCK_HEIGHT BLOCK_WIDTH
#define BOARD_WIDTH (BLOCK_WIDTH * BOARD_COLS)
#define BOARD_HEIGHT (BLOCK_HEIGHT * BOARD_ROWS)
#define BOARD_WALL_WIDTH BLOCK_WIDTH
#define BOARD_WALL_HEIGHT BLOCK_HEIGHT
#define BOARD_OUTER_WIDTH (BOARD_WIDTH + (BOARD_WALL_WIDTH * 2))
#define BOARD_OUTER_HEIGHT (BOARD_HEIGHT + (BOARD_WALL_HEIGHT * 2))
#define NEXT_TETROMINO_WINDOW_COLS 5
#define NEXT_TETROMINO_WINDOW_ROWS 6
#define NEXT_TETROMINO_WINDOW_WIDTH (BLOCK_WIDTH * NEXT_TETROMINO_WINDOW_COLS)
#define NEXT_TETROMINO_WINDOW_HEIGHT (BLOCK_HEIGHT * NEXT_TETROMINO_WINDOW_ROWS)
#define PADDING_WIDTH 30
#define PADDING_HEIGHT 30

typedef struct RenderContext {
    SDL_Renderer *renderer;
    TTF_Font *font;
} RenderContext;

void DrawBoard(RenderContext *renderContext, TetrisContext *tetrisContext);
void DrawSettingsScreen(RenderContext *renderContext, TetrisContext *tetrisContext);
