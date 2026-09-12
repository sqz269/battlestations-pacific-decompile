# Gameplay loose ends, second pass (packet `cc2_loose_ends_2`)

Addresses: `007DA380` `007D9F60` `006EB5C0` `007C78A0` `007619B0` `0077F2D0` `008141E0`
`008198A0` `008137B0` `007583A0` `007583F0` `006D0D90` `008637D0` `00721760` `007CDC70`
`0084D810` `0084E010` `00720180` `00720450`

Three groups of questions earlier packets left open. Ghidra was read-only for this packet. Every
hunt for a writer, reader or caller started from a byte scan of the PE on disk
(`local/pescan.py`: `disp`, `cover`, `disprange`, `imm`, `callto`, `vcall`) because `ghidra xrefs`
under-reports and `ghidra callers` answers nothing for a function reached only through a vtable.
Every scan decodes two-byte (`0F`-prefixed, SSE) opcodes like any other and reports the
displacement width, so a 16-byte `MOVUPS` covering a byte field is not missed.

## Summary

| item | answer |
| --- | --- |
| A1 `ctl+FCh` | a three-valued controller mode (`0` free flight, `1` ground roll, `2` water surface) with six readers; it selects the aerodynamic block in the core law, dispatches `007DA380`'s factor arms, and **zeroes the roll rate when it is `1`** |
| A2 `unit+C01h` | **no reader exists**, at any width, through any base |
| A3 `unit+900h == 3` | no code path produces `3`; it is reachable only from a network state byte or a spawn record, and the catapult consumes it |
| A4 `unit+9C0h` | two sibling class families reuse the offset: a stride-8 byte record on planes, a `MaxSpeed` float on the class-`06` ship family |
| B slot `144h` | one body, `0077F2D0`, in all 25 unit vtables |
| B slot `208h` | four unrelated per-family virtuals at the same index, because the class-`05` base vtable ends one slot earlier |
| B slots `220h`/`224h`/`238h` | **ship-family only**; two bodies each |
| B kind `6Ch` producer | `008132C0`'s `"EngineJam"` branch, through the dedicated constructor `007619B0`, whose kind is a byte immediate and never a `PUSH` |
| C1 director `+1D0h`-`+21Fh` | twenty `-1.0e10` floats; **nothing reads them** |
| C2 `008637D0` | "any enabled weapon category of this unit accepts this target" |
| C3 director `+34h` | the base constructor's parent argument, not the owning unit |
| C4 director `+16Ch` | one reader, `BSP_CommandControllerBase_CopyStateFrom`, and it is a state clone |
| C5 `0084E010` | slot `+7Ch` of `00D0BD98`, the table `0084D810` installs; owner identified, body still unread |
| C6 `00F87574`-`7Ch` | the zero vector: no store exists anywhere in `.text` |

---

# A. Plane leftovers

## A1. `ctl+FCh`, the flight controller mode

`ctl` is `unit+AB0h`. `BSP_PlaneFlightController_Construct` (Ghidra body
`007D7EA0`-`007D7FE7`, `__thiscall(ctl, float*)`, `RET 4`) initialises the field to **zero** at
`007D7FDA` (`MOV [ESI+FCh], EBX`; filtering the whole body for `EBX` finds one write,
`007D7EA7 XOR EBX,EBX`, so the value is `0` and not a stale argument). `+F0h`/`+F4h`/`+F8h` are
the three floats the constructor stores just before it, so `+FCh` closes a `10h`-byte group.

Scanning `.text` for the disp32 `000000FCh` over `007D7000`-`007DE000` returns eleven decoded
operand references and no others, so the reader set below is complete for the controller class.

### Writers

| value | site | law |
| --- | --- | --- |
| `0` | `007DC841` | `BSP_PlaneFlight_FreeFlightStep`, body `007DC830`-`007DCCE2` |
| `1` | `007DCD24` | `BSP_PlaneFlight_GroundRollLaw`, body `007DCCF0`-`007DCDCB` |
| `2` | `007DCDDC` | `BSP_PlaneFlight_WaterSurfaceLaw`, body `007DCDD0`-`007DD9AC` |

No writer stores any other value, so the `> 2` arms below are unreachable in the shipped image.

### Readers, as a rule table

| site | function | rule |
| --- | --- | --- |
| `007DA38D` | `FUN_007DA380` | four-way dispatch: `0` -> `007DA6E6`, `1` -> `007DA542`, `2` -> `007DA3D0`, otherwise the tail at `007DA3AB`. The decode is `SUB EAX,0 / JE` then `SUB EAX,1 / JE` twice |
| `007DA8D9` | `BSP_PlaneFlight_ControlRateLaw` | `CMP [ESI+FCh],1 / XORPS XMM2,XMM2 / JNE 007DA8EB / MOVSS [ESP+18h],XMM2`: **the roll accumulator is zeroed when the mode is `1`** and kept otherwise |
| `007DB6D1` | `BSP_PlaneFlight_CoreLaw` | `CMP EDX,1 / JNE 007DB744`: the ground-plane store at `007DB702`-`007DB743` runs only in mode `1` |
| `007DBE0E` | `BSP_PlaneFlight_CoreLaw` | `TEST EAX,EAX / JNE 007DBEAA` then `CMP EAX,1 / JNE 007DC205`: a three-way split of the aerodynamic block, `0` -> `007DBE1C`-`007DBEA5`, `1` -> `007DBEB3`-`007DC204`, `>= 2` -> `007DC205` |
| `007DA211` | `FUN_007D9F60` (reached from `BSP_PlaneTickElement_AdvancePose`) | `CMP [EBX+FCh],1`, consumed far downstream by `007DA2B1 JNE 007DA33A`: the wheel-height lift runs only in mode `1` |
| `007DD35C` | `BSP_PlaneFlight_WaterSurfaceLaw` | `FLD [EDI+FCh]`, a float load; `EDI` there is not the controller and the base was not established. Not the mode |

So the three values gate three things, not one: **which aerodynamic block the core law runs**,
**which factor arm `007DA380` takes**, and **whether the roll rate survives**.

### `FUN_007DA380`, the mode-dispatched factor provider

`__thiscall(ctl, float* outA, float* outB, unsigned char* outByte)`, `RET 0Ch` (three stack
arguments), Ghidra body `007DA380`-`007DA707`. It calls `007D9A70(ctl)` first and keeps the `ST0`
result in the local the prologue's `PUSH ECX` reserves.

| mode | arm | `*outA` | `*outB` | `*outByte` |
| --- | --- | --- | --- | --- |
| `0` free flight | `007DA6E6`-`007DA705` | the `007D9A70` result | the same value | `1` (`007DA700`) |
| `1` ground roll | `007DA542`-`007DA6E3` | the `007D9A70` result, seeded at `007DA550` | `1.0f` at `007DA55C`, recomputed at `007DA6D7` | `0` (`007DA6DE`) |
| `2` water surface | `007DA3D0`-`007DA53F` | `1.0f` at `007DA3E2` | `1.0f` at `007DA3E6` | `0` (`007DA53A`) |
| `> 2` | `007DA3AB`-`007DA3CD` | `0.01f` (`[00D7A238]`) | `0.01f` | `0` (`007DA3BF`) |

`coverage: partial` for the mode-`1` and mode-`2` arms: the entry and exit stores are read, the
arithmetic between them is not.

### How the rate law consumes the three out-parameters

Call the frame base `E` = `ESP` after `007DA710 SUB ESP,54h / PUSH ESI / PUSH EDI`. The three
`LEA`s at `007DA717`, `007DA71C` and `007DA721` are taken between pushes, so the out-parameters
are the locals `E+18h` (`outA`), `E+28h` (`outB`) and `E+0Bh` (`outByte`). The frame arithmetic
needs one fact: **`00419010` is `RET 14h`** - it cleans its own five float arguments - so the
`SUB ESP,14h` blocks do not shift the frame across a call.

```
E+10h = unit+838h + unit+BB8h                                   ; 007DA732-007DA74C
E+1Ch = 1.0f, or the stall shaping when unit+900h == 6          ; 007DA746, 007DA76C-007DA7B5
007DA380(ctl, &E+18h, &E+28h, &E+0Bh)                           ; 007DA728
E+10h *= classDesc+1A8h RollSpd * (E+18h) * (E+1Ch)             ; 007DA7C2-007DA7DA
if (unit+5Dh) E+10h *= the turbo / stall blend                  ; 007DA7E4-007DA8D5
if (ctl+FCh == 1) E+10h = 0                                     ; 007DA8D9-007DA8E5
pitch term *= E+18h                                             ; 007DA8FC-007DA908
if (E+0Bh) bank-yaw coupling, which adds E+10h                  ; 007DA9EF-007DAA7A
```

`outA` is therefore a shared authority scale on both the roll and the pitch rate, `outByte`
enables the bank-yaw coupling (set only in free flight), and `outB` at `E+28h` is overwritten at
`007DA7ED` before its first read at `007DA95B`, so `007DA380`'s value for it is dead on the
turbo path.

### What the two mode-`1`-only blocks do

**The core law's ground plane.** `007DB6EA`-`007DB743` is additionally gated on the core law's
second stack argument (`[ESP+5Ch]`, the `flag ? 0.0f : 1.0f` the ground law passes) being
non-zero; a zero jumps to `007DC696`. It then stores a plane into `[ctl+10h]+94h`..`+A0h`:

```
h = unit+BFCh - classDesc+1FCh                  ; 007DB702-007DB70B
[ctl+10h]+94h = -h                              ; 007DB724, 007DB73E
[ctl+10h]+98h = 0.0f                            ; 007DB71C
[ctl+10h]+9Ch = 1.0f  ([00D7A24C])              ; 007DB726
[ctl+10h]+A0h = 0.0f                            ; 007DB732
```

Normal `(0, 1, 0)` and distance `-h`: the ground plane the roll law constrains the aircraft to.
`ctl+10h` is the `0D0h` sub-object with vtable `00D06844` the constructor allocates.

**The wheel-height lift.** `007DA2B7`-`007DA338` in `FUN_007D9F60` additionally needs
`unit+BF4h != 0` (the object being stood on) and `unit+3Ch != 0` (the visual node). It refreshes
the node pose when `node+C8h` is clear (`00414DB0`), transforms a point by `node+CCh`
(`00414D10`), queries the stood-on object with `006BC530(unit+BF4h, point, &out)`, and when
`out - (classDesc+1FCh + 0.01)` is negative it lifts the pose by the shortfall through
`007D7D70(ctl+10h)`.

Two consequences for `docs/PLANE_GROUND_OPS.md`'s open questions:

* **`classDesc+1FCh` is the undercarriage height reference**, the distance from the model origin
  to the wheels. It is the subtrahend in the ground-plane store, in the wheel-height lift and in
  the lift-off test `unit+BFCh - classDesc+1FCh > 0.1`.
* **`unit+BFCh` is written by `006BC530` through an out-pointer**, not by the physics body.
  `007BB2E3` and `007C5F1C` are the only `LEA` instructions in `00600000`-`00A00000` that take a
  unit-based address in the `0B82h`-`0C01h` displacement band, and both push `&unit+BFCh` as
  `006BC530`'s third argument.

## A2. `unit+C01h` has no reader

Four independent scans, all negative.

| scan | result |
| --- | --- |
| disp32 `00000C01h` over `.text` | seven raw occurrences; five decode to instructions, **all byte writes**; `00769676` and `00B2E559` fall inside other encodings (`00769675 JMP 0076A27B` is `E9 01 0C 00 00`) |
| every memory operand of any width whose byte range covers offset `C01h` (displacements `0BF2h`-`0C01h`) | the same five writes plus two 16-byte clears, `007D01EC` and `007D6276` `MOVUPS [ESI+BFCh],XMM` (which cover `BFCh`-`C0Bh`). **No read at any width** |
| disp32 `00000C00h` | in `00600000`-`00800000` only `007B96C2 MOV byte [ECX+C00h],1` and `007D0110 MOV byte [ESI+C00h],BL`. No dword or word read at `+C00h`, so the "folded into a dword" hypothesis fails |
| the raw dword `00000C01h` in `.rdata` and `.data` | none, so no offset table names the field |
| every `LEA` with a displacement in `0B82h`-`0C01h` over `00600000`-`00A00000` | the only unit-based ones are `007BB2E3` and `007C5F1C`, both `&unit+BFCh` as `006BC530`'s out-parameter, so neither produces a base a later small displacement could read `C01h` through |

The writers and who calls them:

| site | writes | containing function | callers |
| --- | --- | --- | --- |
| `007B9000` | `C01h = 2` | `TRIV_body_007b9000`, body `007B9000`-`007B9007` | `009B2449`, `009CD59E`, `009CEAC4` |
| `007B9010` | `C01h = 1` | `TRIV_body_007b9010`, body `007B9010`-`007B9017` | `009CE8DC` |
| `007B96C2` | `C00h = 1` | `FUN_007b96c0`, body `007B96C0`-`007B96CE` | `006D0741`, `009B28B8`, `009CD93D` |
| `007CB37E` | `C01h = 2` | `BSP_Plane_LimitAndResetControls` | - |
| `007CC2AD` | `C01h = BL` (`2`) | `BSP_Plane_GroundRollStep` | - |
| `007D0110`/`007D0116` | `C00h`/`C01h` = `BL` | `BSP_PlaneUnitInstance_Construct` | - |

The three one-line setters are called only from the pilot bot (`0099`-`009C`) and one HUD-range
site, at points that read like abort notices: `009CEAC4` fires when the plane still has ground
contact on the expected object, `009CE8DC` when `unit+100h` (the pose Y) exceeds a threshold,
`009B2449` when a float exceeds the double at `[00D7A220]`. **`unit+C00h` and `unit+C01h` are
write-only in the shipped image.**

## A3. `unit+900h` state `3`

No routine in the image produces the value `3`.

* **Immediate stores** to `unit+900h` (disp32 `00000900h` over `00400000`-`00C00000`): `4` at
  `007C1697` and `007C7488`, `5` at `007C171E`, `6` at `007C63F4` and `007CBA12`, `7` at
  `007C6481`, `007C7183` and `007D6600`, `2` at `007CC7E2`, `1` at `007CC857`. Never `3`.
* **The guarded setter** `BSP_Plane_SetFlightState` (`007C1430`) is reached from six sites, which
  push `6` (`007C165D`, `EBX` proven `6` by the `CMP EBX,6` at `007C1658`), `4` (`007C1893`),
  `7` (`007C70EE`), `4 + (byte != 0)` (`007CA4CF SETNE AL / ADD EAX,4`), `6` (`007CB9B9`) and
  `7` (`007CBF77`). Four of them sit behind the guard `EAX in {7, 6, 4, 5}`
  (`007C187F`-`007C1891`), which excludes `3` explicitly.
* **The unguarded writer** is `FUN_007C78A0`, `__thiscall(unit, int newState, int flag)`, body
  `007C78A0`-`007C7A3A`. Its four callers push `6` (`007CB9F3`), `2` (`007CC7B9`), `1`
  (`007CC839`) and, at `007D160F` in `BSP_Plane_ApplyControlStateMessage`,
  `MOVZX ECX, byte [EDI+8Ch]` - **a byte straight out of a replicated record**.
* **Register-sourced stores** elsewhere: `007C627C` `EAX = 2` (`007C625B MOV EAX,2`);
  `007CB673` and `007CB6E2` `EDI = 4` (`007CB5FD MOV EDI,4`); `007D0060` `EBX = 0`
  (`007CFD56 XOR EBX,EBX`); `007D63EA` `byte [EBP+129h]`, `007D65DD` `[EAX+0Ch]` and `007D711A`
  `[ESP+10h]` are all data- or argument-sourced; `00A3960F` belongs to another class (it zeroes
  `+8E8h`..`+900h` in one run).

So `3` is **data-reachable and code-unreachable**: only a replicated state byte or a spawn /
property-bag record can put the plane there.

Its consumers are live, which is why the state is not simply dead:

| site | rule |
| --- | --- |
| `007C6347` | `BSP_Plane_ChooseSpawnFlightState`'s "already launching" arm, which makes no state write |
| `006EC456` | `FUN_006EC240` (body `006EC240`-`006EC61D`, reached only from `006EC632`): `if ([this+3D0h] && [this+3D0h]+900h == 3)` it runs the node-matrix block at `006EC463`-`006EC4DB`. `this+3BCh`, `+3C8h` and `+3D0h` and the calls to `BSP_Node_GetLocalMatrix`, `BSP_Matrix_Copy4x4X87` and `BSP_Matrix_Multiply4x4` make this a catapult holding a plane |
| `007C78D9`, `007C78FE` | `FUN_007C78A0` classifies both the old and the new state as "in `{1, 2, 3, 4, 5}`", so `3` is a surface state for the transition test |

`006EB5C0` is a four-instruction predicate `XOR EAX,EAX / CMP [ECX+900h],3 / SETE AL / RET`
surrounded by `INT3` padding. It has **zero references of any kind** - no `E8`/`E9` rel32 in any
code section and no dword equal to `006EB5C0` anywhere in the image - so it is an emitted but
unreferenced accessor.

## A4. `unit+9C0h`: two class families, one offset

Both readings in the earlier docs are right; they are not the same class.

**The stride-8 byte record (plane family).** `FUN_007CDC70` (body `007CDC70`-`007CDD09`,
`__thiscall(unit)`) is the per-step publisher:

```
idx = word [00E0B6CC]                                  ; the current double-buffer index
unit[idx*8 + 9C1h] = (unit+DECh ? gate : 0)            ; 007CDC73-007CDCA3
unit[idx*8 + 9C0h] = 007B9140(unit, 0)                 ; 007CDCBC, 007CDCC1
unit[idx*8 + 9C2h] = byte unit+520h                    ; 007CDCD0
(float) unit[idx*8 + 9C4h] = 007BCC20(unit)            ; 007CDCE7, 007CDCEE
unit[idx*8 + 9C3h] = 007CD9A0(unit)                    ; 007CDCFF, 007CDD04
```

An entry is `{ byte, byte, byte, byte, float }` and there are two, `9C0h`-`9C7h` and
`9C8h`-`9CFh`. Consumers read it with the **previous** index `[00F876B8]`: `004648FB`,
`007BA747`, `0089E98A` (over a squadron's `+3D0h` slot array, indices `0`-`4`), `0089ECBD`,
`009A85EF`, `009A8745`, `009FA3FB`. The plane's own constructor initialises the offset as bytes:
`007CFE3F MOVUPS [ESI+9B8h],XMM0`, `007CFE4C MOV byte [ESI+9C0h],BL` (`BL = 0`),
`007D014A MOVUPS [ESI+9BCh],XMM0`.

**The `MaxSpeed` float (ship family).** `BSP_UnitInstance_GetReferenceSpeed` (`0080FC30`) reads
`[ESI+9C0h]` with `FLD` at `0080FC58` and `0080FC72`, and `BSP_UnitInstance_SEntityInit`
(`00822C20`) writes it at `00822C65`. `00822C20` appears in exactly five vtables - `00CF9150`,
`00CFA818`, `00CFB7D8`, `00CFC470`, `00D09718` - and each is a class-`06` family vtable plus
`A0h` (`MBattleship`, `MCargo`, `MCruiser`, `MDestroyer` and the class-`06` base). **No plane
vtable carries it**, and no routine mixes the two readings.

---

# B. Unit message arm slot owners

## The vtable lengths settle the question

Measuring the run of code pointers at each unit vtable (`local/vtlen.py len`) shows the unit
hierarchy's tables stop at very different lengths, so a slot number is only meaningful for the
classes whose table reaches it.

| class | vtable | length | `+144h` | `+208h` | `+220h` | `+224h` | `+238h` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `05` unit base | `00D1A698` | `204h` | `0077F2D0` | - | - | - | - |
| `06` ship base | `00D09678` | `240h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `07` `MDestroyer` | `00CFC3D0` | `2C4h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `08` `MSubmarine` | `00D0BF80` | `244h` | `0077F2D0` | `00853230` | `008198A0` | `008137B0` | `008206F0` |
| `09` `MMothership` | `00D01630` | `254h` | `0077F2D0` | `008141E0` | `007583A0` | `007583F0` | `008206F0` |
| `0A` `MCruiser` | `00CFB738` | `248h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `0B` `MCargo` | `00CFA778` | `240h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `0C` `MLandingShip` | `00CFFA30` | `250h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `0074A4C0` |
| `0D` `MBattleship` | `00CF90B0` | `240h` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `0E` `MTorpedoBoat` | `00D0C648` | `27Ch` | `0077F2D0` | `008141E0` | `008198A0` | `008137B0` | `008206F0` |
| `0F` plane base | `00D05F20` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `10` `MPlaneBomber` | `00D06638` | `214h` | `0077F2D0` | `007D0B80` | - | - | - |
| `11` `MPlaneTorpedoBomber` | `00D1A000` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `12` `MPlaneDiveBomber` | `00D19D28` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `13` `MPlaneFighter` | `00D06920` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `14` `MReconPlane` | `00D00070` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `15` `MSmallReconPlane` | `00D0BA80` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `16` `MLargeReconPlane` | `00D00308` | `224h` | `0077F2D0` | `007D0B80` | `004499C0` | - | - |
| `17` `MPlaneKamikaze` | `00D1A2D8` | `20Ch` | `0077F2D0` | `007D0B80` | - | - | - |
| `19` `MLandVehicle` | `00CFFDE0` | `204h` | `0077F2D0` | - | - | - | - |
| `1B` `MLandFort` | `00CFF3F8` | `210h` | `0077F2D0` | `007460E0` | - | - | - |
| `1C` `MCommandBuilding` | `00CFB028` | `21Ch` | `0077F2D0` | `007460E0` | - | - | - |
| `35` unnamed(`05`) | `00CFCD60` | `204h` | `0077F2D0` | - | - | - | - |
| `45` `MAirfield` | `00CF8C08` | `20Ch` | `0077F2D0` | `006D0D90` | - | - | - |
| `46` `MShipyard` | `00D0B770` | `204h` | `0077F2D0` | - | - | - | - |

`MLargeReconPlane`'s `+220h` is its own last slot, an unrelated class-specific virtual; do not
read it as a failure setter.

Three structural facts follow.

1. **`+144h` is never overridden.** All 25 tables carry `0077F2D0`. That is consistent with its
   only message call site, `0095AC28` in `BSP_Unit_HandleMessage` (body `0095ABE0`-`0095AE1F`),
   the class-`05` base handler every unit reaches.
2. **`+208h` is index `130`, the first slot past the class-`05` base's table.** Each family adds
   its own, unrelated virtual there, with its own signature. The `vcall` scan finds five load
   sites for the slot: `00821F12` in `BSP_UnitInstance_HandleMessage` (the `7Bh` arm),
   `007CCFBC` in `BSP_Plane_HandleMessage`, `00744C0C` in `FUN_00744BE0`, `006D293F`, and
   `007B9279`. So `MT_VEHICLE_UNIT_LAUNCH` is the contract of the **ship family's** slot only.
3. **`+220h`, `+224h` and `+238h` exist only on the class-`06` ship family**, which matches
   their only message call sites (`00822122`, `00822142`, `00821F63`) all being in
   `BSP_UnitInstance_HandleMessage`. A plane, land vehicle, fort, airfield or shipyard cannot
   receive kinds `6Ah`-`6Eh` or `94h`: the slot is past the end of its table.

## The bodies

### `0077F2D0`, slot `144h` - the single-device role path

`__thiscall(unit, int slot)`, `RET 4`, body `0077F2D0`-`0077F354`.

| step | site | rule |
| --- | --- | --- |
| assign | `007F2D8` -> `009278A0` | `009278A0(unit, slot)` does the actual station assignment (`contract: unread`) |
| class gate | `007F2E6`, `007F2F5`, `007F304` | return unless `vtable[5Ch](5)`; return if `vtable[5Ch](1Ch)` (`MCommandBuilding`) or `vtable[5Ch](1Bh)` (`MLandFort`) |
| side gate | `007F30A`-`007F323` | `side = unit+188h`; return if `side == 9` or `side == [[00E188A8]+18ECh]` (the local side) |
| owner gate | `007F325`-`007F332` | return if `unit+54h != [[00E188A8] + local*4 + 18CCh]->+28h` |
| mark | `007F334`-`007F34E` | `*BSP_MessageSystem_EntitySuppressSlot([00F8A0C4]+D4h, &unit) = 1` |

The key handed to the map is the **unit**: `EAX` is computed at `0077F33A` from `[ESP+8]` before
the `PUSH`, and `0077F345 MOV [ESP+0Ch],ESI` overwrites the incoming `slot` argument slot with
the unit pointer at the same address.

### `008141E0`, ship slot `208h` - `7Bh MT_VEHICLE_UNIT_LAUNCH`

The arm at `00821F05` builds the call as `EAX = 0080F710(msg)`, then the float `msg+20h`, so the
signature is `__thiscall(unit, int resolved, float heading)`. Ghidra body
`008141E0`-`00814347`.

| step | site | rule |
| --- | --- | --- |
| survey | `008141E9`-`00814215` | walk the device list at `unit+48h` (next at `+44h`), count devices answering `vtable[5Ch](28h)` (`MCatapult`), and call `device->vtable[1D0h](1)` on each; `ready` starts as `unit+638h > 0` and any device answering false clears it |
| cost gate | `00814232`-`0081427E` | when `[00E188A8]+1FE4h != 0`, `BSP_Game_GetEffectiveGameMode() < 4` and (`resolved >= 0` or the local side `>= 0`): return if `00946F00()` is below `BSP_VehicleClass_GetOrCreate()+124h` |
| select | `008142C3`-`00814327` | with more than one catapult, keep the device minimising `abs(BSP_Math_SubtractWrappedAngle(heading, 006ED000(device, ...)))`; the winner lands in `EDI` at `00814320` |
| launch | `00814329` onwards | `006EB680(...)` |

`coverage: partial`: the exact frame slots feeding `006ED000` and the tail past `00814329` were
not resolved.

### `008198A0` / `007583A0`, slot `220h` - the failure setters

`008198A0`, `__thiscall(unit, msg)`, an SEH frame, body `008198A0`-`00819A1F`. Kinds come from
`msg->vtable[0Ch](kind)`.

| kind | site | rule |
| --- | --- | --- |
| `6Ah` `MT_SHIP_SET_EXPLOSIONFAILURE` | `008198C6` | `00876E20()`, `operator new(38h)`, `008782A0(msg+24h, msg+28h)`, then `00818110(unit+310h, effect)` - it attaches a point effect to the unit's node |
| `6Bh` `MT_SHIP_SET_STEERINGJAMFAILURE` | `0081999D` | `unit+9E4h = 1` |
| `6Ch` `MT_SHIP_SET_ENGINEJAMFAILURE` | `008199C7` | `unit+9E5h = 1`, then `BSP_PointEffect_StopChildren` (`00867B10`) on every non-null `unit+A14h[i]` for `i < unit+A18h` |

`007583A0` is `MMothership`'s override, `RET 4`, body `007583A0`-`007583EA`: kind `72h` sets
`unit+11A4h = 1`, kind `74h` sets `unit+11A5h = 1`, and anything else tail-calls `008198A0` at
`007583E1`.

### `008137B0` / `007583F0`, slot `224h` - the failure clears

`008137B0`, `__thiscall(unit, msg)`, `RET 4`, body `008137B0`-`00813821`.

| kind | site | rule |
| --- | --- | --- |
| `6Dh` `MT_SHIP_CLEAR_STEERINGJAMFAILURE` | `008137BD` | `unit+9E4h = 0` |
| `6Eh` `MT_SHIP_CLEAR_ENGINEJAMFAILURE` | `008137D8` | `unit+9E5h = 0`, then `00866B70` on every non-null `unit+A14h[i]` for `i < unit+A18h` |

The null test is `NEG ECX / SBB ECX,ECX / TEST ECX,0E186ECh / JE` at `008137FF`-`00813809`: the
`NEG`/`SBB` pair yields `0` or `-1`, so the odd mask only distinguishes null from non-null.

`007583F0` is `MMothership`'s override, `RET 4`, body `007583F0`-`0075843A`: kind `73h` clears
`unit+11A4h`, kind `75h` clears `unit+11A5h`, and anything else tail-calls `008137B0` at
`00758431`.

So `unit+9E4h` is the steering-jam flag and `unit+9E5h` the engine-jam flag on the ship family,
`unit+A14h`/`+A18h` are the unit's point-effect array and count, and an engine jam stops those
effects while the clear restarts them. (Note the offsets `+9E4h`/`+9E5h` are floats on the plane
family per `docs/PILOT_COMMAND_PATH.md` - another offset two sibling families reuse.)

### `006D0D90`, `MAirfield` slot `208h`

`__thiscall(unit, msg)`, `RET 4`, body `006D0D90`-`006D0DCD`: kind `73h` clears `unit+748h`,
kind `75h` clears `unit+749h`, nothing else. The airfield's *set* kinds (`72h`, `74h`) are not
in this slot; they are somewhere in the caller `006D293F`'s handler, which was not read.

### `008206F0` / `0074A4C0`, slot `238h` - `94h MT_SHIP_STARTLANDING`

`008206F0`, `__thiscall(unit)`, Ghidra body `008206F0`-`00821E78`, an SEH frame and `3ECh` of
stack: it returns `-1` when `[unit+538h]+78Ch == 0` (`0082071F`-`0082072B`). `coverage: partial` -
only the entry gate was read of a `1788h`-byte body. It is also the `95h` producer that the
existing table records.

`0074A4C0` is `MLandingShip`'s override; Ghidra has no function for it. It returns `-8` when
`unit+1200h != 0` (`0074A4DB`-`0074A4E4`) and otherwise tests `unit+C8h` at `0074A4F9`.
`coverage: partial`.

`00853230`, `MSubmarine`'s `+208h` override, was not read: `contract: unread`.

## The message-kind names for `72h`-`78h`

The image carries 235 `MT_*` literals and their order is the reverse of the kind order, with
**index = `234 - kind`**. Three independent anchors fix the mapping: `6Ah` ->
`MT_SHIP_SET_EXPLOSIONFAILURE` (index 128), `79h` -> `MT_VEHICLE_GUN_CONTROL` (113) and `7Ah` ->
`MT_VEHICLE_SHIPYARD_LAUNCH` (112), all three matching `docs/UNIT_MESSAGE_ARMS.md`.

| kind | literal |
| --- | --- |
| `72h` | `MT_AIRBASE_SET_RUNWAYFAILURE` (`00D023E4`) |
| `73h` | `MT_AIRBASE_CLEAR_RUNWAYFAILURE` (`00D023C4`) |
| `74h` | `MT_AIRBASE_SET_HANGARFAILURE` (`00D023A4`) |
| `75h` | `MT_AIRBASE_CLEAR_HANGARFAILURE` (`00D02384`) |
| `76h` | `MT_FORMATION_JOIN` (`00D02370`) |
| `77h` | `MT_FORMATION_LEAVE` (`00D0235C`) |
| `78h` | `MT_FORMATION_SET` (`00D02348`) |

So `MMothership`'s `+220h`/`+224h` overrides carry the carrier's runway and hangar failure
flags, and `MAirfield`'s `+208h` carries the airbase's - the `71h`-`79h` block is not a plain
default row for those two classes.

## The producer of `6Ch MT_SHIP_SET_ENGINEJAMFAILURE`

The kind is not computed. It is a **byte immediate inside a dedicated message constructor**,
which is why a scan of the kind pushed at each of the 426 `0075B430` call sites missed it.

`007619B0`, `__thiscall(msg)`:

```
msg+4h  = 3                                          ; 007619B5
msg+8h  = msg+Ch = 0                                 ; 007619BC, 007619BF
[msg]   = 00D02C68                                   ; 007619C2
msg+10h = 6Ch                       <-- the kind     ; 007619C8  C6 40 10 6C
side    = [[00E188A8]+18ECh]
msg+14h = (0 <= side <= 7) ? [[00E188A8] + side*4 + 18CCh] : 0   ; 007619CC-007619EF
```

Three callers: `00768FA6`, `00813530` and `00827D65`. `00813530` is in `FUN_008132C0` (body
`008132C0`-`008135B5`), the Lua failure binding, whose three string branches are
`"Explosion"` (`00D080C8`) -> `PUSH 6Ah` at `008133D5`, `"SteeringJam"` (`00D09460`) ->
`PUSH 6Bh` at `008134C4`, and `"EngineJam"` (`00CF6124`) -> `007619B0` at `00813530`, after
which it routes the message with `0077C2A0(unit, msg, 7, 0)` at `00813546` and calls
`0093BD80(unit+A20h, 7)` at `0081356F`. `00827D65` is the damage-side producer, in the
`BSP_ShipInstance_OnHealthChanged` range.

---

# C. Director loose ends

## C1. The twenty sentinels at `+1D0h`-`+21Fh`: nothing reads them

`BSP_CommandControllerBase_Construct` (body `00720180`-`00720442`) loads `0D01502F9h` into `EAX`
at `0072032E` and stores it to twenty consecutive dwords, `+1D0h` through `+21Ch`
(`00720333`-`007203A5`). As a float that bit pattern is **exactly `-1.0e10`**, so the band is
twenty "no value yet" floats rather than a guard pattern.

| scan | result |
| --- | --- |
| the immediate `0D01502F9h` over `.text`, `.rdata`, `.data`, `.rsrc` | two raw occurrences; one decodes to the fill at `0072032E`, the other (`00CE4ADC`) falls inside a string blob (`"type\0\0\0\0msg\0"` then the four bytes then `"},},};"`). **No routine anywhere compares against the sentinel** |
| every disp32 in `1D0h`-`21Fh` over `00700000`-`00730000` | 41 matches: the fill itself, and reads that belong to gun classes - `BSP_Gun_RearmBarrel` (`0072D531`, `0072D54C`), `FUN_0072D5D1` (`0072D63E`, `0072D659`), `BSP_Gun_SpawnShotAndEffects` (`0072F9CF`, `0072FB22`), `FUN_00729F70` (`00729F83`), `FUN_0070E450` (`0070E487`) |
| the two `CommandControllerBase` users' own code | `00720180`, `00720450`, `00720850`, `0071F290`, `0071F600` and `00721760` never read the band outside the fill |

`coverage: partial` on the scan itself: those displacements are common on unrelated classes, so
matches outside `00700000`-`00730000` could not be attributed to a director. The decisive
evidence is the single immediate and the absence of any comparison against it anywhere.

## C2. `008637D0`, the per-weapon availability test

`__thiscall(unit, Entity* target)`, `RET 4`, returns `AL`, Ghidra body `008637D0`-`0086383C`.
`00E0A510` is a list of weapon-category ids terminated by any entry `>= 0Ch`.

```
i = 0
while ([00E0A510 + i*4] < 0Ch):                        ; 008637D5, 0086381A
    cat = [00E0A510 + i*4]
    if byte unit[cat + 70h] != 0                       ; 008637F2
       and [unit+60h]->vtable[4h](cat)                 ; 008637F9-00863802
       and BSP_UnitGunneryAi_CategoryAcceptsTarget(unit, cat, target):   ; 0086380E
        return 1                                       ; 00863837
    i += 1
return 0                                               ; 0086382E
```

So the rule is: **true when any category in the global list is present on this unit
(`unit+70h[cat]`), accepted by the object at `unit+60h`, and willing to engage this target.**
`008633D0 BSP_UnitGunneryAi_CategoryAcceptsTarget` is `__thiscall(gunneryAi)(int category,
Entity* target)`, `RET 8`, already in the ledger. `009F1BC0` ANDing this with `aaEnabled`
therefore reads "this unit has an anti-aircraft weapon that will engage the target **and** the
director's anti-aircraft enable is set".

## C3. Which entity `director+34h` is

`00720219 MOV [ESI+34h], EDI` in the base constructor, with `EDI = [ESP+28h]` - the
constructor's **only stack argument** (`0072019E`). There are exactly two construction sites in
the image:

| site | class | argument passed to `+34h` |
| --- | --- | --- |
| `00836403` | the weapon director; installs `00D09EC0` at `[this]` and `00D09EA8` at `+1Ch` | its own first stack argument, forwarded (`008363F6 MOV EAX,[ESP+14h]`, `008363FE PUSH EAX`) |
| `0084D844` | `FUN_0084D810` (body `0084D810`-`0084D8E3`); installs `00D0BD98` at `[this]` and `00D0BD7C` at `+1Ch`, and stores the owning unit at `+224h` (`0084D856`) | `unit+310h`, the unit's tick-element node, or `0` when the preceding test fails (`0084D837`-`0084D843`) |

So `+34h` is the **parent object the controller attaches to**, not the owning unit; the
constructor also calls something on it with the controller as an argument at `007201A4`-`007201A7`.
The two `finished` producers and `0071F600` are reporting different fields on purpose:
`+34h` is the parent, `+24Ch` the weapon director's own owning unit, `+224h` the other
subclass's. No producer is wrong.

## C4. The previous-command record at `+16Ch` has exactly one reader

`BSP_CommandControllerBase_CopyStateFrom`, `__thiscall(dst, src)`, Ghidra body
`00720450`-`007207BA`, reads `dword [src+16Ch]` at `0072046B` and copies the record's floats
`+178h`, `+17Ch`, `+180h` and `+184h` into the destination (`007204AB`-`007204D5`), alongside the
bytes `+3Ch` and `+3Dh`. Its only rel32 caller is `00836689` in
`BSP_WeaponDirector_CopyStateFrom` (body `00836680`-`008366C5`), and it appears as a vtable slot
at `+F0h` of `00CFD99C` (length `124h`) and `+4Ch` of `00D0BD98`.

So the record **is** read, but only by a wholesale state clone - nothing consults it to decide
behaviour, which is consistent with `docs/COMMAND_COMPLETION.md`'s finding that it is not a
revert source.

One near miss ruled out: `FUN_00721760`'s `MOV CX, word [ECX+174h]` at `007217A8` is not a band
read. That routine is `__thiscall(out, void* target, const float* point, float f)`, `RET 0Ch`,
body `00721760`-`007217BA`, a command-record builder; the `ECX` it reads is its first stack
argument (`0072177D MOV ECX,[ESP+4]`), so `+174h` is a word id on the **target**, copied into
`out+2h`. Its only caller is `0072180E`.

## C5. `0084E010`'s owner and vtable slot

`0084E010` (`__fastcall(this)`, Ghidra body `0084E010`-`0084E5DD`) is referenced exactly once in
the whole image: the dword at `00D0BE14`. Walking back the run of code pointers around it gives
table base `00D0BD98` and slot **`+7Ch`**. `00D0BD98` is the table `FUN_0084D810` installs at
`[this]` (`0084D849`), and its code-pointer run ends at `+80h` - the two dwords after `0084E010`
are zero and string data follows - so `+7Ch` is its last live slot.

The owner is therefore the class `FUN_0084D810` constructs: a `BSP_CommandControllerBase`
(`00720180`) with vtables `00D0BD98` at `+0h` and `00D0BD7C` at `+1Ch`, the owning unit at
`+224h`, built from `007F2070` and `007F5000` (neither has a Ghidra function; both are in the
squadron and air-operations range). This is also the fourth table holding `0071F290`, at slot
`+0Ch` (`00D0BDA4`), which answers that open question in `docs/DIRECTOR_UPDATE_ARMS.md`.

`coverage: none` for `0084E010`'s body, unchanged from `docs/COMMAND_COMPLETION.md`. One
unsettled point: `00D0BD98` is `80h` long while `BSP_CommandControllerBase`'s own `00CFD99C` is
`124h`, so `00D0BD98` cannot be a straightforward override table for it; how the two relate was
not established.

## C6. `00F87574`-`7Ch` is the zero vector

`.data` has `rawptr 00A08000` and `rawsize 10000h`, so file bytes back only the virtual range
`00E08000`-`00E18000`; the section's virtual size `297EDCh` runs on to `0109FEDC`. `00F87574`
therefore lies in the zero-filled tail and is `0.0f` at load unless something writes it.

Scanning `.text` for each of the three absolute addresses:

| address | raw occurrences | decoded operand references | stores |
| --- | --- | --- | --- |
| `00F87574` | 262 | 520 | **0** |
| `00F87578` | 174 | 516 | **0** |
| `00F8757C` | 175 | 520 | **0** |

No `MOVSS dword ptr [0F875xxh], xmm`, no `MOVUPS xmmword ptr [...]`, no `FSTP dword ptr [...]`,
no `MOV dword ptr [...], reg`, and no bitwise store form exists for any of the three. Every
reference is a load (`F3 0F 10 05 ...` / `0F 10 05 ...` / `D9 05 ...`, plus the misaligned
`ADC byte ptr` artifacts a linear decoder produces from those same bytes).

**The shared vector is `(0, 0, 0)` for the whole run.** Residual uncertainty: the address is
taken at a few sites - `MOV EAX, 0F87574h` at `00432DDC`, `00433834`, `00433CE3`, `00456483`,
`MOV EBP,` at `00441088`, `MOV ECX,` at `0059BD2B`, `PUSH 0F87574h` at `006DF792` and
`006E141C` - and the two `PUSH` sites read as handing the zero vector in as an input point
(`006DF792` pushes it alongside a float and an out-pointer). A callee writing through one of
those taken addresses was not exhaustively excluded, but 520 absolute loads against zero
absolute stores is a read-only constant.

---

## Corrections to earlier docs

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/PLANE_FLIGHT.md` | line 163: "when `ctl->+FCh != 1` the whole roll term is discarded" | the roll accumulator is discarded when `ctl+FCh == 1`, i.e. **on the ground**; the mode gate keeps it in free flight and on the water | `007DA8D9 CMP dword [ESI+FCh],1`; `007DA8E0 XORPS XMM2,XMM2`; `007DA8E3 JNE 007DA8EB`; `007DA8E5 MOVSS [ESP+18h],XMM2` - the store is reached only when the compare is equal |
| `docs/PLANE_FLIGHT.md` | line 79: `+FCh` is "a mode dword, zeroed each step and tested `== 1`" | it is not zeroed each step; each of the three laws writes its own value, and it is read six ways including a four-way and a three-way dispatch | the three writers `007DC841`/`007DCD24`/`007DCDDC` and the six readers listed above; the whole disp32 `FCh` scan over `007D7000`-`007DE000` |
| `docs/PLANE_GROUND_OPS.md` | "`2` in the water law (`007DCDD3`)" | the store instruction starts at `007DCDDC` | `007DCDDC C7 86 FC 00 00 00 02 00 00 00` |
| `docs/PLANE_GROUND_OPS.md` open questions | "What `classDesc+1FCh` is ... has no key" | the undercarriage height reference: the distance from the model origin to the wheels | `007DB70B FSUB [ECX+1FCh]` builds the ground plane's distance; `007DA30A`-`007DA318` subtracts it plus `0.01` from the sampled deck height before lifting the pose |
| `docs/PLANE_GROUND_OPS.md` open questions | "Which of `unit+ACCh`, `unit+B1Ch` and `unit+BFCh` the physics body writes" | `unit+BFCh` is not written by the physics body: it is `006BC530`'s out-parameter | `007BB2E3 LEA EAX,[ESI+BFCh]` / `007BB2E9 PUSH EAX` before `006BC530`, and `007C5F1C`/`007C5F22` likewise; these are the only unit-based `LEA`s in the `0B82h`-`0C01h` displacement band across `00600000`-`00A00000` |
| `docs/PILOT_COMMAND_PATH.md` | line 389: "`docs/MOTION_DIFFERENTIAL.md` reads `unit+9C0h` as a ship's maximum speed ... the two layouts cannot both hold at the same base" | they hold at two different bases: the stride-8 byte record is the plane family's and the `MaxSpeed` float is the class-`06` ship family's | `00822C20 BSP_UnitInstance_SEntityInit` writes the float at `00822C65` and appears only in `00CF9150`, `00CFA818`, `00CFB7D8`, `00CFC470` and `00D09718`, each a class-`06` vtable `+A0h`; no plane vtable carries it, while `007CFE4C MOV byte [ESI+9C0h],BL` in the plane constructor and `007CDC70`'s five stride-8 stores treat it as bytes |
| `docs/UNIT_MESSAGE_ARMS.md` | "`[208h]` (`MT_VEHICLE_UNIT_LAUNCH`: `(slot, float)`)" as one unresolved slot | slot `+208h` holds four unrelated per-family virtuals, because the class-`05` base vtable ends at `204h`. `(slot, float)` is the ship family's contract only; the plane's is `BSP_Plane_HandleStateMessageKinds(msg)`, the fort's `007460E0(msg)` and the airfield's `006D0D90(msg)` | measured vtable lengths (class `05` `204h`, plane base `20Ch`, ship base `240h`); the five `vcall` load sites `00821F12`, `007CCFBC`, `00744C0C`, `006D293F`, `007B9279` in four different handlers |
| `docs/UNIT_MESSAGE_ARMS.md` | slots `220h`, `224h`, `238h` as unresolved unit virtuals | they exist only on the class-`06` ship family; every other unit vtable ends before them. Two bodies each: `008198A0`/`007583A0`, `008137B0`/`007583F0`, `008206F0`/`0074A4C0` | the vtable length table above; the three slots' only message call sites (`00822122`, `00822142`, `00821F63`) are all in `BSP_UnitInstance_HandleMessage` |
| `docs/UNIT_MESSAGE_ARMS.md` | `71h`-`79h` "default; the base arms `79h`" | `72h`-`75h` are the airbase runway and hangar failure set/clear pair and are **not** defaults for `MMothership` (`007583A0`/`007583F0`, `unit+11A4h`/`+11A5h`) or `MAirfield` (`006D0D90`, `unit+748h`/`+749h`) | the `MT_*` literal order (index = `234 - kind`, three anchors); `007583AD PUSH 72h`, `007583C8 PUSH 74h`, `007583FD PUSH 73h`, `00758418 PUSH 75h`, `006D0D9D PUSH 73h`, `006D0DB8 PUSH 75h` |
| `docs/UNIT_MESSAGE_ARMS.md` | `6Ch` producer "**unresolved**, kind computed" | the producer is `008132C0`'s `"EngineJam"` branch through the constructor `007619B0`; the kind is a byte immediate, not computed | `007619C8 MOV byte [EAX+10h],6Ch`; `00813519 PUSH 0CF6124h` ("EngineJam"), `00813530 CALL 007619B0`, `00813546 CALL 0077C2A0` |
| `docs/COMMAND_COMPLETION.md` open questions | "Which entity `director+34h` is" | the base constructor's only stack argument - the parent object the controller attaches to; `unit+310h` for the `0084D810` class | `0072019E MOV EDI,[ESP+28h]`, `00720219 MOV [ESI+34h],EDI`; `0084D839 LEA EAX,[EDI+310h]` / `0084D843 PUSH EAX` / `0084D844 CALL 00720180` |
| `docs/COMMAND_COMPLETION.md` open questions | "Whether anything reads the previous-command record at `+16Ch`. A targeted scan was not run" | one reader: `BSP_CommandControllerBase_CopyStateFrom`, a state clone | `0072046B MOV EDX,[EDI+16Ch]` with `EDI = [ESP+44h]` (the source controller) and `EBP = ECX`; `007204AB`-`007204D5` copy `+178h`..`+184h` |
| `docs/COMMAND_COMPLETION.md` open questions | "`0084E010`'s owner and its vtable slot" | slot `+7Ch` of `00D0BD98`, the table `FUN_0084D810` installs; the owner keeps its unit at `+224h` and is built from `007F2070` and `007F5000` | the only reference to `0084E010` in the image is the dword at `00D0BE14`, inside the code-pointer run starting at `00D0BD98`; `0084D849 MOV [ESI],0D0BD98h` |
| `docs/DIRECTOR_UPDATE_ARMS.md` open questions | "The twenty `D01502F9h` sentinels at `+1D0h`-`+21Fh`. Nothing in this packet reads them" | nothing in the **image** reads them; the value is the float `-1.0e10` | the immediate occurs in exactly one instruction image-wide (`0072032E`); all `1D0h`-`21Fh` reads in `00700000`-`00730000` belong to gun classes |
| `docs/DIRECTOR_UPDATE_ARMS.md` open questions | "Whether the shared vector at `00F87574`-`7Ch` is a plain zero ... not chased" | it is the zero vector: `.text` holds 1556 decoded operand references to the three addresses and not one store | the store-form scan over all three addresses returns nothing; `00F87574` lies past `.data`'s `rawsize 10000h` |
| `docs/DIRECTOR_UPDATE_ARMS.md` open questions | "The fourth vtable holding `0071F290` at `00D0BDA4`" | `00D0BDA4` is slot `+0Ch` of `00D0BD98`, the table `FUN_0084D810` installs on the class that owns `0084E010` | `0084D849 MOV [ESI],0D0BD98h`; the run walk-back puts `0071F290` at `+0Ch` and `0084E010` at `+7Ch` of the same table |

## Open questions

* `007DA380`'s mode-`1` and mode-`2` arms between their entry and exit stores, and the rate law's
  tail past `007DAB6C`.
* `007DD35C`'s base: a float at `+FCh` on some object inside the water law, not the mode.
* `009278A0`, `00876E20`, `008782A0`, `00818110`, `00866B70`, `006ED000`, `006EB680`,
  `00946F00`, `0093BD80`, `007B9140`, `007BCC20`, `007CD9A0`, `007D7D70`, `006BC530`:
  `contract: unread`.
* `00853230` (`MSubmarine`'s `+208h`), `007460E0` (the fort's), and `006D293F`'s handler, which
  must hold the airbase `72h`/`74h` set arms.
* `008206F0` past `00820744` and `0074A4C0` past `0074A4F9`.
* `0084E010`'s body, and how `00D0BD98` (`80h`) relates to `BSP_CommandControllerBase`'s
  `00CFD99C` (`124h`) when `0084D810` calls the base constructor on the same `this`.
* Whether state `3` is ever actually produced by mission data - nothing in the image can answer
  that.
* Whether `unit+C00h`/`+C01h` were read in an earlier build. A shipped image cannot say.

## `no_ghidra_function`

| address | name | end_address |
| --- | --- | --- |
| `006EB5C0` | `BSP_Plane_IsLaunching` | `006EB5CC` |

Routines this doc describes but does not name, for which Ghidra also has no function:
`0074A4C0`, `00853230`, `006D293F`'s container, `007F2070`, `007F5000`, `007B9279`,
`0074CE90`, `006D14E0`, `008DAD80`, `0071C580`. None is given a name here.

## `flow_gaps`

None. Every listing in this doc was decoded from the disk bytes with a linear decoder
(`local/pescan.py at`), so no `ghidra disasm` instruction runs were dropped; consecutive
addresses were checked in each quoted block.
