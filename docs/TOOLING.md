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
   table's concrete / unimplemented header.
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
