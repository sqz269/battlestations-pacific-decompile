# BSP coverage runs, 2026-10-30 UTC

The four serial runs passed the requested startup/frame/exit gate on one frozen
optimized coverage binary. They establish execution evidence for a function
census. They do not establish deterministic replay or original-game equivalence.
No source rebuild or source change occurred between cases.

## Frozen build and artifact identity

Build source is `9c79eedff6231a0140c6dc401f5305e8526bc4fd`. The run checkout was
`a69726f99a04fb9e35392a1a4b5f34f443434450`; its only difference from build source
was `docs/COVERAGE_BUILD.md`. Both Win32 Release build directories remain in
`J:/PROG/battlestations-pacific-decompile-codex-coverage-build/`.

`BSP_COVERAGE_PDB=ON` adds compiler `/Zi` and linker
`/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO`. Default OFF is unchanged.
Release optimization remains MaxSpeed. Builds used `BSP_CL_MP=1`, at most two
MSBuild nodes each, at most four concurrent compilers total, and Normal priority.
[COVERAGE_BUILD.md](COVERAGE_BUILD.md) gives configure/build commands and the
normal/coverage layout comparison.

| Artifact | SHA-256 |
| --- | --- |
| Coverage EXE | `128fba34c171517a605d0188c00fac76f8edf020b9a6dbad51da6d192c9c6d93` |
| Coverage PDB | `6545e1ea9eefd884b969264f15e74b8f714eaac90bdab4268746089a403e61b4` |
| Coverage map | `66c9f7ab3f36c986cf89094c3c66bf519aee05e457c7006ce5183425444147d6` |
| Coverage xlive_stub.dll | `fd626e9e4ef1b0b413a9213a802ee6e34e35b8319d0f3752e670dd63e3f66d8e` |
| Normal EXE | `dd486dbf0ea2cb426c7374b68561382c5d1840e9239e1cdd9f3aed02bedeb461` |
| Normal map | `1b46b88d0f8e7e258f5217174aebba27298aacb7fe0873c728d0987f4a2f9dec` |
| Normal xlive_stub.dll | `9bcd9f68119a16b3bc0cab1fa164118ba85d3701dc74ca9f1971feea01dc7f70` |

The coverage EXE is 7,498,240 bytes, PDB 92,614,656 bytes. CodeView GUID is
`dcb824dc-35b7-495b-88c3-b1528a05f247`, age 1. Its main image base is
`0x10000000`, end `0x10732000`, entry `0x1056BE00` in both traced children.
The maps have the same 36,087 function-symbol-name/RVA sets; this is not a game
function count. `.text` layout and all nonrelocated code bytes match the normal
build; relocated operands differ. The two EXEs are not byte-identical.

## Actual four-case results

| Case | App limit/completed | Mission requested/ran/simulated | Wall seconds | Root/launcher exit | Drain |
| --- | ---: | --- | ---: | --- | --- |
| smoke-untraced | 620 | 300/300/300 | 8.370 | 0/0 | natural |
| smoke-traced | 620 | 300/300/300 | 74.451 | 0/0 | natural |
| reference-untraced | 5000 | 4800/4800/4800 | 72.308 | 0/0 | natural |
| reference-traced | 5000 | 4800/4800/4800 | 171.016 | 0/0 | natural |

Every case entered state `0x0D`, resolved
`universe/Scenes/missions/USN/usn_19_coralus.scn` and
`Scripts/missions/USN\usn_19_coralus.lua`, reported effective difficulty 1,
and controlled `Fletcher-class01`. The mission windows represent 15 and 240
simulated seconds at `--mission-frame-seconds 0.05`.

Common game arguments were `--press-start-frame 30 --menu-select USN04
--mission-frame-seconds 0.05 --difficulty 1`, with the app/mission limits above
and a distinct `--settings-personal-root` per case. The long recipe used exact
`--frames 5000 --mission-frames 4800`; no cap deviation was needed.
`config/run_game.json` supplied `--window-monitor smallest
--window-resolution fit --present-interval immediate`.

Each isolated shell cleared other `BSP_*` variables and set exactly
`BSP_DEATH_TABLE=1` and `BSP_GUNNERY_RNG_STREAMS=1`. The exact coverage EXE and
stub paths were supplied to `tools/run_game.ps1`, using slot 13/core 28,
`-WaitSeconds 7200 -NoForceStop`. No `--hardware-probe-commit` was passed.
The wrapper honored shared slot/DRAIN handling. The launch shells selected
Normal priority; traced smoke and both long cases had both own processes
observed at Normal, with seven total BSP processes including the five old ones.
The first untraced smoke ended before external priority sampling.

The executed script is archived as `commands/launch-case.ps1`. It was invoked
serially with `-Case smoke-untraced`, `smoke-traced`, `reference-untraced`,
then `reference-traced`. Traced cases prepended official x86 DynamoRIO
11.91.20749 with `-follow_children -t drcov -logdir <private case directory>
-- <exact coverage EXE>`. The actual prefix, binary hash and final argv are
retained in each traced manifest. No original game overlapped these cases.

## Measured absolute start frames

All four logs contain every `native frame complete: frame=N` ordinal from 1
through the app limit, without a gap or reset. Events have the same bracketing
completed-frame markers in traced and untraced cases:

| Event | Previous completed renderer frame | Current renderer frame | Zero-based app index |
| --- | ---: | ---: | ---: |
| Press-start injection | 30 | 31 | 30 |
| Mission state entry | 38 | 39 | 38 |
| First simulated mission frame 1 | 39 | 40 | 39 |

This uses existing instrumentation rather than assuming that equal arguments
imply equal starts. In the frozen source:

- `GameLoopCallbacks::frame` runs the application update, then one
  `clear_and_present`, then increments its app count (`src/game_hosts.cpp`,
  lines 1361 and 1394-1399).
- `GameFrameHost::game_on_move` passes `frame_index_` to the menu then increments
  it (lines 1279-1280); the member starts at zero
  (`include/bsp/game_hosts.hpp`, line 499).
- Native end-frame increments renderer `+0x14` once
  (`src/native_renderer_end_frame.cpp`, line 229). The existing marker prints
  that value immediately afterward
  (`src/game_native_renderer_application.cpp`, line 1259).

`run-receipt.json` retains each event's actual log text, line number and adjacent
frame values. All four cases match on these measured starts and exit 0.

## Private write paths and real-data checks

Real source roots were the installed game and
`C:/Users/sqz269/Documents/Battlestations-Pacific`. Every run asserted that the
ordinary real product and `save` directories already existed. The native
`GameSaveStorageHost` asks for CSIDL_PERSONAL with create=true and calls
`CreateDirectoryA` for those two existing directories only
(`src/game_hosts.cpp:1042,1050`; `src/platform_window.cpp:277`). This is not a
general save-root override.

The approved game/CWD was the retained full-byte private copy:

```text
J:/PROG/battlestations-pacific-decompile-codex-sampler-gaps/local/original-baseline/stage-sampler-gaps-20261029/game
```

Its staging route uses `robocopy /E /COPY:DAT /DCOPY:DAT /XJ`, without hardlinks.
Preflight checked five critical executable/DLL/script/scene files as independent
inodes, link count 1, no reparse, and source-equal SHA-256. This matters because
the Lua guard blocks only `os.execute`/`io.popen`; relative Lua log writers
remain. It is not a filesystem read-only guard.

Each case copied real options bytes verbatim into its own local
`Documents/Battlestations-Pacific/options.txt` and passed the parent Documents
directory through actual `--settings-personal-root`. The native settings
`TextCalls::personal_folder_00ce22f0` returns this override
(`src/game_native_settings_application.cpp:27-35`). All four private options
files still matched their source-before hash after execution.

The actual profile backend is `ReadOnlyProfileIoHost`
(`src/game_profile_load.cpp:456-607`): immediate GENERIC_READ/OPEN_EXISTING reads
bypass queued-read validation stamping; write requests and non-read operations
throw. `BSP_PROFILE_LOAD_ROOT` remained unset. Mission checkpoint binding is
false (`src/game_hosts_lua.cpp:394,775`); EndScene flush is record-only
(`src/game_hosts_mission_result.cpp:659`). The stand-in's optional
`BSP_XLIVE_STUB_LOG` was unset, and actual traced game-child module tables
confirm the exact frozen `xlive_stub.dll`, not the installed custom XLive DLL.

Before and after every case, `tools/original_baseline.py` recorded 17 selected
real runtime files including the original EXE/XLive, plus the four real
options/save files. Hash, identity and timestamp checks were unchanged in all
cases. These are the recorded inventory scope, not a hash census of every
installed asset. No force-stop occurred. Final inventory retained exactly the
five preexisting game PIDs 40812, 15240, 125708, 115880 and 49148; no own or
original game survived, and slot 13 was released naturally.

## Raw process/module records and limits

| Trace | Role | PID / parent PID | Main-module base | All BB records | Main-module BB records |
| --- | --- | --- | --- | ---: | ---: |
| smoke | bootstrap | 101620 / 113036 | `0x10000000` | 24,988 | 1,412 |
| smoke | game handoff child | 102900 / 101620 | `0x10000000` | 535,423 | 139,078 |
| reference | bootstrap | 92948 / 99084 | `0x10000000` | 25,546 | 1,402 |
| reference | game handoff child | 20456 / 92948 | `0x10000000` | 573,208 | 175,058 |

The game child is identified by its exact EXE path, creation/parent identity,
and `--bsp-native-data-handoff` command line. Each process has its own drcov
log. Counts above are raw eight-byte BB table records, not deduplicated blocks,
function totals, reconstruction credit or complete coverage proof. Bootstrap
execution is not full mission execution. Function attribution is a separate
census packet using the matching PDB/map and measured image base.

Long-run combat totals differ: untraced dispatched 695 hits, 57 deaths and
12,859.0 damage; traced dispatched 810 hits, 56 deaths and 13,494.6 damage.
The cause is unknown; this pair does not isolate tracing from existing run
nondeterminism. Matching start, frame count and exit is not gameplay equivalence.
Reconstructed menu startup also differs from original direct-scene startup.

## Retained evidence

The immutable-copy archive is the main checkout's
`local/coverage-census-20261030/bsp/`. `artifact-manifest.json` records every
source/copy size and SHA-256. It contains final binaries/maps/PDB, build
comparison JSONs, readiness/index provenance, command/audit/preflight evidence,
logs/manifests and before/after inventories. Run receipt SHA-256 is
`f46ba9d3ecb189b5d6ae462e0a608d0a7d6e4b012bb3e33245bb61691ded6208`.

The actual raw drcov logs remain under the frozen worktree's
`local/coverage-serial-20261030T040428Z/{smoke-traced,reference-traced}/drcov/`
for the census worker. No raw logs or worktrees were deleted. The deferred
six-file patch and handoff are archived under `bsp/deferred/`; they are not
landed changes and remain deferred.
