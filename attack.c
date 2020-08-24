#include "attack.h"
#include "defs.h"

/*
 * Retrieve the index of the given bitboard's least significant bit. Ex: 
 * getLSB(0x1) = 0, getLSB(0x4) = 2, getLSB(0xC00) = 10. There is undefined
 * behavior if the bitboard is equal to 0.
 * 
 * bitboard:     The 64-bit number whose LSB index is retrieved. Must not be 0.
 * 
 * return:       The index of the given bitboard's least significant bit.
 */
int getLSB(uint64 bitboard) {
    assert(bitboard != 0);
    int index = -1;
    int found = 0;
    while (!found) {
        found = (bitboard >> ++index) & 0x1;
    }
    return index;
}

/*
 * For each index (0-63) there is a 1 bit in the positions that a king at that
 * index would be able to attack.
 * Remember: bit 0 = A1, bit 1 = B1, ... , bit 62 = G8, bit 63 = H8.
 * 
 *                         0 0 0 0 0 0 0 0
 *                         0 0 0 0 0 0 0 0
 *                         0 0 0 0 0 0 0 0
 * Ex: kingAttacks[D4]  =  0 0 1 1 1 0 0 0
 *                         0 0 1 0 1 0 0 0
 *                         0 0 1 1 1 0 0 0
 *                         0 0 0 0 0 0 0 0
 *                         0 0 0 0 0 0 0 0
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

/*
 * Given a bitboard with the position of a king, return a bitboard with a 1 bit
 * in every position that that king can attack. Illegal moves, such as moving
 * into check or moving to a square that is already occupied by a piece of the
 * same color, are not removed and are still included in the returned bitboard.
 * 
 * kingPos:      A bitboard with a 1-bit indicating the position of a king
 *               piece. This number must not be 0 and must have only one bit
 *               set to 1
 * 
 * return:       A bitboard with a 1 bit in every position that the king can
 *               attack.
 */
uint64 getKingAttacks(uint64 kingPos) {
    assert(kingPos != 0);
    assert((kingPos & (kingPos - 1)) == 0);
    return kingAttacks[getLSB(kingPos)];
}