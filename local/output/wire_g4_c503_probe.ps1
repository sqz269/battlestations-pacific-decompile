$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
Import-Module (Join-Path $vsPath 'Common7/Tools/Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x86 -host_arch=x64' | Out-Null
& cl /nologo /std:c++17 /EHsc /O2 /MD /Iinclude /Ithird_party/lua-5.1.1/src `
    local/output/wire_g4_c503_probe.cpp `
    build/win32/Release/bsp_core.lib build/win32/Release/bsp_lua511.lib build/win32/Release/bsp_zlib121.lib `
    /Felocal/output/wire_g4_c503_probe.exe /Folocal/output/wire_g4_c503_probe.obj `
    /link /MANIFEST:EMBED d3d9.lib user32.lib gdi32.lib ole32.lib advapi32.lib shell32.lib PowrProf.lib
if ($LASTEXITCODE) { throw 'chain503 constructor probe build failed' }
& ./local/output/wire_g4_c503_probe.exe
if ($LASTEXITCODE) { throw 'chain503 constructor probe failed' }
