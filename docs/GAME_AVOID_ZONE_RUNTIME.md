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
