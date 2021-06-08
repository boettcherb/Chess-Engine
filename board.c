#include "board.h"
#include "defs.h"
#include "attack.h"
#include "hash.h"

#include <stdio.h>
#include <string.h>

/*
 * Clear the board and set all board variables to their default value. The
 * default value for board.pieces[64] is NO_PIECE (-1). board.sideToMove is set
 * to BOTH_COLORS so that an error will be thrown if it is not changed to
 * either WHITE or BLACK when the board is set up. The default value of
 * everything else is 0.
 * 
 * board:       The board to be cleared, passed in as a pointer. The pointer
 *              must not be NULL.
 */
void resetBoard(Board* board) {
    assert(board != NULL);
    memset(board, 0, sizeof(Board));
    memset(board->pieces, NO_PIECE, sizeof(char) * 64);
    board->sideToMove = BOTH_COLORS;
}

/*
 * Set up a chessboard to the position given by the FEN string. Forsyth–Edwards
 * Notation (FEN) is standard notation for describing a particular board
 * position of a chess game. A FEN string has 6 parts: (1) the piece layout,
 * (2) side to move, (3) castling permissions for both sides, (4) en passant
 * square (if there is one), (5) the number of half moves since the last
 * capture or pawn move, and (6) the move number. After execution the board
 * should be a valid chess position that exactly matches the FEN string. If
 * something goes wrong the function prints an error message and returns 0.
 * 
 * board:      The board to be set up, passed in as a pointer. The pointer must
 *             not be NULL.
 * fen:        The position to set the board to, passed in as a const char
 *             pointer. The pointer must not be NULL.
 * 
 * return:     1 if the board was successfully set to the chess position given
 *             by the FEN string, 0 otherwise.
 */
int setBoardToFen(Board* board, const char* fen) {
    assert(board != NULL);
    assert(fen != NULL);
    resetBoard(board);
    char layout[100], side, castlePerms[100], enPassantSquare[100];
    int fiftyMoveCount, moveNumber;
    if (sscanf(fen, "%s %c %s %s %d %d", layout, &side, castlePerms,
        enPassantSquare, &fiftyMoveCount, &moveNumber) != 6) {
        puts("Error: setBoardToFen: Could not parse or invalid FEN string.");
        return 0;
    }

    // layout
    int layoutLength = strlen(layout), square = 56, numEmpty;
    assert(layoutLength < 70);
    for (int layoutPos = 0; layoutPos < layoutLength; ++layoutPos) {
        switch(layout[layoutPos]) {
            case 'P': board->pieces[square++] = WHITE_PAWN; break;
            case 'N': board->pieces[square++] = WHITE_KNIGHT; break;
            case 'B': board->pieces[square++] = WHITE_BISHOP; break;
            case 'R': board->pieces[square++] = WHITE_ROOK; break;
            case 'Q': board->pieces[square++] = WHITE_QUEEN; break;
            case 'K': board->pieces[square++] = WHITE_KING; break;
            case 'p': board->pieces[square++] = BLACK_PAWN; break;
            case 'n': board->pieces[square++] = BLACK_KNIGHT; break;
            case 'b': board->pieces[square++] = BLACK_BISHOP; break;
            case 'r': board->pieces[square++] = BLACK_ROOK; break;
            case 'q': board->pieces[square++] = BLACK_QUEEN; break;
            case 'k': board->pieces[square++] = BLACK_KING; break;
            case '/': square -= 16; break;
            default:
                numEmpty = layout[layoutPos] - '0';
                if (numEmpty < 1 || numEmpty > 8) {
                    puts("Error: setBoardToFen: Invalid character (layout)");
                    return 0;
                }
                square += numEmpty;
        }
    }
    for (square = 0; square < 64; ++square) {
        int piece = board->pieces[square];
        if (board->pieces[square] != NO_PIECE) {
            assert(piece >= 0 && piece < NUM_PIECE_TYPES);
            board->material[pieceColor[piece]] += material[piece];
            board->pieceBitboards[piece] |= (1ULL << square);
            board->colorBitboards[pieceColor[piece]] |= (1ULL << square);
            board->colorBitboards[BOTH_COLORS] |= (1ULL << square);
        }
    }

    // side to move
    if (side != 'w' && side != 'b') {
        puts("Error: setBoardToFen: color char must be either 'w' or 'b'.");
        return 0;
    }
    board->sideToMove = side == 'w' ? WHITE : BLACK;

    // castle permissions
    int castlePermsLength = strlen(castlePerms); 
    assert(castlePermsLength < 5);
    for (int pos = 0; pos < castlePermsLength; ++pos) {
        switch (castlePerms[pos]) {
            case 'K': board->castlePerms |= CASTLE_WK; break;
            case 'Q': board->castlePerms |= CASTLE_WQ; break;
            case 'k': board->castlePerms |= CASTLE_BK; break;
            case 'q': board->castlePerms |= CASTLE_BQ; break;
            case '-': assert(pos == 0); break;
            default:
                puts("Error: setBoardToFen: Invalid character (castle perms)");
                return 0;
        }
    }

    // en passant square
    assert(strlen(enPassantSquare) < 3);
    if (enPassantSquare[0] != '-') {
        int file = enPassantSquare[0] - 'a';
        int rank = enPassantSquare[1] - '1';
        assert((side == 'w' && file == 5) || (side == 'b' && file == 2));
        assert(rank >= 0 && rank < 8);
        board->enPassantSquare = 1ULL << (file * 8 + rank);
    }

    // fifty move rule
    if (fiftyMoveCount < 0 || fiftyMoveCount > 100) {
        puts("Error: setBoardToFen: invalid half move clock (fifty move count "
        "must be between 0 and 100 inclusive)");
        return 0;
    }
    board->fiftyMoveCount = fiftyMoveCount;

    // move number
    if (moveNumber < 1) {
        puts("Error: setBoardToFen: moveNumber must be >= 1");
        return 0;
    }
    board->ply = 0;

    board->positionKey = generatePositionKey(board);
    assert(checkBoard(board));
    return 1;
}

/*
 * Remove the piece from the given square and update the given board's member
 * variables to reflect the change.
 *
 * board:      The board that is being updated, passed in as a pointer. The
 *             pointer must not be NULL.
 * square:     The square index of the piece to be removed. The index must be
 *             in the range [0-64) and the square must not be empty.
 */
static void clearPiece(Board* board, int square) {
    assert(board != NULL && square >= 0 && square < 64);
    assert(board->pieces[square] != NO_PIECE);
    int piece = board->pieces[square];
    uint64 clearMask = ~(1ULL << square);
    board->pieceBitboards[piece] &= clearMask;
    board->colorBitboards[pieceColor[piece]] &= clearMask;
    board->colorBitboards[BOTH_COLORS] &= clearMask;
    board->pieces[square] = NO_PIECE;
    board->positionKey ^= getPieceHashKey(piece, square);
    board->material[pieceColor[piece]] -= material[piece];
}

/*
 * Add a piece to the given square and update the given board's member
 * variables to reflect the change.
 *
 * board:      The board that is being updated, passed in as a pointer. The
 *             pointer must not be NULL.
 * square:     The index of the square where the piece will be added. The
 *             index must be in the range [0-64) and the square must be empty.
 * piece:      The piece to be added to the given square. This must be a valid
 *             piece type.
 */
static void addPiece(Board* board, int square, int piece) {
    assert(board != NULL && square >= 0 && square < 64);
    assert(piece >= 0 && piece < NUM_PIECE_TYPES);
    assert(board->pieces[square] == NO_PIECE);
    uint64 setMask = 1ULL << square;
    board->pieceBitboards[piece] |= setMask;
    board->colorBitboards[pieceColor[piece]] |= setMask;
    board->colorBitboards[BOTH_COLORS] |= setMask;
    board->pieces[square] = piece;
    board->positionKey ^= getPieceHashKey(piece, square);
    board->material[pieceColor[piece]] += material[piece];
}

/*
 * Move a piece from the square 'from' to the square 'to' and update the given
 * board's pieceBitboards, colorBitboards, and pieces arrays to reflect the
 * change.
 *
 * board:      The board that is being updated, passed in as a pointer. The
 *             pointer must not be NULL.
 * from:       The index of the square the piece started on. The index must be
 *             in the range [0-64) and the square must not be empty.
 * to:         The index of the square the piece will be moved to. The index
 *             must be in the range [0-64) and the square must be empty.
 */
static void movePiece(Board* board, int from, int to) {
    assert(board != NULL);
    assert(from >= 0 && from < 64 && to >= 0 && to < 64);
    assert(board->pieces[from] != NO_PIECE);
    assert(board->pieces[to] == NO_PIECE);
    int piece = board->pieces[from];
    uint64 clearMask = ~(1ULL << from);
    uint64 setMask = 1ULL << to;
    board->pieceBitboards[piece] &= clearMask;
    board->pieceBitboards[piece] |= setMask;
    board->colorBitboards[pieceColor[piece]] &= clearMask;
    board->colorBitboards[pieceColor[piece]] |= setMask;
    board->colorBitboards[BOTH_COLORS] &= clearMask;
    board->colorBitboards[BOTH_COLORS] |= setMask;
    board->pieces[to] = piece;
    board->pieces[from] = NO_PIECE;
    board->positionKey ^= getPieceHashKey(piece, from);
    board->positionKey ^= getPieceHashKey(piece, to);
}

/*
 * This array is used to update the castlePermissions member of the board
 * struct. Whenever a move is made, this operation:
 * board->castlePermissions &= castlePerms[from] & castlePerms[to];
 * is all that is needed to update the board's castle permissions. Note that
 * most squares (except the starting positions of the kings and rooks) are
 * 0xF, which causes the above operation to have no effect. This is because
 * the castle permissions of a chess board do not change if the rooks/kings
 * are not the pieces that are moving/being taken. Example: castlePerms[A1]
 * is 0xD (1101). If the rook on A1 moves/is taken, board->castlePermissions
 * will be updated from 1111 to 1101 with the above operation, which signifies
 * that white can no longer castle queenside.
 */
static const int castlePerms[64] = {
    0xD, 0xF, 0xF, 0xF, 0xC, 0xF, 0xF, 0xE,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0x7, 0xF, 0xF, 0xF, 0x3, 0xF, 0xF, 0xB,
};

/*
 * Make a move on the chessboard and update the board's member variables. The
 * function movePiece() is called as a part of this function. This function also
 * takes care of captured pieces, promoted pieces, casting, en passant, the
 * fifty move rule, repetition, and anything else related to making a move on
 * the chess board. This is the function that will be called by the alpha-beta
 * algorithm to search for the best move and when the user makes a move in the
 * chess GUI.
 * 
 * board:         The board that is being updated. The board must be a valid
 *                chess position.
 * move:          The move that this function is making. Passed in as a 32-bit
 *                integer containing all the necessary information.
 * 
 * return:        1 if the move that was made was a legal move (did not leave
 *                the king in check), 0 if the move was illegal.
 */
int makeMove(Board* board, int move) {
    assert(checkBoard(board));
    assert(validMove(move));
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    board->history[board->ply].move = move;
    board->history[board->ply].castlePerms = board->castlePerms;
    board->history[board->ply].enPassantSquare = board->enPassantSquare;
    board->history[board->ply].fiftyMoveCount = board->fiftyMoveCount;
    board->history[board->ply++].positionKey = board->positionKey;
    if (board->enPassantSquare != 0ULL) {
        int square = getLSB(board->enPassantSquare);
        board->positionKey ^= getEnPassantHashKey(square);
        board->enPassantSquare = 0ULL;
    }
    if ((move & CAPTURE_FLAG) || board->pieces[from] == WHITE_PAWN
        || board->pieces[from] == BLACK_PAWN) {
        board->fiftyMoveCount = 0;
    } else {
        ++board->fiftyMoveCount;
    }
    board->positionKey ^= getCastleHashKey(board->castlePerms);
    board->castlePerms &= castlePerms[from] & castlePerms[to];
    board->positionKey ^= getCastleHashKey(board->castlePerms);
    switch (move & MOVE_FLAGS) {
        case CAPTURE_FLAG:
            clearPiece(board, to);
            break;
        case CAPTURE_AND_PROMOTION_FLAG:
            clearPiece(board, to);
            // intentional fall through
        case PROMOTION_FLAG:
            clearPiece(board, from);
            addPiece(board, from, (move >> 16) & 0xF);
            break;
        case CASTLE_FLAG:
            switch (to) {
                case G1: movePiece(board, H1, F1); break;
                case C1: movePiece(board, A1, D1); break;
                case G8: movePiece(board, H8, F8); break;
                case C8: movePiece(board, A8, D8); break;
                default: assert(0);
            }
            break;
        case PAWN_START_FLAG:
            board->enPassantSquare = 1ULL << ((to + from) / 2);
            board->positionKey ^= getEnPassantHashKey((to + from) / 2);
            break;
        case EN_PASSANT_FLAG:
            clearPiece(board, to + (board->sideToMove * 16 - 8));
            break;
    }
    movePiece(board, from, to);
    board->positionKey ^= getSideHashKey();
    int king = board->sideToMove == WHITE ? WHITE_KING : BLACK_KING;
    board->sideToMove = !board->sideToMove;
    assert(checkBoard(board));
    if (!squareAttacked(board, board->pieceBitboards[king], board->sideToMove)) {
        return 1;
    }
    undoMove(board);
    return 0;
}

/*
 * Undo the last move that was made on the given board. makeMove() must have
 * been called at least once on this board before this function can be called.
 * Retrieve information about the last move that was made by accessing the
 * board's history array and reset the board's member variables to their
 * previous state. This function is also used by the alpha-beta algorithm to
 * return to a lower depth, allowing other move paths to be explored.
 * 
 * board:      The board that is being updated. Must be a valid board position
 *             with a ply of at least 1.
 */
void undoMove(Board* board) {
    assert(checkBoard(board));
    assert(board->ply > 0);
    board->sideToMove = !board->sideToMove;
    int move = board->history[--board->ply].move;
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    movePiece(board, to, from);
    switch (move & MOVE_FLAGS) {
        case CAPTURE_FLAG:
            addPiece(board, to, (move >> 12) & 0xF);
            break;
        case CAPTURE_AND_PROMOTION_FLAG:
            addPiece(board, to, (move >> 12) & 0xF);
            // intentional fall through
        case PROMOTION_FLAG:
            clearPiece(board, from);
            addPiece(board, from, pieces[board->sideToMove][PAWN]);
            break;
        case CASTLE_FLAG:
            switch (to) {
                case G1: movePiece(board, F1, H1); break;
                case C1: movePiece(board, D1, A1); break;
                case G8: movePiece(board, F8, H8); break;
                case C8: movePiece(board, D8, A8); break;
                default: assert(0);
            }
            break;
        case EN_PASSANT_FLAG:
            addPiece(board, to + (board->sideToMove * 16 - 8),
                pieces[!board->sideToMove][PAWN]);
            break;
    }
    board->castlePerms = board->history[board->ply].castlePerms;
    board->fiftyMoveCount = board->history[board->ply].fiftyMoveCount;
    board->enPassantSquare = board->history[board->ply].enPassantSquare;
    board->positionKey = board->history[board->ply].positionKey;
    assert(checkBoard(board));
}
