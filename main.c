#include "defs.h"
#include "board.h"

#include <stdio.h>

int main() {
    Board board;
    char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (!setBoardToFen(&board, fen)) {
        puts("Failed to set board.");
    } else {
        assert(checkBoard(&board));
        puts("Board set successfully!");
        printf("side to move: %s\n", board.sideToMove ? "Black" : "White");
        puts("board:");
        printPieces(&board);
    }
    return 0;
}
