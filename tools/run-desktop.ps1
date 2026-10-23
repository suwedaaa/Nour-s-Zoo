$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
cmake -S $projectRoot -B "$projectRoot/build/desktop" -A x64
if ($LASTEXITCODE -ne 0) { throw 'Desktop configuration failed. Install CMake and Visual Studio C++ Build Tools.' }
cmake --build "$projectRoot/build/desktop" --config Release
if ($LASTEXITCODE -ne 0) { throw 'Desktop build failed.' }
& "$projectRoot/build/desktop/Release/NoursZoo.exe"
