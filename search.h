#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "defs.h"
#include "board.h"

int isRepetition(const Board* board);
int fillpvArray(Board* board, int depth);

#endif
