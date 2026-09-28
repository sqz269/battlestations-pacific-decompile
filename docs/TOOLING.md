# Pair and worktree tooling

Addresses: none (workspace tooling, packet `cc9_tooling_1`)

Tools that replace steps every worker used to do by hand. None of them touches Ghidra, a ledger
or a reconstructed body.

| tool | usage |
| --- | --- |
| `tools/pair_diff.py` | `python tools/pair_diff.py local\X_OFF.log local\X_ON.log [--json out.json] [--limit N]` |
| `tools/pair_export.py` | `python tools/pair_export.py --commit <sha> --flip kSwitch=true [--flip ...] --out local\<name> [--mission E2] [--no-build]` |
| `bsp.py sync` | `python tools/bsp.py sync [--no-fetch]` (and one line in `bsp.py brief`) |
| crash record | `./tools/run_game.ps1 -Log local\crash_test.log -- ... --crash-test [N]` (the record itself is always on) |
| `tools/sample_main_thread.py` | `python tools/sample_main_thread.py --log local\X.log` (start it, then the run with the same -Log) |
| present interval | `./tools/run_game.ps1 ... -- --present-interval <immediate|vsync|native>`; `config/run_game.json` defaults to immediate |
| renderer init failure | automatic: a failed CreateDevice logs `harness renderer init failed: ...` and exits 4 (section 8) |

## 1. `tools/pair_diff.py`: the same-tree pair comparison

It reads two `bsp_game` logs of one pair (the same run parameters, one switch flipped) and prints
a report in order of importance:

1. **Run parameters.** The `bsp_game milestone` header (log path masked) and the `frame jitter`
   line. A difference is printed first as `RUN PARAMETERS DIFFER`. A log from before the jitter
   option has no jitter line and counts as `frame jitter off`.
2. **Clock offset.** `t=` of the first `gunnery step N` line both logs have, ON minus OFF, with
   the `first_hit` delta beside it. The docs have so far judged the offset from the first hit
   ("both first hits land within 0.1 s", docs/SHIP_AI_TAILS.md). The gunnery step is used instead
   because it is the mission clock at a fixed step count, so a real change in the first hit
   cannot hide an offset. The offset is subtracted from every time column (`t`,
   `first_damage`, `sunk_at`) before those are compared. A non-zero offset is never "identical".
3. **Gameplay rows.** Deaths, hit records, hull hits, damage (`summary mission gunnery damage`),
   shots (the last `gunnery step` line), first hit, torpedo-task and dive-bomb-task releases (of
   aircraft), torpedo drops, plane water contacts (the `Plane::water_contact*` native calls), the
   controlled unit's distance moved, units, the mission end or failure time, and the native
   table's concrete / unimplemented counts. These are counted from the rows, without the noise rows,
   so they read one or two below the log's own `host methods` header.
4. **Per-entity tables, row by row.** `death row` lines keyed by victim, `plane death mode` lines
   keyed by unit, and the end-of-run unit table (`unit side guns cats ... sunk_at killed_by`).
   Each reports rows only OFF, only ON, and the changed fields. `death row` lines need
   `BSP_DEATH_TABLE=1`; the report notes when a side has none.
5. **The whole native table, both ways.** Rows after `host methods N concrete, M unimplemented`,
   keyed by name and address: added, removed, status changed (UNIMPLEMENTED / concrete), calls
   moved.
6. **Every `summary` line,** keyed by its leading words. A key that repeats is extended by its
   first field's name, and, if it still repeats, by the whole first field (`mission gunnery
   barrel device=202`), so one extra line does not shift every later key.
7. **Every other line,** compared as a multiset of masked lines. The `host X [addr] status`
   first-call lines are left out because the native table carries the same status with counts.

**Exit code.** 0: identical apart from noise. 1: something differs, but every gameplay row and
per-entity row is identical (the usual result for a switch whose own rows change). 3: a gameplay
or per-entity row moved. 2: usage error, or a log without a native table (an unfinished run).
`--json` writes every row the report truncates.

### The noise list

Each item is masked (or, for the one native row, its count ignored) before anything is compared.

| id | what | why it varies |
| --- | --- | --- |
| heap-pointers | every 8-digit hex value, `0x` or bare | heap objects land inside the image's own address range too (`0x005C2F28` in one run), so no range test separates them. Native-table addresses are parsed before masking |
| log-path | `log=` in the milestone header | each run has its own log name |
| module-directory | `module directory ...` | an OFF and an ON build sit in different directories |
| harness-slot | `harness: single-instance mutex name suffixed with instance tag ...` | `tools/run_game.ps1` hands out slots 0-2 (docs/COORDINATION.md, three slots) |
| harness-affinity | `harness: main thread pinned to processor N` | the slot's core (2, 4, 6) |
| thread-ids | `worker=` and `render_thread=` on the renderer lines | OS thread ids |
| press-start-blink | `color=(...)` on `text press_start_Text` | the title screen's blink runs on the wall clock before the mission (docs/GAME_EXECUTABLE.md, `--frame-jitter` measured table) |
| prewindow-fmod-calls | `fmod_calls=` on `sound startup before window:` | FMOD polls before the window exists, a wall-clock count (131 against 132 in the UC3 pair, docs/AVOID_ZONE_REGISTRY.md) |
| present-interval-header | the line `present interval <x> (harness override)`, dropped | printed only when `--present-interval` overrides the options file (section 7) |
| present-interval-device | `interval=` on `device created by full native startup` | the D3D present interval the override sets; lockstep frames make it a wall-time setting only |
| avoidance-refills | `refills=` on `ship avoidance search:` | differs between identical runs of one binary (257 against 265, 2026-09-22) |
| sector-scan-clip-arc-zones | the `ShipAiSectorScan::clip_arc_zones_00415970` native row: its count, and its presence | differs between identical JM08 runs of one binary (3200/3000, streams on, lockstep 0.05): 12,000 calls in `LSH_OFF_JM08.log`, no row at all in `LSH_OFF2_JM08.log` (worktree cc9-init2), with gameplay, death rows and unit table identical; 6,000 and 12,000 on two ON runs. A row with no calls is not printed, so presence is noise too |
| sector-scan-zone-segment-crossing | the `ShipAiSectorScan::zone_segment_crossing_004158e0` native row: count and presence | moves with the row above: 6,000 then absent on the same OFF pair, 3,000 and 6,000 on the ON runs |
| clearance-static-zone-blocks | the `ShipAiClearance::static_zone_blocks_009d57e0` native row: count and presence | across seven JM08 runs of identical binaries (worktree cc9-init2) it has 3 calls in `QN_ON_JM08.log` and `WS_OFF_JM08.log` and no row in `LSH_OFF/OFF2/ON/ON2_JM08.log` and `WS_ON_JM08.log`, independent of any switch, with gameplay identical in every pair (docs/SENTITY_INIT_ATTACH_ORDER.md section 20.4) |
| clearance-static-zone-clearance | the `ShipAiClearance::static_zone_clearance_00415d70` native row: count and presence | comes and goes with the row above, 3 calls when present, on the same runs |
| clearance-category-enabled | the `ShipAiClearance::category_enabled_009ec770` native row: count and presence | 7,026 in `VS_ON2_USN13.log` against 6,999 in `VS_ON3_USN13.log`: two USN13 3200/3000 runs of one binary (`local\bin\vs_on2`, SHA-256 `B544BC1BA071`, worktree cc9-terrain2) with gameplay identical, moving together with the static-zone and sector-scan rows (docs/SCENE_CONTENTS_HOSTS.md section 15) |
| clearance-avoidance-enabled | the `ShipAiClearance::avoidance_enabled_0080e160` native row: count and presence | 1,109 against 1,108 on the same pair |
| pretranslate-count | calls of the `PlatformLoopCallbacks::pretranslate` native row | window messages (focus, paint) depend on the desktop; printed as `(noise)` |

### Validation

Every pair below is a pair of record in a worker tree, read in place. The tool's rows are the
recorded verdict's rows.

| pair (tree) | recorded verdict | tool |
| --- | --- | --- |
| `seg_off_e2` / `seg_on_e2` (cc9-gunnery2) | docs/PLAYER_GUN_SEAT.md 6.4: only `PlayerGunSeat::segment_query` UNIMPLEMENTED -> concrete (18,158 calls) and the seat line's own values; 836 / 51 both | exit 1. Native: that one status change. Summary: `casts 0 -> 17997, bound 0 -> 1`. Death rows, plane death modes and 81 unit rows identical; other lines 0 / 0 |
| `gf_off_usn02` / `gf_on_usn02` (cc9-side-ai) | docs/CONSTRUCT_WORLD.md 14: +2 rows (the two `Session::` records), four bound rows concrete, `fixed step body` concrete 81,000 -> 117,000 and records 54,000 -> 18,000; 22 deaths, 440 hits both | exit 1. Native 1464 -> 1466: those two added, those four changed. Summary: exactly that `fixed step body` change. 22 death rows identical |
| `UC3_OFF_USN02` / `UC3_ON_USN02` (cc9-plane-release) | docs/AVOID_ZONE_REGISTRY.md, the pairs measured: natives identical on USN02, gameplay identical, only noise (refills, slots, `fmod_calls` 131 / 132) besides the bound line | exit 1. Native: no change. Summary: `mission units contracts: bound 0 -> 1`. Noise masked. Notes the pair has no death rows |
| `CS_OFF_9000` / `CS_ON_9000` (cc9-circle-steer) | docs/PILOT_MOVETO_TASK.md, E2 9000: deaths 43 -> 51, hits 796 -> 841, releases 5/6 -> 5/5, Lexington 4284.80 -> 5833.61, units 92 -> 81, steer UNIMPLEMENTED 13,929 gone and `BotStateMoveToCircle::tick` concrete 3,762 | exit 3 with exactly those rows. Death rows: the eight escort Zeros (#1.2, #2.2, #4.2, #8.2 and wingmen) only ON. Unit table: the eleven phase-2 units only OFF |
| `FJ_J1_9000` / `FJ_J2_9000` (cc9-circle-steer) | docs/GAME_EXECUTABLE.md `--frame-jitter` measured: identical but for the title-screen alpha | exit 0 |
| `FP_OFF_9000` / `FJ_LOCK_9000` (cc9-circle-steer) | same section: identical but for `frame jitter off` and the alpha | exit 0 |

Clock offsets were zero in every pair above. The subtraction of a non-zero offset has not been
exercised on a real pair.

## 2. `tools/pair_export.py`: an export-flip build

It builds a switch's other state without editing the file that holds it, which is usually leased
by another worker.

1. `git archive <commit>` of this repository is unpacked into `--out` (a directory under this
   worktree's `local\`). On a re-export only files whose bytes differ are rewritten and files the
   commit no longer has are removed. `build/` is never touched, so the CMake build there stays
   incremental. `.pair_export.json` in the export records the commit, the flips and the file list.
2. Each `--flip kName=<true|false|1|0>` rewrites the one `[static|inline] constexpr bool kName =
   true|false;` definition in the exported bytes, before they are compared with the disk. So
   dropping or changing a flip on a later export rewrites that file too. A name that is absent,
   or defined more than once in the tree's sources, fails before anything is written.
3. It runs the export's own `scripts/build.ps1`, which builds `<out>\build\win32` and runs ctest.
   The output goes to `<out>\pair_export_build.log`.
4. It prints the SHA-256 prefix of `<out>\build\win32\Release\bsp_game.exe` and the
   `./tools/run_game.ps1 -Exe ... -Log local\<name>_<mission>.log -- ...` line. It never runs the
   game. A failed build prints the log's tail. A binary older than a failed build is called STALE.

**Reference rows are never taken from an export with a flip.** A flipped export measures one
switch against its control, the same export with no flip. Reference rows come from a build of the
landed tree with every switch as landed.

### Validation (main `7e7b78339`, `kPlaneFormationPlacementEnabled`, src/game_hosts_units.cpp)

| step | result |
| --- | --- |
| `--flip kPlaneFormationPlacementEnabled=true --out local\pe_on` into an empty directory | 11,007 files written; flip `false -> true` at src/game_hosts_units.cpp:3104; fresh build and ctest ok in 11 min 12 s; SHA-256 `11D0194D8910` |
| the same commit, no flip, `--no-build --out local\pe_ctl` | `git diff --no-index --diff-filter=M local\pe_ctl local\pe_on`: two files. One is the manifest; the other is the single line `kPlaneFormationPlacementEnabled = false` -> `true`. Every other difference is an added file under `build\`, the build log or ctest's Python caches |
| the same flip again into `local\pe_on` | 0 written, 11,007 unchanged; the binary kept its SHA-256 and mtime (55 s, mostly ctest) |
| no flip into `local\pe_on` | 1 written (`src/game_hosts_units.cpp`); incremental build 14 s; SHA-256 `D6BDA657D168` |
| `--flip kNoSuchSwitch=true` | refused, `found 0 (nowhere)`, nothing written, exit 1 |
| `--flip kPlaneFormationPlacementEnabled=maybe` | refused, exit 1 |

No switch name is defined twice in the tree today, so the duplicate refusal has been exercised
only by reading the code, not on a real tree.

## 3. `bsp.py sync` and the `brief` tip line

`git merge --ff-only main` refused has one cause in this workflow: the branch carries a commit
main does not have. On 2026-09-27 four workers read that refusal as "main moved". `sync` answers
the real question first.

- `python tools/bsp.py sync` fetches `origin`. If the branch tip is an ancestor of `origin/main`
  it fast-forwards to `origin/main` and prints the first-parent commits that moved and a
  shortstat. Otherwise it lists every commit not on `origin/main` (short sha and subject), prints
  `unlanded: report to the lead`, merges nothing and exits 1. `--no-fetch` skips the fetch.
- `python tools/bsp.py brief` prints `branch tip is on main` or `N commits not on main: <shas>`.
  It does not fetch, and a commit counts as on main when `main` or `origin/main` contains it.

The logic is in `tools/branch_sync.py`. `tests/test_pair_tools.py` is the one test: it builds a
bare origin, a lead clone and a worker clone. It checks that the worker fast-forwards when only
main moved, and that it refuses, keeps its HEAD and names its own sha once it has an unlanded
commit.

**Validation in this worktree.** At `7e7b78339`, `sync` printed `already at origin/main` and
`brief` printed `branch tip is on main`. At `7edcebad5`, before landing, `sync` printed the one
unlanded commit and `unlanded: report to the lead` and exited 1, and `brief` printed
`1 commits not on main: 7edcebad5`.

## 4. The harness crash record

An access violation in `bsp_game` used to leave no exception information in its log. The bootstrap
child (the process that writes the log) now installs a top-level exception filter once the log is
open. The filter is harness code in `src/game_main.cpp`. On an unhandled exception it:
- writes one line to the run log, as shown below;
- writes a minidump next to the log (`local\<name>.dmp`), with dbghelp loaded at that moment so
  the build gains no import;
- ends the process with the exception code as its exit code, without a Windows Error Reporting
  dialog.

```
harness crash: exception <code> at <address> [reading|writing|executing <target>] module <name>+<offset> (base <base>) thread <tid> mission_frame <n|none (not in a mission frame yet)>
harness crash: minidump <path>: written
```

- **`module+offset`.** It maps the faulting address to `bsp_game.exe` (or a DLL) independent of
  where the image was loaded. `bsp_game.exe` itself loads at `10000000`.
- **`mission_frame`.** It is the in-mission frame the main thread was running, from a counter the
  mission-frame driver sets next to the frame-jitter hook (`src/game_hosts_mission.cpp`). A fault
  on another thread (the render worker) prints its own thread id with the main thread's frame.
- **What bypasses it.** The reconstructed CRT failure paths (`src/native_crt_*_failure.cpp`) clear
  the filter on purpose, as the image's CRT does. So a /GS or invalid-parameter failure still
  terminates without this line.

**`--crash-test [N]`** is hidden and harness only. It is stripped from argv before the public
parser, in both the parent and the child. It writes through a null pointer in
`harness_crash_test_fault` just before in-mission frame N (default 10), and logs the function's
module offset first so the record can be checked against it. Without the option nothing is armed,
and nothing new is printed.

**Verified, 2026-09-27.** This worktree at `3f81662cf` plus this change, console session:

```
./tools/run_game.ps1 -Log local\crash_test.log -- --frames 400 --press-start-frame 30 --menu-select USN02 --mission-frames 300 --mission-frame-seconds 0.05 --crash-test 10
```

The launcher reported `EXITCODE=-1073741819` (0xC0000005). The log ends:

```
harness crash test: null write in harness_crash_test_fault (bsp_game.exe+a7a20) before mission frame 10
harness crash: exception c0000005 at 100a7a2e writing 00000000 module bsp_game.exe+a7a2e (base 10000000) thread 14144 mission_frame 10
harness crash: minidump J:\PROG\battlestations-pacific-decompile-cc9-tooling\local\crash_test.dmp: written
```

`+a7a2e` lies 0Eh into the fault function at `+a7a20`, and the minidump is 298,352 bytes.

**Nothing else changes.** Main `7e7b78339`, built as a no-flip `tools/pair_export.py` export
(`local\pe_on`, SHA-256 `D6BDA657D168`), was run against this build with the same USN02 1300/1200
arguments and no `--crash-test` (`local\cr_base_usn02.log`, `local\cr_new_usn02.log`).
`tools/pair_diff.py` reports exit 0, identical apart from noise. The whole native table (1,467
rows), all 188 summary lines, the 32-row unit table and every other masked line are identical.

## 5. Where a run's wall time goes (read-only profile, 2026-09-27)

**Result.** The main thread spends about 98.6% of a USN04 run in the host-call bookkeeping
`GameHostLog::record` (`src/game_hosts.cpp`). Rendering is about 1%, and the thread never waits on
vsync. So a headless mode would save almost nothing. Nothing was implemented from this.

**The run.** It was USN04 4700/4500 with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`,
lockstep 0.05, launched through `tools/run_game.ps1` from this worktree (`local\prof_usn04.log`,
build of `439238465`). Wall time was 479 s. The run exited 0 with 4,699 frames presented.

**Method.**
- **No existing profiler was usable.** The harness logs no frame or present times. WPR and xperf
  are installed, but kernel sampling needs elevation, which the session does not have.
- **The sampler.** It is a Python script in the session scratchpad, not committed and nothing
  installed, and it uses only OS debugging APIs. It finds the bootstrap child by its `--log` path.
  At each sample it suspends the thread and reads EIP/ESP (`Wow64GetThreadContext`), then reads
  32 KB of stack and resumes the thread.
- **Callers and symbols.** Callers come from stack scanning: a dword inside a module's code that
  is preceded by a CALL. They are symbolized from `build\win32\bsp_game.map` and from the system
  DLLs' export tables. The main thread is the earliest-created thread.
- **Categories.** The innermost frame that matches decides:
  - logging: `GameHostLog`, stdio, `WriteFile`;
  - rendering: d3d9 or the driver modules, or Render/Present/Draw/Shader/Texture/`GameDeviceHost`;
  - interface: Gui/Hud/Text/Widget/Font;
  - simulation: otherwise, `run_mission_frame_004e4a40` or the fixed step on the stack.
- **Coverage.** 3,578 main-thread samples over 475 s (about 7.5 Hz, limited by the thread
  enumeration each round). The first ~4 s of startup were not sampled.

| main thread, mission phase | samples | share |
| --- | ---: | ---: |
| logging | 3,527 | 98.6% |
| of which the leaf is `GameHostLog::record` | 3,339 | 93.3% |
| rendering | 32 | 0.9% |
| interface | 19 | 0.5% |
| simulation or other | 0 | 0% |
| leaf in any wait (vsync, Sleep) | 0 | 0% |

Simulation reads 0% because the recorder sits inside almost every simulation call, and the
innermost match wins. Its true share is small but unmeasured.

**The cause.** `record()` walks `records_` linearly and compares a `std::string` with the
`const char*` name on every host call. This run's native table has 1,582 rows and 63,346,428
calls. The largest rows are `ShipMotion::ocean_wave_field` and `ShipMotion::ocean_coverage_mask`
(2,784,831 each), then `Gun::step_aim_0085ad80` and two sibling gun rows (2,696,872 each).

**Suggestion, not implemented.** Add an index beside the vector, for example from the name pointer
to the vector slot with a string-keyed fallback. Keep the vector order so the printed table does
not change, and use `tools/pair_diff.py` to show the log is identical. The expected speed-up is
several-fold. It is an estimate, not a measurement.

## 6. The record index (packet `cc9_tooling_record_index`)

**The change.** `GameHostLog::record` (`src/game_hosts.cpp`) no longer walks `records_`. It is
harness only.
- **The index.** Two maps sit beside the vector (`include/bsp/game_hosts.hpp`). One is keyed on
  the caller's name pointer: the host names are string literals, so the pointer is stable. The
  other is keyed on the name string and is authoritative.
- **Pointer hits.** A hit is confirmed with one string compare. So a caller that reuses a buffer
  for different names falls through to the string map and cannot be miscounted.
- **Semantics.** They are those of the old walk: the first record with a name keeps its native
  address and status, and every later call with that name adds to its count.
- **Order.** The vector keeps its first-call order, so the printed native table is unchanged.

**Identity proof.** Two binaries were built:
- *before*: `tools/pair_export.py` of `76962df1b` with no flip (`local\pe_on`, SHA-256
  `CBBB08C87561`);
- *after*: this worktree with only this change.

Each ran once through `tools/run_game.ps1` with USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1` and lockstep 0.05. The logs are `local\ri_before_usn04.log` and
`local\ri_after_usn04.log`.
- `python tools/pair_diff.py local\ri_before_usn04.log local\ri_after_usn04.log` exits 0,
  identical apart from noise: 43 death rows, 43 plane death modes, 81 unit rows, 1,591 native rows
  and 209 summary lines, with 0 other lines differing.
- The native table, header included, is byte-identical row for row (1,592 lines). That includes
  the `PlatformLoopCallbacks::pretranslate` count, which this time did not move.

**Reference rows are unaffected in content.** The record only counts calls, and no simulation code
reads it. A run is faster, not different.

| launcher wall time, USN04 4700/4500 | seconds |
| --- | ---: |
| before (`local\ri_before_usn04.log`) | 493.6 |
| after (`local\ri_after_usn04.log`) | 89.1 |
| after, with the sampler attached (`local\ri_prof_usn04.log`) | 89.6 |

**The profile after the change** comes from `tools/sample_main_thread.py`: 619 main-thread samples
over 85 s of the mission phase.
- **Rendering is now 57% of samples.** 328 of them sit inside `nvd3dum.dll` under
  `end_native_renderer_frame_00b2d8e0`, in ntdll. This tool cannot name ntdll functions, so these
  are counted as rendering, not as waits.
- **The run is paced by vsync.** 4,699 presented frames in about 83 s is about 57 fps, and the
  options file this installation uses has `vsync=1`.
- **Logging is 31% (191 samples).** 155 of them are in `record` itself: hashing, the confirming
  compare and the counter.
- **Interface is 12%.** 61 samples are in `MarkerRuntimeBinding::gui_extent`.

The next lever would be presenting without vsync in harness runs. It is not implemented, and the
options file is not to be edited.

## `tools/sample_main_thread.py`

The sampler from section 5, promoted. It needs no elevation.

```
python tools/sample_main_thread.py --log local\X.log [--out local\X_samples.jsonl] [--map build\win32\bsp_game.map] [--period-ms 5] [--report-only]
```

Start it first, then launch the run through `tools/run_game.ps1` with the same `-Log` path. It
waits for that run's bootstrap child, samples until the process exits and prints the section-5
split. `--report-only` re-reads a samples file.

Known limits:
- ntdll leaves show as `?`, so a wait inside the driver counts under its caller's category, not
  as a wait;
- about 7 Hz on the main thread, because thread enumeration is slow;
- the first few seconds of startup are not sampled;
- the render worker thread is not identified.

## 7. The harness present interval (packet `cc9_tooling_present_interval`)

After the record index, a run is paced by vsync (section 6). `--present-interval
<vsync|immediate|native>` (env `BSP_PRESENT_INTERVAL`, the same words, used only when the option
is absent) overrides the D3D present interval for a harness run. It is harness only.
- **Where it applies.** The host (`src/game_hosts.cpp`) replaces the VSync word of the renderer
  request just before `GameDeviceHost::create`. `00B2AEB0` turns that slot into
  `PresentationInterval` (renderer+1A5C): `immediate` gives `0x80000000` and `vsync` gives 0.
- **What it leaves alone.** The options file and the settings read are not touched, so the
  `window request ... vsync=1` and `renderer init request ... vsync=1` lines keep the options
  file's value. The reset path reuses the stored parameters.
- **The log.** It prints `present interval <x> (harness override)` after the jitter line, only
  when an override is in force. `native` (the default when nothing is given) prints nothing and
  keeps the settings' value, so a run without the override logs exactly what it did before.
- **The launcher.** `tools/run_game.ps1` forwards `config/run_game.json`'s `"present_interval":
  "immediate"` unless the caller passed `--present-interval` or set `BSP_PRESENT_INTERVAL`, in
  the same way as `window_monitor`. Pass `--present-interval native` for the options file's own
  VSync.
- **Why simulation cannot see it.** Mission frames are lockstep (`--mission-frame-seconds 0.05`),
  so the interval changes only how long a frame waits for the screen. The pre-mission wall-clock
  counters were already masked noise (section 1). A presentation-mode switch at run time
  (`native_renderer_presentation_mode.cpp`) would rewrite +1A5C, but none happens in a harness run.

**Proof.** The binary is this worktree at `704f5dd02` plus this change. Both runs are USN04
4700/4500 through `tools/run_game.ps1`, with `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1` and
lockstep 0.05.

| run | option | device line | launcher wall time |
| --- | --- | --- | ---: |
| `local\pi_native_usn04.log` | `--present-interval native` | `interval=0x0`, no override line | 173.7 s |
| `local\pi_imm_usn04.log` | config default (immediate) | `interval=0x80000000`, `present interval immediate (harness override)` | 32.1 s |
| `local\ri_after_usn04.log` (section 6, older source) | none | `interval=0x0` | 89.1 s |

- **Same binary, immediate against native.** `python tools/pair_diff.py local\pi_native_usn04.log
  local\pi_imm_usn04.log` exits 0: 43 death rows, 43 plane death modes, 81 unit rows, 1,591 native
  rows, 209 summary lines and 0 other lines, all identical.
- **Immediate against the 89.1 s run.** The comparison exits 1, with gameplay, native table,
  death and unit rows all identical. One summary line differs, `summary scene terrain`, and it
  does so because its format changed in the scene-entities landings merged between the two
  builds (`e9b533ada`, `c593b83b2`), not because of the override. So the same-binary pair is the
  proof.
- **The native run's wall time.** 173.7 s against 89.1 s for the same path in section 6. It is
  wall-clock only, its log is identical to the immediate run, and the likeliest cause is other
  slots' runs sharing the machine. That was not checked.

**Reference rows are unaffected in content.** They are taken lockstep, and this setting changes
presentation only.

## 8. A failed device creation ends the run cleanly (packet `cc9_tooling_renderer_init`)

**The failure.** From 17:12 on 2026-09-27 every run on this machine died with a null read, right
after `renderer init request 1600x900`. The crash record showed `bsp_game.exe+22a775`, and
`+22a605` on another build (cc9-lua2's `local\sq_off_usn04.log`, `local\cc9-lua2-probe_s2.log`).
- **Where it crashes.** In that tree's `build\win32\bsp_game.map` the two offsets are
  `set_native_renderer_render_state_00b24460+d5` and
  `initialize_native_renderer_default_states_00b26170+2f5`.
- **Why.** Both are called by `initialize_native_renderer_device_00b2aeb0` right after
  CreateDevice. 00B2AEB0 ignores CreateDevice's HRESULT, as the image does, so when device
  creation fails the next render-state call reads the null device.

**The change.** It is harness only and lives in `src/game_native_renderer_application.cpp`, in
the host's `create_device`.
- **The guard.** The host runs 00B2AEB0 under an SEH guard whose filter takes only an access
  violation while the device slot renderer+1A10 is still null. Every other exception, and every
  later one, still reaches the crash record (section 4).
- **The log line.** On that failure the host retries CreateDevice once, only to read an HRESULT,
  and releases a device if one appears. It writes one line and ends the process with **exit code
  4**:

```
harness renderer init failed: the device is null after 00b2aeb0's CreateDevice; retry hr=<hr> adapter="<description>" requested=<w>x<h> format=<fmt> windowed=<0|1> interval=<interval> display=<w>x<h>@<Hz>Hz session=<id> console_session=<id> remote=<0|1> input_desktop=<at failure>/<after retry> logonui=<0|1>/<0|1> (at the failure / after the retry); exiting with code 4
```

- **The success path.** A successful startup runs exactly as before: the guard adds an SEH frame
  and nothing else. 00B2AEB0 itself is unchanged.

**Verified on the failure path, 2026-09-27.** This worktree at `cc5d8f35d` plus this change, with
the machine in the state that crashed every run:

```
./tools/run_game.ps1 -Log local\rinit_try.log -- --frames 400 --press-start-frame 30 --menu-select USN02 --mission-frames 300 --mission-frame-seconds 0.05
```

The launcher printed `EXITCODE=4`. The log ends:

```
renderer init request 1600x900 fullscreen=0 vsync=1 antialias=8
harness renderer init failed: the device is null after 00b2aeb0's CreateDevice; retry hr=0x88760868 adapter="NVIDIA GeForce RTX 5090" requested=1600x900 format=21 windowed=1 interval=0x80000000 display=2560x1440@59Hz session=1 console_session=1 remote=0 input_desktop=Default; exiting with code 4
```

- **What the line says.** `0x88760868` is `D3DERR_DEVICELOST`. The console session is active and
  local.
- **Lock evidence (corrected 2026-09-28).** The evidence for a lock is the input desktop
  (`Winlogon`, printed as `unavailable (Winlogon or another secure desktop)` because
  `OpenInputDesktop` is refused there, against `Default`) and whether `LogonUI.exe` is running.
  A resident `LockApp.exe` is not evidence: it stays loaded after an unlock, and it was present
  throughout eight clean runs. This section's first version said the opposite, that the lock
  screen draws on `Default`, and that was wrong.
- **What the recorded failure shows.** The recorded failure read `input_desktop=Default`, sampled
  once after the retry. By the corrected criterion the input desktop was not the secure desktop
  at that moment, so this line does not establish the lock as the cause of the
  `D3DERR_DEVICELOST`.
- **What the line now prints (from `0d5f4d074`'s successor).** It samples both signals twice, at
  the failure and after the retry, so a desktop switch during the retry is visible. The new fields
  are build-tested only: runs have passed since 18:13, so no failure has printed them yet.

**Identity on the success path: held (2026-09-27, 18:21-18:33 local).** Device creation came back
intermittently: one smoke completed at 17:58 (cc9-ships2), every other run between 17:40 and 18:01
failed with `0x88760868`, and a smoke from this tree completed at 18:22. Both halves of the pair
were run while it worked.
- **The binaries.** They are two `tools/pair_export.py` exports that differ only by the guard:
  `local\rg_off` is the guard's parent `cc5d8f35d` (SHA-256 `B4DC7059F53B`), and `local\rg_on` is
  the guard commit `303acf928` (SHA-256 `8B849697DACA`).
- **The runs.** USN04 4700/4500 through `tools/run_game.ps1`, with `BSP_GUNNERY_RNG_STREAMS=1`,
  `BSP_DEATH_TABLE=1`, lockstep 0.05 and the default present interval (immediate). Logs are
  `local\rg_off_usn04.log` and `local\rg_on_usn04.log`. Both reached the final COM release, in
  34.9 s and 33.4 s.
- **The verdict.** `python tools/pair_diff.py local\rg_off_usn04.log local\rg_on_usn04.log` exits 0,
  identical apart from noise. That covers 44 death rows, 44 plane death modes, 81 unit rows, 1,653
  native rows and 268 summary lines, with 0 other lines differing. The guard is gameplay-neutral on
  the success path.

## 9. Handoff (cc9-tooling, 2026-09-28)

Everything in sections 1-8 is landed on main. Nothing is in flight: no lease, no run, and the
tree is clean. What follows is what a successor needs.

### The four proposed tools (awaiting the user's decision)

These are one line each. None of them is started.

| proposal | what it would do | evidence it rests on | recommendation |
| --- | --- | --- | --- |
| ranking generator from logs | Rank UNIMPLEMENTED native rows across a set of run logs by call count and mission coverage, to order reconstruction work. | Every log's native table carries status and calls (for example `UnitInstance::wake_setting` UNIMPLEMENTED with 17,603 calls on E2). `pair_diff.Run` already parses the table. | Build it. It is small, reuses `Run`, and needs no game run. |
| merge resolver for appended doc sections | Teach `tools/merge_resolve.py` to union two branches that each append a section at the end of the same doc. | These conflicts come up in integration. I have not measured how often; the lead's integration log would say. | Build it only if that count is material. It is a text union like the startup.cmake case. |
| deferred-name queue in the integrate script | Queue reviewed names whose Ghidra function is not defined yet, and apply them after `ghidra_define_function.py`. | Described by the lead. I have not read `integrate_workers.py`'s name step. | Needs a read of that step before sizing. It touches Ghidra writes, so it must take the write lock. |
| reference-row extractor | Print a mission reference table (damage, deaths, hits, shots, first hit, releases, water contacts, controlled moved, mission end, unimplemented) from a log, in the GAME_EXECUTABLE.md column order. | These rows are hand-copied into every baselines section. `pair_diff.Run.headline()` already extracts all of them. | Build it first. It is the cheapest and removes a transcription step. |

### How to read the renderer init failure line (section 8)

`harness renderer init failed: ...; exiting with code 4` appears only when CreateDevice left the
device null. Its fields:
- **`retry hr`.** CreateDevice run once more by the host, with software vertex processing, only to
  get an HRESULT. `0x88760868` is `D3DERR_DEVICELOST`. "(the retry succeeded: transient)" means the
  cause went away in between.
- **`adapter`, `requested`, `format`, `windowed`, `interval`.** The adapter description and the
  D3D present parameters 00B2AEB0 built. `interval` is after any `--present-interval` override.
- **`display`.** The desktop's current mode (`EnumDisplaySettings`).
- **`session`, `console_session`, `remote`.** The process's session, the active console session
  and `SM_REMOTESESSION`. A mismatch or `remote=1` points at a remote-desktop session.
- **`input_desktop=<a>/<b>` and `logonui=<a>/<b>`.** Sampled at the failure and after the retry.
  `Default` with `logonui=0` means no lock or logon screen had input. `unavailable (Winlogon or
  another secure desktop)` or `logonui=1` is the lock evidence. `LockApp.exe` is deliberately not
  reported, because it stays resident after an unlock.

The one recorded failure (2026-09-27, 17:40-18:01) read `input_desktop=Default`, so the lock screen
is ruled out as its cause, and the cause stays open. The two-sample fields are build-tested only.

### The lease overlap rule's known limit (docs/COORDINATION.md)

- **Absolute paths from another worktree.** Paths are normalised relative to *this* checkout's
  root. An absolute path into another worktree does not reduce to a repo-relative path, so it is
  compared as written and will not overlap that worktree's relative leases. Leases are claimed
  with relative paths in practice, so this has not happened.
- **No wildcards.** Globs are not understood: `src/*.cpp` is a literal name.
- **Case-insensitive comparison.** This is right on Windows, and would over-match on a
  case-sensitive filesystem.

### pair_diff's noise list

It is data in `NOISE` at the top of `tools/pair_diff.py`, and each entry cites its evidence
(section 1). A new counter that varies between identical runs of one binary gets a
`native-calls` entry, which ignores both its count and its presence, plus a row in section 1's
table.


## 10. Timed player orders, `--helm-orders <file>` (packet `cc9_scripted_helm_orders`)

- **Lines:** `<mission frame> moveto <unit> <x> <z>` or `<mission frame> moveto <unit>
  <navpoint>`, with `#` for comments.
- **Path:** each line is issued once, on its mission frame, as the player's moveto command form
  (005F9B20's MoveTo object 00E08F68, point descriptor and flags 1 into 0077D600).
- **Log lines:** `helm order applied: ...` for each applied order, and `helm order refused: ...`
  for an unknown unit, an unknown marker or a malformed line. The run continues after a refusal.
- **Without the option nothing changes,** and an empty file is identical too (USN04 pair_diff
  exit 0).
- **Harness limits** are listed in docs/SCRIPTED_HELM.md section 8.2: no HUD, selection or camera,
  and the point is taken directly.
- **Example:**

```
./tools/run_game.ps1 -Log local\x.log -- --frames 9200 --press-start-frame 30 --menu-select USN02 --mission-frames 9000 --mission-frame-seconds 0.05 --helm-orders J:\path\orders.txt
```

Pass an absolute path. The run spawns a second process, and the path is resolved against its
working directory.

**Added by packet `cc9_helm_orders_helm_route`:** two more line forms,
`<frame> takehelm <unit> <throttle> <x> <z>|<navpoint>` and
`<frame> moveto <unit> <x> <z>|<navpoint> repeat <seconds>`.
- `takehelm` takes the controlled unit's helm (role 1, `+184h`) and steers by the AI's own
  rudder law `009DA250`.
- `repeat` re-issues the moveto every N seconds while the unit is alive.
- The labels are in docs/SCRIPTED_HELM.md section 9.2.
