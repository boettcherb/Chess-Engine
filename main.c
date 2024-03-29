#include "defs.h"

#include <stdio.h>   // printf, puts, putchar, fflush
#include <string.h>  // strcmp, strlen

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
	SearchInfo info;
	char* fen = "8/8/1k1K4/8/8/8/8/5Q2 w - - 0 1";
    if (!setBoardToFen(&board, fen)) {
        puts("Failed to set board.");
        return -1;
    }
    assert(checkBoard(&board));
    puts("Board set successfully!");

    char input[256];
	int depth = 0, pvDepth = 0;
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
		}
		else if (input[0] == 't') {
			if (depth == 0) {
				puts("Cannot take back a move");
			} else {
				undoMove(&board);
				--depth;
			}		
		}
		else if (input[0] == 's') {
			info.depth = 6;
			searchPosition(&board, &info);
		}
		else {
			input[strlen(input) - 1] = '\0';
			int move = parseMove(&board, input);
			if (move != 0) {
				storeMove(&board.pvTable, move, board.positionKey);
				makeMove(&board, move);
				++depth;
				pvDepth = depth > pvDepth ? depth : pvDepth;
				if (isRepetition(&board)) {
					printf("REPETITION SEEN\n");
                }
			}
			else {
				printf("Invalid Move: %s\n",input);
			}
        }
		fflush(stdin);
	}
	freeHashTable(&board.pvTable);
    return 0;
}
