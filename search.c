#include "defs.h"

#include <string.h> // memset

#define INFINITY 2000000000
#define MATE 30000

/*
 * Determine if the current state of the board is a repetition of a previous
 * state. Check this by comparing position keys at different stages of the
 * game. In the board's history array, we store the position key after each
 * move, so it is easy to compare the current board's position with its
 * previous positions.
 * 
 * board:      The current state of the board, passed in as a const pointer.
 * 
 * return:     1 if the current board state is a repetition, 0 otherwise.
 */
int isRepetition(const Board* board) {
    assert(checkBoard(board));
    int start = board->ply - 2;
    int end = board->ply - board->fiftyMoveCount;
    for (int i = start; i >= end; i -= 2) {
        assert(i >= 0);
        if (board->positionKey == board->history[i].positionKey) {
            // TODO: verify repetition
            return 1;
        }
    }
    return 0;
}

/*
 * Fill up the board's pvArray with moves that are stored in the board's hash
 * table (pvTable). This function will be called after the alpha-beta algorithm
 * which will store the best moves from the search in the board's pvTable. We
 * have to check to make sure the stored move is legal in the current position
 * with moveExists() due to the possibility of Zobrist hashing collisions. We
 * return the length of the pv line. This will usually be 'depth' (the max
 * depth that we have gone with alpha-beta so far), but it could be lower in
 * the case of a collision.
 * 
 * board:      The board whose pvArray we are filling.
 * depth:      The max length of our pv line. This is how deep we have searched
 *             with alpha-beta for the current position so far.
 * 
 * return:     The length of the pv line that was found in the hash table.
 */
int fillpvArray(Board* board, int depth) {
    assert(board != NULL);
    assert(depth < MAX_SEARCH_DEPTH);
    int movesFound = 0;
    while (movesFound < depth) {
        int move = retrieveMove(&board->pvTable, board->positionKey);
        if (move == 0 || !moveExists(board, move)) {
            // TODO: add print statements to see how often collisions occur
            break;
        }
        makeMove(board, move);
        board->pvArray[movesFound++] = move;
    }
    for (int i = 0; i < movesFound; ++i) {
        undoMove(board);
    }
    return movesFound;
}

// check if search time is up, or if there was an interrupt from the GUI
// static void checkUp() {
//     return;
// }

// clear board's pv array, history heuristics, etc. to get ready for a new search
static void clearForSearch(Board* board, SearchInfo* info) {
    assert(checkBoard(board));
    memset(board->searchHistory, 0, sizeof(int) * NUM_PIECE_TYPES * 64);
    memset(board->searchKillers, 0, sizeof(int) * 2 * MAX_SEARCH_DEPTH);
    clearHashTable(&board->pvTable);
    board->searchPly = 0;
    info->startTime = getTime();
    info->nodes = info->stopped = info->failHigh = info->failHighFirst = 0;
}

// eliminates the horizon effect by going through all capture moves in a position
// static int quiescenseSearch(Board* board, SearchInfo* info, int alpha, int beta) {
//     assert(checkBoard(board));
//     (void) alpha;
//     checkUp();
//     (void) beta;
//     return 0;
// }

static int alphaBeta(Board* board, SearchInfo* info, int alpha, int beta, int depth, int doNull) {
    (void) doNull;
    assert(checkBoard(board));
	
	++info->nodes;

	if (depth == 0) {
		return evaluatePosition(board);
	}
	
	if (isRepetition(board) || board->fiftyMoveCount >= 100) {
		return 0;
	}
	
	if(board->searchPly >= MAX_SEARCH_DEPTH) {
		return evaluatePosition(board);
	}
	
	MoveList list;
    generateAllMoves(board, &list);

	int legal = 0;
	int oldAlpha = alpha;
	int bestMove = -1;
	
	for (int moveNum = 0; moveNum < list.numMoves; ++moveNum) {	
       
        if (!makeMove(board, list.moves[moveNum]))  {
            continue;
        }
        
		++legal;
		int score = -alphaBeta(board, info, -beta, -alpha, depth - 1, 1);		
        undoMove(board);
		
		if (score > alpha) {
			if (score >= beta) {
                if (legal == 1) {
                    ++info->failHighFirst;
                }
                ++info->failHigh;
				return beta;
			}
			alpha = score;
			bestMove = list.moves[moveNum];
		}		
    }
	
	if (legal == 0) {
        int king = board->sideToMove == WHITE ? WHITE_KING : BLACK_KING;
        if (squareAttacked(board, board->pieceBitboards[king], board->sideToMove ^ 1)) {
			return -MATE + board->searchPly;
		} else {
			return 0;
		}
	}
	
	if (alpha != oldAlpha) {
        storeMove(&board->pvTable, bestMove, board->positionKey);
	}
	
	return alpha;
}

// handles iterative deepening
void searchPosition(Board* board, SearchInfo* info) {
    assert(checkBoard(board));
    clearForSearch(board, info);
    for (int depth = 1; depth <= info->depth; ++depth) {
        int score = alphaBeta(board, info, -INFINITY, INFINITY, depth, 1);

        // check if we are out of time

        // print stuff
        printf("depth: %d, score: %d, nodes: %lld\n", depth, score, info->nodes);
        int numMoves = fillpvArray(board, depth);
		printf("pv line of %d moves:", numMoves);
		for (int i = 0; i < numMoves; ++i) {
			char moveString[6];
			getMoveString(board->pvArray[i], moveString);
			printf(" %s", moveString);
		}
		putchar('\n');
        printf("fail high: %.2f\n", info->failHigh);
        printf("fail high first: %.2f\n", info->failHighFirst);
        printf("Ordering: %.2f\n", info->failHighFirst / info->failHigh);
    }
}
