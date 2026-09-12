# Builds the offline XLive stand-in (tools/xlive_stub) to
# build/win32/Release/xlive_stub.dll with the Win32 x86 MSVC toolset.
#
# The CMake build registers the same target through cmake/startup.cmake, so this
# script exists for the case where only the stand-in is wanted: it does not
# configure or build bsp_game, and it writes into the same output directory the
# CMake Release build uses so that --xlive-dll takes the same path either way.
#
# Ad hoc rules this obeys (AGENTS.md): the link embeds a manifest, and the
# output name contains none of install/setup/update/patch, which Windows UAC
# installer detection would otherwise treat as an elevation request and block
# under an unattended run.

param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [switch]$KeepIntermediates
)
$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path $PSScriptRoot -Parent
$source = Join-Path $repoRoot 'tools\xlive_stub\xlive_stub.cpp'
$moduleDefinition = Join-Path $repoRoot 'tools\xlive_stub\xlive_stub.def'
foreach ($required in @($source, $moduleDefinition)) {
    if (!(Test-Path -LiteralPath $required)) { throw "Missing $required" }
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer / vswhere is required.' }
$vsPath = & $vswhere -latest -products '*' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vsPath) { throw 'Install the MSVC x86/x64 C++ tools.' }

# The x86 host/target pair: the original XLive ABI is Win32 and the host
# executable that loads this DLL is Win32.
$vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars32.bat'
if (!(Test-Path -LiteralPath $vcvars)) { throw "Missing $vcvars" }

$outputDir = Join-Path $repoRoot "build\win32\$Configuration"
$null = New-Item -ItemType Directory -Force -Path $outputDir
$objDir = Join-Path $repoRoot "build\win32\xlive_stub_$Configuration"
$null = New-Item -ItemType Directory -Force -Path $objDir

$dll = Join-Path $outputDir 'xlive_stub.dll'
$optimization = if ($Configuration -eq 'Debug') { '/Od /Zi /MTd' } else { '/O2 /MT' }

# /W4 /WX matches the project build. /GS- is deliberate: nothing here takes a
# buffer, and a stand-in loaded into a foreign process should add no CRT
# security-cookie setup of its own beyond what the loader already ran.
# No argument may end in a backslash: cmd.exe reads the trailing \" of a quoted
# path as an escaped quote and swallows the rest of the command line, which is
# why the object file is named in full instead of by output directory.
$objectFile = Join-Path $objDir 'xlive_stub.obj'
$clArguments = @(
    '/nologo', '/c', '/EHsc', '/W4', '/WX', '/std:c++17', '/permissive-'
) + ($optimization -split ' ') + @(
    "/Fo$objectFile", "/Fd$objDir\xlive_stub.pdb", $source
)

$linkArguments = @(
    '/nologo', '/DLL', '/MANIFEST:EMBED', '/MACHINE:X86',
    "/DEF:$moduleDefinition", "/OUT:$dll",
    "/IMPLIB:$objDir\xlive_stub.lib", "/PDB:$objDir\xlive_stub_link.pdb",
    $objectFile, 'kernel32.lib'
)
if ($Configuration -eq 'Debug') { $linkArguments += '/DEBUG' }

function Invoke-Tool([string]$tool, [string[]]$toolArguments) {
    # vcvars32 has to run in the same cmd instance as the tool for INCLUDE/LIB.
    $quoted = ($toolArguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $line = "call `"$vcvars`" >nul && $tool $quoted"
    & cmd.exe /c $line
    if ($LASTEXITCODE) { throw "$tool failed with exit code $LASTEXITCODE" }
}

Invoke-Tool 'cl.exe' $clArguments
Invoke-Tool 'link.exe' $linkArguments

if (!(Test-Path -LiteralPath $dll)) { throw "The link produced no $dll" }
if (!$KeepIntermediates) { Remove-Item -Recurse -Force -LiteralPath $objDir }

Write-Output $dll
