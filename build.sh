#!/bin/bash

MUSL_ROOT=$(find /usr -iname 'musl' -type d -not -path '*/\.*')
MUSL_INC="$MUSL_ROOT/include"
MUSL_LIB="$MUSL_ROOT/lib"
MUSL_BIN="$MUSL_ROOT/bin"

EXTERNAL="external"
GLADDIR="$EXTERNAL/glad"
GLADINC="$GLADDIR/include"
GLADSRC="$GLADDIR/src"

CC="$MUSL_BIN/musl-gcc"
CFLAGS="-Wall -Wpedantic -ggdb -std=c99"
CDEFINES=""
CINCLUDES="-I$EXTERNAL -I$GLADINC"
CFILES="$GLADSRC/gl.c main.c"
CLIBS="-lSDL2 -lSDL2main -lGL -lGLEW"

EXEC="main"

$CC $CFLAGS $CDEFINES $CINCLUDES $CFILES -o $EXEC $CLIBS
./$EXEC
