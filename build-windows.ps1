$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    gcc -std=c99 -Wall -Wextra -Werror -O2 src/game.c src/personality.c src/main.c -o build/Kishmish-Windows.exe -lgdi32 -luser32 -lwinmm
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed' }
    Copy-Item -Recurse -Force package/sounds build/
    Write-Output 'Built build/Kishmish-Windows.exe'
} finally { Pop-Location }
