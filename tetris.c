#include "tetris.h"

bool IsEmpty(TetrisContext *context, int i, int j);
void SetBlock(TetrisContext *context, int i, int j, Block block);
void SetBlockState(TetrisContext *context, int i, int j, BlockState state);
void SetBlockColor(TetrisContext *context, int i, int j, BlockColor color);
bool IsMoving(TetrisContext *context);
bool PopNextTetromino(TetrisContext *context);
int GetRowsToClear(TetrisContext *context, int i);
void ClearRows(TetrisContext *context, int i, int n);
Uint16 LevelToMSPT(Uint8 level);
void IncrementScore(TetrisContext *context, int d);
void UpdateBoard(TetrisContext *context);

void InitTetrisContext(TetrisContext *context, Uint8 level, Uint8 randblocks)
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

void UpdateGamePlay(TetrisContext *context)
{
    Uint64 now = SDL_GetTicks();
    while (now - context->lasttick >= context->mspt) {
        if (!context->gameover && !context->paused) {
            if (!IsMoving(context)) {
                if (!PopNextTetromino(context)) {
                    context->gameover = true;
                }
            } else {
                MoveDown(context);
            }
            UpdateBoard(context);
        }
        context->lasttick += context->mspt;
    }
}

void MoveDown(TetrisContext *context)
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

// TODO This is too complex and simplified, but first rethink the board data structure
// TODO implement wall kick
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

void TogglePause(TetrisContext *context)
{
    context->paused = !context->paused;
}

void IncrementRandBlocks(TetrisContext *context)
{
    if (context->randblocks < RANDBLOCKS_MAX) {
        context->randblocks++;
    }
}

void DecrementRandBlocks(TetrisContext *context)
{
    if (context->randblocks > RANDBLOCKS_MIN) {
        context->randblocks--;
    }
}

void IncrementLevel(TetrisContext *context)
{
    if (context->level < LEVEL_MAX) {
        context->level++;
    }
}

void DecrementLevel(TetrisContext *context)
{
    if (context->level > LEVEL_MIN) {
        context->level--;
    }
}

bool IsEmpty(TetrisContext *context, int i, int j)
{
    return context->board[i][j].state == BLOCK_EMPTY;
}

void SetBlock(TetrisContext *context, int i, int j, Block block)
{
    SetBlockState(context, i, j, block.state);
    SetBlockColor(context, i, j, block.color);
}

void SetBlockState(TetrisContext *context, int i, int j, BlockState state)
{
    context->board[i][j].state = state;
}

void SetBlockColor(TetrisContext *context, int i, int j, BlockColor color)
{
    context->board[i][j].color = color;
}

bool IsMoving(TetrisContext *context)
{
    int i, j;
    for (i = 0; i < BOARD_ROWS; ++i) {
        for (j = 0; j < BOARD_COLS; ++j) {
            if (context->board[i][j].state == BLOCK_MOVING) {
                return true;
            }
        }
    }
    return false;
}

bool PopNextTetromino(TetrisContext *context)
{
    int i, j;
    i = 0;
    j = BOARD_COLS / 2;
    Block block;
    // TODO maybe we should create a function to map each tetromino to indices
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

void UpdateBoard(TetrisContext *context)
{
    int i, n;
    for (i = BOARD_ROWS - 1; i >= 0; --i) {
        n = GetRowsToClear(context, i);
        ClearRows(context, i, n);
        IncrementScore(context, SCORE_PER_ROW * n);
    }
}
