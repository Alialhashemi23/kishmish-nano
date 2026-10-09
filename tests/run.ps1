$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..')
try {
    New-Item -ItemType Directory -Force build | Out-Null
    gcc -std=c99 -Wall -Wextra -Werror src/game.c src/personality.c tests/test_game.c -o build/test-game.exe
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
    & ./build/test-game.exe
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
    gcc -std=c99 -Wall -Wextra -Werror src/game.c src/personality.c tests/test_personality.c -o build/test-personality.exe
    if ($LASTEXITCODE -ne 0) { throw 'Personality test compilation failed' }
    & ./build/test-personality.exe
    if ($LASTEXITCODE -ne 0) { throw 'Personality tests failed' }
    python tests/test_assets.py
    if ($LASTEXITCODE -ne 0) { throw 'Audio asset tests failed' }
} finally { Pop-Location }
