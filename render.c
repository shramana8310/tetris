#include "render.h"
#include "tetris.h"

SDL_FColor ColorFromBlockColor(BlockColor color, float alpha)
{
    SDL_FColor c;
    switch (color) {
        case BLOCK_CYAN:
            c.r = 0x00 / 255.0f; c.g = 0x77 / 255.0f; c.b = 0xd3 / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_YELLOW:
            c.r = 0xfe / 255.0f; c.g = 0xfb / 255.0f; c.b = 0x34 / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_PURPLE:
            c.r = 0x78 / 255.0f; c.g = 0x25 / 255.0f; c.b = 0x6f / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_BLUE:
            c.r = 0x2e / 255.0f; c.g = 0x2e / 255.0f; c.b = 0x84 / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_ORANGE:
            c.r = 0xff / 255.0f; c.g = 0x91 / 255.0f; c.b = 0x0c / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_GREEN:
            c.r = 0x53 / 255.0f; c.g = 0xda / 255.0f; c.b = 0x3f / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_RED:
            c.r = 0xfd / 255.0f; c.g = 0x3f / 255.0f; c.b = 0x59 / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_GRAY:
            c.r = 0x99 / 255.0f; c.g = 0x99 / 255.0f; c.b = 0x99 / 255.0f;
            c.a = alpha;
            break;
        case BLOCK_NONE:
        default:
            c.r = 0.0f; c.g = 0.0f; c.b = 0.0f; c.a = 0.0f;
            break;
    }
    return c;
}

void DrawBlock(RenderContext *renderContext, int i, int j, int x, int y, BlockColor color, float alpha)
{
    x += BLOCK_WIDTH * j;
    y += BLOCK_HEIGHT * i;
    if (color == BLOCK_NONE) {
        return;
    }
    SDL_FColor base = ColorFromBlockColor(color, alpha);
    const float bevel = 2.0f;
    SDL_FColor light = {
        SDL_min(base.r * 1.35f, 1.0f),
        SDL_min(base.g * 1.35f, 1.0f),
        SDL_min(base.b * 1.35f, 1.0f),
        alpha
    };
    SDL_FColor dark = {
        base.r * 0.55f,
        base.g * 0.55f,
        base.b * 0.55f,
        alpha
    };
    // top
    SDL_FRect rect = { 
        x, y, 
        BLOCK_WIDTH, bevel 
    };
    SDL_SetRenderDrawBlendMode(renderContext->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColorFloat(renderContext->renderer, light.r, light.g, light.b, alpha);
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
    SDL_SetRenderDrawBlendMode(renderContext->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColorFloat(renderContext->renderer, base.r, base.g, base.b, alpha);
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // right
    rect = (SDL_FRect){ 
        x + BLOCK_WIDTH - bevel, y + bevel, 
        bevel, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_SetRenderDrawBlendMode(renderContext->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColorFloat(renderContext->renderer, dark.r, dark.g, dark.b, alpha);
    SDL_RenderFillRect(renderContext->renderer, &rect);
    // bottom
    rect = (SDL_FRect){ 
        x, y + BLOCK_HEIGHT - bevel, 
        BLOCK_WIDTH, bevel 
    };
    SDL_RenderFillRect(renderContext->renderer, &rect);
}

void DrawNextTetromino(RenderContext *renderContext, TetrominoType next, int x, int y)
{
    int i;
    int r, c;
    Tetromino tetromino = CreateTetrominoByType(next);
    switch (next) {
        case TETROMINO_I:
            r = 0, c = 0;
            x += BLOCK_WIDTH / 2;
            y += BLOCK_HEIGHT / 2;
            break;
        case TETROMINO_O:
            r = 1, c = 0;
            x += BLOCK_WIDTH / 2;
            break;
        case TETROMINO_T:
        case TETROMINO_L:
        case TETROMINO_J:
        case TETROMINO_S:
        case TETROMINO_Z:
            r = 1, c = 1;
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        tetromino.coordinates[i].row += r;
        tetromino.coordinates[i].col += c;
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        DrawBlock(
            renderContext, 
            tetromino.coordinates[i].row,
            tetromino.coordinates[i].col,
            x, 
            y, 
            tetromino.color,
            1.0f
        );
    }
}

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
        PADDING_WIDTH + BOARD_WALL_WIDTH + 80.0f, 
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
                DrawBlock(renderContext, i, j, x_offset, y_offset, BLOCK_GRAY, 1.0f);
            }
        }
    }
    // main board
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (tetrisContext->board[i][j] == BLOCK_NONE) {
                continue;
            }
            DrawBlock(renderContext, i, j, x_offset, y_offset, tetrisContext->board[i][j], 1.0f);
        }
    }
    // current tetromino
    if (tetrisContext->moving) {
        Coordinate coordinate;
        for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
            coordinate = tetrisContext->current.coordinates[i];
            DrawBlock(renderContext, coordinate.row, coordinate.col, x_offset, y_offset, tetrisContext->current.color, 1.0f);
        }
        Tetromino ghost = GetLandingTetromino(tetrisContext);
        for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
            coordinate = ghost.coordinates[i];
            DrawBlock(renderContext, coordinate.row, coordinate.col, x_offset, y_offset, tetrisContext->current.color, 0.4f);
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
    if (tetrisContext->gameOver) {
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
    SDL_snprintf(text, sizeof(text), "%d", tetrisContext->startingBlocks);
    DrawText(renderContext, text, 30, 190);
    DrawText(renderContext, "UP / DOWN     Change level", 30, 250);
    DrawText(renderContext, "LEFT / RIGHT  Change starting blocks", 30, 280);
    DrawText(renderContext, "ENTER         Start", 30, 325);
    SDL_RenderPresent(renderContext->renderer);
}
