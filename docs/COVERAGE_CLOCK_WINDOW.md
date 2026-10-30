# Original coverage clock window observer

`tools/original_coverage_clock.py` observes one already-owned original-game PID.
It never launches, kills, attaches a debugger, calls a game function, writes
process memory, or follows a replacement PID. It imports the existing
`ProcessReader` and `read_game_clock` from `original_motion_capture.py`; its
process handle has only `PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ`.

This is a control-file producer for the guarded original coverage backends. The
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

The observer waits for an atomic admitted receipt. Default `--tracer-method
drcov` still requires exact schema `bsp.original-baseline.drcov.launch.v1`;
legacy DR receipts may omit `tracer_method`, otherwise it must be `drcov`.
The classic debugger fallback requires explicit `--tracer-method
debugger-function-entries`, exact schema
`bsp.original-baseline.function-entry.launch.v1`, and matching
`tracer_method: debugger-function-entries`. Schemas/methods are not silently
interchanged. Required binding fields remain identical:

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

The observer requires an engine `Loading scene <full scene path>` line followed
by a later engine `GGame::SceneInit()` line in the same observed file epoch
(case/slash normalized). An `Arguments:` echo is never a scene-load event.
Any later engine `Loading scene ...` line clears both markers/readiness,
including a different scene between the expected load and SceneInit. It tracks
observed file replacement/truncation/deletion epochs and clears both markers
when detected. An in-place truncate/regrow between polls is not
generally detectable; the required prelaunch fresh-zero contract is the
initial-stream guarantee. The scene path must match `direct_scene_path`.
The rolling parser reads at most 1 MiB per poll and retains at most 64 KiB of an
incomplete line; oversized lines are discarded. Complete-line processing works
across chunk boundaries. It preserves load/path/init byte offsets and epoch
numbers, plus at most eight recent reset summaries, in `scene_gate`.

This condition comes from the preserved successful native baseline log
`stage-sampler-gaps-20261029/game/bsp_debug.log`: line 2 echoed Arguments at
20:40:20.376, line 154 logged engine Loading scene for USN04 at 20:40:25.267,
and line 760 logged engine GGame::SceneInit at 20:42:53.350. The load-begin
message alone preceded SceneInit by approximately 148 seconds. Live01 exposed
the earlier path-only bug: it selected the Arguments echo at byte 124. That run
still admitted no clock window (zero clean reads, 127 null-game-pointer failures) and
made no target stop request; it is not mission evidence.

Every sample reads the native clock, reads and rechecks context, then reads the
clock again. It retries immediately once if any clock/context field changed or
the fixed-step active byte was set. A final torn sample has `clock: null` and
cannot admit a window or satisfy the target. Failed/torn/retried reads are
counted separately. Equal brackets remain external observations, not an atomic
engine-tick snapshot.

The first mission epoch is admitted only after that fresh ordered engine pair, a valid native
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
return 2. DR backend success additionally requires the matching native client
stop ACK, flushed parent/child trace evidence and its own exit validation.
Reset, timeout and DRAIN remain aborted/nonzero outcomes even if flushing
succeeds. This observer never calls `TerminateProcess` or any equivalent.
The classic function-entry collector owns its separate qualified
`forced_window_stop`: persisted entry observations, bound native clock/source
checks and empty owned jobs. That is an intentional window boundary, not a
natural process exit. The observer does not manufacture either backend's
terminal qualification. Its report records the selected tracer method; the
nonce-bound stop-request schema is unchanged.

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

For the fallback, add `--tracer-method debugger-function-entries` and supply the
function-entry launch receipt. Clock cells, ordered Loading-scene/SceneInit
gate, identity checks and stop-request schema remain unchanged.

## Frozen function-entry target dataset

The ignored generator/manifest are under the worker's
`local/function-entry-targets/`; no new tracked generator tool is required.
`targets.json` uses schema `bsp.original-entry.targets.v1`, flat
`image_sha256`, integer `image_base`/`image_size`, `index_sha256`, `wire_sha256`,
and sorted `points` with eight-hex `address`, integer `expected_byte`, and
Boolean `board_target`. It contains all 21,770 frozen Catalog game starts;
20,892 have the board reference/body target flag, and 878 remain in the cohort
outside that target. Expected bytes come only from the independent private
original EXE copy matching the hash above, not the installation.

Frozen input SHA-256 pins are:

- Index: `bcb7a602b1f0ad7eaa1d5a3237a98154c9876247b9257402405e7b8be6692bfc`.
- Wire: `744d9ff74452777ac820d2c14f4146943dd8c6a246b568e860be2686d3be8811`.
- Scope tool: `b901b3aa5f6b5716923bec20afa270790d91339351e3ed4dc971199c2d7e5183`.

The board mask uses frozen indexed references or positive audited function
bodies, plus historical source-only exceptions `009d5240` and `00a9a600`.
The manifest records the exact policy/status set and classification hash
`869068370c83d5cde27bbb17b53b1eb2b9199cbeb3cc548bdbc7f1bdfb6ec613`.
All generated points are file-backed executable PE bytes; none starts with
`0xCC`. Per-point flags still expose those properties instead of dropping
cohort members. The collector must independently validate runtime PE mapping,
protection, live byte and breakpoint ownership; unsafe/unreadable/mismatched
points remain UNKNOWN in reports.

The frozen SQLite cache contains function starts/ranges but no per-entry
decoded instruction proof. A matching disk byte alone is not instruction-
boundary proof; that uncertainty is explicit in the manifest. One-shot entry
observations measure reached starts, not block spans, frequency or call counts.
An unhit target is not promoted to false execution. The manifest must be
independently hash/cohort reviewed before primary copies or uses it.

## Validation boundary

Python compilation passed. The existing six focused fake-reader/clock checks covered one
retry/final-torn exclusion, scene/step admission, absolute target/reset
isolation, timeout/DRAIN, nonce/FILETIME/PID/fresh-log/path rejection, log
truncation, the concrete Arguments echo at byte 124, engine load-only rejection,
later SceneInit acceptance, intervening/later other-scene invalidation,
seven-byte read chunk boundaries, initial null/unreadable clock recovery, and end-to-end exclusive
atomic JSON publication. The fixture lives
only in ignored `local/coverage-clock-fixture/`; no new test framework or game
process was used. This is synthetic validation, not a live original/DR clock
or native stop/flush validation.
The same receipt fixture also verifies that legacy/default DR remains strict,
function-entry requires explicit mode and matching schema/method, and wrong or
missing method/schema combinations are rejected.
