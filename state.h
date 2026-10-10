#pragma once
#include "tetris.h"
#include "render.h"

typedef enum {
    MODE_SETTINGS,
    MODE_GAMEPLAY,
} GameMode;

typedef struct GameState {
    TetrisContext *tetrisContext;
    RenderContext *renderContext;
    GameMode mode;
} GameState;
