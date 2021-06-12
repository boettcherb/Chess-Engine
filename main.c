#include "defs.h"
#include "board.h"
#include "movegen.h"
#include "search.h"

#include <stdio.h>

int parseMove(Board* board, char* input) {
	if (input[1] > '8' || input[1] < '1') return 0;
    if (input[3] > '8' || input[3] < '1') return 0;
    if (input[0] > 'h' || input[0] < 'a') return 0;
    if (input[2] > 'h' || input[2] < 'a') return 0;
    int from = (input[0] - 'a') + 8 * (input[1] - '1');
    int to = (input[2] - 'a') + 8 * (input[3] - '1');
	MoveList list;
    generateAllMoves(board, &list);
    char pieceChar[NUM_PIECE_TYPES] = "pnbrqkpnbrqk";
	for (int i = 0; i < list.numMoves; ++i) {	
		int move = list.moves[i];
		if ((move & 0x3F) == from && ((move >> 6) & 0x3F) == to) {
			int promoted = (move >> 16) & 0xF;
			if (promoted == 0xF || input[4] == pieceChar[promoted]) {
                return move;
            }
		}
    }
    return 0;
}

int main() {
    initializeAll();
    Board board;
    char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (!setBoardToFen(&board, fen)) {
        puts("Failed to set board.");
        return -1;
    }
    assert(checkBoard(&board));
    puts("Board set successfully!");

    char input[6];
	while (1) {
		printPieces(&board);
		printf("\nPlease enter a move > ");
		char* res = fgets(input, 6, stdin);
		assert(res);
		if (input[0] == 'q') {
			break;
		} else if (input[0] == 't') {
			undoMove(&board);			
		} else {
			int move = parseMove(&board, input);
			if (move != 0) {
				makeMove(&board, move);
				if (isRepetition(&board)) {
					printf("REPETITION SEEN\n");
                }
			} else {
				printf("Move Not Parsed: %s\n",input);
			}
        }
		fflush(stdin);
	}
    return 0;
}
