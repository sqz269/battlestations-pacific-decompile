# Prepare byte copies for an original retail baseline, or invoke the isolation
# guard for an already prepared tree. Never launch retail directly: the guard
# owns restricted-token execution, pre-entry Documents redirection and its job.
# No install/profile ACL, registry setting, junction, hardlink or original file changes.
[CmdletBinding()]
param(
    [string]$GameRoot = 'I:/SteamLibrary/steamapps/common/Battlestations Pacific',
    [string]$StageRoot = '',
    [string]$GuardPath = '',
    [switch]$PrepareOnly,
    [switch]$RunPrepared,
    [switch]$HandleKnownXLiveDebugClose,
    [ValidateSet('low', 'medium-restricted', 'medium-write-locked')][string]$TokenMode = 'low',
    [string[]]$GameArguments = @(),
    [ValidateRange(1, 7200)][int]$RuntimeSeconds = 900,
    [int]$WaitSeconds = 120,
    [ValidateRange(1, 14)][int]$Slots = 14
)

$ErrorActionPreference = 'Stop'
if ($PrepareOnly -and $RunPrepared) { throw 'Choose either -PrepareOnly or -RunPrepared.' }
if (-not $PrepareOnly -and -not $RunPrepared) {
    throw 'LIVE LAUNCH BLOCKED: choose -PrepareOnly or -RunPrepared with a verified restricted-token guard and pre-entry Documents redirection. Environment variables alone do not isolate retail saves.'
}
$coord = Join-Path $env:USERPROFILE '.bsp'
$drain = Join-Path $coord 'DRAIN'
function Assert-NotDraining {
    if (Test-Path -LiteralPath $drain) { throw "DRAIN is set ($drain): baseline preparation/launch refused." }
}
function Assert-NoReparseAncestor([string]$path) {
    $probe = $path
    while ($probe) {
        if (Test-Path -LiteralPath $probe) {
            $item = Get-Item -LiteralPath $probe -Force
            if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse point refused: $probe" }
        }
        $parent = Split-Path $probe -Parent
        if ($parent -eq $probe) { break }
        $probe = $parent
    }
}
function Assert-NoReparseTree([string]$path) {
    Assert-NoReparseAncestor $path
    $link = Get-ChildItem -LiteralPath $path -Recurse -Force -ErrorAction Stop |
        Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint } | Select-Object -First 1
    if ($link) { throw "Source reparse point refused: $($link.FullName)" }
}
function Is-InTree([string]$path, [string]$root) {
    $path.Equals($root, [StringComparison]::OrdinalIgnoreCase) -or
        $path.StartsWith($root.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)
}
function Copy-PrivateTree([string]$source, [string]$destination, [string]$log) {
    Assert-NotDraining
    # Data/attributes/timestamps only. Never copy ACLs, follow reparse points,
    # use hardlinks, mirror deletions, or point any writable path at the source.
    & robocopy.exe $source $destination /E /COPY:DAT /DCOPY:DAT /XJ /R:0 /W:0 /NP /NFL /NDL "/LOG:$log" | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "Private copy failed ($LASTEXITCODE); see $log. Source was not changed." }
}
function Hash-Record([string]$path, [string]$relative) {
    $item = Get-Item -LiteralPath $path
    [ordered]@{ path = $relative; length = $item.Length; sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() }
}
function Assert-SamePath([string]$recorded, [string]$expected) {
    if (-not [IO.Path]::IsPathRooted($recorded) -or
        -not [IO.Path]::GetFullPath($recorded).TrimEnd('\').Equals($expected, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Receipt path does not match expected location: $expected"
    }
}
function Assert-RecordedHashes($records, [string]$root, [switch]$RootFilesOnly, [switch]$AllowPrivateRuntimeLogs) {
    if ($AllowPrivateRuntimeLogs -and -not $RootFilesOnly) { throw 'Private log allowance requires root-file validation.' }
    $seen = @{}
    foreach ($record in $records) {
        $relative = [string]$record.path
        if (-not $relative -or [IO.Path]::IsPathRooted($relative) -or $seen.ContainsKey($relative) -or
            ($RootFilesOnly -and ($relative -match '[\\/]')) -or $record.sha256 -notmatch '^[0-9a-fA-F]{64}$') {
            throw 'Invalid or duplicate hash record in preparation receipt.'
        }
        $seen[$relative] = $true
        $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
        if (-not (Is-InTree $path $root) -or $path -eq $root) { throw 'Hash record escapes its recorded root.' }
        Assert-NoReparseAncestor $path
        # A prepared copy is reusable after the game writes its own logs.
        # Source hashes and every other private root file remain required.
        if ($AllowPrivateRuntimeLogs -and $relative -in @('bsp_debug.log', 'xlive_debug.log')) { continue }
        $actual = Hash-Record $path $relative
        if ($actual.sha256 -ne $record.sha256 -or $actual.length -ne $record.length) { throw "Prepared/original file identity changed: $path" }
    }
}

Assert-NotDraining
$source = (Resolve-Path -LiteralPath $GameRoot).Path.TrimEnd('\')
$personal = [Environment]::GetFolderPath([Environment+SpecialFolder]::MyDocuments)
if (-not $personal) { throw 'Unable to resolve the current Documents known folder.' }
$profileSource = Join-Path $personal 'Battlestations-Pacific'
if (-not $StageRoot) {
    if ($RunPrepared) { throw '-RunPrepared requires an explicit existing -StageRoot.' }
    $StageRoot = Join-Path (Join-Path $PSScriptRoot '..\local\original-baseline') ((Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [Guid]::NewGuid().ToString('N').Substring(0, 8))
}
$stage = [IO.Path]::GetFullPath($StageRoot).TrimEnd('\')
if ((Is-InTree $stage $source) -or (Is-InTree $source $stage) -or (Is-InTree $stage $personal)) {
    throw 'StageRoot must be separate from the installed game and current Documents tree.'
}
if ($PrepareOnly -and (Test-Path -LiteralPath $stage)) { throw "StageRoot must be new; refusing to reuse $stage" }
if ($RunPrepared -and -not (Test-Path -LiteralPath $stage -PathType Container)) { throw "Prepared StageRoot does not exist: $stage" }
Assert-NoReparseAncestor $stage
Assert-NoReparseTree $source
if (Test-Path -LiteralPath $profileSource) { Assert-NoReparseTree $profileSource }
$required = @('battlestationspacific.exe', 'xlive.dll', 'binkw32.dll', 'd3dx9_40.dll', 'fmodex.dll', 'fmod_event.dll')
foreach ($name in $required) {
    if (-not (Test-Path -LiteralPath (Join-Path $source $name) -PathType Leaf)) { throw "Required retail file missing: $name" }
}

if (-not (Test-Path -LiteralPath $coord)) { New-Item -ItemType Directory -Path $coord | Out-Null }
$retailLock = Join-Path $coord 'bsp_original_baseline.lock'
$slotFile = $null
$retailOwned = $false
$deadline = (Get-Date).AddSeconds($WaitSeconds)
$receiptPath = Join-Path $stage 'preparation.json'
$integritySetup = 'pending separate guard; ordinary private byte copies only'
if (-not $GuardPath) {
    $adjacentGuard = Join-Path $PSScriptRoot 'original_baseline_guard.py'
    if (Test-Path -LiteralPath $adjacentGuard -PathType Leaf) { $GuardPath = $adjacentGuard }
}
if ($GuardPath) { $GuardPath = (Resolve-Path -LiteralPath $GuardPath).Path }
$prepared = $null
if ($RunPrepared) {
    if (-not $GuardPath) { throw 'LIVE LAUNCH BLOCKED: original_baseline_guard.py is required.' }
    Assert-NoReparseTree $stage
    $prepared = Get-Content -LiteralPath $receiptPath -Raw | ConvertFrom-Json
    if ($prepared.schema -ne 'bsp.original-baseline.preparation.v1') { throw 'Unsupported preparation receipt schema.' }
    Assert-SamePath $prepared.stage_root $stage
    Assert-SamePath $prepared.source_game $source
    Assert-SamePath $prepared.source_profile $profileSource
    Assert-SamePath $prepared.private_game (Join-Path $stage 'game')
    Assert-SamePath $prepared.private_documents (Join-Path $stage 'Documents')
    $rootRecords = @($prepared.root_files_sha256)
    foreach ($name in $required) {
        if (@($rootRecords | Where-Object { $_.path -ieq $name }).Count -ne 1) { throw "Preparation receipt lacks unique required retail identity: $name" }
    }
    Assert-RecordedHashes $rootRecords $source -RootFilesOnly
    Assert-RecordedHashes $rootRecords $prepared.private_game -RootFilesOnly -AllowPrivateRuntimeLogs
    Assert-RecordedHashes @($prepared.profile_files_sha256) $profileSource
    $privateProfileRoot = Join-Path $prepared.private_documents 'Battlestations-Pacific'
    $privateProfileRecords = @($prepared.profile_files_sha256)
    $nativeOptionsPath = Join-Path $stage 'native-options.json'
    if (Test-Path -LiteralPath $nativeOptionsPath) {
        Assert-NoReparseAncestor $nativeOptionsPath
        $nativeOptions = Get-Content -LiteralPath $nativeOptionsPath -Raw | ConvertFrom-Json
        if ($nativeOptions.schema -ne 'bsp.original-baseline.native-options.v1' -or
            $nativeOptions.original_unchanged -ne $true -or
            $nativeOptions.source_record.path -ne 'options.txt' -or
            $nativeOptions.private_record.path -ne 'options.txt') { throw 'Invalid private native-options receipt.' }
        Assert-SamePath $nativeOptions.stage_root $stage
        Assert-SamePath $nativeOptions.source_path (Join-Path $profileSource 'options.txt')
        Assert-SamePath $nativeOptions.private_path (Join-Path $privateProfileRoot 'options.txt')
        $sourceOptions = @($privateProfileRecords | Where-Object { $_.path -ieq 'options.txt' })
        if ($sourceOptions.Count -ne 1 -or
            $sourceOptions[0].sha256 -ne $nativeOptions.source_record.sha256 -or
            $sourceOptions[0].length -ne $nativeOptions.source_record.length) { throw 'Private options source identity mismatch.' }
        $privateProfileRecords = @($privateProfileRecords | Where-Object { $_.path -ine 'options.txt' }) +
                                 @($nativeOptions.private_record)
    }
    Assert-RecordedHashes $privateProfileRecords $privateProfileRoot
    # Guard preparation is required independently of this copy receipt. The
    # guard revalidates actual labels, links and protected originals at launch.
    $guardPrepared = Get-Content -LiteralPath (Join-Path $stage 'guard-prepare.json') -Raw | ConvertFrom-Json
    if ($guardPrepared.schema -ne 'bsp.original-baseline.guard.prepare.v1' -or $guardPrepared.phase -ne 'prepared' -or
        $guardPrepared.label.rid -ne 4096 -or (($guardPrepared.label.policy -band 1) -ne 1) -or $guardPrepared.label.explicit -ne $true) {
        throw 'LIVE LAUNCH BLOCKED: unsupported/incomplete guard preparation receipt.'
    }
    Assert-SamePath $guardPrepared.stage_root $stage
}
$owner = if ($env:BSP_AGENT) { $env:BSP_AGENT } else { Split-Path (Get-Location) -Leaf }
function New-OwnedLock([string]$path) {
    $stream = [IO.File]::Open($path, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::Read)
    try {
        $bytes = [Text.Encoding]::UTF8.GetBytes("$owner $(Get-Date -Format s) pid=$PID log=$receiptPath")
        $stream.Write($bytes, 0, $bytes.Length)
    } finally { $stream.Dispose() }
}
try {
    # Never reclaim someone else's stale-looking lock. run_game.ps1 can read
    # these slot records normally; this preparer never touches its processes.
    New-OwnedLock $retailLock
    $retailOwned = $true
    while (-not $slotFile) {
        Assert-NotDraining
        $taggedGames = @(Get-CimInstance Win32_Process -Filter "Name='bsp_game.exe'" -ErrorAction Stop)
        for ($slot = 0; $slot -lt $Slots; $slot++) {
            $tagPattern = '--instance-tag"?\s+"?slot' + $slot + '("|\s|$)'
            if (@($taggedGames | Where-Object { $_.CommandLine -match $tagPattern })) { continue }
            $candidate = Join-Path $coord "bsp_game.slot$slot.lock"
            try { New-OwnedLock $candidate; $slotFile = $candidate; break }
            catch [IO.IOException] { if (-not (Test-Path -LiteralPath $candidate)) { throw } }
        }
        if (-not $slotFile) {
            if ((Get-Date) -ge $deadline) { throw "No shared game slot became free within $WaitSeconds seconds." }
            Start-Sleep -Seconds 2
        }
    }
    Assert-NotDraining
    if ($RunPrepared) {
        # The guard monitors DRAIN while running and owns normal-priority child
        # creation, timeout and job cleanup. Keep BOTH locks until it returns.
        $guardArguments = @($GuardPath, 'launch', '--stage', $stage, '--receipt',
                            (Join-Path $stage 'launch.json'), '--seconds', [string]$RuntimeSeconds,
                            '--token-mode', $TokenMode)
        if ($HandleKnownXLiveDebugClose) { $guardArguments += '--handle-known-xlivedebugclose' }
        foreach ($gameArgument in $GameArguments) { $guardArguments += "--arg=$gameArgument" }
        & python @guardArguments
        $guardExit = $LASTEXITCODE
        Assert-RecordedHashes @($prepared.root_files_sha256) $source -RootFilesOnly
        Assert-RecordedHashes @($prepared.profile_files_sha256) $profileSource
        if ($guardExit -ne 0) { throw "Guarded baseline ended with exit code $guardExit; see $(Join-Path $stage 'launch.json')" }
        Write-Output "Guarded baseline completed; original root/profile hashes preserved. Receipt: $(Join-Path $stage 'launch.json')"
        return
    }
    New-Item -ItemType Directory -Path $stage | Out-Null
    # Guard prepare accepts only a new empty stage, grants current-user control
    # there and verifies inheritable low integrity before any private copy exists.
    # Without it, copies remain preparation-only and live launch stays blocked.
    if ($GuardPath) {
        & python $GuardPath prepare --stage $stage --receipt (Join-Path $stage 'guard-prepare.json')
        if ($LASTEXITCODE -ne 0) { throw 'Guard preparation failed; no retail files were copied or launched.' }
        $integritySetup = 'guard prepare succeeded before copies; low-integrity launch and pre-entry redirection still required'
    }
    $copyGame = Join-Path $stage 'game'
    $copyDocuments = Join-Path $stage 'Documents'
    Copy-PrivateTree $source $copyGame (Join-Path $stage 'copy-game.log')
    New-Item -ItemType Directory -Path $copyDocuments | Out-Null
    if (Test-Path -LiteralPath $profileSource) {
        Copy-PrivateTree $profileSource (Join-Path $copyDocuments 'Battlestations-Pacific') (Join-Path $stage 'copy-profile.log')
    }
    Assert-NoReparseTree $stage
    $files = @()
    # Hash every root file, covering executable, emulator, other DLLs/config,
    # and local mod state. Full-copy logs describe all other copied assets.
    foreach ($file in Get-ChildItem -LiteralPath $source -File -Force) {
        $original = Hash-Record $file.FullName $file.Name
        $copy = Hash-Record (Join-Path $copyGame $file.Name) $file.Name
        if ($original.sha256 -ne $copy.sha256) { throw "Retail copy hash mismatch: $($file.Name)" }
        $files += $original
    }
    $profiles = @()
    if (Test-Path -LiteralPath $profileSource) {
        foreach ($file in Get-ChildItem -LiteralPath $profileSource -File -Recurse -Force) {
            $relative = $file.FullName.Substring($profileSource.Length).TrimStart('\')
            $original = Hash-Record $file.FullName $relative
            $copy = Hash-Record (Join-Path (Join-Path $copyDocuments 'Battlestations-Pacific') $relative) $relative
            if ($original.sha256 -ne $copy.sha256) { throw "Private profile copy hash mismatch: $relative" }
            $profiles += $original
        }
    }
    $receipt = [ordered]@{
        schema = 'bsp.original-baseline.preparation.v1'; prepared_utc = [DateTime]::UtcNow.ToString('o')
        source_game = $source; source_profile = $profileSource; stage_root = $stage
        private_game = $copyGame; private_documents = $copyDocuments
        copy_method = 'robocopy full byte copy; COPY:DAT; DCOPY:DAT; no links or ACL copying'
        root_files_sha256 = $files; profile_files_sha256 = $profiles
        integrity_setup = $integritySetup; guard_path = $GuardPath
        live_launch_allowed = $false
        blocker = 'Requires verified low-integrity process token and pre-entry SHGetSpecialFolderPathA(CSIDL_PERSONAL) redirection to private_documents. This script never starts retail.'
    }
    $receipt | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $receiptPath -Encoding UTF8
    Write-Output "Prepared private retail copy: $receiptPath"
    Write-Output 'LIVE LAUNCH BLOCKED: preparation is not runtime save isolation; no game process was started.'
} finally {
    if ($slotFile) { Remove-Item -LiteralPath $slotFile -ErrorAction SilentlyContinue }
    if ($retailOwned) { Remove-Item -LiteralPath $retailLock -ErrorAction SilentlyContinue }
}
