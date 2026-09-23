# Ship natives, second batch: leader key, director arms, scan scale

Packet cc9_ship_natives_2, 2026-09-23. Base: main 4d05f45a6. Switches:
- `kAiLeaderOrderKeyBound` in src/game_hosts_ai.cpp
- `kDirectorCommandArmsBound` in src/game_hosts_commands.cpp
- `kApproachScanScaleBound` in src/game_hosts_ship_ai.cpp

All three are on. Status: reconstructed, build-tested and run-compared on USN04 and USN01. Not
ABI-compatible and not game-validated. Descriptive names are hypotheses.

Addresses: 009FFD70 BSP_Entity_AiClassWeight, 009FDF30 BSP_Ai_ClassWeightForClassId, 00A2E720
(00A2EB97, 00A2EBA2), 00A10D50 (00A10D8B, 00A10D96), 00836920 BSP_WeaponDirector_Step
(008369A0..00836A85, 00836ADC..00836B40, 00836D67..00836DC4, 00836E28..00836E32), 00465080,
009F1BC0 (009F2A44, 009F2AA9), 009E6E80 (009E71A5 scan).

## 1. The leader order key, 009FFD70

`MOV ECX,[ECX+0C4h]; JMP 009FDF30` (009FFD70..009FFD76). It loads the entity's class id and
tail-jumps into the class-weight switch, which FLDs a float weight from the AI tuning block, or
FLD1 (009FDFED) when the class has no row. The value is a float widened to x87 and returned in
ST0.

There are four callers, and each passes the first node of a group's +5640h list, the leader:
- **00A2EB97 / 00A2EBA2** (the compose pass's proximity merge). It stores the first key as a
  double, then `FCOMIP` and `JA 00A2EE13` skip the pair when key(A) > key(B).
- **00A10D8B / 00A10D96** (00A10D50, the merge leader test). It requires other >= own.

The host used the leader's unit index as a stand-in. It now returns `unit_class_weight(leader)`,
the same 009FDF30 lookup the member sort already uses, with 18h for a squadron.

## 2. The director's command arms, 00836920

The arms after the prepass, in the image's order:

| Site | Arm | Host before | Now |
|---|---|---|---|
| 008369A0..00836A7C | generic arrival (see below) | not run | bound |
| 00836A81..00836A85 | `CMP [ESI+48h],2; JE 00836DC9`: a finished stage skips every arm | not modelled | bound |
| 00836A8B..00836AD7 | `stop` | bound earlier | unchanged |
| 00836ADC..00836B40 | `follow` (see below) | record | bound through the existing `weapon_director_follow_arm_00836adc` |
| 00836B45..00836BEB | `attackmove` | bound by cc9_target_release | now also behind the stage skip |
| 00836BF0..00836D66 | `moveonpath` | bound by cc8_ship_moveonpath (the units host calls it) | unchanged |
| 00836D67..00836DC4 | override `attackmove` on director+188h / +18Ch, secondary stage through 0071D9E0 | not run | cannot fire: nothing in the host writes +188h, so it stays skipped |

- **Generic arrival.** With more than one command queued, the stage below 1, the head's
  category (vtable[0Ch]) not 1 or 2, and the last queued command's category 1 or 2, it
  resolves the last command's target (00836A07). It takes 00427EB0 x and z for the target and
  the unit. The squared distance is stored to float at 00836A64, and the head ends at stage 2
  (00836A7C) when the double 4000000.0 at 00D09FE8 is above it.
- **Follow.** The order ends at stage 2 (00836B3B) unless all of these hold: the unit has a
  group, its leader exists and is not the unit, the command's target is that leader, and only
  one slot is filled.

**A host bug the follow arm exposed.** The host's 00465080 always named the unit itself, so every
`follow` the idle tail issued (00836E28..00836E32 passes the leader) targeted its own follower.
00465080 now names the object it is passed, behind the same switch.

## 3. The scan scale, 009F2A44

nested+1284h is word 2 of the nested+127Ch block. 009F2A44 loads [target+370h], the target's
health, and 009F2AA9 stores 10000.0f (00CE3D64) when there is no target. The 119-step standoff
scan (009E71A5) divides it by each sample's own curve value.

The scale is one positive factor on every sample, so it cannot move the argmin except through
float rounding at near-ties or a zero scale. The host returned 10000.0 always. It now runs the
same `fill_target_block_127ch` that the own curve and the ring use.

## 4. Predictions and measurement

Predictions were written to local/n_predictions.txt before any result was read.
- Item 1: auto-merges stay 0 and merge counts may move; low confidence.
- Item 2: no behavioural change.
- Item 3: identical.

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides. USN04 ran at 4700/4500 and USN01 at
3200/3000. Every build uses the same source with the switches set per item.

| Run (log) | Hits | Damage | Deaths | Lexington | Standoff choices per escort |
|---|---|---|---|---|---|
| control (local/n_ctl_usn04.log) | 240 | 18379.1 | 26 | sunk 221.81 s | 153 |
| item 1 only (local/n_t1_usn04.log) | 255 | 19269.2 | 25 | afloat | 148 |
| item 2 only (local/n_t2_usn04.log) | 240 | 18379.1 | 26 | sunk 221.81 s | 153 |
| item 3 only (local/n_t3_usn04.log) | 240 | 18379.1 | 26 | sunk 221.81 s | 153 |
| all three (local/n_all2_usn04.log) | 255 | 19269.2 | 25 | afloat | 148 |

The all-three row matches item 1 only.

**Item 1.** Auto-merges stay 0 and proximity merges stay 12, which held. The merge direction
changed, however. Equal class weights now pass both orders where the unit index passed one.
Which group absorbs which fixes the member order among equal weights, so the Val group #3's
leader changes:
- control: `order_attack ... target_leader=D3A Val #3.1|.-2`
- treatment: `target_leader=D3A Val #3.1`

That is the first difference: the US group's `movetoattack` leader distance moves from 8289.1 to
8229.9, and tick orders move from 71 to 72. Everything downstream follows from the moved target
point, including the shorter pre-release standoff sequence (153 to 148 choices) and Lexington
surviving. My prediction did not foresee the leader change; the mechanism is traced term by
term above.

**Item 2.** Ship-AI rows, impacts and deaths are identical to the control, which held.
- Six Yorktown-group escorts (Northampton-class03 to 05 and Fletcher-class05 to 07) have their
  follow ended at fixed step 2. Each joined Yorktown's column (log line 1979) and was then moved
  into Lexington's (line 2281), so the follow no longer names its leader. The idle tail
  reissues follow on the new leader.
- On USN01, Dunlap and SaltLakeCity's follow now names Enterprise instead of themselves. Their
  ship-AI and gunnery rows are identical.
- The generic arrival never fired.

**Item 3.** The logs match line for line apart from the method table, which held.

**USN01 with all three** (local/n_ctl_usn01.log and local/n_all_usn01.log): 118 hits, 4467.9
damage and 5 deaths on both sides. Ship-AI, impact and death lines are identical.

**Unimplemented calls**, USN04 4500 frames, summed over the method table:

| Build | Unimplemented calls |
|---|---|
| control | 6,808,418 |
| all three | 6,161,959 |
| fall | 646,459 |

The fall is smaller than 0.87M because the scan-scale site now runs about 88,000 times, not
250,848. Since cc9_target_release the escorts stop scanning at their release.

Decisions: all three landed. Items 2 and 3 change no decision row. Item 1 changes one enemy
group's leader through the image's own merge order.

## 5. Open

- The override arm 00836D67 stays unreachable until something writes director+188h / +18Ch
  (00835C92).
- Lexington's fate now turns on an enemy group's leader identity. Its sinking in the release and
  retask packets and its survival here are both inside a sensitive regime.
