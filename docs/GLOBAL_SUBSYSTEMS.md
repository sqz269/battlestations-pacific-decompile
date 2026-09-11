# Global subsystem construction

Addresses: 004DC6A0. The name is a hypothesis, not a recovered symbol.

`construct_global_subsystems_004dc6a0` replaces the whole-phase callback in
`MissionSceneLoadHost`. Mission loading now supplies current publication slots,
individual native services, and the existing Lua runtime, then calls this body.
It directly constructs and loads the canonical panel and weather owners.

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

Independent next packets are the race loader00800160, robot loader00901610,
marker loader006DBEB0, recon registration00803A40, traffic004A43C0/0049D690,
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
