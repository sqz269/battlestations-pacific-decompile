# Original coverage clock window observer

`tools/original_coverage_clock.py` observes one already-owned original-game PID.
It never launches, kills, attaches a debugger, calls a game function, writes
process memory, or follows a replacement PID. It imports the existing
`ProcessReader` and `read_game_clock` from `original_motion_capture.py`; its
process handle has only `PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ`.

This is a control-file producer for the guarded original drcov backend. The
backend owns admission, the nonce-authenticated stop nudge, trace flushing and
exit validation. A request alone is not evidence that a trace flushed or the
game exited successfully.

## Required identity and private paths

The caller supplies required `--pid`, `--expected-exe`, `--expect-sha256`,
`--launch-receipt`, `--output` and `--stop-request`. The expected EXE must be an
absolute independent private `battlestationspacific.exe` under a `local/` path,
with no reparse points and link count 1. The process image path, on-disk SHA-256,
fixed-base PE identity and exact creation FILETIME must match the guard receipt.
The original installed build hash used by this workstream is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Receipt, output and stop-request paths must be distinct and inside the
observer's own repository `local/`. Both outputs must also be inside the
receipt's fresh run directory. They must not exist. The script derives its
repository from its own file location, so integration must precede use with
another worktree's control directory. Existing files are never overwritten.

The observer waits for an atomic receipt with schema
`bsp.original-baseline.drcov.launch.v1` and `admitted: true`. Required binding
fields are:

```text
root_pid: integer, equal to --pid
root_created_filetime: exact positive integer Windows 100ns FILETIME
root_executable: canonical absolute path, equal to --expected-exe
root_executable_sha256: equal to --expect-sha256
run_directory: private directory under this repo/local
stop_request_path: equal to --stop-request, inside run_directory
stop_nonce: exactly 16 hexadecimal characters (64-bit native nudge contract)
scene_log_path: declared private absolute generated-log path
scene_log_start_size: integer 0
scene_log_reset_before_launch: true
direct_scene_path: universe/Scenes/missions/USN/usn_19_coralus.scn
```

FILETIME remains a Python/JSON integer; it never passes through a JavaScript
floating-point number. The receipt identity is bound once, and live identity
is checked again before publishing a stop request. Receipt changes do not make
the observer switch targets.

## Fresh scene and clean native clock

The backend archives/resets **only** its two declared private generated logs,
`bsp_debug.log` and `xlive_debug.log`, before launch. The actual native scene
marker is in the private game's `bsp_debug.log`; the receipt selects its path.
The observer never resets or writes either log. A nonzero old-file offset is
rejected: native startup may truncate an existing stream, so file size alone
cannot establish a fresh marker. This prevents a copied stale `LoadingScene`
line from admitting a window.

The observer reads the fresh stream for the full scene path (case/slash
normalized). It tracks observed file replacement/truncation epochs and clears
the marker when detected. An in-place truncate/regrow between polls is not
generally detectable; the required prelaunch fresh-zero contract is the
initial-stream guarantee. The scene marker must match `direct_scene_path`.

Every sample reads the native clock, reads and rechecks context, then reads the
clock again. It retries immediately once if any clock/context field changed or
the fixed-step active byte was set. A final torn sample has `clock: null` and
cannot admit a window or satisfy the target. Failed/torn/retried reads are
counted separately. Equal brackets remain external observations, not an atomic
engine-tick snapshot.

The first mission epoch is admitted only after the fresh marker, a valid native
game pointer, positive `fixed_step_count`, and positive native
GameTime. Earlier clean clock reads are retained separately as
`first_clean_clock_read`, without treating a menu/uninitialized clock as the
mission start. The target is **absolute native GameTime >= 240**, not another
240 seconds after attachment. A late first observation can therefore meet the
target immediately; its first observed time remains explicit.

Native cells and their verified writers are documented in
[ORIGINAL_GAME_CLOCK.md](ORIGINAL_GAME_CLOCK.md). In particular:

- `game_time` is float `[00F876A4]`, the Lua GameTime cell.
- `game_frame` is uint32 `[game+648h]`, the AppOnMove counter; it is not a
  mission-relative frame ordinal.
- `fixed_step_count` is uint32 `[00F876B0]`, and `fixed_step_dt` is float
  `[00D0DE84]` (approximately 0.05).
- `engine_dt` is scaled per-OnMove float `[game+21F0h]`, not the sample's wall
  interval and not a promise that every engine frame is a fixed step.

The observer also reads uint32 `game+1FE4h` non-campaign flag,
signed `game+6ACh` effective difficulty, and controlled pointer `[00E188D8]`.
It derives GetDifficulty exactly as `008AE113..008AE130`: return 2 when the
non-campaign flag is nonzero, otherwise the effective field
(`src/lua_binding_core.cpp:26`). The controlled global is a 32-bit x86 pointer;
its native producer at `004C0893` and live xrefs were verified by the baseline
workstream (`include/bsp/controlled_unit.hpp`). No extra reconstructed game-state
field participates in admission. A mismatch against the BSP reference difficulty
1 is recorded, never corrected. Controlled pointer is context only; the
observer does not inspect or auto-follow a selected unit.

After admission, regression of clean game time, app frame or fixed-step count,
or a changed game pointer, creates a reset epoch and requests `clock_reset`.
No time or frame span is silently combined across that boundary. The report
retains first/end observations, wall times, engine/fixed deltas, brackets,
context, epoch boundary and read counts.

## Stop protocol and outcomes

The stop file is published once using schema `bsp.original-coverage.stop.v1`:

```json
{
  "schema": "bsp.original-coverage.stop.v1",
  "pid": 1234,
  "process_created_filetime": 134000000000000123,
  "expected_exe": "J:/.../local/private-game/battlestationspacific.exe",
  "nonce": "0123456789abcdef",
  "reason": "game_time_target",
  "clock": {"game_time": 240.05, "game_frame": 5000, "context": {}},
  "epochs": [],
  "recorded_utc": "..."
}
```

`clock` contains flat native clock fields, plus `context`, observed wall time
and UTC; it is null if a timeout/drain occurred before mission admission.
Allowed request reasons are `game_time_target`, `clock_reset`, `timeout` and
`drain`. `~/.bsp/DRAIN` is always honored. Poll defaults to 0.1 seconds;
`--wall-timeout` defaults to and cannot exceed 3600 seconds, including receipt
waiting (bounded file-I/O retry adds up to two seconds per operation).

Both files use an exclusively created temporary file, flush/fsync, and Windows
atomic rename without replacement. Sharing/access failures retry for at most
two seconds. The stop request is published before the final evidence report;
if report publication fails afterward, the complete authenticated stop file
remains the control evidence. An observer error/interruption or process exit
does not manufacture a successful target request. Before admission, timeout
or DRAIN is reported without an unbound stop request.

Observer exit 0 means it published a `game_time_target` request. Other reasons
return 2. Backend success additionally requires the matching native client
stop ACK, flushed parent/child trace evidence and its own exit validation.
Reset, timeout and DRAIN remain aborted/nonzero outcomes even if flushing
succeeds. This observer never calls `TerminateProcess` or any equivalent.

Example after the receipt has root identity fields (run from the integrated
repository; all placeholder paths must be replaced with that run's receipt):

```powershell
python tools/original_coverage_clock.py --pid 1234 `
  --expected-exe 'J:/.../local/private-game/battlestationspacific.exe' `
  --expect-sha256 b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6 `
  --launch-receipt local/original-drcov/run/launch.json `
  --target-game-seconds 240 --wall-timeout 3600 `
  --output local/original-drcov/run/clock-window.json `
  --stop-request local/original-drcov/run/stop-request.json
```

## Validation boundary

Python compilation passed. Six focused fake-reader/clock checks covered one
retry/final-torn exclusion, scene/step admission, absolute target/reset
isolation, timeout/DRAIN, nonce/FILETIME/PID/fresh-log/path rejection, log
truncation, initial null/unreadable clock recovery, and end-to-end exclusive
atomic JSON publication. The fixture lives
only in ignored `local/coverage-clock-fixture/`; no new test framework or game
process was used. This is synthetic validation, not a live original/DR clock
or native stop/flush validation.
