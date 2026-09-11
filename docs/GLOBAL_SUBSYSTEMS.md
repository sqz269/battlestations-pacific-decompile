# Global subsystem construction

Addresses: 004DC6A0. The name is a hypothesis, not a recovered symbol.

`construct_global_subsystems_004dc6a0` replaces the whole-phase callback in
`MissionSceneLoadHost`. Mission loading now supplies current publication slots,
individual native services, and the existing Lua runtime, then calls this body.
It directly executes the global script folders, race and robot configuration,
recon table hierarchy and marker loader, then constructs and loads the canonical
panel and weather owners.

The verified listing runs from004DC6A0 through RET004DC93F. ECX is the game;
there are no stack arguments. This C++ API uses borrowed slots and explicit
services; it does not reproduce the native game layout or calling convention.

The pooled `Game_Global` temporary is released immediately after FileBlock
construction. Global script, race, robot, recon, and marker initialization then
run in order.006F7B50 is a verified single RET. The mission Lua owner is reloaded
after these calls; a nonnull +4 instance receives `collectgarbage("collect")`
with arguments0,0,2.

The effect loop reloads00432650 for each count test, independently for indexed
validation, and again for the final +2D8 zero store. Count uses the original
32-bit pointer subtraction, arithmetic shift by3, and unsigned comparison.
A returning invalid-parameter handler can repair the captured owner; begin+10
is reloaded after it returns. Name acquisition receives a pointer to that
current eight-byte NativeString, with flag1. The vector append receives the
actual local effect reference. Cleanup decrements its real +4 reference word
atomically, dispatches the current zero-reference virtual slot when necessary,
then clears the local. Vector growth and effect acquisition remain required
native services; neither is a newly reconstructed library body.

The five allocations and publications are ordered:

| Native bytes | Constructor | Publication | Follow-up |
| --- | --- | --- | --- |
|58h|004A43C0|game+21D0|0049D690|
|38h|00452660|game+21E4|0044FA30|
|1C4h|008EDC60|00F88C30|008ECEC0|
|1B0h|0098A020|game+21E0|009870A0|
|7Ch|00445B10|game+21C4|00444D20|

Each publication precedes its follow-up call. The original does not guard the
follow-up against a null constructor result. The concrete existing allocation
service retries the CRT new handler or throws; it does not invent null success.
Panel and weather allocate their larger canonical host projections with the
native byte sizes retained in allocation requests. Untouched allocation words
are required explicit inputs, never inferred as zero. Previously published
owners are not destroyed if a later load fails. C++ scope cleanup for local
strings, effect references, and FileBlock is an adaptation; native SEH partial
construction and unwinding are not validated.

## Validation and follow-up packets

Validation results are recorded in `reports/global_subsystems.json`. This is
normal-flow reconstruction with required external services, not a runnable
game or gameplay validation. Existing warning code covers only table loading
inside009870A0; full warning constructor/init remains external here.

Independent next packets are traffic004A43C0/0049D690,
powerup008EDC60/008ECEC0, and warning0098A020/009870A0. The00432650 singleton
and00871BA0 effect-acquisition chain also need concrete bindings. Check current
leases before assigning them. The descriptive powerup role is provisional.

The combined Win32 build and both existing checks passed. Four focused fixtures
passed against the same library: full startup publication and reference cleanup,
panel owner lifetime, weather real-Lua loading, and generic Lua numeric conversions.
All28 names/evidence comments were saved and read back with prior comments intact;
affected exports were refreshed. The tested revision, library hash, logs and later
main integration result are retained in the report. No gameplay validation was run.

Integration with concurrent main work preserved the new GUI pre-property hooks.
The newly landed type-dispatch factory now also requires the same live CRT mode
alias and passes it to Screen and FrameBox readers. That reference must outlive
the factory and its types. The combined build and both checks passed after this
caller migration; see the report for the exact code revision and log.

Final integration tested revision `e1a5d542e18689f0586a275d56d8446a788d7bb2`
and fast-forwarded main to that same revision. Both existing tests passed; the
helper saved all28 reviewed names. Four focused fixtures had passed against
the earlier same-wave library recorded separately, before concurrent main
changes and the GUI caller migration. This evidence is not gameplay proof.

## Script and configuration integration

The next batch removes the five required whole-loader callbacks00886900,
00800160,00901610,00803A40 and006DBEB0. Their recovered implementations now
execute directly with canonical registry/publication slots and real Lua services.
`mission_lua_services_1a08` supplies only the service binding for the captured
mission owner; script-folder precedence and file/chunk execution use recovered
sequences. The marker loader independently resolves the current embedded game
Lua owner at+1A0C. Recon construction uses the mission instance's mutable state
slot and preserves per-category owner reloads. These two Lua owners must remain
distinct even when a fixture binds them to the same Lua state.

Race publication precedes its field reads and raw pointer overwrites do not free
old records. Robot configuration has nine actual descriptor types, six ordered
difficulty reads, concrete numeric readers and validators, unique-name registry
insertion and native alias-publication order. Marker duplicate insertion retains
the key/cell and overwrites the descriptor pointer without releasing the old one.
Recon construction ensures the0..2 / enemy-neutral-unknown-own /19-category table
hierarchy; it installs no numeric recon values.

See `GLOBAL_SCRIPT_FOLDERS.md`, `RACE_CONFIG.md`, `ROBOT_CONFIG.md`,
`RECON_VALUES.md` and `MARKER_CLASSES.md` for address evidence, original ABIs,
ownership and adapter boundaries. The report's `configuration_integration_g`
records this batch separately from the earlier validation above. Required
mission/resource service bindings, traffic/powerup/warning owners, native
STL/SEH/ABI and gameplay validation remain outstanding.

The configuration batch passed the combined Win32 build and both existing tests
at `eb86365727631c936b74f0e498a2c353a207b794`, then fast-forwarded main to
that same revision. Six focused fixtures passed against the earlier same-batch
library at `958ea08cc49292289937716edb194e79e569776b`; their exact logs and
separate library hashes are retained in the report. All74 annotations were saved
again by integration. Nine missing functions and25 race/marker/robot call gaps
were repaired; the documented00886370 stored-body limitation remains.

## Concrete traffic, powerup and warning owner integration

The six remaining constructor/loader callbacks for traffic004A43C0/0049D690,
powerup008EDC60/008ECEC0 and warnings0098A020/009870A0 now execute recovered
direct bodies. Typed canonical publication slots and explicit allocation-word
inputs preserve constructor writes and untouched fields. Allocation retains each
native requested size separately from the host projection size. Each owner is
published before its loader runs; powerup additionally publishes00F88C30 inside
its constructor before the parent repeats that store.

See `TRAFFIC_CONFIG.md`, `POWERUP_CONFIG.md` and `WARNING_OWNER.md` for complete
normal-flow evidence, original ABI, ownership and standard-library boundaries.
The warning initializer retains Lua references through effects/hooks and uses
the existing canonical warning tables. Configuration reload persistence and
native effect/texture reference ordering remain explicit. Host cleanup helpers
do not substitute for the unresolved full runtime destructors.

The combined Win32 Release build and both existing tests passed. Four focused
fixtures passed against that same library: startup publication and direct Lua
loading, traffic reload/sentinel behavior, powerup descriptors/resources, and
warning native source iterators/Lua lifetime/effect reentry. Three missing
functions were defined, one reachable cleanup gap repaired, and seven incorrect
inventory tags retired with prior state recorded. `owner_integration_h` in the
report separates this evidence from earlier batches and final main integration.

The00432650 configuration singleton and00871BA0 acquisition chain remain the
next concrete startup dependencies. Full runtime destruction, native ABI/SEH
compatibility and gameplay validation remain outstanding.

Final owner integration merged concurrent main15482729 before building, tested
`063506cb5adfb59a5bf1c2d0d993ae8070b2b3c8`, and fast-forwarded main to that exact revision.
The Win32 build and both existing tests passed; Ghidra saved all58 annotations.
Four focused fixtures passed on the earlier source60af126 library, with its
separate hash retained in the report. This remains build/fixture evidence,
not native ABI or gameplay validation.
