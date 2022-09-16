#include "defs.h"

#define HASH_TABLE_SIZE 0x100000 * 2  // 2 MB

/*
 * Initialize the hash table. Set hashTable->numEntries to the correct value
 * and allocate memory for the hash entries. 
 *
 * hashTable:     The hash table that is being initialized.
 */
void initHashTable(HashTable* hashTable) {
    freeHashTable(hashTable);
    hashTable->numEntries = HASH_TABLE_SIZE / sizeof(HashEntry);
    hashTable->table = (HashEntry*) malloc(HASH_TABLE_SIZE);
    clearHashTable(hashTable);
}

/*
 * Free the dynamically allocated block of memory pointed to by
 * hashTable->table. Any data stored in the hash table will be lost.
 * 
 * hashTable:     The hash table whose memory we are freeing.
 */
void freeHashTable(HashTable* hashTable) {
    free(hashTable->table);
    hashTable->table = NULL;
}

/*
 * Delete all data from the hash table. Any entries stored in the hash table
 * will be removed and the entire block of memory will be set to 0.
 * 
 * hashTable:     The hash table that is being cleared.
 */
void clearHashTable(HashTable* hashTable) {
    memset(hashTable->table, 0, HASH_TABLE_SIZE);
}

/*
 * Store a move in the hash table. This function is be used to store the best
 * move in a position where the board's position key is equal to positionKey.
 * That position key is then used as the hash key into the hash table so that
 * we can quickly retrieve the best move in the position.
 * 
 * hashTable:     The board's principal variation table which will store the
 *                move. Passed in as a pointer which must not be null.
 * move:          The move to store into the hash table. Must be a valid move.
 * positionKey:   The board's position key where move is a valid move. Used as
 *                the hash key into hashTable
 */
void storeMove(HashTable* hashTable, int move, uint64 positionKey) {
    assert(hashTable != NULL);
    assert(validMove(move));
    int index = positionKey % hashTable->numEntries;
    hashTable->table[index].move = move;
    hashTable->table[index].positionKey = positionKey;
}

/*
 * Retrieve a move from the hash table using the given position / hash key.
 * 
 * hashTable:     The board's principal variation table which will store the
 *                move. Passed in as a pointer which must not be null.
 * positionKey:   The board's position key where move is a valid move. Used as
 *                the hash key into hashTable
 * 
 * return:        The move at the position specified by the given position key
 *                if the keys match. If they do not match, return 0.
 */
int retrieveMove(HashTable* hashTable, uint64 positionKey) {
    assert(hashTable != NULL);
    int index = positionKey % hashTable->numEntries;
    if (hashTable->table[index].positionKey == positionKey) {
        return hashTable->table[index].move;
    }
    return 0;
}
