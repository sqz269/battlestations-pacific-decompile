# USN04 execution coverage census

The guarded original run observed **7,097 unique game-function entries** in
USN04. The frozen reconstruction run reached **1,740 source-verified C++ body
mappings**, including **342 observed only through inline sites**. The original
ended at native GameTime **240.05098**, fixed-step count **4,800**; the
reconstruction completed 4,800 mission frames / 240.00 simulated seconds.

The original uses the brief's one-shot debugger-entry fallback after two DR
startup failures. Its unhit functions remain **unknown**, and its BB and call
counts remain null. The reconstruction uses complete drcov tables plus PDB
outer/inline attribution. These are observed reach lower bounds from different
methods, not behavior, ABI or parity proof.

`config/coverage_census_summary.json` supplies board `orig_live=7097` and
`bsp_live=1740`. All observed addresses belong to the pinned 20,892 board target.
The full frozen game-classified table has 21,770 rows; its 878 outside-target
addresses remain separate. The reconstruction executable is frozen at
`9c79eedff6231a0140c6dc401f5305e8526bc4fd`; later Main changes were not executed
by that reference run. Long maneuvers, campaign/direct-scene equivalence and
motion-trace parity remain deferred.

## Tracer and build provenance

The official [DynamoRIO release](https://github.com/DynamoRIO/dynamorio/releases/tag/cronbuild-11.91.20749)
was downloaded into ignored `local/tools/dynamorio/`, without an installer or
system PATH change. Version: `cronbuild-11.91.20749`, published 2026-10-24.
The 467,145,305-byte Windows ZIP SHA-256 is
`9ef2d8f990a6acb4d303385fec0105bd45e54d36e92269494ac8d45e966c3844`,
matching its official release asset digest.

`BSP_COVERAGE_PDB` defaults OFF. ON adds `/Zi` and
`/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO` to separate Win32 Release builds.
The same-commit normal/coverage builds completed with exit 0. All 36,087 common
map function-name RVA sets and all section RVAs match; `.text` size is identical.
The 11,109 differing text bytes are confined to matching relocation operands;
nonrelocated text bytes match. Data/debug layout differs, so neither byte-
identical executables nor identical complete images are claimed.

| Artifact | SHA-256 |
| --- | --- |
| Coverage EXE | `128fba34c171517a605d0188c00fac76f8edf020b9a6dbad51da6d192c9c6d93` |
| Coverage PDB | `6545e1ea9eefd884b969264f15e74b8f714eaac90bdab4268746089a403e61b4` |
| Normal EXE | `dd486dbf0ea2cb426c7374b68561382c5d1840e9239e1cdd9f3aed02bedeb461` |
| Original EXE, classic entry trace | `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |

The full PDB is 92,614,656 bytes, GUID
`dcb824dc-35b7-495b-88c3-b1528a05f247`, age 1. Offline attribution verified
that identity against the exact EXE. Details: `COVERAGE_BUILD.md`.

## Actual reconstruction runs

All runs used `tools/run_game.ps1`, Normal priority, shared slot/DRAIN protocol,
the byte-copy private installed game data, per-case private settings/Documents,
and exactly the two reference BSP variables `BSP_DEATH_TABLE=1` and
`BSP_GUNNERY_RNG_STREAMS=1`. Source runtime hashes (17) and options/save files
(4, including timestamps) remained unchanged. Read-only profile IO rejects
writes; checkpoint binding is OFF; mission-result storage flush is record-only.
Existing real product/save directories were asserted before bootstrap calls.
The actual traced children loaded the exact frozen `xlive_stub.dll`.
See `COVERAGE_BSP_RUNS.md` for the write audit, commands and process receipts.

| Run | App / mission frames | Wall seconds | Exit | Mission entry / first tick, app index |
| --- | ---: | ---: | ---: | ---: |
| Smoke, untraced | 620 / 300 | 8.370 | 0 | 38 / 39 |
| Smoke, traced | 620 / 300 | 74.451 | 0 | 38 / 39 |
| Reference, untraced | 5000 / 4800 | 72.308 | 0 | 38 / 39 |
| Reference, traced | 5000 / 4800 | 171.016 | 0 | 38 / 39 |

The long arguments were exactly `--frames 5000 --press-start-frame 30
--menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05`.
This produced 240.00 simulated seconds. Source-verified completed-render ordinals
recover entry/first-tick app indices; matching starts are measured, not inferred
from identical command arguments. All runs drained naturally; no old process
was stopped and no owned game process survived.

Long traced bootstrap PID 92948 and game child 20456 are both included.
Their module tables bind the frozen executable at base `10000000`.
The whole-run union includes startup/menu blocks. Drcov has no per-block game
timestamp, so the collection cannot be trimmed into a mission-only set.

Combat outcomes differ: the untraced long run reported 695 dispatched hits,
57 deaths and 12,859 damage; traced reported 810, 56 and 13,494.6. The cause is
not established. Fixed mission delta and matching starts therefore do not
establish deterministic gameplay equivalence.

## Attributed reach and uncertainty

The actual logs use drcov file version 3, module table 5. Version 3 offsets are
relative to a segment; version 2 offsets are relative to its containing module.
The strict parser handles both separately. Both complete long BB tables contain
176,460 raw main-module records, deduplicated to 104,884 distinct blocks in the
bootstrap+child union. Child alone contributes 104,559; 1,003 are shared and
325 are bootstrap-only. Repeated block records are not function call counts.

The PDB scan covered every one of 2,346,688 observed bytes, with no API or
inventory errors. Qualified/source identity and native-address suffix evidence
exclude bare foreign leaf-name collisions such as `std::next`. Earlier
provisional 2,191 and 2,173 mapped-name counts are superseded. A source-body
review removed 433 apparent credits carried only by mismatched identities.
For example, a HUD adapter named with0059CB90 is not the declared frontend
body; legitimate ship/flight-body and host-HUD-body hits remain. A native
address suffix creates a candidate only; confirmed body source or exact
qualified identity is required, and a known source mismatch rejects it. The separate smoke count is 1,571.

| Frozen game-address state | Count |
| --- | ---: |
| Identified executed mappings | 1,740 |
| Of those, function-ledger mappings | 1,740 |
| Observed only via inline sites | 342 |
| Matched identity, not observed in this run | 1,989 |
| Execution attribution remains unknown | 18,041 |

The frozen census table contains 21,770 game-classified addresses, including
unknown unreferenced rows outside the board's 20,892 reconstruction target.
Those are different denominators. All 1,740 observed body mappings belong to
the board target; none comes from its excluded unreferenced addresses.

Uncertainty buckets overlap: 74,085 unresolved PDB site records (23,635 unique
names), 284 ambiguous executed candidates (275 without another definite hit),
85 inventory candidates without source confirmation, 206 source-identifier-
unknown wire rows and 16,408 rows without a verified matching body/inline record.
Missing records are not definitely labelled optimized away, removed or ICF-folded.
No host/adapter suffix, source-wiring flag, OFF switch, vendor body or shared-RVA ambiguity alone
receives game execution credit. Source-wiring states remain labels from the frozen analysis, not execution proof.

| Board category | Original entries | BSP bodies | Candidate frontier |
| --- | ---: | ---: | ---: |
| ai | 598 | 203 | 11 |
| core | 415 | 152 | 35 |
| files | 162 | 95 | 44 |
| gui | 849 | 127 | 61 |
| lua | 337 | 141 | 34 |
| mission | 283 | 48 | 9 |
| net | 121 | 13 | 5 |
| other | 144 | 27 | 60 |
| render | 1,460 | 371 | 465 |
| scene | 813 | 177 | 46 |
| sound | 237 | 77 | 63 |
| units | 1,678 | 309 | 59 |

Host/adapter candidate reach is separate:1,233 records mention882 candidate
addresses;471 have no independent positive body hit. These hints overlap
other audit buckets and are not reconstruction-body execution credit.

## Original run and wiring candidates

The successful fallback ran the same private installed-game byte copy at
Normal priority through `run_original_baseline.ps1`, the shared slot/DRAIN
protocol, `medium-write-locked` token and 69,700 independently retained source
read/no-delete handles. The classic debugger redirects only the application
Documents import before entry, retains the exact known-xlive exception filter,
and rejects additional executables before continuation. No new process cap or
DynamoRIO instrumentation was added to this classic mode.

```
pwsh -NoProfile -File tools/run_original_baseline.ps1 -RunPrepared -TokenMode medium-write-locked -HandleKnownXLiveDebugClose -FunctionEntryCoverage local/original-coverage/function-entry-targets.json -StageRoot local/original-coverage/stage-usn04 -RuntimeSeconds 3600 -WaitSeconds 7200
```

The launcher supplied exactly `universe/Scenes/missions/USN/usn_19_coralus.scn`.
The visible HUD showed USS Lexington and the protection objective. The main
window was activated; no maneuver/fire inputs were sent. Real engine
`Loading scene` and subsequent `GGame::SceneInit()` lines, in the fresh log's
same epoch, gate the clean positive native clock. The argument echo is rejected.

| Original entry run | Measured value |
| --- | --- |
| Runtime source head | `bb875438fd9787c808c7873c69f49e300718ee72` |
| Root PID / initial thread | `612` / `79876` |
| GameTime, first mission / end | `0.05000000074505806` / `240.0509796142578` |
| Native fixed-step count | `1` / `4800`, delta about `0.05` seconds |
| AppOnMove counter | `331` / `14631`; these are not mission-relative frames |
| Mission clock span | `240.0009796` native seconds over `244.8485008` wall seconds |
| Clean / torn / initial failed reads | `2515` / `135` / `41`; one retry permitted |
| Function-start points / hits / skips | `21770` / `7097` / `0` |
| Hit threads / queued duplicate traps | `31` / `2639`; duplicates are not call counts |
| Effective native difficulty / BSP reference | `0` / `1` |
| End classification | Intentional owned-job forced window stop, game `E0000002`; guard exit `0` |
| Cleanup / preservation | Job `[0]` before lock release; 17 installed runtime and four real profile files unchanged, including timestamps |

Only CONTROL/EIP is restored after each owned one-shot INT3. No trap flag,
rearming, per-function call counting or fake one-byte basic block is used.
Pending cross-thread traps at an already restored point are rewound safely.
The native fixture exercised 64 threads, 4,096 actual calls, 32 unique points
and 72 queued traps. Fixture clocks never qualify as retail mission evidence.
The frozen starts lack a separately cached per-entry instruction-boundary
proof; that provenance limit and possible interior-entry misses remain explicit.

The target manifest SHA-256 is
`ea21cb138729b25d18951be235f1bf8d69eccb6723c81297d603931e16f832e7`;
the qualified first-hit trace SHA-256 is
`36044442b9b05bc93ac30e57f3a5d09660bd75f826dcf1f4829887ec755764c3`.
Image, manifest, index, wire snapshot, PID/creation FILETIME and terminal scene,
clock, preservation and empty-job proofs are checked by the consumer. The same
frozen 77,189-function index and wire snapshot are used for both sides.

### Comparison

| Frozen game-address cell | Count |
| --- | ---: |
| Original entry observed / BSP body observed | 1,429 |
| Original entry observed / BSP body known, not observed | 892 |
| Original entry observed / BSP attribution unknown | 4,776 |
| Original unknown / BSP body observed | 311 |
| Original unknown / BSP body known, not observed | 1,097 |
| Original unknown / BSP attribution unknown | 13,265 |

All 7,097 original positives belong to the board target; 110 lack a
function-reconstruction-ledger record and are listed separately in the detailed
result. The original has 13,795 unhit board targets, plus 878 outside-target
unknowns. Undefined-code execution, EH and vendor-body execution are unprobed
by this entry point set, not measured zero. Original-no/BSP-yes and both-no
conclusions are unavailable with this method.

The **892-row candidate frontier** is original-entry-positive and BSP-body-
known-but-unobserved. Its rank uses static original caller count descending,
then address, because original executed-BB counts are unavailable. First-hit
sequence is audit data, not frequency. Frozen wiring labels are 778 `wired`,
15 `ready` and 99 `callees_done_no_wired_caller`; source wiring alone did not
establish body execution. Another 4,776 positive-original / unknown-BSP rows
form an attribution-review queue, not confirmed wiring gaps.

| Address | Analysis name | Static callers |
| --- | --- | ---: |
| `004072d0` | `BSP_LegacySboString_Destroy` | 768 |
| `00408720` | `BSP_CharacterString_AssignCounted` | 650 |
| `00888aa0` | `BSP_ObjectHandle_FromLuaTable` | 281 |
| `004261a0` | `BSP_NativeString_Concat` | 223 |
| `00415350` | `BSP_SingletonLifetime_GetManager` | 210 |
| `00b66250` | `BSP_LuaObject_GetBoolean` | 132 |
| `004bec00` | `BSP_InputManager_GetSingleton` | 105 |
| `00b6da70` | `BSP_SceneNode_SetVisibilityFactor` | 95 |
| `00b74640` | `BSP_GeneratedModel_GetGeometry` | 95 |
| `00abbe50` | `BSP_GuiText_SetLocalisedCString` | 91 |

Host groups prefer 1,830 cached frozen-wire rows; remaining groups explicitly
use the current source at `bb875438...`, with file hashes. They are not all a
snapshot of the frozen BSP build. Groups overlap: 749 frontier rows have no
textual host mention; the largest named groups include units (46), Lua (21),
gunnery (21), script orders (19), mission frame (16) and scene contents (14).
Check current Main before turning this historical candidate list into work.

Original startup is direct-scene; BSP uses its menu path. Native difficulty
`0` differs from BSP `1`; seeds and scheduling are not matched. Even BSP's
same-executable traced/untraced combat totals differed. These limits prevent
calling this frontier a pure missing-implementation list or a parity result.

### Rejected DR attempts

Both real DR attempts passed entry admission, the 69,700-object audit, source
locks, Documents/stdcall/FP/CALL5 checks and complete startup BB-table parsing.
They did not reach a valid mission clock or SceneInit. All owned jobs drained
before parent lock release and both external inventories found zero changes.

| Attempt | PID | Result |
| --- | ---: | --- |
| live01 | 98252 | Startup exit `C0000005`; complete 328,332-row BB file. |
| live02 | 99608 | Same exit; complete 313,131-row BB file; one pass-through AV observation in `nvd3dum.dll`, RVA `001B0285`, reading `00000040`. |

The observed NVIDIA module version is `32.0.16.1742`, SHA-256
`617c9af288fb28023f29cca57963c219aa117647df0fea75c144765efc048fe1`.
The observation does not establish the initiating cause or a driver defect.
The missing `causticstexturesource` warning also occurs in the successful native
baseline. DR kernel child limits and classic debug-event child rejection are
unmatched compatibility variables. No driver, asset or power setting changed.

GPU-native exclusion can miss callbacks into game code; the pinned Windows
retakeover hook is unimplemented. It cannot justify negative execution claims:
[pinned implementation](https://github.com/DynamoRIO/dynamorio/blob/cronbuild-11.91.20749/core/win32/native_pe.c#L52-L62),
[official issue](https://github.com/DynamoRIO/dynamorio/issues/978).
Stock drrun lacked restricted creation/explicit-handle support; suspended attach
failed before entry, PIN's official download returned 403 and stock TinyInst
needed a pre-entry adapter. The classic fallback was used without relaxing the
existing source protections. See `COVERAGE_ORIGINAL_GUARD.md`.

## Artifacts and limits

The main checkout's ignored `local/coverage-census-20261030/` retains:

- `coverage_census_paired.json` (149,155,444 bytes), SHA-256
  `3fb3cd7e44cbe342f7ebd5cf1d07785bc709350d0a94baefab316af5737ce97c`;
- `ranked_wiring_frontier.csv`, `original_observed_bsp_unknown.csv` and
  `paired-summary.json`;
- the prior BSP-only detailed census, smoke result, frozen EXE/PDB/map/stub,
  index/wire provenance, compressed final trace inputs and private fixture proofs;
- original DR startup attempts and qualified `original/entries-live01/` evidence,
  source inventories, exact launcher sources, clock/entry logs and peer reviews.

Working output is `local/output/coverage_census_paired.json`; the board reads
only the committed summary. Raw intermediate traces are removed only after
hash-verified compressed preservation and completed attribution. `coverage_census.py`
keeps explicit unknowns and a future count interface; the one-shot source does
not provide call counts. Coverage establishes reach in these named runs, not
correctness, ABI compatibility or behavior parity.
