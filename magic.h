#ifndef MAGIC_H_INCLUDED
#define MAGIC_H_INCLUDED

#include "defs.h"

void initBishopAttackTable(void);
void initRookAttackTable(void);
int getBishopAttackIndex(int square, uint64 blockers);
int getRookAttackIndex(int square, uint64 blockers);

#endif
