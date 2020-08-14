#include "defs.h"
#include "board.h"

#include <stdio.h>

int main() {
    Board board;
    char pieceChar[NUM_PIECE_TYPES] = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'
    };
    char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (!setBoardToFen(&board, fen)) {
        puts("Failed to set board.");
    } else {
        assert(checkBoard(&board));
        puts("Board set successfully!");
        printf("side to move: %s\n", board.sideToMove ? "Black" : "White");
        puts("board:");
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                // This method to calculate square makes sure the ranks are
                // printed in decreasing order (normally rank 8 is at the top).
                int square = 56 - i * 8 + j;
                assert(square >= 0 && square < 64);
                if (board.pieces[square] == NO_PIECE) {
                    printf("- ");
                } else {
                    printf("%c ", pieceChar[board.pieces[square]]);
                }
            }
            putchar('\n');
        }
    }

    return 0;
}
