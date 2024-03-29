#include "defs.h"

#if defined(OS_WINDOWS)
    #include <windows.h>   // GetTickCount
#elif defined(OS_LINUX)
    #include <sys/time.h>  // struct timeval, gettimeofday
#endif

const int pieceColor[NUM_PIECE_TYPES] = {
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
};

const int pieces[BOTH_COLORS][NUM_PIECE_TYPES] = {
    { WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING },
    { BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING },
};

const int material[NUM_PIECE_TYPES] = { 1, 3, 3, 5, 9, 0, 1, 3, 3, 5, 9, 0 };

/*
 * Call this function once at the start of the program to initialize the bishop
 * and rook attack tables and to initialize the hash keys.
 */
void initializeAll() {
    initBishopAttackTable();
    initRookAttackTable();
    initHashKeys();
}

/*
 * Retrieve the index of the given bitboard's least significant bit. Ex: 
 * getLSB(0x1) = 0, getLSB(0x4) = 2, getLSB(0xC00) = 10. There is undefined
 * behavior if the bitboard is equal to 0.
 * 
 * bitboard:   The 64-bit number whose LSB index is retrieved. Must not be 0.
 * 
 * return:     The index of the given bitboard's least significant bit.
 */
int getLSB(uint64 bitboard) {
    assert(bitboard != 0);
#if defined(COMPILER_GCC)
    return __builtin_ctzll(bitboard);
#elif defined(COMPILER_MSVS)
    DWORD index;
    _BitScanForward64(&index, bitboard);
    return (int) index;
#else
    int index = 0;
    while (!(bitboard & 0x1)) {
        bitboard >>= 1;
        ++index;
    }
    return index;
#endif
}

/*
 * Retrieve the index of the given bitboard's most significant bit. Ex: 
 * getMSB(0x1) = 63, getLSB(0x8) = 60. There is undefined behavior if the 
 * bitboard is equal to 0.
 * 
 * bitboard:   The 64-bit number whose MSB index is retrieved. Must not be 0.
 * 
 * return:     The index of the given bitboard's most significant bit.
 */
int getMSB(uint64 bitboard) {
    assert(bitboard != 0);
#if defined(COMPILER_GCC)
    return 63 - __builtin_clzll(bitboard);
#elif defined(COMPILER_MSVS)
    DWORD index;
    _BitScanReverse64(&index, bitboard);
    return (int) index;
#else
    int index = 63;
    while (!(bitboard & 0x8000000000000000)) {
        bitboard <<= 1;
        --index;
    }
    return index;
#endif
}

/*
 * Count and return the number of bits that are set to 1 in the given
 * bitboard. Ex: countBits(11011100101) = 7, countBits(45) = 4.
 * 
 * bitboard:   The 64-bit number whose set bits are counted.
 * 
 * return:     The number of bits in the bitboard that are set to 1.
 */
int countBits(uint64 bitboard) {
#if defined(COMPILER_GCC)
    return __builtin_popcountll(bitboard);
#elif defined(COMPILER_MSVS)
    return (int) __popcnt64(bitboard);
#else
    int bits;
    for (bits = 0; bitboard; ++bits) {
        bitboard &= bitboard - 1;
    }
    return bits;
#endif
}

/*
 * Return a time value in milliseconds. A time value on its own is meaningless.
 * This function is meant to be called twice so that an elapsed time in
 * milliseconds can be computed (endTime - startTime). Retrieving a time value
 * in milliseconds is OS specific: Windows uses GetTickCount() and linux uses
 * gettimeofday() and a timeval struct.
 *
 * return: a time value in milliseconds
 */
uint64 getTime() {
#if defined(OS_WINDOWS)
    return (uint64) GetTickCount();
#elif defined(OS_LINUX)
    struct timeval t;
    gettimeofday(&t, 0);
    return (uint64) (t.tv_sec * 1000ULL + t.tv_usec / 1000ULL);
#endif
}
