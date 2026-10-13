# CC13 VER 15: the carrier and airfield launch side, verified against the bytes

Packet cc13-ver15. This pass is verification only. It checks the launch slot state machine, the
elevator and spot, the wing-fold hold, the takeoff task and deck run, and the plane stock. Every
host function in scope was compared with the disk bytes (Capstone `disasm-raw`, with x87 read from
the opcode bytes and the `sc2_x87.py` tracer). Constants were read from the PE.

The evidence is the PRCP01 run `lead_kl2_on_PRCP01.log` and the JM08 run `lead_kl2_on_JM08.log`
(both in the cc9 tree, dated 2026-10-13), plus the USN21 run `l56_c1_u21f` (cc9-lua52). No new runs
were made. The host files were read at f0beabf8e.

## Verdict

- **Per-plane launch timing is image-faithful.** The PRCP01 Enterprise trace runs at about 12 s
  per plane. Every interval in it is accounted for by image constants, and the host reproduces
  each one to within a 0.05 s step.
- **One outcome-relevant divergence: the stock is never consumed** (section 4, D1). It inflates
  the player strike counts in JM08 and USN21.
- **One smaller divergence: 006CC690's busy arm is dropped** (D2). No current log reaches it.
- **Several divergences that only matter once the carrier is dead** (A-D1..D3).
- **A correction to docs/CC13_GPR_48.md section 8.3 P1.** The host is equivalent to the image
  there; no switch is needed.

No host file was edited. `src/air_operations.cpp` and `include/bsp/air_operations.hpp` were leased
to cc13-fix5 (`cc13-fix5-airops`) when D1 was found, so the fixes ship as models in
`src/launch_ver_cc13.cpp` and `include/bsp/launch_ver_cc13.hpp` with an adoption note.

## 1. The cycle as measured (PRCP01, Enterprise_sqn01, fighters class 101)

| step | host time | image source of the interval |
|---|---|---|
| launch task sends member 1 | 30.95 | 007F1F00, PlaneSendInterval +190h = 2.0 |
| placed on the lift, lift raises | 33.55 (+2.60) | the empty lift goes down 2.10 s, then LiftDelay 0.5 (tuning+510h), then one step |
| ground state 2 -> 4, takeoff task installed | 35.70 (+2.15) | lift up 2.10 s + one step |
| prepare -> SlowTakeoff | 40.60 (+4.90) | wings spread 3.3 s, then PrepareTime 1.5 s, permission at once |
| SlowTakeoff -> Takeoff | 40.70 (+0.10) | 009CFB60, +AA0h <= 0 arm |
| bow edge, lift-off C6h | 44.70 (+4.0) | the deck run (009CE2C0 aligned arm, 007DB680) |
| member 2 placed | 45.55 | lift blocked while member 1 was inside lift z-28..z+14 (006D02F0); 2.6 s after the unblock |

**The elevator (006FC380, 006FC480).**
- +1Ch = depth * 0.1 (the double at 00D7A3A0). This installation's `shipglobals.lua` gives
  ElevatorSpeed 4.0 and ElevatorDepth 7.0, so a trip is 8.4 m at 4 m/s, which is 2.10 s.
- The travel scale +4Ch is 1.0 (00D7A24C). 006FBCE0 changes it only when the session mode is 2,
  the client case.

**The wing hold.**
- 007C1430's state change calls 007C11E0(0). States 4-7 set the wings' target to spread.
- 007DE1E0 steps the channel at the rate 1/class+5E0h. class+5E0h is the gap between
  PartAnims.Wings [1] and [2], which is 3.3 s for class 101. The step is the plain dt with no
  other scale, and the value is clamped to [0, 1].
- 009CDE50 returns at 009CE106, before it adds to its +1Ch clock, until 007B8D10 reads the wings
  at exactly 1.0. PrepareTime (tuning+4DCh, 1.5 s) therefore starts only after the spread.

The cadence is about 5 planes a minute per lift. The deck run sets it; the elevator does not.

## 2. Verification tables

### 2a. Elevator, spot and ground state (src/game_hosts_units.cpp)

| image | host line | verdict |
|---|---|---|
| 007F1F00 launch task | 13673 | MATCH for timing; A-D1 |
| 006FC380 setup, 006CF980/006CF9F0 site tick | 13384, 13749 | MATCH |
| 007C5F60 place on spot | 13785 | MATCH. The tail call, squadron vtable+DCh = 007F22E0 (see the note below the table), is skipped. |
| 006CFE90, 006CFF70 (3.0 at 00D7A2B0), 006D0150, 006D0390 (14, 2, 5, 1) | 13423-13466 | MATCH |
| 006FC0D0 carry | 13554 | MATCH (the host also zeroes velocities, labelled) |
| 006FC250 release | 13583 | A-D2 |
| 006D02F0 blocked (14.0 at 00E08FC8, 2.0 at 00D7A308; `D8 CB`, `DE E9`) | 13910 | MATCH |
| 006FC640, 006FC810, 006FC720 | 13927, 13944, 14362 | MATCH |
| 006D0600 site tick (1.3889 at 00CF8AAC, LiftDelay, last-candidate rule) | 14387 | MATCH on timing; A-D3 |
| 006FC480 platform tick | 14488 | MATCH |
| 007CC7A0, 007CC820, 007CA3F0 | 13599, 13633, 13979 | MATCH |

The note on 007F22E0, the skipped tail call of 007C5F60:
- It has no Ghidra function; it sits inside the 007F1FE0 candidate.
- It snaps the squadron's pose to the leader's and invalidates the child poses.
- Within 2 s of a scripted expiry (squadron+308h) it zeroes the members' effect intensity +2F4h.
- Skipping it has no effect on timing, orders or formation.

### 2b. Takeoff task and deck run

| image | host line | verdict |
|---|---|---|
| 0099A4A0, 009CF8E0, 009CD420 (15.0 at 00CE5380, 25.0 at 00CE89CC) | 14014 | MATCH |
| 009CDE50 prepare step (wing hold, 0.1 s wobble, member within 10.0 m at 00CE38B8) | 30739 | MATCH |
| 009CDD10 permission, 006D01C0/006BEFF0 deck order | 14306, 14326 | MATCH |
| 009CE1D0 (±0.3 at 00D06888 / 00CE69C8), 009CFC70, 009CFB60 | 14297, 30663, 30726 | MATCH; 009CFB60 rests on the labelled +AA0h = 0.0 substitution |
| 009CE2C0 run: heading/steer, lateral tolerance, alignment, brake hold, taxi arms, aligned throttle arm 009CF2ED-009CF504 (42 constants) | 30834-31274 | MATCH |
| 007CC1B3-007CC2B3 lift-off request (0.8 at 00CE3D40, 0.1 at 00D7A3A0) | 33696-33714 | MATCH (single player) |
| 007C6F50 lift-off | 14107 | MATCH, with the open point below |
| 007C71E0 / 007D9CE0 reparent | 14519, 13335, 13348 | equivalent; see section 3 |
| 007DB680 ground law | 33664 | not re-read here; ver1 (CC13_GPR_48.md section 8) found only rounding differences |

The open point on 007C6F50:
- It reads byte +20h of the C6h message and, when the byte is set, calls 007D83D0, a 0.8 s
  direction hold.
- Both send sites (007CC212, 007CC26E) build the message on the stack, and 00762A00 does not
  write +20h, so the image reads an uninitialised stack byte.
- The host takes it as 0, and that is labelled in the source. The effect would come after
  lift-off only. Confidence in the image's value is low.

### 2c. Wing fold (ON in the host)

| image | host | verdict |
|---|---|---|
| 007EABC0 channel +44h; 007D39F9-007D3A4F duration ([3] overrides at 007D3ADE) | units.cpp 13487, 13497 | MATCH |
| 007DE1E0 channel step, 007DE3A0 block step | torpedo_release_spawn.cpp 20, 47 | MATCH |
| 007C11E0 notify (states 4-7 spread) | units.cpp 13518 | MATCH, with one labelled substitution (below) |
| 006CFFC0 (20.0 at 00CE3930, FCOMI/JBE, NaN gives false) | 13546 | MATCH |
| 007CBFA0 steps 2 and 7, 006D06E8 (taken down only at 0.0), 006FC0D0 zeroing | 33639, 33669, 14421, 13564 | MATCH |

The substitution in the notify:
- The host compares the state once per step instead of inside 007C1430.
- That can move the start of the spread by one step, at most 0.05 s. This is not visible in the
  launch spacing.
- Not checked: whether the deck-spawn sites 007C641A and 007C649F push argument 0 or 1.

### 2d. Slot state machine and stock (src/air_operations.cpp)

| image | host line | verdict |
|---|---|---|
| 006C0510 slot tick (5.0 at 00CE3850) | 684 | MATCH |
| 006BF230, 006BD3F0, 006C7210, 00895D20, 006BF620, 006BED60 | | MATCH |
| 006C64B0 queued wait (1.0 at 00D7A24C, COMISS/JBE at 006C64E5) | 1027 | MATCH. The measured order-to-build is 19 frames at 20 Hz. |
| 006C65B0 release (state 1, timer 5.0, +34h cleared) | 495 | MATCH |
| 006C7490 launch start | 521 | MATCH, with two gaps that do not affect timing: +28h is stored even for a null build (006C7516), and 00922F30/006BF150 are not hosted |
| 006C56D0 / 006C58A0 | 480 | MATCH in store order |
| 006CA640 queue slot | 932 | **D1** |
| 006CC690 LaunchSquadron | 599 | **D2** |
| 006CCDA0 slot command | 948 | partial: the state-5 order-2 stand-down (006CCE01-006CCE7B) is not modelled |
| 006CD240 | units.cpp 15375 | MATCH. It is the landing-sequencer timer (14h records at block+98h, 006CC9F0); it does not regenerate stock |
| 006CD810 AI launch (gated on block+F4h, every 2.0 s) | | NOT HOSTED; the writer of +F4h/+E8h was not found |
| 006CD350 | 80 | hosted model with no caller. It is SetAirBaseSlot's routine and does not launch |

The image order inside 006CDC70 is:
1. 006C58A0
2. 006C0DA0
3. 006C77E0
4. a return when the game state is 2
5. the virtual at block+3Ch
6. 006C64B0
7. 006C6540
8. 006CD240
9. 006C5B70, when the deck is free
10. 006CD810

The host does not run 006C77E0, 006C6540 or 006CD810.

## 3. Correction: CC13_GPR_48.md section 8.3, P1 (007D9CE0)

The P1 entry predicts a 10-20 deg heading error for a plane parented to a turning carrier. That
prediction is wrong.

- The host does add and subtract the parent's world velocity (game_hosts_units.cpp:14525 and
  :14111).
- `carrier_deck_capture` (13343) stores the result in carrier axes, and every ground step
  `carry_in` (33633 -> 13356) rotates it back to world with the carrier's current matrix.
- That is R_new^-1 (v - v_parent) on the way in and R_old v_local + v_parent on the way out, which
  is the image's arithmetic. The relative velocity does turn with the carrier.
- Residuals: ctl+24h holds body rates, which do not depend on the frame. The ctl+30h copy is
  inert, because the host does not carry C2. B63D50 is a scaled inverse, so the two agree only
  when the carrier matrix has unit scale (not checked).
- No P1 switch is needed.

## 4. Divergences

**D1. The stock is never consumed. Outcome-relevant; high confidence.**

Image:
- 006CA640 at 006CA6A0-006CA6AB looks for the slot index in the queue at block+10h.
- If the index is not there, 006CA6D5 calls 006C7D10 ConsumeStock(slot+4, slot+8).
- If it is there, 006C48F0 (list::remove) unlinks it and it is re-appended with no consume.
- 006C7D10 does entry.count -= min(entry.count, count) (006C7D81-006C7D8E), then sends the
  STOCKITEM message (006BD9E0, 0077C7B0) and calls 00984EB0 for the statistics.
- 006CA8E0's flag-1 arm (00896750, launch now) also consumes.
- A direct 006CC690 launch (block+38h clear) does not consume.

Host:
- src/air_operations.cpp:942 marks 006C7D10 "not read", and nothing else subtracts from
  `deck.stock`.
- The C7h stock return (game_hosts_units.cpp:14198, ON) adds 1 for each landed plane.
- Stock therefore never drops. In PRCP01 the script's AddAirBaseStock(class 101, +20) took
  entity 1's stock from 30 to 50. If the two 3-plane Enterprise sorties queued before it came from
  that stock, the image would show 44 (not established which deck entity 1 is). No stock-return send happened in that run (`sends=0`).

Measured against the image cap:

| mission (log) | deck, class | host sorties x planes | stock | image cap |
|---|---|---|---|---|
| JM08 (`lead_kl2_on_JM08`) | airfield, 166 | 7 x 3 | 9 | 3 sorties |
| JM08 | airfield, 167 | 6 x 3 | 9 | 3 |
| JM08 | airfield, 152 / 154 | 11 x 3 each | 30 | 10 |
| USN21 (`l56_c1_u21f`) | six carriers, 38 or 113 | 16-25 x 3 | 40 per class | about 13, then 1 plane, then 0 |
| PRCP01 | Enterprise | inside stock | | unchanged |

All 43 JM08 launches are `player air ops launch`, the 006CCDA0 order 3 -> 006CA640 path, so all
of them consume in the image.

Predicted effect:
- JM08 and USN21 player strikes stop once each class's stock is spent. Win rows that depend on
  repeated airfield or carrier strikes lose the extra sorties.
- After exhaustion 006C0F00 yields a count of 0. What the host's harness and builder do with a
  0-count launch must be checked when the fix is paired.
- No RNG draw changes.

Fix: `bsp::air_ops_queue_slot_consume_006ca640` and `bsp::air_ops_consume_stock_006c7d10`
(models, OFF until adopted).

**D2. 006CC690's busy arm is dropped. High confidence on the code; no current effect.**

- Image (006CC715-006CC72C): with block+38h set, it calls 006CA770(class, count) and then
  006CA640, so the stock nets to zero and the slot launches about 1 s after the brake clears.
- Host (air_operations.cpp:630-633): it only sets `queued`, and the slot is never launched.
- 0 hits in PRCP01, JM08, USN21, IJN08, IJN18 and J06, because the scripts test
  IsReadyToSendPlanes first.
- Model: `bsp::air_ops_launch_busy_arm_006cc690`.

**A-D1 / A-D2 / A-D3. These only matter once the carrier is dead or out of play; no timing
effect.**

- **A-D1, 007F1F5C-007F1F71.** A null squadron, null block, null owner or dead owner retires the
  launch task through vtable+10h. The host returns and keeps the task alive (units.cpp:13686-13687).
- **A-D2, 006FC279-006FC28F.** 006FC250 detaches the plane only in mode 0 with +18h greater than
  the depth. The host detaches unconditionally (units.cpp:13592), and the top release re-enables
  it under `kBaseLaunchChainBound` (14409). This affects hit queries only.
- **A-D3, 006D0689-006D06A3.** The top arm also requires carrier+1188h->+7Ch to be present and
  alive. The host checks only that the owner slot exists (units.cpp:14390).

**Other observations, not divergences of the code in scope.**
- In PRCP01, Junyo's launch brake was set at frame 1560 and never released (held_at_end = 2).
  IsReadyToSendPlanes then answered false for the rest of the run, 202 calls. The cause lies in
  the 006C5B70/007ED740 substitution, which was not established here.
- 00896750 LuaBinding_LaunchAirBaseSlot (cc13-fix5 is hosting it; verified only, not wired):
  - It takes (entity, 1-based slot, optional bool). A slot already in state 2 returns nothing.
  - Otherwise it routes 83h {slot-1, bool} to 006CA8E0:
    - bool true: 006C7490(slot, 1), then ConsumeStock.
    - bool false: state 2, the timer pair, and the slot re-queued, with no consume.
  - Only then does it write slot+34h = 1. It runs 006CDC70(0.0) between two calls of an extra
    step.
  - In that dt = 0 update, 006C0510 lifts the timer to 5.0, so 006C64B0 launches the queued slot
    in the same call when block+38h is clear and 006BED60 holds.
  - If slot+28h is then set, it returns `thisTable[squadron+174h]`.

## 5. Models (reconstructed, build-tested, not wired)

`include/bsp/launch_ver_cc13.hpp`, `src/launch_ver_cc13.cpp`, registered through one line appended
to `cmake/startup.cmake`.

| address | function |
|---|---|
| `006C7D10` | `air_ops_consume_stock_006c7d10` |
| `006CA640` (stock arm) | `air_ops_queue_slot_consume_006ca640` |
| `006CC690` (busy arm 006CC715-006CC72C) | `air_ops_launch_busy_arm_006cc690` |

Build: `./scripts/build.ps1` in this tree passed; `bsp_game` linked with `launch_ver_cc13.obj`, and
the CTest suites `reconstructed_math` and `tool_tests` passed. No test was added. The call sites the
models cite are in `reports/cc13_ver15_calls.json`; `tools/verify_report_calls.py` checked them
against live Ghidra (7 rows, 0 failed). Ledger: three records in
`config/reconstruction/006c0000.jsonl`.

Adoption, for whoever holds `src/air_operations.cpp`:
- Call `air_ops_queue_slot_consume_006ca640` in place of `air_ops_queue_slot_006ca640`. The call
  site is `air_ops_slot_command_006ccda0`, :961.
- Call `air_ops_launch_busy_arm_006cc690` in the `else` arm at :630-633.
- Put both behind one OFF switch and pair JM08 and USN21.

## 6. Not checked and open

- The engine spool (+9F0h) and the thrust ramp of the deck run, and 007DB680 beyond ver1.
- The order, inside one image frame, of the site tick (+4h), the platform tick (+8h, 006CFE40)
  and the squadron task. It is bounded by one step.
- Who writes block+F4h/+E8h (006CD810, the AI launch), and so which decks run it.
- The C6h message byte +20h (007C6F50).
- Junyo's brake in PRCP01, which was held to the end of the run (section 4).
- How the host handles a 0-count player launch once the stock is spent (needed before D1 is
  switched on).

## Handoff

This packet is done. Every timing item on the launch side was verified. D1 and D2 are waiting on
the holder of `src/air_operations.cpp` (cc13-fix5 at the time of writing) to adopt them. The
correction to P1 in section 3 should be copied into docs/CC13_GPR_48.md section 8.3 by that
document's owner.

## 7. Adoption (cc13-fix7)

Both fixes are in the host behind their own switches, both OFF and unchanged byte for byte until
they are paired. They are build-tested, not game-run.

| switch | divergence | call site | sha |
|---|---|---|---|
| `kAirOpsQueueConsumeStockBound` | D1 | `air_ops_slot_command_006ccda0` calls `air_ops_queue_slot_consume_006ca640` | 7c7e90a08 |
| `kAirOpsBusyArmQueueLaunchBound` | D2 | the `else` arm of `air_ops_launch_squadron_006cc690` calls `air_ops_launch_busy_arm_006cc690` | 7faa8d8cb |

- **One ConsumeStock.** launch_ver_cc13 had its own copy of 006C7D10, and cc13-fix5 had
  `air_base_consume_stock_006c7d10` (script_natives_fix5_cc13). The two differed only in what
  they returned: the amount taken against the node's count afterwards. The image returns
  neither. It leaves 00984EB0's EAX at 006C7E0E, and no caller reads it. The launch_ver copy is
  deleted, so the model now calls fix5's. The result field is renamed `stock_left`.
- **A 0-count player launch.** In the image, 00673A10 calls 006BF310(class) at 0067480E. That
  returns 006BF230's total. When the total is 0 or less, 0067481B JLE 00674B46 leaves before
  006C0F00, so the slot is never written. The host ran 006C0F00 first and refused only once it
  returned 0 (game_hosts_script_orders.cpp). By then a count of 0, and the class when it
  differed, had already been written into the idle slot, which 006BF230 and 006BD3F0 both read.
  Under D1's switch the host now refuses before the fill, as the image does. Without consume,
  the total only reaches 0 for a class the base never stocked.
- **Open, separate from D1 and not changed here.** When the class has stock but 006BD460's plane
  room clamps the count to 0, the image goes on to route 82h with a count of 0 (0067484D
  onwards; it does not test the count). The host refuses at "006C0F00 left the slot empty". What
  a 0-plane launch does in 006C7490 was not traced.
