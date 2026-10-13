# Two plane-input checks from cc13-gp7a (cc9-physics4, 2026-10-12)

Addresses: 007B8D40, 007B8DA0, 007B8DC0, 007B8DE0, 007B8D10, 006D06E8, 007CC18E, 009CF2F8, 0099AE52,
006FC15E, 007DE9A0, 007DE7E0, 007B95E0, 007EE790, 0099DF8B.

Source: docs/CC13_GP_007A_007F.md "For cc9". Ghidra read only; names are hypotheses.

## (a) 007B8D40 at the elevator site is channel C, not the gear

`006D06E8` (`BSP_AirOpsElevatorSite_Tick`) admits a landed plane slower than 1.389 m/s
(`00CF8AAC`) as the elevator's candidate only when `007B8D40` is true. `007B8D40` reads the actuator
block's channel C (`[+DECh]+44h`): true when its enabled byte (+44h) is 0 or its value (+48h) is
exactly 0.0, false otherwise (and for a NaN). The gear is channel B (+28h, `007B8860`).

Who moves channel C, from the call sites:

| site | routine | what |
| --- | --- | --- |
| `009CF2F8` | `BSP_TakeoffStateTakeoff_Step` | extends it (`007B8DC0`) whenever `007B8D10` says it is not at 1.0 |
| `0099AE52` | `BSP_PilotBot_Tick` | extends it, only for a player-held pilot role (`006DEEC0` false; docs/CONTROLLED_UNIT.md) |
| `007CC18E` | `BSP_Plane_GroundRollStep` | retracts it (`007B8DA0`) for a landed plane (`+904h`) whose holder surface (`+BF4h`) still holds it (`vtable[38h]`) |
| `006FC15E` | `BSP_ElevatorPlatform_CarryPlane` | resets it to zero (`007B8DE0`) while carrying |

Channel layout (`007DE7E0`, `007DE9A0`): +0 enabled, +1 target, +4 value, +8 moving, +0Ch rate, +10h/+14h
the blend ends, +18h the start time on the game clock `00F876A4`. The block drives a model animation frame
(`animFrame`, `007DEA60`): the channels are animated parts.

**What the host feeds.** Nothing: the host carries no `+DECh` block and answers true
(`src/game_hosts_units.cpp`, the `007B8D40` comment in the elevator site). **Whether it changes a decision.** Yes
in the image whenever channel C is enabled for the class: a plane that took off from a carrier has channel C at
1.0, and after landing the elevator cannot take it until the retract started at `007CC18E` has run the value
back to 0.0. So the image adds the retract time to every carrier turnaround. Binding it needs what this check did
not establish: where the channel's enabled byte and rate come from (the property bag / model bind), and where
the value +4h is committed when a move completes (`007DE7E0` only computes the blend). Not bound; follow-up:
read the `+DECh` block's construction and its per-class enable for the carrier aircraft.

## (b) 007B95E0 counts ACTIVE wingmen (`kPlaneLeaderTurnRollBound`, OFF)

`007B95E0` is true for `+9D8h == 0` (the flight leader), a squadron at `+9D4h`, and `squadron+3BCh > 0`.
`007EE790` writes `+3BCh` as the number of members 1..count-1 whose `+9C4h[[00F876B8]*8]` is non-negative,
which is `007BCC20`'s published wait value: non-negative only while the member's first task answers
`vtable[4Ch]` with `009BE3E0` (a follow or prepare state). The host never asked: `max_bank`
(`src/plane_ai_control.cpp`, `0099DFB2`) used TurnRoll for every aircraft (CC13_AUDIT_PLANNER_ROLL.md finding 3).

Binding: `GameUnitsHost::Impl::plane_is_leader_with_wingmen_007b95e0` counts the following wingmen with the same
`member_follows_007bcc20` test the moveto speed's `007EF2C0` already used (factored out, unchanged), and the roll
scale takes TurnRollLeader (`class+260h`) for a leader with at least one. SUBSTITUTION, labelled: the image
recounts on `007EE790`'s countdown and reads the previous buffer index; the host counts at the call.

**Predictions, before the pair** (USN01 `p3_u1_z4`, a plane-heavy win; USN04 has no win row): a leader's bank cap
falls from TurnRollLimitLarge (0.977) toward TurnRollLeader (0.873 for the classes CC13_AUDIT_PLANNER_ROLL.md
read) while wingmen follow; `summary mission leader turn roll: uses` non-zero ON only. Leaders turn wider, so
torpedo and dive releases may move; verdict 3 likely, the win should hold.

**The pair** (OFF = tree build at `8b0e7bfce`, ON = its flip; `local\p4l_{off,on}_{u1z4,u12}.log`): the mechanism engages
(`leader turn roll: uses` 21546 on USN01, 32243 on USN12, 0 OFF). USN01 `p3_u1_z4`: OFF completed 1014.63 s with the
guard; ON **no completion** (deaths 90 -> 68, hits 3965 -> 2690, torpedo-task releases 6 of 13 -> 5 of 10). USN12
`s38_u12_p2`: INVALID, both sides ran without orders (the file's tree was removed; `helm orders refused`). Faithful, and it loses the USN01 plan: **kept OFF** for the lead's decision (flip with a
re-plan, as for gunnery38's O1).

## (a), the follow-up read: channel C is the wings, and the delay is the fold

- **Per-class enable and rate.** `007EABC0` `BSP_PlaneActuatorBlock_Construct` (`007EAD70`-`007EADB6`), inside its
  `model+130h != 0` (animated model) branch: channel C's enabled byte `+44h` = class `+5E4h` (the PartAnims.Wings
  present flag), target 0, value 0, rate `+50h = 1 / class+5E0h` (`FLD1` / `FDIV`), blend ends `+54h = +5DCh`,
  `+58h = +5D8h`. Class `+5D8h..+5E4h` is `PartAnims.Wings` (`007D39AE`, docs/PLANE_CLASS_FIELDS.md): `[1]`, `[2]`,
  duration `|[1] - [2]|` or `[3]`. Channel B is Gears (`+5C8h`), the third BayDoor (`+5E8h`).
- **This installation** (vehicleclasses.lua, mtime 2026-05-09): ten Wings records. Durations: Avenger and
  Avenger Tiny Tim 3.0 s, Helldiver and AD-2 2.5, Wildcat 3.3, Kate 3.0, B6N Tenzan 2.8, SB2U Vindicator 3.83,
  B7A Ryusei 4.0. Every other class has channel C disabled, so `007B8D40` is true for it, as the host answers.
- **The value commit.** `007DE1E0` `BSP_PlaneActuatorChannel_Step`: `value += rate * (target ? +1 : -1) * dt`,
  clamped to [0, 1], moving byte cleared when the value sits at the target's end. `007DE3A0` steps channel C every
  call (the gear only once the wings sit at 1.0, the bay only once the gear sits at 0.0).
- **The target.** `007C11E0` `BSP_Plane_NotifyFlightState` sets it to `state in {4, 5, 6, 7}` on each state change
  (value set at once when its argument says so); `007CBFD2` extends it at the head of every ground-roll step and
  `007CC18E` retracts it when the plane is landed (`+904h`) and its holder surface still holds it; the takeoff
  step `009CF2F8` extends it; `006FC15E` (the elevator carry) zeroes the value. So 1 is spread, 0 folded.
- **The decision.** A carrier aircraft of those ten classes lands with its wings at 1.0; the fold starts at the
  first held ground-roll step and takes the class duration; the elevator (`006D06E8`) takes it only once the value
  is exactly 0.0 and its speed is under 1.389 m/s. The host takes it at the speed test alone. The fold overlaps the
  rollout, so the delay is the part of the 2.5-4.0 s fold still left when the plane stops.
- **Not bound yet:** the host reads no PartAnims field (the Lua reader stops at one nesting level,
  `read_vehicle_class_nested_number`), and `include/bsp/plane_class_fields.hpp`'s `part_anim_wings` reader is not wired.
  The binding needs `VehicleClass[i].PartAnims.Wings[1..3]` from the live state (lua lane), then a per-plane channel C
  value stepped in the plane's fixed step with the target rule above, and the elevator check `value == 0.0` when the
  class has Wings.
