# The mission Lua self table `thisTable`

Addresses: 00ce7494 00ce74a0 004dfb70 004e01f7 004e0305 00928a00 00927b40 00887750 00887e50 00898750 009290a0 00884240 00885da0 00440e10 009292b0 00b65fb0 00b67580 00b67350 00b67630 00b673a0 00b67530 00b675d0 00b679b0 00b67800 00b67720 00b663d0 00b66400 00b66430 00803a40

Packet `cc_mission_natives`, worktree `agent/cc-mission-natives`. Ghidra was read-only. Every
name here is a hypothesis, not a recovered symbol.

`docs/MISSION_LUA_MACHINE.md` left the `Mission` global and the `this` of a mission script
entry point unread, and its probe stubbed every binding, so six installed scripts failed. This
document settles what `thisTable` is, who fills it and how a script reaches its own object. The
reconstruction is `include/bsp/mission_lua_bindings.hpp` and `src/mission_lua_bindings.cpp`; the
entity return side is `docs/LUA_BINDING_ENTITY.md` and the table survey is
`docs/LUA_BINDING_TABLE.md`.

## Corrections

**`00CE7494` is the string `"thisTable"`, not a vtable.** The `mission_lua_self_table` follow-up
row in `docs/MISSION_LUA_MACHINE.md` lists `00ce7494` alongside two function addresses and the
packet brief calls it "the vtable 00ce7494". `python tools/bsp.py ghidra bytes 00ce7494` gives
`74 68 69 73 54 61 62 6c 65 00`, and every one of its thirty-three referencing functions pushes
it as a name argument to `00B67800`, `00B67910` or `006B8460`. Previous value recorded before
this correction: "`mission_lua_self_table` | 00887750 00884240 00ce7494 | ... the vtable
00ce7494".

**`00884240` is not part of the self table.** The same follow-up row lists it as one of the two
functions that populate `thisTable`. Its complete body is eleven instructions,
`_vsprintf_s(dst, 8, fmt, va)`, and its sole caller is `00885DA0
BSP_MissionLua_PushArgumentRecord`. It is the `"%d"` formatter that produces the self key, which
is how it reaches this subject, but it writes nothing into any table. Previous value recorded
before this correction: "`00887750 00884240 00ce7494` ... who populates the `thisTable` self
object".

**`Effect` (`008A9730`) does not return an entity.** The `lua_binding_find_entity` follow-up row
pairs `FindEntity` with `Effect`. `008A9730` never pushes `00CE7494` and never reaches
`00B663D0`; it scans a parameter table through `00B67080` and its argument converters are
`00888760`, `008889C0` and `00888AA0`. The nineteen bindings that do return an entity are listed
in `docs/LUA_BINDING_ENTITY.md`. Previous value recorded before this correction: "the `FindEntity`
and `Effect` rows of 00e0b7b8 | ... The return convention of the entity-returning bindings".

## What the table is

A game entity reaches Lua as a **plain table** held in the global `thisTable`, keyed by the
decimal text of the entity's 16-bit id. There is no userdata proxy for the entity and no
metatable on the table; the only native pointer in it is the `Ptr` field, which is light
userdata.

| Part | Where | Evidence |
| --- | --- | --- |
| the global name | `00CE7494` | the literal every referencing function pushes |
| the key | decimal of the u16 at `entity+174h` | `00898F98 MOVZX EAX, word ptr [EAX + 0x174]`, then `004260B0` |
| the key cached on the entity | `entity+178h`, a NativeString | `00928A36 LEA ESI,[EDI + 0x178]`, filled at `00928A54`/`00928A6A` |
| the same key on the argument side | `00884240` with `"%d"` at `00CE3A34` | `00885DA0` tag 3 formats a `short` into an 8-byte buffer |

The two paths agree by construction: `00885DA0` case 3 does
`thisTable[sprintf("%d", short)]` and every binding does `thisTable[NativeString::FromInt(u16)]`.

## Who creates the table

`BSP_Game_LoadMissionScene` (`004DFB70`) creates the global once per mission load, at
`004E01F7..004E02AC`:

```
globals = 00B67980(owner at game+1A0Ch)          // kind 1, LUA_GLOBALSINDEX
if (00B65FB0(globals["thisTable"]))              // lua_type(...) == LUA_TNIL
    00B67580(globals, "thisTable")               // globals["thisTable"] = {}
005E2F00()                                       // the LobbySettings table
00B67350(globals, "recon")                       // globals["recon"] = nil
```

`00B65FB0` already carried the ledger name `BSP_LuaObject_IsNil` from the GUI-side reading in
`docs/GUI_LUA_READER.md`; this packet reconfirmed it from the mission side and appended the
default branch rather than renaming it. It is only true for a **bound** object whose `lua_type`
is `LUA_TNIL`: kind 0 answers
`LUA_TNONE` (`00B65FB9 OR EAX,0xFFFFFFFF`) and every other kind is hard-coded to `LUA_TTABLE`
at `00B65FE2`. So a table that survived a previous mission is kept rather than replaced, and
stale per-entity slots are cleared one at a time by `00928A00` instead.

The `recon` clear at `004E02F7`/`004E0305` is the same `00B67350` set-nil shape over the string
at `00CE74A0`. It is recorded here because it sits in the same pass and because it is the one
thing the sweep could not reconcile; see Uncertainties.

## Who fills a slot

`00928A00`, `__thiscall(ECX = entity)`, sole caller `0077E830`, body `00928A00..00928C79`:

| Step | Address | Effect |
| --- | --- | --- |
| format the key | `00928A2F` | `004260B0` over the u16 at `+174h` |
| cache it on the entity | `00928A54`, `00928A6A` | the NativeString at `entity+178h` |
| read the current slot | `00928A9F` | `00927B40`, `thisTable[entity->key]` |
| drop a stale slot | `00928AFF` | `00B67350`, only when `00B65FB0` said the slot was not nil |
| assign a fresh table | `00928B53` | `00B67580`, `thisTable[key] = {}` |
| `ID` | `00928BA5` | `00B67630` -> `lua_pushstring`, so **a string**, not a number |
| `Dead` | `00928BC7` | `00B673A0` -> `lua_pushboolean(0)` |
| `Ptr` | `00928C2F` | `00B67530` -> `lua_pushlightuserdata(entity)` |

The field names are `00CE59B4` `ID`, `00CF829C` `Dead` and `00CFAD08` `Ptr`. The branch at
`00928AB4` (`entity->[C0]` non-null with kind 3) skips the whole block and was not read.

`Ptr` closes the loop with the already-reconstructed reverse direction: `00888AA0
BSP_ObjectHandle_FromLuaTable` reads `table["Ptr"]` through `00B67800` and converts it with
`00B662D0 BSP_LuaObject_ToUserdata`. It has 278 callers, so it is *the* way a binding takes an
entity argument. It is reused (`object_from_lua_table_00888aa0` in
`include/bsp/object_handle_resolvers.hpp`), not restated.

`00927B40`, `__thiscall(ECX = entity, [esp+4] = out)`, is the canonical getter with 25 callers:
`globals(owner at [00E188A8]+1A0Ch)["thisTable"][entity->keyString]`, via `00B67980`,
`00B67800` and `00B68100`.

### Fields other subsystems add

`Class` is not seeded by `00928A00`; a per-kind setter adds it later, all through `00B675D0`
(set a name to another object's value) on the object `00927B40` returns:

| Setter | Global it indexes | Index |
| --- | --- | --- |
| `009292B0` | `VehicleClass` (`00CE5880`) | the value at `009293C0` |
| `00440E10` | `DeviceClass` (`00CE44BC`) | `entity->[354h]->[6Ch]` |

This is why `FindEntity("Zuiho-class 01").Class.Height` resolves in the shipped scripts: `Class`
is a row of a datatable-built global table, not a native object.

## What `this` is

`00887750 BSP_MissionLuaHost_CallNamed` is `__thiscall` with `RET 1Ch`, so seven stack
arguments. Argument 1 is a NativeString self key. Block `008878C7..0089790F` runs only when the
key pointer is non-null **and its size word is non-zero** (`008878CD CMP dword ptr [EDI],0x0`,
so the size, not the data pointer, is the test) and emits four calls in this order:

| Address | Call | Effect |
| --- | --- | --- |
| `008878E1` | `006B8460` | `lua_getglobal("thisTable")` |
| `008878F6` | `006B8120` | `lua_pushstring(key)`, empty string at `00F87904` when the data pointer is null |
| `00887900` | `006B8470` | `lua_gettable(-2)` |
| `0088790A` | `006B7EA0` | `lua_remove(-2)` |

`EBX` is then 1, which is the argument count the call starts from. So `this` inside a named
call is the entity's own table, the same object the entity-returning bindings hand back.

## How `Mission` reaches the scripts

Installed-file-checked. `Scripts/missions/usn/usn_2_java.lua` has `Mission = this` at line 45,
inside `function luaInit(this)`, and `luaStageInit` at line 31 contains `CreateScript("luaInit")`.

`CreateScript` is the binding at `00898750`. It allocates a **new** 0x1E4-byte script entity
(`00898834 PUSH 0x1E4` into `00BF55BE`, zeroed by `00BF79F0`, constructed by `00928630`, four
vtable words at `00898874..00898888`, `[+C4] = 3`), places it in the scene through the virtual
at `[*entity+98h]`, registers the name through `009290A0`, and returns that entity's self table
by the tail in `docs/LUA_BINDING_ENTITY.md`. The engine later calls the registered name with
that table as `this`, and the script assigns it to the global `Mission`.

### The forwarded arguments

`00898945 CMP EAX,1` / `0089894A MOV EDI,0x2` sets a stack index to 2 when the binding saw more
than one argument and leaves it 0 otherwise. `009290A0` (`__thiscall(ECX = entity)`, `RET 10h`)
forwards `(entity+178h, name, 0, that index, -1)` to `00887E50
BSP_MissionLuaHost_CallNamedThreadSafe` at `00929139`. `00887EE1..00887F19` normalises both ends
the way `lua_absindex` does (`00887EFD LEA ECX,[EAX + EDI*0x1 + 0x1]` after `lua_gettop`) and a
first index of zero means "no arguments". So the range is **every Lua argument after the name**,
and `CreateScript(name, a, b)` produces a later call `name(this, a, b)`.

Installed-file-checked: `Scripts/global/timetable.lua` line 1 is
`function luaDoTimeTable(this, timetable, paramTable)` and `COTP-IJN/jm04.lua` reaches it with
`argc=3`. Without the forwarded range the probe failed at `timetable.lua:25` on a nil
`this.TimeTable`; with it the call succeeds.

## The probe

`src/mission_script_probe.cpp` gained `--no-self-table`, `--recon-tables`, `--sweep` and
`--quiet`. It creates `thisTable` the way `004E01F7` does, mints an entity table per
entity-returning call the way `00928A00` does, and calls every `CreateScript` registration with
`thisTable[key]` plus the forwarded arguments.

Sweep over all 299 installed mission scripts, `--sweep --recon-tables`:

| Outcome | Count |
| --- | --- |
| chunk loads, all defined entry points run, no error | 299 |
| load failures | 0 |

Without `--recon-tables` the count is 297; the two exceptions are named under Uncertainties.
`docs/MISSION_LUA_MACHINE.md`'s figure was 293 of 299.

## Uncertainties

- `recon`. `004E0305` nils the global on every mission load, but its only builder is
  `00803A40 BSP_ReconTables_Install`, whose sole caller is `004DC6A0
  BSP_Game_ConstructGlobalSubsystems`, a bring-up step, plus `recon = {}` at
  `Scripts/global/luamw_init.lua:525`. Those do not compose: under the probe's ordering the
  table is gone by `luaEngineMovieInit`, and both copies of
  `COTP-USN/usn_02_battle_of_cape_esperance` then fail at `commandhelpers.lua:12343` on
  `recon[lParty].own`. Either the clear is ordered differently against the recon install than
  the probe assumes, or a per-mission rebuild exists that this packet did not find. The empty
  table `luamw_init.lua` leaves would not be enough either, because the scripts index
  `recon[party][relation]`.
- The `00928AB4` branch of `00928A00` (`entity->[C0]` with kind 3) was not read, so what a
  script entity that already owns a Lua object does instead is unknown.
- Three further callers of `00B675D0` that push the literal `"Class"` were not read: `006E2E10`,
  `004AED50` and the pair at `00742E76`/`00742EE4`.
- Which entity kinds `0077E830` reaches `00928A00` for was not established; only that it is the
  sole caller.
- `00887750`'s arguments 2 through 7 beyond the self key were not attributed individually. Only
  argument 1 and the byte at argument 7 (`00887782 CMP byte ptr [ESP + 0x48],BL`) were read.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_recon_rebuild` | 004e0305 00803a40 004dc6a0 | docs/MISSION_RECON_REBUILD.md | What rebuilds the global `recon` between the mission-load clear and `luaEngineMovieInit` |
| `mission_entity_lua_attach` | 0077e830 00928a00 00928c80 00928f50 | docs/MISSION_ENTITY_LUA_ATTACH.md | Which entity kinds get a self table, and the unread `entity->[C0]` kind-3 branch |
| `mission_named_call_arguments` | 00887750 00887e50 00887220 | docs/MISSION_NAMED_CALL_ARGS.md | Arguments 2 through 7 of the named call and the result-collection mode |
| `lua_object_kind_three` | 00b679b0 00b67720 00b67800 | docs/LUA_OBJECT_API.md | Kind 3, the call-frame object, as an addition to the existing LuaObject document |

## no_ghidra_function

none. Every code address in the `Addresses:` line has a Ghidra function, checked with
`python tools/bsp.py ghidra proto <addr> --brief` over all twenty-seven; `004E01F7` and
`004E0305` resolve to their enclosing function `004DFB70`. `00CE7494` and `00CE74A0` are data,
the two string literals, and are listed because they are the subject, not because they are code.
Twenty-four **binding handlers** have no Ghidra function; they are tabulated in
`docs/LUA_BINDING_TABLE.md`, not here, because they are that document's subject.
