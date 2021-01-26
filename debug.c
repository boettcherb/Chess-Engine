#include "defs.h"
#include "board.h"
#include "movegen.h"

#include <string.h>
#include <stdio.h>

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
    // No pawns can be on the first or eight rank
    assert(!(board->pieceBitboards[WHITE_PAWN] & 0xFF000000000000FF));
    assert(!(board->pieceBitboards[BLACK_PAWN] & 0xFF000000000000FF));
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

/* Check the given move to make sure it is valid. Each move has 5 parts: the
 * 'from' square, the 'to' square, the captured piece, the promoted piece, and
 * the move flags. Check for discrepencies such as the captured piece being a
 * king or the promotion flag and castle flag both being set. This is a
 * debugging function that should only be used inside assert statements.
 * 
 * move:              A 64-bit integer containing all the information about
 *                    a single move.
 * 
 * return:            True if the move is valid. If the move is not valid, one
 *                    of the internal asserts should fail.
 */
int validMove(uint64 move) {
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    int captured = (move >> 12) & 0xF;
    int promoted = (move >> 16) & 0xF;
    int flags = (move >> 20) & 0x1F;
    assert(~MOVE_FLAGS & flags == 0);
    if (flags & CAPTURE_FLAG) {
        assert(!(flags & (EN_PASSANT_FLAG | CASTLE_FLAG | PAWN_START_FLAG)));
        assert(captured >= WHITE_PAWN && captured <= BLACK_KING);
        assert(captured != WHITE_KING && captured != BLACK_KING);
    } else {
        assert(captured == 0xF);
    }
    if (flags & PROMOTION_FLAG) {
        assert(!(flags & (EN_PASSANT_FLAG | CASTLE_FLAG | PAWN_START_FLAG)));
        if ((1ULL << from) & 0x00FF000000000000) {
            assert((1ULL << to) & 0xFF00000000000000);
        } else {
            assert((1ULL << from) & 0x000000000000FF00);
            assert((1ULL << to) & 0x00000000000000FF);
        }
        assert(promoted > WHITE_PAWN && promoted < BLACK_KING);
        assert(promoted != WHITE_KING && promoted != BLACK_PAWN);
    } else {
        assert(promoted == 0xF);
    }
    if (flags & CASTLE_FLAG) {
        assert(!(flags & (EN_PASSANT_FLAG | CAPTURE_FLAG)));
        assert(!(flags & (PAWN_START_FLAG | PROMOTION_FLAG)));
        if (from == E1) {
            assert(to == G1 || to == C1);
        } else {
            assert(from == E8);
            assert(to == G8 || to == C8);
        }
    }
    if (flags & EN_PASSANT_FLAG) {
        assert(!(flags & (PAWN_START_FLAG | CAPTURE_FLAG)));
        assert(!(flags & (CASTLE_FLAG | PROMOTION_FLAG)));
        if ((1ULL << from) & 0x00000000FF000000) {
            assert((1ULL << to) & 0x0000000000FF0000);
        } else {
            assert((1ULL << from) & 0x000000FF00000000);
            assert((1ULL << to) & 0x0000FF0000000000);
        }
    }
    if (flags & PAWN_START_FLAG) {
        assert(!(flags & (EN_PASSANT_FLAG | CAPTURE_FLAG)));
        assert(!(flags & (CASTLE_FLAG | PROMOTION_FLAG)));
        if ((1ULL << from) & 0x00FF000000000000) {
            assert((1ULL << to) & 0x000000FF00000000);
        } else {
            assert((1ULL << from) & 0x000000000000FF00);
            assert((1ULL << to) & 0x00000000FF000000);
        }
    }
    return 1;
}

/*
 * Print an 8x8 grid of pieces to the console. If there is not a piece on a
 * square, print '-'. The board will be printed from white's perspective (8th
 * rank on top, 1st rank on the bottom, A file on the left, and H file on the
 * right).
 * 
 * board:       The board whose pieces will be printed to the console. Passed
 *              in as a pointer which must not be NULL.
 */
void printPieces(const Board* board) {
    assert(board != NULL);
    const char pieceChar[NUM_PIECE_TYPES] = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'
    };
    for (int square = 56; square >= 0; square -= 8) {
        for (int fileIndex = 0; fileIndex < 8; ++fileIndex) {
            int piece = board->pieces[square + fileIndex];
            printf("%c ", piece == NO_PIECE ? '-' : pieceChar[piece]);
        }
        putchar('\n');
    }
}
