#!/bin/sh
cd "$(dirname "$0")" || exit 1
SAVE_ROOT="${HOME:-/mnt/Anbernic}/.kishmish-nano"
mkdir -p "$SAVE_ROOT"
exec >"$SAVE_ROOT/boot.log" 2>&1
printf 'Kishmish bundled-runtime build\n'
date
pwd
ls -l kishmish lib/
./lib/ld-musl-armhf.so.1 --library-path ./lib ./kishmish
STATUS=$?
printf 'Exit status: %s\n' "$STATUS"
exit "$STATUS"
