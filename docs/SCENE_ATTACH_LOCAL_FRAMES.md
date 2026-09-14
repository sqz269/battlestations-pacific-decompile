# Scene attach: where a pose local frame comes from

Addresses: `009258F0`-`009259FA` (`BSP_SceneNode_AttachToParents`, the `009259C4` copy into
`+74h`), `00928860`-`00928914` (`BSP_GameEntity_PlaceInWorld`), `00929C80`-`00929CAB` (no Ghidra
function), `006DFE40`/`0074E180`/`007EFA70` (ICF thunks), vtable slot `+98h`, the dispatch census
listed in section 2, `0046CF40`-`0046D927` (`BSP_SceneFile_ReadEntityBlock`, the `localframe`
parse at `0046D123`-`0046D222` and the creator call at `0046D57E`-`0046D5A4`), `0085DC80`-`0085DD30`
(the Gram-Schmidt orthonormaliser), `00924280`-`009242BC` (the 16-float property copy),
`00823CAD`-`00823D3D`, `007438A1`-`007438BD`, `007F48BE`-`007F48D2`, `004B219E`-`004B21B3`,
`00491A15`-`00491A30`, `006F3778`-`006F3798`.

Constants: `00CE3820` = double `1e-10`, `00D0D0A0` = double `0.9990000128746033`,
`00D7A208` = float `-0.0`, `00D7A24C` = float `1.0`, `00CEA008` = `"HierarchyMatrix"`,
`00CEA018` = `"HierarchyParent"`, `00CEA028` = `"SetHierarchy"`, `00CE5840` = `"localframe"`.

## 0. The question and the answer

`docs/MATRIX_ORTHOGONAL_INVERSE.md` proved that `00B63D50` is a true affine inverse **iff** the
three basis rows are mutually orthogonal and non-zero, that it is branch-free so there is no guard,
and that even a *correct* inverse with a uniform scale `s` makes the gun read `asin(y/s)` instead of
`asin(y)` because `0042D0D0` runs with `normalize = 0`. That packet surveyed the producers of a pose
local and found none that introduces scale, but left one link unread: `009259C4` copies a
caller-supplied `localFrame` from `[ESP+14h]` into `+74h` unvalidated, and the attach callers were
never enumerated. It therefore recorded a **bounded negative**, not a proof.

**The bounded negative is wrong.** A non-orthonormal local frame *does* reach `entity+74h` in the
shipped game, and it reaches gun-bearing units. The authored source is the `localframe` statement
that every entity in a shipped `.scn` carries. `BSP_SceneFile_ReadEntityBlock` canonicalises it, and
the canonicalisation **removes shear and non-uniform scale but deliberately preserves a uniform
scale**: the frame that leaves the parser is `s*R + t` with `R` orthonormal and
`s = |authored row 0|`. Across the 259 shipped `.scn` files, 7,621 of 133,655 `localframe`
statements carry `s` outside `1 +/- 1e-3`, `s` ranging from **0.1586 to 3.0**, and 105 of them are
`LandFort` entities whose names are anti-aircraft emplacements (`"Heavy AA, Japanese 01"`,
`s = 0.7069`). Two further frames are degenerate: their authored row 2 is the zero vector, and the
orthonormaliser's zero guard propagates that into **two zero basis rows**, the divide-by-zero case.

So the arc's precondition is violated by shipped data, in the restricted form the merged packet
already showed to be harmful.

## 1. `BSP_SceneNode_AttachToParents` and the argument in question

`__thiscall(node /*ECX*/, hierarchyParent, worldNode, localFrame)`, `RET 0Ch`, body
`009258F0`-`009259FA`. Three stack arguments; the `RET 0Ch` is the count, not the pushes.

The prologue is `PUSH EBX; PUSH ESI` only, so at `00925902` the incoming arguments sit at
`[ESP+0Ch]`, `[ESP+10h]`, `[ESP+14h]`. The body confirms the roles by what it stores:

| Site | Instruction | Meaning |
| --- | --- | --- |
| `00925902` | `MOV EAX,[ESP+10h]` then `MOV [ESI+30h],EAX` | arg2 is the world node, kept at `+30h` |
| `0092592F` | `MOV EAX,[ESP+0Ch]` then `MOV [ESI+3Ch],EAX` | arg1 is the hierarchy parent, kept at `+3Ch` |
| `009259BB` | `MOV EAX,[ESP+14h]`; `PUSH EDI`; `PUSH EAX`; `LEA ECX,[ESI+74h]`; `009259C4 CALL 004134F0` | arg3 is copied verbatim into `+74h` |

`004134F0` is `BSP_Matrix_Copy4x4X87`. There is no test of the matrix on the path: the only branch
before `009259C4` is the `+0BCh` already-attached gate at `009258F6` and the `+0B4h`/`+0B8h` flush.
Whatever the caller pushes at `[ESP+14h]` is the node's pose local.

## 2. The caller census, and why it is exhaustive

### 2.1 Direct calls - a byte-level `rel32` scan, not the call graph

`local/scan_attach_refs.py` walks the PE section table of the shipped
`battlestationspacific.exe`, and for every `E8` and every `E9` byte in `.text`
(`00401000`-`00CE1136`, raw `0x1000`+`0x8E1000`) computes `site + 5 + rel32` and compares it with
the target. Every byte offset is tested, so the scan cannot miss an instruction through
misalignment.

**Exactly two direct sites: `009288D6` and `00929C94`.** No `E9` tail-call site.

Ghidra's `xrefs` agrees but hides the second one: `009288D6` and `004B21B3` are rows 1 and 2, then
twenty-eight `[DATA]` rows, and `00929C94` is row 33 - past the 25-row cap a sibling packet hit. The
byte scan is the primary evidence here and the `xrefs` listing only the confirmation.

### 2.2 `009258F0` is a virtual method: 30 vtables, slot `+98h`

The same scan, run over every section for the little-endian word `F0 58 92 00` with no alignment
filter, finds **30 occurrences, all in `.rdata`**:

```
00CE3BC0 00CE3EF8 00CE4068 00CE5AB8 00CE5C28 00CE5D88 00CE5F00 00CE61C8 00CE6528 00CE6B48
00CE6EA8 00CE70A0 00CE7240 00CEC290 00CEC4D8 00CEC688 00CF42D0 00CF44A8 00CF49C8 00CF5D80
00CF62E8 00CF9E88 00CFD668 00CFF710 00D04550 00D047E8 00D04BF0 00D09350 00D191B8 00D23238
```

The slot is `+98h` (index 38). Two independent lines of evidence, because a naive backward walk over
consecutive `.text` pointers runs into an adjacent sibling vtable and over-reports the index:

* fifteen of the thirty sit exactly `0x98` above a word whose predecessor is `00000000`, which
  terminates the run;
* for two of the ambiguous ones the vtable start is fixed by the constructor that stores it.
  `0065083A` and `00650852`, 0x18 apart in one constructor, store `00CF6250` and `00CF6218` - two
  base subobjects of a multiply-inherited class - and `00CF62E8 - 00CF6250 = 0x98`. Likewise
  `005193FB` and `0051949A` store `00CEC578` and `00CEC5F0`, and `00CEC688 - 00CEC5F0 = 0x98`.

### 2.3 The slot's other implementations both forward the matrix verbatim

| Implementation | Vtables | What it does with `localFrame` |
| --- | --- | --- |
| `009258F0` | 30 | copies it into `+74h` (section 1) |
| `00928860` `BSP_GameEntity_PlaceInWorld` | 44, plus 3 ICF thunks | forwards unchanged |
| `00929C80` (no Ghidra function) | 1 (`00D19598`) | forwards unchanged |

`00928860`: the prologue is `PUSH -1; PUSH 00CA6FE8; PUSH FS:[0]; MOV FS:[0],ESP; SUB ESP,8;
PUSH ESI; PUSH EDI` and later `PUSH EBX; PUSH EBP`, i.e. 0x24 of stack below the return address, so
its own arguments are at `[ESP+28h]`, `[ESP+2Ch]`, `[ESP+30h]`. At `009288C9`-`009288D6`:
`MOV EAX,[ESP+30h]; MOV ECX,[ESP+28h]; PUSH EAX; PUSH EBP; PUSH ECX; MOV ECX,ESI; CALL 009258F0`
with `EBP = [ESP+2Ch]` loaded at `009288A4`. Arg3 -> arg3, arg2 -> arg2, arg1 -> arg1. It introduces
nothing.

`00929C80` has no Ghidra function. Body `00929C80`-`00929CA9` **inclusive**; the final instruction is
the three-byte `RET 0Ch` (`C2 0C 00`) **at** `00929CA9`, so `end_exclusive` is `00929CAC`, followed
by `INT3` padding. The entry is `00929C80 MOV EAX,[ESP+0Ch]`, immediately after the `INT3` at
`00929C7F`. It reads its three arguments, pushes them in the same order, calls `009258F0`, then
registers the node with the list at `[worldNode+438h]` through `00484540`. It introduces nothing.

The three `JMP 00928860` stubs at `006DFE40`, `0074E180` and `007EFA70` are MSVC `/OPT:ICF`
identical-COMDAT folding thunks, each a single five-byte `JMP` followed by `INT3` padding.

### 2.4 The dispatch sites - enumerated by encoding, not by one guessed pattern

`0x98` is greater than `0x7F`, so a `+98h` displacement can only be encoded as `disp32`
(`mod = 10`). Four encodings could reach the slot, and `local/slot98_sites.py` and
`local/classify_slot98.py` test all four over every byte of `.text`:

| Encoding | Meaning | Sites |
| --- | --- | --- |
| `FF /2 disp32`, `FF /3 disp32` | `CALL dword ptr [reg+98h]` | **0** |
| `8B /r disp32` then `CALL <that same register>` within 32 bytes | load-then-call | **71** |
| `8D /r disp32` (`LEA reg,[reg+98h]`) then `CALL dword ptr [reg]` within 24 bytes | pointer arithmetic | **0** |
| `81 /0 imm32` (`ADD reg,98h`) then `CALL dword ptr [reg]` within 24 bytes | pointer arithmetic | **0** |

443 instructions in `.text` carry a `mod = 10` `8B /r` with `disp32 = 0x98`; 71 of those are
followed by a `CALL` of the loaded register. Every dispatch in this binary is the two-step form -
the single-instruction `CALL [reg+98h]` is used nowhere, which is exactly the mistake mode the
checklist warns about.

The 71 are not all this hierarchy: `+98h` is also an ordinary field on other objects. The
discriminator is the **second stack argument**, which `009258F0` stores at `+30h` as the world node:
on this hierarchy it is either `[*00E188A8 + 19CCh]` (the scene singleton's world node) or
`[someNode+30h]`. **42 of the 71 match** - 35 through `+19CCh` and 7 through `+30h`. Two more,
`0092388C` in `FUN_00923870` and `007AB672` (no Ghidra function), push a forwarded parameter and a
frame local respectively and are probable but unconfirmed. The remaining 27 push one argument or
none - nine of them through a singleton at `00F8D394`, six with no stack argument at all - and
belong to other classes that happen to have a field or a slot at `+98h`.

**Limit of the census.** It is exhaustive over direct calls, over vtable membership, and over the
four instruction encodings above. It would miss a dispatch whose slot load and indirect call are
more than 32 bytes apart, or one that stores the loaded pointer to memory before calling it. Nothing
in the 443-site listing looked like either.

## 3. What each dispatch pushes at `[ESP+14h]`

Argument roles are read from the push order (`PUSH matrix; PUSH world; PUSH parent; MOV ECX,this`)
against the `RET 0Ch` count. `local/argsum.py` prints the three pushes and the last instruction that
defined each register.

| Call site | Containing function | What is pushed as `localFrame` | Scale / shear / zero? |
| --- | --- | --- | --- |
| `009288D6` | `BSP_GameEntity_PlaceInWorld` `00928860` | its own arg3, `[ESP+30h]` | forwarder - inherits its caller's |
| `00929C94` | `00929C80`, no Ghidra function | its own arg3, `[ESP+0Ch]` | forwarder - inherits its caller's |
| `004F0584` | `BSP_SceneUnit_CreateDestroyerGen` | the creator's own `localFrame` parameter | **from the `.scn` `localframe`, section 4** |
| `004F0654` | `BSP_SceneUnit_CreateSubmarineGen` | same | same |
| `004F0724` | `BSP_SceneUnit_CreateTBoatGen` | same | same |
| `004F07F4` | `BSP_SceneUnit_CreateLandingShipGen` | same | same |
| `004F08C4` | `BSP_SceneUnit_CreateMotherShipGen` | same | same |
| `004F0994` | `BSP_SceneUnit_CreateAirField` | same | same |
| `004F0A64` | `BSP_SceneUnit_CreateShipyard` | same | same |
| `004F0B75` | `BSP_SceneUnit_CreatePlaneSquadronGen` | same | same |
| `004F1056` | `BSP_SceneUnit_CreateLandFort` | same | same |
| `004F1113` | `BSP_SceneUnit_CreateCommandBuilding` | same | same |
| `004F04FA` | `FUN_004F04C0` | parameter at `[ESP+10h]` | creator-family parameter, producer not traced |
| `004F0E42` `004F0F42` `004F2792` | `FUN_004F0DB0` `FUN_004F0EB0` `FUN_004F2700` | parameter at `[ESP+24h]` | same |
| `004E9A80` `004E9BB0` `004E9CE0` `004E9DE2` | `FUN_004E99B0` `FUN_004E9AE0` `FUN_004E9C10` `FUN_004E9D40` | parameter at `[ESP+28h]` | same |
| `004E9F4B` | `BSP_SceneCloud_Instantiate` | parameter at `[ESP+2Ch]` | same |
| `004F1502` `004F1B02` | `FUN_004F1460` `FUN_004F1A60` | parameter at `[ESP+28h]` | same |
| `004EA6FB` | `BSP_SceneDatabase_CreatePath` | local stack matrix, `LEA EAX,[ESP+18h]` at `004EA6E0` | **construction not read - partial** |
| `004EA80B` | `FUN_004EA760` | local stack matrix, `LEA EAX,[ESP+18h]` at `004EA7F0` | **construction not read - partial** |
| `006AF842` | `FUN_006AF780` | local stack matrix, `LEA EAX,[ESP+20h]` at `006AF82F` | **construction not read - partial** |
| `0082197D` | `FUN_008206F0` | local stack matrix, `LEA EAX,[ESP+0C0h]` at `00821966` | **construction not read - partial** |
| `00823D3D` | `BSP_UnitInstance_SEntityInit` | local stack matrix at `[ESP+38h]` | **identity, no** - see below |
| `00927201` | `BSP_MissionEntity_ApplySpawnDescriptor` | local stack matrix at `[ESP+30h]` | **`HierarchyMatrix` property, section 5** |
| `007438BD` | `BSP_LandConvoy_AttachAndBuildRoster` | `LEA EAX,[ESI+74h]` at `007438AE` | propagation of an existing pose local |
| `007F48D2` | `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | `LEA ECX,[ESI+74h]` at `007F48C9` | propagation of an existing pose local |
| `007F48BA` | same function, the other branch | local stack matrix, `LEA EAX,[ESP+64h]` at `007F482B` | **construction not read - partial** |
| `004B21B3` | `FUN_004B2030` | local stack matrix, `LEA ECX,[ESP+2Ch]` at `004B21A7` | **construction not read - partial** |
| `00491A30` | `FUN_00491950` | `MOV EAX,EBP; SHL EAX,6; ADD EAX,[EBX+4]` at `00491A1F`-`00491A27`: element `EBP` of a **64-byte-stride matrix array** at `[EBX+4]` | **array producer not traced - partial, and the shape is the one most likely to be authored** |
| `006F3798` | `FUN_006F3660` | `LEA EAX,[EDI+ECX]; ADD EAX,18h` at `006F3778`-`006F3789`: the 64-byte field at `+18h` of an array record | **array producer not traced - partial** |
| `00484C7D` `00487F16` `0048C819` `00498A61` `0049BD9D` | `FUN_00484BC0` `FUN_00487A70` `FUN_0048C6A0` `FUN_00498790` `FUN_0049B1F0` | local stack matrices (`LEA ECX,[ESP+4/44h/28h/38h/60h]`) | **construction not read - partial** |
| `007A5EB3` | `FUN_007A5A00` | local stack matrix, `LEA ECX,[ESP+34h]` at `007A5E3F` | **construction not read - partial** |
| `007AB672` | no Ghidra function | local stack matrix, `LEA ECX,[ESP+1Ch]` at `007AB5EF` | **construction not read - partial** |
| `0089AB8E` | `BSP_LuaBinding_SetFireTarget` | local stack matrix, `LEA ECX,[ESP+44h]` at `0089AB0C` | **construction not read - partial** |
| `00A32433` | `BSP_AiController_Create` | local stack matrix, `LEA ECX,[ESP+8]` at `00A323B5` | **construction not read - partial** |
| `009362FE` | `FUN_00935D30` | the third push lies further back than the 160-byte window; world argument confirmed as `[EDX+19CCh]` at `009362E5` | **not located - partial** |
| `0092388C` | `FUN_00923870` (`00923870`-`0092389D`) | its own arg3, read at `00923877 MOV ECX,[ESP+10h]` after the prologue `PUSH ESI` and pushed at `0092387D`; it re-dispatches through `[ESI]+98h` | forwarder - inherits its caller's |

`BSP_UnitInstance_SEntityInit` is read in full because it is the unit path. `00823CAD`
`XORPS XMM0,XMM0` and `00823CB0 MOVSS XMM1,[00D7A24C]` (the float `1.0`) load zero and one, and
`00823CC4`-`00823D1E` write sixteen `MOVSS` into `[ESP+3Ch]`-`[ESP+78h]`, which after the
`00823CC0 PUSH EAX` is the buffer that `00823CBC LEA EAX,[ESP+38h]` addressed. `XMM1` goes to
`+3Ch`, `+50h`, `+64h`, `+78h` - the four diagonal slots - and `XMM0` to the other twelve. It is the
identity, with no scale.

## 4. The authored producer: the `.scn` `localframe` statement

The ten `BSP_SceneUnit_Create*` creators take `localFrame` as a parameter
(`docs/SCENE_UNIT_CREATORS.md`: `create(classId /*ECX*/, entityName /*EDX*/, hierarchyParent,
const float localFrame[16], PropertyBag*, unused)`, `RET 10h`). The caller that supplies it is
`BSP_SceneFile_ReadEntityBlock`.

### 4.1 The parse

`00CE5840` is the ASCII `"localframe"`, and the only reference to it in the whole image is
`0046D13B PUSH 00CE5840; MOV ECX,EBP; CALL 008D9930` - `BSP_SceneTokenizer_ExpectToken`. The
statement is then sixteen floats:

```
0046D150  LEA EDX,[ESP+23h]      ; token scratch
0046D154  PUSH EDX
0046D157  CALL 008D9B40          ; BSP_SceneTokenizer_ReadFloat, RET 4
0046D15C  FSTP dword ptr [ESP+ESI*4+48h]
0046D160  ADD ESI,1
0046D163  CMP ESI,10h
0046D166  JL 0046D150
```

The buffer is at `ESP+48h`. Ghidra calls it `local_164`, and the naming is consistent: the prologue
is `PUSH -1; PUSH 00C61E08; PUSH FS:[0]` (0x0C) + `SUB ESP,190h` + four register pushes (0x10), so
the frame base is `ESP_entry - 1ACh` and `local_164` sits at `ESP_entry - 164h` = base + `48h`.

A shipped statement looks like

```
localframe 1.0000 0.0000 0.0000 0.0000 0.0000 1.0000 0.0000 0.0000 0.0000 0.0000 1.0000 0.0000 2300.0000 0.0000 -1100.0000 1.0000 ;
```

row-major, translation in row 3 - the same layout `00B63D50` assumes.

### 4.2 The canonicalisation, and what it does **not** remove

```
0046D168..0046D184   s2 = m[0]*m[0] + m[1]*m[1] + m[2]*m[2]      ; |row 0| squared, stored at ESP+14h
0046D188..0046D196   FLD qword [00CE3820] (1e-10); FCOMI; JBE
0046D198             CALL 00BF7030                                ; the CRT sqrt helper -> s
0046D1AB..0046D1B0   else  s = 0.0                                ; XORPS XMM0,XMM0; MOVSS [ESP+14h],XMM0
0046D1B6  LEA ECX,[ESP+48h]
0046D1BA  CALL 0085DC80                                           ; orthonormalise in place
0046D1BF..0046D222   m[0,1,2] *= s ; m[4,5,6] *= s ; m[8,9,10] *= s
```

Ghidra's decompilation of `0046CF40` renders the same block over `local_164` and `local_198`, so the
x87 stack reading is confirmed by a second representation.

`0085DC80` is Gram-Schmidt. `0085DC85 MOV ESI,ECX; 0085DC87 LEA EBX,[ESI+20h]` makes row 2 the
reference (`+20h` = float 8) and `0085DCB9 LEA EDI,[ESI+10h]` row 1. It normalises row 2, then
tests `|row1 . row2|` against the double `0.9990000128746033` at `00D0D0A0`: below it, row 1 is
orthogonalised against row 2, normalised, and row 0 becomes `cross(row1, row2)`; above it (rows 1
and 2 nearly parallel) row 0 plays the role row 1 would have, and the cross product is negated
through the float `-0.0` at `00D7A208`. Either way the result is orthonormal.

**Then the nine basis elements are multiplied by `s` again.** The parser does not produce an
orthonormal frame; it produces a **similarity** `s*R + t`, discarding shear and any per-row scale
difference but preserving one uniform scale taken from the authored row 0. That is a deliberate
design decision in the shipped engine, not an oversight of the caller.

### 4.3 The frame reaches the creator, and therefore `+74h`

```
0046D57E  MOV EAX,[ESP+1B8h]        ; arg4, never read by any of the ten
0046D585  MOV EDX,[ESP+1B4h]        ; hierarchyParent
0046D58C  PUSH EAX
0046D58D  MOV EAX,[ESP+28h]         ; the {classId, creator} record
0046D591  PUSH EBX                  ; the property bag
0046D592  LEA ECX,[ESP+50h]         ; = base + 48h  ->  the canonicalised localframe
0046D596  PUSH ECX
0046D597  MOV ECX,[EAX]             ; classId
0046D599  MOV EAX,[EAX+4]           ; the creator
0046D59C  PUSH EDX                  ; hierarchyParent, the last push = arg1
0046D59D  LEA EDX,[ESP+0B0h]        ; = base + A0h = local_10c, the entity name
0046D5A4  CALL EAX
```

`0046D592` is two pushes past the frame base, so `[ESP+50h]` is base + `48h` - the same buffer the
parse loop filled. `0046D59D` is four pushes past, so `[ESP+0B0h]` is base + `A0h` = `local_10c`,
which Ghidra shows as the name argument of `BSP_SceneEntity_ShouldGenerate` in the same block. Both
offsets agree with Ghidra's naming, which is the independent check on the ESP arithmetic.

The complete chain is therefore

```
.scn  localframe <16 floats>
  -> 0046D150..0046D166   parse into base+48h
  -> 0046D168..0046D1B0   s = |row 0|   (0 when |row 0|^2 <= 1e-10)
  -> 0046D1BA  0085DC80   orthonormalise
  -> 0046D1BF..0046D222   basis *= s          ->  s*R + t
  -> 0046D592/0046D5A4    creator localFrame argument
  -> 004F0584 etc.        vtable[98h](hierarchyParent, world, localFrame)
  -> 009259C4  004134F0   entity+74h
```

## 5. The other authored key: `HierarchyMatrix`

`00CEA008` is `"HierarchyMatrix"`, `00CEA018` `"HierarchyParent"`, `00CEA028` `"SetHierarchy"`.
`BSP_MissionEntity_ApplySpawnDescriptor` reads them:

```
009271C0  PUSH 00CEA008
009271C5  CALL 008F2260              ; BSP_ScenePropertyBag_Find
009271CA  TEST EAX,EAX
009271CC  JZ 009271E4
009271CE  LEA EDX,[ESP+70h]; PUSH EDX; MOV ECX,EAX; CALL 00924280
009271DA  PUSH EAX; LEA ECX,[ESP+34h]; CALL 004134F0
```

`00924280` is `__thiscall(property /*ECX*/, float* destination)`, `RET 4`:
`00924280 MOV ECX,[ECX+20h]; MOV EAX,[ESP+4]` then sixteen `FLD`/`FSTP` pairs, returning the
destination in `EAX`. `+20h` on a scene property record is the **array data pointer**
(`docs/SCENE_PROPERTY_BAG.md`; type 9 `FA` is `<count>` then `<count>` floats at `+20h`/`+24h`,
stride 4). So `HierarchyMatrix` is a sixteen-element float array copied verbatim.

Three things are absent from the call site: no check of the type tag at `+4` (contrast
`HierarchyParent` at `0092717D`, which requires type 5), no check of the element count at `+24h`,
and no orthonormalisation. A `HierarchyMatrix` authored as an `FA` is sixteen arbitrary floats
landing in `+74h`.

`004F03C0` is the writer of the same three keys - `docs/SCENE_UNIT_CREATORS.md` records that it
pushes the sixteen floats of the local frame with `REP MOVSD ECX=10h` and sets `HierarchyMatrix`
through `008F3880`. So the keys are a runtime round-trip of an existing pose local, not a separate
authoring channel.

**They are not used by shipped data.** A grep of the whole game installation finds
`HierarchyMatrix`, `HierarchyParent` and `SetHierarchy` **only inside `battlestationspacific.exe`**;
no shipped `.scn` contains any of the three, and no shipped `.scn` uses the `FA` type at all (257 of
the 259 do contain `CommandTarget`, so the grep is not failing on the format). This path can carry
an arbitrary matrix but the shipped data never exercises it.

## 6. The shipped-data audit

`local/scan_scale.py` parses every `localframe` in all 259 `.scn` files under
`I:\SteamLibrary\steamapps\common\Battlestations Pacific` and computes `s = |row 0|`, the only
non-rotational quantity that survives section 4.2, attributing each to the `entity "<name>" (<Kind>)`
block that contains it.

```
localframe statements   133,655        distinct s values   266
s == 1.0 exactly        120,477        |s - 1| <= 1e-3     126,034
|s - 1| > 1e-3            7,621        s == 0                   0
```

| s | count |
| --- | --- |
| 1.0 | 120,477 |
| 1.0001 / 0.9999 | 2,778 / 2,665 |
| 1.006 | 924 |
| 0.7069 | 766 |
| 1.2054 | 368 |
| 0.9063 / 0.9062 | 281 / 199 |
| 1.1057 | 228 |
| 0.8066 | 203 |
| 1.5663 | 170 |
| 1.3051 | 154 |

Extremes: minimum `s` = 0.1586, maximum `s` = 3.0.

Scaled frames by entity kind: `LandFort` 7,133, `Stationary` 421, `CommandBuilding` 17,
`DestroyerGen` 7, `Path` 2.

A second pass (`local/scan_localframes.py`) confirms the parser really is discarding shear: the
authored data contains 5,032 non-orthogonal row pairs (worst `|row_i . row_j|` = 1.3488) and rows of
mutually different lengths, none of which survive `0085DC80`. Only the uniform `s` does.

### 6.1 Is a scaled node ever above a gun mount?

`docs/GUN_MOUNT_POSITIONS.md` resolves `gun+3CCh` to a **model node** of the unit's model, the first
non-null of the nodes named `"barrel"` (`00CF70C4`) and `"base"` (`00CFACD8`) and the model's own
first node, and the muzzle is that node's `worldMatrix`. The model hangs off the unit, so the unit's
scene-node pose is an ancestor of every gun it carries.

| Kind | Scaled placements | Carries guns? | Example |
| --- | --- | --- | --- |
| `LandFort` | 7,133 | **yes for the AA subset** | `'Heavy AA, Japanese 01'`, `s = 0.7069`, `chg\hrv_mission.scn:6424` |
| `LandFort` | 106 of those named `Fort*` | fortifications | `'Fortress element, Small tower 01'`, `s = 1.5045`, `COTP-IJN\ijn_11_invasion_of_fiji.scn:8869` |
| `LandFort` | 3 named `*Bunker*` | yes | `'Medium Bunker 03, Concrete 01'`, `s = 0.8634` |
| `DestroyerGen` | 7 | **yes** | `'L Hancock'`, `s = 0.866`, `COTP-USN\bulls_run.scn:1355`; `'Fubuki-class 13'`, `s = 0.98984`; `'FakePoW'`, `s = 1.00277` |
| `CommandBuilding` | 17 | base structures | `'Headquarter 01'`, `s = 1.13864`, in eight shipped missions |
| `Path` | 2 | no | `'AvoidZoneG all 0 #001/2'`, `s = 1.07` / `0.83` |
| `Stationary` | 421 | not established | - |

105 of the scaled `LandFort` placements have `AA` in the entity name. Those are the clearest case: an
anti-aircraft emplacement is a gun platform, and `s = 0.7069` (approximately `1/sqrt(2)`) is the
most common scaled value in the whole data set.

### 6.2 What the scale does to the arc

With a mount basis `s*R`, `00B63D50` returns `N[i][j] = M[j][i] / |row j|^2 = R[j][i] / s`, which
is the correct affine inverse of `s*R`. But `0042D0D0` runs with `normalize = 0`, so the direction
that reaches `00521370` has magnitude `1/s`, and the pitch read back is `asin(sin(theta)/s)`:

| s | where | a true 30 degree pitch reads as |
| --- | --- | --- |
| 0.7069 | Heavy AA emplacements | 45.017 degrees |
| 0.866 | destroyer `'L Hancock'` | 35.266 degrees |
| 1.13864 | `'Headquarter 01'` | 26.048 degrees |
| 1.5045 | fortress tower | 19.411 degrees |
| 3.0 | the extreme in the data | 9.594 degrees |

At `s = 0.7069` any true pitch above 45.05 degrees drives `sin(theta)/s` past 1 and the clamp
saturates at 90 degrees.

### 6.3 The degenerate case

`local/detail_cases.py` finds two shipped `localframe` statements whose authored **row 2** is exactly
`(0, 0, 0)`:

```
NavPoint  'Navpoint Port'  universe\scenes\missions\multi\scene175.scn:27460
NavPoint  'Java_land'      universe\scenes\missions\multi\scene904.scn:2950
```

`0085DC80` guards the division - `0085DC96 FLDZ; FCOMI; JBE 0085DCAC` sets the reciprocal to `0.0`
rather than dividing - so row 2 stays the zero vector. Then `|row1 . row2| = 0`, which is below the
`00D0D0A0` threshold, so the else arm runs: row 1 is unchanged and normalised, and
`row 0 = cross(row1, row2) = (0, 0, 0)`. The frame that reaches `+74h` has **two zero basis rows**,
and `00B63D50` divides by `|row 0|^2 = 0` and `|row 2|^2 = 0`.

Both are `NavPoint` entities, which carry no guns, so the practical consequence is confined to
whatever reads a navpoint's pose inverse. `s == 0` never occurs: `|row 0|` is non-zero in all
133,655 statements, so the `0046D1AB` zero arm of the scale is not reached by shipped data.

## 7. What is proven, what is assumed, and what is partial

**Proven.**

* The direct-call census of `009258F0` is complete: a byte-level `rel32` scan over every byte of
  `.text` finds exactly two sites, and both forward their own third argument unchanged.
* The vtable census is complete: 30 slots, all at `+98h`, the offset fixed by two constructor pairs
  and fifteen zero-terminated runs.
* The dispatch encoding census is complete over the four encodings a `+98h` displacement admits;
  the single-instruction `CALL [reg+98h]` form does not occur anywhere in this binary.
* `009259C4` copies the argument into `+74h` with no validation, the only branches before it being
  the `+0BCh` gate and the `+0B4h`/`+0B8h` flush.
* `BSP_SceneFile_ReadEntityBlock` canonicalises the authored `localframe` to `s*R + t` and passes
  that buffer to the creator at `0046D5A4`; the two ESP offsets are cross-checked against Ghidra's
  `local_164` / `local_10c` naming.
* The shipped `.scn` numbers in section 6 are a full parse of all 259 files.

**Assumed, carried over, not re-derived here.**

* That `00414DB0` composes `world_child = local_child * world_parent` without normalising, so a
  scaled root pose propagates to descendants - `docs/MATRIX_ORTHOGONAL_INVERSE.md`.
* That `0042D0D0` runs with `normalize = 0` and `00521370` takes `asin` of the raw `y` - same doc.
  Section 6.2's arithmetic is that doc's formula with `s` filled in, not an independent derivation.
* That the model-node chain under a unit inherits the unit's scene-node pose. `docs/GUN_MOUNT_POSITIONS.md`
  establishes that `gun+3CCh` is a model node and the muzzle is its `worldMatrix`; the composition
  from the scene node into the model node's world matrix is **not** read by this packet.
* That nothing overwrites `+74h` after the attach for these entities. `docs/MATRIX_ORTHOGONAL_INVERSE.md`
  surveyed the `+74h` writers (`00904600`, `006E5D03`, `00925CE0`, and `00825F20` which never
  writes it); a `LandFort` is static and has no motion update, but this packet did not prove that a
  `DestroyerGen`'s pose local is never rewritten by ship motion.
* `docs/SCENE_UNIT_CREATORS.md` for the ten creators' ABI and for `004F03C0`'s write-back. The
  creator ABI is independently confirmed here by the four pushes and the two `LEA`s at
  `0046D57E`-`0046D5A4`; `004F03C0` is not re-read.

**Partial - callers whose matrix producer was not traced.** Every row in section 3 marked
*construction not read* is a local stack matrix whose construction lies outside the addresses this
packet read. Two of them deserve a follow-up more than the rest, because their shape is the one an
authoring tool produces rather than a runtime composition:

* `00491A30` in `FUN_00491950` - `MOV EAX,EBP; SHL EAX,6; ADD EAX,[EBX+4]`, element `EBP` of a
  64-byte-stride **array of matrices**.
* `006F3798` in `FUN_006F3660` - the 64-byte field at `+18h` of an array record.

Also unread: whether the `Stationary` kind (421 scaled placements) carries guns, and whether the
`FUN_004E99B0`/`FUN_004F04C0` sub-family's `localFrame` parameters come from the same
`0046D5A4` creator dispatch as the named ten.

**Ghidra was read-only for this packet.** No renames, comments, prototypes, function creation or
saves. `0046CF40`, `0085DC80`, `008F2260`, `004F03C0` and the model/scene-node system are read here
as a contract and are **not** reconstructed or leased by this packet; `009258F0`, `0085DC80` and
`00924280` are the only leased addresses, and the lease on the last two covers the two ledger names
below. `lease check` reported `0046CF40`, `0085DC80` and `00924280` unleased at the time of reading;
no claim was refused.

## 8. Corrections for the integrator

1. **`docs/MATRIX_ORTHOGONAL_INVERSE.md`, "Where scale could enter" and the follow-up list.** The
   bounded negative - "no producer read to date puts scale into a pose local" - is now superseded.
   The attach callers are enumerated and one of them, the `.scn` entity reader through the ten unit
   creators, puts a **uniform scale** into `+74h` by design. `s` is outside `1 +/- 1e-3` in 7,621 of
   the 133,655 shipped `localframe` statements, and 105 of those are anti-aircraft `LandFort`
   entities. That document's sentence "on the balance of that evidence the arc's mount bases are
   orthonormal and both the inverse and the pitch are correct in practice" should be replaced by:
   the mount bases are orthonormal for the 94.3 per cent of shipped placements with `s = 1`, and for
   the remainder the pitch is wrong by `asin(sin(theta)/s)`.
2. **Same document, "Zero-length rows".** A zero basis row is reachable from shipped data, though
   not on a gun: two `NavPoint` entities author a zero row 2, and `0085DC80`'s zero guard turns that
   into two zero rows rather than rejecting it (section 6.3).
3. **The `009258F0` ledger record** (and the `exports/` header generated from it) said "sole caller
   `00928860`". It is wrong twice over: `00929C94` is a second direct caller, and the routine is a
   virtual method at slot `+98h` in 30 vtables with 42 confirmed dispatch sites. This packet appended
   the census to the record rather than replacing the text, so the old sentence is still present in
   the record's history; the integrator should decide whether to strike it.
4. None of these corrections changes `00B63D50`'s own contract, which this packet re-uses unchanged.

## 9. Follow-up packets

* **`scene_entity_reader_frames`** - `0046CF40`, the scene-file entity block reader, and `0085DC80`,
  the orthonormaliser, as a reconstruction rather than a contract. That is the packet that should
  own the `s*R + t` canonicalisation and decide whether the surviving uniform scale is intended
  (a model-scale authoring feature) or a bug. It belongs with the scene/model territory, not here.
* **`pose_local_rewrite_survey`** - does anything overwrite `+74h` for a `DestroyerGen` after the
  attach? If ship motion rewrites it with a pure rotation the seven scaled destroyers are harmless
  and only the static `LandFort`/`CommandBuilding` set matters.
* **`matrix_array_attach_sources`** - `FUN_00491950`'s 64-byte-stride matrix array at `[EBX+4]` and
  `FUN_006F3660`'s `+18h` record field, the two untraced producers whose shape suggests authored
  data.
* **`gun_mount_world_composition`** - the step this packet assumes: how a model node's
  `worldMatrix` is composed from the unit's scene-node pose, and whether any normalisation happens
  on the way.
* **`stationary_entity_guns`** - whether the `Stationary` kind (421 scaled placements) mounts guns.
