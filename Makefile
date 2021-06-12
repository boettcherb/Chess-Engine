c_files = defs.c board.c movegen.c attack.c magic.c hash.c search.c

debug:
	rm -f chess_debug
	gcc -Wpedantic -Wall -Wextra -Og $(c_files) main.c debug.c -o chess_debug

release:
	rm -f chess
	gcc -Wpedantic -Wall -Wextra -O3 $(c_files) main.c -D NDEBUG -o chess

perft:
	rm -f perft
	gcc -Wpedantic -Wall -Wextra -O3 $(c_files) perft.c -D NDEBUG -o perft -lpthread -DPERFT_MULTITHREADED

perft_debug:
	rm -f perft_debug
	gcc -Wpedantic -Wall -Wextra -Og $(c_files) perft.c debug.c -o perft_debug -lpthread

clean:
	rm -f chess_debug chess perft perft_debug
