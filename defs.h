#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

// Determine the operating system. This allows us to include os-specific
// libraries. Ex: <sys/time.h> for linux vs <windows.h> for windows
// TODO: handle MAC as well?
#undef OS_WINDOWS
#undef OS_LINUX
#if defined(_WIN32)
    #if !defined(_WIN64)
        #error "Must compile with 64-bit Windows (x64)"
    #endif
    #define OS_WINDOWS
#elif defined(__linux__)
    #define OS_LINUX
#endif
#if !defined(OS_WINDOWS) && !defined(OS_LINUX)
    #error "Unrecognized Operating System!"
#endif

// Determine the compiler. This allows us to call compiler-specific functions
// Ex: __builtin_popcountll() for gcc vs __popcnt64() for visual studio
// TODO: add support for more C compilers (clang, minGW, etc.)
#undef COMPILER_MSVS
#undef COMPILER_GCC
#if defined(_MSC_VER)
    #define COMPILER_MSVS
#elif defined(__GNUC__)
    #define COMPILER_GCC
#endif

#include <assert.h>
#ifndef NDEBUG
    #include <stdio.h>
#endif

typedef unsigned long long uint64;
#define MAX_GAME_MOVES 512
#define NUM_PIECE_TYPES 12
#define MAX_SEARCH_DEPTH 64
#define NO_PIECE -1

#define MOVE_FLAGS                 0x1F00000
#define CAPTURE_FLAG               0x0100000
#define PROMOTION_FLAG             0x0200000
#define CAPTURE_AND_PROMOTION_FLAG 0x0300000
#define CASTLE_FLAG                0x0400000
#define EN_PASSANT_FLAG            0x0800000
#define PAWN_START_FLAG            0x1000000

#define CASTLE_WK 0x1
#define CASTLE_WQ 0x2
#define CASTLE_BK 0x4
#define CASTLE_BQ 0x8

enum Piece {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING,
};

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

enum Color {
    WHITE, BLACK, BOTH_COLORS,
};

extern const int pieceColor[NUM_PIECE_TYPES];
extern const int pieces[BOTH_COLORS][NUM_PIECE_TYPES];
extern const int material[NUM_PIECE_TYPES];

void initializeAll();
int getLSB(uint64 bitboard);
int getMSB(uint64 bitboard);
int countBits(uint64 bitboard);
uint64 getTime();

/*
 * A single entry in a HashTable. Each entry stores a move and a position key.
 * When the search algorithm finds a good move in a certain position, it will
 * store the move and the board's position key in the hash table. We store the 
 * position key as well because hash collisions are possible and when we go to 
 * retrieve the move, we want to be sure that the move is for the correct board
 * (The move could have been overridden by another board with the same hash).
 * 
 * positionKey:     The position key for the board whose move we are storing.
 * move:            The move to be stored.
 */
typedef struct {
    uint64 positionKey;
    int move;
} HashEntry;

/*
 * A hash table used to store the best moves found by the search algorithm. The
 * table uses dynamically allocated data from the heap. Each board has a single
 * hash table (named pvTable) to store its best moves.
 * 
 * table:           The table of hashEntries.
 * numEntries:      The number of hashEntries in the table. This value is set
 *                  when the table is allocated from the heap.
 */
typedef struct {
    HashEntry* table;
    int numEntries;
} HashTable;

/*
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
 *                    and updated incrementally as moves are made and unmade.
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
 * pvArray:           An array of moves (ints) storing the principal variation
 *                    (best / main line) of the current position.
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
    int pvArray[MAX_SEARCH_DEPTH];
} Board;

/******************************************************************************
Each move in a MoveList is a 64-bit integer with the following information:
0 0000 0000 0000 0000 0011 1111   6 bits for the 'from' square
0 0000 0000 0000 1111 1100 0000   6 bits for the 'to' square
0 0000 0000 1111 0000 0000 0000   4 bits for the captured piece
0 0000 1111 0000 0000 0000 0000   4 bits for the promoted piece
0 0001 0000 0000 0000 0000 0000   1 bit for the capture flag
0 0010 0000 0000 0000 0000 0000   1 bit for the promotion flag
0 0100 0000 0000 0000 0000 0000   1 bit for the castle flag
0 1000 0000 0000 0000 0000 0000   1 bit for the en passant flag
1 0000 0000 0000 0000 0000 0000   1 bit for the pawn start flag

The remaining 7 bits contain the move score. This score will be used with
the minimax / alpha-beta algorithm. A move will have a higher score if it is
likely to be a good move (Ex: captures, promotions, castling). Sorting moves
by their score will help the search algorithm run faster, as more pruning
can occur if the best moves are considered first.
******************************************************************************/

/*
 * Use a MoveList to store all of the moves that are generated by 
 * generateAllMoves. Each MoveList stores all the possible moves for a single
 * board position, including pseudo-legal moves (moves that leave the king in
 * check).
 * 
 * numMoves:     An integer storing the number of moves in the MoveList. The 
 *               number of moves in the list cannot exceed MAX_GAME_MOVES.
 * moves:        An array of moves. Each move contains multiple pieces of 
 *               information which is combined into 1 32-bit integer.
 */
typedef struct {
    int numMoves;
    int moves[MAX_GAME_MOVES];
} MoveList;

// board.h
int setBoardToFen(Board* board, const char* fen);
int makeMove(Board* board, int move);
void undoMove(Board* board);

// hashkey.h
void initHashKeys();
uint64 generatePositionKey(const Board* board);
uint64 getSideHashKey();
uint64 getPieceHashKey(int piece, int square);
uint64 getEnPassantHashKey(int square);
uint64 getCastleHashKey(int castlePerm);

// hashtable.h
void initHashTable(HashTable* table);
void clearHashTable(HashTable* table);
void freeHashTable(HashTable* table);
void storeMove(HashTable* table, int move, uint64 positionKey);
int retrieveMove(HashTable* table, uint64 positionKey);

// movegen.h
void generateAllMoves(const Board* board, MoveList* list);
int moveExists(Board* board, int move);

// attack.h
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

// magic.h
void initBishopAttackTable(void);
void initRookAttackTable(void);
int getBishopAttackIndex(int square, uint64 blockers);
int getRookAttackIndex(int square, uint64 blockers);

// search.h
int isRepetition(const Board* board);
int fillpvArray(Board* board, int depth);

// functions only used in debug mode
#ifndef NDEBUG
    int checkBoard(const Board* board);
    int validMove(int move);
    void printPieces(const Board* board);
    void printBoard(const Board* board);
    void getMoveString(int move, char* moveString);
    void printBitboard(uint64 bitboard);
#endif

#endif
