# Unit parts: descriptors, the damage-state instance, and detaching

Addresses: 00934150 007135C0 00958A30 0080E440 00821E80 0080E410 00937C90 0087BCC0 0087BF80
00939CB0 0080DEC0 0072AB10 00935C70

Packet `cc2_unit_parts`, read-only in Ghidra, 2026-09-11. Every descriptive name below is a
hypothesis, not a recovered symbol. This packet continues the part rows of
`docs/UNIT_HIT_PATH.md` and the `00958A30` coverage row of `docs/UNIT_DAMAGE_AND_DEATH.md`.

## Three different "part" objects

The word *part* covers three unrelated records on a unit. Keeping them apart is the main result of
this packet.

| holder | what it is | producer |
| --- | --- | --- |
| `class+18h` | vector of `30h`-byte part descriptors (begin `+1Ch`, end `+20h`) | unread, see below |
| `unit+344h` | vector of `const PartDesc*` pointing into `class+18h`; begin `+348h`, end `+34Ch` | `0087BCC0` |
| `unit+360h` | the `1ACh` damage-state model instance | `007135C0` |
| `unit+1018h` | the unit **motion controller**, which also carries all per-part physics and detach state | `0080DEC0` -> `00939CB0` |

### Correction: `unit+1018h` is the controller, not a separate parts object

`docs/UNIT_DAMAGE_AND_DEATH.md` lists `+1018h` as a "breakable-parts object, per-part health vector
at its `+310h`/`+314h`" with no writer found. The writer is `0080DEC0` (and the two variants
`007497D6`, `00857C66`, found by searching for `MOV [ESI+1018h],EAX`): `operator new(390h)` at
`0080DED7`/`0080DEDE`, then `00939CB0(obj, unit)` at `0080DEF9`, then the store at `0080DF06`. That
constructor is `BSP_UnitController_Construct` of `docs/UNIT_CONTROLLER_UPDATE.md`. So the per-part
health vector, the `0x20`-byte records that document left undecoded at `controller+8Ch`, and the
detach target of `00934150` all live on the motion controller.

### Correction: the parts vector on the unit begins at `+348h`

`docs/UNIT_HIT_PATH.md` says "the vector at `this+344h`". `+344h` is the vector *object*:
`0087BD46` takes `LEA EBP,[ESI+344h]` as the `this` of the resize `0087B460`, and the element
pointers are then read at `[EBP+4]` (`0087BD58`) and `[EBP+8]` (`0087BD5F`), that is `unit+348h`
(begin) and `unit+34Ch` (end), 4-byte elements. `0087BF80`, an unrelated Lua-facing enumerator,
reads exactly the same pair (`0087C070`, `0087C07E`) and confirms it.

## The `30h` part descriptor

`contract: unread`. `0087BCC0` computes the count as `([class+20h] - [class+1Ch]) / 30h` and stores
`[class+1Ch] + i*30h` into the unit's pointer vector; nothing in this packet dereferences one.
`00960230` (the class base reader) and `0087CA80` `BSP_DamageableClass_ReadLuaFields` (which fills
`HP` `+48h` at `0087CC98` and `Armour` `+4Ch` at `0087CCB4`) do not write `+18h`..`+20h`, so the
part-descriptor vector is filled somewhere else, most likely on the model side that also owns
`class+50h`. Finding its producer is the first follow-up below.

## The `1ACh` damage-state instance at `unit+360h`

`007135C0` is `__thiscall(obj, unit, partSet)`, body `007135C0..00713723`, called from eight sites;
`0087BCC0` is one of them (`0087BE58`, `0087BE9B`). The fields it writes:

| offset | written | meaning |
| --- | --- | --- |
| `+0h` | `PTR_LAB_00CE89E8`, then `PTR_LAB_00CFD7B8` | base vptr, then the concrete vptr |
| `+4Ch` | `unit`, rewritten from `+164h` at the end | owner |
| `+160h` | `partSet` (`[class+50h]->vtable[8](unit->vtable[190h](), lod)`) | selected part set; two stack arguments, see `NATIVE_UNIT_HEALTH_PARTS.md` |
| `+164h` | `unit` | owner |
| `+16Ch`/`+170h`/`+174h` | zero | vector of `10h`-byte damage states (the `>> 4` is in `0072AB10`) |
| `+17Ch` | `007103A0()` | allocated sub-object |
| `+184h` | byte 0 | flag |
| `+18Ch`, `+198h` | `007103C0()` | two more allocated sub-objects |
| `+180h`, `+190h`, `+19Ch`..`+1A8h` | zero | unread |

The damage-state vector is what the health path drives. `0072AB10(unit)` reads `unit+360h`, and
returns `(([inst+170h] - [inst+16Ch]) >> 4) - 1` when that vector is non-empty and `0` otherwise, so
it is "the index of the last damage state". `00958A30` passes that value to `vtable[1B4h]` when a
sub-object dies and `0` when it is repaired, so `vtable[1B4h]` selects the damaged model variant.

`0087BCC0`'s tail `0087BEAA..0087BF73` (unread in the hit packet) belongs here: it resolves a value
through `00876EC0`/`008F2260` into `unit+35Ch` and, when `unit+360h` is set and the unit answers
`IsKindOf(6)` or `IsKindOf(1Bh)`, calls `00711BE0(unit+360h, value)` at `0087BF43`; a failure path
nulls `unit+360h` at `0087BF5B`. The value's meaning is `contract: unread`.

## The controller's per-part state

`00939CB0` allocates `390h` bytes and runs
`eh_vector_constructor_iterator(this+8Ch, 20h, 14h, 009318D0, 0092FD50)`: **20** records of `20h`
bytes at `+8Ch..+30Bh`. `00934150` indexes them with `SHL EAX,5` / `LEA EDI,[EAX+ESI]` at
`0093417A` and then reads `[EDI+90h]`, so record `i` starts at `controller+8Ch + i*20h`.

| offset | type | meaning | writer | reader |
| --- | --- | --- | --- | --- |
| record `+4h`/`+8h`/`+Ch` | vector | scene nodes owned by the part | `00937C90` | `00934180`, `0093418A` |
| record `+14h`/`+18h`/`+1Ch` | vector of int | slot indices into the unit's effect array | `00937C90` | `009341E0`, `009341EE` |
| `+8h`/`+Ch` | vector of node ptr | every hull node, walked in the detach tail | `00937C90` | `00935367` |
| `+1Ch` | ptr | the owning unit | `00939CB0` | `00934114`, `0093428E` |
| `+2Ch` | ptr | hull rigid body | `docs/UNIT_CONTROLLER_UPDATE.md` | - |
| `+310h`/`+314h`/`+318h` | vector of float | per-part health, one entry per part | `00937C90` (`009383C9`, `009383FC`) | `00935C80` (`00935C70`) |
| `+320h`/`+324h` | vector of `10h` | debris groups; each `{?, begin +4h, end +8h, cap +Ch}` of `14h`-byte entries | `00937C90` | `009343DA` on |
| `+330h`/`+334h` | vector of `10h` | second group list, same shape | `00937C90` (`0093919D`) | `009351xx` |
| `+340h`/`+344h` | vector of `10h` | third group list, same shape | `00937C90` | `0093438A` on |
| `+34Ch + i` | signed byte | group slot for part `i`, `-1` when the model has no nodes for it | `00937C90` at `0093813A` (`FFh`) and `009381B5` | `00934356` |

`00937C90` `BSP_UnitController_BuildSubObjectsAndHullBody` is the producer for all of them. It looks
model nodes up by name through `[[controller+1Ch]+4A4h]`: the format strings it pushes are
`"fizika_%02d"` (`00CEB90C`, at `0093804E`), `"ep_fizika"` (`00CEB900`, at `00938204`), and the
plain names `"front"` (`00D196A0`), `"back"` (`00D19698`) and `"firstnode"` (`00D1968C`). It writes
`FFh` into `controller+34Ch+i` first (`0093813A`) and overwrites it with a running counter
(`009381B5`, `[ESP+18h]`, incremented right after) only when the node lookup returned a non-empty
list, so **the group slots are numbered over the parts that have model nodes, and a part with no
nodes keeps `-1`**. Its Hungarian diagnostic `"Hajodarabnak nincs utkozoje:fizika_%d, kb %d. db"`
(`00D196A8`, "the ship piece has no collider") is emitted at `0093871C`.

The `14h`-byte group entries hold a node pointer at `+0h` (dereferenced everywhere in `00934150`),
a dword at `+4h` and three dwords at `+8h`..`+10h` copied straight into the debris spec
(`00934E3E` onwards); their full layout is `contract: unread`.

## Detaching a part: `00934150`

`__thiscall(controller, int index, const float impulse[3])`, `RET 8`, body `00934150..0093553A`
(1422 instructions). Both callers pass a zero impulse, so every effect of `impulse` below is
inferred from the arithmetic, not observed with a non-zero value.

| step | site | what happens |
| --- | --- | --- |
| 1 | `0093417A` | `rec = controller + 8Ch + index*20h` (`SHL EAX,5`, `LEA EDI,[EAX+ESI]`) |
| 2 | `009341C1` | for every node in `rec+4h..rec+8h`: `00B6DFA0` `BSP_Node_UnlinkAndRelease`, then null the slot |
| 3 | `00934248` | for every int `e` in `rec+14h..rec+18h`: `fx = [[controller+1Ch]+A14h][e]`; when non-null, `00867B10` `BSP_PointEffect_StopChildren(fx)` and `fx+9h = 1` (the debris byte of `docs/GAME_DYNAMICS_LIST.md`) |
| 4 | `00934267`, `00934277` | release that effect (`InterlockedDecrement` on `fx+4h`, destructor through `fx->vtable[0]`) and null the slot |
| 5 | `009342E0`, `0093433D` | clear both vectors (the `memmove_s` erase-to-begin idiom, `00BF67A7`) |
| 6 | `00934356`, `00934366` | `g = (int8)[controller+34Ch+index]`; `JL 00935520` when `g < 0`, so a part with no group stops here |
| 7 | `0093442F`, `009344EE`, `00934530` | pass over group `+340h[g]`: `00B6DA70` set visibility, `00B6E0A0` position + a `00CF81F0` bias, node `vtable[2Ch]` |
| 8 | `00934671`, `009346EE`, `00934758` | pass over group `+320h[g]`: sum `vtable[48h]` centres into a centroid and keep the minimum `y` |
| 9 | `0093487A` / `009348EA` | `008685E0` `BSP_PointEffect_CreateFromPosition` at the centroid, lowered by `00D7A308`; the effect handle is `[[unit+538h]+7E8h][0]` when that class vector is non-empty, otherwise `[00424C40()+3A8h]`; the result's `+9h` byte is set to 1 |
| 10 | `00934930` | `00C31F40`, the physics-library entry that opens the debris build |
| 11 | `009349DD`..`00934D94` | per group entry: normalise the caller's impulse (`0042B260` + `00BD2F10`), add the radial direction from the centroid, and derive a spin from the cross product |
| 12 | `00934C70` | `00B6D890` `BSP_Node_PropagateRootRegistration` after `unit->vtable[190h]` and the entry node's `vtable[10h]` |
| 13 | `00934F7C`, `00934FAB` | `sprintf(buf, "fizika_%d, %d. db")` (`00BF7A6A`) and `00408720` `BSP_CharacterString_AssignCounted` name the debris body |
| 14 | `00934FD1` | `00447510` `BSP_GameDynamicsList_Add(spec, flag)` with `ECX = [00E188A8]+30h`: the rigid body and its record are created there (`docs/GAME_DYNAMICS_LIST.md`) |
| 15 | `00935171`..`009353xx` | pass over group `+330h[g]`: refresh the world matrix when `node+5Ch & 2` is clear, `vtable[4Ch]`, `0092E250`, `00427D10` twice, set visibility, `vtable[2Ch]` |
| 16 | `00935350` | `004A5C60` |
| 17 | `009353F8`..`00935512` | tail over `controller+8h..+Ch`: for every node whose `+ACh` exceeds `00D7A218` (`0.0f`), refresh, `vtable[4Ch]`, `0092E250`, and when `0085BF20()` is true also set visibility and `vtable[2Ch]` |
| 18 | `00935529` | `[unit+1174h] = 1` |

Steps 7-18 all sit inside the `g >= 0` branch, so a part without a model group is unlinked and its
effects are stopped but no debris is created and `unit+1174h` is not set.

## What decides to detach

`00934150` has two callers and neither computes a threshold.

* `00935C70` `BSP_UnitParts_DetachAllLiveParts` walks `controller+310h..+314h` and, for every entry
  greater than `0.0f`, stores `-10000.0f` (`00D19620`) and calls `00934150(controller, i, {0,0,0})`.
  Its only caller is the Lua binding `ExplodeToParts` (`0088E2CF`). So the live test is
  `health > 0.0f` and the detached marker is `-10000.0f`.
* `0080E440` `BSP_UnitInstance_DetachPart` reads `unit+1018h`, zeroes a stack `vec3` and calls with
  the caller's index (`0080E467`, `ADD ESP,0xC` at `0080E46C`).

`00821E80` is **not** a threshold rule: it is the unit's message handler,
`undefined1 __thiscall(unit, msg)` with `RET 4`, switching on the byte at `msg+10h`
(`00821E9D..00821EB7`: `ADD EAX,-4Bh`, `CMP EAX,55h`, byte table at `00822400`, 27 target dwords at
`00822394`). Target index `13h` is `00821FF0`, selected by byte `4Eh` of the index table, so the
kind is `4Bh + 4Eh = 99h`. That case reads `[msg+20h]` as the part index and calls `0080E440`:

```
00821ff0: MOV ECX,dword ptr [ESI + 0x20]   ; msg+20h, the part index
00821ff3: PUSH ECX
00821ff4: MOV ECX,EDI                       ; the unit
00821ff6: CALL 0x0080e440
```

So a part detaches when the unit receives message kind `99h`. The neighbours in the same table are
kind `98h` -> `00814520` (which calls `00935D30`), kind `9Ah` -> `00814560` and kind `9Bh` ->
`0093AA90` with three fields of the message; `9Ah` is also the death-message id of
`docs/UNIT_DAMAGE_AND_DEATH.md` step 8, so this table and the session message ids share a
numbering. **Who sends kind `99h`, and with which index, is `contract: unread`** and is the open
question that a threshold rule would answer. Nothing in this packet writes `controller+310h` after
the build either, so the per-part health that `00935C70` tests has no observed damage path.

## Health changes on a sub-object: the `00958A30` branch

`__fastcall(this)`, `RET 0`, body `00958A30..00958DCF`. `this+71Ch` is the owning unit and is
non-null only on a damageable sub-object; `+370h` is health, `+36Ch` maximum health, `+720h` a byte,
`+728h` the lockout float, `+538h` the class descriptor. The root-unit branch (`+71Ch == 0`) is the
one `docs/UNIT_DAMAGE_AND_DEATH.md` already covers. The sub-object branch splits three ways:

| condition | what happens |
| --- | --- |
| `health <= 0` and `+720h == 0` | the **break** sequence: start the named effect `"InferiorFailure"` through `vtable[194h](name, 0)`, then `vtable[1B4h](0072AB10(this))`, then set `+728h` from the class, then the owner-damage rule below |
| `health <= 0`, `+720h != 0`, `+358h == 0` and `+728h <= 0` | the same sequence **without** the effect: `vtable[1B4h](0072AB10(this))` and the `+728h` class selection, then return |
| `+720h != 0` and `max <= health` | the **repair** sequence: `vtable[19Ch]("InferiorFailure", 0)` stops the effect, and when `+728h > 0` it is zeroed and `vtable[1B4h](0)` restores the intact model |

The `+728h` class selection is one rule used twice, and its short-circuit matters:

```
728h = [[this+71Ch]+538h + 190h]
if IsKindOf(46h) || (IsKindOf(1Bh) && vtable[204h](46h)) : 728h = [class + 18Ch]
if IsKindOf(45h) || (IsKindOf(1Bh) && vtable[204h](45h)) : 728h = [class + 18Ch]
```

`IsKindOf` is `vtable[5Ch]`; `vtable[204h]` is a second class test that only class `1Bh` answers.
The class ids `45h`, `46h` and `1Bh` are not resolved to names in this packet. Note that both tests
read the **owner's** class descriptor (`[this+71Ch]+538h`), not the sub-object's.

The owner-damage rule closes the break sequence: when `IsKindOf(1Bh)` and the sub-object's own
`[[this+538h]+174h]` byte is set, the owner takes
`owner->vtable[1ACh]([[[this+71Ch]+538h]+194h])` at `00958BC2` (`FLD [EAX+194h]` at `00958BB2`,
`ECX` = the owner), that is `AddDamage` with a fixed per-class amount. Separately, `vtable[1B4h](0)`
is issued at `00958CF4` whenever `+720h` is set (`00958CD4`) and `+728h` is not positive
(`COMISS XMM1,[ESI+728h]` / `JC` at `00958CE1`), before the `health >= max` repair test at
`00958D12`.

## Resetting: `0080E410`

`BSP_UnitInstance_ResetCondition`, `__thiscall(unit)`, body `0080E410..0080E43F`: `00878340` at
`0080E413` restores full health, then `1.0f` (`00D7A24C`) into `unit+9D8h` and `unit+9DCh` and the
bytes `unit+9E4h` and `unit+9E5h` are cleared. Its only call site is `00758373` in `00758370`, but
its address also appears in eight vtables (`00CF9268`, `00CFA930`, `00CFB8F0`, `00CFC588`,
`00CFFBE8`, `00D09830`, `00D0C138`, `00D0C800`), so it is a virtual override; which slot is
`contract: unread`. It does **not** reattach parts: nothing in it touches `controller+310h`, the
`+8Ch` records or `unit+360h`.

## Coverage

| routine | coverage |
| --- | --- |
| `00934150` | complete for the control flow and the named call sites; the `14h`-byte group entry layout and the debris spec fields beyond `docs/GAME_DYNAMICS_LIST.md` are `contract: unread` |
| `0080E440`, `0080E410` | complete |
| `00821E80` | partial: the `99h` case and the switch decoding; the other 26 targets are named only by their dispatch bytes |
| `00958A30` | complete for the sub-object branch's structure and the `+728h` rule; the six virtual slots it calls are `contract: unread` |
| `007135C0` | complete for the fields it writes; `FUN_007103A0`, `FUN_007103C0`, `FUN_00713380`, `FUN_00712440`, `FUN_00711C60` are `contract: unread` |
| `00937C90` | partial: read only for the writes to `+310h`, `+320h`/`+330h`/`+340h` and `+34Ch` and for its node-name strings; `00937C90..00939C8F` is otherwise unread |
| `0087BCC0` tail | complete for `0087BEAA..0087BF73` |
| `00939CB0`, `0080DEC0` | read as the producers of the controller allocation |
| the `30h` part descriptor | `contract: unread` |

## Host table

One row per native call site modelled in `src/unit_parts.cpp`.

| site | callee | method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009341C1` | `00B6DFA0` | `release_part_node` | node / - / void | slot non-null |
| `00934248` | `00867B10` | `stop_effect_children` | effect / - / void | slot non-null |
| `00934267` | `[00CE2220]` | `release_effect_ref` | effect / - / void | slot non-null |
| `0093442F`, `00935477`, `009353xx` | `00B6DA70` | `set_node_visibility` | node / factor / void | per pass |
| `009344EE`, `009352DB`, `009354BF` | `00B6E0A0` | `node_world_position` | node / - / vec3 | per pass |
| `00934530`, `00935329`, `0093550C` | `vtable+2Ch` | `node_set_position` | node / vec3 / void | per pass |
| `00934671`, `009346EE`, `00934758` | `vtable+48h` | `node_bounds_centre` | node / - / vec3 | group `+320h` pass |
| `0093487A`, `009348EA` | `008685E0` | `spawn_debris_effect` | - / handle, position / effect | group `+320h` non-empty |
| `009348BC` | `00424C40` | `game_settings` | - / - / settings | class effect list empty |
| `00934930` | `00C31F40` | `physics_begin_debris` | body / - / void | group `+320h` non-empty |
| `009349DD`, `00934AD3`, `00934D78` | `0042B260` | `normalize_with_floor` | - / vec3 / vec3 | per entry |
| `009349FD`, `00934AF3`, `00934D94` | `00BD2F10` | `reciprocal_length` | - / float / float | per entry |
| `00934A89` | `unit vtable+190h` | `unit_detail_for_lod` | unit / - / handle | per entry |
| `00934C70` | `00B6D890` | `register_debris_node` | node / - / void | per entry |
| `00934CD8`, `00935171`, `009353F8` | `00B6DB70` | `refresh_world_matrix` | node / - / void | `node+5Ch & 2` clear |
| `00934F7C` | `00BF7A6A` | `format_debris_name` | - / `"fizika_%d, %d. db"` / string | per entry |
| `00934FAB` | `00408720` | `assign_debris_name` | string / counted chars / void | per entry |
| `00934FD1` | `00447510` | `add_game_dynamics_body` | dynamics list / spec, flag / void | per entry |
| `00935196`, `00935421` | `0092E250` | `apply_node_transform` | node / - / void | passes `+330h` and the tail |
| `009351A5`, `009351B5` | `00427D10` | `compose_node_transform` | - / matrix / void | pass `+330h` |
| `00935350` | `004A5C60` | `flush_scene_batch` | - / - / void | group present |
| `00935444` | `0085BF20` | `hull_node_is_visible` | - / - / bool | tail pass |
| `00821FF6` | `0080E440` | `unit_detach_part` | unit / `[msg+20h]` / void | message kind `99h` |
| `0080E467` | `00934150` | `controller_detach_part` | controller / index, zero impulse / void | none |
| `00935D16` | `00934150` | `controller_detach_part` | controller / index, zero impulse / void | `health > 0` |
| `00958ACE`, `00958C03` | `0072AB10` | `last_damage_state_index` | - / unit / int | sub-object break |
| `00958ADC`, `00958C11` | `vtable+1B4h` | `set_damage_state` | sub-object / last index / void | sub-object break |
| `00958CF4`, `00958D94` | `vtable+1B4h` | `set_damage_state` | sub-object / `0` / void | `+728h <= 0`, and after a repair |
| `00958D43` | `vtable+19Ch` | `stop_named_effect` | sub-object / name, 0 / void | `health >= max` |
| `00958AA1` | `vtable+194h` | `start_named_effect` | sub-object / name, 0 / void | first break |
| `00958A83`, `00958D25` | `0041E870` | `make_effect_name` | string / `00CF0B74` `"InferiorFailure"` / string | both effect calls |
| `00958B8F`, `00958B07`, `00958B40` | `vtable+5Ch` | `is_kind_of` | sub-object / class id / bool | per test |
| `00958B20`, `00958B68` | `vtable+204h` | `secondary_kind_test` | sub-object / class id / bool | `IsKindOf(1Bh)` |
| `00958BC2` | `owner vtable+1ACh` | `owner_add_damage` | owner / `[[owner+538h]+194h]` / void | `IsKindOf(1Bh)` and `[this+538h]+174h` |
| `0080E413` | `00878340` | `restore_full_health` | unit / - / void | none |

## Open questions

* The producer of the `30h` part descriptors in `class+18h`. Neither `00960230` nor `0087CA80`
  writes that vector, so it is filled by the model or part-set loader that also owns `class+50h`.
* Who sends unit message kind `99h`. Without it there is no evidence for a per-part damage
  threshold anywhere: `00937C90` fills `controller+310h` at build time and only `00935C70` was
  found writing it afterwards.
* The relation between the part index used by `00934150` (`0..19`, indexing the controller records)
  and the entries of `unit+344h` (the class descriptors). No routine read here uses both.
* `docs/UNIT_HIT_PATH.md`'s part pass keeps the worst part's index but does not pass it on; the
  consumer of that index is still unidentified, and it is not `00934150`.
* The six virtual slots of `00958A30` (`5Ch`, `194h`, `19Ch`, `1B4h`, `204h`, `1ACh`) and the class
  ids `45h`, `46h`, `1Bh`.
