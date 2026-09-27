# Mission avoidance geometry bindings

Addresses: 00424D00 0041D1E0 0041CCD0 00417E40 00417E90 00417EF0 00422500
00423190 00417610 0041B840 0071C4F0 0080E000 00811D10 00811D80 00815F30

The rebuilt process now loads supported authored avoidance paths at the
004E07BE rebuild call and supplies the planner, search, stop-state bounds check,
and follower with real geometry. `GameAvoidZoneRuntime` owns the native24h
corner/storage records and stable process handles. The scene's Path and
Landscape creators remain unresolved: this is an explicit data projection,
not a claim that those native entities or the singleton ABI have been rebuilt.
Descriptive function names are hypotheses. Project/program remain
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`.

The reader retains numeric Point keys, local coordinates, composed world
matrices, and authored parent identities. The runtime uses that actual composed
matrix through007AF800's valid-cache path. It creates a named layer before
checking the original entity's Party at+54h. The value2 means Neutral; it is
unrelated to007B34F0's source-holder kind. All21 Marshall paths have that Party.
Their four parents are actual retained Landscape records, and the verified
004F1360 predicate accepts the constructor's0x44 query. Native scene `created`
flags remain false for unreconstructed creators.

0041CCD0 performs spacing, bounds clipping, winding and derived-corner work
with the existing native arithmetic and allocator boundaries. A successfully
constructed zero-corner zone is freed and omitted, matching0041D24B, while its
named group remains. Geometry snapshots preserve group/zone/corner order.
The00416DD0 query now calls exact004F3730 crossing and0085C910 SAT kernels.
Point pushes and nearest-boundary queries use the preceding native record and
distance work; left detour output maps to backward and right to forward.

Corner clearance uses the actual tracked critical section, enters it before
resolving the selected group, updates the original record's+20h cache, then
frees selected edge runs before unlocking. A stable compact follower anchor
is copied from the native record; the different layouts are never cast into
one another. Allocation/free reuse the existing malloc/new-handler service.
All query storage and the scene owner must outlive their borrowed identities.

The full semantic AI order pair now owns lateral subrecords as well as the four
published scalar fields. Slot0 is unit+AEC and slot1 is+A98. The current slot
is indexed directly; the reader uses the opposite published slot.0080E000
runs through the existing direct-control prologue before its gates.00815F30
uses actual corner X/Z and clearance plus the live front plan's width.
009F4D10 changes only+40/+44/+48/+4C; promotion clears the old valid flag,
flips the index, and00811D10 copies only+00..+3F into the new current slot.
Both timers start at-1 as written by0081EF85/91. The persistent storage is a
semantic record, not a native84-byte allocation; the owner is this controller's
unit index. It is separate from GameUnitsHost's20h control-order ring.

World bounds come from the actual Map block, with native multiplayer selection
and0071C4F0's inclusive-edge/unordered behavior. See `WORLD_MAP_BOUNDS.md`.
The path-cost ramp comes from the live ShipGlobals Lua state, yielding
0.261799395/1.39626336/1200 for the installed script. See
`SHIP_AI_PATH_TURN_RAMP.md`. The CRT dispatch flag is now one process-owned
value shared by sound and geometry. The legacy math-runtime aggregate is also
persistent; the previous binding retained a pointer to a temporary aggregate.

Validation: Win32 Release and both existing CTests passed.24 recorded runtime
call sites match live Ghidra. The final build completed120 USN01 mission frames
with21 zones,2193 corners,21 parent associations,6 groups, and authored bounds
NW(-10000,0,10000)/SE(10000,0,-10000). It ran18480 AI steps,240 search ticks,
239 follower points and18480 order promotions, then shut down with exit0 and
no FMOD errors. No corner-detour arm or lateral publish occurred in that run.
Corner selection/clearance therefore retains the separately documented
original-byte fixture evidence; gameplay detour behavior is not established.
After merging current main, a second120-frame run accepted the supported
`moveto:Airfield2` command for Enterprise, completed238 search ticks and three
plan swaps, and exited0. It also reached no corner-detour arm. An earlier
attempt hit another orchestrator's single-instance mutex; that process was
left untouched. `MoveToPos` is an internal state description, not a recognized
command token; the successful command uses the registry's `moveto` spelling.
The differential probes and limitations are in `AVOID_ZONE_CLEARANCE.md`,
`WORLD_MAP_BOUNDS.md`, and their reports. No new tracked tests were added.

Use the verified private Microsoft XLive files described in
`XLIVE_PRIVATE_RUNTIME.md`, explicitly selecting the XLive DLL and preloading
its matching msidcrl40 dependency, plus the installed FMOD DLL paths. The
installed AlterBSP replacement attempts to patch an original-game address in
the rebuild and crashes inside xlive.dll+319049. The Microsoft system DLL
alone instead fails with error182 because the system credential DLL lacks
ordinal43. The existing extracted matching Microsoft pair resolves both
problems. No installed game or Windows file was changed; DLLs remain ignored
local runtime inputs and are not committed.

Remaining coverage: native scene creators/singleton lifetime, physics hull work
after00424DDF, unrelated engage/sector contracts, and documented approximations
in the existing search/detour/follower routines. The historical runtime log
field `draft_layers=unresolved` refers to that physics tail; follow-up analysis
shows it selects existing groups and builds physics hulls rather than inserting
additional zone groups. Unsupported named path/parent interfaces fail explicitly
instead of supplying a synthetic empty world. No binary replacement, native
SEH, visual/render parity, or complete gameplay validation is claimed.

Evidence: `reports/game_avoid_zone_runtime.json`, the five dependency reports
it references, and the preserved ignored runtime/probe artifacts.

## Addition from docs/SHIP_AI_AVOID_ZONE_SEARCH.md

The geometry owner now exposes the actual009D7050 cache refresh,004158A0
selected-list cleanup and004158E0 crossing through explicit caller-owned cache
and list records. Its semantic/native group identity and zone order are shared
with the existing loaded map. The adapter rejects an empty manager table.
This supplies a concrete cache dependency; the ship controller's sector gates
remain pending persistent director state and00415970 arc clipping. No new
runtime steering or native singleton-ABI claim follows from these methods.

## The zone creators on the measured missions (packet `cc9_ring_scan_probe_usn01`, 2026-09-27)

`docs/AVOID_ZONE_REGISTRY.md` is leased to cc9-plane-release, so this finding is recorded here.
It is doc-only, and no source changed.

**The creation is already bound.** `GameAvoidZoneRuntime::rebuild` reproduces 00424D00's walk
over the world entity list (`[world+370h]`). An entity named `AvoidZoneG <word> <layer> #nnn`
goes through these steps:
- 00424DAE parses the layer;
- 00424DBE `00417CA0(layer)` finds or creates its group;
- 0041D1E0 adds the entity when its Party `+54h` is 2 and its path interface answers;
- 0041CCD0 builds the polygon from the class-47h path, with its class-44h parent's matrix.

**What the three scenes author** (this installation, `universe/Scenes/missions/USN/`):

| scene | date | `AvoidZone` entities | `landscape` lines | include / `.scn` lines | runtime census |
| --- | --- | ---: | ---: | ---: | --- |
| `usn_19_coralus.scn` (USN04, E2) | 2024-08-09 | 0 | 0 | 0 | `groups=1 zones=0` |
| `usn_2_java.scn` (USN02) | 2024-07-13 | 0 | 0 | 0 | `groups=1 zones=0` |
| `usn_1_marshall.scn` (USN01) | - | 21 (the first `AvoidZoneG all 1 #001` at line 3450) | 47 | 0 | `groups=6 zones=21 corners=2193` |

So `zones=0` is exact for the two reference missions: nothing is missing there.

**What `native_scene_creators=unresolved` means.** It is a separate gap: the image's native
creation of the Path and Landscape scene entities themselves.
- **Landscape**, class 44h: creator 004F1460, IsKindOf answer 004F1360, vtable 00CEA090.
- **Path**, class 47h: creator 004EA650, IsKindOf answer 00480930, vtable 00CE6290 (both from
  `docs/ENTITY_CLASS_IDS.md`).

The runtime reads those records as data from the retained scene parse, and sets no native
`created` flag on them. The zones it builds are the image's, as long as no later reader needs a
live Path or Landscape object. Binding the two creators (their constructors, the path interface
007AC9D0 on a live object, and the landscape matrix) would be a scene-contents packet of its own.
It moves nothing on USN02 or E2, which author neither class.

### The USN01 probe pair: predictions, written before the runs

One tree (main `7eb3679dd`), with `kShipAiRingScanProbeBound` false against true (its landed
state). USN01 3200/3000, streams and the death table on.
- **Zones.** Both sides have 6 groups and 21 zones. The ships' navigation layer, class+560h = 11
  (`docs/AVOID_ZONE_ESCAPE.md`), is the group holding 4 zones: `AvoidZoneG all 11 #008` and the
  three `#011` copies under parents 93, 99 and 105.
- **The probe is never reached on USN01.** The day's USN01 control (`local\fp2_ctl_usn01.log`)
  has `ring_scans=0` and no `ShipAiRingScan::*` row. 009E6640 runs from the approach ring scan
  009E76D0, and no USN01 ship enters it.
- **So:**
  - `ring probe spaces=0 moved_starts=0 casts=0 hits=0` on both sides;
  - the whole native table, every summary line and every death row identical, with 7 deaths, 141
    hit records and 447 shots;
  - the rack drop's 8 torpedo drops appear on both sides. That move belongs to the control
    against reference c, not to this pair.
- **Consequence.** The probe cannot be measured against real zones on any of the four reference
  missions. A mission that both authors zones and puts ships into an approach would be needed;
  none was surveyed here.

### The USN01 probe pair, measured

- **Builds:** a `git archive` export of main `7eb3679dd` plus the finding commit `76d9e3ecf`, in
  `local\ap_src`. The switch is flipped in the export only: `local\pu_off`
  (`1F97F05CB460`) against `local\pu_on` (`B4DA67FE2547`).
- **Logs:** `local\pu_{off,on}_usn01.log`. Both show the 1600x900 override and their own module
  directory, and both exited 0.

**Identity, as predicted.**
- Both sides load `groups=6 zones=21`. Both read `ring probe spaces=0 moved_starts=0 casts=0
  hits=0`: no USN01 ship reaches the approach ring scan.
- The whole native table (1,299 rows), every summary line but the probe line's `bound=` field,
  and every death row are identical.

**A failed prediction, on absolute values.** Both sides read 7 deaths, **150** hit records (85
hull), **583** shots and **0** torpedo drops. That differs from the afternoon's control
(`local\fp2_ctl_usn01.log` on `e3aba0f36`: 141 / 447 / 8 drops).
- The move is between the two trees, not inside this pair. Main gained other landings between
  `e3aba0f36` and `7eb3679dd`.
- The USN01-relevant candidates were paired only on USN04 and USN02: the planes' avoid-zone layer
  sample `0d02479e5` (Marshall has terrain, unlike the reference seas) and the dive bombers'
  carried rounds `e3f5d58ab`.
- This is flagged for reference d (`docs/GAME_EXECUTABLE.md`, reference c section).
