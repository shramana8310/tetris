#include "input.h"
#include "state.h"
#include "tetris.h"
#include <SDL3/SDL.h>

void HandleKeyDownInSettings(GameState *gamestate, SDL_Event *event)
{
    TetrisContext *tetrisContext = gamestate->tetrisContext;
    switch (event->key.key) {
        case SDLK_RIGHT:
            IncrementStartingBlocks(tetrisContext);
            break;
        case SDLK_LEFT:
            DecrementStartingBlocks(tetrisContext);
            break;
        case SDLK_UP:
            IncrementLevel(tetrisContext);
            break;
        case SDLK_DOWN:
            DecrementLevel(tetrisContext);
            break;
        case SDLK_RETURN:
            InitTetrisContext(tetrisContext, tetrisContext->level, tetrisContext->startingBlocks);
            gamestate->mode = MODE_GAMEPLAY;
            break;
    }
}

void HandleKeyDownInGamePlay(GameState *gamestate, SDL_Event *event)
{
    TetrisContext *tetrisContext = gamestate->tetrisContext;
    if (tetrisContext->gameOver) {
        switch (event->key.key) {
            case SDLK_SPACE:
                InitTetrisContext(tetrisContext, tetrisContext->level, tetrisContext->startingBlocks);
                break;
        }
        return;
    }
    if (event->key.key == SDLK_ESCAPE) {
        TogglePause(tetrisContext);
        return;
    }
    if (tetrisContext->paused) {
        return;
    }
    switch (event->key.key) {
        case SDLK_LEFT:
            MoveLeft(tetrisContext);
            break;
        case SDLK_RIGHT:
            MoveRight(tetrisContext);
            break;
        case SDLK_DOWN:
            if (tetrisContext->moving) {
                MoveDown(tetrisContext);
            }
            break;
        case SDLK_UP:
            if (!event->key.repeat) {
                Rotate(tetrisContext);
            }
            break;
        case SDLK_SPACE:
            if (!event->key.repeat) {
                HardDrop(tetrisContext);
            }
            break;
    }
}

SDL_AppResult HandleKeyDown(GameState *gamestate, SDL_Event *event)
{
    switch (gamestate->mode) {
        case MODE_SETTINGS:
            HandleKeyDownInSettings(gamestate, event);
            break;
        case MODE_GAMEPLAY:
            HandleKeyDownInGamePlay(gamestate, event);
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

