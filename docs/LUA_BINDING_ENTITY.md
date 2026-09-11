# The entity return convention of the mission Lua bindings

Addresses: 00898e30 00898750 008990b0 00899af0 00892860 00896750 0089c360 0089cfe0 0089d250 0089d470 008a6de0 008a9460 008ab070 008c2c50 008c3610 008c3880 008cefc0 008cf350 00944fd0 008a9730 0088b1b0 0088b840 00888aa0 00b679b0 00b67910 00b678e0 00b67800 00b67720 00b663d0 00b66400 00b66430 00b65eb0 004260b0

Packet `cc_mission_natives`, worktree `agent/cc-mission-natives`. Ghidra was read-only. Every
name here is a hypothesis, not a recovered symbol.

What a binding hands back when it returns a game entity, and how a later binding reads it. The
self table itself is `docs/MISSION_LUA_SELF_TABLE.md`; the reconstruction is
`include/bsp/mission_lua_bindings.hpp`.

## Corrections

**`Effect` is not an entity-returning binding.** The `lua_binding_find_entity` follow-up row in
`docs/MISSION_LUA_MACHINE.md` names "the `FindEntity` and `Effect` rows". `008A9730` never
pushes `00CE7494`, never calls `00B67910` and never reaches `00B663D0`. Its second argument is a
table it scans with `00B67080`/`00B66420` through `0088B840`, and its entity-valued arguments
go the **other** way, through `008889C0` and `00888AA0`, which read `Ptr` off a table the script
already holds. Previous value recorded before this correction: "the `FindEntity` and `Effect`
rows of 00e0b7b8 | ... The return convention of the entity-returning bindings, which is what
the six remaining probe failures need."

## The tail

Nineteen of the 560 rows end in the same nine calls. `00898E30 FindEntity` is the reference
reading, `00898F98..00899045`; `00898750 CreateScript` repeats it verbatim from `008989C7`.

```
id   = *(u16*)(entity + 0x174)              // 00898F98  MOVZX EAX, word ptr [EAX + 0x174]
key  = NativeString::FromInt(id)            // 00898FA7  004260B0, decimal text
tbl  = frame["thisTable"]                   // 00898FC4  00B67910 -> 00B67800
val  = tbl[key]                             // 00898FDA  00B678E0 -> 00B67800
val.push()                                  // 00898FE9  00B663D0
return lua_gettop(L) - frame.base           // 00899045  00B66400
```

and on the not-found branch, reached when the lookup returned a null pointer
(`00898F90 CMP EAX,EBP` with `EBP` zero, `00898F92 JZ 0x00899038`):

```
frame.push_nil()                            // 0089903C  00B66430 -> lua_pushnil
return lua_gettop(L) - frame.base           // 00899045  still one value
```

So the result is **a plain Lua table taken out of the global `thisTable`, or nil**. No userdata
is pushed for the entity, no metatable is set, and nothing is copied: `00B663D0` is
`lua_checkstack(L, 1)` then `lua_pushvalue(L, slot)`, a second reference to the stored table.

`00B66400` makes the returned count `lua_gettop(L) - this->base`, where `base` is the
`lua_gettop` the frame object cached at construction. For this tail that is always exactly one,
because the handler pushes exactly one value above its arguments.

## The frame object the tail indexes

Every binding opens with `00B66C00` (a `LuaStateOwner` over the live state, which also installs
`DoFile` through `00A67B20 lua_pushcclosure` with `00B69E00`) and then `00B679B0`:

| Offset | Written by `00B679B0` | Meaning |
| --- | --- | --- |
| +00 | `ECX`, the owner | the owner, whose +04 is the `lua_State*` |
| +04 | 3 | a kind `docs/LUA_OBJECT_API.md` does not list |
| +08 | 1 | the first argument's stack slot |
| +0C | `lua_gettop(L)` | the entry top, that is, the argument count |
| +10 | 0 | the flag byte |

Kind 3 changes what a **name** lookup means. `00B67800` branches on it at `00B67804
CMP dword ptr [ESI + 0x4],0x3`: kind 3 takes the `00B6784B` path and ends at `00B67876
MOV EDX,0xFFFFD8EE`, which is `LUA_GLOBALSINDEX` (-10002); every other kind uses `this->slot`
from `00B67846`. That is why `frame["thisTable"]` in the tail is the **global** table and not a
field of anything.

An **index** lookup means something else again. `00B67720` on a non-kind-2 object
(`00B677B7..00B677D4`) produces a kind-2 object at `slot = this->slot + index`, so with the
frame's slot of 1, index 0 is the first Lua argument. `00B677E0` is the wrapper the bindings
call; `00B65EB0` is `lua_gettop` on the owner and is how a binding learns its argument count.

The two name wrappers differ only in where the name comes from: `00B67910` passes a literal
`const char*` straight through, `00B678E0` takes a NativeString and substitutes the empty string
at `0108FF2C` when its data pointer is null.

## The nineteen bindings

Determined mechanically: every row of `00E0B7B8` whose handler pushes `00CE7494` into
`00B67910` and then reaches `00B663D0`. Ordered by handler address.

| Handler | Binding | Handler | Binding |
| --- | --- | --- | --- |
| `00892860` | `GetLastCatapulted` | `008A6DE0` | `UnitGetAttackTarget` |
| `00896750` | `LaunchAirBaseSlot` | `008A9460` | `GetPrimaryTarget` |
| `00898750` | `CreateScript` | `008AB070` | `GetSelectedUnit` |
| `00898E30` | `FindEntity` | `008C2C50` | `GetTargetInfoTarget` |
| `008990B0` | `FindEntityByID` | `008C3610` | `GetDevice` |
| `00899AF0` | `GetFormationLeader` | `008C3880` | `GetKamikazeByDummy` |
| `0089C360` | `GetFireTarget` | `008CEFC0` | `GetPayload` |
| `0089CFE0` | `GetSquadronPlane` | `008CF350` | `GetGun` |
| `0089D250` | `GetPlaneSquadron` | `00944FD0` | `GenerateObject` |
| `0089D470` | `GetSquadronLandedBase` | | |

Three further functions share the tail but are not binding rows: `00885DA0
BSP_MissionLua_PushArgumentRecord` (case 3), `00887750 BSP_MissionLuaHost_CallNamed` and
`00887E50 BSP_MissionLuaHost_CallNamedThreadSafe`, which use it to push the call's `this`.

Five of the nineteen are reached at load time by at least one installed script; see
`docs/LUA_BINDING_TABLE.md` for the counts.

## How `FindEntity` finds the entity

`0088B1B0`, sole caller `00898E30`, takes a NativeString **by value** as two stack dwords: the
caller makes room with `00898F53 SUB ESP,0x8` and builds the string in place, and the callee
reads the data pointer at `0088B1CA MOV EDI,dword ptr [ESP + 0x28]`. It walks the bucket lists
at `[[00E188A8]+19CCh]+1Ch+n` for a hard-coded set of kind indices (the jump table at
`0088B310` over `-6..0x47`), skips entries whose four flag bytes at `+5C..+5E` and `piVar4[0x17]`
disagree, and compares each candidate's `(*vtable + 0x10)()` name against the argument. A null
argument matches a null name. The entity pointer comes back in `EAX`.

The lookup is by **name**, and the name comes from Lua argument 0 read as a string through
`00B662B0`, which returns whatever `lua_tolstring` gives and therefore yields nothing useful for
a non-string argument rather than raising.

## How a later binding reads the entity back

Through the table, not through the id. `00888AA0 BSP_ObjectHandle_FromLuaTable` is already
named and already reconstructed (`object_from_lua_table_00888aa0` in
`src/object_handle_resolvers.cpp`): it does `00B67800(table, out, "Ptr")` then `00B662D0
BSP_LuaObject_ToUserdata`, and it has **278 callers**. `008889C0` is the guarded variant, adding
`00B661B0 BSP_LuaObject_IsTable` and `00B66160` before the same two calls; `Effect` uses it.

So the round trip is: a binding pushes `thisTable[key]`, the script passes that table back, and
the receiving binding reads its `Ptr` light userdata. The `ID` string is the key, not the
handle, and nothing in the path this packet read converts `ID` back to an entity.

## Leases not taken

`00B67800 BSP_LuaObject_GetByName` is the routine the kind-3 branch lives in and is central to
this reading, but it is held by `agent/orch2-20260910:orch2_native_shader_lua_state_reader_ah`.
It was read, not annotated, and no ledger record for it was written. Its kind-3 behaviour is
recorded here and on `00B679B0`, which this packet does hold.

## Uncertainties

- Whether a row can be entity-returning without the literal `00CE7494` in its own body -- for
  example by calling a shared helper -- was not ruled out. The nineteen are those whose own
  bodies carry it.
- `00B67720`'s kind-2 branch (`00B67735..00B67772`, `lua_pushnumber` then `lua_gettable`) is a
  real table index and is not modelled; no binding read here reaches it on a frame object.
- The set of kind indices `0088B1B0` searches was not decoded past the jump table; which entity
  classes are and are not findable by name is therefore open.
- `00944FD0 GenerateObject` is listed by the tail rule alone. Its body was not read, so whether
  it creates the entity it returns is not established.
- The `00898F0E PUSH EBP` argument index in `FindEntity` is zero because `EBP` is the function's
  zero register (`00898F90 CMP EAX,EBP` is a null test, `00898F62 MOV [ESI],EBP` zeroes a
  NativeString). No second-argument path was read.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `lua_binding_entity_lookup` | 0088b1b0 00e188a8 0088b310 | docs/LUA_BINDING_ENTITY_LOOKUP.md | The kind-index jump table and the four flag bytes that gate a name match |
| `lua_binding_effect` | 008a9730 0088b840 008889c0 00888760 | docs/LUA_BINDING_EFFECT.md | `Effect`'s parameter table and its four argument converters |
| `lua_binding_generate_object` | 00944fd0 00944e0a | docs/LUA_BINDING_GENERATE_OBJECT.md | Whether `GenerateObject` creates the entity whose table it returns |

## no_ghidra_function

none for the addresses above: all thirty-three have a Ghidra function, checked with
`python tools/bsp.py ghidra proto <addr> --brief`.

Twenty-four **handlers elsewhere in the table** have none; they are listed in
`docs/LUA_BINDING_TABLE.md`.
