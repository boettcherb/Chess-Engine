#include "hash.h"
#include "defs.h"
#include "board.h"

#include <stdlib.h>

/* 
 * These values are the hash keys that will be used to generate position keys
 * for each chess position. Position keys are generated using a technique
 * called Zobrist Hashing which takes into account the locations of every
 * piece, the side to move, the castling rights of each side, and the en
 * passant square (if there is one). If 2 position keys from 2 boards are
 * different, we can be certain that those boards are different as well. We can
 * quickly check for repetitions by comparing the current position key with the
 * position keys in the board's history array. However it is possible (although
 * VERY unlikely) that a hash collision can occur, which will cause 2 different
 * boards to generate the same position key. Therefore, if 2 position keys are
 * the same, we must manually verify that the boards are equal as well. Since
 * repetitions in chess are unlikely, this should not have much of an impact on
 * performance. 
 */
static uint64 sideKey;
static uint64 pieceKeys[NUM_PIECE_TYPES][64];
static uint64 castleKeys[16];
static uint64 enPassantKeys[64];

/*
 * Generate a random 64-bit unsigned integer using stdlib.h's rand() function.
 * RAND_MAX is 32767, which is only 15 bits. To create a random 64-bit number,
 * we call rand() 5 times and shift each number into place.
 * 
 * return: A random 64-bit integer.
 */
static uint64 randomUInt64() {
    uint64 r1 = rand(), r2 = rand(), r3 = rand(), r4 = rand(), r5 = rand();
    return r1 | (r2 << 15) | (r3 << 30) | (r4 << 45) | (r5 << 60);
}

/*
 * Initialize the hash keys that are used to generate a board's position key.
 * This function is only called once at the start of the program. Each of the
 * hash keys (sideKey, pieceKeys, castleKeys, enPassantKeys) are given a random
 * 64-bit value.
 */
void initHashKeys() {
    srand(3859);
    sideKey = randomUInt64();
    for (int pieceType = 0; pieceType < NUM_PIECE_TYPES; ++pieceType) {
        for (int square = 0; square < 64; ++square) {
            pieceKeys[pieceType][square] = randomUInt64();
        }
    }
    for (int castlePerm = 0; castlePerm < 16; ++castlePerm) {
        castleKeys[castlePerm] = randomUInt64();
    }
    for (int square = 0; square < 64; ++square) {
        enPassantKeys[square] = randomUInt64();
    }
}

/*
 * Generate a unique position key for the given chessboard. This position key
 * will be used to detect repetitions. After every move the position key can be
 * very quickly updated along with the chessboard, so comparing two positions
 * is as simple as comparing their position keys.
 * 
 * board:    The board we want to generate a position key for. Passed in as
 *           a const pointer which must not be NULL.
 * 
 * return:   The position key for the given board.
 */
uint64 generatePositionKey(const Board* board) {
    assert(board != NULL);
    uint64 positionKey = board->sideToMove == WHITE ? sideKey : 0ULL;
    for (int square = 0; square < 64; ++square) {
        if (board->pieces[square] != NO_PIECE) {
            positionKey ^= pieceKeys[board->pieces[square]][square];
        }
    }
    positionKey ^= castleKeys[board->castlePerms];
    if (board->enPassantSquare != 0ULL) {
        int square = getLSB(board->enPassantSquare);
        positionKey ^= enPassantKeys[square];
    }
    return positionKey;
}

/*
 * Retrieve the hash key that is used to factor in which side it is to move.
 * When it is white's move, the hash key is xor-ed into the board's position
 * key, and when it is black's move it is removed. This way, if 2 boards have
 * the same piece layouts but one has white to move and the other has black
 * to move, they will have a different position key.
 * 
 * return:      The side hash key as a 64-bit integer.
 */
uint64 getSideHashKey() {
    return sideKey;
}

/*
 * Retrieve the hash key that is used to mark that a certain piece is on a
 * certain square. There are 64 * 12 = 768 different piece hash keys: one for
 * the 12 piece types on each of the 64 squares. These hash keys are used to
 * mark in the board's position key which pieces are on which squares so that
 * if 2 boards have different piece layouts, their position keys will also be
 * different.
 * 
 * piece:       The piece type that is on the given 'square'. Must be a valid   
 *              piece type.
 * square:      The square that the given 'piece' is on. An integer in the 
 *              range [0-64).
 * 
 * return:      The piece hash key for a piece of type 'piece' on the given
 *              'square' as a 64-bit integer.
 */
uint64 getPieceHashKey(int piece, int square) {
    assert(piece >= WHITE_PAWN && piece <= BLACK_KING);
    assert(square >= 0 && square < 64);
    return pieceKeys[piece][square];
}

/*
 * Retrieve the hash key that is used to mark an en passant capture is possible
 * on the given 'square'. There are 64 en passant hash keys, one for each
 * square, although in reality only 16 of these will ever be used (the keys for
 * the 3rd and 6th ranks because these are the only squares where en passant
 * captures are possible). If 2 boards have the same piece layouts but on one
 * board an en passant capture is possible, their 2 position keys will be
 * different.
 * 
 * square:      The square where an en passant capture is possible and whose en
 *              passant hash key we want to retrieve.
 * 
 * return:      The en passant hash key for an en passant move on the given
 *              'square' as a 64-bit integer.
 */
uint64 getEnPassantHashKey(int square) {
    assert(square >= 0 && square < 64);
    assert((1ULL << square) & 0x0000FF0000FF0000);
    return enPassantKeys[square];
}

/*
 * Retrieve the hash key that is used to mark the castling permissions of a
 * chessboard. There are 16 castle hash keys, one for each of the 16 possible
 * permutations of castling permissions. If 2 boards have the same piece
 * layouts but different castling permissions, their 2 position keys will be
 * different.
 * 
 * castlePerm:  The current castling permissions of a chessboard. Only the 4
 *              least significant bits can ever be 1.
 * 
 * return:      The castle hash key for the given castling permissions as a
 *              64-bit integer.
 */
uint64 getCastleHashKey(int castlePerm) {
    assert((castlePerm & 0xFFFFFFF0) == 0);
    return castleKeys[castlePerm];
}
