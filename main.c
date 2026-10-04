#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define BOARD_COLS 10
#define BOARD_ROWS 20

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
#define WINDOW_WIDTH (BOARD_OUTER_WIDTH + NEXT_TETROMINO_WINDOW_WIDTH + (PADDING_WIDTH * 3))
#define WINDOW_HEIGHT (BOARD_OUTER_HEIGHT + (PADDING_HEIGHT * 2))

#define SCORE_PER_ROW 100
#define SCORE_PER_FIX 10
#define SCORE_PER_LEVEL 1000

#define LEVEL_MAX 9
#define LEVEL_MIN 1

#define MSPT_MIN 100 

#define RANDBLOCKS_MAX 20
#define RANDBLOCKS_MIN 0

#define FONT_FILENAME "press-start-2p-latin-400-normal.ttf"

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

typedef enum {
    MODE_SETTINGS,
    MODE_GAMEPLAY,
} GameMode;

typedef struct GameState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    GameMode mode;
    TetrisContext *context;
} GameState;

bool IsEmpty(TetrisContext *context, int i, int j)
{
    return context->board[i][j].state == BLOCK_EMPTY;
}

void SetBlockState(TetrisContext *context, int i, int j, BlockState state)
{
    context->board[i][j].state = state;
}

void SetBlockColor(TetrisContext *context, int i, int j, BlockColor color)
{
    context->board[i][j].color = color;
}

void SetBlock(TetrisContext *context, int i, int j, Block block)
{
    SetBlockState(context, i, j, block.state);
    SetBlockColor(context, i, j, block.color);
}

Uint16 LevelToMSPT(Uint8 level)
{
    return SDL_max(1000 - (level * 100), MSPT_MIN);
}

void IncrementScore(TetrisContext *context, int d)
{
    context->score += d;
    if (context->score >= SCORE_PER_LEVEL * context->level) {
        context->level = SDL_min(LEVEL_MAX, context->level+1);
        context->mspt = LevelToMSPT(context->level);
    }
}


void Init(TetrisContext *context, Uint8 level, Uint8 randblocks)
{
    int i, j;
    int cnt, r, c;
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            SetBlockState(context, i, j, BLOCK_EMPTY);
            SetBlockColor(context, i, j, BLOCK_NONE);
        }
    }
    cnt = 0;
    while (cnt < randblocks) {
        r = (BOARD_ROWS - 4) + SDL_rand(4);
        c = SDL_rand(BOARD_COLS);
        if (context->board[r][c].state == BLOCK_FIXED) {
            continue;
        }
        SetBlockState(context, r, c, BLOCK_FIXED);
        SetBlockColor(context, r, c, BLOCK_GRAY);
        ++cnt;
    }
    context->next = SDL_rand(TETROMINO_COUNT);
    context->lasttick = SDL_GetTicks();
    context->level = level;
    context->randblocks = randblocks;
    context->mspt = LevelToMSPT(context->level);
    context->score = 0;
    context->gameover = false;
    context->paused = false;
}

bool NotMoving(TetrisContext *context)
{
    int i, j;
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                return false;
            }
        }
    }
    return true;
}

bool GetNext(TetrisContext *context)
{
    int i, j;
    i = 0;
    j = BOARD_COLS / 2;
    Block block;
    switch (context->next) {
        case TETROMINO_I:
            if (!(IsEmpty(context, i, j) && 
                  IsEmpty(context, i+1, j) && 
                  IsEmpty(context, i+2, j) && 
                  IsEmpty(context, i+3, j))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_CYAN;
            SetBlock(context, i, j, block);
            SetBlock(context, i+1, j, block);
            SetBlock(context, i+2, j, block);
            SetBlock(context, i+3, j, block);
            break;
        case TETROMINO_O:
            if (!(IsEmpty(context, i, j-1) && 
                  IsEmpty(context, i, j) && 
                  IsEmpty(context, i+1, j-1) && 
                  IsEmpty(context, i+1, j))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_YELLOW;
            SetBlock(context, i, j-1, block);
            SetBlock(context, i, j, block);
            SetBlock(context, i+1, j-1, block);
            SetBlock(context, i+1, j, block);
            break;
        case TETROMINO_T:
            if (!(IsEmpty(context, i, j-1) && 
                  IsEmpty(context, i, j) && 
                  IsEmpty(context, i, j+1) && 
                  IsEmpty(context, i+1, j))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_PURPLE;
            SetBlock(context, i, j-1, block);
            SetBlock(context, i, j, block);
            SetBlock(context, i, j+1, block);
            SetBlock(context, i+1, j, block);
            break;
        case TETROMINO_L:
            if (!(IsEmpty(context, i, j-1) && 
                  IsEmpty(context, i+1, j-1) && 
                  IsEmpty(context, i+2, j-1) && 
                  IsEmpty(context, i+2, j))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_ORANGE;
            SetBlock(context, i, j-1, block);
            SetBlock(context, i+1, j-1, block);
            SetBlock(context, i+2, j-1, block);
            SetBlock(context, i+2, j, block);
            break;
        case TETROMINO_J:
            if (!(IsEmpty(context, i, j) && 
                  IsEmpty(context, i+1, j) && 
                  IsEmpty(context, i+2, j) && 
                  IsEmpty(context, i+2, j-1))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_BLUE;
            SetBlock(context, i, j, block);
            SetBlock(context, i+1, j, block);
            SetBlock(context, i+2, j, block);
            SetBlock(context, i+2, j-1, block);
            break;
        case TETROMINO_S:
            if (!(IsEmpty(context, i, j) && 
                  IsEmpty(context, i, j+1) && 
                  IsEmpty(context, i+1, j-1) && 
                  IsEmpty(context, i+1, j))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_GREEN;
            SetBlock(context, i, j, block);
            SetBlock(context, i, j+1, block);
            SetBlock(context, i+1, j-1, block);
            SetBlock(context, i+1, j, block);
            break;
        case TETROMINO_Z:
            if (!(IsEmpty(context, i, j-1) && 
                  IsEmpty(context, i, j) && 
                  IsEmpty(context, i+1, j) && 
                  IsEmpty(context, i+1, j+1))) {
                return false;
            }
            block.state = BLOCK_MOVING;
            block.color = BLOCK_RED;
            SetBlock(context, i, j-1, block);
            SetBlock(context, i, j, block);
            SetBlock(context, i+1, j, block);
            SetBlock(context, i+1, j+1, block);
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
    context->next = SDL_rand(TETROMINO_COUNT);
    return true;
}

void Fall(TetrisContext *context)
{
    int i, j;
    bool fix;
    fix = false;
    for (i = BOARD_ROWS-1; i >= 0; --i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                if (i == BOARD_ROWS-1 || context->board[i+1][j].state == BLOCK_FIXED) {
                    fix = true;
                }
            }
        }
    }
    if (fix) {
        for (i = BOARD_ROWS-1; i >= 0; --i) {
            for (j = 0; j < BOARD_COLS; ++j) {
                if (context->board[i][j].state == BLOCK_MOVING) {
                    SetBlockState(context, i, j, BLOCK_FIXED);
                }
            }
        }
        IncrementScore(context, SCORE_PER_FIX);
    } else {
        for (i = BOARD_ROWS - 1; i >= 0; --i) {
            for (j = 0; j < BOARD_COLS; ++j) {
                if (context->board[i][j].state == BLOCK_MOVING) {
                    SetBlock(context, i+1, j, context->board[i][j]);
                    SetBlockState(context, i, j, BLOCK_EMPTY);
                }
            }
        }
    }
}

int GetRowsToClear(TetrisContext *context, int i)
{
    int j;
    for (j = 0; j < BOARD_COLS; ++j) {
        if (context->board[i][j].state != BLOCK_FIXED) {
            return 0;
        }
    }
    return i == 0 ? 1 : 1 + GetRowsToClear(context, i-1);
}

void ClearRows(TetrisContext *context, int i, int n)
{
    int j;
    for ( ; i >= 0; --i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (i - n >= 0) {
                SetBlock(context, i, j, context->board[i-n][j]);
            } else {
                SetBlockState(context, i, j, BLOCK_EMPTY);
            }
        }
    }
}

void UpdateBoard(TetrisContext *context)
{
    int i, n;
    for (i = BOARD_ROWS - 1; i >= 0; --i) {
        n = GetRowsToClear(context, i);
        ClearRows(context, i, n);
        IncrementScore(context, SCORE_PER_ROW * n);
    }
}

void TogglePause(TetrisContext *context)
{
    context->paused = !context->paused;
}

void MoveLeft(TetrisContext *context)
{
    int i, j;
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                if (j == 0 || context->board[i][j-1].state == BLOCK_FIXED) {
                    return;
                }
            }
        }
    }
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                SetBlock(context, i, j-1, context->board[i][j]);
                SetBlockState(context, i, j, BLOCK_EMPTY);
            }
        }
    }
}

void MoveRight(TetrisContext *context)
{
    int i, j;
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = BOARD_COLS - 1; j >= 0; --j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                if (j == BOARD_COLS - 1 || context->board[i][j+1].state == BLOCK_FIXED) {
                    return;
                }
            }
        }
    }
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = BOARD_COLS - 1; j >= 0; --j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                SetBlock(context, i, j+1, context->board[i][j]);
                SetBlockState(context, i, j, BLOCK_EMPTY);
            }
        }
    }
}

void Rotate(TetrisContext *context)
{
    int x_min = BOARD_COLS;
    int x_max = -1;
    int y_min = BOARD_ROWS;
    int y_max = -1;

    int i, j;

    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                x_min = SDL_min(x_min, j);
                x_max = SDL_max(x_max, j);
                y_min = SDL_min(y_min, i);
                y_max = SDL_max(y_max, i);
            }
        }
    }

    if (x_max == -1) {
        return;
    }

    int x_axis = (x_min + x_max) / 2;
    int y_axis = (y_min + y_max) / 2;

    int new_rows[4];
    int new_cols[4];
    int count = 0;

    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                int dx = j - x_axis;
                int dy = i - y_axis;

                /*
                 * 90-degree clockwise rotation in screen
                 * coordinates:
                 *
                 *     (dx, dy) -> (-dy, dx)
                 */
                int new_col = x_axis - dy;
                int new_row = y_axis + dx;

                if (new_row < 0 || new_row >= BOARD_ROWS ||
                    new_col < 0 || new_col >= BOARD_COLS) {
                    return;
                }

                if (context->board[new_row][new_col].state == BLOCK_FIXED) {
                    return;
                }

                new_rows[count] = new_row;
                new_cols[count] = new_col;
                ++count;
            }
        }
    }

    BlockColor color;

    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                color = context->board[i][j].color;
                SetBlockState(context, i, j, BLOCK_EMPTY);
            }
        }
    }

    for (i = 0; i < count; ++i) {
        SetBlockState(context, new_rows[i], new_cols[i], BLOCK_MOVING);
        SetBlockColor(context, new_rows[i], new_cols[i], color);
    }
}

void DrawText( GameState *gamestate, const char *text, float x, float y)
{
    SDL_Color color = { 255, 255, 255, 255 };

    SDL_Surface *surface = TTF_RenderText_Blended(
        gamestate->font,
        text,
        0,
        color
    );

    if (surface == NULL) {
        SDL_Log("Could not render text: %s", SDL_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(
        gamestate->renderer,
        surface
    );

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

    SDL_RenderTexture(
        gamestate->renderer,
        texture,
        NULL,
        &dst
    );

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

Uint8 ClampColor(int value)
{
    if (value < 0)   return 0;
    if (value > 255) return 255;
    return (Uint8)value;
}

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

void DrawBlock(GameState *gamestate, int i, int j, int x, int y, BlockColor color)
{
    x += BLOCK_WIDTH * j;
    y += BLOCK_HEIGHT * i;
    if (color == BLOCK_NONE) {
        return;
    }
    SDL_FColor base;
    switch (color) {
        case BLOCK_CYAN:
            base = (SDL_FColor){ 0x2b / 255.0f, 0xb9 / 255.0f, 0xc8 / 255.0f, 1.0f };
            break;
        case BLOCK_YELLOW:
            base = (SDL_FColor){ 0xf0 / 255.0f, 0xd5 / 255.0f, 0x2c / 255.0f, 1.0f };
            break;
        case BLOCK_PURPLE:
            base = (SDL_FColor){ 0x91 / 255.0f, 0x2d / 255.0f, 0x83 / 255.0f, 1.0f };
            break;
        case BLOCK_BLUE:
            base = (SDL_FColor){ 0x0d / 255.0f, 0x7f / 255.0f, 0xc8 / 255.0f, 1.0f };
            break;
        case BLOCK_ORANGE:
            base = (SDL_FColor){ 0xef / 255.0f, 0x80 / 255.0f, 0x10 / 255.0f, 1.0f };
            break;
        case BLOCK_GREEN:
            base = (SDL_FColor){ 0x4f / 255.0f, 0xc9 / 255.0f, 0x3c / 255.0f, 1.0f };
            break;
        case BLOCK_RED:
            base = (SDL_FColor){ 0xe9 / 255.0f, 0x46 / 255.0f, 0x5e / 255.0f, 1.0f };
            break;
        case BLOCK_GRAY:
            base = (SDL_FColor){ 0x99 / 255.0f, 0x99 / 255.0f, 0x99 / 255.0f, 1.0f };
            break;
        default:
            return;
    }
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
    SDL_SetRenderDrawColorFloat( gamestate->renderer, light.r, light.g, light.b, 1.0f);
    SDL_RenderFillRect(gamestate->renderer, &rect);
    // left
    rect = (SDL_FRect){ 
        x, y + bevel, 
        bevel, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_RenderFillRect(gamestate->renderer, &rect);
    // center
    rect = (SDL_FRect){ 
        x + bevel, y + bevel, 
        BLOCK_WIDTH - bevel * 2, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_SetRenderDrawColorFloat( gamestate->renderer, base.r, base.g, base.b, 1.0f);
    SDL_RenderFillRect(gamestate->renderer, &rect);
    // right
    rect = (SDL_FRect){ 
        x + BLOCK_WIDTH - bevel, y + bevel, 
        bevel, BLOCK_HEIGHT - bevel * 2 
    };
    SDL_SetRenderDrawColorFloat( gamestate->renderer, dark.r, dark.g, dark.b, 1.0f);
    SDL_RenderFillRect(gamestate->renderer, &rect);
    // bottom
    rect = (SDL_FRect){ 
        x, y + BLOCK_HEIGHT - bevel, 
        BLOCK_WIDTH, bevel 
    };
    SDL_RenderFillRect(gamestate->renderer, &rect);
}

void DrawNextTetromino(GameState *gamestate, int x, int y)
{
    TetrisContext *context = gamestate->context;
    int i = 1, j = 2;
    switch (context->next) {
        case TETROMINO_I:
            DrawBlock(gamestate, i, j, x, y, BLOCK_CYAN);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_CYAN);
            DrawBlock(gamestate, i+2, j, x, y, BLOCK_CYAN);
            DrawBlock(gamestate, i+3, j, x, y, BLOCK_CYAN);
            break;
        case TETROMINO_O:
            DrawBlock(gamestate, i, j-1, x, y, BLOCK_YELLOW);
            DrawBlock(gamestate, i, j, x, y, BLOCK_YELLOW);
            DrawBlock(gamestate, i+1, j-1, x, y, BLOCK_YELLOW);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_YELLOW);
            break;
        case TETROMINO_T:
            DrawBlock(gamestate, i, j-1, x, y, BLOCK_PURPLE);
            DrawBlock(gamestate, i, j, x, y, BLOCK_PURPLE);
            DrawBlock(gamestate, i, j+1, x, y, BLOCK_PURPLE);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_PURPLE);
            break;
        case TETROMINO_L:
            DrawBlock(gamestate, i, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(gamestate, i+1, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(gamestate, i+2, j-1, x, y, BLOCK_ORANGE);
            DrawBlock(gamestate, i+2, j, x, y, BLOCK_ORANGE);
            break;
        case TETROMINO_J:
            DrawBlock(gamestate, i, j, x, y, BLOCK_BLUE);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_BLUE);
            DrawBlock(gamestate, i+2, j, x, y, BLOCK_BLUE);
            DrawBlock(gamestate, i+2, j-1, x, y, BLOCK_BLUE);
            break;
        case TETROMINO_S:
            DrawBlock(gamestate, i, j, x, y, BLOCK_GREEN);
            DrawBlock(gamestate, i, j+1, x, y, BLOCK_GREEN);
            DrawBlock(gamestate, i+1, j-1, x, y, BLOCK_GREEN);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_GREEN);
            break;
        case TETROMINO_Z:
            DrawBlock(gamestate, i, j-1, x, y, BLOCK_RED);
            DrawBlock(gamestate, i, j, x, y, BLOCK_RED);
            DrawBlock(gamestate, i+1, j, x, y, BLOCK_RED);
            DrawBlock(gamestate, i+1, j+1, x, y, BLOCK_RED);
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
}

void DrawGameOver(GameState *gamestate)
{
    SDL_Renderer *renderer = gamestate->renderer;

    // Overlay rectangle
    SDL_FRect rect = {
        PADDING_WIDTH + BOARD_WALL_WIDTH,
        PADDING_HEIGHT + (BLOCK_HEIGHT * 4),
        BLOCK_WIDTH * 8,
        BLOCK_HEIGHT * 2,
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &rect);

    DrawText(
        gamestate,
        "GAME OVER",
        PADDING_WIDTH + BOARD_WALL_WIDTH + 80.0f,
        PADDING_HEIGHT + (BLOCK_HEIGHT * 4) + 20.0f
    );
}

void DrawBoard(GameState *gamestate)
{
    SDL_SetRenderDrawColor(gamestate->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gamestate->renderer);

    TetrisContext *context = gamestate->context;

    int i, j;
    int x_offset, y_offset;

    /*
     * Main board.
     */
    x_offset = PADDING_WIDTH + BOARD_WALL_WIDTH;
    y_offset = PADDING_HEIGHT + BOARD_WALL_HEIGHT;

    for (i = -1; i < BOARD_ROWS + 1; ++i) {
        for (j = -1; j < BOARD_COLS + 1; ++j) {
            if (i == -1 || i == BOARD_ROWS ||
                j == -1 || j == BOARD_COLS) {
                DrawBlock(
                    gamestate,
                    i, j,
                    x_offset, y_offset,
                    BLOCK_GRAY
                );
            }
        }
    }

    /*
     * Blocks.
     */
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_EMPTY) {
                continue;
            }

            DrawBlock(
                gamestate,
                i, j,
                x_offset, y_offset,
                context->board[i][j].color
            );
        }
    }

    /*
     * Next tetromino window.
     */
    x_offset = PADDING_WIDTH + BOARD_OUTER_WIDTH + PADDING_WIDTH;
    y_offset = PADDING_HEIGHT + (BLOCK_HEIGHT * 4);

    SDL_FRect rect = {
        x_offset,
        y_offset,
        NEXT_TETROMINO_WINDOW_WIDTH,
        NEXT_TETROMINO_WINDOW_HEIGHT
    };

    SDL_SetRenderDrawColor(
        gamestate->renderer,
        0x99, 0x99, 0x99,
        SDL_ALPHA_OPAQUE
    );

    SDL_RenderRect(gamestate->renderer, &rect);

    DrawNextTetromino(
        gamestate,
        x_offset,
        y_offset
    );

    /*
     * Information below next tetromino.
     */
    int info_y = y_offset + NEXT_TETROMINO_WINDOW_HEIGHT + 15;

    char text[64];

    SDL_snprintf(
        text,
        sizeof(text),
        "LEVEL  %d",
        context->level
    );
    DrawText(gamestate, text, x_offset, info_y);

    SDL_snprintf(
        text,
        sizeof(text),
        "SCORE  %d",
        context->score
    );
    DrawText(gamestate, text, x_offset, info_y + 30);

    if (context->paused) {
        DrawText(
            gamestate,
            "PAUSED",
            x_offset,
            info_y + 60
        );
    }

    if (context->gameover) {
        DrawGameOver(gamestate);
    }

    SDL_RenderPresent(gamestate->renderer);
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    GameState *gamestate;
    TetrisContext *context;
    SDL_Window *window;
    SDL_Renderer *renderer;
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
    window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    TTF_Font *font = TTF_OpenFont(FONT_FILENAME, 14.0f);
    if (font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    gamestate = SDL_malloc(sizeof(GameState));
    if (gamestate == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize game state.");
        return SDL_APP_FAILURE;
    }
    context = SDL_malloc(sizeof(TetrisContext));
    if (context == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize tetris context.");
        return SDL_APP_FAILURE;
    }
    gamestate->window = window;
    gamestate->renderer = renderer;
    gamestate->font = font;
    gamestate->mode = MODE_SETTINGS;
    gamestate->context = context;
    Init(gamestate->context, LEVEL_MIN, RANDBLOCKS_MIN);
    *appstate = gamestate;
    return SDL_APP_CONTINUE;
}

void TetrisMain(GameState *gamestate)
{
    TetrisContext *context = gamestate->context;
    Uint64 now = SDL_GetTicks();
    while (now - context->lasttick >= context->mspt) {
        if (!context->gameover && !context->paused) {
            if (NotMoving(context)) {
                if (!GetNext(context)) {
                    context->gameover = true;
                }
            } else {
                Fall(context);
            }
            UpdateBoard(context);
        }
        context->lasttick += context->mspt;
    }
    DrawBoard(gamestate);
}

void RenderSettingsUI(GameState *gamestate)
{
    TetrisContext *context = gamestate->context;

    SDL_SetRenderDrawColor(
        gamestate->renderer,
        0, 0, 0,
        SDL_ALPHA_OPAQUE
    );
    SDL_RenderClear(gamestate->renderer);

    /*
     * Title.
     */
    DrawText(
        gamestate,
        "TETRIS",
        30, 30
    );

    /*
     * Level.
     */
    DrawText(
        gamestate,
        "LEVEL",
        30, 80
    );

    char text[64];

    SDL_snprintf(
        text,
        sizeof(text),
        "%d",
        context->level
    );

    DrawText(
        gamestate,
        text,
        30, 110
    );

    /*
     * Starting blocks.
     */
    DrawText(
        gamestate,
        "STARTING BLOCKS",
        30, 160
    );

    SDL_snprintf(
        text,
        sizeof(text),
        "%d",
        context->randblocks
    );

    DrawText(
        gamestate,
        text,
        30, 190
    );

    /*
     * Controls.
     */
    DrawText(
        gamestate,
        "UP / DOWN     Change level",
        30, 250
    );

    DrawText(
        gamestate,
        "LEFT / RIGHT  Change starting blocks",
        30, 280
    );

    DrawText(
        gamestate,
        "ENTER         Start",
        30, 325
    );

    SDL_RenderPresent(gamestate->renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    GameState *gamestate = (GameState *) appstate;
    switch (gamestate->mode) {
        case MODE_SETTINGS:
            RenderSettingsUI(gamestate);
            break;
        case MODE_GAMEPLAY:
            TetrisMain(gamestate);
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

void HandleKeyDownSettings(GameState *gamestate, SDL_Keycode keycode)
{
    TetrisContext *context = gamestate->context;
    switch (keycode) {
        case SDLK_LEFT:
            if (context->randblocks > RANDBLOCKS_MIN) {
                context->randblocks--;
            }
            break;
        case SDLK_RIGHT:
            if (context->randblocks < RANDBLOCKS_MAX) {
                context->randblocks++;
            }
            break;
        case SDLK_UP:
            if (context->level < LEVEL_MAX) {
                context->level++;
            }
            break;
        case SDLK_DOWN:
            if (context->level > LEVEL_MIN) {
                context->level--;
            }
            break;
        case SDLK_RETURN:
            Init(context, context->level, context->randblocks);
            gamestate->mode = MODE_GAMEPLAY;
            break;
    }
}

void HandleKeyDownGamePlay(GameState *gamestate, SDL_Keycode keycode)
{
    TetrisContext *context = gamestate->context;
    if (keycode == SDLK_P) {
        TogglePause(context);
        return;
    }
    if (context->paused) {
        return;
    }
    switch (keycode) {
        case SDLK_I:
            gamestate->mode = MODE_SETTINGS;
            break;
        case SDLK_R:
            Init(context, context->level, context->randblocks);
            break;
        case SDLK_LEFT:
            MoveLeft(context);
            break;
        case SDLK_RIGHT:
            MoveRight(context);
            break;
        case SDLK_DOWN:
            Fall(context);
            break;
        case SDLK_UP:
            Rotate(context);
            break;
    }
}

SDL_AppResult HandleKeyDown(GameState *gamestate, SDL_Keycode keycode)
{
    switch (gamestate->mode) {
        case MODE_SETTINGS:
            HandleKeyDownSettings(gamestate, keycode);
            break;
        case MODE_GAMEPLAY:
            HandleKeyDownGamePlay(gamestate, keycode);
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    GameState *gamestate = (GameState *) appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return HandleKeyDown(gamestate, event->key.key);
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    GameState *gamestate = (GameState *) appstate;
    TTF_CloseFont(gamestate->font);
    TTF_Quit();
    SDL_free(gamestate->context);
    SDL_free(gamestate);
}
