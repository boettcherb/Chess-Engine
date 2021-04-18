
#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "defs.h"

/**
 * A structure to hold information about moves that were already made. The
 * board struct stores each move that was made to get to its current position
 * in an array called history[]. Moves are placed in the history[] array in
 * the makeMove() function. The undoMove() function accesses the last element
 * of the history[] array in order to undo the previous move.
 * 
 * move:              A 64-bit integer containing most of the necessary
 *                    information about the move.
 */
typedef struct {
    uint64 move;
} PreviousMove;

/*
 * In this engine, a chessboard is represented using bitboards. Each bitboard
 * is a 64-bit number where the least significant bit (bit 0) represents square
 * A1 and the most significant bit (bit 63) represents square H8. A bit is set
 * to '1' if there is a piece on that square.
 * 
 * pieceBitboards:    The bitboards for this chessboard. There are 12 of them,
 *                    one for each piece type.
 * colorBitboards:    3 bitboards to represent (1) all the white pieces, (2)
 *                    all the black pieces, and (3) all pieces of both colors.
 *                    These bitboards are used often and it is more efficient
 *                    to keep them updated with the piece bitboards as moves
 *                    are made then to calculate them when needed.
 * pieces:            An array of 64 chars to hold the piece type for each
 *                    square. This allows quick access of the piece type of a
 *                    given square and is also updated incrementally with the
 *                    piece bitboards.
 * sideToMove:        An integer that is either 0 (white) or 1 (black) denoting
 *                    whose turn it is in the current position.
 * ply:               An integer holding the number of half moves made to get
 *                    to the current board position.
 */
typedef struct {
    uint64 pieceBitboards[NUM_PIECE_TYPES];
    uint64 colorBitboards[3];
    signed char pieces[64];
    int sideToMove;
    int ply;
    PreviousMove history[MAX_GAME_MOVES];
} Board;

void resetBoard(Board* board);
int setBoardToFen(Board* board, const char* fen);

// functions only used in debug mode. NDEBUG is included with <assert.h>
#ifndef NDEBUG
    int checkBoard(const Board* board);
    void printPieces(const Board* board);
#endif

#endif
