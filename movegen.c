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
    assert(~flags & MOVE_FLAGS == 0);
    uint64 move = from | (to << 6) | ((captured & 0xF) << 12);
    return move | ((promoted & 0xF) << 16) | (flags << 20);
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
    if (board->sideToMove == WHITE) {
        // king
        int from = getLSB(board->pieceBitboards[WHITE_KING]);
        uint64 attacks = getKingAttacks(board->pieceBitboards[WHITE_KING]);
        attacks &= ~board->colorBitboards[WHITE];
        generatePieceMoves(board, list, attacks, from);

        // knight
        uint64 knights = board->pieceBitboards[WHITE_KNIGHT];
        while (knights) {
            from = getLSB(knights);
            uint64 attacks = getKnightAttacks(from);
            attacks &= ~board->colorBitboards[WHITE];
            generatePieceMoves(board, list, attacks, from);
            knights &= knights - 1;
        }
    }
    else {
        // king
        int from = getLSB(board->pieceBitboards[BLACK_KING]);
        uint64 attacks = getKingAttacks(board->pieceBitboards[BLACK_KING]);
        attacks &= ~board->colorBitboards[BLACK];
        generatePieceMoves(board, list, attacks, from);

        // knight
        uint64 knights = board->pieceBitboards[BLACK_KNIGHT];
        while (knights) {
            int from = getLSB(knights);
            uint64 attacks = getKnightAttacks(from);
            attacks &= ~board->colorBitboards[BLACK];
            generatePieceMoves(board, list, attacks, from);
            knights &= knights - 1;
        }
    }
}
