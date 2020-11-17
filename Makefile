c_files = main.c defs.c board.c movegen.c attack.c magic.c

debug:
	rm -f chess_debug
	gcc -Wpedantic -Wall -Wextra -Og $(c_files) debug.c -o chess_debug

release:
	rm -f chess
	gcc -Wpedantic -Wall -Wextra -O3 $(c_files) -D NDEBUG -o chess

clean:
	rm -f chess_debug chess
