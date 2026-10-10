#include "render.h"

SDL_FColor ColorFromBlockColor(BlockColor color)
{
    SDL_FColor c;
    switch (color) {
        case BLOCK_CYAN:
            c = (SDL_FColor){ 0x00 / 255.0f, 0x77 / 255.0f, 0xd3 / 255.0f, 1.0f };
            break;
        case BLOCK_YELLOW:
            c = (SDL_FColor){ 0xfe / 255.0f, 0xfb / 255.0f, 0x34 / 255.0f, 1.0f };
            break;
        case BLOCK_PURPLE:
            c = (SDL_FColor){ 0x78 / 255.0f, 0x25 / 255.0f, 0x6f / 255.0f, 1.0f };
            break;
        case BLOCK_BLUE:
            c = (SDL_FColor){ 0x2e / 255.0f, 0x2e / 255.0f, 0x84 / 255.0f, 1.0f };
            break;
        case BLOCK_ORANGE:
            c = (SDL_FColor){ 0xff / 255.0f, 0x91 / 255.0f, 0x0c / 255.0f, 1.0f };
            break;
        case BLOCK_GREEN:
            c = (SDL_FColor){ 0x53 / 255.0f, 0xda / 255.0f, 0x3f / 255.0f, 1.0f };
            break;
        case BLOCK_RED:
            c = (SDL_FColor){ 0xfd / 255.0f, 0x3f / 255.0f, 0x59 / 255.0f, 1.0f };
            break;
        case BLOCK_GRAY:
            c = (SDL_FColor){ 0x99 / 255.0f, 0x99 / 255.0f, 0x99 / 255.0f, 1.0f };
            break;
        case BLOCK_NONE:
        default:
            c = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 0.0f };
            break;
    }
    return c;
}

void DrawBlock(RenderContext *renderContext, int i, int j, int x, int y, BlockColor color)
{
    x += BLOCK_WIDTH * j;
    y += BLOCK_HEIGHT * i;
    if (color == BLOCK_NONE) {
        return;
    }
    SDL_FColor base = ColorFromBlockColor(color);
    const float bevel = 2.0f;
    SDL_FColor light = {
        SDL_min(base.r * 1.35f, 1.0f),
        SDL_min(base.g * 1.35f, 1.0f),
        SDL_min(base.b * 1.35f, 1.0f),
        1.0f
    };
    SDL_FColor dark = {
        base.r * 0.55f,
        base.g * 0.55f,
        base.b * 0.55f,
        1.0f
    };
    // top
    SDL_FRect rect = { 
        x, y, 
        BLOCK_WIDTH, bevel 
    };
    SDL_SetRenderDrawColorFloat(renderContext->renderer, light.r, light.g, light.b, 1.0f);
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // left
    rect = (SDL_FRect){ 
        x, y + bevel, 
        bevel, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // center
    rect = (SDL_FRect){ 
        x + bevel, y + bevel, 
        BLOCK_WIDTH - bevel * 2, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_SetRenderDrawColorFloat(renderContext->renderer, base.r, base.g, base.b, 1.0f);
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // right
    rect = (SDL_FRect){ 
        x + BLOCK_WIDTH - bevel, y + bevel, 
        bevel, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_SetRenderDrawColorFloat(renderContext->renderer, dark.r, dark.g, dark.b, 1.0f);
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // bottom
    rect = (SDL_FRect){ 
        x, y + BLOCK_HEIGHT - bevel, 
        BLOCK_WIDTH, bevel 
    };
    SDL_RenderFillRect(renderContext->renderer, &rect);
}

void DrawNextTetromino(RenderContext *renderContext, Tetromino next, int x, int y)
{
    int i = 1, j = 2;
    // TODO define a function that maps each tetromino to coordinates and block color
    switch (next) {
        case TETROMINO_I:
            DrawBlock(renderContext, i, j, x, y, BLOCK_CYAN);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_CYAN);
            DrawBlock(renderContext, i+2, j, x, y, BLOCK_CYAN);
            DrawBlock(renderContext, i+3, j, x, y, BLOCK_CYAN);
            break;
        case TETROMINO_O:
            DrawBlock(renderContext, i, j-1, x, y, BLOCK_YELLOW);
            DrawBlock(renderContext, i, j, x, y, BLOCK_YELLOW);
            DrawBlock(renderContext, i+1, j-1, x, y, BLOCK_YELLOW);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_YELLOW);
            break;
        case TETROMINO_T:
            DrawBlock(renderContext, i, j-1, x, y, BLOCK_PURPLE);
            DrawBlock(renderContext, i, j, x, y, BLOCK_PURPLE);
            DrawBlock(renderContext, i, j+1, x, y, BLOCK_PURPLE);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_PURPLE);
            break;
        case TETROMINO_L:
            DrawBlock(renderContext, i, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(renderContext, i+1, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(renderContext, i+2, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(renderContext, i+2, j, x, y, BLOCK_ORANGE);
            break;
        case TETROMINO_J:
            DrawBlock(renderContext, i, j, x, y, BLOCK_BLUE);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_BLUE);
            DrawBlock(renderContext, i+2, j, x, y, BLOCK_BLUE);
            DrawBlock(renderContext, i+2, j-1, x, y, BLOCK_BLUE);
            break;
        case TETROMINO_S:
            DrawBlock(renderContext, i, j, x, y, BLOCK_GREEN);
            DrawBlock(renderContext, i, j+1, x, y, BLOCK_GREEN);
            DrawBlock(renderContext, i+1, j-1, x, y, BLOCK_GREEN);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_GREEN);
            break;
        case TETROMINO_Z:
            DrawBlock(renderContext, i, j-1, x, y, BLOCK_RED);
            DrawBlock(renderContext, i, j, x, y, BLOCK_RED);
            DrawBlock(renderContext, i+1, j, x, y, BLOCK_RED);
            DrawBlock(renderContext, i+1, j+1, x, y, BLOCK_RED);
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
}

// TODO caching
void DrawText(RenderContext *renderContext, const char *text, float x, float y)
{
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface *surface = TTF_RenderText_Blended(renderContext->font, text, 0, color);
    if (surface == NULL) {
        SDL_Log("Could not render text: %s", SDL_GetError());
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderContext->renderer, surface);
    if (texture == NULL) {
        SDL_Log("Could not create text texture: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }
    SDL_FRect dst = {
        x,
        y,
        (float)surface->w,
        (float)surface->h
    };
    SDL_RenderTexture(renderContext->renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void DrawGameOver(RenderContext *renderContext)
{
    SDL_Renderer *renderer = renderContext->renderer;
    SDL_FRect rect = {
        PADDING_WIDTH + (BOARD_WALL_WIDTH * 3),
        PADDING_HEIGHT + (BLOCK_HEIGHT * 4),
        BLOCK_WIDTH * 6,
        BLOCK_HEIGHT * 2,
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &rect);
    DrawText(
        renderContext, 
        "GAME OVER", 
        PADDING_WIDTH + BOARD_WALL_WIDTH + 100.0f, 
        PADDING_HEIGHT + (BLOCK_HEIGHT * 4) + 23.0f
    );
}

void DrawBoard(RenderContext *renderContext, TetrisContext *tetrisContext)
{
    SDL_SetRenderDrawColor(renderContext->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderContext->renderer);
    int i, j;
    int x_offset, y_offset;
    // main board (outer blocks)
    x_offset = PADDING_WIDTH + BOARD_WALL_WIDTH;
    y_offset = PADDING_HEIGHT + BOARD_WALL_HEIGHT;
    for (i = -1; i < BOARD_ROWS + 1; ++i) {
        for (j = -1; j < BOARD_COLS + 1; ++j) {
            if (i == -1 || i == BOARD_ROWS ||
                j == -1 || j == BOARD_COLS) {
                DrawBlock(renderContext, i, j, x_offset, y_offset, BLOCK_GRAY);
            }
        }
    }
    // main board
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (tetrisContext->board[i][j].state == BLOCK_EMPTY) {
                continue;
            }
            DrawBlock(renderContext, i, j, x_offset, y_offset, tetrisContext->board[i][j].color);
        }
    }
    // next tetromino
    x_offset = PADDING_WIDTH + BOARD_OUTER_WIDTH + PADDING_WIDTH;
    y_offset = PADDING_HEIGHT + (BLOCK_HEIGHT * 4);
    SDL_FRect rect = {
        x_offset,
        y_offset,
        NEXT_TETROMINO_WINDOW_WIDTH,
        NEXT_TETROMINO_WINDOW_HEIGHT
    };
    SDL_SetRenderDrawColor(renderContext->renderer, 0x99, 0x99, 0x99, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderContext->renderer, &rect);
    DrawNextTetromino(renderContext, tetrisContext->next, x_offset, y_offset);
    int info_y = y_offset + NEXT_TETROMINO_WINDOW_HEIGHT + 15;
    char text[64];
    SDL_snprintf(text, sizeof(text), "LEVEL  %d", tetrisContext->level);
    DrawText(renderContext, text, x_offset, info_y);
    SDL_snprintf(text, sizeof(text), "SCORE  %d", tetrisContext->score);
    DrawText(renderContext, text, x_offset, info_y + 30);
    if (tetrisContext->paused) {
        DrawText(renderContext, "PAUSED", x_offset, info_y + 60);
    }
    if (tetrisContext->gameover) {
        DrawGameOver(renderContext);
    }
    SDL_RenderPresent(renderContext->renderer);
}

void DrawSettingsScreen(RenderContext *renderContext, TetrisContext *tetrisContext)
{
    SDL_SetRenderDrawColor(
        renderContext->renderer,
        0, 0, 0,
        SDL_ALPHA_OPAQUE
    );
    SDL_RenderClear(renderContext->renderer);
    DrawText(renderContext, "TETRIS", 30, 30);
    DrawText(renderContext, "LEVEL", 30, 80);
    char text[64];
    SDL_snprintf(text, sizeof(text), "%d", tetrisContext->level);
    DrawText(renderContext, text, 30, 110);
    DrawText(renderContext, "STARTING BLOCKS", 30, 160);
    SDL_snprintf(text, sizeof(text), "%d", tetrisContext->randblocks);
    DrawText(renderContext, text, 30, 190);
    DrawText(renderContext, "UP / DOWN     Change level", 30, 250);
    DrawText(renderContext, "LEFT / RIGHT  Change starting blocks", 30, 280);
    DrawText(renderContext, "ENTER         Start", 30, 325);
    SDL_RenderPresent(renderContext->renderer);
}

