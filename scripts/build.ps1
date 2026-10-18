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

# Machine-wide build slots. Every worktree builds through this script, so it admits at most
# BSP_BUILD_SLOTS builds at once (default 3) and the rest wait here. A slot is an exclusively opened
# file under ~/.bsp; the handle closes when this process exits, so a killed build frees its slot.
# MSBuild gets 4 nodes: there are only a handful of targets, and cl /MP (CMakeLists.txt) does the fan-out.
$slotCount = if ($env:BSP_BUILD_SLOTS) { [int]$env:BSP_BUILD_SLOTS } else { 3 }
$slotDir = Join-Path $env:USERPROFILE '.bsp'
New-Item -ItemType Directory -Force -Path $slotDir | Out-Null
$slot = $null
$announced = $false
while (-not $slot) {
    for ($i = 0; $i -lt $slotCount -and -not $slot; $i++) {
        try {
            $slot = [System.IO.File]::Open((Join-Path $slotDir "build.slot$i.lock"), 'OpenOrCreate', 'ReadWrite', 'None')
        } catch [System.IO.IOException] { }
    }
    if (-not $slot) {
        if (-not $announced) { Write-Host "waiting for one of $slotCount build slots ($slotDir\build.slot*.lock)"; $announced = $true }
        Start-Sleep -Seconds 5
    }
}
try {
    & $cmakeExe -S $repoRoot -B $buildPath -G $generator -A Win32 "-DCMAKE_GENERATOR_INSTANCE=$vsPath" "-DCMAKE_PROJECT_INCLUDE=$startupRegistry"
    if ($LASTEXITCODE) { throw 'CMake configure failed.' }
    if ($Diagnostic) {
        & $cmakeExe --build $buildPath --config $Configuration --parallel 4 -- /v:diag /nodeReuse:false
    } else {
        & $cmakeExe --build $buildPath --config $Configuration --parallel 4
    }
    if ($LASTEXITCODE) { throw 'C++ build failed.' }
} finally {
    $slot.Dispose()
}
& $ctestExe --test-dir $buildPath -C $Configuration --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed.' }
