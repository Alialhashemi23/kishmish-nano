$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..')
try {
    New-Item -ItemType Directory -Force build | Out-Null
    gcc -std=c99 -Wall -Wextra -Werror src/game.c tests/test_game.c -o build/test-game.exe
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
    & ./build/test-game.exe
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
} finally { Pop-Location }
