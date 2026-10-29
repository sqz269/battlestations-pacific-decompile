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
#
# Each run also writes <log>.manifest.json (arguments, BSP_* environment, exe hash and build
# commit) and copies its file-valued arguments to <log>.inputs\; tools/run_replay.py <manifest>
# prints or launches the same run from those copies.
[CmdletBinding(PositionalBinding=$false)]
param(
    [string]$Exe = 'build\win32\Release\bsp_game.exe',
    [string]$GameRoot = 'I:/SteamLibrary/steamapps/common/Battlestations Pacific',
    [string]$XLiveDll = '',
    [string]$Log = 'local\game_run.log',
    [string]$Lock = "$env:USERPROFILE\.bsp\bsp_game.lock",
    [int]$WaitSeconds = 2400,
    [int]$Slots = 14,
    # Optional fixed lane for serial comparisons amid independent harnesses.
    # -1 preserves the existing first-free scan; no occupied lock is replaced.
    [int]$SlotIndex = -1,
    [int[]]$Cores = @(2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28),
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$GameArgs = @()
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path $Exe)) { throw "bsp_game.exe not found at $Exe (run from the worktree root after a build)" }
if ($XLiveDll -eq '') { $XLiveDll = Join-Path (Split-Path $Exe -Parent) 'xlive_stub.dll' }
if (-not (Test-Path $XLiveDll)) { throw "XLive stand-in not found at $XLiveDll" }
$GameArgs = @($GameArgs | Where-Object { $_ -ne '--' })
# Default window placement (harness only): config/run_game.json's "window_monitor" is forwarded
# as --window-monitor unless the caller passed --window-monitor or --window-origin, or set
# BSP_WINDOW_MONITOR (which the executable reads itself). See docs/WINDOW_MONITOR.md.
$runDefaults = Join-Path $PSScriptRoot '..\config\run_game.json'
if (Test-Path $runDefaults) {
    $runDefaultValues = Get-Content $runDefaults -Raw | ConvertFrom-Json
    if (-not $env:BSP_WINDOW_MONITOR -and -not @($GameArgs | Where-Object { $_ -match '^--window-(monitor|origin)$' })) {
        $windowMonitor = $runDefaultValues.window_monitor
        if ($windowMonitor) { $GameArgs = @('--window-monitor', "$windowMonitor") + $GameArgs }
    }
    # Packet cc9_tooling_present_interval: "present_interval" is forwarded as --present-interval
    # unless the caller passed it or set BSP_PRESENT_INTERVAL. `--present-interval native` runs the
    # options file's own VSync. Simulation is lockstep, so the interval changes wall time only.
    if (-not $env:BSP_PRESENT_INTERVAL -and -not @($GameArgs | Where-Object { $_ -eq '--present-interval' })) {
        $presentInterval = $runDefaultValues.present_interval
        if ($presentInterval) { $GameArgs = @('--present-interval', "$presentInterval") + $GameArgs }
    }
    if (-not $env:BSP_WINDOW_RESOLUTION -and -not @($GameArgs | Where-Object { $_ -eq '--window-resolution' })) {
        $windowResolution = $runDefaultValues.window_resolution
        if ($windowResolution) { $GameArgs = @('--window-resolution', "$windowResolution") + $GameArgs }
    }
}
# Difficulty pin (packet cc15-prof2): kProfileDifficultyCc15Bound makes an unpinned run take the
# save's SelectedDifficulty (this installation's save holds 0), so every harness run gets
# --difficulty 1 unless the caller passed --difficulty. BSP_DIFFICULTY_PIN=<n> pins another value;
# BSP_DIFFICULTY_PIN=off runs unpinned. The manifest's "difficulty" records which one applied.
$callerDifficulty = @($GameArgs | Where-Object { $_ -eq '--difficulty' -or $_ -like '--difficulty=*' })
if ($callerDifficulty) {
    $diffIndex = [Array]::IndexOf($GameArgs, '--difficulty')
    $diffValue = if ($diffIndex -ge 0 -and $diffIndex + 1 -lt $GameArgs.Count) { $GameArgs[$diffIndex + 1] } else { ($callerDifficulty[0] -split '=', 2)[1] }
    $difficultyRecord = [ordered]@{ value = "$diffValue"; source = 'caller' }
} elseif ($env:BSP_DIFFICULTY_PIN -in @('off', 'none', 'unpinned')) {
    $difficultyRecord = [ordered]@{ value = 'unpinned'; source = 'BSP_DIFFICULTY_PIN' }
} elseif (-not ([Text.Encoding]::GetEncoding(28591).GetString([IO.File]::ReadAllBytes((Resolve-Path $Exe).Path)).Contains('--difficulty'))) {
    # Exes older than the option reject it (exit 2 with usage); they also predate the profile
    # difficulty switch, so an unpinned run already starts at the old default.
    Write-Host "run_game: $Exe has no --difficulty option (older build); running unpinned"
    $difficultyRecord = [ordered]@{ value = 'unpinned'; source = 'exe-lacks-option' }
} else {
    $pin = if ($env:BSP_DIFFICULTY_PIN) { $env:BSP_DIFFICULTY_PIN } else { '1' }
    $GameArgs = @('--difficulty', "$pin") + $GameArgs
    $difficultyRecord = [ordered]@{ value = "$pin"; source = 'pin' }
}
$owner = if ($env:BSP_AGENT) { $env:BSP_AGENT } else { Split-Path (Get-Location) -Leaf }

$lockDir = Split-Path $Lock -Parent
if (-not (Test-Path $lockDir)) { New-Item -ItemType Directory -Path $lockDir | Out-Null }
if ($Slots -lt 1 -or $Cores.Count -lt $Slots) { throw "-Cores must list at least -Slots processors" }
if ($SlotIndex -lt -1 -or $SlotIndex -ge $Slots) { throw "-SlotIndex must be -1 or an index below -Slots" }
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
$slotStart = if ($SlotIndex -ge 0) { $SlotIndex } else { 0 }
$slotEnd = if ($SlotIndex -ge 0) { $SlotIndex + 1 } else { $Slots }
while ($slot -lt 0) {
    # A drain (the coordinator's ~\.bsp\DRAIN file) stops queued launches here, also while waiting for a slot.
    if ((Test-Path "$env:USERPROFILE\.bsp\DRAIN") -and $env:BSP_IGNORE_DRAIN -ne '1') {
        Write-Error "DRAIN is set ($env:USERPROFILE\.bsp\DRAIN): not starting $logFull. Stop queuing runs; report to your lead." -ErrorAction Continue
        exit 75
    }
    # Fixed-lane selection must still reject a log owned by ANY lane. This
    # read-only pass neither clears stale locks nor reserves other processors.
    foreach ($otherLock in @(Get-ChildItem -LiteralPath $lockDir -Filter 'bsp_game.slot*.lock' -File -ErrorAction SilentlyContinue)) {
        $otherText = (Get-Content -LiteralPath $otherLock.FullName -ErrorAction SilentlyContinue) -join ' '
        if ($otherText -match 'log=(.+)$' -and $Matches[1].Trim() -ieq $logFull) {
            throw "log path $logFull is in use by $($otherLock.Name) ($otherText); give every run its own -Log"
        }
    }
    for ($k = $slotStart; $k -lt $slotEnd; $k++) {
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
    if ((Get-Date) -gt $deadline) {
        if ($SlotIndex -ge 0) { throw "gave up after $WaitSeconds s: slot $SlotIndex busy" }
        throw "gave up after $WaitSeconds s: all $Slots slots busy"
    }
    Start-Sleep -Seconds 10
}
$tag = "slot$slot"
$core = $Cores[$slot]
"slot=$slot tag=$tag core=$core log=$logFull"

# Run manifest (packet cc13-tool1): <log>.manifest.json records the arguments, the BSP_*
# environment, the executable's hash and build commit, and the host commit; every argument
# whose value is an existing file (--helm-orders and the like) is copied to <log>.inputs\ so
# the run can be replayed after the tree that held it is gone (tools/run_replay.py). A
# manifest failure only warns: it never stops the run.
function Get-GitState([string]$dir) {
    $ErrorActionPreference = 'Continue'
    $state = [ordered]@{ root = $null; head = $null; dirty = $null }
    $probe = $dir
    while ($probe -and -not (Test-Path (Join-Path $probe '.git'))) { $probe = Split-Path $probe -Parent }
    if (-not $probe) { return $state }
    $state.root = $probe
    $state.head = (& git -C $probe rev-parse HEAD 2>$null | Select-Object -First 1)
    $state.dirty = [bool](& git -C $probe status --porcelain --untracked-files=no 2>$null | Select-Object -First 1)
    $state
}
function Write-RunManifest($manifest) {
    try {
        [System.IO.File]::WriteAllText("$logFull.manifest.json", ($manifest | ConvertTo-Json -Depth 6))
    } catch { Write-Warning "run manifest not written: $_" }
}
$manifest = $null
function New-RunManifest([string[]]$runArgs) {
    $exeItem = Get-Item $Exe
    $inputDir = "$logFull.inputs"
    $inputs = @()
    for ($i = 0; $i -lt $runArgs.Count; $i++) {
        $opt = $runArgs[$i]; $value = $null; $index = $i
        if ($opt -match '^(--[^=]+)=(.+)$') { $opt = $Matches[1]; $value = $Matches[2] }
        elseif ($opt -like '--*' -and $i + 1 -lt $runArgs.Count) { $value = $runArgs[$i + 1]; $index = $i + 1 }
        # --log is this run's output and may still hold an older run's log.
        if (-not $value -or $opt -eq '--log' -or -not (Test-Path -LiteralPath $value -PathType Leaf)) { continue }
        $source = (Resolve-Path -LiteralPath $value).Path
        if (-not (Test-Path $inputDir)) { New-Item -ItemType Directory -Path $inputDir | Out-Null }
        $name = Split-Path $source -Leaf
        $same = @($inputs | Where-Object { $_.source -eq $source })
        if ($same) { $name = $same[0].archived }
        else {
            if (@($inputs | Where-Object { $_.archived -eq $name })) { $name = "$index.$name" }
            Copy-Item -LiteralPath $source -Destination (Join-Path $inputDir $name) -Force
        }
        $inputs += [ordered]@{ option = $opt; arg_index = $index; value = $value; source = $source
            archived = $name; sha256 = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLower() }
    }
    $envBsp = [ordered]@{}
    Get-ChildItem env: | Where-Object { $_.Name -like 'BSP_*' } | Sort-Object Name | ForEach-Object { $envBsp[$_.Name] = $_.Value }
    [ordered]@{
        started_utc = (Get-Date).ToUniversalTime().ToString('o')
        cwd = (Get-Location).Path
        owner = $owner; slot = $slot; instance_tag = $tag; affinity_core = $core
        log = $logFull
        launcher = @{ exe = $Exe; game_root = $GameRoot; xlive_dll = $XLiveDll; game_args = @($GameArgs) }
        arguments = @($runArgs)
        difficulty = $difficultyRecord
        env = $envBsp
        exe = [ordered]@{ path = $exeItem.FullName; size = $exeItem.Length
            mtime_utc = $exeItem.LastWriteTimeUtc.ToString('o')
            sha256 = (Get-FileHash -LiteralPath $exeItem.FullName -Algorithm SHA256).Hash.ToLower()
            build_checkout = (Get-GitState $exeItem.DirectoryName) }
        host = (Get-GitState (Get-Location).Path)
        inputs = @($inputs)
        exit_code = $null; finished_utc = $null
    }
}
try {
    $logDir = Split-Path $Log -Parent
    if ($logDir -and -not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir | Out-Null }
    $arguments = @('--game-root', $GameRoot, '--xlive-dll', $XLiveDll, '--log', $Log, '--instance-tag', $tag, '--affinity-core', "$core") + $GameArgs
    try { $manifest = New-RunManifest $arguments; Write-RunManifest $manifest }
    catch { Write-Warning "run manifest not written: $_" }
    & $Exe @arguments 2>&1 | Select-Object -Last 1
    $code = $LASTEXITCODE
    "EXITCODE=$code"
    if ($manifest) {
        $manifest.exit_code = $code; $manifest.finished_utc = (Get-Date).ToUniversalTime().ToString('o')
        # Effective difficulty (packet cc15-chg): game+6ACh as the run's log printed it. 0058BF37
        # stores the record's forcedDifficultyLevel when it is not 3 (a CHG challenge row holds 2),
        # and 0058BF58 stores the chosen game+6B0h (the pin) only when it is 3.
        try {
            $effLine = Select-String -LiteralPath $logFull -List -Pattern 'MissionStart::set_effective_difficulty game\+6ACh=(-?\d+)'
            $recLine = Select-String -LiteralPath $logFull -List -Pattern 'side blocks: .* difficulty=(-?\d+)\s*$'
            if ($effLine -and $manifest.difficulty) {
                $forced = if ($recLine) { $recLine.Matches[0].Groups[1].Value } else { '' }
                $manifest.difficulty.effective = $effLine.Matches[0].Groups[1].Value
                $manifest.difficulty.effective_source = if ($forced -ne '' -and $forced -ne '3') { "record forcedDifficultyLevel $forced" } else { 'chosen game+6B0h' }
            }
        } catch { Write-Warning "effective difficulty not recorded: $_" }
        Write-RunManifest $manifest
    }
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
