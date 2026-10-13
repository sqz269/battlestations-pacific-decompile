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

## (b) 007B95E0 counts ACTIVE wingmen (`kPlaneLeaderTurnRollBound`, ON by ruling (a))

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

**Flipped ON (cc9-physics5, 2026-10-13)** by the lead's ruling (a): the mechanism is faithful and engages, so it flips by mechanism; the USN01 `p3_u1_z4` loss is covered by the USN01 re-plan on current main (docs/WINS_TABLE.md). Code only, no new run.

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

## (a), the binding (cc9-physics5, 2026-10-13): `kPlaneWingFoldElevatorBound` and `kPlaneWingSpreadTakeoffBound`, ON since the JM08 pair (cc9-physics6)

**Census.** A rel32 scan of the image (`local\p5_rel32_scan.py`) finds exactly the seven calls the Ghidra xrefs
list: `007B8D40` from `006D06E8`; `007B8D10` from `009CDEED` and `009CF2ED`; `007B8DC0` from `009CF2F8` and
`0099AE52`; `007B8DA0` from `007CC18E`; `007B8DE0` from `006FC15E`. The inline writers of `+45h` are `007C11E0`
and `007CBFA0`'s two extend blocks (`007CBFD2`, `007CC195`). `0099AE52` is the player-held pilot's and is not bound.

**The site's hold test.** `007CC16C`-`007CC186` call `(+BF4h)->+4h->+3Ch` `vtable[38h]`. The carrier elevator site's
vtable is `00CF8A58` (slot +4h `006D0600`; constructors `006CFAF0`, `006D0470`); its slot +38h is `006CFFC0`:
`20.0f` (`00CE3930`) `> 006CFE90` (the nose's x/z distance to the lift), `FCOMI`/`JBE` so a NaN is false. The airfield
launch site's vtable `00CF89F8` has `006CEBF0` there: `XOR AL,AL; RET 4`, never held. So wings fold only on a
carrier deck, within 20 m of the lift.

**The takeoff consumers.** `009CDEED` (`BSP_TakeoffStatePrepare_Step`): wings not at 1.0 jump to `009CE106` and return
before the prepare clock `+1Ch` advances. `009CF2ED` (`BSP_TakeoffStateTakeoff_Step`): wings not at 1.0 extend
(`009CF2F8`) and join the not-aligned path `009CF2FD`; the aligned arm (`009CF34C`) runs only with the wings at 1.0.

**The block step order** (`007DE3A0`, raw listing): `+44h` steps every call, `+28h` only when `+44h` is disabled or at 1.0,
`+60h` only when `+28h` is disabled or at 0.0. `src/torpedo_release_spawn.cpp` guards each channel by its own
value; with the gear (`+28h`) never enabled in this host the two orders step the same channels. Routed to the lead.

**Binding** (`src/game_hosts_units.cpp`, packet `cc9_wing_fold`). `kPlaneWingFoldElevatorBound`: channel `+44h` (the
host's `channel_a`) enabled per class from `VehicleClass[i].PartAnims.Wings` (`read_vehicle_class_part_anim`, rate
1/duration, value 0); a 007C11E0 stand-in sets the target on each `+900h` change (state 4..7 spread; the first state
seen sets the value at once, as the spawn chooser's argument 1 does); the ground roll extends off the deck and folds
on a held carrier deck; the elevator carry zeroes it; the elevator takes a landed plane only at value 0.0.
`kPlaneWingSpreadTakeoffBound` (needs the first): the prepare hold and the run's extend and aligned gate.
SUBSTITUTIONS, labelled: the notify is detected once per fixed step before the block step rather than at the 19
call sites; every plane is taken as having an animated model; a carrier deck whose elevator this host has not built
answers not-held. Summary line: `summary mission wing fold`.

**Predictions, before the pair** (carrier-heavy: USN01 `p3_u1_z4`, LOMP10 `p3_l10_b`). OFF: `classes=0`, every count
0. ON (elevator half): `classes` = the Wings-class planes spawned (Avenger, Helldiver, Wildcat, Kate, Tenzan,
Vindicator, Ryusei in this installation); retracts about one per carrier landing; `elevator_waits` non-zero, each
landed Wings plane reaching the lift up to its 2.5-4.0 s fold later than OFF; `resets` near 0 (the carry only counts a
value that was not already 0.0, and the elevator takes only folded planes). Recovery and restrike times move by seconds, so verdict 3; the wins should
hold unless a restrike's timing matters. ON (takeoff half, on top): `prep_holds` non-zero only for a plane whose
wings are below 1.0 at prepare (a relaunched plane after the carry's reset spreads over its duration first), so
relaunches move later by up to 4 s; deck spawns in state 6/7 start at 1.0 and never hold.

## The block step order: `kActuatorBlockStepOrderBound` (cc9-physics5, 2026-10-13), OFF

The wing pair found that the host's `plane_actuator_block_step_007de3a0` stepped a channel only when it was disabled
or at exactly 1.0f. So an enabled wing channel starting at 0 never moved: with both wing switches ON, every
Enterprise Wildcat waited forever in the takeoff prepare (2774 holds, no takeoff run). The torpedo bay also stalled
one step below 1.0f once it began to close. The switch takes the raw listing's gates (header comment,
`include/bsp/torpedo_release_spawn.hpp`). **Predictions, before the pair:** with the wing switches OFF, the
`+44h` and `+28h` channels are disabled, so `+60h` (the bay) steps every call and now closes fully after a
release. Nothing in the host reads the bay below 1.0f except `007BBBA0`'s `!= 1.0f` guard, which a stalled bay and a
closed bay both pass. So: gameplay identical (verdict 0 or 1) on the torpedo rows (USN01 plan d, LOMP10, ESMP08). If
so, the switch can flip ON and the wing pairs are re-run on top of it.

**The pair** (OFF = tree build at `1f83f2ffa`/`c01d9516e`, ON = `local\p5_so`, `local\p5so_{off,on}_*.log`): USN01 plan d 22000 exit 1, LOMP10 `p3_l10_b` exit 1 (completed 1158.10 s both), ESMP08 30000 exit 0. As predicted: gameplay identical. **Flipped ON** (cc9-physics5, 2026-10-13).


**The pair on a carrier-launch row** (cc9-physics6, 2026-10-13; tree at origin/main 168cb72a9, block step order ON;
OFF = tree build SHA-256 69A028621EE7, elevator half = `local\p5_wf` 04EAF0FA58D9, both = `local\p5_wt` 80C6922E93C9;
JM08 `jm08_s45_j8_m10`, 72000 frames, reference env; logs `local\p6wf_{off,el,on}_j8.log`). The earlier pairs (USN01,
LOMP10, ESMP08 without orders) had no Wings plane taking off from a carrier; JM08's Hosho launches Kates (class 162,
`PartAnims.Wings {5.2, 8.2}`, 3.0 s).
- Elevator half alone: gameplay identical (completed 3528.42 s both, 273 deaths), `classes=36`, every other count 0
  (no Wings plane lands on a carrier in this row).
- Both: GAMEPLAY MOVED, `classes=24 prep_holds=390`, the rest 0. The first divergence is the takeoff prepare: Hosho_sqn07
  leaves prepare at 310.30 s instead of 307.30 s, exactly the class duration. **Prediction corrected:** the Hosho
  squadrons start inside the carrier in state 1 (`base launch inside start`), so their wings start folded (0.0) and the
  first launch holds too, not only a relaunch. 009CDEED's hold is the image's; the delay is the mechanism, not a defect.
  The later Hosho strikes arrive seconds late, the battle re-rolls (deaths 273 -> 258, dive-bomb releases 47/75 ->
  27/99) and JM08 does not complete in 72000 frames.
- **Flipped ON, both switches** (cc9-physics6, 2026-10-13): mechanism matches, so the lost win goes to re-plan (JM08
  is a knife-edge already: it fails under `BSP_FRAME_JITTER 20,3`).
- ESMP08 WITH orders (`g38_ord_e8.txt`, 90000 frames; OFF = `local\p6_wo` F1AC46BB9E55, ON = tree at ae34e5462;
  `local\p6wf_{off,on}_e8.log`): `classes=714 prep_holds=3948`, the rest 0; GAMEPLAY MOVED (first hit 374.68 ->
  376.78 s, torpedo drops 68 -> 36, deaths 911 -> 903); no completion either side (OFF: Dunlap 1892.62 s and Fanning
  2757.92 s sink; ON: Dunlap only, 2729.99 s). The ESMP08 re-plan starts from the ON run.