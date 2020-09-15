#ifndef ATTACK_H_INCLUDED
#define ATTACK_H_INCLUDED

#include "defs.h"

uint64 getKingAttacks(uint64 king);
uint64 getKnightAttacks(uint64 knights);
uint64 getPawnAttacksRight(uint64 pawns, int color);
uint64 getPawnAttacksLeft(uint64 pawns, int color);

#endif
