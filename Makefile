c_files = defs.c board.c movegen.c attack.c magic.c hash.c search.c
all_warnings = -Wall -Wextra -Wpedantic -Werror

debug:
	rm -f chess_debug
	gcc $(all_warnings) -Og $(c_files) main.c debug.c -o chess_debug

release:
	rm -f chess
	gcc $(all_warnings) -O3 $(c_files) main.c -D NDEBUG -o chess

perft:
	rm -f perft
	gcc $(all_warnings) -O3 $(c_files) perft.c -D NDEBUG -o perft -lpthread -DPERFT_MULTITHREADED

perft_debug:
	rm -f perft_debug
	gcc $(all_warnings) -Og $(c_files) perft.c debug.c -o perft_debug

clean:
	rm -f chess_debug chess perft perft_debug
