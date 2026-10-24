#include "tetris.h"
#define WALL_KICK_PAIR_SIZE 4

struct Pair { int r; int c; };

void SpawnNextTetromino(TetrisContext *context);
Tetromino RotateTetromino(Tetromino tetromino);
Tetromino ShiftTetromino(Tetromino tetromino, int r, int c);
bool CanPlaceTetromino(TetrisContext *context, Tetromino tetromino);
void GetWallKickPairs(TetrominoType type, Orientation orientation, struct Pair pairs[WALL_KICK_PAIR_SIZE]);
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
    SpawnNextTetromino(context);
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
    if (context->gameOver || context->paused) {
        while (now - context->lastTick >= context->mspt) {
            context->lastTick += context->mspt;
        }
        return;
    }

    if (!context->moving) {
        SpawnNextTetromino(context);
    }
    while (now - context->lastTick >= context->mspt) {
        MoveDown(context);
        context->lastTick += context->mspt;
    }
    UpdateTetrisContext(context);
}

void MoveDown(TetrisContext *context)
{
    if (context->gameOver || context->paused) {
        return;
    }
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.row+1 >= BOARD_ROWS || 
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

void HardDrop(TetrisContext *context)
{
    if (context->gameOver || context->paused) {
        return;
    }
    int i;
    Coordinate coordinate;
    context->current = GetLandingTetromino(context);
    context->moving = false;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        context->board[coordinate.row][coordinate.col] = context->current.color;
    }
    IncrementScore(context, SCORE_PER_FIX);
}

void MoveLeft(TetrisContext *context)
{
    if (context->gameOver || context->paused) {
        return;
    }
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.col-1 < 0 || 
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
    if (context->gameOver || context->paused) {
        return;
    }
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = context->current.coordinates[i];
        if (coordinate.col+1 >= BOARD_COLS || 
            context->board[coordinate.row][coordinate.col+1] != BLOCK_NONE) {
            return;
        }
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        ++context->current.coordinates[i].col;
    }
}

void Rotate(TetrisContext *context)
{
    if (context->gameOver || context->paused) {
        return;
    }
    int i;
    Tetromino after;
    after = RotateTetromino(context->current);
    if (CanPlaceTetromino(context, after)) {
        context->current = after;
        return;
    }
    Tetromino temp;
    struct Pair pairs[WALL_KICK_PAIR_SIZE];
    GetWallKickPairs(context->current.type, context->current.orientation, pairs);
    for (i = 0; i < WALL_KICK_PAIR_SIZE; ++i) {
        temp = ShiftTetromino(after, pairs[i].r, pairs[i].c);
        if (CanPlaceTetromino(context, temp)) {
            context->current = temp;
            return;
        }
    }
}

void TogglePause(TetrisContext *context)
{
    if (context->gameOver) {
        return;
    }
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
    tetromino.type = tetrominoType;
    tetromino.orientation = ORIENTATION_NORTH;
    switch (tetrominoType) {
        case TETROMINO_I:
            tetromino.coordinates[0].row = 1;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 1;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 2;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 3;
            tetromino.color = BLOCK_CYAN;
            break;
        case TETROMINO_O:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 1;
            tetromino.coordinates[1].row = 0;
            tetromino.coordinates[1].col = 2;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 2;
            tetromino.color = BLOCK_YELLOW;
            break;
        case TETROMINO_T:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 1;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 0;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 2;
            tetromino.color = BLOCK_PURPLE;
            break;
        case TETROMINO_L:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 2;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 0;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 2;
            tetromino.color = BLOCK_ORANGE;
            break;
        case TETROMINO_J:
            tetromino.coordinates[0].row = 0;
            tetromino.coordinates[0].col = 0;
            tetromino.coordinates[1].row = 1;
            tetromino.coordinates[1].col = 0;
            tetromino.coordinates[2].row = 1;
            tetromino.coordinates[2].col = 1;
            tetromino.coordinates[3].row = 1;
            tetromino.coordinates[3].col = 2;
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

Tetromino GetLandingTetromino(TetrisContext *context)
{
    Tetromino tetromino;
    int r, i;
    bool found;
    Coordinate coordinate;
    tetromino = context->current;
    found = false;
    for (r = 1; r < BOARD_ROWS; ++r) {
        for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
            coordinate = tetromino.coordinates[i];
            if (coordinate.row + r == BOARD_ROWS || 
                context->board[coordinate.row + r][coordinate.col] != BLOCK_NONE) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        tetromino.coordinates[i].row += r-1;
    }
    return tetromino;
}

void SpawnNextTetromino(TetrisContext *context)
{
    Tetromino tetromino;
    Coordinate coordinate;
    int i, c;
    tetromino = CreateTetrominoByType(context->next);
    c = (BOARD_COLS / 2) - 2;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        tetromino.coordinates[i].col += c;
    }
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

Tetromino RotateTetromino(Tetromino tetromino)
{
    switch (tetromino.type) {
        case TETROMINO_I:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row -= 1;
                    tetromino.coordinates[0].col += 2;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 1;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 2;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 2;
                    tetromino.coordinates[0].col += 1;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col -= 1;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col -= 2;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row += 1;
                    tetromino.coordinates[0].col -= 2;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row -= 1;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 2;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row -= 2;
                    tetromino.coordinates[0].col -= 1;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 1;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col += 2;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_O:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_T:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 1;
                    tetromino.coordinates[0].col += 1;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 1;
                    tetromino.coordinates[0].col -= 1;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row -= 1;
                    tetromino.coordinates[0].col -= 1;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row -= 1;
                    tetromino.coordinates[0].col += 1;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_L:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col -= 2;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row -= 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 2;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_J:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 2;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col -= 2;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row -= 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_S:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 1;
                    tetromino.coordinates[0].col += 1;
                    tetromino.coordinates[1].row += 2;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row -= 1;
                    tetromino.coordinates[2].col += 1;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 1;
                    tetromino.coordinates[0].col -= 1;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col -= 2;
                    tetromino.coordinates[2].row += 1;
                    tetromino.coordinates[2].col += 1;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row -= 1;
                    tetromino.coordinates[0].col -= 1;
                    tetromino.coordinates[1].row -= 2;
                    tetromino.coordinates[1].col += 0;
                    tetromino.coordinates[2].row += 1;
                    tetromino.coordinates[2].col -= 1;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row -= 1;
                    tetromino.coordinates[0].col += 1;
                    tetromino.coordinates[1].row += 0;
                    tetromino.coordinates[1].col += 2;
                    tetromino.coordinates[2].row -= 1;
                    tetromino.coordinates[2].col -= 1;
                    tetromino.coordinates[3].row += 0;
                    tetromino.coordinates[3].col += 0;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_Z:
            switch (tetromino.orientation) {
                case ORIENTATION_NORTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col += 2;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_EAST;
                    break;
                case ORIENTATION_EAST:
                    tetromino.coordinates[0].row += 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row += 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col -= 1;
                    tetromino.orientation = ORIENTATION_SOUTH;
                    break;
                case ORIENTATION_SOUTH:
                    tetromino.coordinates[0].row += 0;
                    tetromino.coordinates[0].col -= 2;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col -= 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row -= 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_WEST;
                    break;
                case ORIENTATION_WEST:
                    tetromino.coordinates[0].row -= 2;
                    tetromino.coordinates[0].col += 0;
                    tetromino.coordinates[1].row -= 1;
                    tetromino.coordinates[1].col += 1;
                    tetromino.coordinates[2].row += 0;
                    tetromino.coordinates[2].col += 0;
                    tetromino.coordinates[3].row += 1;
                    tetromino.coordinates[3].col += 1;
                    tetromino.orientation = ORIENTATION_NORTH;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
    return tetromino;
}

void GetWallKickPairs(TetrominoType type, Orientation orientation, struct Pair pairs[WALL_KICK_PAIR_SIZE])
{
    switch (type) {
        case TETROMINO_I:
            switch (orientation) {
                case ORIENTATION_NORTH:
                    pairs[0].r = 0;  pairs[0].c = -2;
                    pairs[1].r = 0;  pairs[1].c = 1;
                    pairs[2].r = 1;  pairs[2].c = -2;
                    pairs[3].r = -2; pairs[3].c = 1;
                    break;
                case ORIENTATION_EAST:
                    pairs[0].r = 0;  pairs[0].c = -1;
                    pairs[1].r = 0;  pairs[1].c = 2;
                    pairs[2].r = -2; pairs[2].c = -1;
                    pairs[3].r = 1;  pairs[3].c = 2;
                    break;
                case ORIENTATION_SOUTH:
                    pairs[0].r = 0;  pairs[0].c = 2;
                    pairs[1].r = 0;  pairs[1].c = -1;
                    pairs[2].r = -1; pairs[2].c = 2;
                    pairs[3].r = 2;  pairs[3].c = -1;
                    break;
                case ORIENTATION_WEST:
                    pairs[0].r = 0;  pairs[0].c = 1;
                    pairs[1].r = 0;  pairs[1].c = -2;
                    pairs[2].r = 2;  pairs[2].c = 1;
                    pairs[3].r = -1; pairs[3].c = -2;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_O:
        case TETROMINO_T:
        case TETROMINO_L:
        case TETROMINO_J:
        case TETROMINO_S:
        case TETROMINO_Z:
            switch (orientation) {
                case ORIENTATION_NORTH:
                    pairs[0].r = 0;  pairs[0].c = -1;
                    pairs[1].r = -1; pairs[1].c = -1;
                    pairs[2].r = 2;  pairs[2].c = 0;
                    pairs[3].r = 2;  pairs[3].c = -1;
                    break;
                case ORIENTATION_EAST:
                    pairs[0].r = 0;  pairs[0].c = 1;
                    pairs[1].r = 1;  pairs[1].c = 1;
                    pairs[2].r = -2; pairs[2].c = 0;
                    pairs[3].r = -2; pairs[3].c = 1;
                    break;
                case ORIENTATION_SOUTH:
                    pairs[0].r = 0;  pairs[0].c = 1;
                    pairs[1].r = -1; pairs[1].c = 1;
                    pairs[2].r = 2;  pairs[2].c = 0;
                    pairs[3].r = 2;  pairs[3].c = 1;
                    break;
                case ORIENTATION_WEST:
                    pairs[0].r = 0;  pairs[0].c = -1;
                    pairs[1].r = 1;  pairs[1].c = -1;
                    pairs[2].r = -2; pairs[2].c = 0;
                    pairs[3].r = -2; pairs[3].c = -1;
                    break;
                default:
                    break;
            }
            break;
        case TETROMINO_COUNT:
        default:
            break;
    }
}

bool CanPlaceTetromino(TetrisContext *context, Tetromino tetromino)
{
    int i;
    Coordinate coordinate;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        coordinate = tetromino.coordinates[i];
        if (coordinate.row < 0 || coordinate.row >= BOARD_ROWS ||
            coordinate.col < 0 || coordinate.col >= BOARD_COLS) {
            return false;
        }
        if (context->board[coordinate.row][coordinate.col] != BLOCK_NONE) {
            return false;
        }
    }
    return true;
}

Tetromino ShiftTetromino(Tetromino tetromino, int r, int c)
{
    int i;
    for (i = 0; i < NO_BLOCKS_PER_TETROMINO; ++i) {
        tetromino.coordinates[i].row += r;
        tetromino.coordinates[i].col += c;
    }
    return tetromino;
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
