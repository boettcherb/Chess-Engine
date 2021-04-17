#ifndef ATTACK_H_INCLUDED
#define ATTACK_H_INCLUDED

#include "defs.h"
#include "board.h"

uint64 getKingAttacks(uint64 king);
uint64 getKnightAttacks(int square);
uint64 getWhitePawnAttacksRight(uint64 pawns);
uint64 getWhitePawnAttacksLeft(uint64 pawns);
uint64 getBlackPawnAttacksRight(uint64 pawns);
uint64 getBlackPawnAttacksLeft(uint64 pawns);
uint64 getBishopAttacks(int square, uint64 blockers);
uint64 getRookAttacks(int square, uint64 blockers);
uint64 getQueenAttacks(int square, uint64 blockers);
int squareAttacked(const Board* board, uint64 squares, int side);

#endif
