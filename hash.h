#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include "defs.h"
#include "board.h"

void initHashKeys();
uint64 generatePositionKey(const Board* board);
uint64 getSideHashKey();
uint64 getPieceHashKey(int piece, int square);
uint64 getEnPassantHashKey(int square);
uint64 getCastleHashKey(int castlePerm);

#endif
