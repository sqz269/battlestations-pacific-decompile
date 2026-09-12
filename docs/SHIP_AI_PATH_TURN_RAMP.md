# Ship path-search turn penalty inputs

Addresses: 00424C40 0083D492 009EC280

Packet `orch6_ship_ai_path_turn_ramp`, worker `agent/orch6-zone-wrappers`.
Ghidra target `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, verified
by the repository client before each analysis/export batch. Ghidra stayed read
only. Descriptive names remain hypotheses. The new source is
`src/ship_ai_path_turn_ramp.cpp`; evidence is in
`reports/ship_ai_path_turn_ramp.json`.

The path search's zero-valued runtime ramp has a real producer. 00424C40 is
the gameplay settings singleton accessor, not a ramp calculation. On a miss,
it allocates76Ch bytes at00424C9F and invokes00424A10 at00424CB6; that
constructor invokes the Lua loader0083B5E0 at00424BFC. The constructor does
not initialize+6F0h/+6F4h/+6F8h before that loader. In particular, zero is not
a recovered default for those fields.

| Code | Inclusive coverage | Result |
| --- | --- | --- |
| 00424C40 | complete body read00424C40-00424CFF; no new singleton implementation | Existing accessor/ownership retained |
| 0083B5E0 fragment | partial projection, complete sequence0083D492-0083D575 | Three keyed number/default reads, immediate stores, temporary releases |
| 009EC280 | bounded correction009EC3BE-009EC43D within existing body009EC280-009EC67E | Plus-cost arm reads minus endpoint's position |

0083D492 is inside0083B5E0, not a separate Ghidra function. The loader spans
0083B5E0-00842951. The new fragment does not cover0083B5E0-0083D491 or
0083D576-00842951, including the script runner, table-owner assignments,
unrelated settings and native SEH. Existing gameplay-settings reconstruction
and docs own those paths. 00424C40 and009EC280 have zero live flow gaps;
the selected loader fragment is contiguous. No missing Ghidra function needs
definition.

## Producer and exact values

0083CB43 pushes `Navigator` (00D09958);0083CB57 looks it up from ShipGlobals
into the object at stack+120h. 0083D450 pushes `PathFinderParams` (00D0B02C),
0083D464 fetches it, and0083D479 assigns it into the reusable current-table
object. The new native-object entry accepts that actual selected object.
It calls the already reconstructed00B67800,00B66330 and00B67700 helpers.

| Store | Existing GameplayTuningSettings field | Key | Fallback address/bits | Value |
| --- | --- | --- | --- | --- |
| 0083D4C4 ->+6F0 | navigator_path_finder_params_length_modifier_dir_diff_min | LengthModifier_DirDiffMin | 00D05AA8 /3E860A92 | 0.2617993950843811 |
| 0083D510 ->+6F4 | navigator_path_finder_params_length_modifier_dir_diff_max | LengthModifier_DirDiffMax | 00CE3C64 /3FC90FDB | 1.5707963705062866 |
| 0083D55C ->+6F8 | navigator_path_finder_params_length_modifier_length_addon | LengthModifier_LengthAddon | 00CED724 /44BB8000 | 1500 |

The existing field declarations and offset assertions are reused. No new
settings overlay or tuning globals are created. The old broad semantic loader
uses rounded decimal fallbacks0.261799f and1.5708f; this fragment preserves the
actual binary32 words. It does not alter that unrelated loader's implementation.

00B66330 requires kind2 and exact Lua NUMBER type3, then rounds the Lua number
to float before returning in ST0. Numeric strings, booleans, nil and other
types take the fallback. The getter is ECX object, one stack float, RET4.
The name lookup is ECX table, output and key on stack, RET8; destruction takes
ECX object and plain RET. Each store occurs before its temporary is destroyed
and before the following key is looked up. There is no finite test, degree
conversion, endpoint-order repair or clamp in the producer.

The current installation's `scripts/datatables/shipglobals.lua` lines345-350
authors `DEG(15)`, `DEG(80)`, and1200. Its actual `luaMW_init.lua` defines DEG
as `a * math.pi / 180` at434. Executing both installed files in the fixture
produced bits3E860A92,3FB2B8C2,44960000. These installed values are evidence,
not compiled runtime constants. The runtime reader reads the already-loaded
ShipGlobals environment, including whatever real script variants populated it.

## Callable boundary

`load_ship_ai_path_turn_ramp_0083d492` consumes an actual
`NativeLuaObjectStorage` for PathFinderParams and writes the existing
`ShipAiPathSearchTurnRamp` in field order. `ship_ai_path_turn_ramp_from_settings`
is the corresponding value projection of an existing loaded settings snapshot.

`read_ship_ai_path_turn_ramp_lua` selects the real
ShipGlobals.Navigator.PathFinderParams through the native Lua helpers inside
`lua_pcall`, then runs the fragment. A temporary native tracking owner borrows
the same interpreter in this isolated C-call frame, where at most four tracked
positions are live. There is no second interpreter, script execution or DoFile
replacement. Successful lookups retain Lua metamethod behavior. The new adapter
restores the caller's stack top on success and failure.

Lua errors return false with text. Completed earlier field stores remain
visible, while later fields retain their old values, so the caller must reject
the failed load. A malformed/missing parent table is an error; it does not
silently create a table of defaults. The native exception/SEH ABI is not
reproduced. The protected callback has only trivially destructible locals;
Lua error unwinding skips no C++ resource destructor.

`GameMissionLuaHost::read_path_turn_ramp(out,error)` exposes this reader over
its existing private `state_`. No state returns false with an error and leaves
the output alone. The primary worker owns when the runtime loads/stores this
ramp and how a failed load affects planner execution.

## Consumer correction

009EC30B/009EC31E/009EC331 call00424C40 separately and immediately read+6F0,
+6F4,+6F8 at009EC310/323/336. The existing search host combines those three
field reads into one value. The first two are interpolation x endpoints; y0
is FLDZ, y1 is the third field. 00419010 receives five stack floats and RET14h.
The input x is the absolute wrapped bearing difference from the node's seed.

The plus-cost arm tests `[node+24h]` and that child's+45h at009EC3BE-009EC3C9.
It then executes `MOV EAX,[ESI+20h]` at009EC3CB, loads that minus child's
position at009EC3CE/009EC3E0, and writes the interpolated value into node+50h
at009EC43D. The older source used the plus child's position. The correction
changes only that argument to `turn_cost`, preserving the plus-child gate.
Both active cost arms therefore use the minus endpoint's bearing, even when
the children point in different directions. A null minus pointer in an active
plus-cost arm is native-invalid; no replacement bearing is invented.

## Verification

The standalone MSVC Win32 Release build and existing CTest checks are recorded
in the report. One ignored probe, compiled with `/W4 /WX /fp:strict` and
`/link /MANIFEST:EMBED`, passed7 checks. It executes the actual installed
luaMW_init.lua and shipglobals.lua, with the actual Lua file runner handling
ScriptOptions.lua in the fixture, and validates the authored values, stack
preservation, exact fallback bits, metamethod error ordering/partial stores,
missing-parent errors, and the plus-cost regression using distinct endpoints.
The fixture contains no game-host stand-in behavior for these reads; unrelated
search host methods throw if unexpectedly called.

No tracked tests were added. The fixture is installed-script and integration
evidence, not a native whole-loader/cost-pass differential test or gameplay
validation. The runtime host wrapper is compiled; the primary worker still
owns the executable path that exercises it.
