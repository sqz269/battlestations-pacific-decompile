# Ship natives, third batch: begin_command, throttle ceiling, neighbour list, order tail

Packet cc9_ship_natives_3, 2026-09-23. Base: main 8d47bb2c5. Switches
`kDirectorBeginCommandBound` and `kShipAiOrderTailBound` in src/game_hosts_ship_ai.cpp, both
on. Status: reconstructed, build-tested and run-compared on USN04 and USN01. Not
ABI-compatible and not game-validated. Descriptive names are hypotheses.

Addresses: 00835C70 BSP_WeaponDirector_BeginCurrentCommand, 0071F290 (arms 5 and 6),
0071E7F0 BSP_WeaponDirector_SetOverrideCommand, 00836D67 (the override arm), 009F4DA0
BSP_ShipAi_ThrottleCeilingStep, 009F0D20 BSP_ShipAi_NeighbourListAdd, 009F1A25 (its only call),
009F0100 (the order tail), 0092D730, 009D8010.

Two items landed, two are read and left as records, and one premise is corrected:

| Item | Result |
|---|---|
| begin_command / override | bound (the accepted byte); the override is unreachable in these missions |
| throttle ceiling 009F4DA0 / 009F4DC7 | read; not bound (inputs outside the boundary) |
| neighbour list add 009F0D20 | read; not bound (the candidate walk and node constructor are unprojected) |
| order tail 009F0100 | bound for the empty list, the only state this process reaches |

## 1. begin_command, and the override the brief expected it to unlock

**The premise is wrong.** 00835C92 does not write director+188h. It is `LEA EAX,[ESI+18Ch]`:
00835C70 selects slot 0 (+54h, +58h) when its argument byte is set and the override (+188h,
+18Ch) when it is clear (00835C7E..00835C92), and reads that descriptor.

A store census finds the writers of +188h:
- `89 ?? 88 01 00 00`: 0071E89E in 0071E7F0 SetOverrideCommand, 00720296 (the constructor) and
  0072074E (the state copy)
- `C7 ?? 88 01 00 00`: 0071E6AB in 0071E610 (the clear) and 007784A9 (the session pump)

0071E7F0 is called from 00721A40 and 00721890. In the host that call is the record
`WeaponDirector::queue_command`, and it is never reached on USN04 or USN01. The override arm
00836D67 therefore stays unreachable here, and binding begin_command does not change that. A
dated correction is appended to docs/SHIP_NATIVES_2.md, and the commands host's comment is
fixed.

**What was bound.** 0071F290's arm 6 (0071F34F..0071F372) calls vtable[78h], which is
00835C70, only while director+44h is 0 and a head command exists. 00835C70 writes that accepted
byte, so the arm runs once per command (docs/DIRECTOR_UPDATE_ARMS.md). The host passed
`queue_accepted = false` every update, so it called begin on every step (94,500 calls on
USN04). It now keys +44h on the head command it was written for: slot 0's command object and its
target's object id, through the new `GameCommandsHost::director_head_key`.

One approximation remains. An identical re-issue of the same command on the same target does not
re-arm the byte. The host's order ring suppresses such re-issues anyway.

The begin body itself stays one record per command, for two reasons. Its `attackmove` arm
(00835DAE..00835E07) sets the director's fire target through 00835860, which belongs to the
gunnery host. Its `cruise` arm already runs where the command is pushed (milestone 2l).

## 2. The throttle ceiling, 009F4DA0 (read, not bound)

`__thiscall(brain)(float)`, RET 4, body 009F4DA0..009F50D0. Its only exit is the tail call to
009F3F80 at 009F50C6.

1. **Setup.** brain+350h = 1.0f (00D7A24C). A set brain+0B38h skips everything.
2. **009F4DC7.** brain+34Ch = min(brain+34Ch, brain+0AF0h), the brain speed scale.
3. **Leader arm** (00778890 true, 009F4E13..009F4F05):
   - g = min(0070D140(group), 0070D0F0(group)), divided by 0080FC30(unit), the reference speed
   - brain+34Ch = min(brain+34Ch, g)
   - brain+350h = 0082E850(class) / (0070E3C0(group) x 1.2), where 1.2 is the double at
     00CEC160; set to 0.75 (00CEE07C) when below it, capped at 1.0
4. **Follower arm** (007788B0 true, 009F4F1D..009F50B5):
   - 00863780(1) on unit+6DCh
   - s = (0070D0F0(group) + 6.70421028) / 0080FC30(unit), with the addend the double at
     00D21B38, passed through 00415620 with the floats 1.0 (00D7A24C) and 1.25 (00CF29A8);
     the argument order was not re-derived
   - when brain+3ADh is set, 009F4F99..009F5003 writes brain+34Ch from +3A4h, or its
     difference from the float at 00D7A208, depending on +364h and the sign, and sets or clears
     brain+374h. The arm was read only at this level
   - brain+34Ch = min(brain+34Ch, s)
   - brain+350h = min(0070E3C0(group) x 1.25 / 0082E850(class), 0082E850(class) / 200.0),
     with 1.25 the double at 00CF87C0 and 200.0 at 00CE4D70, then 00415690 in place with the
     floats 1.25 and 1.0 (argument order not re-derived)

**Why it is not bound.** The group speeds 0070D140, 0070D0F0 and 0070E3C0 live on the unit
group. The units host does not compute them (its note: "0070DA00's speed ceiling over the new
membership is NOT run"), and the units files are outside this packet. brain+0AF0h has writers
only in the attackmove, follow and land states (009E262F, 009E2A1F, 009F392F, 009F3952,
009E1E48..009E200C), and the host implements none of them.

So both the 009F4DA0 and 009F4DC7 records stay. Today the host caps against
`tail.throttle_ceiling_344` from 009EEF0A, or 1.0.

## 3. The neighbour list add, 009F0D20 (read, not bound)

Its only call is 009F1A25 in the brain pre-pass's proximity walk (009F158A..009F1BB8). That
walk runs over the world list [[00E188A8]+19CCh] on the +0B48h / +0B50h countdowns and is not
projected. The admission itself is reconstructed: `ship_ai_neighbour_admit_009f0d20` in
src/ship_ai_neighbour_admission.cpp admits kind 14 or kind 12 without BigLandingShip, and ages
the entries against the +194h memory of 00424C40.

It needs three things this process lacks:
- the candidate walk
- the complete node constructor 009E52E0
- a producer for the node footprint +44h..+60h

**How it differs from the traffic records (docs/SHIP_AI_TRAFFIC.md).** Traffic records hold
detected enemies within weapon range + 200. They are weighted by their firepower and steer the
avoid slot term. Neighbour records are nearby small surface units of any side, in 90h-byte
nodes. They feed the sector scan and the obstacle drive, and the order tail below reads them.

## 4. The order tail, 009F0100 (bound for the empty list)

`__thiscall(blk)(float)`, RET 4, body 009F0100..009F0A17. Both arms walk the neighbour list:
- blk+35Ch == 0 with |0092D730 body speed| < 1.0 walks the slots at +608h and, for a node where
  009D8010 answers, clears node+88h and node+75h
- otherwise it walks and runs the geometric body

Both test blk+604h first (009F0163 `JLE 009F09FF`, 009F01BF `JLE 009F09FF`). Before either
test the routine only reads the speed, so an empty list stores nothing.

The host's list is always empty, because item 3 is unbound. `kShipAiOrderTailBound` marks that
arm as implemented, and a non-empty list would still reach the record. The geometric body
009F01CB..009F09F9 is unread.

## 5. Predictions and measurement

Predictions were written first to local/n3_predictions.txt: no decision, speed or gunnery row
moves, and only the method table changes. That held.

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides. Logs: local/n3_{ctl,trt}_usn04.log at
4700/4500 and local/n3_{ctl,trt}_usn01.log at 3200/3000. The ship-AI step, standoff, impact,
death and throttle lines are identical in both pairs; the only differing line is the run header.

| Native | USN04 ctl | USN04 trt | USN01 ctl | USN01 trt |
|---|---|---|---|---|
| CommandController::begin_command (unimplemented) | 94,500 | 67 | 186,000 | 81 |
| ShipAiOrder::tail_009f0100 | 81,000 unimplemented | 81,000 implemented | 42,000 unimplemented | 42,000 implemented |

**Unimplemented calls on USN04**, summed over the method table:

| Build | Unimplemented calls |
|---|---|
| control | 6,122,588 |
| treatment | 5,947,155 |
| fall | 175,433 |

Decision: both bindings landed. Neither changes any decision row.

## 6. Open

- The throttle ceiling needs the unit-group speed routines 0070D140, 0070D0F0 and 0070E3C0 in
  the units host, and producers for brain+0AF0h (the attackmove, follow and land speed scale).
- The neighbour list needs the pre-pass proximity walk 009F158A..009F1BB8 and 009E52E0.
- The override arm needs 0071E7F0 bound on the 00721A40 message path, and a mission that sends a
  non-queued command.
- 00835C70's attackmove arm sets the director fire target, which is gunnery territory.
