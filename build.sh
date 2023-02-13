#!/bin/bash

EXTERNAL="external"
GLADINC="$EXTERNAL/glad/include"
GLADSRC="$EXTERNAL/glad/src"

CC="cc"
EXEC="main"
CFILES="$GLADSRC/gl.c main.c"
CINCLUDES="-I$EXTERNAL -I$GLADINC"
CFLAGS="-Wall -Wpedantic -Wshadow -std=c99"
CLIBS="-lSDL2 -lSDL2main -lGL -lGLEW "
CDEFINES=""

debug () {
    CFLAGS="$CFLAGS -ggdb"
    echo "$CC $CFLAGS $CINCLUDES $CDEFINES $CFILES -o $EXEC $CLIBS"
    $CC $CFLAGS $CINCLUDES $CDEFINES $CFILES -o $EXEC $CLIBS
}

release () {
    CFLAGS="$CFLAGS -march=native -O2"
    echo "$CC $CFLAGS $CINCLUDES $CDEFINES $CFILES -o $EXEC $CLIBS"
    $CC $CFLAGS $CINCLUDES $CDEFINES $CFILES -o $EXEC $CLIBS
}
default () {
    release
}

if [ "${1,,}" == "debug" ]; then
    debug
elif [ "${1,,}" == "release" ]; then
    release
else
    default
fi

echo "./$EXEC"
./$EXEC
