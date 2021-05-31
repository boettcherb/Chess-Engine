#include "movegen.h"
#include "defs.h"
#include "board.h"
#include "attack.h"

#include "stdlib.h"

/*
 * These arrays are used to find the move score of a particular move. The move
 * score is used to rank the moves genereted by generatedAllMoves() based on 
 * how likely they are to be good moves. Promotions and captures will have a 
 * higher score than normal moves. This is used to speed up the alpha-beta
 * algorithm, since more pruning can occur if the best moves are considered
 * first. Note that move evaluations are different from static board
 * evaluations, which determine which move is actually chosen by the alpha-beta
 * algorithm. Move evalutions only determine the order in which moves are
 * considered, not which moves are chosen.
 * 
 * Ex: A pawn move to capture a queen will be considered before a knight move
 * to capture a pawn:
 *     captureScore[WHITE_PAWN][BLACK_QUEEN] = 59
 *     captureScore[WHITE_KNIGHT][BLACK_PAWN] = 37
 * Ex: pawn moves are considered before king moves:
 *     moveScore[WHITE_PAWN] = 27, moveScore[WHITE_KING] = 22
 * Ex: queen promotions are considered before rook promotions: 
 *     promotionScore[WHITE_QUEEN] = 60, promotionScore[WHITE_ROOK] = 19
 * 
 * moveScore and promotionScore have NUM_PIECE_TYPES + 4 elements to avoid
 * a warning for array index out of bounds.
 */
int captureScore[NUM_PIECE_TYPES][NUM_PIECE_TYPES] = {
    { 0, 0, 0, 0, 0, 0, 43, 52, 53, 56, 59, 0 },
    { 0, 0, 0, 0, 0, 0, 37, 46, 47, 51, 58, 0 },
    { 0, 0, 0, 0, 0, 0, 36, 44, 45, 50, 57, 0 },
    { 0, 0, 0, 0, 0, 0, 35, 41, 42, 48, 55, 0 },
    { 0, 0, 0, 0, 0, 0, 34, 38, 39, 40, 49, 0 },
    { 0, 0, 0, 0, 0, 0, 30, 31, 32, 33, 54, 0 },
    { 43, 52, 53, 56, 59, 0, 0, 0, 0, 0, 0, 0 },
    { 37, 46, 47, 51, 58, 0, 0, 0, 0, 0, 0, 0 },
    { 36, 44, 45, 50, 57, 0, 0, 0, 0, 0, 0, 0 },
    { 35, 41, 42, 48, 55, 0, 0, 0, 0, 0, 0, 0 },
    { 34, 38, 39, 40, 49, 0, 0, 0, 0, 0, 0, 0 },
    { 30, 31, 32, 33, 54, 0, 0, 0, 0, 0, 0, 0 },
};
int moveScore[NUM_PIECE_TYPES + 4] = {
    27, 26, 25, 24, 23, 22, 27, 26, 25, 24, 23, 22,
};
int promotionScore[NUM_PIECE_TYPES + 4] = {
    0, 19, 18, 20, 60, 0, 0, 19, 18, 20, 60, 0,
};

/*
 * A comparator function used by qsort at the end of generateAllMoves() to
 * sort the moves in the move list by their score. The final operation is
 * "score of first move minus score of second move" because we want to sort
 * the moves in descending order.
 * 
 * m1:      The first move to be compared. Passed in as a const void pointer.
 * m2:      The second move to be compared. Passed in as a const void pointer.
 * 
 * return:  The difference between the two move scores, as an integer. If the
 *          difference is positive, the second move has a higher score and
 *          will be placed before the first move in the sorted list, and vice
 *          versa.
 */
int compareMoves(const void* m1, const void* m2) {
    int move1 = *((int*) m1);
    int move2 = *((int*) m2);
    return (move2 >> 25) - (move1 >> 25);
}

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
 * Add the given move to the movelist. Update the promotion if the move is a
 * pawn start, castle, en passant, or promotion.
 *
 * move:      the move to be added to the list.
 * list:      a struct which contains an array of moves
 */
static void addMove(int move, MoveList* list) {
    assert(list->numMoves >= 0);
    switch (move & MOVE_FLAGS) {
        case PAWN_START_FLAG: move |= (28 << 25); break;
        case CASTLE_FLAG:     move |= (29 << 25); break;
        case EN_PASSANT_FLAG: move |= (43 << 25); break;
        case PROMOTION_FLAG:

            move &= 0x01FFFFFF;
            move += promotionScore[(move >> 16) & 0xF] << 25;
            break;
        case CAPTURE_AND_PROMOTION_FLAG:
            move &= 0x01FFFFFF;
            move += (promotionScore[(move >> 16) & 0xF] + 1) << 25;
            
    }
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
int captured, int score) {
    int flags = captured == NO_PIECE ? 0 : CAPTURE_FLAG;
    int move = getMove(from, to, captured, NO_PIECE, flags) | (score << 25);
    if ((1ULL << to) & 0xFF000000000000FF) {
        move = (move & 0xFFF0FFFF) | PROMOTION_FLAG;
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
        addPawnMove(board, list, to - 8, to, NO_PIECE, moveScore[WHITE_PAWN]);
        pawnMoves &= pawnMoves - 1;
    }
    while (pawnStarts) {
        int to = getLSB(pawnStarts);
        addMove(getMove(to - 16, to, NO_PIECE, NO_PIECE, PAWN_START_FLAG), list);
        pawnStarts &= pawnStarts - 1;
    }
    while (leftAttacks) {
        int to = getLSB(leftAttacks);
        int score = captureScore[WHITE_PAWN][board->pieces[to]];
        addPawnMove(board, list, to - 7, to, board->pieces[to], score);
        leftAttacks &= leftAttacks - 1;
    }
    while (rightAttacks) {
        int to  = getLSB(rightAttacks);
        int score = captureScore[WHITE_PAWN][board->pieces[to]];
        addPawnMove(board, list, to - 9, to, board->pieces[to], score);
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
        addPawnMove(board, list, to + 8, to, NO_PIECE, moveScore[BLACK_PAWN]);
        pawnMoves &= pawnMoves - 1;
    }
    while (pawnStarts) {
        int to = getLSB(pawnStarts);
        addMove(getMove(to + 16, to, NO_PIECE, NO_PIECE, PAWN_START_FLAG), list);
        pawnStarts &= pawnStarts - 1;
    }
    while (leftAttacks) {
        int to = getLSB(leftAttacks);
        int score = captureScore[BLACK_PAWN][board->pieces[to]];
        addPawnMove(board, list, to + 7, to, board->pieces[to], score);
        leftAttacks &= leftAttacks - 1;
    }
    while (rightAttacks) {
        int to  = getLSB(rightAttacks);
        int score = captureScore[BLACK_PAWN][board->pieces[to]];
        addPawnMove(board, list, to + 9, to, board->pieces[to], score);
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
    int piece = board->pieces[from];
    while (attacks) {
        int to = getLSB(attacks);
        int flag, score;
        if (board->pieces[to] == NO_PIECE) {
            flag = 0;
            score = moveScore[piece];
        } else {
            flag = CAPTURE_FLAG;
            score = captureScore[piece][board->pieces[to]];
        }
        int move = getMove(from, to, board->pieces[to], NO_PIECE, flag);
        addMove(move | (score << 25), list);
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
    qsort(list->moves, list->numMoves, sizeof(int), compareMoves);
}
