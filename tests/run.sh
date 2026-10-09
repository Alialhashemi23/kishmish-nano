#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
${HOST_CC:-gcc} -std=c99 -Wall -Wextra -Werror src/game.c src/personality.c tests/test_game.c -o build/test-game
./build/test-game

${HOST_CC:-gcc} -std=c99 -Wall -Wextra -Werror src/game.c src/personality.c tests/test_personality.c -o build/test-personality
./build/test-personality
python3 tests/test_assets.py
