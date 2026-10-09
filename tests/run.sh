#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
${HOST_CC:-gcc} -std=c99 -Wall -Wextra -Werror src/game.c tests/test_game.c -o build/test-game
./build/test-game
