#include "board.h"
#include "defs.h"

#include <string.h>

/*
 * Clear the board and set all board variables to their default value. The
 * default value for board.pieces[64] is NO_PIECE (-1). board.sideToMove is set
 * to BOTH_COLORS to that an error will be thrown if it is not changed to
 * either WHITE or BLACK when the board is set up. The default value of
 * everything else is 0.
 * 
 * board:       The board to be cleared, passed in as a pointer. The pointer
 *              must not be NULL.
 */
void resetBoard(Board* board) {
    assert(board != NULL);
    memset(board, 0, sizeof(Board));
    memset(board->pieces, NO_PIECE, sizeof(char) * 64);
    board->sideToMove = BOTH_COLORS;
}
