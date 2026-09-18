# Serialized launcher for bsp_game.exe.
#
# The executable is single-instance (WinMain's mutex, 008f8301) and every run
# is two processes: the native-data bootstrap resumes a suspended copy of the
# executable as the real run. A second concurrent run on this machine leaves
# its child on the modal "already running" box and its parent waiting, with no
# way to finish unattended, and the stuck child keeps every later run
# colliding until it is killed. Several agents run the executable from their
# own worktrees, so this wrapper takes a machine-wide lock file, waits for any
# live bsp_game.exe, runs the executable through a pipe (a WIN32-subsystem exe
# returns immediately otherwise) and releases the lock.
#
#   ./tools/run_game.ps1 -Log local\usn02.log -- --frames 3200 --press-start-frame 30 `
#       --menu-select USN02 --mission-frames 3000 --mission-frame-seconds 0.05
#
# -Exe defaults to build\win32\Release\bsp_game.exe under the current
# directory, -GameRoot to the installed game and -XLiveDll to the stand-in
# built next to the executable; every argument after `--` is passed through.
# Run it from the worktree root. If a run is left hanging (log stops near
# 3 KB, a bsp_game window titled "Error"), stop both bsp_game processes and
# delete the lock file yourself; this script never kills a process, because
# the live one may be another agent's run.
[CmdletBinding(PositionalBinding=$false)]
param(
    [string]$Exe = 'build\win32\Release\bsp_game.exe',
    [string]$GameRoot = 'I:/SteamLibrary/steamapps/common/Battlestations Pacific',
    [string]$XLiveDll = '',
    [string]$Log = 'local\game_run.log',
    [string]$Lock = "$env:USERPROFILE\.bsp\bsp_game.lock",
    [int]$WaitSeconds = 900,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$GameArgs = @()
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path $Exe)) { throw "bsp_game.exe not found at $Exe (run from the worktree root after a build)" }
if ($XLiveDll -eq '') { $XLiveDll = Join-Path (Split-Path $Exe -Parent) 'xlive_stub.dll' }
if (-not (Test-Path $XLiveDll)) { throw "XLive stand-in not found at $XLiveDll" }
$GameArgs = @($GameArgs | Where-Object { $_ -ne '--' })
$owner = if ($env:BSP_AGENT) { $env:BSP_AGENT } else { Split-Path (Get-Location) -Leaf }

$lockDir = Split-Path $Lock -Parent
if (-not (Test-Path $lockDir)) { New-Item -ItemType Directory -Path $lockDir | Out-Null }
$deadline = (Get-Date).AddSeconds($WaitSeconds)
while ($true) {
    $holder = if (Test-Path $Lock) { (Get-Content $Lock -ErrorAction SilentlyContinue) -join ' ' } else { $null }
    $live = Get-Process bsp_game -ErrorAction SilentlyContinue
    if (-not $holder -and -not $live) { break }
    if ((Get-Date) -gt $deadline) {
        throw "gave up after $WaitSeconds s: lock held by '$holder', live bsp_game pids: $(($live | ForEach-Object Id) -join ',')"
    }
    Start-Sleep -Seconds 10
}
Set-Content -Path $Lock -Value "$owner $(Get-Date -Format s) pid=$PID" -Encoding ascii
try {
    $logDir = Split-Path $Log -Parent
    if ($logDir -and -not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir | Out-Null }
    $arguments = @('--game-root', $GameRoot, '--xlive-dll', $XLiveDll, '--log', $Log) + $GameArgs
    & $Exe @arguments 2>&1 | Select-Object -Last 1
    $code = $LASTEXITCODE
    "EXITCODE=$code"
    # The parent can return while its native-data child is still tearing down
    # (a lost D3D device at shutdown has been seen), and a survivor blocks the
    # next launch through the single-instance mutex. Wait for this run's own
    # processes, identified by their path under this worktree, then stop any
    # that remain. Processes from other worktrees are never touched.
    $exeRoot = (Resolve-Path $Exe).Path
    $treeRoot = (Get-Location).Path
    $waited = 0
    while ($waited -lt 30) {
        $mine = Get-Process bsp_game -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($treeRoot, [System.StringComparison]::OrdinalIgnoreCase) }
        if (-not $mine) { break }
        Start-Sleep -Seconds 1; $waited += 1
    }
    if ($mine) {
        $mine | Stop-Process -Force -ErrorAction SilentlyContinue
        "stopped lingering bsp_game pids after ${waited}s: $(($mine | ForEach-Object Id) -join ',')"
    }
    if (Test-Path $Log) {
        Select-String -Path $Log -Pattern '^summary mission gunnery damage|^summary mission pilot attack|^summary window_created|^startup failed|^host methods' |
            ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) }
    } else {
        "NO LOG at $Log (the executable exits 2 before opening it when an argument is wrong)"
    }
    exit $code
} finally {
    Remove-Item $Lock -ErrorAction SilentlyContinue
}
