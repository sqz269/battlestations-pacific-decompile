# A from-scratch configure + build of the whole reconstruction into a directory
# of its own, so it cannot reuse anything in build\win32 and cannot disturb a
# peer agent building there. Mirrors scripts/build.ps1's arguments exactly.
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$cmakeExe = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctestExe = Join-Path (Split-Path $cmakeExe) 'ctest.exe'
$vsVersion = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
$generator = switch (($vsVersion -split '\.')[0]) {
    '18' { 'Visual Studio 18 2026' }
    '17' { 'Visual Studio 17 2022' }
    default { throw "Unsupported Visual Studio version: $vsVersion" }
}
$buildPath = Join-Path $repoRoot 'build\clean-verify'
if (Test-Path -LiteralPath $buildPath) { Remove-Item -Recurse -Force -LiteralPath $buildPath }
$startupRegistry = Join-Path $repoRoot 'cmake\startup.cmake'
& $cmakeExe -S $repoRoot -B $buildPath -G $generator -A Win32 "-DCMAKE_GENERATOR_INSTANCE=$vsPath" "-DCMAKE_PROJECT_INCLUDE=$startupRegistry"
if ($LASTEXITCODE) { throw 'CMake configure failed.' }
& $cmakeExe --build $buildPath --config Release --parallel
if ($LASTEXITCODE) { throw 'C++ build failed.' }
& $ctestExe --test-dir $buildPath -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed.' }
Write-Output "CLEAN BUILD OK"
Get-ChildItem (Join-Path $buildPath 'Release\bsp_game.exe') | ForEach-Object { Write-Output ("exe: {0} {1} bytes {2}" -f $_.Name, $_.Length, $_.LastWriteTime) }
