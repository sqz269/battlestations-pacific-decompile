param([string]$SchemaPath = '')
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
if (!$SchemaPath) {
    $target = Get-Content -LiteralPath (Join-Path $repo 'config/target.json') -Raw | ConvertFrom-Json
    $SchemaPath = Join-Path (Split-Path $target.binary -Parent) 'universe/library/scene.props'
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vsPath) { throw 'MSVC x86/x64 tools are required.' }
$output = Join-Path $repo 'local/output'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$objects = @(Get-ChildItem -LiteralPath (Join-Path $repo 'build/win32/bsp_game.dir/Release') -Filter '*.obj' |
    Where-Object Name -ne 'game_main.obj')
if (!$objects.Count) { throw 'Run scripts/build.ps1 first.' }
$objects += @(Get-ChildItem -LiteralPath (Join-Path $repo 'build/win32/bsp_core.dir/Release') -Filter 'cmake_pch*.obj')
$inputs = @($objects | ForEach-Object FullName)
foreach ($library in @('bsp_core', 'bsp_lua511', 'bsp_zlib121')) {
    $inputs += Join-Path $repo "build/win32/Release/$library.lib"
}
$response = Join-Path $output 'c760-postprocess-probe.rsp'
@($inputs | ForEach-Object { '"' + $_ + '"' }) +
    @('d3d9.lib', 'user32.lib', 'gdi32.lib', 'ole32.lib', 'advapi32.lib', 'shell32.lib', 'PowrProf.lib') |
    Set-Content -LiteralPath $response -Encoding ascii
$command = Join-Path $output 'c760-postprocess-probe.cmd'
$vcvars = Join-Path $vsPath 'VC/Auxiliary/Build/vcvarsall.bat'
@('@echo off', ('call "' + $vcvars + '" x86 >nul'), 'if errorlevel 1 exit /b 1',
    'cl /nologo /EHsc /std:c++17 /O2 /MD /Gy /Iinclude tools/c760_postprocess_probe.cpp @local/output/c760-postprocess-probe.rsp /Folocal/output/c760-postprocess-probe.obj /link /OPT:REF /MANIFEST:EMBED /OUT:local/output/c760-postprocess-probe.exe') |
    Set-Content -LiteralPath $command -Encoding ascii
Push-Location $repo
try {
    & $env:ComSpec /d /c $command
    if ($LASTEXITCODE) { throw 'Post-process probe build failed.' }
    # Dedicated console main: reads the schema and exercises the adapter only.
    # game_main.obj is excluded; no game loop, window or renderer is started.
    & (Join-Path $output 'c760-postprocess-probe.exe') $SchemaPath
    if ($LASTEXITCODE) { throw 'Post-process probe failed.' }
} finally { Pop-Location }
