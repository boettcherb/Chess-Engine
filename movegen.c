#include "movegen.h"
#include "defs.h"
#include "board.h"

/*
 * Generate all legal and pseudo-legal moves for the given chess position and 
 * store them in the MoveList. Each move in chess moves a piece from one square
 * to another. A move could be a capture or a special move (castle, en passant, 
 * promotion, double pawn move). Each move has a 'from' square (the square that
 * the moved piece started on), the 'to' square, the captured piece (if there
 * is one), the promoted piece (the piece that a pawn promoted to, if
 * applicable), and 1-bit flags indicating if the move was a special move. Each
 * piece of information about the move is combined into 1 64-bit integer and
 * stored in the MoveList.
 * 
 * board:       The current chess position. Moves will be generated based on
 *              this position. Passed in as a pointer which must not be NULL.
 * list:        The MoveList which will store each move that is generated from
 *              the current board position. Passed in as a pointer which must
 *              not be NULL.
 */
void generateAllMoves(const Board* board, MoveList* list) {
    assert(board != NULL);
    assert(list != NULL);
    assert(checkBoard(board));
    list->numMoves = 0;
    /*
    TODO: implement these functions
    generateKingMoves(board, list);
    generateKnightMoves(board, list);
    generateRookMoves(board, list);
    generateBishopMoves(board, list);
    generateQueenMoves(board, list);
    generatePawnMoves(board, list);
    */
}
