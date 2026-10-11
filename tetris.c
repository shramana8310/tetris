#include "tetris.h"

void PopNextTetromino(TetrisContext *context);
int GetRowsToClear(TetrisContext *context, int r);
void ClearRows(TetrisContext *context, int r, int n);
Uint16 LevelToMSPT(Uint8 level);
void IncrementScore(TetrisContext *context, int d);
void UpdateTetrisContext(TetrisContext *context);

void InitTetrisContext(TetrisContext *context, Uint8 level, Uint8 startingBlocks)
{
    int i, r, c;
    for (r = 0; r < BOARD_ROWS; ++r) {
        for (c = 0; c < BOARD_COLS; ++c) {
            context->board[r][c] = BLOCK_NONE;
        }
    }
    i = 0;
    while (i < startingBlocks) {
        r = (BOARD_ROWS - 4) + SDL_rand(4);
        c = SDL_rand(BOARD_COLS);
        if (context->board[r][c] != BLOCK_NONE) {
            continue;
        }
        context->board[r][c] = BLOCK_GRAY;
        ++i;
    }
    context->next = SDL_rand(TETROMINO_COUNT);
    PopNextTetromino(context);
    context->lastTick = SDL_GetTicks();
    context->level = level;
    context->startingBlocks = startingBlocks;
    context->mspt = LevelToMSPT(context->level);
    context->score = 0;
    context->gameOver = false;
    context->paused = false;
}

void UpdateGamePlay(TetrisContext *context)
{
    Uint64 now = SDL_GetTicks();
    while (now - context->lastTick >= context->mspt) {
        if (!context->gameOver && !context->paused) {
            if (context->moving) {
                MoveDown(context);
            } else {
                PopNextTetromino(context);
            }
            UpdateTetrisContext(context);
        }
        context->lastTick += context->mspt;
    }
}

void MoveDown(TetrisContext *context)
{
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.row == BOARD_ROWS-1 || 
            context->board[coordinate.row+1][coordinate.col] != BLOCK_NONE) {
            context->moving = false;
        }
    }
    if (context->moving) {
        for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
            ++context->current.coordinates[i].row;
        }
    } else {
        for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
            coordinate = context->current.coordinates[i];
            context->board[coordinate.row][coordinate.col] = context->current.color;
        }
        IncrementScore(context, SCORE_PER_FIX);
    }
}

void MoveLeft(TetrisContext *context)
{
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.col == 0 || 
            context->board[coordinate.row][coordinate.col-1] != BLOCK_NONE) {
            return;
        }
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        --context->current.coordinates[i].col;
    }
}

void MoveRight(TetrisContext *context)
{
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.col == BOARD_COLS-1 || 
            context->board[coordinate.row][coordinate.col+1] != BLOCK_NONE) {
            return;
        }
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        ++context->current.coordinates[i].col;
    }
}

// TODO fix rotation logic
void Rotate(TetrisContext *context)
{
    int x_min = BOARD_COLS;
    int x_max = -1;
    int y_min = BOARD_ROWS;
    int y_max = -1;

    int i, j;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        x_min = SDL_min(context->current.coordinates[i].col, x_min);
        x_max = SDL_max(context->current.coordinates[i].col, x_max);
        y_min = SDL_min(context->current.coordinates[i].row, y_min);
        y_max = SDL_max(context->current.coordinates[i].row, y_max);
    }

    if (x_max == -1) {
        return;
    }

    int x_axis = (x_min + x_max) / 2;
    int y_axis = (y_min + y_max) / 2;

    int new_rows[4];
    int new_cols[4];
    int count = 0;

    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        Coordinate coordinate = context->current.coordinates[i];
        int dx = coordinate.col - x_axis;
        int dy = coordinate.row - y_axis;

        int new_col = x_axis - dy;
        int new_row = y_axis + dx;

        if (new_row < 0 || new_row >= BOARD_ROWS ||
            new_col < 0 || new_col >= BOARD_COLS) {
            return;
        }

        if (context->board[new_row][new_col] != BLOCK_NONE) {
            return;
        }

        new_rows[count] = new_row;
        new_cols[count] = new_col;
        ++count;

    }

    BlockColor color;

    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        context->current.coordinates[i].row = new_rows[i];
        context->current.coordinates[i].col = new_cols[i];
    }

}

void TogglePause(TetrisContext *context)
{
    context->paused = !context->paused;
}

void IncrementStartingBlocks(TetrisContext *context)
{
    if (context->startingBlocks < STARTING_BLOCKS_MAX) {
        context->startingBlocks++;
    }
}

void DecrementStartingBlocks(TetrisContext *context)
{
    if (context->startingBlocks > STARTING_BLOCKS_MIN) {
        context->startingBlocks--;
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

Tetromino CreateTetrominoByType(TetrominoType tetrominoType)
{
    Tetromino tetromino;
    switch (tetrominoType) {
        case TETROMINO_I:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 0;
            tetromino.coordinates[2].row = 2;
            tetromino.coordinates[2].col = 0;
            tetromino.coordinates[3].row = 3;
            tetromino.coordinates[3].col = 0;
            tetromino.color = BLOCK_CYAN;
            break;
        case TETROMINO_O:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 0;
            tetromino.coordinates[1].col = 1;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 0;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 1;
            tetromino.color = BLOCK_YELLOW;
            break;
        case TETROMINO_T:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 0;
            tetromino.coordinates[1].col = 1;
            tetromino.coordinates[2].row = 0;
            tetromino.coordinates[2].col = 2;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 1;
            tetromino.color = BLOCK_PURPLE;
            break;
        case TETROMINO_L:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 0;
            tetromino.coordinates[2].row = 2;
            tetromino.coordinates[2].col = 0;
            tetromino.coordinates[3].row = 2;
            tetromino.coordinates[3].col = 1;
            tetromino.color = BLOCK_ORANGE;
            break;
        case TETROMINO_J:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 1;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 1;
            tetromino.coordinates[2].row = 2;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 2;
            tetromino.coordinates[3].col = 0;
            tetromino.color = BLOCK_BLUE;
            break;
        case TETROMINO_S:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 1;
            tetromino.coordinates[1].row = 0;
            tetromino.coordinates[1].col = 2;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 0;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 1;
            tetromino.color = BLOCK_GREEN;
            break;
        case TETROMINO_Z:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 0;
            tetromino.coordinates[1].col = 1;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 2;
            tetromino.color = BLOCK_RED;
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
    return tetromino;
}

void PopNextTetromino(TetrisContext *context)
{
    Tetromino tetromino;
    Coordinate coordinate;
    int i, r, c;
    tetromino = CreateTetrominoByType(context->next);
    r = 0;
    c = (BOARD_COLS - 1) / 2;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        tetromino.coordinates[i].row += r;
        tetromino.coordinates[i].col += c;
    }
    // TODO adjust col or rotate?
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = tetromino.coordinates[i];
        if (context->board[coordinate.row][coordinate.col] != BLOCK_NONE) {
            context->gameOver = true;
            return;
        }
    }
    context->current = tetromino;
    context->moving = true;
    context->next = SDL_rand(TETROMINO_COUNT);
}

int GetRowsToClear(TetrisContext *context, int r)
{
    int c;
    for (c = 0; c < BOARD_COLS; ++c) {
        if (context->board[r][c] == BLOCK_NONE) {
            return 0;
        }
    }
    return r == 0 ? 1 : 1 + GetRowsToClear(context, r-1);
}

void ClearRows(TetrisContext *context, int r, int n)
{
    int c;
    for ( ; r >= 0; --r) {
        for (c = 0; c < BOARD_COLS; ++c) {
            if (r - n >= 0) {
                context->board[r][c] = context->board[r-n][c];
            } else {
                context->board[r][c] = BLOCK_NONE;
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

void UpdateTetrisContext(TetrisContext *context)
{
    int r, n;
    for (r = BOARD_ROWS - 1; r >= 0; --r) {
        n = GetRowsToClear(context, r);
        ClearRows(context, r, n);
        IncrementScore(context, SCORE_PER_ROW * n);
    }
}
