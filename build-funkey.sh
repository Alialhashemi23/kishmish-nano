#!/bin/sh
set -eu
# Run after sourcing the official FunKey SDK environment-setup file.
: "${CC:?Source the FunKey SDK environment-setup first}"
mkdir -p build/opk/lib
SDK_SYSROOT=$($CC -print-sysroot)
cp "$SDK_SYSROOT/lib/libc.so" build/opk/lib/libc.so
cp "$SDK_SYSROOT/lib/libc.so" build/opk/lib/ld-musl-armhf.so.1
cp -L "$SDK_SYSROOT/usr/lib/libSDL-1.2.so.0" build/opk/lib/
chmod +x build/opk/lib/ld-musl-armhf.so.1
$CC ${CFLAGS:-} -std=c99 -Wall -Wextra -O2 src/game.c src/personality.c src/main.c -o build/opk/kishmish -lSDL
cp package/kishmish.funkey-s.desktop package/kishmish.anbernic.desktop build/opk/
cp package/kishmish.png package/launch.sh build/opk/
chmod +x build/opk/launch.sh
cp -r package/sounds build/opk/
mkdir -p build/opk/licenses
cp licenses/* build/opk/licenses/
cp LICENSE THIRD_PARTY.md build/opk/licenses/
mksquashfs build/opk build/kishmish-nano.opk -all-root -noappend -comp gzip
echo 'Built build/kishmish-nano.opk'
