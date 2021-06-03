#include "defs.h"
#include "board.h"
#include "movegen.h"
#include "hash.h"

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
    assert(board->positionKey == generatePositionKey(board));
    assert(board->sideToMove == WHITE || board->sideToMove == BLACK);
    assert(board->colorBitboards[WHITE] == getColorBitboard(board, WHITE));
    assert(board->colorBitboards[BLACK] == getColorBitboard(board, BLACK));
    assert(board->colorBitboards[BOTH_COLORS] ==
        getColorBitboard(board, BOTH_COLORS));
    assert(board->ply >= 0);
    // Loop through each piece in the board and count the number of each piece
    // type. Make sure the numbers of each piece type are valid. Also verify
    // that board->material[] is correct.
    int pieceCounts[NUM_PIECE_TYPES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int materialCounts[2] = { 0, 0 };
    for (int square = 0; square < 64; ++square) {
        if (board->pieces[square] != NO_PIECE) {
            int piece = board->pieces[square];
            materialCounts[pieceColor[piece]] += material[piece];
            assert(piece >= 0 && piece < NUM_PIECE_TYPES);
            ++pieceCounts[piece];
        }
    }
    assert(materialCounts[WHITE] == board->material[WHITE]);
    assert(materialCounts[BLACK] == board->material[BLACK]);
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
    // check to make sure the en passant square is valid
    if (board->enPassantSquare != 0ULL) {
        int sq = getLSB(board->enPassantSquare);
        assert(board->pieces[sq] == NO_PIECE);
        if (board->sideToMove == WHITE) {
            // en passant square was made by a black pawn
            assert(!(board->enPassantSquare & 0xFFFF00FFFFFFFFFF));
            assert(board->pieces[sq - 8] == BLACK_PAWN);
        } else {
            // en passant square was made by a white pawn
            assert(!(board->enPassantSquare & 0xFFFFFFFFFF00FFFF));
            assert(board->pieces[sq + 8] == WHITE_PAWN);
        }
    }
    // make sure that only the last 4 bits of the castlePerms int are used
    assert(!(board->castlePerms & 0xFFFFFFF0));
    return 1;
}

/* 
 * Check the given move to make sure it is valid. Each move has 5 parts: the
 * 'from' square, the 'to' square, the captured piece, the promoted piece, and
 * the move flags. Check for discrepencies such as the captured piece being a
 * king or the promotion flag and castle flag both being set. This is a
 * debugging function that should only be used inside assert statements.
 * 
 * move:              An integer containing all the information about
 *                    a single move.
 * 
 * return:            True if the move is valid. If the move is not valid, one
 *                    of the internal asserts should fail.
 */
int validMove(int move) {
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    int captured = (move >> 12) & 0xF;
    int promoted = (move >> 16) & 0xF;
    assert(move >> 25);
    if (move & CAPTURE_FLAG) {
        assert(!(move & (EN_PASSANT_FLAG | CASTLE_FLAG | PAWN_START_FLAG)));
        assert(captured >= WHITE_PAWN && captured <= BLACK_KING);
        assert(captured != WHITE_KING && captured != BLACK_KING);
    } else {
        assert(captured == 0xF);
    }
    if (move & PROMOTION_FLAG) {
        assert(!(move & (EN_PASSANT_FLAG | CASTLE_FLAG | PAWN_START_FLAG)));
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
    if (move & CASTLE_FLAG) {
        assert(!(move & (EN_PASSANT_FLAG | CAPTURE_FLAG)));
        assert(!(move & (PAWN_START_FLAG | PROMOTION_FLAG)));
        if (from == E1) {
            assert(to == G1 || to == C1);
        } else {
            assert(from == E8);
            assert(to == G8 || to == C8);
        }
    }
    if (move & EN_PASSANT_FLAG) {
        assert(!(move & (PAWN_START_FLAG | CAPTURE_FLAG)));
        assert(!(move & (CASTLE_FLAG | PROMOTION_FLAG)));
        if ((1ULL << from) & 0x00000000FF000000) {
            assert((1ULL << to) & 0x0000000000FF0000);
        } else {
            assert((1ULL << from) & 0x000000FF00000000);
            assert((1ULL << to) & 0x0000FF0000000000);
        }
    }
    if (move & PAWN_START_FLAG) {
        assert(!(move & (EN_PASSANT_FLAG | CAPTURE_FLAG)));
        assert(!(move & (CASTLE_FLAG | PROMOTION_FLAG)));
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
    const char* pieceChar = "PNBRQKpnbrqk";
    int rank = 8;
    for (int square = 56; square >= 0; square -= 8) {
        printf("%d  ", rank--);
        for (int fileIndex = 0; fileIndex < 8; ++fileIndex) {
            int piece = board->pieces[square + fileIndex];
            printf("%c ", piece == NO_PIECE ? '-' : pieceChar[piece]);
        }
        putchar('\n');
    }
    printf("\n  ");
    for (int i = 0; i < 8; ++i) {
        printf(" %c", (char) ('a' + i));
    }
    putchar('\n');
}

/*
 * Print the given bitboard to the console. Print an 'X' to mark a piece and
 * a '-' to mark an empty space. We want to print the board so that a1 is in
 * the bottom left and h8 is in the top right so that it is as readable as
 * possible.
 * 
 * bitboard:      A 64-bit bitboard to be printed to the console. 
 */
void printBitboard(uint64 bitboard) {
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 0; j < 8; ++j) {
            printf("%c ", (bitboard & (1ULL << (i + j))) ? 'X' : '-');
        }
        putchar('\n');
    }
}

/*
 * Print every piece of information relating to the given board (except the
 * history array). Print every piece bitboard, the color bitboards, the pieces
 * array, and every other member variable to the console so that the user can
 * see everything.
 *
 * board:     The board struct to be printed to the screen. Passed in as a
 *            pointer which must not be NULL.
 */
void printBoard(const Board* board) {
    assert(board != NULL);
    puts("============================================");
    printf("side to move: %c\n", board->sideToMove == WHITE ? 'w' : 'b');
    puts("pieces:");
    printPieces(board);
    puts("white pawns:");
    printBitboard(board->pieceBitboards[0]);
    puts("white knights:");
    printBitboard(board->pieceBitboards[1]);
    puts("white bishops:");
    printBitboard(board->pieceBitboards[2]);
    puts("white rooks:");
    printBitboard(board->pieceBitboards[3]);
    puts("white queens:");
    printBitboard(board->pieceBitboards[4]);
    puts("white king:");
    printBitboard(board->pieceBitboards[5]);
    puts("black pawns:");
    printBitboard(board->pieceBitboards[6]);
    puts("black knights:");
    printBitboard(board->pieceBitboards[7]);
    puts("black bishops:");
    printBitboard(board->pieceBitboards[8]);
    puts("black rooks:");
    printBitboard(board->pieceBitboards[9]);
    puts("black queens:");
    printBitboard(board->pieceBitboards[10]);
    puts("black king:");
    printBitboard(board->pieceBitboards[11]);
    puts("===================================");
    puts("White pieces:");
    printBitboard(board->colorBitboards[WHITE]);
    puts("Black pieces:");
    printBitboard(board->colorBitboards[BLACK]);
    puts("All pieces:");
    printBitboard(board->colorBitboards[BOTH_COLORS]);
    printf("castle permissions: ");
    putchar(board->castlePerms & CASTLE_WK ? 'K' : '-');
    putchar(board->castlePerms & CASTLE_WQ ? 'Q' : '-');
    putchar(board->castlePerms & CASTLE_BK ? 'k' : '-');
    putchar(board->castlePerms & CASTLE_BQ ? 'q' : '-');
    puts("\nen passant square:");
    printBitboard(board->enPassantSquare);
    //printf("fifty move count: %d\n", board->fiftyMoveCount);
    printf("ply: %d\n", board->ply);
    puts("============================================");
}

/*
 * Given a move on the chessboard, fill the 'moveString' parameter with the
 * text version of the move. For example, if the 'from' square is 8 (a2) and
 * the 'to' square is 24 (a4), the moveString will be "a2a4". Also, if the move
 * results in a promotion, append the letter of the promoted piece to the move
 * string. For example: "a7a8Q".
 * 
 * move:        A 32-bit integer with all the information of the move.
 * moveString:  A string where the text-version of the move will be placed. The
 *              string must be at least 6 chars long.
 */
void getMoveString(int move, char* moveString) {
    assert(validMove(move));
    const char* pieceChar = "PNBRQKpnbrqk\0\0\0\0\0";
    moveString[0] = (move & 0x3F) % 8 + 'a';
    moveString[1] = (move & 0x3F) / 8 + '1';
    moveString[2] = ((move >> 6) & 0x3F) % 8 + 'a';
    moveString[3] = ((move >> 6) & 0x3F) / 8 + '1';
    moveString[4] = pieceChar[(move >> 16) & 0xF];
    moveString[5] = 0;
}
