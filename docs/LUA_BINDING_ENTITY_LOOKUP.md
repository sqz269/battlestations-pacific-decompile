# Entity lookup by name (`0088B1B0`) and the `FindEntity` binding

Addresses: 0088b1b0 00898e30 00484540 006fe620 00922fd0 00903670 00903610 00487210 00cfc3d0 00ce8550

Packet `cc2_entity_lookup`, worktree `agent/cc2-entity-lookup`. Ghidra was read-only for this
packet; every descriptive name below is a hypothesis, not a recovered symbol. It answers the
`lua_binding_entity_lookup` follow-up of `docs/LUA_BINDING_ENTITY.md`, whose open item was
"the set of kind indices `0088B1B0` searches was not decoded past the jump table".

Reconstruction: `include/bsp/lua_binding_entity_lookup.hpp`, `src/lua_binding_entity_lookup.cpp`.

## ABI

`0088B1B0` is `__stdcall`, `RET 8`: two stack dwords, a **NativeString by value**
`{ std::int32_t length; char* data; }`. The caller makes room with `00898F53 SUB ESP,8`, fills the
two slots in place and lets the callee read them at `0088B1CA MOV EDI,[ESP+0x28]` (data) and
`0088B26C MOV EAX,[ESP+0x24]` (length). The callee **destroys the argument**: on every exit it
runs `00419CC0` / `00BD1510` with `(data, length + 1, 1)`, returning the buffer to the sized
storage pool. The entity pointer, or zero, comes back in `EAX`.

The decompiler renders that release as `BSP_SizedStoragePool_GetSingleton(data, length+1, 1)`.
That is wrong: `00419CC0` takes no arguments (`docs/NATIVE_STRING_POOL_OWNER.md`) and the three
pushes are `00BD1510`'s, which is `__thiscall(pool, block, size, flag)` with `RET 0Ch`.

## The lists it walks

`[00E188A8]` is the game object and `[game+19CCh]` the world node (`docs/GAME_WORLD_CONSTRUCT.md`,
`docs/UNIT_INSTANCE_UPDATE.md`). The world carries an array of intrusive list bases. The producer
is `00484540`, list push-back:

| Object | Offset | Field |
| --- | --- | --- |
| list base | `+0h` | element count, `0048457E ADD dword ptr [ESI],1` |
| list base | `+4h` | head, `00484586` |
| list base | `+8h` | tail, `00484578` |
| node | `+0h` | prev, `0048456C` |
| node | `+4h` | next, `0048456C`/`0048458C` |
| node | `+8h` | value, the entity, `00484566` |

`0088B203 MOV ESI,[EDX + EBP*1 + 0x1C]` with `EBP = (kind + 6) * 0Ch` therefore loads the **head**
of the base at `world + 18h + (kind + 6)*0Ch`. The `+18h` base is confirmed by the registration
sites, which take `ECX = [instance+30h]` (the parent node, `= [game+19CCh]` for a world-level
entity) and add `30h`, `48h`, `54h`, `60h`, `6Ch` (`006FE620`), `36Ch` (`00487210`), and so on:
every one of the 174 observed call sites uses an offset of the form `18h + n*0Ch`. `docs/MISSION_RESULT_DECISION.md`'s walk of
`[[game+19CCh]+28h]` (next `+4h`, value `+8h`) is the same array at `n = 1`.

The loop runs `kind = -6 .. 5Ah` (`0088B1D8`, `0088B2CA CMP EBX,0x5B / JL`), 97 buckets.

## The jump table at `0088B310`

`0088B1E0 CMP EBX,0x47 / JA 0088B2C4` is an **unsigned** bound, so the six negative kinds and
`48h..5Ah` skip without reading the table. The table is two dwords at `0088B310`
(`0088B1F7` = walk, `0088B2C4` = skip) and a 72-byte selector at `0088B318`; fourteen of its
entries are zero.

Each kind is a class in the entity hierarchy: a class registers itself onto one bucket per level,
so the walked kind is the most derived level of the classes that are findable by name. The class
column below is the constructor that installs the vtable holding the registrar (the registrar sits
at vtable `+130h` in all fourteen), and the scene class name is the row of the 26-class table of
`docs/SCENE_ENTITY_FACTORY.md` whose creator calls that constructor.

| kind | bucket base | registrars | traced registrar | vtable | constructor | class |
| --- | --- | --- | --- | --- | --- | --- |
| `00h` | `+60h` | 9 | `006FE620` | `00CFC3D0` | `006FE460` `BSP_UnitInstance_Construct` | every `via unit` scene class (`MDestroyer` exemplar); eight further classes share the bucket |
| `09h` | `+0CCh` | 9 | `0084CAE0` | `00D0BA80` | `0084C920` | unnamed runtime class; eight further classes share the bucket |
| `12h` | `+138h` | 1 | `007F10B0` | `00D087C0` | `007F2C60` | `PlaneSquadronGen` (creator `004F0AD0`) |
| `13h` | `+144h` | 1 | `0074DE10` | `00CFFDE0` | `0074DCC0` | unnamed runtime class (`0074DF10`) |
| `14h` | `+150h` | 1 | `004F25D0` | `00CEA570` | `004F2410` | `LandConvoy` (creator `004F2700`) |
| `15h` | `+15Ch` | 2 | `006F59B0` | `00CFF3F8` | `00745940` | unnamed runtime class (`006F5610`, `00747000`); `006F5A50` also registers here |
| `16h` | `+168h` | 1 | `006F5A50` | `00CFB028` | `006F5AE0` | unnamed runtime class (`006F5CA0`), a subclass of the `15h` class |
| `3Bh` | `+324h` | 1 | `004E78D0` | `00CE8550` | `004E99B0` | `NavPoint` |
| `3Eh` | `+348h` | 1 | `004F1400` | `00CEA090` | `004F11C0` | `Landscape` (creator `004F1460`) |
| `3Fh` | `+354h` | 1 | `006D36D0` | `00CF8C08` | `006D3B10` | unnamed runtime class (`006D40D0`) |
| `40h` | `+360h` | 1 | `00846CF0` | `00D0B770` | `00848080` | unnamed runtime class (`00848380`) |
| `41h` | `+36Ch` | 1 | `00487210` | `00CE6290` | `0047B660` | `Path` (creator `004EA650`) |
| `44h` | `+390h` | 1 | `004E7E90` | `00CE8390` | `004E5850` | `CameraPath` (creator `004EA760`) |
| `47h` | `+3B4h` | 1 | `004F1910` | `00CEA218` | `004F1610` | `SpawnPoint` (creator `004F1A60`) |

`registrars` is how many of the 174 registration sites target that bucket; `traced registrar` is
the one whose vtable and constructor were resolved. Kinds `00h` and `09h` are shared by nine
classes each, so their `class` column names one member, not the bucket.

The fourteen were read from the selector bytes, `00 01 01 01 01 01 01 01 01 00 01 ...` at
`0088B318`; the bucket offsets and registrars from a sweep of all 52 callers of `00484540` (174 call sites)
(`local/map_world_lists.py`, output `local/world_list_offsets.txt`); the vtables from
`get_xrefs_to` on each registrar, all of which are referenced from exactly one data slot.

The **kind is not the scene class id**. `NavPoint` is scene class `41h` and world kind `3Bh`;
`Path` is scene class `47h` and world kind `41h`. The two numberings collide by coincidence.

Consequences worth stating: `MovieCamPos` and `MovieCamLookat` register kinds `3Ch` and `3Dh`
(`004E7930`, `004E7990`, the two vtables adjacent to `NavPoint`'s) and are **not** findable by
name. Neither is anything on the six negative buckets, which include the three base levels every
entity registers (`-4`, `-2`, `-1`) — the lookup deliberately searches derived buckets only.

## The four flag bytes

`0088B213..0088B235`, all on the entity, all `char`:

| offset | required | producer evidence |
| --- | --- | --- |
| `+5Ch` | non-zero | `00922FD0` clears it when the entity is marked for release; `00904BF0` gates the per-frame update on it (`docs/GAME_WORLD_ENTITIES.md`). "Live". |
| `+5Dh` | zero | `00922FD0` sets it with the same store; `00923BE0` reports `0.0f` health and `00825420` stops the audio while it is set (`docs/UNIT_TIMED_SUBUPDATES.md`). "Destroyed". |
| `+5Eh` | zero | `00922FD0` sets it; `00903670` walks the child chain for nodes whose `+5Eh` is set and `+6Ch` clear and marks them. "Release requested". |
| `+60h` | zero | no producer read in this packet. Recorded as a gate with an **unread** meaning. |

`00922FD0` sets `+5Eh = 1`, `+5Dh = 1`, `+5Ch = 0` and `+6Ch = 1` down the `+48h`/`+44h` child
chain, and `00903610` ages `+6Ch` to 3 before destroying the node, so a released entity fails the
`+5Ch` test on the frame it is marked, well before it is freed.

## The name compare

The candidate's name is `(*(*entity + 10h))()`, `__thiscall`, no arguments, `const char*` in
`EAX` (`0088B23D..0088B242`). That is the same virtual `0046C550` reads a parent's name through
(`docs/SCENE_ENTITY_FACTORY.md`, step 3), so all fourteen kinds compare the **same accessor**;
there is no per-kind field.

| argument | candidate name | result |
| --- | --- | --- |
| `data == 0` | null | match, `0088B24A` |
| `data == 0` | empty | match: `0088B24C..0088B25B` measures it and requires length 0 |
| `data == 0` | non-empty | skip |
| `data != 0` | null | match only when the NativeString's **length** field is 0, `0088B29E CMP [ESP+0x24],EAX` with `EAX == 0` |
| `data != 0` | non-null | `00BF7FBF __stricmp`, case-insensitive |

The first match wins, in kind order then list order; the routine returns the node's value.

## `00898E30 BSP_LuaBinding_FindEntity`

Body `00898E30-008990AE`. The argument decode and the entity tail were already recorded in
`docs/LUA_BINDING_ENTITY.md`; this packet confirms them against the listing and adds the result
handling.

| site | callee | name | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `00898EF0` | `00B66C00` | `BSP_LuaStateOwner_ConstructBorrowed` | owner, `L` | owner | always |
| `00898F09` | `00B679B0` | open call frame, kind 3, slot base 1 | frame | frame | always |
| `00898F20` | `00B677E0` | argument 0 as a LuaObject | frame, 0 | object | always |
| `00898F2F` | `00B662B0` | `BSP_LuaObject_GetString` (`lua_tolstring`) | object | `char*` | always |
| `00898F83` | `00BF7680` | `memcpy` into the fresh NativeString buffer | dst, src, len | — | length non-zero |
| `00898F8B` | `0088B1B0` | `BSP_MissionEntity_FindByName` | (NativeString by value) | entity or 0 | always |
| `00898F98` | — | `MOVZX EAX, word ptr [entity+174h]`, the u16 id | — | — | entity non-null |
| `00898FA7` | `004260B0` | `BSP_NativeString_FromInt`, the decimal key | string, id | string | entity non-null |
| `00898FC4` | `00B67910` | `BSP_LuaObject_GetByLiteral`, `"thisTable"` at `00CE7494` | frame, out, literal | object | entity non-null |
| `00898FDA` | `00B678E0` | `BSP_LuaObject_GetByNativeString`, `thisTable[key]` | table, out, key | object | entity non-null |
| `00898FE9` | `00B663D0` | `BSP_LuaObject_PushValue` | object | — | entity non-null |
| `0089903C` | `00B66430` | `BSP_LuaObject_PushNil` | frame | — | entity null |
| `00899045` | `00B66400` | `BSP_LuaObject_ResultCount`, `lua_gettop - base` | frame | count | always |

`EBP` is the function's zero register (`00898F90 CMP EAX,EBP` is the null test), so the nil tail is
taken exactly when `0088B1B0` returned zero. The binding never raises: a non-string argument
reaches `lua_tolstring`, which yields a null data pointer, and a null data pointer with a zero
length matches an entity whose name accessor returns null — in practice nothing, so the call
returns `nil`.

## Coverage

| address | coverage |
| --- | --- |
| `0088B1B0` | complete, `0088B1B0..0088B30A` |
| `00898E30` | complete for the lookup and the result handling; the SEH-state stores and the `"luaMW_FindEntity failed:"` one-time string init at `00898E30..00898EF0` are not modelled |
| `00484540` | complete, read as the producer of the list layout |
| `00922FD0`, `00903670`, `00903610` | read only far enough to establish the flag-byte meanings |

## Open questions

- `+60h` has no producer in this packet; the gate is real but its meaning is unread.
- Six of the fourteen kinds are runtime classes with no row in the scene class table; naming them
  needs the constructors at `0084C920`, `0074DCC0`, `00745940`, `006F5AE0`, `006D3B10`, `00848080`.
- Whether an entity can appear on two walked buckets at once (and so be found under the earlier
  kind) was not checked; no observed registrar writes two walked buckets.
