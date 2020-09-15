#include "defs.h"

const int pieceColor[NUM_PIECE_TYPES] = {
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
};

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
