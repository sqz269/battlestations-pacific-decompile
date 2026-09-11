# Unit instance layout (the 0x1188 object every ship-class scene entity becomes)

Addresses: 006fe590, 006fe460, 0081ed40, 0095cc90, 0087b670, 0077eed0, 00928630, 00925ce0,
009553d0, 006dfe40, 00928860, 009258f0, 006fe620, 00928560, 006fe670, 00875890, 008531a0,
006eb290, 00749150, 00956390, 009564e0, 00956240

`docs/SCENE_UNIT_CREATORS.md` established that the vehicle-class descriptor's vtable slot `+28h`
allocates the unit instance and that `MDestroyer`'s is 0x1188 bytes with vtable `00CFC3D0`. This
packet reads the constructor chain behind that allocation and the two routines that place the
instance in the world. Every name below is a hypothesis, not a recovered symbol.

The headline result: **`006FE460` is not the constructor**. It is the last 98 bytes of a
six-level chain. `006FE460` installs eight vtable pointers, writes one byte-sized class id and
returns; everything else in the 0x1188 bytes is written by five base constructors, and the
deepest of them (`00925CE0`) is a scene-graph node, not a vehicle.

## The constructor chain

`006FE590` (`descriptor->vtable[28h]`) is `__thiscall(descriptor, int flag)`, `RET 4`, SEH scope
table `00C83AB8`. `ECX` is saved in `EDI` at `006FE5AD` and is the only use of the descriptor;
`flag` is the creator's literal `0` read back from `[ESP+1Ch]` at `006FE5D6`.

```
006fe5af  00BF55BE operator new(1188h)          ; PUSH 1188h, cdecl, ADD ESP,10h at 006fe5c3
006fe5be  00BF79F0 memset(instance, 0, 1188h)
006fe5dd  006FE460(instance, flag)              ; only when operator new returned non-null
006fe5f3  009553D0(instance, descriptor)        ; ECX = instance, PUSH EDI
006fe5fc  MOV [ESI+354h], EDI                   ; the second, non-owning back-pointer
```

The zero test at `006FE5CC` is real: on a null allocation `ESI` is zeroed at `006FE5E6` and
`009553D0` is still called with `ECX = 0`, which faults. That is the shipped code, not a
reading error.

| Level | Routine | `this` | Writes `+C4h` | Calls up |
| --- | --- | --- | --- | --- |
| 6 | `006FE460` | instance | `7` at `006FE4B3` | `0081ED40` at `006FE468` |
| 5 | `0081ED40` | instance | `6` | `0095CC90` at `0081ED6A` |
| 4 | `0095CC90` | instance | `5` | `0087B670` at `0095CCB3` |
| 3 | `0087B670` | instance | `4` | `0077EED0` at `0087B694` |
| 2 | `0077EED0` | instance | `2` | `00928630` at `0077EEF9` |
| 1 | `00928630` | instance | `1` | `00925CE0` at `00928651` |
| 0 | `00925CE0` | instance | `0` | `00925490` (weak owner at `+24h`) at `00925D34` |

The dword at `+C4h` is written once per level with the level's own depth, so the final value
names the most derived class. `include/bsp/unit_instance.hpp` already recorded `+C4h` as
`kUnitOffClassId` with the value 7 for `MDestroyer`; this packet adds that `3` is skipped (no
level writes it) and that the chain, not the creator, is what produces the value.

Each level also rewrites the same eight vtable pointers, so the eight are one multiple-inheritance
vptr set carried all the way down:

| vptr offset | `00925CE0` | `0077EED0` | `0087B670` | `0095CC90` | `0081ED40` | `006FE460` |
| --- | --- | --- | --- | --- | --- | --- |
| `+0h` | `00CECCC8` then `00D19120` | `00D03E80` | `00D0DF70` | `00D1A698` | `00D09678` | `00CFC3D0` |
| `+10h` | `00CE3CD4` then `00D19104` | `00D03E68` | `00D0DF54` | `00D1A680` | `00D0965C` | `00CFC3B8` |
| `+24h` | `00D190FC` | `00D03E60` | `00D0DF4C` | `00D1A678` | `00D09654` | `00CFC3B0` |
| `+170h` | — | `00D03E5C` | `00D0DF48` | `00D1A674` | `00D09650` | `00CFC3AC` |
| `+1E4h` | — | `00D03E54` | `00D0DF40` | `00D1A66C` | `00D09648` | `00CFC3A4` |
| `+310h` | — | — | `00D0DF28` | `00D1A654` | `00D09630` | `00CFC38C` |
| `+38Ch` | — | — | — | `00D1A650` | `00D0962C` | `00CFC388` |
| `+72Ch` | — | — | — | — | `00D09628` | `00CFC384` |

`0077EED0` also parks `&PTR___purecall_00D03D98` at `+1E4h` before its array construction and
replaces it with `00D03E54` afterwards (`0077EF04`..`0077EF60`), the standard vtable-during-
construction pattern.

`0081ED40`'s highest write is `param_1[0x461]` = `+1184h`, four bytes short of the 0x1188
allocation, so level 5 owns the whole object and `MDestroyer` (level 6) adds no fields at all.
`Cargo`'s 0x118C and `Submarine`'s 0x1288 are the same base plus their own tails.

## Layout

Offsets are from the instance base. "Writer" is the instruction that writes the field during
construction, or the routine that writes it afterwards. Fields the existing
`include/bsp/unit_instance.hpp` already names are marked with their constant.

| Offset | Size | Meaning | Writer | Value |
| --- | --- | --- | --- | --- |
| `+0h` | 4 | primary vptr | six levels, last `006FE46D` | `00CFC3D0` |
| `+4h`..`+0Ch` | 12 | zeroed by level 0 | `00925CE0` | 0 |
| `+10h` | 4 | vptr 2 | `006FE473` | `00CFC3B8` |
| `+14h`..`+1Ch` | 12 | zeroed | `00925CE0` | 0 |
| `+20h` | 1 | byte | `00925CE0` | 0 |
| `+24h` | 4 | vptr 3, also the weak-owner sub-object base | `00925D34` then `006FE47A` | `00CFC3B0` |
| `+30h` | 4 | world node (parent), `kUnitOffParentNode` | `00925906` in `009258F0` | the creator's `*(*(00E188A8)+19CCh)` |
| `+34h` | 4 | world chain prev, `kUnitOffSiblingPrev` | `0092591E` | previous tail |
| `+38h` | 4 | world chain next, `kUnitOffSiblingNext` | `00925929` | 0 |
| `+3Ch` | 4 | hierarchy parent (the creator's `hierarchyParent`) | `00925935` | may be 0 |
| `+40h` | 4 | hierarchy prev | `00925948` / `0092595C` / `00925982` | previous tail |
| `+44h` | 4 | hierarchy next | `0092594E` / `00925962` / `00925988` | 0 |
| `+48h` | 4 | own child list head | `00925CE0` (0), children on attach | 0 |
| `+4Ch` | 4 | own child list tail | `00925CE0` (0) | 0 |
| `+50h` | 4 | own child count | `00925CE0` (0) | 0 |
| `+54h` | 4 | dword | `00925CE0` | 2 |
| `+58h` | 4 | dword | `00925CE0` | `FFFFFFFFh` |
| `+5Ch` | 1 | live gate, `kUnitOffActiveByte` | `00925CE0` (0) | 0 at construction |
| `+5Dh` | 1 | `kUnitOffSimulateByte` | `00925CE0` | 0 |
| `+5Eh` | 1 | `kUnitOffDeadByte` | `00925CE0` | 0 |
| `+5Fh` `+60h` `+61h` `+62h` | 1 each | further gate bytes | `00925CE0` | 0 |
| `+64h` | 4 | dword | `00925CE0` | 0 |
| `+68h` | 1 | byte | `00925CE0` | 0 |
| `+6Ch` `+70h` | 4 each | dwords | `00925CE0` | 0 |
| `+74h` | 64 | local 4x4 matrix | `00925DCA` and `00925EA0` (identity), then `009259C4` | identity, then the creator's `localFrame` |
| `+B4h` `+B8h` | 4 each | deferred pair flushed on attach | `009259AA` / `009259B0` | cleared on attach |
| `+BCh` | 1 | attached-once gate | `009258F6` reads, `009259EE` sets | 0, then 1 |
| `+BDh` | 1 | byte | `00925CE0` | 0 |
| `+C0h` | 4 | property-bag reference holder | `0046DB9B` / `0046DBE0` (`docs/SCENE_ENTITY_CREATE.md`) | 0 at construction |
| `+C4h` | 4 | class id, `kUnitOffClassId` | six levels, last `006FE4B3` | 7 |
| `+C8h` | 1 | `kUnitOffPoseValid` | `009259CE` clears on attach | 0 |
| `+10Ch` | 1 | byte cleared on attach | `009259D4` | 0 |
| `+150h` | 4 | dword | `00925CE0` | 0 |
| `+154h` | 4 | name length, `kUnitOffNameLength` | `00925DD5`, `0041DD40` | 0 |
| `+158h` | 4 | name buffer pointer | `00925DD7`, `0041DD40` | 0 |
| `+15Ch` `+160h` | 4 each | the rest of the string object | `00925CE0` | 0 |
| `+168h` `+16Ch` | 4 each | dwords | `00925CE0` | 0 |
| `+170h` | 4 | vptr 4 | `006FE481` | `00CFC3AC` |
| `+174h` | 2 | u16 entity id (Lua self key, message sender id) | `009286ED` | `009517C0(registry, flag, this)` |
| `+178h` `+17Ch` | 4 each | dwords | `00928630` | 0 |
| `+180h` | 4 | role/party slot | `00928707` | 9 |
| `+184h` | 1 | byte | `00928701` | 0 |
| `+1ACh`..`+1CCh` | 4 x 9 | nine dwords | `00928630` | 8 each |
| `+1D0h` | 4 | dword from `00F876A4` | `00928630` | global |
| `+1E4h` | 4 | vptr 5 | `006FE48B` | `00CFC3A4` |
| `+310h` | 4 | vptr 6, also the fixed-step tick node base | `00875890` then `006FE495` | `00CFC38C` |
| `+324h` | 4 | tick group index (node `+14h`) | `00875890` (0), then `0081ED40` | 1 |
| `+338h` | 4 | tick payload (node `+28h`) | `0087B6A9` | the instance itself |
| `+348h`..`+350h` | 4 each | dwords | `0087B670` | 0 |
| `+354h` | 4 | non-owning class back-pointer, `kUnitOffDescriptor` | `006FE5FC` | the descriptor |
| `+35Ch` | 4 | dword | `0087B670` | `FFFFFFFFh` |
| `+364h` | 4 | float from `00D0E12C` | `0087B704` | global |
| `+374h` | 4 | dword | `0087B70C` | `3Fh` |
| `+380h`..`+388h` | 4 each | dwords | `0087B670` | 0 |
| `+38Ch` | 4 | vptr 7 | `006FE49F` | `00CFC388` |
| `+394h` | 12 x 12 | array, element ctor `00952640` | `0095CC90` | — |
| `+538h` | 4 | owning class reference, `kUnitOffClassBlock` | `0095CC90` (from its third argument, `0`), then `009553D0` | 0, then the descriptor |
| `+53Ch` | 24 x 10 | array, element ctor `004C2D40` | `0095CC90` | — |
| `+72Ch` | 4 | vptr 8, also the `00809270` sub-object | `0081ED7B` then `006FE4A9` | `00CFC384` |
| `+838h` | — | order ring, `BSP_UnitOrderRing_Construct` | `0081EE95` | see `docs/UNIT_STATE_MESSAGE.md` |
| `+A00h` `+A20h` `+A98h` `+AECh` `+B44h` `+B54h` `+BD0h` `+10D4h` | — | further sub-objects | `0081ED40`, sites below | — |
| `+109Ch` | 1 | round-robin slot from the global `00F87151` | `0081F1A8` | `0..11`, the global then wraps |
| `+10A0h` | 4 | sub-object vptr | `0081ED40` | `00D09624` |
| `+1018h` | 4 | controller, `kUnitOffController` | `008255B0` path, not the constructor | 0 at construction |

`+838h` is confirmed independently: `docs/UNIT_STATE_MESSAGE.md` records the throttle slew at
`ring+154h` = `unit+98Ch`, and `838h + 154h = 98Ch`.

The `+109Ch` byte is the only construction-time global side effect: `0081ED40` stores the current
`00F87151`, increments it, and resets it to 0 once it exceeds 11. Twelve instances therefore cycle
through twelve slot values.

## What `009553D0` does

`BSP_Unit_SetVehicleClass`, `__thiscall(instance, descriptor)`, `RET 4`, body
`009553D0..0095540E`. A refcounted setter for `+538h`:

1. `009553DB` early-outs when the new value equals the old one, so the field is never re-addrefed.
2. `009553E1` stores the new pointer, then `009553ED` calls `[00CE221C]` with `descriptor+4`:
   the refcount sub-object is at `+4h` of the descriptor, not at `+0h`.
3. `009553FB` calls `[00CE2220]` with `old+4`; when that returns zero, `0095540B` calls the old
   descriptor's `vtable[0]` with `ECX = old` and **no stack argument**, so slot 0 here is a plain
   virtual release, not a scalar deleting destructor taking a flag.

Nineteen functions call it out of line and three more (`00956390`, `009564E0`, `00956240`) inline
the same six instructions; see the creator table below.

## Placement: `vtable[98h]` -> `006DFE40` -> `00928860` -> `009258F0`

`006DFE40` is a five-byte `JMP 00928860`. `00928860` is `__thiscall(instance, hierarchyParent,
worldNode, matrix)`, `RET 0Ch`, SEH scope table `00CA6FE8`. Argument slots are `[ESP+28h]`,
`[ESP+2Ch]` and `[ESP+30h]` after the `EBX`/`EBP` pushes at `0092889F`; `EBP` holds the world node.

```
0092887C  00928240()                    ; registry singleton; EDI = [ret+4], a CRITICAL_SECTION*
00928895  [00CE2218](EDI)               ; enter, then [EDI+18h] += 1
009288C7  instance->vtable[134h]()      ; only when worldNode != [instance+30h] and the old is non-null
009288D6  009258F0(instance, hierarchyParent, worldNode, matrix)
009288F1  instance->vtable[130h]()      ; only when the node changed and [instance+30h] is now non-null
009288FC  [00CE2210](EDI)               ; leave, after [EDI+18h] -= 1
```

`BL` is the "node changed" predicate, set once at `009288AA` and reused at `009288DC`, so the
leave-old and enter-new hooks are driven by the same comparison. The same critical section and the
same `+18h` counter appear in `00928630`, where Ghidra resolved the imports as
`EnterCriticalSection` / `LeaveCriticalSection`, which is how `[00CE2218]` and `[00CE2210]` are
identified here.

`009258F0`, `__thiscall(instance, hierarchyParent, worldNode, matrix)`, `RET 0Ch`, body
`009258F0..009259FA`, is the actual attach and it is **gated once**: `009258F6` returns
immediately when the byte at `+BCh` is non-zero, and `009259EE` sets that byte. So a second
`00928860` with a different world node runs the two vtable hooks but changes nothing else unless
`vtable[134h]` clears `+BCh` first. In order:

1. `+30h = worldNode`. The world node's child list object is at `worldNode+4h`, shaped
   `{head +0h, tail +4h, count +8h}`; the instance is appended through `+34h`/`+38h`.
   `docs/GAME_WORLD_ENTITIES.md` reaches the same list from the other end: `00904BF0` walks
   `[world+4]` through `entity+38h` and gates on `entity+5Ch`.
2. `+3Ch = hierarchyParent`. When it is non-null the instance is appended to that parent's
   `{head +48h, tail +4Ch, count +50h}` list through `+40h`/`+44h`; when it is null the same
   link fields are used to append to the list object at `[worldNode+8h]` instead. The two
   linkages are independent: a unit is always in the world chain and additionally in exactly one
   hierarchy list.
3. `+B4h`/`+B8h`, when either is non-zero, are passed to `009245A0(instance, [+B8h], [+B4h])`
   and cleared first.
4. `004134F0` copies the 16-float `matrix` into `+74h`.
5. `+C8h` and `+10Ch` are cleared, then every child on the `+48h` list is walked through `+44h`
   and passed to `0042ED50`.

## World registration: `vtable[130h]` = `006FE620`

`__fastcall(instance)`, no stack arguments, `RET`, body `006FE620..006FE665`. Exported but with
no static caller, because it is only reached through `vtable[130h]`; `00CFC3D0+130h` is
`006FE620`, which is the call at `009288F1`.

```
006fe623  00928560(instance)            ; ECX = [instance+30h] + 24h, one 00484540 push
006fe62f  00484540(parentList +30h, instance)
006fe63b  00484540(parentList +48h, instance)
006fe647  00484540(parentList +54h, instance)
006fe653  00484540(parentList +60h, instance)
006fe65f  00484540(parentList +6Ch, instance)
```

So the registration is **six** lists, not five: `00928560` (body `00928560..0092856C`, three
instructions) adds the one at `parent+24h`. All six are on the world node reached through
`[instance+30h]`, which `009258F0` has just written, which is why the order in `00928860`
matters. `00484540` `BSP_UnitList_PushBack` belongs to the `cc_unit_lists` packet and is cited
here as a contract, not read.

The counterpart `00CFC3D0+134h` is `006FE670`, already named
`BSP_UnitInstance_DetachFromWorldLists`, and it mirrors the registration list for list. Each level
scans its list's head at `list+4h`, compares each node's `+8h` value with the instance and erases
the match through `004837D0`; lists are `{count +0h, head +4h, tail +8h}` and nodes are
`{prev +0h, next +4h, value +8h}`.

| Remover | Lists |
| --- | --- |
| `00928570`, called by `006D3620` at `006D3623` | `parent+24h` |
| `006D3620`, called by `006DFFC0` at `006DFFC3` | `parent+30h`, `parent+48h` |
| `006DFFC0`, called by `006FE670` at `006FE673` | `parent+54h`, `parent+60h` |
| `006FE670` itself | `parent+6Ch` |

`006FE670` does not clear `+BCh`, so the attach-once gate survives a detach. That asymmetry is
recorded, not explained.

`docs/LUA_BINDING_ENTITY_LOOKUP.md` lists `006FE620` as the traced registrar of `FindEntity`
kind `00h` at bucket base `+60h`; `60h` is one of the six list offsets above, which is the link
between the registration and the name lookup.

## `006FE620` had no known caller

The packet brief listed `006FE620` as exported with no known caller. It is
`00CFC3D0+130h`, called indirectly at `009288F1` inside `00928860`. There is no direct call site
in the image.

## The entity id at `+174h`

`00928630` writes it at `009286ED` as a 16-bit store of `AX` from `009517C0`. The call is
selected by the level-2 argument: `0077EEF9` passes the literal `1`, and `009286BC`..`009286D8`
turn that into `ECX = 00F89A08`; the `0` arm would use `00F89A5C`. The creation `flag` and the
instance are pushed, so the call is `009517C0(registry 00F89A08, flag, instance)` returning a u16.
`docs/MISSION_ENTITY_LUA_ATTACH.md` uses this u16 as the Lua self-table key and
`docs/UNIT_STATE_MESSAGE.md` uses its low 12 bits as the network sender id.

Immediately after, `00928630` calls `instance->vtable[148h]` with `(1FFh, 9)`; for `00CFC3D0`
that slot is `0077F360`, which has no Ghidra function (the enclosing candidate is `0077F2D0`) and
was **not read**. `+180h` is set to 9 just before the call, so 9 is the same value in both places.

## The other classes' slot `+28h` creators

Read from each descriptor constructor's `MOV [ESI],<vtable>` and then the dword at
`vtable+28h`. All are `__thiscall(descriptor, int flag)`, `RET 4`.

| `VehicleClass.Type` | descriptor vtable | `+28h` creator | instance size | instance ctor | read |
| --- | --- | --- | --- | --- | --- |
| `Destroyer` | `00D1ACF8` | `006FE590` | `1188h` | `006FE460` | full |
| `Cruiser` | `00D1AD38` | `006FB430` | `1188h` | `006FB300` | entry only |
| `LandingShip` | `00D1AD78` | `0074BE00` | `122Ch` | `0074BB00` | entry only |
| `Cargo` | `00D1ADBC` | `006EB290` | `118Ch` | `006EB160` | full |
| `BattleShip` | `00D1ADF8` | `006DFEF0` | `118Ch` | `006DFC90` | entry only |
| `Submarine` | `00D1AE38` | `008531A0` | `1288h` | `00852F10` | full |
| `TorpedoBoat` | `00D1AE78` | `00857E20` | `1190h` | `00857CD0` | entry only |
| `MotherShip` | `00D1AEBC` | `00758D30` | `12C8h` | `00758550` | entry only |
| `ReconPlane` | `00D19BF4` | `008091D0` | `E94h` | `0074E0F0` | entry only |
| `SmallReconPlane` | `00D1A5A8` | `0084CA50` | `E94h` | `0084C920` | entry only |
| `LargeReconPlane` | `00D1A5EC` | `0074E540` | `E94h` | `0074E2D0` | entry only |
| `Fighter` | `00D19C30` | `007DDAE0` | `E94h` | `007DD9B0` | entry only |
| `DiveBomber` | `00D19C70` | `00956390` | `E94h` | `00951B60` | entry + tail |
| `TorpedoBomber` | `00D19F4C` | `009564E0` | `E94h` | not read | entry only |
| `Kamikaze` | `00D1A224` | `00956240` | `E94h` | not read | entry only |
| `LevelBomber` | `00D1A4F8` | `007D7850` | `E94h` | `007D7720` | entry only |
| `AirField` | `00D1A9A0` | `006D3110` | `8E4h` | `006D1C20` | entry only |
| `Shipyard` | `00D1A9DC` | `00848380` | `7A4h` | `00848080` | entry only |
| `LandVehicle` | `00D1AA18` | `0074DF10` | `740h` | `0074DCC0` | entry only |
| `LandFort` | `00CFF790` | `00747000` | `758h` | `00745940` | entry only |
| `CommandBuilding` | `00D1A538` | `006F5C10` | `7E8h` | `006F5610` | entry only |
| `DummyTargetVehicle` | `00D1AA58` | `00749150` | — | — | full |

`008531A0` (`Submarine`) and `006EB290` (`Cargo`) were read instruction by instruction against
`006FE590`: identical except the size literal, the constructor address and the SEH scope table
(`00C945F8` and `00C82E28`). "Entry only" means the `PUSH <size>`, the `operator new`/`memset`
pair and the constructor call were read from the first 30 instructions and the `009553D0` call
was taken from that routine's caller list; the tails were not read.

Three divergences:

- `00749150` (`DummyTargetVehicle`) is `XOR EAX,EAX ; RET 4`. That class never allocates an
  instance, which is consistent with its single `vehicleclasses.lua` row and its name.
- `00956390`, `009564E0` and `00956240` (`DiveBomber`, `TorpedoBomber`, `Kamikaze`) have the
  refcounted setter inlined instead of calling `009553D0`; `00956390`'s tail was read and is the
  same six instructions on `+538h` with `[00CE221C]` and `[00CE2220]`. They are not in
  `009553D0`'s caller list for that reason alone.
- `00701170` calls `009553D0` and allocates `740h`, but no descriptor vtable read here points at
  it. Which class owns it is an open question.

Eight of the nineteen out-of-line creators construct through `0081ED40` (`006DFC90`, `006EB160`,
`006FB300`, `006FE460`, `0074BB00`, `00758550`, `00852F10`, `00857CD0`), which is exactly the
ship set: `BattleShip`, `Cargo`, `Cruiser`, `Destroyer`, `LandingShip`, `MotherShip`, `Submarine`,
`TorpedoBoat`. The `E94h` aircraft and the `7xxh` structures use a different level-5 base.

## Native call sites

One row per native call site read in this packet.

| Site | Callee | `this` | Arguments | Returns | Gate |
| --- | --- | --- | --- | --- | --- |
| `006fe5af` | `00BF55BE` `operator new` | — | `1188h` | instance or 0 | none |
| `006fe5be` | `00BF79F0` `memset` | — | instance, 0, `1188h` | — | none |
| `006fe5dd` | `006FE460` | instance | `flag` | instance | allocation non-null |
| `006fe5f3` | `009553D0` | instance | descriptor | — | none |
| `006fe468` | `0081ED40` | instance | `flag` | instance | none |
| `0081ed6a` | `0095CC90` | instance | `flag`, `0` | instance | none |
| `0081ed7b` | `00809270` | instance+`72Ch` | — | — | none |
| `0081ee95` | `00812D40` `BSP_UnitOrderRing_Construct` | instance+`838h` | — | — | none |
| `0081ef47` | `0093BCC0` | instance+`A20h` | — | — | none |
| `0081f03d` | `00815600` | instance+`BD0h` | — | — | none |
| `0081f15f` | `0074E7B0` | instance+`10D4h` | — | — | none |
| `0081f200` | `00BF681B` `operator new` | — | `2Ch` | block stored at `+73Ch` | none |
| `0095ccb3` | `0087B670` | instance | `flag` | instance | none |
| `0087b694` | `0077EED0` | instance | `flag` | instance | none |
| `0087b6a9` | `00875890` `BSP_TickRegistration_Construct` | instance+`310h` | instance, `0` | node | none |
| `0077eef9` | `00928630` | instance | `1`, `flag` | instance | none |
| `0077f072` | `0077EB50` | — | `0` | — | `[00E188A8]+1FE4h == 2` and `00E0AF20 == 0` |
| `00928651` | `00925CE0` | instance | — | instance | none |
| `00928695` | `00928240` | — | — | registry singleton | none |
| `009286e4` | `009517C0` | `00F89A08` | `flag`, instance | u16 id | level-2 argument non-zero |
| `00928760` | `00926BE0` | not resolved | — | — | none |
| `00925d34` | `00925490` | instance+`24h` | — | — | none |
| `00925dca` | `004134F0` `BSP_Matrix_Copy4x4X87` | instance+`74h` | stack identity | — | none |
| `00925ea0` | `004134F0` `BSP_Matrix_Copy4x4X87` | instance+`74h` | stack identity | — | none |
| `00925ea9` | `0041DD40` `BSP_NativeString_Resize` | instance+`154h` | `0`, `0` | — | none |
| `0092887c` | `00928240` | — | — | registry singleton | none |
| `009288d6` | `009258F0` | instance | hierarchyParent, worldNode, matrix | — | none |
| `009259b6` | `009245A0` | instance | `[+B8h]`, `[+B4h]` | — | either non-zero |
| `009259c4` | `004134F0` `BSP_Matrix_Copy4x4X87` | instance+`74h` | matrix argument | — | `+BCh == 0` |
| `009259e2` | `0042ED50` | each child | — | — | child list non-empty |
| `006fe623` | `00928560` | instance | — | — | none |
| `00928567` | `00484540` `BSP_UnitList_PushBack` | `[instance+30h]+24h` | instance | — | none |
| `006fe62f` | `00484540` | `[instance+30h]+30h` | instance | — | none |
| `006fe63b` | `00484540` | `[instance+30h]+48h` | instance | — | none |
| `006fe647` | `00484540` | `[instance+30h]+54h` | instance | — | none |
| `006fe653` | `00484540` | `[instance+30h]+60h` | instance | — | none |
| `006fe65f` | `00484540` | `[instance+30h]+6Ch` | instance | — | none |
| `006fe673` | `006DFFC0` | instance | — | — | none |
| `006fe694` | `004837D0` | `[instance+30h]+6Ch` | node | — | node found |

Indirect sites: `009553ED` and `009553FB` through `[00CE221C]` / `[00CE2220]`; `009286AE`, `00928774`, `00928895` and
`009288FC` through `[00CE2218]` / `[00CE2210]`; `009288C7` and `009288F1` through
`instance->vtable[134h]` / `[130h]`; `009286F4` through `instance->vtable[148h]`.

## Coverage

| Routine | Coverage |
| --- | --- |
| `006FE590` | complete |
| `006FE460` | complete |
| `009553D0` | complete |
| `006FE620`, `00928560` | complete |
| `00928860` | complete |
| `009258F0` | complete |
| `006FE670` | complete |
| `00925CE0` | complete for field writes; `00925490`, `004134F0`, `0041DD40` are contracts |
| `00928630` | complete for field writes; `00925CE0`, `00928240`, `009517C0`, `00926BE0` and `vtable[148h]` are contracts |
| `0077EED0` | complete for field writes; the eight callees are contracts |
| `0087B670` | complete |
| `0095CC90` | complete for field writes; the two array constructions are contracts |
| `0081ED40` | partial: every scalar field write is transcribed, but the eight sub-object constructors (`00809270`, `00812D40`, `0093BCC0`, `00815600`, `0074E7B0`, and three vector iterators) and their interior layouts were not read |

## Open questions

- `0077F360` (`vtable[148h]`, called with `1FFh` and `9`) has no Ghidra function and was not read.
  The meaning of `+180h = 9` and of the nine dwords of `8` at `+1ACh`..`+1CCh` is unread.
- `00925CE0` writes the identity matrix to `+74h` twice (`00925DCA` and `00925EA0`) with the same
  `ECX`, established by filtering the whole body for `EBP`, which is loaded once at `00925D5B`.
  Two member initialisations were folded onto one offset, or the second is dead. Not resolved.
- `00928570` (the `parent+24h` remover) was identified from `006D3620`'s first call but its body
  was not read.
- `+BCh` is never cleared by anything read here, so a second placement of the same instance is
  a no-op. Whether any path clears it is unread.
- `00701170` calls `009553D0` but is not any of the 22 descriptor `+28h` slots read here.
- `+60h` still has no producer (`docs/LUA_BINDING_ENTITY_LOOKUP.md` recorded the same gap); the
  constructor only zeroes it.

## Correction to `docs/SCENE_UNIT_CREATORS.md`

That doc says `006FE620` pushes the instance onto **five** lists. It is six: `00928560`, which it
calls first, pushes onto the list at `parent+24h`. The doc also calls `006FE460` "the instance
constructor"; it is the most derived of six, and the fields it writes are eight vptrs and one
class id.

## Follow-up

| Packet | Addresses | Contract |
| --- | --- | --- |
| `unit_instance_base_subobjects` | 00809270 0093bcc0 00815600 0074e7b0 00812d40 | The eight sub-objects `0081ED40` constructs and their interior layouts |
| `unit_entity_id_registry` | 009517c0 00f89a08 00f89a5c | The u16 id allocator behind `+174h` and the two registries |
| `unit_role_slot` | 0077f360 0077f2d0 | `vtable[148h](1FFh, 9)` and the meaning of `+180h` |
