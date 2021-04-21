#include "movegen.h"
#include "defs.h"
#include "board.h"
#include "attack.h"

/* Assemble all the parts of a move into a single 64-bit integer. See movegen.h
 * for the layout of a single move.
 *
 * from:        The square the moving piece started on. Must be in the
 *              range [0-64).
 * to:          The square the moving piece is moving to. Must be in the
 *              range [0-64).
 * captured:    If the move is a capture move, record what type of piece was
 *              captured. NO_PIECE if not a capture move.
 * promoted:    If the move is a pawn promotion, record what type of piece the
 *              pawn is promoting to. NO_PIECE if not a promotion.
 * flags:       5 bits indicating if this move is a 'special' move (castle, en
 *              passant, promotion, capture, pawn start).
 * 
 * return:      A 64-bit integer containing all the parts of the move.
 */
static uint64 getMove(int from, int to, int captured, int promoted, int flags) {
    assert(from >= 0 && from < 64 && to >= 0 && to < 64);
    assert((flags & ~MOVE_FLAGS) == 0);
    uint64 move = from | (to << 6) | ((captured & 0xF) << 12);
    return move | ((promoted & 0xF) << 16) | flags;
}

/* Add the given move to the movelist.
 *
 * move:      the move to be added to the list.
 * list:      a struct which contains an array of moves
 */
static void addMove(int move, MoveList* list) {
    assert(list->numMoves >= 0);
    assert(validMove(move));
    list->moves[list->numMoves++] = move;
}

static void addPawnMove(const Board* board, MoveList* list, int from, int to,
int captured, int flags) {
    if ((1ULL << to) & 0xFF000000000000FF) {
        int side = board->sideToMove;
        flags |= PROMOTION_FLAG;
        addMove(getMove(from, to, captured, pieces[side][KNIGHT], flags), list);
        addMove(getMove(from, to, captured, pieces[side][BISHOP], flags), list);
        addMove(getMove(from, to, captured, pieces[side][ROOK], flags), list);
        addMove(getMove(from, to, captured, pieces[side][QUEEN], flags), list);
    } else {
        addMove(getMove(from, to, captured, NO_PIECE, flags), list);
    }
}

static void generateWhitePawnMoves(const Board* board, MoveList* list) {
    uint64 pawns = board->pieceBitboards[WHITE_PAWN];
    uint64 allPieces = board->colorBitboards[BOTH_COLORS];
    uint64 opponentPieces = board->colorBitboards[BLACK];
    uint64 pawnMoves = (pawns << 8) & ~allPieces;
    uint64 pawnStarts = ((pawnMoves & 0x0000000000FF0000) << 8) & ~allPieces;
    uint64 leftAttacks = getWhitePawnAttacksLeft(pawns) & opponentPieces;
    uint64 rightAttacks = getWhitePawnAttacksRight(pawns) & opponentPieces;
    while (pawnMoves) {
        int to = getLSB(pawnMoves);
        addPawnMove(board, list, to - 8, to, NO_PIECE, 0);
        pawnMoves &= pawnMoves - 1;
    }
    while (pawnStarts) {
        int to = getLSB(pawnStarts);
        addMove(getMove(to - 16, to, NO_PIECE, NO_PIECE, PAWN_START_FLAG), list);
        pawnStarts &= pawnStarts - 1;
    }
    while (leftAttacks) {
        int to = getLSB(leftAttacks);
        addPawnMove(board, list, to - 7, to, board->pieces[to], CAPTURE_FLAG);
        leftAttacks &= leftAttacks - 1;
    }
    while (rightAttacks) {
        int to  = getLSB(rightAttacks);
        addPawnMove(board, list, to - 9, to, board->pieces[to], CAPTURE_FLAG);
        rightAttacks &= rightAttacks - 1;
    }
    if (board->enPassantSquare != 0ULL) {
        int square = getLSB(board->enPassantSquare);
        if (square != 47 && board->pieces[square - 7] == WHITE_PAWN) {
            addMove(getMove(square - 7, square, NO_PIECE,
                NO_PIECE, EN_PASSANT_FLAG), list);
        }
        if (square != 40 && board->pieces[square - 9] == WHITE_PAWN) {
            addMove(getMove(square - 9, square, NO_PIECE,
                NO_PIECE, EN_PASSANT_FLAG), list);
        }
    }
}

static void generateBlackPawnMoves(const Board* board, MoveList* list) {
    uint64 pawns = board->pieceBitboards[BLACK_PAWN];
    uint64 allPieces = board->colorBitboards[BOTH_COLORS];
    uint64 opponentPieces = board->colorBitboards[WHITE];
    uint64 pawnMoves = (pawns >> 8) & ~allPieces;
    uint64 pawnStarts = ((pawnMoves & 0x0000FF0000000000) >> 8) & ~allPieces;
    uint64 leftAttacks = getBlackPawnAttacksLeft(pawns) & opponentPieces;
    uint64 rightAttacks = getBlackPawnAttacksRight(pawns) & opponentPieces;
    while (pawnMoves) {
        int to = getLSB(pawnMoves);
        addPawnMove(board, list, to + 8, to, NO_PIECE, 0);
        pawnMoves &= pawnMoves - 1;
    }
    while (pawnStarts) {
        int to = getLSB(pawnStarts);
        addMove(getMove(to + 16, to, NO_PIECE, NO_PIECE, PAWN_START_FLAG), list);
        pawnStarts &= pawnStarts - 1;
    }
    while (leftAttacks) {
        int to = getLSB(leftAttacks);
        addPawnMove(board, list, to + 7, to, board->pieces[to], CAPTURE_FLAG);
        leftAttacks &= leftAttacks - 1;
    }
    while (rightAttacks) {
        int to  = getLSB(rightAttacks);
        addPawnMove(board, list, to + 9, to, board->pieces[to], CAPTURE_FLAG);
        rightAttacks &= rightAttacks - 1;
    }
    if (board->enPassantSquare != 0ULL) {
        int square = getLSB(board->enPassantSquare);
        if (square != 16 && board->pieces[square + 7] == BLACK_PAWN) {
            addMove(getMove(square + 7, square, NO_PIECE,
                NO_PIECE, EN_PASSANT_FLAG), list);
        }
        if (square != 23 && board->pieces[square + 9] == BLACK_PAWN) {
            addMove(getMove(square + 9, square, NO_PIECE,
                NO_PIECE, EN_PASSANT_FLAG), list);
        }
    }
}

/* Given the starting position of a piece and its attack bitboard, generate all
 * possible moves for that piece and add them to the movelist. This function
 * can be used for all pieces except pawns (pawns have multiple special moves
 * and flags to handle).
 *
 * board:      The current chess position. Passed in as a pointer which must
 *             not be NULL.
 * list:       The list of moves for the current chess position. Passed in as
 *             a pointer which must not be NULL.
 * attacks:    The attack bitboard for the piece on the 'from' square.
 * from:       The starting square of the piece whose moves we are generating.
 */
static void generatePieceMoves(const Board* board, MoveList* list,
uint64 attacks, int from) {
    while (attacks) {
        int to = getLSB(attacks);
        int flag = board->pieces[to] == NO_PIECE ? 0 : CAPTURE_FLAG;
        addMove(getMove(from, to, board->pieces[to], NO_PIECE, flag), list);
        attacks &= attacks - 1;
    }
}

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
    assert(board != NULL && list != NULL && checkBoard(board));
    list->numMoves = 0;
    uint64 samePieces, allPieces = board->colorBitboards[BOTH_COLORS];
    uint64 kings, knights, rooks, bishops, queens;
    if (board->sideToMove == WHITE) {
        generateWhitePawnMoves(board, list);
        kings = board->pieceBitboards[WHITE_KING];
        knights = board->pieceBitboards[WHITE_KNIGHT];
        rooks = board->pieceBitboards[WHITE_ROOK];
        bishops = board->pieceBitboards[WHITE_BISHOP];
        queens = board->pieceBitboards[WHITE_QUEEN];
        samePieces = board->colorBitboards[WHITE];
    } else {
        generateBlackPawnMoves(board, list);
        kings = board->pieceBitboards[BLACK_KING];
        knights = board->pieceBitboards[BLACK_KNIGHT];
        rooks = board->pieceBitboards[BLACK_ROOK];
        bishops = board->pieceBitboards[BLACK_BISHOP];
        queens = board->pieceBitboards[BLACK_QUEEN];
        samePieces = board->colorBitboards[BLACK];
    }
    uint64 attacks = getKingAttacks(kings);
    generatePieceMoves(board, list, attacks & ~samePieces, getLSB(kings));
    while (knights) {
        int knight = getLSB(knights);
        uint64 attacks = getKnightAttacks(knight);
        generatePieceMoves(board, list, attacks & ~samePieces, knight);
        knights &= knights - 1;
    }
    while (rooks) {
        int rook = getLSB(rooks);
        uint64 attacks = getRookAttacks(rook, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, rook);
        rooks &= rooks - 1;
    }
    while (bishops) {
        int bishop = getLSB(bishops);
        uint64 attacks = getBishopAttacks(bishop, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, bishop);
        bishops &= bishops - 1;
    }
    while (queens) {
        int queen = getLSB(queens);
        uint64 attacks = getQueenAttacks(queen, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, queen);
        queens &= queens - 1;
    }
}
