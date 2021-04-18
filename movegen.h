#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "defs.h"
#include "board.h"

/******************************************************************************
Each move in a MoveList is a 64-bit integer with the following information:
0 0000 0000 0000 0000 0011 1111   6 bits for the 'from' square
0 0000 0000 0000 1111 1100 0000   6 bits for the 'to' square
0 0000 0000 1111 0000 0000 0000   4 bits for the captured piece
0 0000 1111 0000 0000 0000 0000   4 bits for the promoted piece
0 0001 0000 0000 0000 0000 0000   1 bit for the capture flag
0 0010 0000 0000 0000 0000 0000   1 bit for the promotion flag
0 0100 0000 0000 0000 0000 0000   1 bit for the castle flag
0 1000 0000 0000 0000 0000 0000   1 bit for the en passant flag
1 0000 0000 0000 0000 0000 0000   1 bit for the pawn start flag

The remaining 39 bits contain the move score. This score will be used with
the minimax / alpha-beta algorithm. A move will have a higher score if it is
likely to be a good move (Ex: captures, promotions, castling). Sorting moves
by their score will help the search algorithm run faster, as more pruning
can occur if the best moves are considered first.
******************************************************************************/

/*
 * Use a MoveList to store all of the moves that are generated by 
 * generateAllMoves. Each MoveList stores all the possible moves for a single
 * board position, including pseudo-legal moves (moves that leave the king in
 * check).
 * 
 * numMoves:     An integer storing the number of moves in the MoveList. The 
 *               number of moves in the list cannot exceed MAX_GAME_MOVES.
 * moves:        An array of moves. Each move contains multiple pieces of 
 *               information which is combined into 1 64-bit integer.
 */
typedef struct {
    int numMoves;
    uint64 moves[MAX_GAME_MOVES];
} MoveList;

void generateAllMoves(const Board* board, MoveList* list);

#ifndef NDEBUG
    int validMove(uint64 move);
#endif

#endif
