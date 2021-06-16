#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include "defs.h"

// position key / hash key
// --------------------------------------------
void initHashKeys();
uint64 generatePositionKey(const void* board);
uint64 getSideHashKey();
uint64 getPieceHashKey(int piece, int square);
uint64 getEnPassantHashKey(int square);
uint64 getCastleHashKey(int castlePerm);

// hash table
// --------------------------------------------
typedef struct {
    uint64 positionKey;
    int move;
} HashEntry;

typedef struct {
    HashEntry* table;
    int numEntries;
} HashTable;

void initHashTable(HashTable* table);
void clearHashTable(HashTable* table);
void freeHashTable(HashTable* table);

#endif
