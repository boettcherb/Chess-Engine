#include "search.h"
#include "defs.h"
#include "board.h"

/*
 * Determine if the current state of the board is a repetition of a previous
 * state. Check this by comparing position keys at different stages of the
 * game. In the board's history array, we store the position key after each
 * move, so it is easy to compare the current board's position with its
 * previous positions.
 * 
 * board:      The current state of the board, passed in as a const pointer.
 * 
 * return:     1 if the current board state is a repetition, 0 otherwise.
 */
int isRepetition(const Board* board) {
    assert(checkBoard(board));
    int start = board->ply - 2;
    int end = board->ply - board->fiftyMoveCount;
    for (int i = start; i >= end; i -= 2) {
        assert(i >= 0);
        if (board->positionKey == board->history[i].positionKey) {
            // TODO: verify repetition
            return 1;
        }
    }
    return 0;
}
