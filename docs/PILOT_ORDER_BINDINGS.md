# The pilot order Lua bindings

Packet `cc7_pilot_order_bindings`. Read-only analysis of `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Five mission-Lua bindings that end at
`0077D600 BSP_Entity_IssueCommand`:

| address | binding | body | `0077D600` at | command class |
| --- | --- | --- | --- | --- |
| `008A4C90` | `PilotSetTarget` | `008A4C90`-`008A4EFB` | `008A4EAC` | whatever `007EEC50` chooses |
| `008A4150` | `PilotMoveTo` | `008A4150`-`008A42F2` | `008A42A7` | `moveto` `00E08F68` |
| `008A4590` | `PilotMoveToRange` | `008A4590`-`008A47A5` | `008A472A` | `moveto` `00E08F68` |
| `008A4300` | `PilotRetreat` | `008A4300`-`008A4581` | `008A4532` | `retreat` `00E08F90` |
| `008A47B0` | `PilotLand` | `008A47B0`-`008A4952` | `008A4907` | `land` `00E08FA0` |

`src/pilot_order_bindings.cpp` and `include/bsp/pilot_order_bindings.hpp` carry the
reconstruction. Descriptive C++ names there are hypotheses; the five **binding names
are recovered symbols**, see below.

## The names are recovered, not guessed

Each of the five addresses has exactly one reference in the whole image, and it is a
data reference from the registration table at `00E0BC90` (`ghidra xrefs`, one row each:
`00E0BCCC`, `00E0BC94`, `00E0BC9C`, `00E0BCAC`, `00E0BCB4`). The table is an array of
8-byte `{const char* name, lua_CFunction fn}` pairs:

```
00e0bc90  8c 06 d1 00 50 41 8a 00   "PilotMoveTo"        -> 008A4150
00e0bc98  78 06 d1 00 90 45 8a 00   "PilotMoveToRange"   -> 008A4590
00e0bca0  68 06 d1 00 70 3e 8a 00   "PilotMoveOnPath"    -> 008A3E70
00e0bca8  58 06 d1 00 00 43 8a 00   "PilotRetreat"       -> 008A4300
00e0bcb0  4c 06 d1 00 b0 47 8a 00   "PilotLand"          -> 008A47B0
00e0bcb8  38 06 d1 00 60 49 8a 00   -> 008A4960
00e0bcc0  20 06 d1 00 10 4b 8a 00   -> 008A4B10
00e0bcc8  10 06 d1 00 90 4c 8a 00   "PilotSetTarget"     -> 008A4C90
```

with the literals read at `00D1064C`-`00D10697`. The order and the addresses agree
with `src/mission_lua_host.cpp:171-178` exactly, so that file's mapping is confirmed
against the binary: the two previously unnamed entries are **`PilotRetreat`**
(`008A4300`) and **`PilotLand`** (`008A47B0`).

A second, independent confirmation: each body opens by building the pooled literal
`"luaMW_<Name> failed:"` (`0041DD40` reserve, `00BF7680` memcpy, then the pool round
trip `00419CC0`/`00BD1510` that frees it again without reading it). `008A4CB5`
reserves `1Ch` = 28 = `strlen("luaMW_PilotSetTarget failed:")`, `008A4175` reserves
`19h` = 25 = `strlen("luaMW_PilotMoveTo failed:")`.

## Original ABI, shared by all five

`__fastcall(ECX = script context)`, `RET 0` (`ADD ESP,<frame>` then `RET`), returns the
Lua result count in `EAX`. Ghidra's `undefined4 __fastcall FUN_008a4c90(undefined4)` is
right about the register but not about the callee cleanup: there are no stack arguments.

The frames are large because a whole `LuaStateOwner` lives on the stack. Offsets below
are relative to `ESP0`, the value of `ESP` after the prologue (`SUB ESP,n` plus the four
register pushes):

| binding | frame | arg pack | `LuaStateOwner` | descriptor |
| --- | --- | --- | --- | --- |
| `PilotSetTarget` | `0x530` | `ESP0+18h` | `ESP0+6Ch` | `ESP0+40h` |
| `PilotMoveTo` | `0x530` | `ESP0+18h` | `ESP0+6Ch` | `ESP0+54h` |
| `PilotMoveToRange` | `0x530` | `ESP0+18h` | `ESP0+6Ch` | `ESP0+40h` |
| `PilotRetreat` | `0x528` | `ESP0+3Ch` | `ESP0+64h` | `ESP0+24h` |
| `PilotLand` | `0x530` | `ESP0+18h` | `ESP0+6Ch` | `ESP0+54h` |

### The argument machinery

* `00B66C00 BSP_LuaStateOwner_ConstructBorrowed` — `__thiscall(owner)(lua_State* L)`,
  `RET 4`. Zeroes the object, clears `0x32` entries of `0x18` bytes from `+28h`, stores
  `L` at `+4h`, and registers a `DoFile` closure (`00B69E00`) into the globals table
  (`00A67B20` `lua_pushcclosure`, then `00A67E40` `lua_setfield` with
  `EDX = 0xFFFFD8EE` = `LUA_GLOBALSINDEX` and `00D62C04` = `"DoFile"`). The object spans
  `0x4C8` bytes, which is where the `0x530` frames come from.
* `00B679B0 BSP_LuaObject_OpenCallFrame` — `__thiscall(owner)(pack* out)`, `RET 4`.
  Writes `{owner, kind = 3, base = 1, count = lua_gettop(L), 0}`.
* `00B663F0` — `return pack[0Ch]`, i.e. `lua_gettop` as captured at frame open. This is
  the **argument count**.
* `00B677E0 BSP_LuaObject_ArgumentAt(out, index)` — `RET 8`, forwards to `00B67720`,
  whose non-`kind == 2` path (`00B677B7`) sets `out = {owner, kind = 2,
  stack_index = pack->base + index}`. `base` is 1, so **index 0 is the first Lua
  argument**. Verified at `00B677C0 ADD ECX,[ESP+14h]`.
* `00B66290` — `_ftol(lua_tonumber(L, stack_index))`: integer read.
  `00B66270` — the same without the `_ftol` tail: float read.
* `00B66400` — `lua_gettop(L) - pack[0Ch]`: the count of values pushed since the frame
  opened, returned to Lua. None of the five pushes anything, so all five return 0.

### How a Lua argument becomes an entity — the `Ptr` / `ID` split

The trap the brief flagged is real and the two paths genuinely differ:

* The **unit** argument (index 0) goes straight through
  `00888AA0 BSP_ObjectHandle_FromLuaTable`, which reads the table field named by the
  literal at `00CFAD08` — **`Ptr`** — and converts it as Lua userdata. It never touches
  `ID`. Evidence: the reconstruction comment already on `00888AA0`
  (`src/object_handle_resolvers.cpp`, `docs/OBJECT_HANDLE_RESOLVERS.md`).
* The **target** argument (index 1) goes through
  `0088A810 BSP_LuaObject_ReadCommandTarget`, which reads the field named by the literal
  at `00CE59B4` — **`ID`** — *only as a discriminator* (`00B65FB0` nil test at
  `0088A84A`). When `ID` is not nil it calls `00888AA0` on the same table, so the
  pointer still comes from **`Ptr`**; when `ID` is nil it reads the table as a `Vector3`
  position (`00888760`).

So both read `Ptr` for the pointer, and `ID` decides entity-versus-position. Neither
resolves through the id the way `src/game_hosts_script_orders.cpp:190`
`entity_from_argument` does. A host that only fills `ID` will make
`ReadCommandTarget` take the entity branch and then get a null pointer out of
`00888AA0`; a host that only fills `Ptr` will make it take the *position* branch and
read the table as a vector. **Both fields must be present on a target table.**

The descriptor `0088A810` fills is the 18h-byte `SceneCommandTarget` of
`include/bsp/scene_deferred_refs.hpp`.

### Correction to `include/bsp/scene_deferred_refs.hpp`

That header calls the descriptor's `+14h` field `float reserved{0.0f}; // always 0`.
It is **not** always 0: `PilotMoveToRange` writes its third Lua argument there
(`008A4708 FSTP float ptr [ESP+54h]`, and the descriptor base is `ESP0+40h`, so
`+54h` = descriptor `+14h`). `0077D600` copies it into the order message's `trailing`
field (`include/bsp/entity_orders.hpp`), so it is a live payload field, not padding.
I do not own that header; flagged for the integrator.

## `PilotSetTarget` — `008A4C90`

```
PilotSetTarget(unit, target)
PilotSetTarget(unit, target, attackType)
```

Ground truth for the third argument, from the shipped scripts of this installation:

* `scripts/global/luamw_init.lua:237-239`
  `ATTACKTYPE_ANY = 1`, `ATTACKTYPE_GUN_ONLY = 2`, `ATTACKTYPE_BOMB_OR_TORPEDO = 3`.
* `scripts/global/commandhelpers.lua:2936` AutoDoc:
  `attackType: mixed, ATTACKTYPE_ANY, ATTACKTYPE_GUN_ONLY, ATTACKTYPE_BOMB_OR_TORPEDO, nil`.

### Body

| step | addresses | what happens |
| --- | --- | --- |
| 1 | `008A4CAF`-`008A4D4B` | the `"luaMW_PilotSetTarget failed:"` literal and the one-shot `"luakod"` init behind `00F87FC4 & 1`. No observable effect. |
| 2 | `008A4D4B`-`008A4D6A` | `LuaStateOwner` at `ESP0+6Ch` from `ECX`; arg pack at `ESP0+18h`. `EBP`, which held the context, is reloaded with `1` at `008A4D59` — the attack-type default. |
| 3 | `008A4D6F`-`008A4D90` | argument 0 -> `00888AA0` -> `EDI` = unit. |
| 4 | `008A4DA8`-`008A4DD2` | liveness gate on the unit: `EDI != 0`, `[EDI+5Ch] != 0`, `[EDI+5Dh] == 0`, `[EDI+60h] == 0`, `[EDI+5Eh] == 0`. Any failure jumps to the epilogue at `008A4EB1` and the binding does nothing. This is the standard quartet of `docs/AIRFIELD_TAXI.md:130`. |
| 5 | `008A4DD8`-`008A4DF5` | argument 1 -> `0088A810` -> descriptor at `ESP0+40h`. |
| 6 | `008A4E0B`-`008A4E38` | `if (00B663F0() == 3) EBP = 00B66290(argument 2)`. An **exact** compare with 3, not `>= 3`. |
| 7 | `008A4E4B` | `00521EA0 BSP_CommandTarget_ResolveObject(descriptor)` -> `ESI` = target entity, cached into descriptor `+4h`. |
| 8 | `008A4E54`-`008A4E5F` | `BL = (EBP != 3)`, `AL = (EBP != 2)`. |
| 9 | `008A4E6B` | `EAX = 007EEC50(ECX = unit)(ESI, AL, BL)`. **Unconditional** — it runs before the target is checked. |
| 10 | `008A4E70`-`008A4E8A` | the same liveness quartet on `ESI`. Failure -> `008A4EB1`. |
| 11 | `008A4E8E`-`008A4EA0` | when `EAX == 0`, the same call again with the same three arguments; still 0 -> `008A4EB1`. |
| 12 | `008A4EA2`-`008A4EAC` | `0077D600(ECX = unit)(EAX, ESP0+40h, 1)`. |
| 13 | `008A4EB1`-`008A4EFB` | `00B66400` result count, destructors, return. |

### The two flags

`007EEC50 BSP_Unit_ChooseAttackCommand` is
`CommandClass* __thiscall(unit)(Entity* target, char prefer_ordnance, char allow_guns)`
(`docs/ATTACK_COMMANDS.md`). Push order at `008A4E68`-`008A4E6A` is `BL`, `AL`, `ESI`,
so `arg1 = target`, `arg2 = (attackType != 2)`, `arg3 = (attackType != 3)`:

| `attackType` | `prefer_ordnance` | `allow_guns` | effect |
| --- | --- | --- | --- |
| 1 `ATTACKTYPE_ANY` (and the default) | 1 | 1 | the seven ordnance classes first, guns as the fallback |
| 2 `ATTACKTYPE_GUN_ONLY` | 0 | 1 | the ordnance pass is skipped; `dogfight` / `strafe` only |
| 3 `ATTACKTYPE_BOMB_OR_TORPEDO` | 1 | 0 | a gun-only answer is discarded |

This is a derivation from the two `SETNZ` instructions plus `007EEC50`'s documented
parameter roles, and it lands exactly on the three script constants and their Hungarian
comments (`ATTACKTYPE_GUN_ONLY` = "csak gepagyuval lojunk meg ha van bomba is" —
"shoot with the machine gun only, even if there is a bomb too"). I treat the mapping as
established.

**A nil third argument is not the default.** `commandhelpers.lua:2955` reads
`PilotSetTarget(entity, target, attacktype)` — lower-case, a different (nil) global from
the `attackType` parameter it guards on. `lua_gettop` still counts 3, so step 6 fires
and `00B66290` turns nil into **0**, not 1. `0` happens to behave like
`ATTACKTYPE_ANY` (`0 != 2` and `0 != 3`), so the shipped script's typo is benign — but a
host that substitutes the default for a nil third argument would still be wrong for any
script that passes a non-numeric value.

### The repeated `007EEC50` call

Steps 9 and 11 call the same function with the same three arguments. The call targets
were checked against the raw bytes, not just Ghidra's flow: `008A4E6B`
`e8 e0 9d f4 ff` -> `007EEC50`, `008A4E99` `e8 b2 9d f4 ff` -> `007EEC50`. The only
difference is the upper three bytes of the second argument's dword (a stale pointer at
`008A4E69`, a stale string length reloaded from `[ESP0+10h]` at `008A4E90`); `007EEC50`
reads it as `char`, so the argument values are identical.

I could not establish why. `007EEC50`'s reconstruction describes only reads, which would
make the second call redundant, but I did not verify that it writes nothing, so I do not
claim the retry is a no-op. `src/pilot_order_bindings.cpp` reproduces both calls.

## `PilotMoveTo` — `008A4150`

`PilotMoveTo(unit, target)`. Straight-line, no branches at all between `008A422D` and
`008A42A7`:

1. `008A422D`-`008A424F` argument 0 -> `00888AA0` -> `ESI`.
2. `008A4266`-`008A4284` argument 1 -> `0088A810` -> descriptor at `ESP0+54h`.
3. `008A4299`-`008A42A7` `0077D600(ECX = ESI)(00E08F68, ESP0+54h, 1)`.

**No argument-count test, no liveness gate, and no null check on the unit.** When the
Lua table has no usable `Ptr`, `ECX` is 0 at the call. The native trusts the script.

## `PilotMoveToRange` — `008A4590`

`PilotMoveToRange(unit, target)` / `PilotMoveToRange(unit, target, range)`.

1. `008A466D`-`008A4692` argument 0 -> `00888AA0` -> `ESI`.
2. `008A46A9`-`008A46C7` argument 1 -> `0088A810` -> descriptor at `ESP0+40h`.
3. `008A46DC`-`008A4717` `if (00B663F0() == 3) descriptor->+14h = 00B66270(argument 2)`.
   The `FSTP float ptr [ESP+54h]` at `008A4708` is descriptor `+14h`; when the test
   fails the field keeps the 0 that `0088A8C7` wrote.
4. `008A471C`-`008A472A` `0077D600(ECX = ESI)(00E08F68, ESP0+40h, 1)` — the **same**
   `moveto` class as `PilotMoveTo`. The range is the whole difference.
5. `008A472F`-`008A475A` three copies of
   `if (![ESI+C8h]) 00414DB0(ESI)`, each short-circuiting to the epilogue, executed
   *after* the issue. `00414DB0 BSP_EntityPose_RefreshWorld` sets `+C8h` itself, so only
   the first body can ever run; the repetition is an unrolled guard. Its effect is
   "make the unit's world matrix current". I could not establish why it sits after the
   command rather than before it, and nothing in the binding reads the matrix.

Naming `descriptor+14h` a *range* is inference from this binding and its name; the field
is opaque to `0077D600`, which only forwards it.

## `PilotRetreat` — `008A4300`

`PilotRetreat(unit)` — one argument. This is the only one of the five that builds its
descriptor by hand instead of from a Lua table.

1. `008A43DE`-`008A4412` argument 0 -> `00888AA0` -> `ESI`. No liveness gate.
2. `008A441D` `EDI = [ESI+54h]`, the unit's side (`docs/ATTACK_COMMANDS.md:271`), read
   before the pose guard at `008A4417`-`008A4424` (the same `+C8h` / `00414DB0` pair as
   above, here written once).
3. `008A4429`-`008A4439` `004C7730(ECX = [00E188A8])(&ESI+FCh, EDI, 0, 0)`.
   `[00E188A8]` is the world object `0075B430` also reads
   (`include/bsp/entity_orders.hpp`); `ESI+FCh` is the unit's world position.
   `004C7730` is `int __thiscall(int, float*, int, float*, undefined4*)`,
   body `004C7730`-`004C7A96`, five callers (`007F16D0 BSP_Plane_ResolveReturnToBase`,
   `004C7AA0`, `008A4300`, `008AECD0`, `009C8D40`). **Its body is not reconstructed by
   this packet.** What it returns is a record whose `+10h`, `+1Ch`, `+28h` and `+34h`
   are four `Vector3`s at stride `0Ch` — the binding reads only `.x` (`+0h`) and `.z`
   (`+8h`) of each. `007F16D0`'s `retreat` arm takes the same side-plus-position shape
   (`docs/ATTACK_COMMANDS.md:270`), so "the retreat zone for this side near this point"
   is the reading; four corners of a rectangle is the shape, not proven semantics.
4. `008A443E`-`008A452E` the position:
   `x = 0.25 * (c0.x+c1.x+c2.x+c3.x)`, `y = 0`, `z = 0.25 * (c0.z+c1.z+c2.z+c3.z)`.
   The `0.25` is `FLD double ptr [00D7A348]` at `008A450C`; the eight bytes there are
   `00 00 00 00 00 00 d0 3f` = `0.25`. `y` is the `XORPS XMM0,XMM0` of `008A4445`
   stored at `008A4465`, i.e. forced to zero, **not** taken from the corners.
   The accumulation seeds corner 0 through an `FLDZ` add (`008A445C`-`008A4488`) and
   folds 1, 2, 3 in; the multiply is done in double and stored through `FSTP float`.
5. The descriptor at `ESP0+24h`: `kind = 0` (`008A4471`), `position_valid = 1`
   (`008A445E`), `object_id = 0` (`008A4475`), `object = null` (`008A446D`),
   `+14h = 0` (`008A447E`). Identical to the position branch of `0088A810`.
6. `008A4532` `0077D600(ECX = ESI)(00E08F90, ESP0+24h, 1)`. The `1` was pushed early,
   at `008A444B`, then the descriptor at `008A44EA` and the class at `008A44EF`.

## `PilotLand` — `008A47B0`

`PilotLand(plane, base)`. The same shape as `PilotMoveTo` with a different class:

1. `008A488D`-`008A48AF` argument 0 -> `00888AA0` -> `ESI`.
2. `008A48C6`-`008A48E4` argument 1 -> `0088A810` -> descriptor at `ESP0+54h`.
3. `008A48F9`-`008A4907` `0077D600(ECX = ESI)(00E08FA0, ESP0+54h, 1)`.

No argument-count test, no liveness gate, no pose refresh. `00E08FA0` is `land`
(`docs/SCENE_COMMAND_TYPES.md` row 22, category 3, `requires_target` yes). Script usage
confirms the second argument is a carrier or an airfield
(`PilotLand(Mission.Avenger, Mission.USNCarrier)`, `PilotLand(Mission.Zero,
Mission.Airfields[1])`).

## The command records, side by side

`0077D600` is `__thiscall(entity)(void* command, SceneCommandTarget* target, int flags)`,
`RET 0Ch` (`include/bsp/entity_orders.hpp`). All five pass `flags = 1`.

| binding | `ECX` | `command` | `target.kind` | `target.object` | `target.position` | `target+14h` |
| --- | --- | --- | --- | --- | --- | --- |
| `PilotSetTarget` | arg0 `Ptr` | `007EEC50`'s answer | from arg1's `ID`/`Ptr` | resolved by `00521EA0` before the call | from arg1 when `ID` is nil | 0 |
| `PilotMoveTo` | arg0 `Ptr` | `00E08F68` | from arg1 | unresolved (`0088A810` leaves `+4h` as it found it) | from arg1 when `ID` is nil | 0 |
| `PilotMoveToRange` | arg0 `Ptr` | `00E08F68` | from arg1 | unresolved | from arg1 when `ID` is nil | arg2 as float, else 0 |
| `PilotRetreat` | arg0 `Ptr` | `00E08F90` | 0 (position) | null | zone centroid, `y = 0` | 0 |
| `PilotLand` | arg0 `Ptr` | `00E08FA0` | from arg1 | unresolved | from arg1 when `ID` is nil | 0 |

`PilotSetTarget` is the only one that resolves the target object itself; the other four
leave that to `0077D600`, which does the same lookup internally (`0077D676`-`0077D6B3`).

## Wiring contract for the host

What `MissionLuaNative::PilotSetTarget` must do, in this order. The other four are the
same skeleton with the marked steps removed.

1. `unit = Ptr(arg 0)` — the Lua table's **`Ptr`** field as a raw entity pointer,
   the way `00888AA0` does it. Not `ID`.
2. *(`PilotSetTarget` only)* if `unit` is null or fails the liveness quartet
   (`+5Ch` set, `+5Dh`/`+5Eh`/`+60h` clear), **return 0 and do nothing else**.
   The other four skip this and will call `0077D600` with a null `this`.
3. `target = ReadCommandTarget(arg 1)`: if the table's `ID` field is non-nil, build an
   entity descriptor whose pointer comes from `Ptr` and whose `object_id` is
   `entity+174h`; otherwise read the table as a `Vector3` and build a position
   descriptor. `+14h` starts at 0.
   *(`PilotRetreat` has no step 3: it builds a position descriptor from step 3' below.)*
   * 3'. `PilotRetreat`: read `unit+54h` (side), make the unit's world pose current,
     ask the world object for the retreat zone near `unit+FCh` for that side, and set
     `position = (0.25*sum(corner.x), 0, 0.25*sum(corner.z))`, `kind = 0`,
     `position_valid = 1`, `object_id = 0`, `object = null`.
4. *(`PilotSetTarget`)* `attackType = (lua_gettop() == 3) ? (int)lua_tonumber(L, 3) : 1`.
   An exact 3. Do not coerce a nil to the default.
   *(`PilotMoveToRange`)* `target.reserved = (lua_gettop() == 3) ? (float)lua_tonumber(L, 3) : 0`.
5. *(`PilotSetTarget`)* `object = ResolveTargetObject(target)` — the `00521EA0` handle
   lookup, which also caches the pointer into the descriptor.
6. *(`PilotSetTarget`)* `command = ChooseAttackCommand(unit, object,
   attackType != 2, attackType != 3)` — `WeaponDirector`'s existing class selector.
   Call it **before** checking the target, as the native does.
7. *(`PilotSetTarget`)* if `object` is null or fails the liveness quartet, return 0.
8. *(`PilotSetTarget`)* if `command` is null, repeat step 6 once; if it is still null,
   return 0.
9. `entity_issue_command(unit, command, target, 1)` where `command` is
   `007EEC50`'s answer for `PilotSetTarget`, `00E08F68` for `PilotMoveTo` and
   `PilotMoveToRange`, `00E08F90` for `PilotRetreat`, `00E08FA0` for `PilotLand`.
10. *(`PilotMoveToRange` only)* make the unit's world pose current.
11. Return 0 to Lua. All five push nothing; `00B66400` computes
    `lua_gettop(L) - argc`, which is 0 for every one of them.

Contract items the host cannot satisfy from this packet alone:

* The retreat zone (step 3'). `004C7730` is not reconstructed, so a host has to supply
  the four corners from somewhere else, or stub `PilotRetreat` and say so.
* `ChooseAttackCommand`. `007EEC50` is reconstructed in `docs/ATTACK_COMMANDS.md` and
  the arms live in `src/cruise_command.cpp` / `docs/ENTITY_COMMAND_ARMS.md`; this packet
  only establishes the two flags it is called with.

## Status and gaps

* **Exported / reconstructed.** All five bodies, read from the Ghidra listing
  instruction by instruction. Stack offsets were derived from the prologue and checked
  against the epilogue in each function.
* **Recovered symbols.** The five binding names and the three command classes.
* **Ground truth from data, not inference.** The `ATTACKTYPE_*` values and the
  `attackType` argument's documented role, from the shipped scripts of this
  installation. This install carries mod artefacts, so the provenance matters:
  `scripts/global/luamw_init.lua` has the bulk mtime 2024-07-13 and
  `scripts/global/commandhelpers.lua` 2024-10-29 — neither is near the local mod's
  2026-05-09 edits, and the constants also agree with what the binary computes.
* **Build-tested.** `src/pilot_order_bindings.cpp` compiles clean as a standalone
  translation unit under MSVC (`cl /c /EHsc /W4 /std:c++17`). It is not registered in
  `CMakeLists.txt` — the integrator owns that.
* **Not fixture-tested, not game-validated.** No test was added.
* **Not established.** Why `PilotSetTarget` calls `007EEC50` twice; why
  `PilotMoveToRange` refreshes the pose after the issue; what `004C7730` actually
  returns; whether `descriptor+14h` really means a range.
* **Not ABI-compatible.** `PilotOrderHost` is a reconstruction seam, not the native
  call graph.

## The last link: `007EEC50`'s feasibility inputs, and one contradiction to resolve first

`PilotSetTarget`'s argument path is wired and measured (`src/game_hosts_script_orders.cpp`,
`run_pilot_set_target`). On `USN01` it resolves five calls to real units and real targets:

```
PilotSetTarget: unit=Mav1 target_object_id=45 attack_type=1 prefer_ordnance=1 allow_guns=1
PilotSetTarget: unit=Mav2 target_object_id=44 ...
```

It stops short of issuing, because the command class comes from `007EEC50` and its
`AttackFeasibilityInputs` (`include/bsp/attack_commands.hpp`) need about a dozen per-class
capability answers. Four of them are the ordnance inventory:

```
has_level_bomb_ordnance    007ED830 -> 007B9500, kind 31h
has_general_bomb_ordnance  007ED7E0 -> 007B9320, kind 2Ah minus 2Ch/31h/2Bh/33h/2Dh
has_drop_kamikaze_ordnance 007ED880 -> 007B93E0, kind 2Fh
has_torpedo_ordnance       007ED8D0 -> 007B93F0, kind 2Bh
```

Each walks the weapon controller's `+994h` slots at `+974h`, takes `slot->vtable[220h](loadout)` and
asks the descriptor `descriptor->vtable[8](kind)`.

### The lead

The host does not build those descriptors, but it may not need to. `GameGunRow::bullet_class`
already indexes the authored bullet-class table, and `read_bullet_class_string(bullet_class, "Type")`
already reads its `Type`. The shipped `bulletclasses.lua` carries exactly the right vocabulary:

```
Artillery  Bomb  Bullet  Depthcharge  DummyKamikazePlane  DummySubmarine
DummyTarget  Flak  Kamikaze  Paratrooper  Rocket  Torpedo  WaterMine
```

And `docs/ENTITY_CLASS_IDS.md` gives ids that line up with the kind numbers almost exactly:

| kind | `ENTITY_CLASS_IDS` | class used for |
| --- | --- | --- |
| `2Ah` | `MBomb` | general bomb |
| `2Bh` | `MTorpedo` | torpedo |
| `2Ch` | `MDepthCharge` | depth charge |
| `2Fh` | `MDummyKamikazePlane` | drop kamikaze |
| `33h` | `MRocket` | rocket |
| **`31h`** | **`MParatrooper`** | **`007B9500` calls this `levelbomb`** |

So the hypothesis is that the descriptor's `vtable[8]` kind *is* the projectile's entity class id,
which would make the whole inventory readable from data the host already loads, through the same
enum resolution that already turns `Type = E ShipClasses : <symbol>` into an id.

### Why it is not being acted on

**Five of six fit and the sixth contradicts.** `31h` is `MParatrooper`, and nothing about dropping
paratroopers is a level bombing run. That is exactly the shape of mistake this project has made
repeatedly - a census that fits most cases and is then assumed to fit the rest - and
`docs/ATTACK_COMMANDS.md`'s `levelbomb` label is itself a hypothesis, so the contradiction could lie
on either side.

Building the ordnance inventory on an unproven correspondence would let `007EEC50` pick command
classes, which would let orders issue, which would make every gunnery number downstream look
validated. That is the one failure mode this reconstruction cannot afford, so the mapping gets
proved before it gets used.

### The packet

**`ordnance_kind_identity`.** Read `007B91C0`'s `descriptor->vtable[8]` call and establish what
enumeration its argument belongs to. Then resolve the `31h` case specifically: either `007B9500`'s
kind is not `31h`, or `31h` is not `MParatrooper` in this context, or `levelbomb` is the wrong label
for that helper. One of the three is wrong and the answer decides whether the host can read its
ordnance inventory straight out of `bulletclasses.lua`.

If it can, the remaining feasibility inputs are the `vt[5Ch](n)` capability queries
(`self_is_level_bomber`, `target_is_submarine`, `target_is_bomb_excluded`,
`target_is_strafe_fallback` and the rest), which are a second packet and a smaller one - several are
already derivable from the unit-kind classification the gunnery contact path uses.
