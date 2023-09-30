@echo off

echo Removing previous compilation...
del perft.exe

echo Compiling chess engine...

set warnings=-Wall -Wextra -Wpedantic -Werror
set c_files=defs.c board.c movegen.c attack.c magic.c hashkey.c hashtable.c libs\tinycthread.c

gcc perft.c %c_files% -O3 %warnings% -DNDEBUG -o perft.exe

if exist perft.exe (
    echo Running perft tests...
    .\perft.exe
) else (
    echo compilation failed! aborting...
)


