#include "movegen.h"
#include "defs.h"
#include "board.h"
#include "attack.h"

/* 
 * Assemble all the parts of a move into a single 32-bit integer. See movegen.h
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
 * return:      A 32-bit integer containing all the parts of the move.
 */
static int getMove(int from, int to, int captured, int promoted, int flags) {
    assert(from >= 0 && from < 64 && to >= 0 && to < 64);
    assert((flags & ~MOVE_FLAGS) == 0);
    int move = from | (to << 6) | ((captured & 0xF) << 12);
    return move | ((promoted & 0xF) << 16) | flags;
}

/* 
 * Add the given move to the movelist.
 *
 * move:      the move to be added to the list.
 * list:      a struct which contains an array of moves
 */
static void addMove(int move, MoveList* list) {
    assert(list->numMoves >= 0);
    assert(validMove(move));
    list->moves[list->numMoves++] = move;
}

/*
 * An alternative function to addMove() that is used exclusively to add pawn
 * moves. White pawn moves that go from the 7th rank to the 8th rank, and black
 * pawn moves that go from the 2nd rank to the 1st rank are promotion moves. A
 * promotion could turn a pawn into a rook, bishop, queen, or knight. This
 * function checks to see if the move is a promotion move, and if so, adds all
 * 4 moves to the MoveList.
 * 
 * board:       The current chess position, passed in as a pointer.
 * list:        The list of moves that is being generated. All pawn moves from
 *              this function will be added to the MoveList.
 * from:        The square the moving piece started on. Must be in the
 *              range [0-64).
 * to:          The square the moving piece is moving to. Must be in the
 *              range [0-64).
 * captured:    If the move is a capture move, record what type of piece was
 *              captured. NO_PIECE if not a capture move.
 * flags:       5 bits indicating if this move is a 'special' move (castle, en
 *              passant, promotion, capture, pawn start).
 */
static void addPawnMove(const Board* board, MoveList* list, int from, int to,
int captured, int flags) {
    int move = getMove(from, to, captured, NO_PIECE, flags);
    if ((1ULL << to) & 0xFF000000000000FF) {
        move = (move & 0xFFFFFFFFFFF0FFFF) | PROMOTION_FLAG;
        addMove(move | (pieces[board->sideToMove][KNIGHT] << 16), list);
        addMove(move | (pieces[board->sideToMove][BISHOP] << 16), list);
        addMove(move | (pieces[board->sideToMove][ROOK] << 16), list);
        addMove(move | (pieces[board->sideToMove][QUEEN] << 16), list);
    } else {
        addMove(move, list);
    }
}

/*
 * Generate pawn moves from the given board. Pawn move generation is separated
 * from the generateAllMoves() function because pawns move uniquely from the
 * other pieces. There are many special rules regarding pawns and all pawn
 * moves for one side (regular moves and attacks) can be generated using just a
 * few shift operations. Pawn move generation is separated into 2 functions
 * because there are many small differences when generating white pawn moves vs
 * black pawn moves.
 * 
 * board:       The current chess position which must be a valid position.
 *              Passed in as a pointer.
 * list:        The MoveList which will store each move that is generated from
 *              the current board position. Passed in as a pointer.
 */
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

/* 
 * Given the starting position of a piece and its attack bitboard, generate all
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
 * Generate all castle moves for the current position. Castle move generation
 * is split into 2 functions: 1 for the white pieces and 1 for the black
 * pieces. These function first check the castle flags. If castling is allowed,
 * they check to see if there are any pieces in between the king and rook, if
 * the king is in check, or if the king will pass through check or end up in
 * check. If all of these conditions pass, castling moves are added to the
 * move list. 
 * 
 * board:      The current chess position. Passed in as a pointer which must
 *             not be NULL.
 * list:       The list of moves for the current chess position. Passed in as
 *             a pointer which must not be NULL.
 */
static void generateWhiteCastleMoves(const Board* board, MoveList* list) {
    if (board->castlePerms & CASTLE_WK) {
        if (!(board->colorBitboards[BOTH_COLORS] & 0x0000000000000060ULL) &&
        !squareAttacked(board, 0x0000000000000070ULL, BLACK)) {
            addMove(getMove(E1, G1, NO_PIECE, NO_PIECE, CASTLE_FLAG), list);    
        }
    }
    if (board->castlePerms & CASTLE_WQ) {
        if (!(board->colorBitboards[BOTH_COLORS] & 0x000000000000000EULL) &&
        !squareAttacked(board, 0x000000000000001CULL, BLACK)) {
            addMove(getMove(E1, C1, NO_PIECE, NO_PIECE, CASTLE_FLAG), list);
        }
    }
}
static void generateBlackCastleMoves(const Board* board, MoveList* list) {
    if (board->castlePerms & CASTLE_BK) {
        if (!(board->colorBitboards[BOTH_COLORS] & 0x6000000000000000ULL) &&
        !squareAttacked(board, 0x7000000000000000ULL, WHITE)) {
            addMove(getMove(E8, G8, NO_PIECE, NO_PIECE, CASTLE_FLAG), list);
        }
    }
    if (board->castlePerms & CASTLE_BQ) {
        if (!(board->colorBitboards[BOTH_COLORS] & 0x0E00000000000000ULL) &&
        !squareAttacked(board, 0x1C00000000000000ULL, WHITE)) {
            addMove(getMove(E8, C8, NO_PIECE, NO_PIECE, CASTLE_FLAG), list);
        }
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
        generateWhiteCastleMoves(board, list);
        generateWhitePawnMoves(board, list);
        knights = board->pieceBitboards[WHITE_KNIGHT];
        bishops = board->pieceBitboards[WHITE_BISHOP];
        rooks = board->pieceBitboards[WHITE_ROOK];
        queens = board->pieceBitboards[WHITE_QUEEN];
        kings = board->pieceBitboards[WHITE_KING];
        samePieces = board->colorBitboards[WHITE];
    } else {
        generateBlackCastleMoves(board, list);
        generateBlackPawnMoves(board, list);
        knights = board->pieceBitboards[BLACK_KNIGHT];
        bishops = board->pieceBitboards[BLACK_BISHOP];
        rooks = board->pieceBitboards[BLACK_ROOK];
        queens = board->pieceBitboards[BLACK_QUEEN];
        kings = board->pieceBitboards[BLACK_KING];
        samePieces = board->colorBitboards[BLACK];
    }
    while (knights) {
        int knight = getLSB(knights);
        uint64 attacks = getKnightAttacks(knight);
        generatePieceMoves(board, list, attacks & ~samePieces, knight);
        knights &= knights - 1;
    }
    while (bishops) {
        int bishop = getLSB(bishops);
        uint64 attacks = getBishopAttacks(bishop, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, bishop);
        bishops &= bishops - 1;
    }
    while (rooks) {
        int rook = getLSB(rooks);
        uint64 attacks = getRookAttacks(rook, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, rook);
        rooks &= rooks - 1;
    }
    while (queens) {
        int queen = getLSB(queens);
        uint64 attacks = getQueenAttacks(queen, allPieces);
        generatePieceMoves(board, list, attacks & ~samePieces, queen);
        queens &= queens - 1;
    }
    uint64 attacks = getKingAttacks(kings);
    generatePieceMoves(board, list, attacks & ~samePieces, getLSB(kings));
}
