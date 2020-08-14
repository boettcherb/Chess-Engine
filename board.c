#include "board.h"
#include "defs.h"

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
