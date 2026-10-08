param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [ValidateRange(1,2147483647)][int]$Frames = 300,
    [string]$GameRoot = '',
    [string]$RuntimeDirectory = '',
    [string]$SettingsPersonalRoot = '',
    [string]$LogPath = '',
    [string[]]$GameArguments = @()
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (!$GameRoot) {
    $target = Get-Content -LiteralPath (Join-Path $repoRoot 'config/target.json') -Raw | ConvertFrom-Json
    $GameRoot = Split-Path $target.binary -Parent
}
$GameRoot = [IO.Path]::GetFullPath($GameRoot)
if (!$RuntimeDirectory) { $RuntimeDirectory = Join-Path $repoRoot 'local/gfwl-private-runtime' }
$RuntimeDirectory = [IO.Path]::GetFullPath($RuntimeDirectory)
if (!$SettingsPersonalRoot) { $SettingsPersonalRoot = Join-Path $repoRoot 'local/run-personal' }
$SettingsPersonalRoot = [IO.Path]::GetFullPath($SettingsPersonalRoot)
if (!$LogPath) { $LogPath = Join-Path $repoRoot 'local/run/game.log' }
$LogPath = [IO.Path]::GetFullPath($LogPath)
$gameExecutable = Join-Path $repoRoot "build/win32/$Configuration/bsp_game.exe"
if (!(Test-Path -LiteralPath $gameExecutable)) { throw 'Build the Win32 game with scripts/build.ps1 first.' }
& python (Join-Path $repoRoot 'tools/prepare_xlive_runtime.py') --game-root $GameRoot --output $RuntimeDirectory
if ($LASTEXITCODE) { throw 'Private XLive runtime preparation failed.' }
New-Item -ItemType Directory -Path (Join-Path $SettingsPersonalRoot 'Battlestations-Pacific') -Force | Out-Null
New-Item -ItemType Directory -Path (Split-Path $LogPath -Parent) -Force | Out-Null
$launchArguments = @('--game-root', $GameRoot, '--frames', "$Frames", '--log', $LogPath,
    '--settings-personal-root', $SettingsPersonalRoot,
    '--xlive-dll', (Join-Path $RuntimeDirectory 'xlive.dll'),
    '--xlive-dependency', (Join-Path $RuntimeDirectory 'msidcrl40.dll'),
    '--window-monitor', 'smallest', '--window-resolution', 'fit', '--present-interval', 'immediate')
$launchArguments += $GameArguments
# A GUI executable can return to PowerShell before it exits. Wait explicitly and
# use the actual process exit code, preserving each argument without shell parsing.
$start = [Diagnostics.ProcessStartInfo]::new()
$start.FileName = $gameExecutable
$start.WorkingDirectory = $repoRoot
$start.UseShellExecute = $false
foreach ($launchArgument in $launchArguments) { $start.ArgumentList.Add($launchArgument) }
$process = [Diagnostics.Process]::Start($start)
$process.WaitForExit()
$gameExitCode = $process.ExitCode
$process.Dispose()
Write-Host "bsp_game exited $gameExitCode; log: $LogPath"
exit $gameExitCode
