#include "defs.h"
#include "board.h"

#include <string.h>

/* 
 * Calculate a board's color bitboards from its piece bitboards. If color is
 * WHITE, combine all the white piece bitboards. If color is BLACK, combine all
 * the black piece bitboards. If color is BOTH_COLORS, combine all the piece
 * bitboards of both colors. Note that we are not just returning
 * board.colorBitboard[color]. Instead we are calculating the color bitboard
 * from the piece bitboards for debugging purposes to make sure they match.
 * This function is only used in checkBoard(), so it is marked static.
 * 
 * board:    The board whose piece bitboards will be combined. Passed as a
 *           pointer which must not be NULL.
 * color:    The color of the color bitboard we want to calculate. Must be
 *           either WHITE (0), BLACK (1), or BOTH_COLORS (2).
 * 
 * return:   The calculated color bitboard as an unsigned long long. This value
 *           will be compared to board.colorBitboard[color] to make sure they
 *           are equal.
 */
static uint64 getColorBitboard(const Board* board, int color) {
    assert(board != NULL);
    assert(color == WHITE || color == BLACK || color == BOTH_COLORS);
    uint64 colorBitboard = 0ULL;
    int firstPiece = color == BLACK ? BLACK_PAWN : WHITE_PAWN;
    int lastPiece = color == WHITE ? WHITE_KING : BLACK_KING;
    for (int piece = firstPiece; piece <= lastPiece; ++piece) {
        colorBitboard |= board->pieceBitboards[piece];
    }
    return colorBitboard;
}

/*
 * Check the board to make sure that it is a valid chess position. The piece
 * bitboards should match the color bitboards, the bitboards should match the
 * piece array, both sides should have a king, etc. Each variable in the board
 * struct is updated incrementally as moves are made and unmade, and this
 * function assures that nothing is missed. This function should only be used 
 * with asserts for debugging. Ex: assert(checkBoard(board));
 * 
 * board:    The board to be checked to make sure all its variables are in sync
 *           and that it is a valid position. Passed in as a pointer that must
 *           not be NULL.
 * 
 * return:   Always return 1 (true). If something is wrong with the board, an 
 *           assertion will fail and the program will terminate.
 */
int checkBoard(const Board* board) {
    assert(board != NULL);
    assert(board->sideToMove == WHITE || board->sideToMove == BLACK);
    assert(board->colorBitboards[WHITE] == getColorBitboard(board, WHITE));
    assert(board->colorBitboards[BLACK] == getColorBitboard(board, BLACK));
    assert(board->colorBitboards[BOTH_COLORS] ==
        getColorBitboard(board, BOTH_COLORS));
    assert(board->ply >= 0);
    // Loop through each piece in the board and count the number of each piece
    // type. Make sure the numbers of each piece type are valid.
    int pieceCounts[NUM_PIECE_TYPES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int square = 0; square < 64; ++square) {
        if (board->pieces[square] != NO_PIECE) {
            int piece = board->pieces[square];
            assert(piece >= 0 && piece < NUM_PIECE_TYPES);
            ++pieceCounts[piece];
        }
    }
    assert(pieceCounts[WHITE_KING] == 1 && pieceCounts[BLACK_KING] == 1);
    assert(pieceCounts[WHITE_PAWN] <= 8 && pieceCounts[BLACK_PAWN] <= 8);
    assert(pieceCounts[WHITE_KNIGHT] + pieceCounts[WHITE_PAWN] <= 10);
    assert(pieceCounts[WHITE_BISHOP] + pieceCounts[WHITE_PAWN] <= 10);
    assert(pieceCounts[WHITE_ROOK] + pieceCounts[WHITE_PAWN] <= 10);
    assert(pieceCounts[WHITE_QUEEN] + pieceCounts[WHITE_PAWN] <= 9);
    assert(pieceCounts[BLACK_KNIGHT] + pieceCounts[BLACK_PAWN] <= 10);
    assert(pieceCounts[BLACK_BISHOP] + pieceCounts[BLACK_PAWN] <= 10);
    assert(pieceCounts[BLACK_ROOK] + pieceCounts[BLACK_PAWN] <= 10);
    assert(pieceCounts[BLACK_QUEEN] + pieceCounts[BLACK_PAWN] <= 9);
    // Create a copy of the board to manipulate (the original is const). Then,
    // for each piece in the board, remove it from both the piece and color
    // bitboards. Check to make sure they still match using getColorBitboard().
    // This will ensure that there is only 1 piece on each square.
    Board boardCopy;
    memcpy(&boardCopy, board, sizeof(Board));
    for (int square = 0; square < 64; ++square) {
        int piece = boardCopy.pieces[square];
        if (piece != NO_PIECE) {
            assert(piece >= 0 && piece < NUM_PIECE_TYPES);
            int color = pieceColor[piece];
            boardCopy.pieceBitboards[piece] &= ~(1ULL << square);
            boardCopy.colorBitboards[color] &= ~(1ULL << square);
            boardCopy.colorBitboards[BOTH_COLORS] &= ~(1ULL << square);
            assert(getColorBitboard(&boardCopy, color) == 
                boardCopy.colorBitboards[color]);
            assert(getColorBitboard(&boardCopy, BOTH_COLORS) == 
                boardCopy.colorBitboards[BOTH_COLORS]);
            boardCopy.pieceBitboards[piece] |= (1ULL << square);
            boardCopy.colorBitboards[color] |= (1ULL << square);
            boardCopy.colorBitboards[BOTH_COLORS] |= (1ULL << square);
        }
    }
    return 1;
}
