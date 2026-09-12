# The plane on the ground and on the water (packet `cc2_plane_ground_ops`)

Addresses: 007CCB30 007C1430 007CBFA0 007DCCF0 007CBA50 007DCDD0 007C6340 007C7110 007DA380
007DB680 006049F0 00762A00 007CEC30

Ghidra was read-only for this packet. Every name is a hypothesis, **except the eight state
labels**: those are the shipped executable's own debug strings, reached through the jump table at
`007CCBC0`. The reconstruction is `include/bsp/plane_ground_ops.hpp` and
`src/plane_ground_ops.cpp`; the evidence rows are `reports/plane_ground_ops.json`.

## 1. The `unit+900h` state machine

`docs/PLANE_FLIGHT.md` left "what `unit+900h`'s six values mean" open and
`docs/PILOT_CONTROLS.md` established that there are eight values but said "the labels are not
established". The labels are in the binary. `007CCB30` (Ghidra body `007CCB30`-`007CCBBF`,
`__fastcall(outString, int state)`, `RET 0`) formats the field for the debug dump:

```
007ccb31: cmp edx, 7
007ccb3f: ja  0x7ccbb1                  ; the out-of-range arm, "huh?"
007ccb41: jmp dword ptr [edx*4 + 0x7ccbc0]
```

The eight-entry table at `007CCBC0` gives the enum outright. Value `3` is the only in-range value
whose entry points at the out-of-range arm, so the shipped build has **no label for state 3**; its
name below is this packet's hypothesis from the setter's side effects.

| value | table entry | label | string | what it is |
| --- | --- | --- | --- | --- |
| `0` | `007CCB48` | `N/A plane state` | `00D05E74` | the constructor's value; no motion arm |
| `1` | `007CCB57` | `Inside` | `00D05E6C` | stowed inside the hangar or the airbase |
| `2` | `007CCB66` | `Locked` | `00D05E64` | held on a launch spot; the setter cuts the throttle |
| `3` | `007CCBB1` | none, prints `huh?` | `00D05E34` | **launching** (provisional): the setter forces full throttle |
| `4` | `007CCB75` | `Runway` | `00D05E5C` | free ground roll on a runway or a deck |
| `5` | `007CCB84` | `Runway on Path` | `00D05E4C` | ground roll driven along a path (taxi) |
| `6` | `007CCB93` | `Water` | `00D05E44` | floating or ditched |
| `7` | `007CCBA2` | `Flying` | `00D05E3C` | airborne |

### The setter and the four bookkeeping fields

`007C1430 BSP_Plane_SetFlightState` (Ghidra body `007C1430`-`007C156C`,
`__thiscall(unit, int state)`, `RET 4`) is the guarded writer. Its jump table at `007C1550` is
indexed by `state - 3` over five entries (`007C1462 add eax,-3`, `007C1467 cmp eax,4`,
`007C1472 ja`), so `0`, `1`, `2` and anything above `7` share the tail at `007C1542`.

| arm | site | effect |
| --- | --- | --- |
| pre-check | `007C1434`-`007C143C` | a requested state of `2` zeroes the throttle `unit+9F0h` **before** the equality test, so even a redundant request cuts it |
| no-op | `007C144C` | returns `0` and stores nothing when the state already matches |
| every change | `007C145B`, `007C146A` | store, then `unit+C04h = -1.0f` (`00D7A260`) |
| `3` | `007C1532` | `unit+9F0h = 1.0f` (`00D7A24C`), full throttle |
| `4`, `5` | `007C147F` | only when `classDesc+198h MinWaterSpd != 0`: `unit+904h = (unit+908h > 5.0f)` (`00CE3850`) and `unit+C18h = 3`, then return |
| `6` | `007C150A` | `unit+910h = 0`, clear `unit+904h`, `unit+90Ch = 0` |
| `7` | `007C14D0` | `unit+910h = 0`, clear `unit+904h`, `unit+908h = 0` when the previous state was `4` or `3`, else `3600.0f` (`00CFDEB0`) |

That fixes the meaning of the two fields the earlier docs carried without one:

* `unit+908h` is the **airborne clock in seconds**. The free-flight arm accumulates it
  (`007CEC4E`, `[ESI+5F8h] += step` with `ESI = unit+310h`, when `unit+9E0h` is clear); entering
  `Flying` from `Runway` or `Launching` resets it to zero, and entering `Flying` from anything else
  seeds it with one hour so a plane placed in the air counts as having flown.
* `unit+904h` is **"this ground contact followed a real flight"**, i.e. *landed* as opposed to
  *parked*. It is derived on entering `Runway` / `Runway on Path` as `clock > 5 s` and cleared on
  entering `Water` and `Flying`. `007CBFA0` and `007DA380` both branch on it.

### Every writer

`docs/PILOT_CONTROLS.md` has the table for the five named routines; this packet adds the guards and
the routines it had listed by address only. Nine sites store `unit+900h` without going through the
setter, six of them inlined copies of the setter's tail (the giveaway is the adjacent
`MOVSS [ESI+C04h], XMM0`).

| site | containing function | value | guard |
| --- | --- | --- | --- |
| `007D0060` | `007CFD20 BSP_PlaneUnitInstance_Construct` | `0` | none; `EBX` is zeroed at `007CFD56` and nothing writes it again before the store (the whole listing was filtered for `EBX`) |
| `007CC857` | `007CC820 BSP_Plane_EnterFlightStateOne` | `1` | differs |
| `007CC7E2` | `007CC7A0 BSP_Plane_EnterFlightStateTwo` | `2` | differs |
| `007C1697` | `007C1680 BSP_Plane_FlightStateFiveToFour` | `4` | from `5` only (`007C1680 cmp ...,5`) |
| `007C7488` | `007C7430` | `4` | from `5` only (`007C747F cmp ...,5`) |
| `007C171E` | `007C16F0 BSP_Plane_FlightStateFourToFive` | `5` | from `4` only |
| `007CBA12` | `007CB9E0` | `6` | not already `6` (`007CB9FA`) |
| `007C63F4` | `007C6340` | `7` | the placement chooser, below |
| `007C6481` | `007C6340` | `6` | the placement chooser, below |
| `007C7183` | `007C7110` | `7` | not already `7`; the become-airborne routine, below |
| `007C36BC` | no Ghidra function | `EAX`, an argument | `007C36B9 cmp eax,7` bounds it; also clears `unit+904h` |
| `007C627C` | `007C5F60` | `EAX` | inlined setter tail (`007C6282` stamps `+C04h`) |
| `007C78F1` | `007C78A0` | `ECX` | inlined; `007C78EE`/`007C78F9` then branch on `1` and `2` |
| `007CB673`, `007CB6E2` | `007CB5F0` | `EDI` | two inlined copies, both stamping `+C04h` |
| `007D63EA`, `007D65DD`, `007D6600`, `007D711A` | no Ghidra function | `EAX` / `7` | `007D63E4` and `007D65D7` bound with `cmp eax,7` |

Three routines request a state through the setter rather than storing it: `007C1570`,
`007C6F50` and `007CBA50`. `007CA3F0 BSP_Plane_HandleTouchdownOrCrash` requests `4 + bool`.

### The transitions

| from | to | routine | trigger |
| --- | --- | --- | --- |
| (construction) | `0` | `007CFD20` | the plane unit is built |
| `0`/any | `1` | `007CC820` | plane message sub-kind `1` |
| any | `2` | `007CC7A0` | plane message sub-kind `2` |
| `3` | (held) | `007C6340` | placement while already launching: pose refresh only, no state write |
| (placement) | `7` or `6` | `007C6340` | the height probe, below |
| `5` | `4` | `007C1680`, `007C7430` | leave the path, roll free |
| `4` | `5` | `007C16F0` | join the path (taxi) |
| `4`/`5`/`6`/`3` | `7` | `007C7110` | the takeoff message (kind `C6h`); `007CBA50` for the water arm |
| `4`/`5` | `6` | `007CB9E0` | plane message sub-kind `6` |
| `6`/`7` | `4`/`5` | `007CA3F0(0/1)` | touchdown or crash |

## 2. A correction to the motion dispatch

`docs/PLANE_FLIGHT.md` records the water arm's gate as `unit+5F0h == 6` and
`include/bsp/plane_flight.hpp` carries a `surface_mode` input for it. There is no second enum.
In `007CEC30`-`007CECB4`, `EDI` is the unit and `ESI` is `unit+310h`: `LEA ECX,[ESI+364h]` feeds
`0085DC80`, which that doc itself writes as `unit+674h`, and `[ESI+41Ch]` is its own `unit+72Ch`.
Applying the same bias to `007CEC99 CMP dword ptr [ESI+5F0h],6` gives `unit+900h == 6`, which is
exactly the `Water` value of the table above. The arms are:

| arm | gate | call |
| --- | --- | --- |
| free flight | `(*(unit+72Ch))->vtable[+38h]()` at `007CEC3F` | `007CEC6E 007CC2F0(unit, step)` |
| ground roll | `unit+900h` is `4` or `5` (`007CEC7B`, `007CEC80`) | `007CEC92 007CBFA0(unit, step)` |
| water | `unit+900h == 6` (`007CEC99`) | `007CECAF 007CBA50(unit, step)` |

## 3. The ground-roll arm, `007CBFA0`

Ghidra body `007CBFA0`-`007CC2C5`, `__thiscall(unit, float step)`, `RET 4`. Its only caller is
`007CE040 BSP_PlaneTickElement_FixedStep`. As a rule table, in order:

| step | site | rule |
| --- | --- | --- |
| 1 | `007CBFC3` | `007C5AC0(step)`, the pre-pass the free-flight arm also runs |
| 2 | `007CBFC8`-`007CBFEC` | when `unit+904h` is clear (not a landing), on the object at `unit+DECh`: if `+45h != 1` then `+45h = 1` and `+4Ch = 1`; then if `+4Ch` is set, `+11h = 1` |
| 3 | `007CBFF0`-`007CC03D` | while `unit+5Dh` is set: `unit+C44h -= step`; when `unit+C0Ch` is **clear** the remainder is clamped **down** to `5.0f`, so losing steering authority shortens the fuse; at zero or below, and only in net modes `0` and `1`, raise `"explosion"` (`00D05A18`) through `vtable[+194h]` and **return** |
| 4 | `007CC0C9`-`007CC126` | `unit+C0Ch = (unit+904h == 0 && unit+900h != 5)`; then, unless the squadron `unit+9D4h` exists with `squadron+360h` set, AND it with `tuning+18Ch AirField/PlayerControlSpd` `KMH(80)` `< unit+B1Ch`. So a plane steers on the runway only when it did not just land, is not on a path, and is either squadron-overridden or faster than 80 km/h |
| 5 | `007CC13F` | `unit->vtable[+1ECh](step)`, the per-axis clamp `007CAF10` |
| 6 | `007CC14A`-`007CC15E` | `007DCCF0(unit+AB0h, step, SETZ(unit+900h == 2))`; `EBX` holds `2` from `007CC145` |
| 7 | `007CC163`-`007CC1AF` | when `unit+904h` is set: `(unit+BF4h)->+4h->+3Ch->vtable[+38h](unit)`, and `007B8DA0(unit)` if it answers true; otherwise the step-2 block again |
| 8 | `007CC1B3`-`007CC2B3` | the lift-off request, section 5 |

## 4. `007DCCF0`, the ground-roll law

Ghidra body `007DCCF0`-`007DCDCB`, `__thiscall(ctl, float step, int flag)`, `RET 8` (so two stack
arguments). The flag is tested as a byte at `007DCD20`, which is why the call site's `SETZ AL`
over a stale `EAX` is harmless. `coverage: complete`.

| rule | site |
| --- | --- |
| when `unit+BF8h` is clear **and** `unit+900h != 5`, run `007DC830(step)` (the free-flight step) and return `0` | `007DCCF9`, `007DCD01` |
| otherwise `ctl+FCh = 1`, `ctl+4h = 0`, `ctl+5h = 0` | `007DCD24`-`007DCD31` |
| `007DB680(ctl, step, flag ? 0.0f : 1.0f, 0, 0)` | `007DCD34`-`007DCD4x` |
| `ctl+80h..8Ch = { 0, cos(classDesc+200h GroundPitch), sin(GroundPitch), tuning+2A4h RunwaySmoothStrength }` | `007DCD9x`, `007DCDB9` |
| return `1` | `007DCDC6` |

So `Runway on Path` always uses the ground law, while `Runway` falls back to free flight the moment
ground contact is lost - which is how the plane keeps flying through the frames between leaving the
deck and the state reaching `Flying`.

`ctl+FCh` is the controller mode, and the three laws set it to three different values:
`0` in free flight (`007DC841`), `1` here, `2` in the water law (`007DCDD3`).

## 5. Takeoff, end to end

The launch side is `docs/AIR_OPERATIONS.md`: a slot or `MCatapult::Fire 006EC8E0` creates the
squadron and its planes, and `007F4580` is where the planes come into being
(`docs/PLANE_SQUADRON.md`). Where the plane's own state machine picks them up is
`007C6340` (Ghidra body `007C6340`-`007C64F5`, `__fastcall(unit)`, only caller `007F2920`):

| arm | site | rule |
| --- | --- | --- |
| already launching | `007C6347` | `unit+900h == 3`: clear `unit+C8h` and `unit+10Ch`, invalidate the node subtree poses, `vtable[+3Ch](0)`, `007BC550()`, return. **No state write.** |
| airborne placement | `007C636A`, `007C63F4` | when `classDesc+A8h - 0.5 <= unit+A8h` **or** `classDesc+198h MinWaterSpd != 0`: `unit+90Ch = 0`, `unit+900h = 7`, clear `unit+904h` and `unit+910h`, `unit+908h = 3600.0f`, `007C11E0(1)`, `vtable[+3Ch](classDesc+18Ch)` |
| water placement | `007C6481` | otherwise `unit+900h = 6` with `unit+904h`, `+910h`, `+90Ch` and `+908h` all zero, `007C11E0(1)` |

Both placement arms write `unit+900h` directly and notify with `007C11E0(1)` rather than the
setter's `(0)`, so the `-1.0f` stamp at `unit+C04h` is not made on a placement.

The ground run then raises the takeoff request. `007CBFA0`'s tail, `007CC1B3`-`007CC2B3`, has two
paths and both build the **same message, kind `C6h`**: `00762A00` stores `+4h = 3`,
vtable `00D02C68` and `+10h = 0xC6`, and `0077C2A0 BSP_Session_RouteMessage(unit, msg, 1, 0)`
sends it.

| condition | site | action |
| --- | --- | --- |
| `unit+900h == 5` | `007CC1BA` | nothing; a plane on a path never takes off by itself |
| net mode `!= 2` and `unit+BFCh - classDesc+1FCh > 0.1` and `unit+ACCh > 0.1` | `007CC1DE`-`007CC212` | send `C6h` and return |
| `unit+BF8h` still set | `007CC245` | nothing |
| `(unit+BF4h)->+4h->+7Ch ->vtable[+5Ch](9)` is true | `007CC258`-`007CC26E` | send `C6h`: ground contact was lost over a class-`9` object, i.e. the plane ran off a ship's deck |
| net mode `!= 2` | `007CC2AD` | `unit+C01h = 2` (`BL`, and `EBX` is `2` from `007CC145`) |

`006049F0` is the accessor that pairs the two fields: `if (unit+BF8h) return unit+BF4h; else 0`
(Ghidra body `006049F0`-`00604A02`). So `unit+BF8h` is the ground-contact flag and `unit+BF4h` the
object being stood on. Class `9` is the ship/carrier test `docs/AIR_OPERATIONS.md` uses at
`007F1BAD`; an airbase is `45h` and reaches takeoff only through the height test.

The message lands on `007C7110` (Ghidra body `007C7110`-`007C71DF`, `__fastcall(unit)`), the
become-airborne routine, which `docs/PILOT_CONTROLS.md` reached from `007CCFA0` sub-kind `7`:

1. `007C711A`: on a client (net mode `2`) it only calls `007C6F50(0)` when
   `(*(unit+72Ch))->vtable[+38h]()` is false, and never notifies the surface.
2. `007C7136`-`007C7154`: on the authority, when the state is `4` or `5` and `unit+BF4h` is set,
   `(unit+BF4h)->+4h->+3Ch ->vtable[+28h](unit)` - **the plane telling the runway or deck it has
   left**. This is the handoff back to the launch side; the air-operations slot itself is freed by
   `006C65B0 BSP_AirOps_ReleaseSquadronSlot` on the landing path, not here.
3. `007C7169`-`007C71D0`: the setter's state-`7` arm inlined, including
   `unit+908h = (previous == 4 || previous == 3) ? 0 : 3600.0f`.

## 6. Landing, end to end

The approach is flown by the pilot bots (`docs/BOT_TASKS.md`, the `land` task `009B3240` /
`009B3C60` with `Pilot/Landing/CruisingAlt` 1400) and is `contract: unread` here. What the plane's
own code does:

1. **Touchdown.** `007CA3F0 BSP_Plane_HandleTouchdownOrCrash` (`007CA3F0`-`007CA5E6`) requests
   `007C1430(4 + bool)` and resets the controls, per `docs/PILOT_CONTROLS.md`. Entering `4` or `5`
   sets `unit+904h` when the airborne clock passed five seconds, which is what marks the contact
   as a landing rather than a parked start.
2. **The arrestor wire.** Inside `007DB680 BSP_PlaneFlight_CoreLaw` (Ghidra body
   `007DB680`-`007DC82A`), `007DBEEE`-`007DC088`:

   | site | rule |
   | --- | --- |
   | `007DBEF6`-`007DBF0C` | `[esp+60h] = classDesc+1E0h WheelBrake (80) * max(two loads)` |
   | `007DBF10` | when the flag byte at `[esp+13h]` is clear, jump to `007DC085`: `ctl+ACh = 0` |
   | `007DBF22` | when `ctl+ACh <= 0.1` (`00D7A3A0`), leave it untouched and skip the block |
   | `007DBFE4`-`007DC015` | `ctl+ACh += Vector2f_LengthWithCutoff({v.x, v.z}) * [esp+5Ch] * tuning+518h Pilot/Landing/WireRope (7.25)` |
   | `007DC01C` | `ctl+ACh = MinFloatByRef(tuning+51Ch Pilot/Landing/MaxWireRope (100), ctl+ACh)` |
   | `007DC038`-`007DC081` | when `006049F0(unit)` is non-null, `ctl+ACh *= InterpolateClamped(160, 1.4, 250, 1.0, owner+B4h)`: **a shorter deck brakes harder**, 1.4x at 160 units falling to 1.0x at 250 |

   So the wire is not a separate mode. It is an accumulator on the flight controller that builds up
   with ground speed while the plane is braking and is capped at `MaxWireRope`.
3. **Wheel friction.** `007DC08C`-`007DC17A`, also in the core law:
   `BSP_Math_ClampFloatByRef(ctl+68h, tuning+29Ch WheelFrictionAccel/1 = 0.0,
   tuning+2A0h WheelFrictionAccel/2 = 8.0)`, and reads of `+290h WheelFriction` (0.75) at
   `007DC170`, `+294h WheelFrictionSpeed/1` (1.0) at `007DC17A` and `+298h WheelFrictionSpeed/2`
   (1.4) at `007DC15E`. `coverage: partial`, the surrounding arithmetic was not read.
4. **Taxi.** `007C16F0` moves `4` to `5` and `007C1680` / `007C7430` move `5` back to `4`; the
   `Runway on Path` state is the one the ground law always serves and the one that has no steering
   authority (`007CC0D3`) and never self-initiates a takeoff (`007CC1BA`).
5. **Steering at taxi speed.** `007DA380` (Ghidra body `007DA380`-`007DA707`) holds the runway
   band, `007DA540`-`007DA616`:

   | site | rule |
   | --- | --- |
   | `007DA563` | above `tuning+2ACh RunwayYawTurnSpdLimit` `KMH(25)` the band does not apply |
   | `007DA586`, `007DA590` | `rate = max(classDesc+1B0h YawSpd * tuning+2B0h RunwayYawTurnSpdMul (2.2), DEG(50) / YawSpd)`; `DEG(50)` is the `00D057E0` double `0.872665` |
   | `007DA5C3`-`007DA5DC` | halved (`00D7A280` = `0.5`) when `0047B850` answers true, i.e. `vtable[+5Ch](10h)` or `vtable[+5Ch](16h)`, **and** `unit+904h` is set: a plane that has just landed steers half as hard |
   | `007DA5E6`-`007DA611` | `InterpolateClamped(tuning+2A8h, speed, tuning+2ACh, 1.0f, ctl+6Ch)` blends it in |

   `+2A8h` and `+2ACh` hold the same installed value because the tuning parser pushes index `1`
   twice (`docs/GAME_TUNING_SINGLETON.md`, line 541).
6. **The slot.** Freeing the air-operations slot is `006C65B0 BSP_AirOps_ReleaseSquadronSlot`
   through `007F1B70`, already complete in `docs/AIR_OPERATIONS.md`: it clears the slot's `+28h`,
   sets `+8h = 0`, `+2Ch = 1`, `+30h = 5.0` and `+34h = 0`. Nothing in the plane's ground code
   touches it, so the slot release is an **external contract** for this packet.

## 7. The water arm

`007CBA50` (Ghidra body `007CBA50`-`007CBF93`, `__thiscall(unit, float step)`) is the arm the
dispatch takes when `unit+900h == 6`. In order:

| step | site | rule |
| --- | --- | --- |
| 1 | `007CBA6x` | `unit->vtable[+1ECh](step)`, the same control clamp the ground arm runs |
| 2 | `007CBA89`-`007CBB99` | the impact test: with `unit+5Dh` set, the net mode not `2` and `unit+911h` clear, if `unit+100h < 3.0f` and any of `speed() > classDesc+19Ch`, `unit+ACCh < -tuning+2B4h Water/MaxVSpd (10)`, `unit+C64h < -tuning+2B8h Water/MaxDownPitch (DEG 15)`, `unit+C64h > tuning+2BCh Water/MaxUpPitch (DEG 50)` or `abs(unit+C68h) > tuning+2C0h Water/MaxRoll (DEG 20)` holds, raise `"splash"` (`00D05A10`) and latch `unit+911h = 1` |
| 3 | `007CBBD3` | with `unit+5Dh` set and `007D7A80` false, `007D8180(0)` and `unit+C10h = 00D059A0` |
| 4 | `007CBC31` | `007DCDD0(unit+AB0h, step, unit+90Ch)`, the water law; it returns a depth in `ST0` |
| 5 | `007CBCBC`-`007CBD2B` | a loop over `classDesc+5BCh..+5C0h` (twelve-byte entries) transforming each point and sampling `0078CF20 BSP_GameWorld_SampleWaterHeight`; on the first point above the reference it breaks out, otherwise `(unit+72Ch)->vtable[+24h](0)`. `coverage: partial` |
| 6 | `007CBD74`-`007CBE0D` | `threshold = tuning+2D4h Water/MaxDepth (6.0) * InterpolateClamped(tuning+2C8h Water/NormalYawControlSpd KMH(45), 1.0f, tuning+24Ch LevelFlight (1.8) * classDesc+184h StallSpd, 00CE3800, speed())`; when the depth exceeds it, net modes `0` and `1` raise `"powerlost"` (`00D059F8`) and return |
| 7 | `007CBE7A`-`007CBED1` | `unit+90Ch += (depth + 0.5) * (1 - speed/classDesc+198h MinWaterSpd)`, only while there is depth and the plane is below `MinWaterSpd` |
| 8 | `007CBED9`-`007CBF80` | when the state is not `7`: on the authority build message kind `C3h` carrying the requested `7` and the current state and route it with `0075B430` / `0077C2A0` at `007CBF4F`; on a client (net mode `2`) call `007C1430(7)` at `007CBF7B`, and only from state `6`, `4` or `5` |

The law runs **before** the hull sampling, not after it: the call is at `007CBC31` and the loop's
first `00414DB0` is at `007CBCBC`.

`007DCDD0` (Ghidra body `007DCDD0`-`007DD9AC`) is the water law. It sets `ctl+FCh = 2` and
`ctl+4h`/`ctl+5h` to zero like the ground law, then, **only when `classDesc+198h MinWaterSpd is
zero**, samples the water height at three hull points (`0.0`, `00CE3918`, `00CE3CC8` along the
local X, each transformed by `BSP_Vector3f_TransformAffinePoint` off `unit+CCh`) through
`BSP_GameWorld_SampleWaterHeight`. `coverage: partial`: the mode stores and the hull sampling only;
the body past the sampling was not read.

`MinWaterSpd` is therefore the discriminator between a hull that floats and one that does not: it
gates the water law's floating arm (zero takes it), it forces the airborne placement in `007C6340`
when non-zero, it is the planing speed the submersion accumulator measures against, and it gates
the setter's `Runway` bookkeeping.

## 8. Installed tuning and class values

| key | offset | installed | read at |
| --- | --- | --- | --- |
| `AirField/PlayerControlSpd` | tuning `+18Ch` | `KMH(80)` | `007CC10D` |
| `Dynamics/SpdMultipliers/LevelFlight` | tuning `+24Ch` | `1.8` | `007CBD79` |
| `Dynamics/WheelFriction` | tuning `+290h` | `0.75` | `007DC170` |
| `Dynamics/WheelFrictionSpeed/1`, `/2` | tuning `+294h`, `+298h` | `1.0`, `1.4` | `007DC17A`, `007DC15E` |
| `Dynamics/WheelFrictionAccel/1`, `/2` | tuning `+29Ch`, `+2A0h` | `0.0`, `8.0` | `007DC095`, `007DC08D` |
| `Dynamics/RunwaySmoothStrength` | tuning `+2A4h` | `4.0` | `007DCDB9` |
| `Dynamics/RunwayYawTurnSpdLimit/1` | tuning `+2A8h`, `+2ACh` | `KMH(25)` both | `007DA60A`, `007DA565` / `007DA5F8` |
| `Dynamics/RunwayYawTurnSpdMul` | tuning `+2B0h` | `2.2` | `007DA592` |
| `Dynamics/Water/MaxVSpd` .. `MaxRoll` | tuning `+2B4h`-`+2C0h` | `10`, `DEG(15)`, `DEG(50)`, `DEG(20)` | the impact test |
| `Dynamics/Water/NormalYawControlSpd` | tuning `+2C8h` | `KMH(45)` | `007CBDB4` |
| `Dynamics/Water/MaxDepth` | tuning `+2D4h` | `6.0` | `007CBDC6` |
| `Pilot/Landing/WireRope` | tuning `+518h` | `7.25` | `007DC00D` |
| `Pilot/Landing/MaxWireRope` | tuning `+51Ch` | `100.0` | `007DC01C`, `007DB639` |
| `StallSpd` | class `+184h` | per class | `007CBD7F` |
| `MinWaterSpd` | class `+198h` | per class, 72 classes set it | `007C1485`, `007C63Dx`, `007CBEA1`, `007DCDEx` |
| `MaxWaterSpd` | class `+19Ch` | per class | the impact test |
| `YawSpd` | class `+1B0h` | per class | `007DA576` |
| `WheelBrake` | class `+1E0h` | `80` | `007DBEF6`, the only read in the image |
| (no key recorded) | class `+1FCh` | - | `007CC1CC`, the lift-off height reference |
| `GroundPitch` | class `+200h` | 51 classes set it | `007DCD9x` |

## Host table

| Site | Callee | Host method | this / args | Returns | Gate |
| --- | --- | --- | --- | --- | --- |
| `007CBFC3` | `007C5AC0` | `run_pre_pass_007c5ac0` | unit; step | void | always |
| `007CBFDE` | (stores) | `arm_ground_subsystem_007cbfd2` | `unit+DECh` | void | `unit+904h` clear |
| `007CC06C` | `0041E870` + `vtable[+194h]` | `raise_effect` | unit; name | void | fuse expired, net mode 0/1 |
| `007CC13F` | `007CAF10` | `clamp_controls_007caf10` | unit; step | void | always |
| `007CC15E` | `007DCCF0` | `run_ground_law_007dccf0` | `unit+AB0h`; step, `state == 2` | bool | always |
| `007CC186` | `(owner+4h)->+3Ch` `vtable[+38h]` | `surface_still_holds_007cc186` | the deck object; unit | bool | `unit+904h` set |
| `007CC18E` | `007B8DA0` | `run_surface_hold_007b8da0` | unit | void | the above answered true |
| `007CC212`, `007CC26E` | `00762A00` + `0077C2A0` | `send_takeoff_message_00762a00` | stack message; unit, 1, 0 | void | either lift-off path |
| `007CC2AD` | (store) | `stamp_pending_byte_007cc2ad` | unit | void | no contact, not a client |

## Coverage

`complete`: the `unit+900h` enum and its labels; `007CCB30`; the setter `007C1430` and its jump
table; the full writer inventory for `unit+900h` (every `disp32` store in `.text`); the motion
dispatch `007CEC30`-`007CECB4` with the `ESI` bias resolved; `007DCCF0`; `006049F0`; the
lift-off request `007CC1B3`-`007CC2B3`; `007C7110`; `007C6340`; the arrestor-wire band
`007DBEEE`-`007DC088`; the runway steering band `007DA540`-`007DA616`; the water arm's impact
test, drown test, accumulator and state request.

`partial`:
* `007CBFA0` (`007CBFA0`-`007CC2C5`): the eight steps above are read; the object at `unit+DECh`
  and the two message payloads are not.
* `007DB680` (`007DB680`-`007DC82A`): the wheel-brake, wire-rope and wheel-friction bands only;
  `007DB760`-`007DBEC8` and `007DC190`-`007DC82A` unread.
* `007DA380` (`007DA380`-`007DA707`): the runway band only; `007DA380`-`007DA540` and
  `007DA616`-`007DA707` unread, and the three out-parameters `docs/PLANE_FLIGHT.md` names are
  still unread as a whole.
* `007CBA50` (`007CBA50`-`007CBF93`): steps 1-8; the hull loop at step 4 and the
  `007D7A80` / `007D8180` block at step 3 unread.
* `007DCDD0` (`007DCDD0`-`007DD9AC`): the mode stores and the three-point hull sampling only.
* `007C6340` (`007C6340`-`007C64F5`): the three arms; the tail past `007C64B0` unread.

Nothing here is game-validated: `bsp_game.exe` has no path that reaches a plane on a deck, so no
run-time evidence is offered for any claim in this doc.

## Corrections

See `reports/plane_ground_ops.json` for the `was` / `is` / `evidence` triples. In summary:

* `docs/PLANE_FLIGHT.md` and `include/bsp/plane_flight.hpp` record the water arm's gate as a
  second enum at `unit+5F0h`. It is `unit+900h == 6`; the doc applied the `ESI = unit+310h` bias to
  the other two rows of the same table and not to this one.
* `docs/PLANE_FLIGHT.md`'s open question "what `unit+900h`'s six values mean" is answered by the
  game's own label table, and the count is eight, not six - `docs/PILOT_CONTROLS.md` already
  corrected the count.
* `docs/PLANE_FLIGHT.md` attributes `WheelBrake` `classDesc+1E0h` to "ground arm". It is read in
  `007DB680 BSP_PlaneFlight_CoreLaw` at `007DBEF6`, the shared core law, not in `007CBFA0` or
  `007DCCF0`; that single read is the only one in the image.

## Open questions

* What reads `unit+C01h`. Five sites write it (`007B9000` = 2, `007B9010` = 1, `007CB37E` = 2,
  `007CC2AD` = 2, `007D0116` = 0) and a full `disp32` scan of `.text` found no reader, so it is
  either read through a base with a small displacement or folded into a dword at `unit+C00h`.
* Whether state `3` is really the launch state. The setter forcing full throttle, `007C6340`'s
  dedicated no-write arm for it, and the state-`7` clock reset treating `3` exactly like `4` all
  point that way, but no routine was found that *requests* `3`, so the plane may never enter it in
  the shipped build.
* `docs/PLANE_FLIGHT.md` records `007DA8D9` as discarding the whole roll term when `ctl+FCh != 1`.
  With `ctl+FCh` now established as `0` in free flight, `1` on the ground and `2` on the water,
  that reading would confine the roll term to the ground roll. One of the two is wrong; `007DA8D9`
  was not re-read here.
* What `classDesc+1FCh` is. It is the reference the lift-off height test subtracts, sits between
  the two `GroundPitch` slots `+1F8h` and `+200h`, and has no key in
  `docs/PLANE_CLASS_FIELDS.md`.
* Which of `unit+ACCh`, `unit+B1Ch` and `unit+BFCh` the physics body writes. All three are read as
  finished values here.

## `no_ghidra_function`

Every routine this doc names has a Ghidra function and its body range is quoted where the routine
is introduced. Five `unit+900h` store sites lie inside blocks Ghidra has no function for and their
starts were not established: `007C36BC`, `007D63EA`, `007D65DD`, `007D6600` and `007D711A`. None is
named here.
