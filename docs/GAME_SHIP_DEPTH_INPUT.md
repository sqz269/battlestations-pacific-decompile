# Mission Lua ship depth input

`GameMissionLuaHost::read_ship_depth_input(type_id, session_mode, out, error)`
reads the actual selected class+570h scalar from its existing mission Lua state.
The result also identifies the selected settings record, scalar offset, and
static Lua key. It borrows the caller's actual `VehicleClass` index and session
mode; no descriptor, session, Lua state, or depth value is synthesized.

This is a **partial scalar projection** of settings loader 0083B5E0 and the
ship-leaf loaders, with the complete 00837DE0 record-selection rule reused from
`ship_tuning_block_offset`. It does not execute the entire singleton constructor,
both 28-integer records, other leaf effects, or the class+560h..56Ch array stores.
It is a new C++ interface, not an ABI-compatible native replacement. Runtime
construction and full 009E0270 pre-step integration belong to the integrator.

## Native producer and consumers

The inclusive settings-loader body is 0083B5E0..00842951. Its sole native caller
is the call at 00424BFC in 00424A10; ECX receives the allocated settings object.
The depth region starts at 00841B7B. The two loop passes set EDI to settings+80h
at 00841B81 or settings+F0h at 00841B8E, select `AvoidZoneDepthsSingle` or
`AvoidZoneDepthsMulti`, and write the actual integer records. The inspected
producer region is 00841B7B..008425A1; the report lists each selected scalar's
GetByName, GetByIndex(1), GetInteger, and final record store.

00837DE0..00837DFA takes the settings object in ECX, reads game singleton
00E188A8 then game+1FE4h, returns settings+80h when the mode equals zero, and
settings+F0h for every nonzero mode. It takes no stack arguments and uses plain
RET. All nine caller functions were read before interpreting this contract;
their selector call sites and adjacent register/result instructions are in the
report. Ship-leaf calls supply the result of singleton accessor 00424C40 in ECX.
The manager 00424D00 is the other consumer of the same selected native record.

The reader reuses `kShipLeafClasses`, `kShipLeafTuningSources`, and
`vehicle_class_kind_row`. The last performs the existing native factory's
case-insensitive Type lookup; spelling differences must not change the branch.
Offsets below are relative to the selected settings record, **not class fields**.

| VehicleClass.Type / flag | Lua class key | Scalar offset | Installed Single | Installed Multi |
| --- | --- | --- | --- | --- |
| MotherShip | MotherShip | 00h | 0 | 3 |
| Destroyer | Destroyer | 08h | 0 | 1 |
| TorpedoBoat | TBoat | 10h | 0 | 1 |
| LandingShip / BigLandingShip false | SmallLandingShip | 18h | 0 | 1 |
| LandingShip / BigLandingShip true | LargeLandingShip | 20h | 0 | 3 |
| BattleShip | BattleShip | 28h | 0 | 3 |
| Cargo | CargoShip | 30h | 0 | 3 |
| Cruiser / HeavyCruiser false | LightCruiser | 38h | 0 | 3 |
| Cruiser / HeavyCruiser true | HeavyCruiser | 40h | 0 | 3 |
| Submarine | Submarine | 5Ch | 0 | 1 |

Cruiser body 006FB550..006FB655 reads `HeavyCruiser` with false default at
006FB582/006FB593. Its selected scalar reaches class+570h at 006FB5EF (40h)
or 006FB641 (38h). LandingShip body 0074C630..0074CC46 reads `BigLandingShip`
at 0074C66A/0074C67B and stores class+570h at 0074CA9D (20h) or 0074CADB
(18h). Both use ECX=descriptor, one Lua-object stack argument, and RET 4.
The flags use exact Boolean-type default behavior, not Lua truthiness.
The other six ship leaf types reuse the already reconstructed source table.
The producer defines `MiniSub` at 48h, but no established leaf scalar selects
that slot; this packet introduces no MiniSub type or class mapping.

All four leased entries already existed as Ghidra functions. No definitions,
renames, or other Ghidra mutations were performed. Inclusive unrepresented
listing gaps in 0083B5E0 are 0083BD5D..0083BD5F, 0083BE15..0083BE1F,
00840474..00840507, and 0084139A..0084139F. The LandingShip listing has
0074C865..0074C86F. Each follows a jump and lies outside this projection;
none is a false no-return call gap. The selector and Cruiser bodies have none.

## Lua behavior and lifetime

`read_ship_depth_input_lua` is the same callable borrowed-state reader used by
the member wrapper. It executes through a protected Lua C frame, uses the
existing native Lua object/state storage helpers, restores the caller's stack,
and leaves output unchanged on failure. The C frame contains only trivial
automatic objects, so Lua errors do not cross C++ destructors. The temporary
native tracking owner stays within its existing 50-slot/5-reference capacities;
it never owns or closes the mission state.

GetByName 00B67800 and GetByIndex 00B67720 use actual table access, including
metamethods and their errors; native cleanup is RET 8 for each. The final scalar
uses bare GetInteger 00B66290: lua_tonumber at 00B6629B, float32 spill at
00B662A0, reload, and tail transfer at 00B662A9 to CRT conversion 00BF7420.
Numeric strings therefore work, and final nil/nonnumeric values yield the
native zero conversion result. That is recovered behavior, not a fallback for
an unavailable ship descriptor or missing parent table. Missing parent/class
arrays raise the actual Lua indexing error; unsupported Types and unavailable
mission state return false with an error. Non-string Lua errors are handled.

The borrowed helper takes the existing CRT SSE2 conversion-mode projection
explicitly. The public member obtains current SSE2 capability through
`IsProcessorFeaturePresent`, matching the existing decal loader's represented
process capability binding; this is not a read of the original process global
0109EEA4. The result preserves the native signed integer's uint32 bit pattern.
`class_key` points to a static literal and survives subsequent Lua activity.

## Domain, adjacent Width binding, and verification

Supported Types are the eight established ship descendants (kind IDs 7..14).
The installed-data probe accepted 160 ship rows and rejected 473 other rows,
including AirField (2), LandFort (368), CommandBuilding (7), Shipyard (2), and
aircraft/land/dummy types listed in the report. These are legitimate non-ship
descriptors, not zero-depth ships. The existing factory descriptor sizes for
AirField 140h, LandFort 180h, CommandBuilding 1ACh, and Shipyard 138h do not
even contain offset 570h. Runtime must use actual Type/class-row availability
to scope ship navigation. The native ship-brain virtual allocation domain is
separate integrator evidence; this adapter does not claim to reconstruct it.

By coordination with the hull-extents worker, `GameVehicleClassRow` also gains
`width` and the existing row reader obtains actual `Width`. Native 00960230
uses GetByName at 00960354, bare GetNumber at 00960363, FSTP class+A4h at
00960368, and destruction at 0096037A. The binding reuses
`lua_number_float32_00b66270(lua_tonumber(...))`, including numeric strings and
native zero conversion for an absent final value. The hull-extents worker owns
00960230 annotation and downstream width geometry; this packet only changes
the jointly coordinated Lua files. Width was build-tested, not probed here.

Win32 build and existing CTest checks passed **2/2** after seed verification.
One ignored `/MANIFEST:EMBED` probe compiled an exact excerpt of this reader
against the existing Lua 5.1.1 and reconstructed core. It loaded actual installed
`ShipGlobals.lua`, `vehicleclasses.lua`, fundamentals/unlocks, ShipGlobals'
ScriptOptions dependency, and the actual DEG/KMH/MPH/KTS/isUnlock definitions
from luamw_init.lua. PC=true matches the existing native platform bootstrap.
Its concrete DoFile reads installed physical files; this is not a VFS test.
All 160 supported rows were checked with session 0 and -1, with stack balance
and the ten keys above. A compact boundary fixture covered numeric-string
float32 rounding, case-insensitive Type, non-Boolean flag default, nil scalar,
missing parent, and non-string Lua errors; failures=0.

The fixture exercises the exact borrowed reader, not the whole mission host or
a running mission. The member forwarder and Width binding were compiled in
`bsp_game`. There are no new tracked tests. Local probe sources/logs, input
hashes, report call checks, and coverage details are recorded in
`reports/game_ship_depth_input.json`. Full settings/descriptor storage,
concurrent Lua mutation, and runtime pre-step execution are outside this packet.
