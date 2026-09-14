# Mount frame scale: closing the remaining local-frame producers

Addresses: `00491950`-`00491A9D` (`FUN_00491950`, the dispatch at `00491A30` and the
array index at `00491A15`-`00491A27`), `006F3660`-`006F38DE` (`FUN_006F3660`, the dispatch
at `006F3798` and the record base at `006F3772`-`006F3789`), `006F5CC0`-`006F5EC8`
(`FUN_006F5CC0`, the adoption pass; the matrix capture at `006F5E24`-`006F5E2F` and the
push-back at `006F5E85`-`006F5E92`), `006AF780`-`006AF86F` (`FUN_006AF780`, the savegame
restore; the Lua reader call at `006AF81C` and the dispatch at `006AF842`),
`004EA650`-`004EA753` (`BSP_SceneDatabase_CreatePath`, the `REP MOVSD` at
`004EA6C3`-`004EA6D0`), `004EA760`-`004EA863` (`004EA7D3`-`004EA7E0`),
`004B2030`-`004B21D5` (`FUN_004B2030`, the pose-local copy at `004B20D7`-`004B20E8` and
the cached inverse at `004B214C`-`004B215F`), `008206F0`-`00821E78` (`FUN_008206F0`, the
heading normalisation at `00821035`-`0082109C` and the basis build at
`00821810`-`00821940`), `00484BC0`, `00487A70`, `0048C6A0`, `00498790`, `0049B1F0`,
`007A5A00`, `007F4580`, `0089A8B0`, `00A32350`, `00935D30` (the identity fills),
`007AB590`-`007AB7C5` (no Ghidra function), `0082FE30`-`00831801`
(`BSP_ShipClass_BindModelData_Provisional`, the `"idle"` loop `00831430`-`008316FD`).

Constants: `00D7A24C` = float `1.0`, `00E0B68C`/`00E0B690`/`00E0B694` = the float triple
`(0, 1, 0)` (read-only; the only writer is `CG_static_init_00CCFFE0`), `00CF82A4` =
`"localFrame"`, `00CF82B0` = `"parentID"`, `00D099B8` = `"idle"`, `00CFB24C` =
`"MinLevel"`, `00CFB244` = `"LevelX"`, `00CE4780` = `"Type"`, `00CE5804` = `"Party"`,
`00CF8838` = `"Skill"`.

Read as a contract, **not** leased or reconstructed here: `00B63D50`
(`docs/MATRIX_ORTHOGONAL_INVERSE.md`), `00955630` and `0042D0D0`
(`docs/GUN_GRAVITY_ARC.md`), `0046CF40` / `0085DC80` / `009258F0` / `00924280`
(`docs/SCENE_ATTACH_LOCAL_FRAMES.md`), `00414DB0` (`docs/POSE_REFRESH.md`,
`docs/ENTITY_LOCAL_MATRIX.md`), `0071AD50` and the model-node subtree
(`docs/GUN_MOUNT_POSITIONS.md`, peer-owned - section 5).

## 0. The answer in four lines

1. **All 18 open dispatch sites are closed.** Eleven push a literal identity, two are
   verbatim forwarders, one pushes an orthonormal frame built from model geometry, one
   propagates an existing pose local, two propagate a scale that an earlier producer put
   there, and one builds its own basis that is orthogonal but not necessarily unit.
2. **The `.scn` `localframe` is still the only AUTHORING source of scale**, but it is no
   longer the only *path*. Three sites carry an existing scale into a new pose local; none
   of them multiplies a fresh scale in.
3. **`00491A30` cannot carry scale.** It indexes `shipClass+6ECh`, a
   `std::vector<Matrix44>` filled by `BSP_ShipClass_BindModelData_Provisional` from the
   model's `"idle"` marker polygons, each basis pushed through the Gram-Schmidt
   orthonormaliser `0085DC80` **with no re-multiply by a scale** - unlike the `.scn` parser,
   which re-multiplies at `0046D1BF`-`0046D222`.
4. **`006F3798` is the one that matters.** It respawns a capture point's garrison at the
   entity world matrix captured when the capture point adopted it, and the three kinds it
   adopts are `MLandFort` (`1Bh`), `MAirfield` (`45h`) and `MShipyard` (`46h`). `LandFort`
   is the kind that carries **7,075 of the 7,621 scaled `localframe` statements in this
   installation**, including all 105 `"Heavy AA, Japanese"` anti-aircraft emplacements.
   The garrison respawn therefore reproduces the scaled mount frame of a gun-bearing
   entity; it does not repair it.

## 1. The dispatch census, closed

`docs/SCENE_ATTACH_LOCAL_FRAMES.md` section 3 left fifteen rows marked *construction not
read - partial*, plus `00491A30` and `006F3798` marked *array producer not traced* and
`009362FE` marked *not located*. All eighteen are below. Every argument role is read from
the push order against `009258F0`'s `RET 0Ch` and the role table in that document
(`arg1` hierarchy parent, `arg2` world node, `arg3` local frame).

| Call site | Containing function | Construction read | Verdict |
| --- | --- | --- | --- |
| `004EA6FB` | `BSP_SceneDatabase_CreatePath` `004EA650` | `004EA6C3 MOV ESI,[ESP+6Ch]; MOV ECX,10h; LEA EDI,[ESP+18h]; REP MOVSD` | **forwarder** - a verbatim 16-dword copy of its own matrix parameter |
| `004EA80B` | `FUN_004EA760` | `004EA7D3`-`004EA7E0`, the same four instructions | **forwarder** |
| `006AF842` | `FUN_006AF780` | `006AF81C CALL 00BD6830` with the descriptor pair `{7, [ESP+28h]}` and the terminator `{0, 00CF82A4 "localFrame"}`; the sibling pair is `{4, ...}` / `"parentID"` | **can carry scale** - savegame round-trip, section 3 |
| `0082197D` | `FUN_008206F0` | `00821810`-`00821940`, section 4 | **orthogonal, not necessarily unit; can have two zero rows** - partial |
| `007F48BA` | `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | 16 `MOVSS` at `007F4830`-`007F48A5` into `[ESP+64h]` | **identity** |
| `004B21B3` | `FUN_004B2030` | `004B20DC LEA ESI,[EBX+74h]; MOV ECX,10h; LEA EDI,[ESP+2Ch]; REP MOVSD` | **propagation** of the source entity's pose local, section 3 |
| `00484C7D` | `FUN_00484BC0` | 16 `MOVSS` at `00484C0A`-`00484C64` into `[ESP+4]` | **identity** |
| `00487F16` | `FUN_00487A70` | `00487E9D`-`00487EFA` into `[ESP+44h]` | **identity** |
| `0048C819` | `FUN_0048C6A0` | `0048C7A8`-`0048C802` into `[ESP+28h]` | **identity** |
| `00498A61` | `FUN_00498790` | `004989F1`-`00498A4B` into `[ESP+38h]` | **identity** |
| `0049BD9D` | `FUN_0049B1F0` | `0049BD0F`-`0049BD81` into `[ESP+60h]` | **identity** |
| `007A5EB3` | `FUN_007A5A00` | `007A5E47`-`007A5EA1` into `[ESP+34h]` | **identity** |
| `007AB672` | no Ghidra function, `007AB590`-`007AB7C5` | `007AB607`-`007AB661` into `[ESP+1Ch]` | **identity** |
| `0089AB8E` | `BSP_LuaBinding_SetFireTarget` | `0089AB11`-`0089AB6E` into `[ESP+44h]` | **identity** |
| `00A32433` | `BSP_AiController_Create` | `00A323BA`-`00A32414` into `[ESP+8]` | **identity** |
| `00491A30` | `FUN_00491950` | `00491A15`-`00491A27`, element `EBP` of the 40h-stride vector at `[[this+20h]+538h]+6ECh`; producer in section 2 | **orthonormal - cannot carry scale** |
| `006F3798` | `FUN_006F3660` | `006F3772`-`006F3789`, `record+18h` of the 64h-stride array at `[this+778h]`; producer in section 3 | **can carry scale** |
| `009362FE` | `FUN_00935D30` | `00936253 LEA ECX,[ESP+78h]; PUSH ECX` then 16 `MOVSS` at `00936258`-`009362DC` | **identity** |

### 1.1 What "identity" means here, and how it was checked

Every identity row is the same shape as `00823CAD` in the merged packet: `XORPS XMM0,XMM0`
then `MOVSS XMM1,[00D7A24C]` (the float `1.0`), then sixteen `MOVSS` that write `XMM1` at
element 0, 5, 10 and 15 and `XMM0` at the other twelve, into the buffer the third push
addresses. **Register provenance was established by filtering each whole function listing
for `XMM0` and `XMM1`**, not by reading the two instructions next to the fill, because five
of these functions write `XMM0`/`XMM1` from entity fields earlier in the body
(`00487A70` at `00487CDC`/`00487CEF`/`00487CFF`/`00487DA0`/`00487DC9`, `0049B1F0` at
`0049BBA2`/`0049BBB3`/`0049BC4F`/`0049BC78`, `007A5A00` at
`007A5C5F`/`007A5C6D`/`007A5C7B`, `00935D30` at `00936073`/`00936086`). In every case the
**last** write to each register before the fill is the zero/one pair, and in every case
that pair is the instruction immediately preceding the `LEA`/`PUSH` of the buffer.

`007AB672` has no Ghidra function. Its routine is `007AB590`-`007AB7C5` **inclusive**; the
entry is `007AB590 SUB ESP,8Ch` immediately after the `INT3` run that ends at `007AB58F`,
and the final instruction is the three-byte `RET 4` (`C2 04 00`) **at** `007AB7C3`, so
`end_exclusive` is `007AB7C6`, followed by `INT3` padding. Read from the disk listing with
`disasm-raw`; Ghidra's `FUN_007AB230` ends at `007AB32C` and the next function starts at
`007AB7D0`.

`009362FE`'s third push is `00936253 LEA ECX,[ESP+78h]; 00936257 PUSH ECX` - 171 bytes back
from the call, which is why the merged packet's 160-byte window missed it. The matrix
occupies `[ESP+7Ch]`-`[ESP+0B8h]` **after** that push, i.e. the same buffer.

## 2. `00491A30`: the ship class's `"idle"` frames are orthonormal

`FUN_00491950(this, index)` creates one sub-entity of a per-unit collection and attaches it:

```
00491A15  MOV ECX,[EDI]                 ; the created node; EDI = &this->nodes[index]
00491A17  MOV EDX,[ECX]                 ; its vtable
00491A19  MOV EDX,[EDX+98h]
00491A1F  MOV EAX,EBP                   ; EBP = index
00491A21  SHL EAX,6                     ; * 40h
00491A24  ADD EAX,[EBX+4]               ; EBX = [[this+20h]+538h] + 6ECh, so [EBX+4] = +6F0h
00491A27  PUSH EAX                      ; arg3 = &matrices[index]
00491A28  MOV EAX,[this+20h]
00491A2B  MOV EBX,[EAX+30h]
00491A2E  PUSH EBX                      ; arg2 = the owner's world node
00491A2F  PUSH EAX                      ; arg1 = the owner entity, the hierarchy parent
00491A30  CALL EDX
```

The bound check at `00491A00`-`00491A0E` reads `[EBX+8] - [EBX+4]` and shifts right by 6, so
the element stride is `40h` - a 4x4. `[this+20h]` is the owner entity; `+538h` is its class
pointer (independently confirmed at `006F509F MOV EAX,[ESI+538h]; FLD [EAX+190h]` in
`FUN_006F4D10` and at `0082193A`/`008217AA` in `FUN_008206F0`). The class field at `+6F0h` is
the `_Myfirst` of a secure-SCL `std::vector` whose object starts at `+6ECh`; `+6F0h`/`+6F4h`/
`+6F8h` are nulled together in `BSP_ShipClass_ConstructBase` at
`009634C8`-`009634D4` and freed in `CG_vector_deleting_dtor_00963600`.

**The producer.** `BSP_ShipClass_BindModelData_Provisional` `0082FE30` runs a loop over the
model markers named `"idle"` (the string `00D099B8`, pushed with length 4 at
`00831430`-`00831432`) and, for each:

```
008314CD  XORPS XMM0,XMM0 / MOVSS XMM1,[00D7A24C]   ; then 16 MOVSS -> identity at [ESP+88h]
008315A8  MOVSS [ESP+0C0h],XMM0 ... [ESP+0C8h]      ; translation row  = vertex[0]
008315DB  FLD [EAX];  FSUB [EBP]  ...               ; row 2            = vertex[2] - vertex[0]
008316AB  CALL 004F9B30 (BSP_Vector3f_Cross)        ; row 1            = the cross product
008316E0  LEA ECX,[ESP+88h]; CALL 0085DC80          ; orthonormalise
008316E5  LEA EAX,[ESP+88h]; PUSH EAX
008316ED  LEA ECX,[EDI+6ECh]; CALL 0082CA40         ; push_back, stride 40h
```

`0082CA40` is `std::vector<Matrix44>::push_back`: it compares `((last - first) >> 6)` against
`((end - first) >> 6)` and, on the fast path, constructs in place and does `last += 40h`.

`0085DC80` is read here **only as the contract `docs/SCENE_ATTACH_LOCAL_FRAMES.md` already
proved**, and its tail confirms the part this packet depends on: it scales
`param_1[8..10]` by a reciprocal length, Gram-Schmidts one of the other two rows against it
and calls `BSP_Vector3f_Normalize`, then takes a cross product for the third and normalises
that too. All three basis rows leave unit length.

**The decisive difference from the `.scn` path.** The `.scn` parser calls the same
orthonormaliser at `0046D1BA` and then **multiplies the nine basis elements by
`s = |authored row 0|` again** at `0046D1BF`-`0046D222`. `0082FE30` does not: `008316E0`
is followed immediately by `008316E5 LEA EAX,[ESP+88h]` and the push_back. So
`shipClass->idleFrames[i]` is a pure rotation plus translation, for every ship class,
always. The only way it degenerates is an authored marker triangle with a zero
`vertex[2] - vertex[0]`, which would divide by zero inside `0085DC80` - not read here.

**Can these carry guns?** No. The consumer `FUN_00494F30` (the only caller of `00491950`)
runs a per-frame camera-distance test - it reads `matrices[i]+30h` as a point, transforms it
with `BSP_Vector4f_Transform` against the owner's matrix at `+74h`, subtracts the camera
position `[cam+0FCh..104h]` and compares the squared distance against a threshold derived
from `[[00E188A8]+19FCh]+1C4h`, creating the sub-entity when near and calling
`BSP_MissionEntity_Kill` when far. These are camera-LOD decoration actors keyed to the
`"idle"` markers, not weapon mounts. **Partial:** the two factory vectors at `this+44h` and
`this+54h` that supply the actor type were not traced, so "decoration" is inferred from the
marker name and the LOD consumer, not from the factory.

## 3. `006F3798`: the capture point's garrison keeps the scale it was placed with

`FUN_006F3660(this)` walks a 64h-stride record array at `this+778h`, count at `this+77Ch`
(the stride is proven by `006F506A ADD EDI,64h` against `006F506D CMP EBP,[ESI+77Ch]` in the
sibling pass `FUN_006F4D10`). For each record whose `+5Ch` value clears the level at
`this+770h` it creates an entity through `BSP_VehicleClass_GetOrCreate` `00964790` and the
class vtable's `+28h` creator, then:

```
006F3772  MOV ECX,[ESI+778h]
006F3778  LEA EAX,[EDI+ECX*1]      ; the record
006F377B  MOV ECX,[EAX+14h]        ; the entity just created - this
006F377E  MOV EDX,[ECX]
006F3780  MOV EDX,[EDX+98h]
006F3786  ADD EAX,18h              ; &record->matrix
006F3789  PUSH EAX                 ; arg3 = local frame
006F378A  MOV EAX,[00E188A8]; MOV EAX,[EAX+19CCh]
006F3795  PUSH EAX                 ; arg2 = the world root
006F3796  PUSH 0                   ; arg1 = NO hierarchy parent
006F3798  CALL EDX
```

Afterwards it re-reads `record+58h`, a cloned property bag, for `"Party"` (`00CE5804`),
`"Skill"` (`00CF8838`) and `"Type"` (`00CE4780`) through `BSP_ScenePropertyBag_Find`.

**The producer is `FUN_006F5CC0`.** It walks the world entity list at
`[[00E188A8]+19CCh]+58h`, keeps every entity that answers true to the vtable `+5Ch` kind
predicate for `1Bh`, `45h` or `46h` (`006F5D10`, `006F5D1F`, `006F5D2E`), is within
`[this+7C8h]` squared metres and has a non-null property bag at `+724h`, and then:

```
006F5DEF  MOV [ESP+40h],00CEDDA0        ; the record's vtable; record base = [ESP+40h]
006F5E0B  MOV [ESP+54h],EDI             ; record+14h = the adopted entity
006F5E1D  MOV ECX,EDI; CALL 00414DB0    ; make its world matrix current
006F5E24  LEA EAX,[EDI+0CCh]            ; its WORLD matrix
006F5E2A  PUSH EAX
006F5E2B  LEA ECX,[ESP+5Ch]             ; = record+18h once the push is accounted for
006F5E2F  CALL 004134F0                 ; BSP_Matrix_Copy4x4X87 - 64 bytes, verbatim
006F5E3A  CALL 008F41F0                 ; clone the property bag   -> record+58h
006F5E4D  CALL 008F2260 ("MinLevel")    ->                            record+5Ch
006F5E70  CALL 008F2260 ("LevelX")      ->                            record+60h
006F5E85  LEA ECX,[ESI+778h]; CALL 006F4B30   ; push_back, stride 64h
```

`[EDI+0CCh]` is the entity world matrix and `[EDI+0C8h]` its valid byte: `00414DB0`
(`BSP_EntityPose_RefreshWorld`) reads the pose local at `+74h`, multiplies it by
`[[entity+3Ch]+0CCh]` with `BSP_Matrix_Multiply4x4` when a hierarchy parent exists, copies
the result to `+0CCh` with `BSP_Matrix_Copy4x4X87`, and **never normalises anything**.

So the record stores the *world* matrix, and the respawn hands it back as a *local* frame
under the world root - correct arithmetic, because the world root's own frame is identity,
and therefore **exactly scale-preserving**. It adds no scale and removes none.

`004B21B3` is the same shape one level lower: `FUN_004B2030` allocates a 19Ch-byte object
(vtable `00CE71A8`, kind `3Bh` written at `004B209E`) and attaches it with
`004B20DC LEA ESI,[EBX+74h]; MOV ECX,10h; LEA EDI,[ESP+2Ch]; REP MOVSD` - the source
entity's pose local copied verbatim. When the source has no hierarchy parent
(`004B20D7 MOV EAX,[EBX+3Ch]` zero) the translation row alone is re-expressed in the new
parent's frame through `00B63D50`'s cached inverse at `node+110h` (`004B214C`-`004B215F`,
valid byte `+10Ch`) and `00439820`; the **basis is never touched**. This is also an
independent confirmation of `include/bsp/gun_gravity_arc.hpp`'s claim that the cached
affine inverse lives at `+110h` and is written by `00B63D50` from `+0CCh`.

`006AF842` is the savegame leg. `FUN_006AF780` builds a Lua reader over `_savedata` /
`_entities` (`009238A0`, then `BSP_LuaReader_Construct` `004425C0`) and drives `00BD6830`
with a descriptor list whose last pair is `{0, 00CF82A4}` = `"localFrame"`; the type-7
target is the buffer at `[ESP+20h]` that `006AF82F` then pushes as `arg3`. Whatever a saved
game holds for `localFrame` becomes the restored entity's pose local unchanged. **Partial:**
the writer side of the savegame (which field of the live entity is serialised into
`localFrame`) was not read, so this is "round-trips an arbitrary matrix", not "round-trips
`+74h`" - although `+74h` is the only 64-byte local frame an entity has.

### 3.1 Which entities, and how many, in this installation

`006F5CC0` adopts kinds `1Bh`, `45h`, `46h`. `docs/ENTITY_CLASS_IDS.md` rows 198, 240 and
241 name them `MLandFort`, `MAirfield` and `MShipyard`; the capture point itself is
`MCommandBuilding` (`1Ch`, parent `1Bh`), and `BSP_CommandBuilding_Construct` `006F5610`
calls `BSP_LandFort_Construct` `00745940`, so a capture point is a LandFort.

`local/cc7/scan_garrison.py` re-scans every `.scn` in this installation, attributing each
`localframe` to the `entity "<name>" (<Kind>)` block that contains it and computing
`s = |row 0|` the way `docs/SCENE_ATTACH_LOCAL_FRAMES.md` section 6 does:

```
.scn files                    259      containing a CommandBuilding      105
LandFort localframes       72,008      with |s - 1| > 1e-3            7,075
Airfield localframes          254      with |s - 1| > 1e-3                0
Shipyard localframes          241      with |s - 1| > 1e-3                0
scaled LandForts in a file that also has a CommandBuilding             4,893
gun-named scaled LandForts    108      of which "Heavy AA, Japanese"    105
```

The gun-named scale histogram is `0.7069` x 91, `0.7068` x 7, `0.8156` x 7
(`"Medium Bunker 03, Concrete"`), `0.8634` x 3. So **7,075 of the 7,621 scaled `localframe`
statements in the whole game - 93 percent - sit on the one kind the garrison path adopts**,
and the 105 `"Heavy AA, Japanese"` emplacements the merged packet already identified are
inside that set.

Two `.scn` files fail this parser on a malformed float token (`1.-`):
`universe/scenes/missions/ijn/ESMP/10_san_jose.scn` and the `usn/LOMP` file of the same
name. Their rows are excluded from the counts above, so the counts are lower bounds.

**This installation is modded** (BSPRM/AlterBSP). Of the 259 `.scn` files, 258 carry mtimes
between 2024-07-13 and 2025-06-02 and exactly one carries 2026-05-09, the local
modification date. Say "this installation", not "retail".

## 4. `0082197D`: the one site that builds its own basis

`FUN_008206F0` is the landing-operation spawn (string immediates `"CommandBuilding"`,
`"LandingPoint"`, `"LandingShip"`, `"SpawnPhase"`, `"LandingCommanderPlayer"`). It has no
callers in the index - it is reached through a vtable.

The matrix at `[ESP+0C0h]` is assembled, not copied:

| Element | Value | Site |
| --- | --- | --- |
| row 1 = `f4,f5,f6` | `[00E0B68C]`, `[00E0B690]`, `[00E0B694]` = `(0, 1, 0)` | `0082180A`-`00821846` |
| row 2 = `f8,f9,f10` | `([ESP+28h], 0, [ESP+34h])` - a horizontal heading | `008218A1`-`008218C5` |
| row 0 = `f0,f1,f2` | the x87 cross product of rows 1 and 2 | `008218D4`-`00821913` |
| `f3,f7,f11` | `0` | `00821868`, `00821873`, `0082187E` |
| translation `f12..f14` | `[EAX]`, `[EAX+4]`, `[EAX+8]`, `EAX = [ESP+4Ch] + index*0Ch` | `0082191A`-`00821940` |
| `f15` | `1.0` | `0082188E` |

The `PUSH 0` at `008218CE` is the creator's argument, consumed by the `__thiscall` at
`00821954`; that is how `[ESP+0C4h]` at `008218FD` and `[ESP+0C0h]` at `00821966` are the
same slot, `f0` and the base.

The globals are read-only data: `00E0B684..00E0B697` holds
`00 00 00 00 | 00 00 00 00 | 00 00 00 00 | 00 00 80 3F | 00 00 00 00 | 00 00 00 00`, so
`(0, 1, 0)` exactly, and the only other reference in `.text` is `CG_static_init_00CCFFE0`.

Because row 1 is unit and row 2 has a zero y, the two are orthogonal and
`|row 0| = |row 1 x row 2| = |row 2|`. **The basis is always orthogonal, and its scale is
`sx = sz = |(h0, h1)|`, `sy = 1` - uniform only when `|(h0, h1)| = 1`.** The pair is
normalised at `00821035`-`0082109C`: `x*x + z*z`, `CALL 00BF7030` (the CRT square root),
and then

```
00821050  JBE 0082105F        ; length <= 0
00821052  FLD1; FDIVRP        ; reciprocal
00821056  FSTP [ESP+88h]
0082105F  XORPS XMM0,XMM0     ; the degenerate branch stores ZERO, not infinity
00821064  MOVSS [ESP+88h],XMM0
```

so on the normal path the frame is orthonormal and on the degenerate path **rows 0 and 2
are both the zero vector** - the exact divide-by-zero input `00B63D50` has no guard for.

**Partial, and stated as partial.** `[ESP+34h]`, one half of the heading pair, is written
twice: at `0082109C` by the normalisation above and again at `00821771` inside the placement
loop, where it takes an affine-transform result (`FLD [EDI+18h] ... FADD [EDI+38h]`) and is
copied on to `[ESP+114h]`. Which of the two reaches `008217E3 FLD [ESP+28h]` on each of the
two paths into `0082180A` (the `JMP` at `008217E7` and the fall-through at `00821808`) was
**not settled**. If the second write reaches it, the frame carries a non-uniform scale
`sx = sz != 1 = sy` from shipped landing-point geometry. If only the first does, the frame
is orthonormal except in the zero case. This is the single unclosed sub-question of the
packet.

## 5. The model node, and where this packet stops

`docs/GUN_MOUNT_POSITIONS.md` establishes the gun's origin as
`TransformAffinePoint(class->muzzleOffsets[gun+44Ch], (gun+3CCh)->worldMatrix)`, with
`gun+3CCh` resolved once at setup to the first non-null of the model nodes named
`"barrel"` (`00CF70C4`) and `"base"` (`00CFACD8`) and the model's own first node, through
`0071AD50` on the model at `[[gun+360h]+160h]`. That document proves the node's world matrix
sits at `node+0F0h` (the fallback branch reads the translation at `node+120h`/`+124h`/
`+128h`, which is `+0F0h + 30h`).

**`node+0F0h` is a different class from the entity's `+0CCh`.** This packet establishes the
boundary and does not cross it:

* The entity chain is fully known and unnormalised. `00414DB0` composes
  `world(+0CCh) = local(+74h) * parent(+3Ch)->world(+0CCh)` with `BSP_Matrix_Multiply4x4`,
  which is a plain 4x4 multiply. A scale anywhere on that chain reaches the entity world
  matrix intact.
* The model-node chain is **not** composed by any of the 149 callers of
  `BSP_Matrix_Multiply4x4` - none of them writes a matrix at `+0F0h`. A byte scan of the
  four `ADD reg,0F0h` encodings (`81 C0/C1/C2/C3/C6/C7 F0 00 00 00`) across `.text` finds
  only **consumers** of `node+0F0h`: `00730160 BSP_Gun_Fire` at `007301D1`,
  `0072AA80 BSP_PointEffect_SetRelativeMatrix` at `0072AAC4` (which multiplies a relative
  matrix by the node world, again with `BSP_Matrix_Multiply4x4` and again with no
  normalisation), `0070F720 BSP_UnitPartCollisionShape_TestSphere` at `0070F752`,
  `00707B00` at `00707B85` and `00725550` at `0072556B`. The producer uses a different
  addressing form and lives inside the model/scene/resource subsystem.

**The contract, as far as it is proven:** a gun's shot origin is a point in the *model
node's* frame, the node's world matrix is consumed unnormalised by at least two independent
consumers, and nothing between the entity frame and the node frame normalises. So the entity
frame's scale is necessarily present in the node world matrix. Whether the model subtree
adds a scale **of its own** on top of it is the open question.

A follow-up **inside the peer's territory** would have to answer exactly three things:

1. Where `node+0F0h` is written - the routine that composes a model node's world matrix -
   and whether it multiplies by a parent world the way `00414DB0` does.
2. Whether a model node carries its own local matrix loaded from the model file, and
   whether that local can be a non-similarity (per-axis scale, shear) rather than the
   similarity the `.scn` parser is restricted to.
3. Whether the model instance at `[[gun+360h]+160h]` carries a whole-model scale factor
   applied at the root of the node hierarchy - the natural place for an authoring tool to
   put one, and the one that would multiply with the `.scn` `s` rather than replace it.

Until (1) is read, **the safe assumption is that a gun mount node CAN be scaled
independently of its entity frame**, because nothing on the path removes a scale and the
one composition step that is readable (`0072AAC4`) is a plain multiply.

## 6. What this means for the reconstruction

`src/game_hosts_gunnery.cpp` today gives every gun one shared hull origin raised by the
class `Height`. `docs/GUN_MOUNT_POSITIONS.md` already refuted the `Height` half. The
consequence of this packet is the frame half: **the placeholder has no mount frame at all,
so the scale defect is unreachable in the reconstruction today**, and the moment a real
mount frame is introduced it becomes reachable in the same shipped data that triggers it
natively.

A host that builds real mount frames must:

1. **Use the model node's world matrix**, `(gun+3CCh)->worldMatrix` at `node+0F0h`, as the
   frame for both the origin and the arc - not the unit's pose local and not the unit's
   world matrix. That is what `BSP_Gun_Fire` reads at `007301BD`-`007301E0`.
2. **Not normalise it, if the goal is native parity.** Native never does: `00414DB0`,
   `0072AA80` and `00B63D50` all take the matrix as given, and `0042D0D0` is called from
   `00955630` with `normalize = 0`. Normalising would produce *correct* gunnery and
   *divergent* gunnery; those are different goals and the choice has to be explicit.
3. **Know the frame scale.** `mount_frame_scale` returns the three row lengths and the
   worst off-diagonal; `mount_frame_is_invertible_by_00b63d50` is the guard the native
   inverse does not have; `mount_frame_arc_is_exact` is the stricter predicate the arc
   needs, because a uniform scale passes the first and fails the second.

**Where the error enters.** `00955630` solves the arc in world space, transforms the solved
direction into the mount frame through `0042D0D0` with `normalize = 0`, and `00521370` takes
`asin` of the raw `y`. With a uniform mount scale `s` the reported elevation is

```
reported = asin( sin(true) / s )
```

which `include/bsp/mount_frame_scale.hpp` publishes. It reproduces both figures the merged
packet derived independently - `45.017` degrees for a true `30` at `s = 0.7069` and `35.266`
degrees at `s = 0.866` - and it exposes a failure that document did not state:

| `s` | true 10 deg | true 30 deg | true 45 deg |
| --- | --- | --- | --- |
| `0.7069` (105 AA emplacements) | 14.220 | 45.017 | **domain error** |
| `0.8156` | 12.293 | 37.810 | 60.109 |
| `0.8660` (`'L Hancock'`) | 11.567 | 35.266 | 54.738 |
| `1.2054` | 8.283 | 24.507 | 35.917 |
| `1.5663` | 6.365 | 18.616 | 26.837 |
| `3.0` (the shipped maximum) | 3.318 | 9.594 | 13.633 |
| `0.1586` (the shipped minimum) | **domain error** | **domain error** | **domain error** |

For `s < 1` there is a critical elevation `asin(s)` - `44.98` degrees at `s = 0.7069`,
`9.13` degrees at `s = 0.1586` - above which `sin(true)/s` leaves `[-1, 1]` and the native
`asin` at `00521370` is a domain error, not a clamp. The published function returns a quiet
NaN there rather than clamping, because the native code has no clamp either. **This is a
prediction from the formula, not an observed run:** `bsp_game.exe` does not reach
`00955630`, so rule 6 of the checklist does not apply and no run log supports it.

## 7. What is proven, and what is assumed

**Proven, from the listing:**

* The construction behind all 18 dispatch sites, with the sites listed in section 1.
* `shipClass+6ECh` is a `std::vector<Matrix44>`, filled only by the `"idle"` loop in
  `0082FE30` through `0085DC80` with no scale re-multiply, and read by `00491950` with a
  `40h` stride.
* The garrison record layout (`+14h` entity, `+18h` matrix, `+58h` property bag, `+5Ch`
  MinLevel, `+60h` LevelX, stride `64h`), its producer `006F5CC0`, and that the matrix is a
  verbatim 64-byte copy of the adopted entity's world matrix at `+0CCh`.
* `00414DB0`'s composition and its lack of normalisation, read here in full.
* `(0, 1, 0)` at `00E0B68C` and the zero substitution at `0082105F`-`00821064`.
* The shipped-data counts in section 3.1, from this installation's own `.scn` files.

**Assumed or inferred, and labelled as such:**

* That `00491950`'s sub-entities are decoration rather than weapons. Inferred from the
  `"idle"` marker name and the camera-LOD consumer; the two factory vectors at `this+44h`
  and `this+54h` were not traced.
* That `006AF780`'s savegame `localFrame` is the entity's `+74h`. The serialiser was not
  read; only that the deserialised matrix becomes the pose local.
* That the model node can scale independently of its entity frame. Section 5 - the
  producer of `node+0F0h` was not found, so this is the *safe* assumption, not a proof.
* The domain-error column of the table in section 6 is arithmetic on the published
  formula, not an observed native failure.

**Coverage.** `FUN_008206F0` is `partial`: the basis build at `00821810`-`00821940` is read
in full, the heading normalisation at `00821035`-`0082109C` is read in full, and the
`[ESP+34h]` reaching-definition question at `00821771` is open. Every other routine in
section 1 is `complete` for the question asked (what reaches `arg3`); none of them is
reconstructed, so none carries a coverage claim beyond that.

**Ghidra was read-only for this packet.** No renames, comments, prototypes, function
creation or saves. `00B63D50`, `00955630`, `0046CF40`, `0085DC80` and `009258F0` were read
as contracts and are not leased. Leased: `00491950`, `00491A30`, `006F3660`, `006F3798`,
`006F5CC0`, `006AF780` - the six addresses this packet names in the ledger.

## 8. Follow-up packets

1. **`model_node_world_matrix`** (peer territory - the model/scene/resource orchestrator).
   The three questions in section 5. This is the last thing standing between this packet
   and a complete answer to "can a gun mount be scaled".
2. **`landing_ship_heading_reaching_def`**. Settle the `[ESP+34h]` question in
   `FUN_008206F0` (section 4) and, if the second write reaches `008217E3`, audit the
   shipped `LandingPoint` geometry for the resulting non-uniform scale.
3. **`savegame_entity_serialiser`**. Read the writer that produces the `_savedata`
   `localFrame` key consumed at `006AF81C`, to upgrade section 3's savegame leg from
   "round-trips an arbitrary matrix" to a proven `+74h` round trip.
4. **`ship_class_idle_actors`**. Trace the factory vectors at `FUN_00491950`'s `this+44h`
   and `this+54h` to replace the inference in section 2 with the actual actor kinds, and
   check whether an authored `"idle"` triangle can be degenerate.
5. **`mount_frame_host`**. Replace `src/game_hosts_gunnery.cpp`'s shared hull origin with a
   real per-gun mount frame, using `include/bsp/gun_mount_positions.hpp` for the origin and
   `include/bsp/mount_frame_scale.hpp` for the guard, and decide explicitly whether the
   reconstruction reproduces the native `asin(y/s)` defect or corrects it.
