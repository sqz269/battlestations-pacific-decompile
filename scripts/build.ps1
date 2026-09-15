param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [switch]$Diagnostic
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer / vswhere is required.' }
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vsPath) { throw 'Install the MSVC x86/x64 C++ tools.' }
$cmakeExe = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctestExe = Join-Path (Split-Path $cmakeExe) 'ctest.exe'
if (!(Test-Path -LiteralPath $cmakeExe)) { throw 'Install the Visual Studio CMake component.' }
$vsVersion = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
$generator = switch (($vsVersion -split '\.')[0]) {
    '18' { 'Visual Studio 18 2026' }
    '17' { 'Visual Studio 17 2022' }
    default { throw "Unsupported Visual Studio version: $vsVersion" }
}
$buildPath = Join-Path $repoRoot 'build\win32'
$startupRegistry = Join-Path $repoRoot 'cmake\startup.cmake'
& $cmakeExe -S $repoRoot -B $buildPath -G $generator -A Win32 "-DCMAKE_GENERATOR_INSTANCE=$vsPath" "-DCMAKE_PROJECT_INCLUDE=$startupRegistry"
if ($LASTEXITCODE) { throw 'CMake configure failed.' }
if ($Diagnostic) {
    & $cmakeExe --build $buildPath --config $Configuration --parallel -- /v:diag /nodeReuse:false
} else {
    & $cmakeExe --build $buildPath --config $Configuration --parallel
}
if ($LASTEXITCODE) { throw 'C++ build failed.' }
& $ctestExe --test-dir $buildPath -C $Configuration --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed.' }
