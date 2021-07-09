#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "defs.h"
#include "hash.h"

/**
 * A structure to hold information about moves that were already made. The
 * board struct stores each move that was made to get to its current position
 * in an array called history[]. Moves are placed in the history[] array in
 * the makeMove() function. The undoMove() function accesses the last element
 * of the history[] array in order to undo the previous move.
 * 
 * move:              A 64-bit integer containing most of the necessary
 *                    information about the move.
 * castlePerms:       A combination of bit flags denoting which castling moves
 *                    are legal. Ex: If (castlePerms & CASTLE_WQ != 0), then
 *                    white can castle queenside in the current position.
 * fiftyMoveCount:    An integer holding the number of half moves since the
 *                    last capture or pawn move. Used for the fifty move rule.
 * enPassantSquare:   A bitboard with only 1 bit set to 1: the square that the
 *                    current side to move could attack by the en passant rule.
 * positionKey:       A 64-bit integer that is unique to the current position.
 *                    This value is used to check for 3-fold repetitions.
 */
typedef struct {
    int move;
    int castlePerms;
    int fiftyMoveCount;
    uint64 enPassantSquare;
    uint64 positionKey;
} PreviousMove;

/*
 * In this engine, a chessboard is represented using bitboards. Each bitboard
 * is a 64-bit number where the least significant bit (bit 0) represents square
 * A1 and the most significant bit (bit 63) represents square H8. A bit is set
 * to '1' if there is a piece on that square.
 * 
 * pieceBitboards:    The bitboards for this chessboard. There are 12 of them,
 *                    one for each piece type.
 * colorBitboards:    3 bitboards to represent (1) all the white pieces, (2)
 *                    all the black pieces, and (3) all pieces of both colors.
 *                    These bitboards are used often and it is more efficient
 *                    to keep them updated with the piece bitboards as moves
 *                    are made then to calculate them when needed.
 * pieces:            An array of 64 chars to hold the piece type for each
 *                    square. This allows quick access of the piece type of a
 *                    given square and is also updated incrementally with the
 *                    piece bitboards.
 * positionKey:       A 64-bit integer that is unique to the current position.
 *                    This value is used to check for 3-fold repetitions.
 * material:          Two integers holding the overall material for each side.
 *                    (Q=9, R=5, B=3, N=3, P=1). First set in setBoardToFen()
 *                    and updated incrementally in makeMove() and undoMove().
 * sideToMove:        An integer that is either 0 (white) or 1 (black) denoting
 *                    whose turn it is in the current position.
 * ply:               An integer holding the number of half moves made to get
 *                    to the current board position.
 * castlePerms:       A combination of bit flags denoting which castling moves
 *                    are legal. Ex: If (castlePerms & CASTLE_WQ != 0), then
 *                    white can castle queenside in the current position.
 * fiftyMoveCount:    An integer holding the number of half moves since the
 *                    last capture or pawn move. Used for the fifty move rule.
 * enPassantSquare:   A bitboard with only 1 bit set to 1: the square that the
 *                    current side to move could attack by the en passant rule.
 * history:           An array of PreviousMove structs that hold info about all
 *                    the moves made to get to the board's current position.
 * pvTable:           Principle Variation table. A hash table used to store the
 *                    best moves found by the alpha-beta algorithm, allowing us
 *                    to speed up our search.
 */
typedef struct {
    uint64 pieceBitboards[NUM_PIECE_TYPES];
    uint64 colorBitboards[3];
    signed char pieces[64];
    uint64 positionKey;
    int material[2];
    int sideToMove;
    int ply;
    int castlePerms;
    int fiftyMoveCount;
    uint64 enPassantSquare;
    PreviousMove history[MAX_GAME_MOVES];
    HashTable pvTable;
} Board;

void resetBoard(Board* board);
int setBoardToFen(Board* board, const char* fen);
int makeMove(Board* board, int move);
void undoMove(Board* board);

// functions only used in debug mode. NDEBUG is also used in assert.h. If
// NDEBUG is defined, asserts will not run.
#ifndef NDEBUG
    int checkBoard(const Board* board);
    int validMove(int move);
    void printPieces(const Board* board);
    void printBoard(const Board* board);
    void getMoveString(int move, char* moveString);
    void printBitboard(uint64 bitboard);
#endif

#endif
