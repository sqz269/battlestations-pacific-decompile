# Mission entity Lua self table: who attaches it, and how

Addresses: 00928A00 0077E830 00928C80 00928F50 00779AF0 00951F30 00CE6290
004426C0 006D0C80 00743450 00743E90 0074CCF0 007CDF20 007F4580 00810F60 008429E0

Files: `include/bsp/mission_entity_lua_attach.hpp`, `src/mission_entity_lua_attach.cpp`,
`reports/mission_entity_lua_attach.json`.

Every descriptive name is a hypothesis, not a recovered symbol. Two things here *are* read out
of the image and are not hypotheses: the `M...` class names, which come from the descriptor
strings already recorded in `docs/VEHICLE_CLASS_DESCRIPTORS.md`, and the Lua field names
`ID`, `Dead`, `Ptr`, `LastPosition`, `x`, `y`, `z`, `Race`, `Party`, which are the string
literals the routines push.

## The headline: it is a virtual slot, not a call graph

`docs/MISSION_LUA_SELF_TABLE.md` recorded `00928A00` as a `__thiscall` with the sole caller
`0077E830`. That is true of direct calls and misleading about the shape. The dword at
**`00CE6290 + 9Ch` is `00928A00` itself**: the root entity vtable installs it as virtual slot 39,
and `0077E830` is a *derived override of the same slot* that calls the base implementation
directly at `0077E834` before doing anything else.

Reading the same vtable at two more offsets pins the whole contract:

| Slot | Offset | Base implementation | Role |
| --- | --- | --- | --- |
| 11 | `+2Ch` | `00928F50` | set party and race, mirrored into the self table |
| 32 | `+80h` | `00928C80` | on-killed: null `Ptr`, record `LastPosition` |
| 39 | `+9Ch` | `00928A00` | attach or refresh the self table |

The slot is uniform. Every one of the 43 vtables that carries an attach carries it at `+9Ch`,
every party/race at `+2Ch` and every on-killed at `+80h`. The check is mechanical: for each of
the 43 data references to an attach function, the address minus `9Ch` is a vtable start (an
address code stores into an object), and reading `+2Ch` and `+80h` of that same start yields a
party/race and an on-killed function every time. `00951F30` and `00951FB0` are the adjustor
thunks the multiple-inheritance classes install in place of the base pair.

So the question "which mission entities get a Lua self table" has a structural answer: **every
class whose vtable slot 39 is one of these ten functions**, which is every class in the table
below plus the 17 that take `0077E830` unchanged.

## `00928A00`, the base attach

`__thiscall(ECX = entity)`, `RET`, body `00928A00..00928C79`. Coverage: **complete**.

The field-by-field sequence is already in `docs/MISSION_LUA_SELF_TABLE.md` and is not restated.
What this packet adds is the branch that document left unread and the ordering around it.

### The kind-3 branch is a skip, not an alternative path

```
00928a9f: CALL 0x00927b40            ; self = thisTable[key]
00928aa4: MOV EAX,dword ptr [EDI + 0xc0]
00928aaa: XOR EBP,EBP
00928aac: CMP EAX,EBP
00928ab2: JZ  0x00928abe             ; no descriptor -> normal path
00928ab4: CMP dword ptr [EAX + 0x4],0x3
00928ab8: JZ  0x00928bee             ; kind 3 -> straight to Ptr
00928abe: LEA ECX,[ESP + 0x20]
00928ac2: CALL 0x00b65fb0            ; BSP_LuaObject_IsNil
```

`00928BEE` is inside the `Ptr` sequence that the fall-through also reaches (`00928BEE PUSH EBX`,
`00928BEF PUSH 0x3`, the three-character name, then `00928C2F CALL 00B67530`). So when the
entity has a descriptor of kind 3 the routine jumps over, in one hop:

- the `IsNil` test,
- the stale-slot clear (`00928AFF`),
- the fresh-table assign (`00928B53`),
- `ID` (`00928BA5`) and `Dead` (`00928BC7`).

A script-owned entity therefore **keeps the Lua object it already has** and only has `Ptr`
rebound to the live native pointer. Its `ID` and `Dead` are whatever a previous attach or a
script left there. The key at `+178h` is still refreshed either way, because the formatting and
the cache (`00928A2F`, `00928A54`, `00928A6A`) run *before* the test.

That is the piece the six failing probe scripts in `docs/MISSION_LUA_MACHINE.md` need: an entity
re-attached in a later pass must not lose its table, and an entity created fresh must get one
with `ID` and `Dead` seeded.

Kind 1 is the only other value read anywhere in this packet, and it is read by `0077E830`, not
here. `00928A00` treats a null descriptor and a descriptor of any kind other than 3 identically,
which is why the reconstruction keeps `descriptor_present` separate from `descriptor_kind`.

## `0077E830`, the override the nine callers use

`__thiscall(ECX = entity)`, `RET`, body `0077E830..0077E88D`. Coverage: **complete**. No stack
arguments: the prologue is `PUSH ECX; PUSH ESI; MOV ESI,ECX` and the epilogue is
`POP ESI; POP ECX; RET` with no immediate, so nothing is popped for a caller.

```
0077e834: CALL 0x00928a00            ; the base slot-39 attach, ECX unchanged
0077e839: MOV EAX,dword ptr [ESI + 0xc0]
0077e841: JZ  0x0077e88b             ; no descriptor -> done
0077e847: JNZ 0x0077e88b             ; kind != 1 -> done
0077e849: FLDZ                       ; the default, pushed before the name
0077e84c: MOV ECX,dword ptr [EAX + 0x8]
0077e857: CALL 0x0048f5d0            ; property "ResourceUsage" at 00CFACAC
0077e864: FST  float ptr [ESI + 0x304]
0077e86e: FUCOMIP ST0,ST1            ; against FLDZ
0077e873: TEST AH,0x44
0077e876: JNP 0x0077e88b
0077e886: CALL 0x0043d870            ; same name, same ECX, reloaded from +C0h
```

Two details the pseudocode hides. The float comparison is `FUCOMIP` plus `TEST AH,0x44` plus
`JNP`: `TEST` isolates ZF and PF, and an *unordered* compare sets PF, so `JNP` does not skip.
A NaN stored at `+304h` therefore reaches `0043D870`; only an ordered equality with zero skips
it. And `0077E878` reloads `[ESI+0xC0]` rather than reusing `EAX`, so the second call's `this`
is read a second time.

The gate value `1` at `descriptor+4h` is a different kind from the `3` that `00928A00` tests, on
the same field of the same descriptor object.

## The nine callers

All nine are themselves slot-39 overrides, and all nine call `0077E830` with an
`UNCONDITIONAL_CALL`, which calls `00928A00` unconditionally at its first instruction. **All
nine reach the base attach**; there is no kind of entity behind them that is skipped.

Each class below is established by the same chain, which is proof rather than proximity: the
class's descriptor slot `+28h` allocator, recorded in `docs/VEHICLE_CLASS_DESCRIPTORS.md`, calls
exactly one function whose body contains the store of the vtable in question, and that vtable's
`+9Ch` is the override.

| Override | Call site | Classes (vtable) | Reaches `00928A00` |
| --- | --- | --- | --- |
| `004426C0` | `004426DB` | unnamed weapon/device class (`00CFDC58`, constructor `00728960`) | yes |
| `006D0C80` | `006D0C99` | `MAirfield` (`00CF8C08`) | yes |
| `00743450` | `0074347E` | unnamed land convoy (`00CEA570`, constructor `004F2410`) | yes |
| `00743E90` | `00743EA9` | `MCommandBuilding` (`00CFB028`), `MLandFort` (`00CFF3F8`) | yes |
| `0074CCF0` | `0074CCF3` | `MLandVehicle` (`00CFFDE0`) | yes |
| `007CDF20` | `007CDF24` | `MReconPlane` (`00D00070`), `MLargeReconPlane` (`00D00308`), unnamed (`00D05F20`), `MPlaneBomber` (`00D06638`), `MPlaneFighter` (`00D06920`), `MSmallReconPlane` (`00D0BA80`), `MPlaneDiveBomber` (`00D19D28`), `MPlaneTorpedoBomber` (`00D1A000`), `MPlaneKamikaze` (`00D1A2D8`) | yes |
| `007F4580` | `007F45A7` | plane squadron, unnamed (`00D087C0`, constructor `007F2C60`) | yes |
| `00810F60` | `00810F7B` | `MBattleship` (`00CF90B0`), `MCargo` (`00CFA778`), `MCruiser` (`00CFB738`), `MDestroyer` (`00CFC3D0`), `MLandingShip` (`00CFFA30`), `MMothership` (`00D01630`), unnamed (`00D09678`), `MSubmarine` (`00D0BF80`), `MTorpedoBoat` (`00D0C648`) | yes |
| `008429E0` | `008429F9` | `MShipyard` (`00D0B770`) | yes |

Three of the nine are not vehicle classes and are named only provisionally:

- `004426C0` reads the Lua properties `platform` and `_device` through the `BSP_LuaReader_*`
  family. Its vtable `00CFDC58` is written only by `00728960`, whose sole caller `0072E510`
  sits in the gun segment among `barreldelaytime` and `nextfirebarrel`. That is consistent with
  the `DeviceClass`-keyed `Class` setter at `00440E10` in `docs/MISSION_LUA_SELF_TABLE.md`,
  which sits in the same segment 3, but the class name was not recovered.
- `00743450` reads `Columns`, `Rows`, `RowGap`, `ColumnGap`, `Reverse`, `Offset`, `Speed`,
  `Position` and `Type`: a formation. Its vtable `00CEA570` is written by `004F2410`, called
  from `004F2700` in the segment whose keywords include `landconvoy`.
- `007F4580` belongs to the object `BSP_SceneUnit_CreatePlaneSquadronGen` (`004F0AD0`) allocates
  through `007F2C60`, which `docs/SCENE_UNIT_CREATORS.md` already describes as the 0x414-byte
  squadron. The body reads `PlaneParentID`, `WingCount`, `classIndex` and `_planeSquadron`.

Seventeen further vtables install `0077E830` itself at `+9Ch` with no override of their own:
`00CEA218 00CF9438 00CFBA80 00CFC698 00CFC910 00CFCB48 00CFCD60 00CFD018 00D03E80 00D040B8
00D05060 00D054D0 00D090E8 00D0C3E8 00D0D130 00D0DF70 00D1A698`. **Partial coverage**: none of
these seventeen was named, and neither were `00D05F20` or `00D09678`.

## `00928C80`, the other half of the contract

`__thiscall(ECX = entity)`, `RET`, body `00928C80..00928F44`, root slot 32. Coverage:
**partial** — the Lua half and the log line are read; `00928F1C..00928F44` is not.

It is not a release of the self table. Under the entity lock it does two things and leaves the
table in place:

| Step | Address | Effect |
| --- | --- | --- |
| gate | `00928C9B` | the whole Lua half runs only when the cached key at `+178h` is non-empty |
| lock | `00928CAA`, `00928CC3` | `004C1570` singleton, `EnterCriticalSection` on holder`+4h`, recursion counter at `+18h` by hand |
| `Ptr` | `00928D04` | `00B67530` with a literal `0`: `Ptr` becomes `lightuserdata(NULL)`, not `nil` |
| `LastPosition` | `00928D49`, `00928D81` | `00B67580` new table, then `00B67800` to bind it |
| pose | `00928D97` | `00414DB0` first, but only when the byte at `+C8h` is zero |
| `x`, `y`, `z` | `00928DE6`, `00928E32`, `00928E7E` | `00B67400` over `+FCh`, `+100h`, `+104h` |
| log | `00928F14` | `004254B0 "Entity %u killed: %s (%s)"`, outside the lock and outside the gate |

`Ptr` set to a null light userdata rather than to nil matters for the reverse direction:
`00888AA0 BSP_ObjectHandle_FromLuaTable` reads `table["Ptr"]` and converts it with
`00B662D0 BSP_LuaObject_ToUserdata`, so after a kill it gets a null pointer from a live field
rather than a missing field.

**Nothing here sets `Dead` to true.** `00928BC7` seeds it false on attach and this routine never
writes it. Whatever writes it is outside the three slots this packet read.

The direct caller `00779AF0` is an override of the *same* slot 32 for player-controllable units.
It ends with a tail jump, not a call:

```
00779b55: MOV ECX,EBX
00779b57: POP EBX
00779b58: JMP 0x00928c80
```

`tools/verify_report_calls.py` rejected an earlier row in the report that claimed a call at
`00779B58`; the row is now recorded under `tail_jumps`. The entity reaches the base in `ECX`
and `00928C80` returns to `00779AF0`'s caller.

## `00928F50`, party and race

`__thiscall(ECX = entity, three stack arguments)`, `RET 0Ch`, body `00928F50..00929090`, root
slot 11, reached through the adjustor thunk `00951F30` in every class with a second vptr.
Coverage: **complete**.

```
00928f7d: CALL 0x00923b80            ; the base, with the three arguments forwarded
00928f89: CALL 0x00927b40            ; self = thisTable[key]
00928fc7: MOV EDX,dword ptr [ESI + 0x58]
00928fd9: CALL 0x00b67460            ; self["Race"]  = that dword
00929034: MOV EDX,dword ptr [ESI + 0x54]
00929046: CALL 0x00b67460            ; self["Party"] = that dword
```

The mirror reads the *entity fields* after the base call has written them, not the arguments, so
`Race` and `Party` in the self table are always what the entity holds and never an argument the
base might have rejected. `RET 0Ch` is where the three-argument count comes from; the arguments
were not attributed individually, which is why the reconstruction forwards them opaquely.

The relationship to the attach is one-directional: the self table must already exist, because
`00927B40` looks it up by the key at `+178h`. Party and race changes before the first attach are
lost to Lua.

## What the executable needs

Reconstructed in `src/mission_entity_lua_attach.cpp` as four routines over one host interface
with one virtual method per native call site, plus three pure rules: the kind-3 skip, the kind-1
ResourceUsage gate, and the unordered float test. The key conversion reuses
`mission_entity_self_key` from `include/bsp/mission_lua_bindings.hpp` rather than declaring a
second rule, and `LuaObject` comes from `include/bsp/lua_object.hpp`.

For an entity created by the Instantiate pass or by a Spawn binding to have the `thisTable` self
object the probe scripts wait for, the host has to reach slot 39 for it. That is the whole
requirement: the key comes from the u16 network id at `+174h`, and `ID`, `Dead` and `Ptr` follow
from the base routine.

Not a drop-in replacement: this is a new C++ contract over the call sites, not the native ABI.
Build-tested only.

## Uncertainties

- `00927050` (`00928A1E`), `0043D870` (`0077E886`), `00414DB0` (`00928D97`), `00923050`
  (`00928F30`) and `00923B80` (`00928F7D`) were not read. They are named by address in the
  report's host table and carry `contract: unread`; no verb was invented for any of them.
- Nothing found sets `Dead` to true.
- The class names of `00CFDC58`, `00CEA570`, `00D05F20`, `00D09678` and the seventeen
  no-override vtables were not recovered.
- `00928C80`'s tail past the log line, and `00928F50`'s three arguments, were not attributed.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_entity_dead_flag` | 00928bc7 00888aa0 00b673a0 | docs/MISSION_ENTITY_DEAD_FLAG.md | What writes `self.Dead = true`, given that the on-killed slot does not |
| `mission_entity_unnamed_classes` | 00cfdc58 00cea570 00d05f20 00d09678 | docs/MISSION_ENTITY_LUA_ATTACH.md | The four unnamed attach vtables and the seventeen that take `0077E830` unchanged |
| `entity_property_reader` | 0048f5d0 0043d870 00927050 | docs/ENTITY_PROPERTY_READER.md | The named-property read and apply pair `0077E830` uses, and the attach preamble |
