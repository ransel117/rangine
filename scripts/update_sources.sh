#!/bin/bash

SOURCE="$PWD"
EXTERNAL="$SOURCE/external"

echo "----- Get latest version of ranmath -----"
cd "$EXTERNAL/ranmath"
git pull
cd $SOURCE

command -v glad >/dev/null 2>&1 || { echo >&2 "I require glad but it's not installed.\nAborting..."; exit 1; }

echo "--------- Refresh opengl loader ---------"
cd "$EXTERNAL"
rm -r glad
glad --api gl:core=3.3 --extensions "" --out-path glad c
cd $SOURCE
