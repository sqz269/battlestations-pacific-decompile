# The ship motion tail after the controller step (packet `cc9_ship_post_motion_read`)

Addresses: 00826B84..00826D69 (in 00825F20 `BSP_UnitInstance_UpdateShipMotion`), 008160B0,
0092E5B0, 00811890, 00810190, 00778890, 0070DB60, 0077A650.

This is a read-only packet. No source was touched, and Ghidra was read only.
- **Ranking 3 row 2** is `ShipMotion::unit_post_motion`, the host record at 00826B84. It has
  163,434 calls on E2 and 285,540 on USN02.
- This document says what the image does from 00826B84 to the epilogue, what the host does
  instead, and how to bind it.
- `src/game_hosts_units.cpp` belongs to cc9-plane-release while this is written, so the binding
  is a plan with exact hunk boundaries.

## 1. Registers and scope

In 00825F20 `ECX` is the unit's sub-object at unit+310h:
- `EDI = this` (00825F2A), and `ESI = EDI-310h` (00825F32) is the unit;
- `[ESP+0C4h]` is the scaled delta.

Every `EDI`-relative offset below is written as a unit offset (`EDI+n` = unit+310h+n).

`docs/SHIP_MOTION.md` reconstructed 00826121..00826B84 (`ship_motion_step_00825f20`), and
`docs/SHIP_AI_RUDDER_HOP.md` reconstructed 00826C34..00826D69 (`ship_motion_tail_00826c34`,
`src/ship_ai_rudder_hop.cpp`). The host binds the first and not the second. This document covers
00826B84 itself and the unbound tail, and it finishes the reading of the tail's four callees.

## 2. What the image does

| address | what happens | unit fields | host today |
| --- | --- | --- | --- |
| 00826B59..00826B6A | `0092BE80(controller = [unit+1018h], dt)`, the controller step | - | bound (`ShipMotion::controller_step`) |
| **00826B84** | `unit->vtable[1ECh](dt)`. Every ship vtable (00CFC3D0, 00D0BF80, 00D01630, 00CFB738, 00CFA778, 00CFFA30, 00CF90B0, 00D0C648) holds **008160B0**, see section 3 | reads class+D0h, +5Ch..+60h | **the record `ShipMotion::unit_post_motion`**. The repair tick it reaches runs in the gunnery host instead (section 3) |
| 00826B86..00826C34 | two inlined world-matrix accessors: if `unit+C8h` is clear, rebuild `unit+CCh` from `unit+74h` (times `[unit+3Ch]+CCh` when there is a parent) through 00413920 / 004134F0, set `+C8h` and clear `+10Ch` | writes +CCh..+108h, +C8h, +10Ch | not needed: the host's pose cache is always valid |
| 00826C3B..00826C56 | `unit+1050h = atan2(row2.x, row2.z)` of the world matrix (`FLD [+20h]; FLD [+28h]; CALL 00BF701A`) | writes **+1050h** | not stored. The host computes the same expression where it needs a heading (`unit_heading_radians`, the wake block) |
| 00826C5C | `0092E5B0(controller, 0)`, section 4 | controller+40h..+5Ch | not called; a no-op in single player |
| 00826C61..00826C81 | `yawRate = 00811890(unit, [unit+984h])`: the **ordered** rudder (the ring's published value) through the rudder curve | reads +984h | **wrong input**: the host passes `result.steering.rate_row1`, labelled PROVISIONAL at `game_hosts_units.cpp:16057` |
| 00826CC5..00826CDB | `heading2 = unit->vtable[50h]()` = 006DFD60 `FLD [ECX+1050h]; RET` (no argument). The yaw rate pushed before it stays on the stack as 00810190's last argument | reads +1050h | - |
| 00826CDD..00826CEE | `00810190(unit+0BD0h, &unit+0FCh, heading2, yawRate)` `BSP_UnitWake_AppendSample` | writes the wake at +BD0h | **bound** (`UnitWake::append_sample`, `game_hosts_units.cpp:16049..16069`), with the provisional yaw rate |
| 00826CF3..00826CFC | `if (00778890(unit))`: the unit's group record `[unit+284h]` has `+14h` pointing back at it, i.e. the unit leads its group | reads +284h | answered elsewhere in the host (`unit_formation_group_0284`) |
| 00826CFE..00826D3E | `t = unit+1158h - dt; unit+1158h = t; if (t < 0) { unit+1158h = t + settings+430h; 0070DB60([unit+284h]); }` | writes **+1158h** | **missing** |
| 00826D43..00826D5A | `if ([unit+308h] != 0.0f) 0077A650(unit)` (the `UCOMISS/LAHF/TEST AH,44h/JNP` inequality; an unordered compare also calls) | reads +308h | **missing** |
| 00826D5F..00826D69 | epilogue, `RET 4` | - | - |

`settings+430h` is `Formacio.UpdateInterval` (`include/bsp/gameplay_settings.hpp:165`). This
installation's `scripts/datatables/shipglobals.lua` line 414 sets it to `10.0`, commented "how
often we try to swap ships".

## 3. 008160B0, vtable slot 1ECh: the repair tick (no Ghidra function)

`ghidra proto --brief 008160B0` shows no function. **The body is 008160B0..008160EC**: `RET 4` at
008160EA, then `INT3` padding from 008160ED. Ledger name `BSP_UnitInstance_TickRepairTask`
(a hypothesis).

```
008160B0  MOV EAX,[ECX+538h]          ; the unit's class
008160B6  CMP byte [EAX+0D0h],0       ; class flag: repairable
008160BD  JE  ret
008160BF  CMP byte [ECX+5Ch],0 ; JE ret   ; +5Ch set
008160C5  CMP byte [ECX+5Dh],0 ; JNE ret  ; +5Dh clear
008160CB  CMP byte [ECX+60h],0 ; JNE ret  ; +60h clear
008160D1  CMP byte [ECX+5Eh],0 ; JNE ret  ; +5Eh clear
008160D7  0093CA20(unit+0A20h, dt)    ; BSP_RepairTask_Update
008160EA  RET 4
```

**The host already runs this tick.** `GameGunneryHost::Impl::run_damage_control`
(`src/game_hosts_gunnery.cpp:6624`, cc9-gunnery2's file) runs 0093CA20 per ship. Its own comment
names 00826B84 and slot 1ECh, and labels two substitutions:
- it runs after the projectile pass instead of inside the motion step;
- the entity flags +5Ch..+60h are read as "not dead".

So the record at 00826B84 is a bookkeeping gap: the work runs, but from another place, as tail
(b) of `docs/SHIP_AI_TAILS.md` section 8 was. The class flag `+D0h` has no host field. The
gunnery host repairs every ship with a damage-control element set.

## 4. 0092E5B0: network correction, closed in single player

`__thiscall(controller, char)`, body 0092E5B0..0092E8A0. Ledger name
`BSP_UnitController_ApplyNetworkCorrection` (a hypothesis).
- Everything is under `controller+60h`.
- With argument 0, it advances a dead-reckoned state: `+48h += 0.05*+40h`, `+4Ch += 0.05*+44h`,
  and `+50h` by the angle rate `+54h`. It then sets the body's linear velocity (00C37E50) and
  angular velocity (00C37E20) toward that state.
- The only writer of `controller+60h` is 0092F2E0 `BSP_UnitController_ApplyNetworkState`
  (ledger), reached from the ship sync message 00816C80. No mission run here receives one.
- The call is therefore a no-op in these runs. A binding answers it as "gate closed".

## 5. 0070DB60: the formation slot swap every 10 s

`__fastcall(group)`, body 0070DB60..0070E39A (defined). Ledger name
`BSP_UnitGroup_SwapSlotsByDistance` (a hypothesis). It is read from the pseudocode; its x87 was
not checked against the listing, and it must be before a reconstruction.
- **It returns at once** when the group type `+4FCh` is 18h, or when any member slot holds the
  controlled unit `[00E188D8]`.
- **Otherwise**, for each ordered pair of members (slots at `+18h`, 34h bytes each, count
  `+4F8h`) whose classes `+538h` are equal:
  - it takes each member's position decomposed against the leader's wake (00811180
    `BSP_Unit_DecomposeAgainstWake`);
  - it takes the two slot offsets from the formation pattern (`+28h` / `+38h` arrays indexed by
    `+500h` plus the slot);
  - it compares the summed distances as assigned against the summed distances swapped
    (`sqrt`, with squared distances at or below `[00CE3820]` taken as 0);
  - when swapping is shorter, it swaps the two member pointers and their `+30h` words.
- **Callers:** 00826D3E only.
- **Host:** the formation groups exist (`GameUnitsHost::Impl::FormationGroup`, created by 0070DB20
  and joined by 0070EF30), with `column` for `+500h` and `type_04fc` for `+4FCh`. Nothing
  reorders their members.

## 6. 0077A650: the scheduled expiry

`__thiscall(unit)`, body 0077A650..0077A689. Ledger name `BSP_UnitInstance_ExpireAtScheduledTime`
(a hypothesis).
- **What it does:** if `[unit+308h] <= [00F876A4]` (the fixed-step clock) and
  `[game+1FE4h] != 2`, it calls 0090EBF0(game+21A0h, unit) and then 00926D90(unit, 3), the entity
  kill with reason 3.
- **Callers:** 00826D5A, and 007F3D1C (the squadron tick).
- **Writers of `unit+308h`:** the constructor at 0077EFFE (`MOVSS [ESI+308h],XMM0`, its value set
  before the window read), and message 51h at 0077F7B0 (`docs/SESSION_MESSAGE_DISPATCH.md` row 7).
- The three ship-AI stores at `+308h` (009E1AC2, 009E5ACA, 009F3061) go through `[state]`, which is
  the brain (`brain+308h`, the requested layer), not the unit.
- **No host path sends message 51h**, so in these runs the branch is taken only if the
  constructor stores non-zero. That is to be confirmed by the census row in the binding.

## 7. Consumers of the writes

- **`unit+1050h`** is read through vtable slot 50h (006DFD60) by:
  - the wake call above;
  - the ship AI (`ShipAi::drive_heading_vtable50`, ranking 3 row 8, a mirror);
  - the HUD's heading readers.
  The host answers those from the pose, with the same expression, so storing it changes no value;
  it only makes the mirror concrete.
- **The wake's yaw rate** is read back only through 0070D290's `out[4]`. 009DF2D0 uses it for a
  follower's speed blend (`docs/SHIP_UNIT_GROUP_FOLLOW.md` section 5b). Followers exist in both
  missions: `ship follow steppers=16` on E2 and `13` on USN02 (`local\rb3_*.log`). Correcting the
  input from the steering rate to `00811890(ordered rudder)` moves the followers' speeds.
- **The slot swap** changes which member takes which station. Followers steer to their stations,
  so every follower's path can move after the first 10 s tick.
  - E2 has 2 groups (25 joins), and USN02 has 3 (21 joins).
  - A group holding the controlled unit (the Lexington, or DeRuyter) never swaps. The logs do not
    list members, so which groups hold them is **not established**. The binding should log group
    membership once at the start.
- **The repair tick** feeds the damage-control element and hull repair (gunnery), which it already
  does.
- **The expiry** kills a unit with reason 3.

## 8. Binding plan

Three switches, in this order, each a separate pair. The hunks are against `main` `e3aba0f36`.

### 8a. `kShipMotionTailBound` (src/game_hosts_units.cpp)

- **Hunk 1, `ShipMotionBinding` (class at line 4887).** Keep `unit_post_motion` at line 5099 as
  is. Beside the class, add a `ShipMotionTailBinding : bsp::ShipMotionTailHost`:
  - `controller_0092e5b0`: `done`, with the gate closed (section 4); no state change;
  - `yaw_rate_from_rudder_00811890(r)`: `bsp::unit_yaw_rate_00811890(r, rudder_)` with
    `r = slot.ordered_rudder` (unit+984h);
  - `refresh_world_matrix`: no-op (the pose is valid);
  - `unit_heading_vtable50`: returns the slot's new `hull_heading_1050` field, **as the tail wrote
    it**;
  - `append_wake_sample_00810190`: the existing `bsp::ship_ai_wake_append_00810190` call;
  - `occupant_owns_unit_00778890`: the leader test the file already answers
    (`unit_formation_group_0284` and the group's `leader`);
  - `occupant_timer_refill_00424c40_430`: the loaded `formacio_update_interval`;
  - `occupant_tick_0070db60`: a **record** until 8b;
  - `field_308_crossed_0077a650`: a record, with a counter.
- **Hunk 2, the call site, lines 16049..16069.** Replace the wake block, from the comment
  `// 00826CEE, the motion TAIL` down to its closing brace after
  `host.done("UnitWake::append_sample", 0x00810190u);`, with a call to
  `bsp::ship_motion_tail_00826c34` through the new binding. Keep the old block as the OFF arm.
  Add `hull_heading_1050` and `occupant_timer_1158` to `GameUnitSlot`.
- **New census rows:** `ShipMotion::tail` 00826C34, one per host method.

### 8b. `kFormationSlotSwapBound` (a reconstruction first)

Reconstruct 0070DB60 whole, checking its x87 against the listing, into a new
`src/unit_group_slot_swap.cpp`. The host's `occupant_tick_0070db60` then calls it on
`formation_groups[slot.formation_group]`, with members and their `+30h` words swapped in the
host's `members` vector.

### 8c. `kShipPostMotionRepairOrder` (bookkeeping, the cross-host step)

`unit_post_motion` (line 5099) evaluates 008160B0's gate and marks the row concrete. The repair
work itself stays in `run_damage_control`. Moving it into the motion step needs a gunnery-host
entry and changes the order of repair against the projectile pass. That is a second, optional
pair, predicted separately.

## 9. Pairs and predictions to write before each

USN02 9200/9000 and E2 = USN04 9200/9000, streams and the death table on, one tree per pair.

| switch | rows that must move | bands |
| --- | --- | --- |
| 8a | `ShipMotion::tail` concrete at the motion-tick count (162,000 E2, 285,540 USN02); the wake appends; the ship-follow speed blend; follower positions | USN02 deaths 18..26 (22) and hit records 380..500 (440), failing between 30 and 60 s (39.65 s). E2 aircraft deaths 45..57 (51), hit records 780..900 (843), no ship sinks. The expiry counter is 0 unless the constructor stores non-zero |
| 8b | swaps per group; follower stations; `ship follow` lines | the same bands. A group holding the controlled unit shows 0 swaps |
| 8c | `ShipMotion::unit_post_motion` concrete at its count; nothing else | identical gameplay |

## 10. no_ghidra_function bodies and names

- **008160B0..008160EC:** vtable slot 1ECh of every ship class; `BSP_UnitInstance_TickRepairTask`.
- **Names added to the ledger** (all hypotheses): 008160B0 `BSP_UnitInstance_TickRepairTask`,
  0070DB60 `BSP_UnitGroup_SwapSlotsByDistance`, 0077A650 `BSP_UnitInstance_ExpireAtScheduledTime`,
  0092E5B0 `BSP_UnitController_ApplyNetworkCorrection`.

## 11. Uncertainty

- **0070DB60** was read from the pseudocode only. The pair order (`local_190` starting at 1 and
  `local_194` running to the count) and the `+30h` word swap (`piVar8[0xC]` against
  `piVar4[0x19]`) must be checked against the listing.
- **`unit+308h`'s constructor value** is set before the window read.
- **Class flag `+D0h`** has no reader in the host besides this gate, and its producer was not read.

## 12. Packet cc9_ship_motion_tail, part 8a (`kShipMotionTailBound`, committed OFF)

2026-09-26, worker cc9-plane-release.

**Two facts settled before binding:**
- **unit+308h is 0.** The constructor stores it from a cleared XMM0 (`0077EFE4` `XORPS XMM0,XMM0`,
  then `0077EFFE` `MOVSS [ESI+308h],XMM0`). No host path sends message 51h, so the tail's
  `!= 0.0f` test at `00826D43` never calls `0077A650` here. Section 11's open question is closed.
- **unit+1158h starts at 0.** `00823C30` in `BSP_UnitInstance_SEntityInit` stores it from a
  cleared XMM0, and a disp32 scan of `58 11 00 00` finds no other writer outside the tail. So a
  group leader's first tail tick expires the timer at once and calls `0070DB60`. The timer is
  then refilled to 10.0 − dt and fires every 10 s.

**The binding.** `ShipMotionTailBinding` sits beside `ShipMotionBinding`, and the call site
replaces the wake block (the old block is the OFF arm).
- **Rudder input.** The ordered rudder is `slot.ring.current_param_b`, ring+14Ch, which is
  unit+984h (`docs/MOTION_DIFFERENTIAL.md`). It is also what `motion.to_turn` copies each step.
  `00811890` gets the unit's post-step forward speed.
- **Heading.** unit+1050h is stored (`hull_heading_1050`) and read back for the wake.
- **The network correction** `0092E5B0` is done, with its gate closed.
- **The leader test** `00778890` is `formation_groups[unit+284h].leader == unit`.
- **The refill** is `ShipGlobals.Formacio.UpdateInterval`, read once from Lua.
- **Records.** `0070DB60` stays a record until 8b. `0077A650` is a record too, with a counter.
- **A mislabel, not changed here.** `step11_inputs()` labels `slot.row.rudder` as +984h, but that
  field is the smoothed rudder; +984h is the ring's ordered value. It has other consumers.

### Predictions (written before the runs)

The pairs are `local\mt_off` against `local\mt_on`, from the same tree with the switch only,
streams on and the death table on.

| row | USN04 9200/9000 (E2) | USN02 9200/9000 |
| --- | --- | --- |
| `ShipMotion::tail` | absent -> concrete at the ship motion-tick count (about 162,000) | about 285,540 |
| `UnitWake::append_sample` | the same count on both sides | same |
| `ship motion tail: Formacio.UpdateInterval` | 10.000 | 10.000 |
| `UnitGroup::swap_slots_by_distance_0070db60` (a record) | 0 -> about 46 per living group leader (1 + 450 s / 10 s), at most 92 for the 2 groups | at most 138 for the 3 groups |
| `UnitInstance::expire_at_scheduled_time_0077a650` | absent | absent |
| followers' speed blend and station keeping | they move: the wake's yaw rate changes from the steering rate to the ordered rudder's curve value | they move |
| deaths, hit records | aircraft deaths 45..57, hit records 780..900, no ship sinks | deaths 18..26, hit records 380..500, the failure between 30 and 60 s |
| rows identical | the plane-only rows until a moved ship changes a plane's world | every row before the first follower speed read |

### 8a, the pairs measured

The logs are `local\MT_OFF_USN02.log` / `MT_ON_USN02.log` and `MT_OFF_USN04.log` /
`MT_ON_USN04.log` in worktree cc9-plane-release, from `a2ea7fe6e` with the switch only, streams
and death table on. All four show the 1600x900 line, a module directory in that tree and the
final COM release.

| row | USN02 9000 OFF -> ON | USN04 9000 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `ShipMotion::tail` | absent -> concrete 285,540 | absent -> concrete 162,000 | the motion-tick count | held |
| `UnitWake::append_sample` | 285,540 on both | 162,000 on both | unchanged | held |
| `Formacio.UpdateInterval` | 10.000 | 10.000 | 10.000 | held |
| `0070DB60` record calls | 46 | 46 | at most 138 / 92 | held. One group leader lives the whole run in each mission (1 + 450 s / 10 s = 46) |
| `0077A650` | absent | absent | absent | held |
| the mission end | failed at 39.65 s on both | none on both | 30..60 s / - | held |
| deaths | 22 -> 22. The first four are identical to 51.65 s; the first moved is Kortenaer at 70.25 -> 70.30 s; one victim changes (Amatsukaze survives, Jupiter dies at 283.40 s) | 51 -> 52. The first moved is D3A #1.1\|.-3 at 93.70 -> 93.65 s; the extra victim is the Lexington CAP's `sqn01\|.-2`; no ship sinks on either side | 18..26 / 45..57 | held |
| hit records | 439 -> 411 | 843 -> 875 | 380..500 / 780..900 | held |
| other rows | follower paths move from the first follower speed read, so most later ship and gunnery lines move | same | they move | held |

**Verdict: `kShipMotionTailBound` ON.** The wake now carries the image's yaw rate, from the
ordered rudder through `00811890`. The followers' speed blend reads it, and every moved row stays
inside its band.
