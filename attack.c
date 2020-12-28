#include "attack.h"
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
    assert(pawns & 0xFF000000000000FF == 0);
    return (pawns << 9) & 0xFEFEFEFEFEFEFEFE;
}
uint64 getWhitePawnAttacksLeft(uint64 pawns) {
    assert(pawns & 0xFF000000000000FF == 0);
    return (pawns << 7) & 0x7F7F7F7F7F7F7F7F;
}
uint64 getBlackPawnAttacksRight(uint64 pawns) {
    assert(pawns & 0xFF000000000000FF == 0);
    return (pawns >> 9) & 0x7F7F7F7F7F7F7F7F;
}
uint64 getBlackPawnAttacksLeft(uint64 pawns) {
    assert(pawns & 0xFF000000000000FF == 0);
    return (pawns >> 7) & 0xFEFEFEFEFEFEFEFE;
}

/*
 * Sliding piece blockers. A "blocker" is a piece that limits the movement of a
 * sliding piece. A blocker must be on the same diagonal as a bishop and the
 * same rank/file of a rook. Pieces on the edge of the board are not blockers.
 * For example, a rook on D4 is "blocked" by a piece on C4 and B4, but not by a
 * piece on A4. The rook can "see" A4 regardless of whether a piece is there.
 * There is no queenBlockers[] because a queen is just the combination of a 
 * rook and bishop.
 * Remember: bit 0 = A1, bit 1 = B1, ... , bit 62 = G8, bit 63 = H8.
 * 
 *    Ex: bishopBlockers[D4]                      Ex: rookBlockers[D4]
 *       0 0 0 0 0 0 0 0               |            0 0 0 0 0 0 0 0
 *       0 0 0 0 0 0 1 0               |            0 0 0 1 0 0 0 0
 *       0 1 0 0 0 1 0 0               |            0 0 0 1 0 0 0 0
 *       0 0 1 0 1 0 0 0               |            0 0 0 1 0 0 0 0
 *       0 0 0 0 0 0 0 0               |            0 1 1 0 1 1 1 0
 *       0 0 1 0 1 0 0 0               |            0 0 0 1 0 0 0 0
 *       0 1 0 0 0 1 0 0               |            0 0 0 1 0 0 0 0
 *       0 0 0 0 0 0 0 0               |            0 0 0 0 0 0 0 0
 */
const uint64 bishopBlockers[64] = {
    0x0040201008040200, 0x0000402010080400, 0x0000004020100A00, 0x0000000040221400,
    0x0000000002442800, 0x0000000204085000, 0x0000020408102000, 0x0002040810204000,
    0x0020100804020000, 0x0040201008040000, 0x00004020100A0000, 0x0000004022140000,
    0x0000000244280000, 0x0000020408500000, 0x0002040810200000, 0x0004081020400000,
    0x0010080402000200, 0x0020100804000400, 0x004020100A000A00, 0x0000402214001400,
    0x0000024428002800, 0x0002040850005000, 0x0004081020002000, 0x0008102040004000,
    0x0008040200020400, 0x0010080400040800, 0x0020100A000A1000, 0x0040221400142200,
    0x0002442800284400, 0x0004085000500800, 0x0008102000201000, 0x0010204000402000,
    0x0004020002040800, 0x0008040004081000, 0x00100A000A102000, 0x0022140014224000,
    0x0044280028440200, 0x0008500050080400, 0x0010200020100800, 0x0020400040201000,
    0x0002000204081000, 0x0004000408102000, 0x000A000A10204000, 0x0014001422400000,
    0x0028002844020000, 0x0050005008040200, 0x0020002010080400, 0x0040004020100800,
    0x0000020408102000, 0x0000040810204000, 0x00000A1020400000, 0x0000142240000000,
    0x0000284402000000, 0x0000500804020000, 0x0000201008040200, 0x0000402010080400,
    0x0002040810204000, 0x0004081020400000, 0x000A102040000000, 0x0014224000000000,
    0x0028440200000000, 0x0050080402000000, 0x0020100804020000, 0x0040201008040200,
};
const uint64 rookBlockers[64] = {
    0x000101010101017E, 0x000202020202027C, 0x000404040404047A, 0x0008080808080876,
    0x001010101010106E, 0x002020202020205E, 0x004040404040403E, 0x008080808080807E,
    0x0001010101017E00, 0x0002020202027C00, 0x0004040404047A00, 0x0008080808087600,
    0x0010101010106E00, 0x0020202020205E00, 0x0040404040403E00, 0x0080808080807E00,
    0x00010101017E0100, 0x00020202027C0200, 0x00040404047A0400, 0x0008080808760800,
    0x00101010106E1000, 0x00202020205E2000, 0x00404040403E4000, 0x00808080807E8000,
    0x000101017E010100, 0x000202027C020200, 0x000404047A040400, 0x0008080876080800,
    0x001010106E101000, 0x002020205E202000, 0x004040403E404000, 0x008080807E808000,
    0x0001017E01010100, 0x0002027C02020200, 0x0004047A04040400, 0x0008087608080800,
    0x0010106E10101000, 0x0020205E20202000, 0x0040403E40404000, 0x0080807E80808000,
    0x00017E0101010100, 0x00027C0202020200, 0x00047A0404040400, 0x0008760808080800,
    0x00106E1010101000, 0x00205E2020202000, 0x00403E4040404000, 0x00807E8080808000,
    0x007E010101010100, 0x007C020202020200, 0x007A040404040400, 0x0076080808080800,
    0x006E101010101000, 0x005E202020202000, 0x003E404040404000, 0x007E808080808000,
    0x7E01010101010100, 0x7C02020202020200, 0x7A04040404040400, 0x7608080808080800,
    0x6E10101010101000, 0x5E20202020202000, 0x3E40404040404000, 0x7E80808080808000,
};

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
    uint64 attackIndex = getBishopAttackIndex(blockers, square);
    assert(attackIndex >= 0 && attackIndex < 512);
    return bishopAttackTable[square][attackIndex];
}
uint64 getRookAttacks(int square, uint64 allPieces) {
    assert(square >= 0 && square < 64);
    assert(allPieces & (1ULL << square));
    uint64 blockers = allPieces & rookBlockers[square];
    uint64 attackIndex = getRookAttackIndex(blockers, square);
    assert(attackIndex >= 0 && attackIndex < 4096);
    return rookAttackTable[square][attackIndex];
}
uint64 getQueenAttacks(int square, uint64 allPieces) {
    assert(square >= 0 && square < 64);
    assert(allPieces & (1ULL << square));
    return getBishopAttacks(square, allPieces) |
        getRookAttacks(square, allPieces);
}
