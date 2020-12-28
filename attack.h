#ifndef ATTACK_H_INCLUDED
#define ATTACK_H_INCLUDED

#include "defs.h"

uint64 getKingAttacks(uint64 king);
uint64 getKnightAttacks(int square);
uint64 getPawnAttacksRight(uint64 pawns, int color);
uint64 getPawnAttacksLeft(uint64 pawns, int color);
uint64 getBishopAttacks(int square, uint64 blockers);
uint64 getRookAttacks(int square, uint64 blockers);
uint64 getQueenAttacks(int square, uint64 blockers);

#endif
