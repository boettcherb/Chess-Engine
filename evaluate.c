#include "defs.h"

/*
 * pieceValue[piece][sq] gives an estimate as to how valuable piece will be
 * when placed on the square sq. This is used for the evaluatePosition() 
 * function to favor moves that are likely to be improving moves. For example,
 * a pawn on E7 has a higher value than a pawn on E5, and a knight in the
 * center has a higher value than a knight on the edge. This will encourage the
 * engine to make improving moves such as bringing the knights to the center
 * and pushing pawns. Each value is in 100ths of a pawn, so a value of 100 is
 * worth 1 pawn.
 */
static const int pieceValue[NUM_PIECE_TYPES][64] = {
    { // white pawn
         0,  0,  0,   0,   0,  0,  0,  0,
        10, 10,  0, -10, -10,  0, 10, 10,
         5,  0,  0,   5,   5,  0,  0,  5,
         0,  0, 10,  20,  20, 10,  5,  5,
        10, 10, 20,  30,  30, 20, 10, 10,
        30, 30, 30,  40,  40, 30, 30, 30,
        50, 50, 50,  50,  50, 50, 50, 50,
         0,  0,  0,   0,   0,  0,  0,  0,
    },
    { // white knight
        -10, -10,  0,  0,  0,  0, -10, -10,
          0,   0,  0,  5,  5,  0,   0,   0,
          0,   0, 10, 10, 10, 10,   0,   0,
          0,   5, 10, 20, 20, 10,   5,   0,
          5,  10, 15, 20, 20, 15,  10,   5,
          5,  10, 10, 20, 20, 10,  10,   5,
          0,   0,  5, 10, 10,  5,   0,   0,
        -10,   0,  0,  0,  0,  0,   0, -10,
    },
    { // white bishop
        -20,  0, -10,  0,  0, -10,  0, -20,
          0,  0,   0, 10, 10,   0,  0,   0,
          0,  0,  10, 15, 15,  10,  0,   0,
          0, 10,  15, 20, 20,  15, 10,   0,
          0, 10,  15, 20, 20,  15, 10,   0,
          0,  0,  10, 15, 15,  10,  0,   0,
          0,  0,   0, 10, 10,   0,  0,   0,
        -20,  0,   0,  0,  0,   0,  0, -20,
    },
    { // white rook
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
        25, 25, 25, 25, 25, 25, 25, 25,
         0,  0,  5, 10, 10,  5,  0,  0,
    },
    { // white queen
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    },
    { // white king
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    },
    { // black pawn
         0,  0,  0,   0,   0,  0,  0,  0,
        20, 20, 20,  30,  30, 20, 20, 20,
        10, 10, 10,  20,  20, 10, 10, 10,
         5,  5,  5,  10,  10,  5,  5,  5,
         0,  0, 10,  20,  20, 10,  0,  0,
         5,  0,  0,   5,   5,  0,  0,  5,
        10, 10,  0, -10, -10,  0, 10, 10,
         0,  0,  0,   0,   0,  0,  0,  0,
    },
    { // black knight
        0,   0,  0,  0,  0,  0,   0, 0,
        0,   0,  5, 10, 10,  5,   0, 0,
        5,  10, 10, 20, 20, 10,  10, 5,
        5,  10, 15, 20, 20, 15,  10, 5,
        0,   5, 10, 20, 20, 10,   5, 0,
        0,   0, 10, 10, 10, 10,   0, 0,
        0,   0,  0,  5,  5,  0,   0, 0,
        0, -10,  0,  0,  0,  0, -10, 0,
    },
    { // black bishop
        -20,  0,   0,  0,  0,   0,  0, -20,
          0,  0,   0, 10, 10,   0,  0,   0,
          0,  0,  10, 15, 15,  10,  0,   0,
          0, 10,  15, 20, 20,  15, 10,   0,
          0, 10,  15, 20, 20,  15, 10,   0,
          0,  0,  10, 15, 15,  10,  0,   0,
          0,  0,   0, 10, 10,   0,  0,   0,
        -20,  0, -10,  0,  0, -10,  0, -20,
    },
    { // black rook
         0,  0,  5, 10, 10,  5,  0,  0,
        25, 25, 25, 25, 25, 25, 25, 25,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
         0,  0,  5, 10, 10,  5,  0,  0,
    },
    { // black queen
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    },
    { // black king
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    },
};

/*
 * Return an evaluation of a board position from the perspective of whose turn
 * it is. If it is white's turn and white is winning, return a positive score.
 * If it is black's turn and black is winning, return a positive score. If the
 * side to move is losing, return a negative score. Each evaluation is an
 * integer value that is an estimate of how well the side to move is doing.
 * This value is based on the material for each side and how the pieces are
 * placed. The values are in 100ths of a pawn, so an evaluation of around 200
 * means the side to move is winning by about 2 pawns. Evaluations are used by
 * the alphaBeta algorithm to determine which move to make in the current
 * position.
 * 
 * board:   The board position that is being evaluated.
 * 
 * return:  An integer representing the evaluation of the position
 */
int evaluatePosition(const Board* board) {
    int eval = (board->material[WHITE] - board->material[BLACK]) * 100;
    for (int piece = 0; piece < NUM_PIECE_TYPES; ++piece) {
        uint64 bitboard = board->pieceBitboards[piece];
        while (bitboard) {
            int sq = getLSB(bitboard);
            if (pieceColor[piece] == WHITE) {
                eval += pieceValue[piece][sq];
            } else {
                eval -= pieceValue[piece][sq];
            }
            bitboard &= bitboard - 1;
        }
    }
    return board->sideToMove == WHITE ? eval : -eval;
}
