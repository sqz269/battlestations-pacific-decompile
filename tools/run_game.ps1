# Serialized launcher for bsp_game.exe.
#
# The executable is single-instance (WinMain's mutex, 008f8301) and every run
# is two processes: the native-data bootstrap resumes a suspended copy of the
# executable as the real run. A second concurrent run on this machine leaves
# its child on the modal "already running" box and its parent waiting, with no
# way to finish unattended, and the stuck child keeps every later run
# colliding until it is killed. Several agents run the executable from their
# own worktrees. Since 2026-09-18 the executable takes two harness options,
# --instance-tag (its own mutex name) and --affinity-core (its own processor in
# place of the image's processor 0), so runs can overlap: this wrapper takes one
# of -Slots numbered slot files, passes that slot's tag and processor, runs the
# executable through a pipe (a WIN32-subsystem exe returns immediately
# otherwise) and releases the slot. Each run must have its own -Log path: the
# log is opened for writing at that path and two runs on one path would clobber
# each other, so the wrapper refuses a path another live slot is using.
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
    [int]$WaitSeconds = 2400,
    [int]$Slots = 3,
    [int[]]$Cores = @(2, 4, 6),
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
if ($Slots -lt 1 -or $Cores.Count -lt $Slots) { throw "-Cores must list at least -Slots processors" }
$logDirEarly = Split-Path $Log -Parent
if ($logDirEarly -and -not (Test-Path $logDirEarly)) { New-Item -ItemType Directory -Path $logDirEarly | Out-Null }
$logFull = if ([System.IO.Path]::IsPathRooted($Log)) { [System.IO.Path]::GetFullPath($Log) } else { [System.IO.Path]::GetFullPath((Join-Path (Get-Location).Path $Log)) }

function Get-TaggedGame([string]$tagName) {
    # Both processes of a run carry the tag: the bootstrap parent passes its
    # arguments through to the child it resumes.
    # The child's arguments are re-quoted by the bootstrap ("--instance-tag" "slot0").
    $pattern = '--instance-tag"?\s+"?' + [regex]::Escape($tagName) + '("|\s|$)'
    Get-CimInstance Win32_Process -Filter "Name='bsp_game.exe'" -ErrorAction SilentlyContinue |
        Where-Object { $_.CommandLine -match $pattern }
}

$deadline = (Get-Date).AddSeconds($WaitSeconds)
$slot = -1
$slotFile = $null
while ($slot -lt 0) {
    for ($k = 0; $k -lt $Slots; $k++) {
        $candidate = Join-Path $lockDir "bsp_game.slot$k.lock"
        if (Test-Path $candidate) {
            $text = (Get-Content $candidate -ErrorAction SilentlyContinue) -join ' '
            if ($text -match 'log=(.+)$' -and $Matches[1].Trim() -ieq $logFull) {
                throw "log path $logFull is in use by slot $k ($text); give every run its own -Log"
            }
            # A slot whose launcher is gone and whose tagged processes are gone is stale.
            $holderAlive = $false
            if ($text -match 'pid=(\d+)') { $holderAlive = [bool](Get-Process -Id ([int]$Matches[1]) -ErrorAction SilentlyContinue) }
            if (-not $holderAlive -and -not (Get-TaggedGame "slot$k")) { Remove-Item $candidate -ErrorAction SilentlyContinue }
        }
        if (-not (Test-Path $candidate) -and -not (Get-TaggedGame "slot$k")) {
            try {
                # Atomic CreateNew: a launcher that loses the race sees the IOException and tries the next slot.
                $stream = [System.IO.File]::Open($candidate, [System.IO.FileMode]::CreateNew, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
                $bytes = [System.Text.Encoding]::ASCII.GetBytes("$owner $(Get-Date -Format s) pid=$PID log=$logFull")
                $stream.Write($bytes, 0, $bytes.Length)
                $stream.Close()
                $slot = $k; $slotFile = $candidate
                break
            } catch [System.IO.IOException] { }
        }
    }
    if ($slot -ge 0) { break }
    if ((Get-Date) -gt $deadline) { throw "gave up after $WaitSeconds s: all $Slots slots busy" }
    Start-Sleep -Seconds 10
}
$tag = "slot$slot"
$core = $Cores[$slot]
"slot=$slot tag=$tag core=$core log=$logFull"
try {
    $logDir = Split-Path $Log -Parent
    if ($logDir -and -not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir | Out-Null }
    $arguments = @('--game-root', $GameRoot, '--xlive-dll', $XLiveDll, '--log', $Log, '--instance-tag', $tag, '--affinity-core', "$core") + $GameArgs
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
    # A worker has seen the pipe close while this run's processes were still
    # alive, so the wait is long: a live mission run is never killed inside it.
    # Twenty minutes is past any 3200-frame run; only a wedged process is stopped.
    $waited = 0
    while ($waited -lt 1200) {
        $mine = Get-TaggedGame $tag
        if (-not $mine) { break }
        Start-Sleep -Seconds 2; $waited += 2
    }
    if ($mine) {
        $mine | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        "stopped lingering bsp_game pids after ${waited}s: $(($mine | ForEach-Object ProcessId) -join ',')"
    }
    if (Test-Path $Log) {
        Select-String -Path $Log -Pattern '^summary mission gunnery damage|^summary mission pilot attack|^summary window_created|^startup failed|^host methods' |
            ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) }
    } else {
        "NO LOG at $Log (the executable exits 2 before opening it when an argument is wrong)"
    }
    exit $code
} finally {
    if ($slotFile) { Remove-Item $slotFile -ErrorAction SilentlyContinue }
}
