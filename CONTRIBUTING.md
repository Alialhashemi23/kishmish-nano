# Contributing

Keep the game readable on a 240×240 screen and playable with the D-pad, A, B, and Start.

1. Build the Windows preview or SDL backend.
2. Run `sh tests/run.sh` on Linux, or `./tests/run.ps1` in PowerShell.
3. Format C files with `clang-format -i src/*.c src/*.h tests/*.c tools/*.c`.
4. For hardware changes, report firmware, launcher, controls, and whether saves survive a restart.

Do not commit personal saves, logs, SDK archives, OPKs, compiled executables, or extracted system libraries. Put downloadable builds in GitHub Releases. Keep hardware observations separate from emulator or desktop results.
