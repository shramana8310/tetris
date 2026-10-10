#include "input.h"

void HandleKeyDownInSettings(GameState *gamestate, SDL_Keycode keycode)
{
    TetrisContext *tetrisContext = gamestate->tetrisContext;
    switch (keycode) {
        case SDLK_RIGHT:
            IncrementRandBlocks(tetrisContext);
            break;
        case SDLK_LEFT:
            DecrementRandBlocks(tetrisContext);
            break;
        case SDLK_UP:
            IncrementLevel(tetrisContext);
            break;
        case SDLK_DOWN:
            DecrementLevel(tetrisContext);
            break;
        case SDLK_RETURN:
            InitTetrisContext(tetrisContext, tetrisContext->level, tetrisContext->randblocks);
            gamestate->mode = MODE_GAMEPLAY;
            break;
    }
}

void HandleKeyDownInGamePlay(GameState *gamestate, SDL_Keycode keycode)
{
    TetrisContext *tetrisContext = gamestate->tetrisContext;
    if (keycode == SDLK_P) {
        TogglePause(tetrisContext);
        return;
    }
    if (tetrisContext->paused) {
        return;
    }
    switch (keycode) {
        case SDLK_I:
            gamestate->mode = MODE_SETTINGS;
            break;
        case SDLK_R:
            InitTetrisContext(tetrisContext, tetrisContext->level, tetrisContext->randblocks);
            break;
        case SDLK_LEFT:
            MoveLeft(tetrisContext);
            break;
        case SDLK_RIGHT:
            MoveRight(tetrisContext);
            break;
        case SDLK_DOWN:
            MoveDown(tetrisContext);
            break;
        case SDLK_UP:
            Rotate(tetrisContext);
            break;
    }
}

SDL_AppResult HandleKeyDown(GameState *gamestate, SDL_Keycode keycode)
{
    switch (gamestate->mode) {
        case MODE_SETTINGS:
            HandleKeyDownInSettings(gamestate, keycode);
            break;
        case MODE_GAMEPLAY:
            HandleKeyDownInGamePlay(gamestate, keycode);
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

