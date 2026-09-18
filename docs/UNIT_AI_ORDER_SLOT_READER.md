# Who reads the ship AI's order slot, and what the last hop really is

Addresses: 00811D10 00815EE0 00815F30 0080E000 00811960 00825F20 00825F2C 00825F77 00826708
0082674C 00826C61 00826CDB 009D4F70 009D8CE0 009D8D2C 009E3DB0 009ED6CE 009EE649 009F4D10

Packet `cc_ai_order_hop`, worker `agent/cc-ai-order-hop`, 2026-09-12 UTC. Ghidra was read-only
for this packet. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every
live query verified both. Descriptive names are hypotheses, not recovered symbols.

## Answer to the packet question

**Nothing turns the promoted slot's `+40h`, `+44h` or `+48h` into the order ring's `+148h` /
`+14Ch`, because the slot is not a command to this unit. It is a prediction this unit publishes
for other units to read.** Three facts settle it, and the third is the one the follow-up in
`docs/UNIT_AUTOPILOT_PAIR.md` could not reach because it grepped `00825F20`'s decompiled body
for unit-relative offsets while that function's `param_1` is `unit+310h`.

1. **The promotion deliberately leaves the order behind.** `00825F77` calls `00811D10` with the
   slot the unit is leaving as the source and the slot it is moving to as `this`. That routine's
   whole body is `00811D14..00811D71`, sixteen field copies over `+00h..+3Fh`, then `RET 4` at
   `00811D74`. `+40h`, `+44h`, `+48h`, `+4Ch` and `+50h` are outside it. So the order the AI just
   published stays where it was written, at `unit + 0A98h + 54h * index` after the flip, and the
   slot the unit is now on keeps a stale triple that nothing refreshes.
2. **Every reader addresses that leftover slot, and every reader is another unit's AI.** The
   scan below finds twelve computations of a slot address in `.text`. Six are the writers and the
   promotion; the rest read the published form. `009D8D2C..009D8DB9`, inside `FUN_009D8CE0`, is
   the clearest: it takes two different units (`[ESI+14h]` and `[ECX+14h]` through `[EBP+14h]`)
   and reads `published+44h` of both (`009D8D35 FLD [EDX+EAX+0ADCh]`, `009D8D71 FLD [EBX+44h]`)
   together with `published+40h` and `+48h` of the first (`009D8DAA`, `009D8DB9`). A unit does not
   read its own command out of another unit.
3. **What `009ED6B0` puts in the triple in the AI's own direct-control modes is the unit's
   present motion, not a target.** In mode 0 (`blk+1C4h == 0`) `009ED95D` reads the unit's own
   heading through `vtable[50h]`, `009ED9C1` adds or subtracts the yaw rate the *current* steering
   command already produces (`00811940`), and the result is `blk+324h`. `blk+32Ch` is a stopping
   distance built from the unit's own speed (`009ED8D1..009ED902`). A command cannot be defined as
   the answer to "where will I be if I keep doing what I am doing".

The slot is therefore the ship AI's **published navigation state**: a heading, a distance to the
point it is steering at, and the length of path it still has to run.

## The scan

Every reader has to turn `[unit+0B40h]` into a slot address, and `54h` is not a power of two, so
the address arithmetic is always `IMUL r32,r32,54h`. Forty-six `6B /r 54` encodings with a
register ModRM exist in `.text`; twelve of them sit next to a `0B40h` displacement.

| site | containing function | what it does |
| --- | --- | --- |
| `009F4D35` | `009F4D10` | writes `current+40h/+44h/+48h/+4Ch` - the publisher |
| `00825F38`, `00825F64` | `00825F20` | the promotion, inlined |
| `00815EE6`, `00815F10` | none (see the table below) | the same promotion, out of line and unreferenced |
| `009ED6CE` | `009ED6B0` | `0080E000(current, seconds)`, which touches `+00h..+3Fh` only |
| `009EE649` | `009ED6B0` | `00815F30(current, ...)`, which writes `+04h` and reads `+14h..+1Ch` |
| `009D4F76` | none | an out-of-line `published` accessor, unreferenced |
| `009D8D32`, `009D8D51` | `009D8CE0` | reads `published+40h`, `+44h`, `+48h` of two units |
| `009E3DBE` | `009E3C00` | passes `published` to `00811D80` |

The other thirty-four `IMUL 54h` sites belong to unrelated arrays; none of them is within a
`0B40h` read. `009D4F70` and `00815EE0` have no reference of any kind: scanning `.text`, `.rdata`
and `.data` for the absolute dword and `.text` for every `E8`/`E9` `rel32` whose target is either
address returns nothing, so both are out-of-line copies the linker kept after the call sites were
inlined.

## The record, in full

`00811D10`'s copy gives the layout of `+00h..+3Fh` exactly, one field per copy, and shows it is
two identical `1Ch`-byte sub-records: every field at `+08h..+20h` has a twin at `+24h..+3Ch`.
`0080E000` confirms it by resetting both with the same five stores.

| offset | type | field | evidence |
| --- | --- | --- | --- |
| `+00h` | float | a blended value slewed toward `+08h` | `0080E079`, `0080E0A6` |
| `+04h` | float | a countdown | `0080E003`, `0080E028`, set by `00815F30` at `00815F43` |
| `+08h` | float | sub-record A's value, the blend's target | `0080E072` |
| `+0Ch`, `+10h`, `+14h`, `+18h`, `+1Ch`, `+20h` | float x2, byte, float x2, dword | the rest of A | `00811D24..00811D46`, reset at `0080E03D..0080E04F` |
| `+24h`..`+3Ch` | the same again | sub-record B | `00811D49..00811D71`, reset at `0080E052..0080E064` |
| `+40h` | float | the distance to the point, from `blk+32Ch` | `009F4D55` |
| `+44h` | float | the heading target, limited by `00811960` | `00811A19` |
| `+48h` | float | the remaining path length, from `blk+330h` | `009F4D62` |
| `+4Ch` | byte | the published flag | `009F4D58`, cleared `00825F4E` |
| `+50h` | pointer | the unit | `0081F1EC`, read `00811964` |

`0080E000` (`__thiscall(record)(float)`, `RET 4`, body `0080E000-0080E0EA`, complete) is the
record's own step and `009ED6DE` is its only call site. While `+04h` is at or above zero it is
decremented by the frame's seconds, and the decrement that drives it below zero clears both
sub-records with `+10h` and `+2Ch` set to `1000.0f` (`00CE3804`). Then `+00h` moves toward `+08h`
by at most `seconds * 50.0` (`00CE3938`, a double) and snaps when the gap is no larger.

### `00811960`, the limit on `+44h`

`__thiscall(slot)(float desired)`, `RET 4`, body `00811960-00811A22`. It reads its own `+50h` as
the unit, takes that unit's heading through `vtable[50h]` (`0081196C`) and its forward speed
through `0092D730` on `unit+1018h` (`0081197B`). At or below `-1.0f` of forward speed the
reference heading is turned by the image's pi (`0081198C..008119A5`, `00D7A264`). The desired
heading is then folded into that reference, clamped and unfolded:

```
delta = 00438B10(reference, desired)                    ; 008119BB
if (delta < -0.785398185253) delta = -0.785398185f      ; 008119C8, 00D09448 / 00D09440
else if (delta >  0.785398185253) delta =  0.785398185f ; 008119DE, 00CEDCD0 / 00CEB5A8
slot+44h = 00438B10(reference, delta)                   ; 00811A14, 00811A19
slot+4Ch = 1                                            ; 00811A1C
```

The bound is `+/- pi/4`, so the window spans a quarter turn and the published heading can never
be more than 45 degrees off the unit's own heading, or off its reverse heading while making
sternway.

## What `00825F7C..00826D6B` actually contains on the steering path

The packet expected the slot reader here. What is here instead is the hop from the **ring's**
rudder to the unit's heading, which no earlier packet had pinned:

| address | what happens |
| --- | --- |
| `00826C61` | `FLD [ESI+984h]` - the ordered rudder the ring tick wrote |
| `00826C75` | `00811890(unit, that rudder)` - the yaw rate, `docs/UNIT_RUDDER_CURVE.md` |
| `00826CDB` | `unit->vtable[50h](yawRate)`, one float argument. **The two call sites of slot `50h` disagree on the prototype and this packet does not resolve it**: `0081196C` calls it with nothing pushed and `RET 0`, which its own epilogue proves (`00811A20 POP ESI; POP ECX; RET 4` only balances if the call cleans nothing), while `00826CD5..00826CD8` sets up one outgoing float and `00826CDE` reloads `ST0` as the result. Either the two objects carry different vtables or one reading is wrong |

and, gated on `unit+61h` at `008266C1`, the inlined pair of ring setters at `008266CE..0082674C`
that overwrite every ring slot and `ring+148h`/`+14Ch` from `unit+0CB4h`/`unit+0CCCh`, which are
`unit+0FC4h`/`unit+0FDCh` seen from `EDI = unit+310h`. `docs/UNIT_MANUAL_AUTOPILOT.md` carries
that half.

The order ring itself is settled in `docs/UNIT_STATE_MESSAGE.md`: `ring+148h` and `ring+14Ch` are
written only by `00813020`'s two `BSP_Math_StepTowards` calls toward `slot[readCursor]`, by the
two immediate setters `0080D9B0`/`0080DA00`, by the ring constructor and by the inlined pair
above. `slot[writeCursor]` is filled only by `0080DAD0`, whose one caller is `00816A40`, whose
three callers are all HUD order routines (`0064BB12`, `00651AA3`, `0067C7CB`). Scanning `.text`
for every `E8` `rel32` reaching `00816A40` or `0080DAD0` finds exactly those four sites and no
other, so on the evidence in the image the ring is a player-order and network path and the AI
never writes it.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `00811D10` | reconstructed as `unit_ai_order_copy_00811d10`, build-tested | complete |
| `0080E000` | reconstructed as `unit_ai_order_slot_step_0080e000`, build-tested | complete |
| `00825F2C..00825F7C` | corrected in `unit_promote_ai_order_00825f2c`, and now run at the head of `ship_motion_step_00825f20` where `00825F20` runs it. `ShipMotionState` carries the slot pair and `ShipMotionStepResult` reports whether the flag was found set | complete |
| `00811960` | read again and its two bounds resolved; reused from `docs/UNIT_RUDDER_CURVE.md` | complete as reconstructed there |
| `009D8CE0` | read only far enough to establish that it reads two units' published slots | partial: `009D8DD3..009D9141` unread |
| `00815F30`, `00811D80`, `009D9E50`, `009E3C00` | not read; named here only as slot or path callees | none |
| `00826C61..00826CDB` | read from the listing; not reconstructed here | none |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_AUTOPILOT_PAIR.md`: `00811D10` is "a sixteen-dword field-by-field copy, so the pair is double-buffered: the AI always writes the slot the unit is currently on, and the motion update flips to the other one and carries the values across so the order keeps standing" | The copy is real but bounded at `+00h..+3Fh`, so it carries none of `+40h`, `+44h`, `+48h` or `+4Ch`. The flip moves the unit onto a slot whose order triple is two frames stale; the freshly published triple is the one at `unit + 0A98h + 54h * index`, which is exactly the address its readers compute | `00811D14..00811D71` then `RET 4` at `00811D74`; `009D8D35 FLD [EDX+EAX+0ADCh]` reads `0A98h+44h` |
| `docs/UNIT_AUTOPILOT_PAIR.md`: "No reader of `slot+40h`, `+44h` or `+48h` was found ... whatever converts a heading target and two distances into the order ring's `+148h`/`+14Ch` is elsewhere" | Readers exist and none of them is on the ring path. The grep that missed them searched `00825F20`'s decompiled body for unit-relative constants, but that function's `param_1` is `unit+310h`, so `unit+0B2Ch` appears there as `param_1+81Ch`. The `IMUL 54h` scan above does not depend on a base | the twelve sites in the scan table; `00825F32 LEA ESI,[EDI-310h]` |
| `docs/UNIT_AUTOPILOT_PAIR.md`: `slot+44h` is "limited to a quarter turn about the unit's own heading" | The bound is `+/- pi/4` either side, so the window is a quarter turn wide and the limit in one direction is an eighth | `00D09448` = `-0.785398185253`, `00CEDCD0` = `+0.785398185253` |
| `src/ship_motion.cpp`: `ship_motion_step_00825f20` began at `00826121`, the ring tick | It begins at `00825F2C`, the order-slot promotion, which is the first thing `00825F20` does | `00825F2C..00825F7C` before `00826121` |
| `src/ship_motion_probe.cpp`: "STAND-IN, not recovered: the bearing to the goal ... which this packet did not project" | The bearing is now the reconstructed navigation arm, `ship_ai_navigation_arm_009ee671` | `docs/SHIP_AI_NAVIGATION_ARM.md` |

## no_ghidra_function

| start | inclusive end | evidence for each boundary |
| --- | --- | --- |
| `00815EE0` | `00815F28` | start: `int3` padding `00815ED6..00815EDF` and a `MOV EAX,[ECX+0B40h]` opening a `__thiscall`. End: `RET` at `00815F28`, then `int3` `00815F29..00815F2F`, then `FUN_00815F30`. Unreferenced |
| `009D4F70` | `009D4F80` | start: `int3` padding `009D4F61..009D4F6F`. End: `RET` at `009D4F80` after `LEA EAX,[EAX+ECX+0A98h]`, then `int3`. Unreferenced |
| `007B8840` | `007B885A` | start: `int3` padding `007B883D..007B883F` and `MOV AL,[ESP+4]`. End: `RET 4` at `007B885A`, then `int3` `007B885D..007B885F`, then `FUN_007B8860`. Unreferenced; see `docs/UNIT_MANUAL_AUTOPILOT.md` |

## Uncertainties

* The meaning of the two `1Ch`-byte sub-records is unread. `00815F30` writes them and
  `009EE66C` is its only call site on this record; its body was not read here.
* `009D8CE0` is read as an inter-unit predictor because it takes two units and reads the
  published triple of both. Its own caller is unknown - Ghidra records none and no reference to
  it was scanned for - so "avoidance" or "formation" would be an invention and is not claimed.
* Whether the stale triple the unit lands on after a flip is ever read is not established. No
  reader of the `current` form of `+40h`/`+44h`/`+48h` exists in the scan, so on this evidence it
  is never read; that is an absence of evidence over one addressing idiom, not a proof.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ship_ai_order_consumer` | `009D8CE0`, `009D8D2C`, `009E3C00`, `009E3DB0`, `00811D80`, `00815F30` | Read the three consumers of the published triple and settle what a ship does with another ship's published heading and distances. `009D8CE0`'s caller has to be found first; it has none in the call graph |
| `ship_ai_throttle_to_ring` | `009F43D6`, `009F443B`, `009F4466`, `009F4486`, `009F449D`, `009F46D6`, `009F4711`, `009F4810`, `009E7B53`, `009EA839`, `009F4DA0` | Where `blk+1D0h` and `blk+1D4h` go for a unit the player is not steering. Nothing on the per-frame chain `009F51C6..009F5227` consumes them, and the ring is a player and network path, so either step 16 `009F4DA0` or one of the `009F42xx..009F48xx` readers of `+1D0h`/`+1D4h` closes the loop. This is the packet's biggest open question |
| `unit_heading_vtable_0050` | `00826CDB`, `0081196C`, `009ED95D`, `009EE8C7` | Slot `50h` of the vtable reached through the object at each site. Three sites call it with no argument and take a float back; `00826CDB` passes a yaw rate. Find the installed function for each class and settle whether the four sites share one vtable. Until then every reconstruction that models it as a heading getter is provisional |

## Correction appended by packet `cc8_ship_ai_rudder_hop` (2026-09-18)

`docs/SHIP_AI_RUDDER_HOP.md` has since read `00826C34..00826D69` instruction by
instruction. Two claims in the section "What `00825F7C..00826D6B` actually contains on the
steering path" and in the `unit_heading_vtable_0050` follow-up row do not survive it.

* **The two call sites of vtable slot `50h` do not disagree.** All four sites are
  `__thiscall float(void)`, `RET 0`, `ST0` result, and the concrete target is `006DFD60`,
  `FLD dword [ECX+1050h]; RET`, installed at `00CFC420` = unit vtable `00CFC3D0` slot
  `50h`. The float this document saw at `00826CD5` is the **third argument of
  `00810190`**, which is `RET 0Ch` at `0081062C` and takes three stack arguments while only
  two are pushed after the virtual call. With `RET 4` at `00826CDB` the function's own
  epilogue at `00826D5F..00826D69` no longer balances.
* **`00826C61..00826CDB` is not a hop from the rudder to the heading.** The ordered rudder
  reaches the body at `00826B54` through `0092E8C0`, which `docs/SHIP_MOTION.md`
  reconstructed in 2026-09; `00826C75`'s yaw rate is only `00810190`'s fourth argument,
  stored in a wake-trail record at `+1Ch`; and `unit+1050h` - the field slot `50h` returns
  - is **written** at `00826C56` from `atan2(worldRow2.x, worldRow2.z)`, so the hull
  heading is an output of the motion tick rather than a steering input.

The rest of this document stands, including its central negative: the AI never writes the
order ring. The gap it identifies is real and still open; it is just not at `00825F7C`.
`ship_ai_heading_to_rudder` in the other document's follow-up table is the packet for it.

## Second correction appended by packet `cc8_ship_ai_heading_to_rudder` (2026-09-18)

This document's central negative does not hold. It says: "Scanning `.text` for every `E8`
`rel32` reaching `00816A40` or `0080DAD0` finds exactly those four sites and no other, so on
the evidence in the image the ring is a player-order and network path and **the AI never
writes the order ring**."

The scan is correct and the conclusion does not follow, because the AI uses neither entry
point. `009F3F80 BSP_ShipAi_DriveOrderRing` ends with

```
009F4CDE  PUSH ECX ; MOV ECX,[ESI+3FCh] ; FSTP [ESP] ; 009F4CE8 CALL 0080E190
009F4CED  FLD [ESP+20h] ; PUSH ECX ; MOV ECX,[ESI+3FCh] ; FSTP [ESP] ; 009F4CFB CALL 0080E170
```

and both callees are five instructions that write the ring's write slot straight from the
unit: `MOV EAX,[ECX+97Ch]; MOVSS XMM0,[ESP+4]; SHL EAX,5; MOVSS [EAX+ECX+838h or +83Ch],XMM0;
RET 4`. `unit+838h` is the ring base and `unit+97Ch` is `ring+144h`, the write cursor.
`00813020` then clamps `slot[ring+140h]`, steps `ring+148h`/`+14Ch` toward it through two
`0042AC60` calls whose `ECX` comes from `008130D1`/`00813104`, copies the write slot forward
with a `REP MOVSD` at `0081317C`, and at `00813197` sets the read cursor to the write cursor.
In a single-player session the live pair therefore follows what the AI wrote one tick
earlier; the client branch at `008131B0` walks the read cursor instead, which is the network
playback regime.

Two more rows in this document change with it:

* `ship_ai_throttle_to_ring`, called here "the packet's biggest open question", was not
  open when this document was written. `docs/SHIP_AI_THROTTLE_TO_RING.md`, packet
  `cc_ai_throttle_ring`, answered it on 2026-09-12: `blk+1D0h` and `blk+1D4h` go to
  `slot[write]+0` and `slot[write]+4` after the deadbands and the slew at
  `009F4BA7..009F4C12`. `src/ship_ai_throttle_ring.cpp` reconstructs `009F4B99..009F4D04`
  and both setters, and the host binds them.
* "`009D8CE0`'s caller is unknown - Ghidra records none" - it records one, `009F3E30`,
  which itself has no caller in the graph.

The twelve readers of `slot+40h`/`+44h`/`+48h` stand, and so does the conclusion that none
of them steers: `009F40BB` takes the heading target from `blk+324h`, not from `slot+44h`.
The published triple is inter-unit state, and this document's own reading of it is right.
