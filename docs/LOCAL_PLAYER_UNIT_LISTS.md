# The eight local-player unit lists (`004C3CB0`)

Addresses: 004c3cb0 004bfdf0 004bf8e0 00484540 004c2be0 008ddf90 008ddf00 008df900 004f2590 004f25b0 006e2790 007efb00 0074e400 006f5890 007004b0 006d20e0 00846c00 0095ff30 0095ffa0 009635e0 007cfd00 00749030 00960030 009600a0 006e8400 006e8410 006ea290 006ea2a0

`docs/IN_MISSION_SUBSYSTEM_TICK.md` call 2 left this body documented but not reconstructed. This
packet reads `004C3D5B..004C404B` instruction by instruction, settles the pop-versus-clear question
against `004BF8E0`, names the eight `vtable[5Ch]` kind codes from the class-id space they belong to,
and reconstructs the whole routine as `include/bsp/local_player_unit_lists.hpp` /
`src/local_player_unit_lists.cpp`. Every name below is a hypothesis, not a recovered symbol, except
the `M*` class names, which are string constants in the image.

## The routine

`__thiscall void(GGame*)`, body `004C3CB0..004C409D`, `RET` (no stack argument). The guard and the
latch (`004C3CB7..004C3CDC`) are already reconstructed as `local_player_unit_lists_run_004c3cb0`;
this packet keeps that function and adds the body.

Registers through the body: `ESI` = game, `EBX` = 0 (the comparison zero, `004C3CB5`), `EBP` = the
current world-list node, `EDI` = the current unit, `[ESP+10h]` = the unit registry pointer saved at
`004C3D00` and reloaded at `004C3D5D` and `004C3EB0`.

## The source: the local player's unit registry

```
004c3ce8: EAX = [ESI + 18ECh]              ; the local-player slot selector
004c3cee: ECX = [ESI + EAX*4 + 18CCh]      ; that slot's player record
004c3cf5: EAX = [ECX + 30h]                ; the unit registry
004c3cf8: EBP = [EAX + DDCh]               ; walk 0 head
```

The registry object carries **five** `{count, head, tail}` lists at `+DD8h`, `+DE4h`, `+DF0h`,
`+DFCh` and `+E08h`. The producer is `008050E0`, the constructor the recon slots use
(`docs/FIXED_STEP_COUNTDOWN.md`): `008050E0` zeroes exactly `+DD8h..+E10h` in fifteen consecutive
dword stores (`0080518D..008051DB`), which sets the triple boundaries, and `008073C0` clears the same
five triples through `008042B0` at `008073D0..00807416`. `004C3CB0` walks the heads of triples 0, 1
and 3 (`+DDCh`, `+DE8h`, `+E00h`) and never touches triples 2 and 4; `docs/HUD_CENTRAL_UPDATES.md`
reads triple 4's head (`+E0Ch`) as "the team unit list".

**Uncertain:** that the object at `player+30h` is the same class as the recon slot objects rests on
the matching five-triple layout, not on a read of the code that writes `player+30h`. What each of
the five lists holds is not settled; the walks are named 0, 1 and 2 by their order in the body.

Node layout in those lists, from the walk itself (`004C3D06..004C3D09`, `004C3D56`):

| offset | field | evidence |
| --- | --- | --- |
| `+0h` | `prev` | `004BF8E0` unlink, `00484540` push |
| `+4h` | `next` | `004C3D56 MOV EBP,[EBP+4]` |
| `+8h` | value | `004C3D06 MOV EDX,[EBP+8]` |

The world lists hold a wrapper, not the unit: `004C3D09 MOV EDI,[EDX+4]` takes the unit from the
value's `+4h`. The game's own eight lists hold the unit pointer directly (`004C3D3E PUSH EDI` into
`00484540`, which stores the argument at `node+8h`).

## The four-byte filter, identical in all three walks

```
004c3d0c: [EDI + 5Ch] != 0     ; must be set
004c3d11: [EDI + 5Dh] == 0
004c3d16: [EDI + 60h] == 0
004c3d1b: [EDI + 5Eh] == 0
```

`+5Ch` is the world tick's activity gate (`docs/GAME_WORLD_ENTITIES.md`, `docs/UNIT_INSTANCE_UPDATE.md`,
`bsp/unit_instance.hpp` `active`). The other three are required **clear**; `bsp/unit_instance.hpp`
calls `+5Dh` `simulate` and defaults it to `true`, which this filter contradicts (see Corrections).
The same four bytes appear at `004C3D76..004C3D94` (walk 1) and `004C3EC8..004C3EE6` (walk 2).

## The eight lists and what reaches each

All eight are `{count, head, tail}` at `game+1964h`, `+1970h`, `+197Ch`, `+1988h`, `+1994h`,
`+19A0h`, `+19ACh` and `+19B8h`. Appends are `00484540` or one of four copies of it that MSVC
inlined; the inlined copies are byte-for-byte the same sequence with the list address folded in.

| list | filled by | kind test | append site |
| --- | --- | --- | --- |
| `+1964h` | walk 0 | `IsKindOf(06h)` true | `004C3D45` (`00484540`) |
| `+1970h` | walk 0 | every unit that passes the filter and is **not** `IsKindOf(2Ah)` | `004C3D51` (`00484540`) |
| `+197Ch` | walks 1, 2 | `IsKindOf(06h)` true | `004C3DA9..004C3DF0`, `004C3EFB..004C3F42` (inlined) |
| `+1988h` | walks 1, 2 | `IsKindOf(18h)` true | `004C3E0B`+`004C3EA0`, `004C3F5D..004C3FA4` (inlined) |
| `+1994h` | walks 1, 2 | `IsKindOf(45h)` true | `004C3E38`+`004C3EA0`, `004C3FBF..004C4006` (inlined) |
| `+19A0h` | walks 1, 2 | `IsKindOf(46h)` true | `004C3E4F`+`004C3EA0`, `004C401E`+`004C403C` |
| `+19ACh` | walks 1, 2 | `IsKindOf(1Bh)` true | `004C3E66`+`004C3EA0`, `004C4035`+`004C403C` |
| `+19B8h` | walk 1 only, then the merge | `IsKindOf(35h)` true, or `008DDF90` true | `004C3E99`+`004C3EA0` |

### Walk 0, `004C3D06..004C3D5B` (head `+DDCh`)

Filter, then `IsKindOf(2Ah)`: **true means skip the unit entirely**. Otherwise `IsKindOf(06h)`
decides whether it also goes to `+1964h`, and every surviving unit is appended to `+1970h`
unconditionally. So `+1970h` is "everything in walk 0 that is not ordnance" and `+1964h` is its ship
subset. No other kind is queried in this walk.

### Walk 1, `004C3D70..004C3EAA` (head `+DE8h`), the full decision chain

```
IsKindOf(06h)  -> +197Ch          (inlined push_back, 004C3DA9)
IsKindOf(18h)  -> +1988h
IsKindOf(0Fh)  -> skip            (004C3E23 JNZ to the loop tail: planes are excluded)
IsKindOf(45h)  -> +1994h
IsKindOf(46h)  -> +19A0h
IsKindOf(1Bh)  -> +19ACh
IsKindOf(35h)  -> +19B8h
008DDF90(unit) -> +19B8h          (004C3E90)
otherwise      -> skip
```

The tests are in that order and each is reached only when every earlier one failed.

### Walk 2, `004C3EC2..004C4046` (head `+E00h`)

The same chain **minus three tests**: no `0Fh`, no `35h` and no `008DDF90` call.

```
IsKindOf(06h)  -> +197Ch
IsKindOf(18h)  -> +1988h
IsKindOf(45h)  -> +1994h
IsKindOf(46h)  -> +19A0h
IsKindOf(1Bh)  -> +19ACh
otherwise      -> skip
```

Because `0Fh` is not tested here, a plane in walk 2 falls through every test and is skipped anyway;
the difference that matters is that walk 2 contributes nothing to `+19B8h` directly.

### The merge tail, `004C404C..004C4098`

`ECX = game+19B8h` for all five calls; the argument is the source list.

```
004c405b: 004C2BE0(+19B8h, +197Ch)
004c4069: 004C2BE0(+19B8h, +1988h)
004c4077: 004C2BE0(+19B8h, +1994h)
004c4085: 004C2BE0(+19B8h, +19A0h)
004c4093: 004C2BE0(+19B8h, +19ACh)
```

`+19B8h` ends as ships + squadrons + airfields + shipyards + land forts, plus whatever walk 1 put
there directly (dummy targets and objective-set members). `+1964h` and `+1970h` are **not** merged.
The sources are not cleared, so every merged unit is in two lists.

## The list ABI

| routine | signature | body | what it does |
| --- | --- | --- | --- |
| `00484540` | `__thiscall void(List*, void* value)`, `RET 4` | `00484540..00484594` | `operator new(0Ch)` node, `node->value = arg`, `node->prev = tail`, link at the tail, `++count` |
| `004BF8E0` | `__thiscall void(List*)`, `RET` | `004BF8E0..004BF925` | **clears the list**: unlink the head, `--count`, `free(node)`, repeat while `count != 0` |
| `004C2BE0` | `__thiscall void(List* dst, List* src)`, `RET 4` | `004C2BE0..004C2C3B` | push_back every value of `src` onto `dst`; `src` is left untouched |
| `004BFDF0` | `__thiscall void(GGame*)`, tail-call | `004BFDF0..004BFE47` | `004BF8E0` on each of the eight heads, in address order |

The allocation failure path of `00484540` and of all four inlined copies is a null dereference:
`operator new` returning 0 leaves `ECX`/`EAX` zero (`00484560 XOR ECX,ECX`, `004C3DC1 XOR EAX,EAX`)
and the next instruction stores through it. That is the MSVC `new T()` shape, not a checked path.

### The pop-versus-clear question: `004BFDF0` clears, it does not pop

`docs/IN_MISSION_SUBSYSTEM_TICK.md` recorded `004BFDF0` as popping one element per list "which looks
like a defect worth confirming against a run". The listing settles it without a run. `004BF8E0`:

```
004bf8e3: CMP dword ptr [ESI],0x0
004bf8e6: JZ  004bf924            ; empty -> return
004bf8e8: EAX = [ESI+4]           ; loop head: the list head node
   ... unlink EAX from the doubly linked list, fixing list.head/list.tail ...
004bf913: ADD dword ptr [ESI],-1
004bf916: PUSH EAX ; CALL 00bf65ac ; ADD ESP,4      ; free(node)
004bf91f: CMP dword ptr [ESI],0x0
004bf922: JNZ 004bf8e8            ; back edge
004bf924: POP ESI ; RET
```

`004BF922` is a back edge to `004BF8E8`, so the routine erases and frees every node. There is no
defect and nothing to confirm against a run: the eight lists are empty when the walks start. The
earlier reading came from the pseudocode, which renders the loop as a single erase.

## The `vtable[5Ch]` kind codes

`vtable[5Ch]` is `bool IsKindOf(int classId)`, `RET 4` (`docs/UNIT_INSTANCE_UPDATE.md`,
`bsp/unit_instance.hpp` `unit_is_kind_of_006fe530`). Each implementation is a chain of
`CMP EAX,imm / JE` against the ids the class answers to, most of them ending with
`CMP EAX,[ECX+0C4h]`, the instance's own id.

The id space is recoverable because the same ids appear in the **class descriptor** vtables, and a
descriptor vtable has a name getter. For every descriptor vtable read here the layout is fixed:
`+4h` scalar deleting destructor, `+0Ch` `const char* GetClassName()` (`MOV EAX,imm32; RET`), `+18h`
`IsKindOf`, `+28h` the instance creator. `MDestroyer` anchors it: descriptor vtable `00D1ACF8`,
name getter `00963B60` -> `"MDestroyer"` at `00D1AD28`, `IsKindOf` `00963B70` answering `[7,6,5,4]`,
creator `006FE590` (the value `docs/SCENE_UNIT_CREATORS.md` already recorded), and the instance
`IsKindOf` `006FE530` answering `[7,6,5,4,2,1,0]` plus `+0C4h`. Descriptor and instance share the id;
the descriptor chain is the instance chain without the entity roots `2,1,0`.

### The eight codes this routine uses

| code | class | evidence |
| --- | --- | --- |
| `06h` | the **ship base class** | descriptor `IsKindOf` `009635E0` = `[6,5,4]`; every ship descriptor chain contains 6: `MDestroyer` 7, `MSubmarine` 8, `MMothership` 9, `MCruiser` 10, `MCargo` 11, `MLandingShip` 12, `MBattleship` 13, `MTorpedoBoat` 14. No name getter, so the class itself is unnamed |
| `0Fh` | the **plane base class** | descriptor `007CFD00` = `[15,5,4]`, instance `0074E400` = `[15,5,4,2,1,0]`; derived: `MPlaneBomber` 16, `MPlaneTorpedoBomber` 17, `MPlaneDiveBomber` 18, `MPlaneFighter` 19, `MReconPlane` 20 (`MSmallReconPlane` 21, `MLargeReconPlane` 22), `MPlaneKamikaze` 23. Unnamed |
| `18h` | the **plane squadron** | instance `IsKindOf` `007EFB00` = `[24,2,1,0]` sits at `+5Ch` of vtable `00D087C0`, which `007F2C60` installs at `007F2CAF`; `007F2C60` is the `0x414`-byte squadron constructor of `docs/SCENE_UNIT_CREATORS.md` (`PlaneSquadronGen`, 2131 authored instances). Not derived from the vehicle root, so a squadron is not a unit |
| `1Bh` | `MLandFort` | descriptor vtable `00CFF790`, name getter -> `"MLandFort"` at `00CFF79C`, `IsKindOf` `00749030` = `[27,5,4]`; instance `006F5890` = `[27,5,4,2,1,0]`. `MCommandBuilding` 28 derives from it (`00953680` = `[28,27,5,4]`), so this test also matches command buildings |
| `2Ah` | `MBomb`, the **ordnance base** | name getter `006EA290` -> `"MBomb"`, descriptor `IsKindOf` `006EA2A0` = `[42,41]` (41 is `MBullet`, `006E8400`/`006E8410`); the instance base constructor `006E2670` writes `[ESI+0C4h] = 2Ah` at `006E271E` and installs vtable `00CF9438`, whose `IsKindOf` `006E2790` = `[42,2,1,0]`. Derived: `MTorpedo` 43, `MDepthCharge` 44, a decoy base 45 (`MDummyTarget` 46, 47, 48), `MParatrooper` 49, `MRocket` 51, `MWaterMine` 52 |
| `35h` | `MDummyTarget` (the vehicle one) | descriptor vtable `00D1AA58`, name getter `00960090`, `IsKindOf` `009600A0` = `[53,5,4]`; the `DummyTargetVehicle` row of `docs/SCENE_UNIT_CREATORS.md` (constructor `00960050`, one authored instance) |
| `45h` | `MAirfield` | descriptor vtable `00D1A9A0`, name getter `0095FF20` -> `"MAirfield"` at `00D1A9CC`, `IsKindOf` `0095FF30` = `[69,5,4]`, creator `006D3110`; that creator's constructor `006D1C20` installs instance vtable `00CF8C08`, whose `IsKindOf` `006D20E0` answers `[5,4,2,1,0]`, `+0C4h` **and `45h`** |
| `46h` | `MShipyard` | descriptor vtable `00D1A9DC`, name getter `0095FF90` -> `"MShipyard"` at `00D1AA08`, `IsKindOf` `0095FFA0` = `[70,5,4]`, creator `00848380`; the shipyard instance `IsKindOf` `00846C00` answers `[5,4,2,1,0]`, `+0C4h` **and `46h`** |

So the eight tests read: ships, planes (only to exclude them), squadrons, land forts and command
buildings, ordnance (only to exclude it), dummy targets, airfields, shipyards. The five lists the
merge feeds are exactly the things a player commands in this game, and individual planes are
deliberately left out in favour of their squadron.

### The wider id space

Ids are unique inside the instance space (every chain that ends with `CMP EAX,[ECX+0C4h]` is rooted
at 0), but **not** across the descriptor families. `MDummySubmarine`'s descriptor answers `[8,48,42,41]`
and `MDummyKamikazePlane`'s answers `[23,47,45,42,41]`, reusing `MSubmarine`'s 8 and
`MPlaneKamikaze`'s 23 so a decoy reports the class it imitates. Ids quoted from a descriptor chain
are only meaningful together with that chain's root (4 for `VehicleClass`, 41 for `MBullet`).

Named ids recovered here, all from a `MOV EAX,<string>; RET` name getter at `+0Ch` of the vtable
whose `+18h` is the quoted `IsKindOf`: 7 `MDestroyer`, 8 `MSubmarine`, 9 `MMothership`,
10 `MCruiser`, 11 `MCargo`, 12 `MLandingShip`, 13 `MBattleship`, 14 `MTorpedoBoat`,
16 `MPlaneBomber`, 17 `MPlaneTorpedoBomber`, 18 `MPlaneDiveBomber`, 19 `MPlaneFighter`,
20 `MReconPlane`, 21 `MSmallReconPlane`, 22 `MLargeReconPlane`, 23 `MPlaneKamikaze`,
25 `MLandVehicle`, 27 `MLandFort`, 28 `MCommandBuilding`, 33 `MRFSGun`, 35 `MRTGun`, 36 `MSTGun`,
37 `MBombPlatform`, 38 `MMultipleBombPlatform`, 39 `MDepthChargeLauncher`, 40 `MCatapult`,
41 `MBullet`, 42 `MBomb`, 43 `MTorpedo`, 44 `MDepthCharge`, 46 `MDummyTarget` (decoy family),
49 `MParatrooper`, 51 `MRocket`, 52 `MWaterMine`, 53 `MDummyTarget` (vehicle family),
69 `MAirfield`, 70 `MShipyard`.

## `008DDF90`, the objective-set membership test

`__thiscall char(SzurkeNyil*, unit)`, body `008DDF90..008DDFD9`, `RET 4`.

```
if ([this+20h] == 0) return 0;                 ; empty set
if (unit->IsKindOf(5))        target = unit;   ; 5 is the vehicle root
else if (unit->IsKindOf(18h)) target = [unit+3D0h];   ; a squadron's plane
else return 0;
return 008DDF00(this, target);
```

`008DDF00` is the set lookup: the container is a `_Tree` at `this+18h` with `_Myhead` at `+1Ch` and
`_Mysize` at `+20h` (producer `008DF900`, which builds the head node through `008DB7B0` at
`008DF943`, self-links it and marks `+11h` nil), `008DD310` is the find, and
`CMP [EDI+4],EBX / SETNZ AL` at `008DDF30` returns "the iterator is not the end node".

Every one of the fourteen call sites builds `this` the same way:
`[game + 21A4h + [00E188A8 + 18ECh]*4]` — the local player's slot in the eight-object array
`docs/GAME_WORLD_CONSTRUCT.md` row 20 records (`008DF900(i)` then `008DA160`, `30h` each,
`game+21A4h..+21C0h`). The class string sits immediately before its vtable: `"SzurkeNyil"` at
`00D16100`, vtable `00D1610C` (Hungarian *szürke nyíl*, "grey arrow"). Note that `004C3E7D` reads the
index from the **global** game `[00E188A8]` while the array base is `this`; the two are the same
object on every path that reaches here.

`004F2590..004F25AB` and `004F25B0..004F25CB` are two identical `__thiscall bool(unit)` thunks that
forward `this` as the argument and fetch the set the same way. They have no Ghidra function.

## Coverage

| routine | reconstruction | coverage |
| --- | --- | --- |
| `004C3CB0` | `build_local_player_unit_lists_004c3cb0` + the three `classify_walk*` rules | complete for `004C3CB0..004C409D` |
| `00484540` | `unit_list_push_back_00484540` | complete |
| `004BF8E0` | `unit_list_clear_004bf8e0` | complete |
| `004C2BE0` | `unit_list_append_all_004c2be0` | complete |
| `004BFDF0` | `clear_all_unit_lists_004bfdf0` | complete |
| `008DDF90` | `objective_set_contains_008ddf90` | complete for `008DDF90`; `008DDF00`/`008DD310` are the STL find and are modelled as a set lookup, not reconstructed |
| `008050E0` | none | partial: only the five-triple zeroing at `0080518D..008051DB` was read |
| `008073C0` | none | partial: only the five `008042B0` clears at `008073D0..00807416` and the list-1-to-list-3 move at `00807634..008076E2` were read |

## Corrections

| what | was | is | evidence |
| --- | --- | --- | --- |
| `004BFDF0` | "pops the head off each of eight lists ... looks like a defect worth confirming against a run" (`docs/IN_MISSION_SUBSYSTEM_TICK.md`) | clears each of the eight lists completely | the back edge `004BF922 JNZ 004BF8E8` after `--count` and `free` |
| kind `18h` | `kUnitTraitHasDrivenSubUnit` (`bsp/unit_instance.hpp`), `kUnitCategoryPlaneAlt` (`bsp/award_trackers.hpp`) | the plane squadron class (24) | `007EFB00` = `[24,2,1,0]` at `+5Ch` of `00D087C0`, installed by the squadron constructor `007F2C60` at `007F2CAF` |
| kind `0Fh` | `kUnitTraitPublishesAnchor` (`bsp/unit_instance.hpp`) | the plane base class (15) | `007CFD00` = `[15,5,4]`; ids 16..23 are the named plane classes |
| kind `05h` | `kUnitTraitDirectlyControlled` (`bsp/unit_instance.hpp`) | the vehicle base class (5): every `VehicleClass` descriptor chain ends `[.., 5, 4]` | `00749010` = `[5,4]`, and 6, 15, 25, 27, 53, 69, 70 all derive from it |
| kind `1Bh` | `kUnitCategoryGunnery` (`bsp/award_trackers.hpp`) | `MLandFort` (27), with `MCommandBuilding` (28) derived | name getter at `00CFF790+0Ch` -> `"MLandFort"`; `IsKindOf` `00749030` = `[27,5,4]` |
| kind `0Bh` | `kUnitCategoryReconPlane` (`bsp/award_trackers.hpp`) | `MCargo` (11); `MReconPlane` is 20 | descriptor vtable `00D1ADBC`, name getter -> `"MCargo"`, `IsKindOf` `00963D00` = `[11,6,5,4]` |
| unit `+5Dh` | `simulate`, default `true` (`bsp/unit_instance.hpp` `UnitInstanceState`) | a byte this filter requires **clear** for a unit to be listed | `004C3D11 CMP byte ptr [EDI+5Dh],BL` / `004C3D14 JNZ` (skip), repeated at `004C3D7F` and `004C3ED1` |

The three headers named above belong to other packets and were not edited. `kUnitCategoryShip`
(`06h`), `kUnitCategorySubmarine` (`08h`) and `kUnitCategoryPlane` (`0Fh`) in `bsp/award_trackers.hpp`
agree with the id table.

## Follow-up packets

| packet | addresses | question |
| --- | --- | --- |
| `unit_registry_five_lists` | 008050e0 008073c0 008042b0 00804e10 00805490 | What the five `{count, head, tail}` triples at `+DD8h..+E08h` separate. `008073C0` moves entries from triple 1 to triple 3; the walks in `004C3CB0` read triples 0, 1 and 3 and the HUD reads triple 4 |
| `unit_list_consumers` | 00586eff 005403b4 0056e9bb 00525fb3 00526377 0059cedc 005d3580 00687387 00746f93 | Who reads the eight lists. `+19B8h` has seven readers outside this file, `+1964h` one, `+1970h` two; naming the lists from a consumer would confirm the "commandable units" reading |
| `unit_class_id_table` | 0042b8f0 0047f190 004f1750 006d1610 006d1650 006dfe50 006e7c00 | The unnamed roots 0, 1, 2, 4, 5, 6, 15, 41 and the 86 `IsKindOf` implementations as one table, and the correction of the three headers listed above |
| `szurkenyil_objective_set` | 008df900 008da160 008ddf00 008dd310 008de*** | What populates the eight `SzurkeNyil` sets at `game+21A4h` and what "grey arrow" marks on the HUD |
| `unit_list_teardown` | 004daba0 004dd340 004ddde0 004c3c90 004d2bb0 | The three other functions that touch all eight heads, one of which is `BSP_Game_DestroyWorld`'s path |

## no_ghidra_function

Start and inclusive end (the last byte of the final `RET`). Every range was read from the disk image
through `bsp.py disasm-raw` or the byte dump, and every end is the last byte of the `C2 04 00` or
`C3` that closes the body; the following byte is `CC` padding or the next function's first byte.

| start | end | what it is | boundary evidence |
| --- | --- | --- | --- |
| `004F2590` | `004F25AB` | `__thiscall bool(unit)`, forwards to `008DDF90` | `004F25AB RET`; `004F25AC..004F25AF` are `CC` |
| `004F25B0` | `004F25CB` | an identical second copy | `004F25CB RET`; next byte begins `004F25D0` |
| `006E2790` | `006E27BB` | `IsKindOf` of the ordnance instance base (id 42) | `MOV EAX,1; RET 4` at `006E27B4..006E27BB` |
| `006E8400` | `006E840C` | `IsKindOf` of `MBullet`'s descriptor (id 41), `SETNZ` form | `006E840A RET 4`; `006E840D` is `CC` |
| `006E8410` | `006E8415` | `MBullet` name getter | `MOV EAX,00CFA15C; RET` |
| `006EA290` | `006EA295` | `MBomb` name getter | `MOV EAX,00CFA4D4; RET` |
| `006EA2A0` | `006EA2BA` | `IsKindOf` of `MBomb`'s descriptor (id 42) | `MOV EAX,1; RET 4` at `006EA2B3..006EA2BA` |
| `006D20E0` | `006D2115` | `IsKindOf` of the airfield instance (adds `45h`) | `MOV EAX,1; RET 4` at `006D210E..006D2115` |
| `006F5890` | `006F58C5` | `IsKindOf` of the `MLandFort` instance (id 27) | same tail form |
| `007004B0` | `007004E5` | `IsKindOf` of the `MDummyTarget` instance (id 53) | same tail form |
| `0074E400` | `0074E435` | `IsKindOf` of the plane instance base (id 15) | same tail form |
| `007CFD00` | `007CFD1F` | `IsKindOf` of the plane descriptor base (id 15) | same tail form |
| `007EFB00` | `007EFB2B` | `IsKindOf` of the squadron instance (id 24) | same tail form |
| `00846C00` | `00846C35` | `IsKindOf` of the shipyard instance (adds `46h`) | same tail form |
| `00749030` | `0074904F` | `IsKindOf` of the `MLandFort` descriptor (id 27) | same tail form |
| `009635E0` | `009635FF` | `IsKindOf` of the ship descriptor base (id 6) | same tail form |
| `00960030` | `0096004F` | `IsKindOf` of the `MLandVehicle` descriptor (id 25) | same tail form |
| `009600A0` | `009600BF` | `IsKindOf` of the `MDummyTarget` descriptor (id 53) | same tail form |
| `0095FF30` | `0095FF4F` | `IsKindOf` of the `MAirfield` descriptor (id 69) | `MOV EAX,1; RET 4` at `0095FF48..0095FF4F`; `0095FF50` is the next function's `PUSH ESI` |
| `0095FFA0` | `0095FFBF` | `IsKindOf` of the `MShipyard` descriptor (id 70) | same tail form; `0095FFC0` is the next function |

`006FE530..006FE56A`, the `MDestroyer` instance `IsKindOf`, is already recorded in
`docs/UNIT_INSTANCE_UPDATE.md` and is not repeated here.

## Correction from docs/OBJECTIVE_UNIT_LIST.md

Packet `cc2-mission-objectives` found that `008DDF90` is `BSP_SzurkeNyil_ContainsUnit` and that the
`+18h`/`+1Ch`/`+20h` offsets this packet declared as `kObjectiveSetTreeOffset` / `HeadOffset` /
`SizeOffset` belong to the marker object it walks, not to the objective set, whose own unit list
sits at `+24h`/`+28h` (16-byte `{unit, marker position}` records, read exactly once by `008DFE50`,
a `__thiscall` on the objective set, never a completion test). The three constants are renamed
`kUnitMarkerTreeOffset` / `kUnitMarkerHeadOffset` / `kUnitMarkerSizeOffset` in
`include/bsp/local_player_unit_lists.hpp` (no source used them); the objective list is declared in
`include/bsp/objective_units.hpp`.

## Correction from docs/RECON_SLOT_LISTS.md (packet cc2_recon_slot_lists)

- **Was:** 008073C0 coverage row 'partial: only the five 008042B0 clears at 008073D0..00807416 and the list-1-to-list-3 move at 00807634..008076E2 were read', with the follow-up packet unit_registry_five_lists asking what the five triples separate
  **Is:** answered and complete. The five triples are own (+DD8h), enemy (+DE4h), neutral (+DF0h), unknown (+DFCh) and the union of those four (+E08h); the 1-to-3 move is the detection drain that copies level-1 records into the unknown triple and removes anything below level 2 from the relation triple
  **Evidence:** the whole body 008073C0-008079A5 is read in docs/RECON_SLOT_LISTS.md section 3; 00806B10's four 00805D90 calls at 00806BE4, 00806C1A, 00806C50 and 00806C86 fix the names; 00807647 and 00807823 are the level compares. That doc's reading that 004C3CB0 walks triples 0, 1 and 3 and the HUD triple 4 is consistent and not superseded.

## The local slot and the rebuilds (packet `cc9_local_player_unit_list`, `kLocalPlayerUnitListRuleBound`)

Worker cc9-hud2, 2026-09-27, base main 6de0d8c6b. Ghidra was read only.

### The image's rule (V, listings)

**The source is the local player's slot.**
- 004C3CB0 walks the recon triples of the slot `[game+18CCh + [game+18ECh]*4]` (004C3CF8, 004C3D61,
  004C3EB4). That is the local player's own record, whose `+28h` is its team.
- The controlled unit plays no part in it.
- The host's stand-in used the side (`+54h`) of the controlled unit when the lists were built
  (`game_local_recon_triple`). On USN01 that unit is the Japanese Airfield2 until 20.1 s, so
  game+1970h held 26 Japanese units for the whole run (`docs/CONTROLLED_UNIT.md`, "Pairs and
  verdict").

**The latch `game+193Ch` is cleared in many places.**
- An operand scan of every `[reg+193Ch]` access finds 17 clears in the game class, not one. They
  are listed in the next table.
- The rebuild runs on the next in-mission tick (004C40B4) after any clear.

| site | function | condition |
| --- | --- | --- |
| 00807995 | 008073C0 `BSP_Recon_RebuildSlotLists`, its tail | the rebuilt slot's `+28h` equals the local slot's `+28h` (0080798D..00807993) |
| 00807393 | the same slot walk, earlier | the same team test (00807384..00807391) |
| 00803BD0 | 00803BA0 `BSP_Recon_MarkSlotDirty` | the marked team is the local slot's |
| 004C9E89 | 004C9CA0 `BSP_Game_ApplyInGameInterface`, mission entry | unconditional. It follows 008073C0 on the local slot (004C9E82), and then 004C3CB0 runs at once (004C9E8F) |
| 004E0360, 004E05A2 | 004DFB70 `BSP_Game_LoadMissionScene` | unconditional |
| 0077D295 | 0077D270 `BSP_UnitInstance_OnKilledLua`, the generic OnKilled | unconditional, first instruction after the prologue |
| 00779B1F | 00779AF0 `BSP_MissionEntity_OnKilledPlayerUnit` | not read |
| 007CA5CE, 007CC809 | 007CA3F0 plane touchdown or crash; 007CC7A0 plane flight state 2 | not read |
| 0045C460, 0045C470, 0045F725, 004C3C98, 006EDC1A, 007712F0 (at 0077134A), 007F1CA0 (at 007F1D25), 004D2C24 (world destroy) | | not read |

**Correction to `docs/IN_MISSION_SUBSYSTEM_TICK.md`**, "Call 2", and its corrections list.
- It said `game+193Ch` is cleared only by 004DFB70, so 004C3CB0 runs once per scene load.
- It is cleared at least at every local-team recon rebuild (every 3 s, `docs/RECON_SLOT_LISTS.md`)
  and at every kill, so the lists are rebuilt through the mission.

### The binding

- **Switch:** `kLocalPlayerUnitListRuleBound` in `include/bsp/game_hosts_world.hpp`, committed OFF.
- **The local slot.** `game_local_recon_triple` reads the recon slot of `kLocalPlayerParty` (0)
  instead of the controlled unit's side. That serves all three callers: 004C3CB0's walks, the
  minimap's union triple (005C1610) and the pick through the lists.
- **The latch.** Before each 004C3CB0 guard, the world host clears it (00807995 or 0077D295) in
  two cases:
  - when the gunnery host's 008073C0 pass counter has advanced and the pass covered party 0;
  - when the units host's destroyed-unit record has grown.
- **SUBSTITUTIONS, labelled:**
  - The local party is 0 in single player, as the HUD's 00645060 inputs take it.
  - The kill clear stands on the destroyed-unit record, not on the OnKilled call itself.
  - The other clears in the table stay unmodelled: mission entry's own 008073C0 and 004C3CB0, the
    plane events, and the unread sites. The first periodic pass stands for mission entry.

### Predictions (written before any run; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

**USN01 3200/3000:**
- **game+1970h** is the Allied side from the first build. It holds the party-0 units of the own
  triple: Enterprise, Northampton, SaltLakeCity, the four destroyers, the Allied forts and
  hangars, and the squadrons once they exist. No Japanese unit is in it.
- **It is rebuilt about every 3 s.** That is about 50 builds over the 150 s, plus one per kill;
  there are 7 deaths.
- **game+19B8h (merged)** fills from the enemy and unknown triples as the recon passes publish
  them. It was 0 in every OFF build.
- **`+8Ch` at the 44h enter after `SetSelectedUnit(Northampton)`** (frame 403) holds Northampton.
  SaltLakeCity and Dunlap follow Northampton, whose 00645060 accepts, so they are skipped. Any
  other party-0 ship is there only if its role word is 9 or 0. The cursor is (0, 0), on
  Northampton if it is entry 0. `+9Ch` stays 3.
- **The 29h pick.** Its `list=1970` census changes from the 26 Japanese units to the Allied list,
  and `list=19b8` becomes non-empty.
  - `owner_140` moves from 2,404, because the lock-radius walk sees different candidates. The
    direction is not predicted.
  - `ray_pick_other` and `ray_pick_own` (2,240 and 160) come from the segment query, not the
    lists. They stay unless a lock changes the camera, and nothing here does.
- **Minimap:** before 20.1 s the union triple is the Allied side instead of the Japanese one.
  This changes log lines only.
- **Gameplay identical, `pair_diff` exit 1.** The lists feed the HUD, the pick and the minimap.
  None of them reaches orders without input.

**USN04 4700/4500:**
- The Lexington is controlled from the start, so the side is already 0. game+1970h keeps the same
  members at the first build.
- It is rebuilt about every 3 s and on each kill, so launched squadrons enter it. game+19B8h
  fills from the enemy triples.
- `+8Ch` at both 44h enters stays {Lexington}.
- Pick `owner_140` (1) and the pick censuses may move. **Gameplay identical, exit 1.**

**USN02 9200/9000:**
- DeRuyter, then Houston, are party 0, so the side is unchanged. Only the rebuild cadence
  changes: game+1970h grows from its single 14-unit build as squadrons and new units appear, and
  game+19B8h fills.
- Pick rows may move. **Gameplay identical, exit 1.**

**Why gameplay should not move.** The host's only readers of the lists and the triple are in the
HUD (grep, base 6de0d8c6b):
- `src/game_hosts_hud.cpp`: the 29h pick and the 006485A0 lists;
- `src/game_hosts_hud_world.cpp`: the minimap union walk.

A move in gameplay would therefore mean one of these reaches a gameplay host, and would be
reported as a failed prediction.

### Pairs and verdict

The OFF side is this tree's build at d978ef2c0. The ON side is the `tools/pair_export.py` export
`local\lp_on` of the same commit with the switch flipped. Streams are ON, with
`BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | gameplay, 7 death rows and 28 unit rows identical. game+1970h has 15 Allied units (Dunlap, Ralph, McCall, Blue, Enterprise, Northampton, SaltLakeCity, then forts and hangars) instead of 26 Japanese. Builds 1 -> 57 (49 recon clears, 7 kill clears). Final counts 7/26/0/0/0/0/0/0 -> 7/15/0/0/1/0/8/9, so game+19B8h holds 9. `owner_140` 2,404 -> 2,400; `ray_pick_other`/`ray_pick_own` 2,240/160 unchanged |
| USN04 4700/4500 | exit 1 | gameplay, 44 death rows and 81 unit rows identical. Builds 1 -> 119 (74 recon, 44 kill). game+1970h 18 -> 34 at the end (squadrons entered). game+19B8h stays 0. `+8Ch` = {Lexington} at both 44h enters. `owner_140` 1 = 1 |
| USN02 9200/9000 | exit 1 | gameplay, 21 death rows and 32 unit rows identical. Builds 1 -> 171 (149 recon, 21 kill). Final counts 14/14/0/0/0/0/0/0 -> 3/3/3/0/0/0/0/3, as dead units leave. `+8Ch` holds 10 units at both 44h enters, cursor (0, 0) at Alden. `owner_140` 241 = 241 |

**Predictions.**
- **Held:** gameplay identity on all three pairs. USN01's list side and its rebuild count (about
  50, measured 57), with game+19B8h filling on USN01. USN04's `+8Ch`. The pick's ray rows
  unchanged.
- **Failed:**
  1. USN01's `+8Ch` at frame 403 holds **3** units with the cursor (0, 0) at **Dunlap**, not
     Northampton alone. Dunlap comes first in game+1970h and passes as a non-follower, or as a
     follower whose leader 00645060 rejects. So `ForceSelectUnit` on USN01 would now select
     Dunlap. At frame 1, with Airfield2 controlled, `+8Ch` = {Northampton}.
  2. USN04's game+19B8h does not fill: the enemy and unknown triples of party 0 stay empty there.
     On USN01 it filled to 9.
- **Unpredicted:** USN02's cursor sits at Alden, entry 0 of 10, not at the controlled
  DeRuyter/Houston.

**Verdict: ON** (`kLocalPlayerUnitListRuleBound = true`). The lists move on all three missions,
and gameplay moves on none.
