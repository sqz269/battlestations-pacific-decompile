# The ship motion tail, and the vtable slot `50h` prototype (packet `cc8_ship_ai_rudder_hop`)

Addresses: 00826C34..00826D69 (inside 00825F20), 00811960, 00811890, 00810190, 006DFD60,
00778890, 0077A650; read as evidence 00CFC3D0, 00811940, 00937440, 009F4060, 0092E5B0,
0070DB60, 00810630, 00BF701A.

Worker `agent/cc8-ship-rudder-hop`, 2026-09-18. Ghidra was read-only for this packet:
no renames, no comments, no prototypes, no saves. Every expression below comes from the
disassembly listing, because the whole tail is x87 and the decompiler loses the stack
aliasing that the packet turns on.

## The headline: the packet's premise was wrong, and the block it names is not a hop

Three documents say that `00826C61..00826CDB` carries "the hop from the ring's ordered
rudder to the unit's heading", and the open question they all name is that the two call
sites of unit vtable slot `50h` disagree on the prototype. Both claims fall to the same
reading.

1. **Slot `50h` takes no argument at either site.** `00810190` is `RET 0Ch`, three stack
   arguments, and only two of them are pushed after the virtual call. The third is the
   slot the `PUSH ECX` at `00826CD5` reserved, staged before the call the way MSVC stages
   any argument whose value is ready early. The virtual call never sees it.
2. **The ordered rudder had already been applied forty instructions earlier.** It reaches
   the body at `00826B54` through `0092E8C0(controller, unit+984h, dt)`, which
   `docs/SHIP_MOTION.md` reconstructed on 2026-09-11 and which `bsp/ship_motion.hpp`
   already binds as `ship_apply_steering_0092e8c0`.
3. **`unit+1050h` is written here, not read as a command.** `00826C56` stores
   `atan2(worldRow2.x, worldRow2.z)` into it, and `006DFD60` - unit vtable slot `50h` - is
   seven bytes that return exactly that field. The hull heading is an **output** of the
   motion tick, produced out of the pose the physics step left behind.

So `00826C75`'s yaw rate is not steering. It is the fourth argument of the wake-trail
sampler, where it is stored beside the position and the heading as trail metadata.

## The slot `50h` verdict

**One ABI, one vtable, one installed function. There was never a disagreement.**

```
__thiscall float BSP_UnitInstance_GetHullHeading(void);   // RET 0, ST0 result
006DFD60  D9 81 50 10 00 00   FLD dword ptr [ECX+1050h]
006DFD66  C3                  RET
```

It is unit vtable `00CFC3D0` slot `50h`: `00CFC420` holds `60 fd 6d 00`. Two independent
documents already place the unit at that table (`docs/CRUISE_COMMAND.md` for `+50h`,
`docs/SHIP_AI_HULL_GEOMETRY.md` for `+20h`), and `docs/SHIP_AI_NEIGHBOUR_BOX.md` had
already read `006DFD60` itself.

| site | ECX | pushed before the call | cleanup proof | ABI |
| --- | --- | --- | --- | --- |
| `00826CDB` | `ESI` = the unit, vtable `[ESI]` at `00826CCC` | nothing of its own | see below | `RET 0` |
| `0081196C` | `[ESI+50h]`, vtable `[[ESI+50h]]` at `00811967` | nothing | `0081196E FSTP [ESP+4]` must land on the `PUSH ECX` local, which needs ESP unchanged across the call; `00811A20 POP ESI; POP ECX; RET 4` balances only then | `RET 0` |
| `009F407B` | `[ESI+3FCh]` | nothing | `009F407D FSTP [ESP+1Ch]` | `RET 0` |
| `009EAE46` | `[[node+14h]]` | nothing | `docs/SHIP_AI_NEIGHBOUR_BOX.md` | `RET 0` |

The object at `00811960`'s `+50h` **is** a unit, so it is the same vtable and the same
function: `00811975 MOV ECX,[EAX+1018h]; 0081197B CALL 0092D730` reads the unit controller
at `unit+1018h`, the same field `src/game_hosts_ship_ai.cpp` already cites at `009F4034`.

### Why `00826CDB` looked like it took a float

The cleanup argument, from the frame base `B` = ESP after the prologue (four pushes and
`SUB ESP,0B0h`; the epilogue at `00826D5F..00826D69` pops exactly that back):

```
00826CD5  PUSH ECX            ; ESP = B-4, a reserved argument slot
00826CD8  FSTP dword [ESP]    ; B-4 := yawRate
00826CDB  CALL EAX            ; slot 50h.  RET 0 -> ESP = B-4
00826CDD  PUSH ECX            ; ESP = B-8
00826CE4  FSTP dword [ESP]    ; B-8 := the heading ST0 the call returned
00826CE7  PUSH ECX            ; ESP = B-12, holds LEA ECX,[ESI+0FCh]
00826CEE  CALL 00810190       ; RET 0Ch -> ESP = B          <- balances
```

With `RET 4` at `00826CDB` the same trace ends at `B+4` and the function's own epilogue
no longer balances. Nothing between `00826CEE` and `00826D5F` adjusts ESP: `00826CF5`,
`00826D21` and `00826D3E` are all no-argument calls and `00826D5A` is one too. `RET 0Ch`
at `0081062C` is read directly off `00810190`'s epilogue, and `00810190` confirms the
three roles from the other end: `00810194 MOV EBX,[ESP+44h]` is the vec3 pointer,
`00810602` and `00810617` load `[ESP+50h]` and `[ESP+54h]` into the new record's `+14h`
and `+1Ch`.

The `PUSH ECX; FSTP [ESP]` pair is this function's idiom for one outgoing float, used at
`00826C6B` for `00811890` and at `00826C25`/`00826CB2` for pointers. Reading it as the
virtual call's argument is what produced the phantom disagreement.

## What `00826C34..00826D69` actually is

`docs/SHIP_MOTION.md` reconstructed `00826121..00826B84` and left "the tail after
`00826B84`" out by name. This is that tail, and it runs after the controller step.
Registers as that document establishes them: `ESI` = unit, `EDI` = unit+310h,
`EBX` = `EBP` = unit+0CCh (the world 4x4, rows at `+00h`/`+10h`/`+20h`/`+30h`),
`[ESP+0C4h]` = the scaled delta.

| address | what happens |
| --- | --- |
| `00826B86`, `00826BE0`, `00826C7A` | three inlined copies of the world-matrix accessor: if `unit+0C8h` is clear, rebuild `unit+0CCh` from `unit+74h` - combined with `[unit+3Ch]+0CCh` through `00413920` when the unit has a parent - by `004134F0`, then set `unit+0C8h` and clear `unit+10Ch`. `docs/ENTITY_LOCAL_MATRIX.md` owns the accessor |
| `00826C3B..00826C46` | `heading = atan2(unit+0ECh, unit+0F4h)` = `atan2(worldRow2.x, worldRow2.z)`. `00BF701A` is the x87 CRT `atan2`, numerator in ST1, and `00826C3B` pushes the x first |
| `00826C56` | `unit+1050h = heading`, through `FSTP [EDI+0D40h]` |
| `00826C5C` | `0092E5B0(unit+1018h, 0)`, a controller method. Not read: `0092E5B0..0092E8A0`, gated on `controller+60h`, reading the physics body at `controller+2Ch` through `00C31F40` |
| `00826C61..00826C81` | `yawRate = 00811890(unit, unit+984h)`, the ordered rudder through the rudder curve. `00811940 BSP_UnitInstance_GetCurrentCommandYawRate` is the same two instructions packaged as a getter |
| `00826CCC..00826CDB` | `heading2 = unit->vtable[50h]()`, which reads back the `unit+1050h` written at `00826C56` |
| `00826CDD..00826CEE` | `00810190(unit+0BD0h, &unit+0FCh, heading2, yawRate)`, the wake-trail sample |
| `00826CF3..00826CFC` | `if (00778890(unit))`, i.e. the unit has an occupant record at `unit+284h` whose `+14h` points back at it |
| `00826CFE..00826D3E` | `t = unit+1158h - dt; unit+1158h = t; if (t < 0) { unit+1158h = t + 00424C40()->[430h]; 0070DB60([unit+284h]); }` - a repeating timer on the occupant record. `EDI-8Ch` is `unit+284h`, the same field `00778890` tests |
| `00826D43..00826D5A` | `if (unit+308h != [00D7A218]) 0077A650(unit)`. `UCOMISS; LAHF; TEST AH,44h; JNP` is MSVC's float `!=`, and an unordered compare also takes the call |
| `00826D5F..00826D69` | the epilogue, `RET 4` |

### `00810190`, the wake-trail sampler

`__thiscall(vec3* worldPos, float heading, float yawRate)`, `RET 0Ch` at `0081062C`. Its
only caller in the image is `00825F20`. It appends to a forty-entry ring of `18h`-byte
records at `this+8`: write cursor at `this+3C8h`, a drift offset at `this+3D0h..+3D8h` and
a flag at `this+3CCh`. Record fields are position at `+0/+4/+8`, the heading at `+14h`,
the cumulative segment length at `+18h` and the yaw rate at `+1Ch`. A minimum
squared-distance gate returns early at the head, and the previous record's `+14h` is
recomputed from the segment bearing when a new sample lands, so the heading argument is
provisional metadata. `00810630 BSP_UnitWake_SampleAtDistance` reads the same cursor
`[ECX+3C8h]` at `00810645`, which is where the wake family name comes from.

The packet read `00810190` far enough to establish the argument roles, the `RET 0Ch` and
the record layout. **It did not reconstruct it**, and the drift/reseat logic in its middle
is unread.

### Where the ordered rudder does steer the ship

A disp32 census over `.text` for the four bytes `84 09 00 00` returns 25 sites, with
`00826C63` as the positive control. Every unit-relative one is a **read**; no literal
displacement writes `unit+984h`, which agrees with `docs/UNIT_COMMAND_PRODUCERS.md` and
with `docs/MOTION_DIFFERENTIAL.md` putting the two writers at `0080DA3A` and `0082674C`,
both through the ring base rather than through `unit`. The readers that matter:

| site | enclosing routine | what it does with it |
| --- | --- | --- |
| `00937572` | `00937440 BSP_UnitController_ApplyShipForces` | `MOV EDX,[EDI+1Ch]; FLD [EDX+984h]`, then multiplied into the applied force with `settings+588h`. This is the steering |
| `00826B54` | `00825F20` | `0092E8C0(controller, unit+984h, dt)`, the steering call `docs/SHIP_MOTION.md` reconstructed |
| `00826C63` | `00825F20` | the wake metadata above |
| `00811943` | `00811940` | the packaged getter |
| `009E2A6A` | `009E26C0 BSP_ShipAi_AttackMoveLeadPursuitSubStateStep` | the AI reads the unit's current rudder |
| `0064B9AE` | `0064B870 BSP_HudUnitOrder_UpdateIntegratedControls` | copies it into the HUD record at `+28h` |

So the AI's published heading still has no established path to `unit+984h`. That gap is
real, but it is not at `00825F7C` and this packet does not close it.

## Reconstruction

`include/bsp/ship_ai_rudder_hop.hpp` and `src/ship_ai_rudder_hop.cpp`:

* `ship_hull_heading_00826c3b(forward_x, forward_z)` - the pure rule, `atan2` with the
  argument order taken from the two `FLD`s rather than from the decompiler.
* `ship_motion_tail_00826c34(inputs, host)` - the whole range as a sequence, with
  `ShipMotionTailHost` naming one method per native call site in call order:
  `controller_0092e5b0`, `yaw_rate_from_rudder_00811890`, `refresh_world_matrix`,
  `unit_heading_vtable50`, `append_wake_sample_00810190`,
  `occupant_owns_unit_00778890`, `occupant_timer_refill_00424c40_430`,
  `occupant_tick_0070db60`, `field_308_crossed_0077a650`.

`ShipMotionTailStep` reports `hull_heading` and `heading_read_back` separately, so a
binder detects a unit whose slot `50h` is not `006DFD60`.

Coverage: **complete** for `00826C34..00826D69`. Every instruction is either computed in
the reconstruction or is one of those nine host calls. Not reconstructed:
`0092E5B0`, `0070DB60` and `00810190`'s body, and the three inlined matrix copies, which
sit below `00826C34`.

## Host methods

`ShipMotionTailHost` belongs with `ShipMotionHost` in `src/game_hosts_units.cpp`, which
was leased to the torpedo attack-mode worker for the whole of this packet. **The binding
is a contract, not a commit**, and nothing in this packet changes run-time behaviour.

The contract, for whoever lands it:

1. `src/game_hosts_units.cpp`, in the `ShipMotion::` binding that already answers
   `ShipMotionHost`: add a `ShipMotionTailHost` implementation beside it. Seven of its
   nine methods have an answer in the same file already - the controller handle for
   `0092E5B0`, `bsp::unit_yaw_rate_00811890` through the existing `UnitRudderHost`, the
   pose it already keeps for `refresh_world_matrix` and `unit_heading_vtable50`, and the
   `00424C40` settings block for the refill.
2. `unit_heading_vtable50` must return `unit+1050h` **as this tail just wrote it**, not a
   separately tracked heading, or `heading_read_back` stops meaning anything.
3. `append_wake_sample_00810190` has no reconstruction to call: record it and drop the
   sample, or bind it to a follow-up packet's `00810190`.
4. Call it at the end of `ship_motion_step_00825f20`'s binding, after `unit_post_motion`,
   and fold `ShipMotionTailStep` into `ShipMotionStepResult`.
5. Census rows to add: one per host method, plus `ShipMotion::tail` at `0x00826c34`.

## Corrections

Appended, not rewritten, to the documents named. The in-place edit is only the stale
comment and log note at the `ShipAiOrder::slot_to_order_ring` record in
`src/game_hosts_ship_ai.cpp`, where the superseded claim is quoted in the new text.

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_AI_ORDER_SLOT_READER.md` and `docs/SHIP_AI_RING_WINNER.md`: `00826C61..00826CDB` "carries the hop from the ring's ordered rudder to the unit's heading" | It carries the rudder to the **wake trail**. The steering application is `00826B54`, already reconstructed, and `unit+1050h` is written here from the pose | the `RET 0Ch` balance above; `00826C56`; `docs/SHIP_MOTION.md`'s `00826B54` row |
| the same two documents: "the two call sites of vtable slot `50h` disagree on the prototype ... `00826CD5` sets up one outgoing float" | The float is `00810190`'s third argument. All four sites are `RET 0`, no arguments, `ST0` result, concrete target `006DFD60` | `0081062C RET 0Ch`; `00CFC420` = `006DFD60`; the epilogue balance |
| `docs/UNIT_AI_ORDER_SLOT_READER.md`: `00826CDE` "reloads ST0 as the result" | Correct, but the result is the **heading**, and it goes to `00810190`'s second argument, not back into the steering | `00826CE4 FSTP [ESP]` then `00826CE7 PUSH ECX` |
| the census record `ShipAiOrder::slot_to_order_ring` at `00825F7C` | Still a misnomer, and now for a second reason. The name is deliberately **not** changed, so the counts stay comparable with the six packets that came before; the correction lives in the comment and the log note | `src/game_hosts_ship_ai.cpp` |

`docs/SHIP_MOTION.md`'s own coverage statement is unchanged and correct: it always said
the tail was out of scope.

## no_ghidra_function

none. Every routine read here has a Ghidra function: `00825F20`, `00811960`, `00811890`,
`00810190`, `00810630`, `006DFD60`, `00778890`, `0077A650`, `0092E5B0`, `0070DB60`,
`00811940`, `00937440`. The range `00826C34..00826D69` lies inside `00825F20`, whose
stored body Ghidra truncates - `bsp.py ghidra disasm 00825f20 --start 00826c40` refuses -
so the listing came from `bsp.py disasm-raw` over the disk bytes, resynchronised from
`00826B70` and cross-checked against the `00826D69 RET 4` and the `00826D6C..00826D6F`
`INT3` padding before `FUN_00826D70`.

## Validation

* `./scripts/build.ps1`, Win32 `/W4 /WX`: clean.
* `ctest --test-dir build/win32 -C Release`: all existing cases pass. No test was added:
  the reconstruction has no caller yet, so there is no behavioural regression to guard.
* USN02, `--frames 3200 --press-start-frame 30 --mission-frames 3000
  --mission-frame-seconds 0.05`:
  `hull=125 deaths=3 total_damage=13673.7 attributions=126 first_hit=41.85 s`, and
  `ship ai ring hops=96000 writes=96000 rudder_law=93000 driven=0`. **Identical to the
  standing census at main `9ac3fcbb7`**, which is the correct result: nothing is bound,
  so nothing can move. The attribution is that this packet changed no run-time path.
* USN01, same settings, for the record.
* `ShipAiOrder::slot_to_order_ring [00825f7c] UNIMPLEMENTED` is still in the log and stays
  there until the contract above lands.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ship_motion_tail_binding` | `00826C34`, `00810190` | Land the contract above once `src/game_hosts_units.cpp` is free. One commit, no new analysis |
| `unit_wake_trail_00810190` | `00810190`, `00810160`, `00810630` | The wake ring proper: the drift offset at `+3D0h`, the reseat branch under the `+3CCh` flag, and what reads the forty records |
| `ship_ai_heading_to_rudder` | `009F4D10`, `00811960`, `00937572`, `0092E8C0` | The gap this packet proves is still open: the AI publishes a limited heading target into `cmd+44h` at `00811A19` and nothing established turns it into `unit+984h`. Start from the twelve slot readers in `docs/UNIT_AI_ORDER_SLOT_READER.md`, not from `00825F7C` |
| `unit_controller_0092e5b0` | `0092E5B0` | The one call in this tail whose body is unread. Gated on `controller+60h`, reads the physics body through `00C31F40` |
| `unit_occupant_record_284` | `00778890`, `0070DB60`, `0077A650`, `unit+308h` | What `unit+284h` is, what the `settings+430h` timer drives, and what `unit+308h` is compared against |
