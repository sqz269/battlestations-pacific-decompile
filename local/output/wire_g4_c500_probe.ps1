$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$dev = Join-Path $vs 'Common7/Tools/VsDevCmd.bat'
Push-Location $repo
try {
    $line = 'call "' + $dev + '" -arch=x86 -host_arch=x64 >nul && cl /nologo /std:c++17 /EHsc /MD /O2 /I include local/output/wire_g4_c500_probe.cpp src/unit_weapons.cpp /Fe:local/output/wire_g4_c500_probe.exe /Fo:local/output/ /link /MANIFEST:EMBED'
    & cmd.exe /d /c $line
    if ($LASTEXITCODE -ne 0) { throw 'probe compilation failed' }
    & ./local/output/wire_g4_c500_probe.exe
    if ($LASTEXITCODE -ne 0) { throw "probe failed: $LASTEXITCODE" }
} finally { Pop-Location }
