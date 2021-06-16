#include "defs.h"
#include "board.h"
#include "movegen.h"
#include "search.h"

#include <stdio.h>
#include <string.h>

int parseMove(const Board* board, char* input) {
	MoveList list;
	generateAllMoves(board, &list);
	for (int i = 0; i < list.numMoves; ++i) {
		char moveString[6];
		getMoveString(list.moves[i], moveString);
		if (strcmp(input, moveString) == 0) {
			return list.moves[i];
		}
	}
	return 0;
}

int main() {
    initializeAll();
    Board board;
	char* fen = "3k1r2/6P1/8/8/8/8/8/3K4 w - - 0 1";
    if (!setBoardToFen(&board, fen)) {
        puts("Failed to set board.");
        return -1;
    }
    assert(checkBoard(&board));
    puts("Board set successfully!");

    char input[256];
	int depth = 0;
	while (1) {
		printf("\nside to move: %s\n", board.sideToMove == WHITE ? "WHITE" : "BLACK");
		printPieces(&board);
		printf("\nPlease enter a move > ");
		char* ret_val = fgets(input, 255, stdin);
		if (ret_val == NULL) {
			printf("ERROR: Failed to read input\n");
			return -1;
		}
		if (input[0] == 'q') {
			break;
		} else if (input[0] == 't') {
			if (depth == 0) {
				puts("Cannot take back a move");
			} else {
				undoMove(&board);
				--depth;
			}		
		} else {
			input[strlen(input) - 1] = '\0';
			int move = parseMove(&board, input);
			if (move != 0) {
				makeMove(&board, move);
				++depth;
				if (isRepetition(&board)) {
					printf("REPETITION SEEN\n");
                }
			} else {
				printf("Invalid Move: %s\n",input);
			}
        }
		fflush(stdin);
	}
    return 0;
}
