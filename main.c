#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "state.h"
#include "tetris.h"
#include "render.h"
#include "input.h"
#define WINDOW_WIDTH (BOARD_OUTER_WIDTH + NEXT_TETROMINO_WINDOW_WIDTH + (PADDING_WIDTH * 3))
#define WINDOW_HEIGHT (BOARD_OUTER_HEIGHT + (PADDING_HEIGHT * 2))
#define FONT_FILENAME "press-start-2p-latin-400-normal.ttf"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_SetAppMetadata("Tetris", "1.0.0", "com.example.tetris")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not set app metadata: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL_ttf: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Window *window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    TTF_Font *font = TTF_OpenFont(FONT_FILENAME, 14.0f);
    if (font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    TetrisContext *tetrisContext = SDL_malloc(sizeof(TetrisContext));
    if (tetrisContext == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize tetrisContext.");
        return SDL_APP_FAILURE;
    }
    InitTetrisContext(tetrisContext, LEVEL_MIN, RANDBLOCKS_MIN);
    RenderContext *renderContext = SDL_malloc(sizeof(RenderContext));
    if (renderContext == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize renderContext.");
        return SDL_APP_FAILURE;
    }
    renderContext->renderer = renderer;
    renderContext->font = font;
    GameState *gameState = SDL_malloc(sizeof(GameState));
    if (gameState == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize gameState.");
        return SDL_APP_FAILURE;
    }
    gameState->tetrisContext = tetrisContext;
    gameState->renderContext = renderContext;
    gameState->mode = MODE_SETTINGS;
    *appstate = gameState;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    GameState *gameState = (GameState *) appstate;
    switch (gameState->mode) {
        case MODE_SETTINGS:
            DrawSettingsScreen(gameState->renderContext, gameState->tetrisContext);
            break;
        case MODE_GAMEPLAY:
            UpdateGamePlay(gameState->tetrisContext);
            DrawBoard(gameState->renderContext, gameState->tetrisContext);
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    GameState *gameState = (GameState *) appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return HandleKeyDown(gameState, event->key.key);
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // TODO cleanup
    GameState *gameState = (GameState *) appstate;
    TTF_CloseFont(gameState->renderContext->font);
    TTF_Quit();
    SDL_free(gameState->tetrisContext);
    SDL_free(gameState);
}
