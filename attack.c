#include "defs.h"

/*
 * Attack bitboards for knights and kings. If there is a king on D4, then you
 * can retrieve the attack bitboard for that king with kingAttacks[D4]. This
 * attack bitboard has a 1-bit in every position that that piece (the king)
 * can attack. Only kings and knights have attack bitboards here because the
 * sliding pieces (bishops, rooks, queens) can be blocked by other pieces and
 * need a different method to generate their attacks (magic numbers), and the
 * pawns only need a couple bit shifts.
 * Remember: bit 0 = A1, bit 1 = B1, ... , bit 62 = G8, bit 63 = H8.
 * 
 *        Ex: kingAttacks[D4] =       |       Ex: knightAttacks[D4] = 
 *         0 0 0 0 0 0 0 0            |          0 0 0 0 0 0 0 0
 *         0 0 0 0 0 0 0 0            |          0 0 0 0 0 0 0 0
 *         0 0 0 0 0 0 0 0            |          0 0 1 0 1 0 0 0
 *         0 0 1 1 1 0 0 0            |          0 1 0 0 0 1 0 0
 *         0 0 1 0 1 0 0 0            |          0 0 0 0 0 0 0 0
 *         0 0 1 1 1 0 0 0            |          0 1 0 0 0 1 0 0
 *         0 0 0 0 0 0 0 0            |          0 0 1 0 1 0 0 0
 *         0 0 0 0 0 0 0 0            |          0 0 0 0 0 0 0 0
 */
const uint64 kingAttacks[64] = {
    0x0000000000000302, 0x0000000000000705, 0x0000000000000E0A, 0x0000000000001C14,
    0x0000000000003828, 0x0000000000007050, 0x000000000000E0A0, 0x000000000000C040,
    0x0000000000030203, 0x0000000000070507, 0x00000000000E0A0E, 0x00000000001C141C,
    0x0000000000382838, 0x0000000000705070, 0x0000000000E0A0E0, 0x0000000000C040C0,
    0x0000000003020300, 0x0000000007050700, 0x000000000E0A0E00, 0x000000001C141C00,
    0x0000000038283800, 0x0000000070507000, 0x00000000E0A0E000, 0x00000000C040C000,
    0x0000000302030000, 0x0000000705070000, 0x0000000E0A0E0000, 0x0000001C141C0000,
    0x0000003828380000, 0x0000007050700000, 0x000000E0A0E00000, 0x000000C040C00000,
    0x0000030203000000, 0x0000070507000000, 0x00000E0A0E000000, 0x00001C141C000000,
    0x0000382838000000, 0x0000705070000000, 0x0000E0A0E0000000, 0x0000C040C0000000,
    0x0003020300000000, 0x0007050700000000, 0x000E0A0E00000000, 0x001C141C00000000,
    0x0038283800000000, 0x0070507000000000, 0x00E0A0E000000000, 0x00C040C000000000,
    0x0302030000000000, 0x0705070000000000, 0x0E0A0E0000000000, 0x1C141C0000000000,
    0x3828380000000000, 0x7050700000000000, 0xE0A0E00000000000, 0xC040C00000000000,
    0x0203000000000000, 0x0507000000000000, 0x0A0E000000000000, 0x141C000000000000,
    0x2838000000000000, 0x5070000000000000, 0xA0E0000000000000, 0x40C0000000000000,
};
const uint64 knightAttacks[64] = {
    0x0000000000020400, 0x0000000000050800, 0x00000000000A1100, 0x0000000000142200,
    0x0000000000284400, 0x0000000000508800, 0x0000000000A01000, 0x0000000000402000,
    0x0000000002040004, 0x0000000005080008, 0x000000000A110011, 0x0000000014220022,
    0x0000000028440044, 0x0000000050880088, 0x00000000A0100010, 0x0000000040200020,
    0x0000000204000402, 0x0000000508000805, 0x0000000A1100110A, 0x0000001422002214,
    0x0000002844004428, 0x0000005088008850, 0x000000A0100010A0, 0x0000004020002040,
    0x0000020400040200, 0x0000050800080500, 0x00000A1100110A00, 0x0000142200221400,
    0x0000284400442800, 0x0000508800885000, 0x0000A0100010A000, 0x0000402000204000,
    0x0002040004020000, 0x0005080008050000, 0x000A1100110A0000, 0x0014220022140000,
    0x0028440044280000, 0x0050880088500000, 0x00A0100010A00000, 0x0040200020400000,
    0x0204000402000000, 0x0508000805000000, 0x0A1100110A000000, 0x1422002214000000,
    0x2844004428000000, 0x5088008850000000, 0xA0100010A0000000, 0x4020002040000000,
    0x0400040200000000, 0x0800080500000000, 0x1100110A00000000, 0x2200221400000000,
    0x4400442800000000, 0x8800885000000000, 0x100010A000000000, 0x2000204000000000,
    0x0004020000000000, 0x0008050000000000, 0x00110A0000000000, 0x0022140000000000,
    0x0044280000000000, 0x0088500000000000, 0x0010A00000000000, 0x0020400000000000,
};

/*
 * Given a bitboard with the position of a king, return a bitboard with a 1-bit
 * in every position that that king can attack. Illegal moves, such as moving
 * into check or moving to a square that is already occupied by a piece of the
 * same color, are not removed and are still included in the returned bitboard.
 * The parameter is a bitboard with a single king (as opposed to its square
 * index) because this is how the board struct stores the king's position.
 * 
 * king:      A bitboard with a 1-bit indicating the position of a king piece.
 *            This number must not be 0 and must have only one bit set to 1.
 * 
 * return:    A bitboard with a 1-bit in every position that the king can
 *            attack.
 */
uint64 getKingAttacks(uint64 king) {
    assert(king != 0);
    assert((king & (king - 1)) == 0);
    return kingAttacks[getLSB(king)];
}

/*
 * Given the position of a knight, return the attack bitboard of that knight.
 * This function takes in the index of a single knight instead of a bitboard
 * of multiple knights so that the caller is able to match the knight with
 * the squares that it attacks.
 * 
 * knights:    The index of a square that contains a knight. Must be in the
 *             range [0-64).
 * 
 * return:     A bitboard where each 1-bit represents a position where the
 *             knight on the given square can attack.
 */
uint64 getKnightAttacks(int square) {
    assert(square >= 0 && square < 64);
    return knightAttacks[square];
}

/*
 * Given a bitboard with the position of all pawns of either color, find all
 * the possible attacks to the right or left for either color. A pawn can
 * attack one square diagonally to the left (ex: D4 to C5) or to the right
 * (ex: D4 to E5). The left and right attacks are separated (as opposed to
 * getWhitePawnAttacks()) so that the caller knows where the attacking pawn
 * is. For example, if getWhitePawnAttacksRight() returns a bitboard with a
 * 1-bit on F3, the caller knows that the pawn must be on E2. If the left and
 * right attacks were combined, the caller wouldn't know whether it was on E2
 * or G2. These functions take in a bitboard with every pawn of one color on
 * the board (as opposed to 1 pawn at a time) because we can use bitwise
 * operations to find the attacks of all pawns at the same time.
 * 
 * pawns:         A bitboard where each 1-bit represents the position of a pawn
 *                of a certain color. All pawns in this bitboard must be the
 *                same color.
 * 
 * return:        A bitboard where each 1-bit represents a position where a
 *                pawn can attack to the right (getPawnAttacksRight()) or to
 *                the left (getPawnAttacksLeft())
 */
uint64 getWhitePawnAttacksRight(uint64 pawns) {
    assert((pawns & 0xFF000000000000FF) == 0);
    return (pawns << 9) & 0xFEFEFEFEFEFEFEFE;
}
uint64 getWhitePawnAttacksLeft(uint64 pawns) {
    assert((pawns & 0xFF000000000000FF) == 0);
    return (pawns << 7) & 0x7F7F7F7F7F7F7F7F;
}
uint64 getBlackPawnAttacksRight(uint64 pawns) {
    assert((pawns & 0xFF000000000000FF) == 0);
    return (pawns >> 9) & 0x7F7F7F7F7F7F7F7F;
}
uint64 getBlackPawnAttacksLeft(uint64 pawns) {
    assert((pawns & 0xFF000000000000FF) == 0);
    return (pawns >> 7) & 0xFEFEFEFEFEFEFEFE;
}

/*
 * Rook and Bishop blocker bitboards. Defined in magic.c. Used in magic.c and
 * in the functions getBishopAttacks() and getRookAttacks().
 */
extern uint64 bishopBlockers[64];
extern uint64 rookBlockers[64];

/*
 * Sliding piece attack tables (defined and initialized in magic.c). Used by 
 * the functions getBishopAttacks(), getRookAttacks(), and getQueenAttacks() to
 * quickly retrieve the attack bitboards of sliding pieces.
 */
extern uint64 bishopAttackTable[64][512];
extern uint64 rookAttackTable[64][4096];

/*
 * Given the position of a sliding piece and a bitboard of all the pieces on
 * the board, find the attack bitboard of that piece. Use getBishopAttacks()
 * for bishops, getRookAttacks() for rooks, and getQueenAttacks() for queens.
 * These functions use the allPieces bitboard to generate a bitboard of
 * blockers, then use this blocker bitboard to generate an index into the
 * attack tables.
 * 
 * square:        The index of a square that contains a sliding piece (a
 *                bishop for getBishopAttacks(), a rook for getRookAttacks(),
 *                and a queen for getQueenAttacks()).
 * allPieces:     A bitboard where each 1-bit represents the position of a
 *                piece. This bitboard contains pieces of both sides and must
 *                contain the sliding piece on the given square.
 * 
 * return:        A bitboard where each 1-bit represents a position where the
 *                sliding piece on the given square could attack.
 */
uint64 getBishopAttacks(int square, uint64 allPieces) {
    assert(square >= 0 && square < 64);
    assert(allPieces & (1ULL << square));
    uint64 blockers = allPieces & bishopBlockers[square];
    uint64 attackIndex = getBishopAttackIndex(square, blockers);
    assert(attackIndex < 512);
    return bishopAttackTable[square][attackIndex];
}
uint64 getRookAttacks(int square, uint64 allPieces) {
    assert(square >= 0 && square < 64);
    assert(allPieces & (1ULL << square));
    uint64 blockers = allPieces & rookBlockers[square];
    uint64 attackIndex = getRookAttackIndex(square, blockers);
    assert(attackIndex < 4096);
    return rookAttackTable[square][attackIndex];
}
uint64 getQueenAttacks(int square, uint64 allPieces) {
    assert(square >= 0 && square < 64);
    assert(allPieces & (1ULL << square));
    return getBishopAttacks(square, allPieces) |
        getRookAttacks(square, allPieces);
}

/*
 * Check to see if pieces of side 'side' are attacking any of the squares in
 * the 'squares' bitboard. This method checks every piece of side 'side' and
 * calls their respective attack functions. This function then checks to see
 * if any of these attack bitboards overlap with the 'squares' bitboard. If
 * so, one or more of the squares in the 'squares' bitboard are being attacked
 * so the function returns 1.
 * 
 * board:       The current chess position. The board must be a valid chess
 *              position. Passed in as a pointer.
 * square:      A bitboard of squares that this function is checking for
 *              attacks.
 * side:        The color who could be attacking the squares in the 'squares'
 *              bitboard. Must be either WHITE or BLACK.
 * 
 * return:      1 if any of the squares in the 'squares' bitboard are being
 *              attacked. 0 otherwise.
 */
int squareAttacked(const Board* board, uint64 squares, int side) {
    assert(checkBoard(board));
    assert(side == WHITE || side == BLACK);
    uint64 attacks = 0ULL;
    uint64 knights, bishops, rooks, queens;
    if (side == WHITE) {
        attacks |= getKingAttacks(board->pieceBitboards[WHITE_KING]);
        attacks |= getWhitePawnAttacksLeft(board->pieceBitboards[WHITE_PAWN]);
        attacks |= getWhitePawnAttacksRight(board->pieceBitboards[WHITE_PAWN]);
        knights = board->pieceBitboards[WHITE_KNIGHT];
        bishops = board->pieceBitboards[WHITE_BISHOP];
        rooks = board->pieceBitboards[WHITE_ROOK];
        queens = board->pieceBitboards[WHITE_QUEEN];
    }
    else {
        attacks |= getKingAttacks(board->pieceBitboards[BLACK_KING]);
        attacks |= getBlackPawnAttacksLeft(board->pieceBitboards[BLACK_PAWN]);
        attacks |= getBlackPawnAttacksRight(board->pieceBitboards[BLACK_PAWN]);
        knights = board->pieceBitboards[BLACK_KNIGHT];
        bishops = board->pieceBitboards[BLACK_BISHOP];
        rooks = board->pieceBitboards[BLACK_ROOK];
        queens = board->pieceBitboards[BLACK_QUEEN];
    }
    uint64 allPieces = board->colorBitboards[BOTH_COLORS];
    while (knights) {
        attacks |= getKnightAttacks(getLSB(knights));
        knights &= knights - 1;
    }
    while (bishops) {
        attacks |= getBishopAttacks(getLSB(bishops), allPieces);
        bishops &= bishops - 1;
    }
    while (rooks) {
        attacks |= getRookAttacks(getLSB(rooks), allPieces);
        rooks &= rooks - 1;
    }
    while (queens) {
        attacks |= getQueenAttacks(getLSB(queens), allPieces);
        queens &= queens - 1;
    }
    return (attacks & squares) != 0ULL;
}
