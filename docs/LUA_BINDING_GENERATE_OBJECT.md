# `GenerateObject` (`00944FD0`), the binding that creates an entity

Addresses: 00944fd0 00944e0a 004c6ba0 004c6be0 0046d930 0046dc10 00888760 0088b840 00874d00 0046aab0

Packet `cc2_entity_lookup`, worktree `agent/cc2-entity-lookup`. Ghidra was read-only for this
packet; every descriptive name below is a hypothesis, not a recovered symbol. It answers the
`lua_binding_generate_object` follow-up of `docs/LUA_BINDING_ENTITY.md`, whose open item was
"its body was not read, so whether it creates the entity it returns is not established".

Reconstruction: `include/bsp/lua_binding_entity_lookup.hpp`, `src/lua_binding_entity_lookup.cpp`.

## Correction: `00944E0A` is not in `00944FD0`

The packet brief placed the call site `00944E0A` inside `GenerateObject`. It is not.
`python tools/bsp.py ghidra proto 00944e0a --brief` reports `BSP_LuaBinding_Spawn`, body
`00944680-00944FC0`; `GenerateObject`'s body is `00944FD0-0094544F`. `00944680` is leased to
`agent/cc-lua-core`, so `00944E0A` is that packet's, was not read here, and no claim in this
document rests on it. The creation site this document establishes is `00945308` (and `009452CA`),
both inside `00944FD0`.

## Yes, it creates the entity

`GenerateObject` reaches one of two thin forwarders on the game object, each of which calls the
**scene database** `[00E18680]`:

| forwarder | ABI | target | argument order |
| --- | --- | --- | --- |
| `004C6BA0` | `__thiscall(game)`, `RET 0Ch` | `0046D930` | `(name, secondName, 0)` |
| `004C6BE0` | `__thiscall(game)`, `RET 10h` | `0046DC10` | `(name, secondName, Vector3*, headingFloat)` |

Both then call `004C3840 BSP_Game_AssignPartyPlayerSlots(0)` when `game+1FE4h` is non-zero and
return the created instance unchanged in `EAX`.

`0046D930` is the creation path, read in the listing:

1. `0046D95B` assigns its first argument into a NativeString and `0046D96F` looks it up in the
   map at `sceneDb+18h` through `00468CD0`. A name that is not in that map returns **0** at
   `0046D9AF` with nothing created.
2. `0046D9DC` takes the found record's class token (`[record+14h]`, then `+0Ch`) and `0046D9FE`
   resolves it in the class map at `sceneDb+34h` through `00468FB0 BSP_SceneClassMap_FindNode` —
   the registry of `docs/SCENE_ENTITY_FACTORY.md`.
3. `0046DB1D` turns the descriptor's class id into its name with `00468660`, and `0046DB27` asks
   `0046C550 BSP_SceneEntity_ShouldGenerate`. A zero answer returns 0 with nothing created.
4. `0046DB4B CALL EAX` with `EAX = [descriptor+4h]` runs the descriptor's **instantiate-pass
   creator**, the same `descriptor[1]` the `.scn` reader uses. `0046DBAB CALL EAX` runs the
   registration-pass creator `[descriptor+8h]` on the second branch.
5. `0046DBE8` finishes with `00925F20 BSP_SEntity_InitAll` and returns the instance in `EAX`.

So `GenerateObject("X")` instantiates the authored scene object named `X` out of the scene
database's named-object map; it is not a class-name spawner and it cannot create an object the
scene files did not declare.

`0046DC10` is the same routine with a placement: `0046DD48` copies the caller's three floats into
the `+30h`/`+34h`/`+38h` translation slots of the local frame it passes to `0046C550`, and
`0046DD4C..0046DD9F` compares the fourth argument against the double `2*pi` at `00CE3828` and,
only when it is **smaller**, applies `00467050(frame, 0.0f, value, 0.0f)`. The fourth argument is
therefore a yaw in radians with a sentinel: the default `10.0f` at `00CE38B8` means "keep the
authored orientation".

## The Lua arguments

Index 0 is the first Lua argument (`docs/LUA_OBJECT_API.md`; the frame's base is 1). `00B663F0`
on the frame returns the argument count, and index `i` exists only while `i < count`.

| index | type | role | evidence |
| --- | --- | --- | --- |
| 0 | string | the authored object's name, the key into `sceneDb+18h` | `009450A0 00B662B0`, then `0046D96F` |
| 1 | string | a second name, forwarded as the creators' second dword | `009450E6 00B660A0`, `00945136` |
| 1 or 2 | table of three numbers | the world position | `009451B7 0088B840`, `009452BC 00888760` |
| 2 or 3 | number | the yaw, radians | `00945248`, `00945265 00B66270` |

When argument 1 is not a string, `009451C4` **copies argument 0's string** into the second slot,
so the second dword is never null. The two tests on argument 1 are mutually exclusive: a table is
not a string.

The shape decision, `0094518A..00945229`:

- `count > 1` and argument 1 is a three-number table: placed, position at index 1.
- otherwise `count > 2`: placed, position at index 2.
- otherwise: the `004C6BA0` form with no position and no yaw.

`0088B840` is the position-table predicate: `0088B864 00B661B0 BSP_LuaObject_IsTable`, then an
iteration whose tail `0088B974 CMP EDI,3` requires exactly three entries. `00888760` is the
reader: `__fastcall(ECX = out Vector3, EDX = LuaObject)` plus **one float stack argument it never
pops** (`RET 0`), so the float stays on the stack and becomes `004C6BE0`'s fourth dword. It writes
`[out]`, `[out+4]`, `[out+8]` at `0088884E..00888867` and returns `out`.

## Host table

One row per native call site of `00944FD0` outside the shared LuaObject plumbing.

| site | callee | name | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `00945076` | `00B679B0` | open call frame, kind 3, slot base 1 | frame | frame | always |
| `009450A0` | `00B662B0` | `BSP_LuaObject_GetString`, argument 0 | object | `char*` | always |
| `009450C0` | `00B663F0` | argument count | frame | count | always |
| `009450E6` | `00B660A0` | `BSP_LuaObject_IsString`, argument 1 | object | bool | `count > 1` |
| `00945136` | `00B662B0` | `BSP_LuaObject_GetString`, argument 1 | object | `char*` | argument 1 is a string |
| `00945159` | `0041DD40` | `BSP_NativeString_Resize`, then `00BF7680 memcpy` | string, len, fill | — | always |
| `009451B7` | `0088B840` | position-table predicate | object | bool | `count > 1` |
| `00945265` | `00B66270` | `BSP_LuaObject_GetNumber`, the yaw | object | float | a slot exists after the table |
| `009452BC` | `00888760` | read three floats into a Vector3 | out, object, yaw | out | placed form |
| `009452CA` | `004C6BE0` | create with position and yaw | game; name, second, Vector3, yaw | entity | placed form |
| `00945308` | `004C6BA0` | create without position | game; name, second, 0 | entity | otherwise |
| `00945311` | `00874D00` | pump deferred work, `CL = 1` | flag | — | always |
| `00945316` | — | `MOVZX EAX, word ptr [entity+174h]` | — | — | always, **unguarded** |
| `00945325` | `004260B0` | `BSP_NativeString_FromInt`, the decimal key | string, id | string | always |
| `00945342` | `00B67910` | `BSP_LuaObject_GetByLiteral`, `"thisTable"` at `00CE7494` | frame, out, literal | object | always |
| `00945358` | `00B678E0` | `BSP_LuaObject_GetByNativeString` | table, out, key | object | always |
| `00945367` | `00B663D0` | `BSP_LuaObject_PushValue` | object | — | always |
| `009453BB` | `0046AAB0` | `BSP_SceneDatabase_ResolveDeferredReferences` | sceneDb | — | always |

`00874D00` is the deferred-work pump shared with `BSP_LuaBinding_Spawn` and
`BSP_LuaBinding_LaunchAirBaseSlot`: `00925F20 BSP_SEntity_InitAll`,
`0077EC20 BSP_Replication_ApplyPendingEntityCreates`, `00903610 BSP_World_ReleaseExpiredObjects`,
`00926700`, `009273A0`, `00929460`, `00874C90`, `00888230`, `0076FFC0`, `00778450`. It runs
between the creation and the table read, which is why the fresh entity already has its Lua self
table (`00928A00`, packet `cc2_entity_lua_attach`) by the time `thisTable` is indexed.

## What it pushes back

The same nine-call tail as `00898E30 BSP_LuaBinding_FindEntity`: `thisTable[decimal(u16 at
entity+174h)]`, then `lua_gettop - base` as the result count. Unlike `FindEntity`, there is **no
nil branch**: `00945316` dereferences the creator's return value with no test, on either path. A
name that is absent from `sceneDb+18h`, or an entity `0046C550` refuses to generate, returns 0
from `0046D930` and faults here. No shipped script was checked against that, so it is a static
reading of a reachable path, not an observed crash.

## Coverage

| address | coverage |
| --- | --- |
| `00944FD0` | complete for the argument decode, both creation calls and the tail, `00945076..009453BB`; the one-time `"luakod"` string init at `00944FDA..00945076` and the SEH unwind funclets are not modelled |
| `004C6BA0`, `004C6BE0` | complete, both are under 0x40 bytes; leased to `agent/cc2-scene-entity-create`, named here only as external contracts |
| `0046D930` | partial: the lookup, the class resolution, the creator call and the return were read; the property-bag and party handling between `0046DA33` and `0046DB02` was not |
| `0046DC10` | partial: the ABI, the position copy and the yaw gate were read; the rest was not |
| `00888760`, `0088B840` | complete for the contract: the predicate's count test and the reader's three stores and `RET 0` |
| `00874D00`, `0046AAB0` | callee lists only, contracts taken from their own callees' ledger names |

## Open questions

- The second string reaches `0046D930` as its second stack argument and **nothing in that body
  reads it**; only the first (the lookup key) and the third (`0046C550`'s `entityName` slot) are
  used. Whether `0046DC10` reads it was not established: its decompilation drops the argument and
  its listing was only read as far as the position and yaw. The Lua-side meaning of the second
  name is therefore open.
- `0046C550`'s `a2` (`entityName`) receives `0046D930`'s **third** argument, which `GenerateObject`
  passes as 0 on the unplaced path, so a generated object keeps the authored name.
- Which shipped scripts call `GenerateObject`, and with how many arguments, was not surveyed.
