# Native Lua object boolean setter

Address: `00B673A0..00B673F0` (81 bytes, complete).

`native_lua_set_boolean_00b673a0` is the canonical boolean field setter for
the actual `NativeLuaObjectStorage`. It uses the repository's existing
`NativeLuaStateStorage`, eight-byte `NativeString`, and linked Lua 5.1.1 API.
`BSP_LuaObject_SetBoolean` is a descriptive hypothesis, not a recovered
symbol. The original ABI is `__thiscall`: ECX is the 14h Lua object, the stack
holds a `NativeString*` and a four-byte value slot, and the function returns
with `RET 8`. The C++ function is an explicit source interface rather than a
drop-in binary replacement.

## Complete behavior

The body calls `lua_checkstack(current state, 2)` and ignores its result. It
then reads the key's DWORD length followed by its data pointer. A null pointer
selects the native empty byte at `0108FF2C`; it does not change the length.
The source uses an ordinary static empty byte for the same library input and
preserves the stored length.

The function reloads `object.owner_00` and `owner.state_04` before every Lua
operation. It calls `lua_pushlstring`, zero-extends only the low byte of the
four-byte value slot, calls `lua_pushboolean`, then reloads the owner, the
object's current `index_08`, and that owner's state before `lua_settable`.
Consequently `0x00000100` is false and `0x00000101` is true. Any nonzero low
byte becomes Lua true.

On a normal path the two pushes and `lua_settable` have zero net stack effect;
`lua_settable` consumes the temporary key and value. The setter itself does
not alter object ownership, kind, index, tracked-slot metadata, or the native
string. It has no kind gate, owner/state validation, rollback, cleanup guard,
or early return when stack growth fails. Lua errors and metamethod effects
continue through the real Lua API.

| Call site | Lua 5.1.1 operation | Original register/stack ABI |
| --- | --- | --- |
| `00B673AD` | `00A672F0 lua_checkstack` | ECX state, EDX 2 |
| `00B673CC` | `00A67A10 lua_pushlstring` | ECX state, EDX data, stacked length |
| `00B673DB` | `00A67BC0 lua_pushboolean` | ECX state, EDX zero-extended low byte |
| `00B673E8` | `00A67E10 lua_settable` | ECX state, EDX current object index |

## Incoming calls

The live xref list has 17 direct calls. Sixteen are contained by defined
functions and are checked by `tools/verify_report_calls.py`. `0074B2B1` is in
an undefined listing region: `get_function_by_address` names `0074AF20` only
as a candidate, while that defined function ends at `0074AF42`. Raw bytes at
`0074B2A0` independently decode the call and its arguments.

| Call | Containing function | Key | Low-byte value source |
| --- | --- | --- | --- |
| `004368B2` | `00436730 FUN_00436730` | dynamic source field name | result of `00B66250` |
| `00436E61` | `00436C50 BSP_ProfileManager_CopyLuaValueAcrossStates` | dynamic source field name | result of `00B66250` |
| `0044316C` | `00443090 BSP_DeviceClass_ResolveFromLua` | `Got` | `1` |
| `006B0120` | `006B0020 FUN_006B0020` | `_R_` | `1` |
| `006B0C92` | `006B0BE0 FUN_006B0BE0` | `_R_` | `1` |
| `006EAA24` | `006EA910 BSP_BulletClass_GetOrCreate` | `Got` | `1` |
| `007012F2` | `00701250 FUN_00701250` | `HitOnTheFly` | `1` |
| `0074AE33` | `0074AD90 FUN_0074AD90` | `LandingFinished` | `1` |
| `0074B2B1` | no defined function | `LandingStarted` | `1` |
| `007CEFF3` | `007CE040 BSP_PlaneTickElement_FixedStep` | `Collided` | `1` |
| `008090C9` | `00808F90 BSP_SensorClassTable_Resolve` | `Got` | `1` |
| `008BD125` | `008BCF10 FUN_008BCF10` | `ai` | player record byte `+09h` |
| `00928BC7` | `00928A00 BSP_MissionEntity_CreateLuaSelfTable` | `Dead` | `0` |
| `00929896` | `00929800 FUN_00929800` | `Dead` | `1` |
| `00964959` | `00964790 BSP_VehicleClass_GetOrCreate` | `Got` | `1` |
| `00A2F0DA` | `00A2EEE0 FUN_00A2EEE0` (`AIGetGroupInfo` role) | `autoGrouping` | group byte `+5648h` |
| `00BD73F2` | `00BD7180 FUN_00BD7180` | caller-supplied native string | caller-supplied value slot |

The class factories therefore use this same primitive for their `Got=true`
write-back. In particular, the device factory writes `Got` before type
dispatch, as established in `NATIVE_DEVICE_CLASS_RESOLUTION_ORCH4.md`.

## Evidence and limits

The live project was `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with 64,124 functions. The exact 81 bytes hash
to SHA-256 `3cf2cdcbdf92594e9af9ed9fb4e9ca16ef7460c8dc3df827fd10708a44d38df5`.
Ghidra was read-only. The source is build-tested and report-call-verified; it
is not a native ABI replacement or gameplay validation. Invalid pointers,
invalid table indices, Lua longjmp behavior, and callbacks that deliberately
move the object across interpreters remain native caller domains rather than
newly guarded source behavior.
