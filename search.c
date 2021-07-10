#include "defs.h"

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
