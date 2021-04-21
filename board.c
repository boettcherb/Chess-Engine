#include "board.h"
#include "defs.h"
#include "attack.h"

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
    char layout[70], side, castlePermissions[5], enPassantSquare[3];
    int fiftyMoveCount, moveNumber;
    if (sscanf(fen, "%s %c %s %s %d %d", layout, &side, castlePermissions,
        enPassantSquare, &fiftyMoveCount, &moveNumber) != 6) {
        puts("Error: setBoardToFen: Could not parse or invalid FEN string.");
        return 0;
    }
    int layoutLength = strlen(layout), square = 56, numEmpty;
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
                    puts("Error: setBoardToFen: Invalid character.");
                    return 0;
                }
                square += numEmpty;
        }
    }
    for (square = 0; square < 64; ++square) {
        int piece = board->pieces[square];
        if (board->pieces[square] != NO_PIECE) {
            assert(piece >= 0 && piece < NUM_PIECE_TYPES);
            board->pieceBitboards[piece] |= (1ULL << square);
            board->colorBitboards[pieceColor[piece]] |= (1ULL << square);
            board->colorBitboards[BOTH_COLORS] |= (1ULL << square);
        }
    }
    if (side != 'w' && side != 'b') {
        puts("Error: setBoardToFen: color char must be either 'w' or 'b'.");
        return 0;
    }
    board->sideToMove = side == 'w' ? WHITE : BLACK;
    if (moveNumber < 1) {
        puts("Error: setBoardToFen: moveNumber must be >= 1");
        return 0;
    }
    board->ply = 2 * moveNumber - (side == 'w');
    // TODO: castle permissions, en passant square, fifty move rule
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
}

/*
 * Move a piece from the square 'from' to the square 'to' and update the given
 * board's member variables to reflect the change.
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
}

int makeMove(Board* board, uint64 move) {
    assert(checkBoard(board));
    assert(validMove(move));
    board->history[board->ply].move = move;
    board->history[board->ply++].enPassantSquare = board->enPassantSquare;
    board->enPassantSquare = 0ULL;
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
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
        case PAWN_START_FLAG:
            board->enPassantSquare = 1ULL << ((to + from) / 2);
            break;
        case EN_PASSANT_FLAG:
            clearPiece(board, to + (board->sideToMove * 16 - 8));
            break;
    }
    movePiece(board, from, to);
    int king = board->sideToMove == WHITE ? WHITE_KING : BLACK_KING;
    board->sideToMove = !board->sideToMove;
    assert(checkBoard(board));
    if (!squareAttacked(board, board->pieceBitboards[king], board->sideToMove)) {
        return 1;
    }
    undoMove(board);
    return 0;
}

void undoMove(Board* board) {
    assert(checkBoard(board));
    assert(board->ply > 0);
    board->sideToMove = !board->sideToMove;
    uint64 move = board->history[--board->ply].move;
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
        case EN_PASSANT_FLAG:
            addPiece(board, to + (board->sideToMove * 16 - 8),
                pieces[!board->sideToMove][PAWN]);
            break;
    }
    board->enPassantSquare = board->history[board->ply].enPassantSquare;
    assert(checkBoard(board));
}
