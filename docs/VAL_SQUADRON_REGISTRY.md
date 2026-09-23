# USN04's plane squadrons, and what happens to a squadron when its leader dies

Packet `cc9_val_squadron_registry`. Addresses: 0094C480 (SpawnNew), 007F4580, 007B8AD0, 007F3970,
007BCAA0, 00926390, 009273A0, 007F3BA0, 007ED610. Every name is a hypothesis, not a recovered
symbol. Ghidra was read only; nothing was written to it. 007BCAA0 and 007F3970 are under another
worker's lease, so they are cited here, not annotated.

## 1. The premise was wrong: the Vals and Kates are in the registry

The brief came from my own note in `docs/RELEASE_COLLAPSE.md` section 2. That note took the comment
on `unit_is_flight_leader_007b8ad0` ("USN04 registers one squadron of three") as current. It is
stale. `local\L0_9000.log` shows every scripted squadron registered:

```
SpawnNew 0094c480: serial 1 party 1, 2 group member(s), callback "luaBombersSpawnedLex", ...
GenerateObject squadron D3A Val #1.1: WingCount=4 -> 4 member plane(s) (007F4580 mode 1 on the held-back row)
GenerateObject squadron A6M Zero #1.2: WingCount=2 -> 2 member plane(s) ...
plane squadron members: 9 of 9 wing record(s) resolved to units over 3 squadron(s)
```

1. The mission script calls `SpawnNew` (0094C480) four times at about 25 s. Each request has two
   members: a D3A or B5N squadron and an A6M Zero pair.
2. The spawn queue creates each member through
   `GameScriptOrdersHost::create_unit_from_scene_record_0046db4b`. That runs 007F4580's member plan
   for class 18h and adds a registry record with all `WingCount` members.
3. The image's form is the same. 0094C480 allocates the 0x414-byte squadron and constructs it with
   007F2C60 (00948563). 007F4580 attaches the wings to +3D0h, with the leader in slot 0.

The flight-leader test already answers as the image does:
- Every leader constructs its attack task into moveto.
- Every wingman constructs into follow and hands over to its own attack run on the first arm tick
  (`divebomb ... hand-overs: moveto>attackrun@0` for #3.1 and `follow>attackrun@0` for #3.1|.-2).

So there was no membership to bind.

## 2. The divergence that is real: nothing leaves the squadron at death

**The image.**
- At zero health, 00958A30 calls vtable[70h](1), which queues the entity on the destroy list.
- The next flush, 009273A0, dispatches vtable[74h] = 00926390. It sets +5Dh = +60h = 1, then
  tail-jumps to vtable[7Ch] (009263A8-009263AE).
- For an aircraft, vtable 00D19D28 slot +7Ch is 007BCAA0. Its 007BCAEB calls
  `007F3970 BSP_Squadron_RemovePlane(squadron, plane, 0)` whenever plane+9D4h is set.
- 007F3970 compacts +3D0h. The next member becomes slot 0, the flight leader that 007B8AD0 and
  007EDA91 read.
- So a plane leaves its squadron **at its death**, not at its removal. A power-lost glider is
  already out of the array.
- 007F3BA0's member loop has a second removal path, at 007F3CE7, for members whose +900h is 1.
  That path is not involved here.

**The host.** `PlaneSquadronHostRecord::member_units` is never shrunk. A dead member keeps its
slot, and a dead leader stays the leader its wingmen are measured against. Measured in L0:
- The Lexington fighter leader died at 395.23 s.
- Both wingmen then switched from aim to follow (`dogfight Lexington-class01_sqn01|.-2 aim ->
  follow`) and followed the dead leader until they died at 399.53 s and 409.32 s.

**Where it costs releases.** Two consumers read the leader answer after the attack starts:
- the torpedo task's leader-only self-engage gate, 009D323A in 009D3210;
- the dive chooser's follow-target test, 009C841F.

A Kate wingman engages only when its squadron's mode reaches 2 or when it is the leader. In
`local\L1b_9000.log` (main's reference, fighter lead on):

| squadron | leader death | wingmen | their releases |
| --- | --- | --- | --- |
| B5N Kate #2.1 | 202.16 s, before releasing | alive until 246.9-250.0 s | 0 |
| B5N Kate #6.1 | 282.05 s, before releasing | alive until 327.5-331.5 s | 0 |
| B5N Kate #4.1 | 215.06 s, after releasing at 162.50 s | released at 193.5-197.1 s | 3 |
| B5N Kate #8.1 | 338.49 s, after releasing at 253.40 s | released at 290.7-294.0 s | 3 |

The two squadrons that lost their leader before release never released at all.

**Correction, after the first pair (section 5): the leader gate is not why.** Every Kate of #2.1 and
#6.1 has aim ticks in both runs, leaders included: 170-290 aim ticks, closing to 496-880 m
(`torpedo ... approach 009D3420` census). The leaders fail to release as well. So those two
squadrons fail at the torpedo release condition, not at the leader-only engage gate. That term is
outside this packet and is listed as open in section 5.

## 3. The binding

`kPlaneSquadronLeaveOnDeathBound`, in `include/bsp/plane_squadron_host.hpp`:
- **The compaction.** `PlaneSquadronHostRecord::remove_member_unit_007f3970` removes the member from
  every parallel array, names included, so a later resolve by name cannot restore it. It also
  re-runs the formation indices, as 007F3A11 does through 007ED260.
- **The sweep.** `squadron_leave_on_death_007bcaa0` runs at the head of each motion step. It removes
  a member once the host has recorded its death: a death mode chosen at 007CA8A0, a removal, or the
  gunnery host's dead flag. It logs `plane squadron leave` with the old and new leader.
- **The AI coordinator's squadrons** (`src/game_hosts_ai.cpp`) read slot 0 as the first member
  still listed in the registry, and an emptied squadron as having no lead plane (007EDA99).

**Substitution, labelled:** the host has no destroy flush. The death is read where the host records
it, and the leave happens at the next motion step's head, which is the same frame boundary as the
image's flush.

007ED610 (PromoteFlightLeader) is the explicit promotion that rotates a chosen member to the front.
Its callers were not needed here: the compaction alone makes the next member the leader.

## 4. Predictions, written before the pair

Pair: switch off (V0, `local\vs0`) against switch on (V1, `local\vs1`), E2 9000 parameters,
`BSP_GUNNERY_RNG_STREAMS=1` on both sides. V0 should reproduce L1b.

| row | V0 (= L1b) | prediction for V1 |
| --- | --- | --- |
| leader deaths that promote a live wingman | 0 | 8-10 (Val #1.1, #3.1, #5.1, movieval, Kate #2.1, #4.1, #6.1, #8.1, and any fighter leader) |
| torpedo drops | 8 | 10-14: the #2.1 and/or #6.1 wings now engage under a promoted leader |
| Kate #2.1 / #6.1 wing releases | 0 / 0 | at least one of the two squadrons releases |
| dive-bomb releases (aircraft / bombs) | 2 / 4 | 2-4 / 4-8. Val wingmen are already in their own attack runs, so there is little change |
| Val and Kate death rows | L1b | the first wave unchanged; later rows move with the Kate paths |
| fighter hits | 66 | 50-80. A fighter leader's death promotes; in L1b no fighter leader died |
| Lexington | alive | more torpedo damage. Sinking is possible if 3 or more extra torpedoes run at it |

## 5. The pair (E2 9000, `BSP_GUNNERY_RNG_STREAMS=1` on both sides)

Logs: `local\V0_9000.log` (switch off, `local\vs0`) and `local\V1b_9000.log` (switch on,
`local\vs1b`). Both come from this tree, main a9db935f0 plus this packet.

**V0 does not reproduce L1b.** Main moved again after the fighter-lead pair. V0 is the reference:
2 bombs from 1 aircraft, 8 torpedoes, fighter hits 73, and the Lexington alive.

**A first treatment run was discarded.** In `local\V1_9000.log`, fighter bursts fell from 21 to 5.
The dogfight task's approach update (009AAC70) and early-edge test (009AAA80) find the target
squadron through the ordered unit, a labelled stand-in for the squadron pointer the image's task
holds. Once that plane died and left, the fighters lost the whole squadron. The registry now keeps
the members that left at death (`departed_units`), and those two lookups use
`find_by_member_or_departed_unit`. The switch-off path is unchanged, so V0 stands.

| row | V0 (off) | V1b (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| members leaving at death | 0 | 32 | - | - |
| leader deaths / promoting a live wingman | 0 / 0 | 21 / 15 | 8-10 promotions | more: second and third promotions within a squadron count too |
| torpedo drops | 8 | 8 | 10-14 | **missed**: the premise was wrong (section 2 correction) |
| dive releases (aircraft / bombs) | 1 / 2 | 2 / 4 (adds Val #7.1\|.-2) | 2-4 / 4-8 | held |
| fighter bursts / hits | 21 / 73 | 16 / 50 | hits 50-80 | held at the low edge |
| deaths: Val / Kate / US fighter | 14 / 16 / 1 | 12 / 14 / 3 | - | - |
| Lexington | alive | alive | more torpedo damage | held: alive |

**What moved, per aircraft.**
- **Val #5.1's wave lives longer:** deaths at 227-238 s become 238-252 s.
- **Val #7.1\|.-2 now releases** at 365.80 s from 203 m and survives. In V0 it died at 339.64 s.
- **Val #3.1\|.-2 survives,** where it died at 316.25 s in V0.
- **The Lexington fighter flight loses two aircraft,** at 396.43 s and 432.12 s. Both survived V0.
  The cause was not traced.

The fighter hit total fell. The dogfight target and the fire picture move as soon as the first
leader leaves at 100.35 s.

**Open, outside this packet:** Kate squadrons #2.1 and #6.1 fly the torpedo aim and close to 500-880
m without releasing, leaders and wingmen alike, in both runs. Their release condition is the next
term to read.

**Switch state landed: `kPlaneSquadronLeaveOnDeathBound` ON.** It is the image's compaction at
death, with one labelled substitution: the leave is read at the next motion step's head.
## 6. Secondary: kPlannerRangeInterpBound ON, predictions written before its pair

The pair is the landed state of this packet with the range factor off (R0, the `local\vs1b` binary,
log `local\V1b_9000.log`) against the same tree with it on (R1, `local\rg1`, log
`local\R1_9000.log`). The predictions come from K0/K1 (`docs/PLANNER_KATE_TARGETING.md`, and this
lineage's `docs/FIGHTER_GUN_LEAD.md` section 5):
- **Both fighter flights are ordered onto Val #1.1's group first, not #3.1's.** In K1 both flights
  took #1.1.
- **The Yorktown flight's early kills of Val #3.1 and #3.1|.-3, at about 100 s, disappear.** Those
  kills came from fighting #3.1. The #3.1 wave then keeps its leader to its dive: releasing
  aircraft rise by 1-4.
- **Fighter bursts and trigger ticks move,** direction uncertain. In K1 they fell to 4 / 107 with
  the lead off.
- **Ship goal replans rise to 2-4x R0's count.** K0 to K1 was 258 to 981. Command-target units
  rise by a few.
- **Torpedo drops and the Lexington's fate are unchanged,** since no fighter is sent onto the Kates.

### 6.1 The range-factor pair, measured

R0 is `local\V1b_9000.log` (this packet's landed state). R1 is `local\R1_9000.log`, the same tree
with `kPlannerRangeInterpBound` ON.

| row | R0 | R1 | prediction | verdict |
| --- | --- | --- | --- | --- |
| fighter group's first order | Val #3.1's group | Val #1.1's group | #1.1 first | held |
| fighter group's second order | Val #1.1's group (12 members) | Zuiho-class01's group (3 members) | - | - |
| fighter bursts / hits | 16 / 50 | 8 / 19 | move | - |
| torpedo drops | 8 | **0** | unchanged | **missed** |
| Kate deaths | 14 | 16 | - | - |
| dive releases (aircraft / bombs) | 2 / 4 | 2 / 4 | up 1-4 | missed: flat |
| ship goal replans | 212 | 42 | 2-4x up | **missed**: down 5x |
| command-target units | 56 | 47 | up | missed: down |
| Lexington | alive | alive | unchanged | held |

**The first divergence is again the fighter group's planner order** (Val #1.1 instead of #3.1).
Nothing differs before it.

**The torpedo loss is Yorktown's movement.**
- In R0, Yorktown-class01 leaves its script path at ship-AI step 2440 (122 s) for a movetopos 24 km
  away.
- In R1 it stays on its path, 3.3-3.8 km from its path target through step 3200.
- Its anti-aircraft fire then downs Kate #4.1 at 158.31 s, before that Kate's release. R0's
  killer column says Northampton-class03, at 215.21 s.
- All four #4.1 Kates and all four #8.1 Kates die before releasing. Their only census rows fall
  after death and are refused.

Which order moves Yorktown at 122 s in R0, and why R1 lacks it, is not traced. R0's 122 s block
issues the escort group's attackmove onto Val #1.1. Yorktown is not in that block.

**Recommendation.** The flip is the image's range law, so it is committed separately as asked. But
it takes torpedo drops from 8 to 0 through a ship order that is not yet explained. Hold it off
main, or drop the commit, until Yorktown's 122 s order is traced.

## 7. Reconciled with main f00ccfebf (kSquadronRemovesDeadBound)

Main landed its own death leave from `cc9-gunnery-host` (`docs/PILOT_SURFACE_CLIMBOUT.md` section
3). In the plane-branch hunk, at the death step, it sets the dead unit's slot to
`kPlaneSquadronNoUnit` and compacts the station flags. It keeps the member's name.
This packet's section 3 binding did the same compaction by erasing the slot. Running both would
compact the station flags twice. So this switch is narrowed to what main's leaves open:

- **Permanence.** The script-orders pass `resolve_plane_squadron_members` refills every slot BY
  NAME whenever the unit count moves. In V0 that happened at 105.0 s, 106.5 s, 426.1 s and
  447.05 s, when new waves spawned. With main's version alone, the pass at 105 s re-seats dead Val
  #3.1 (died 100.3 s) as its squadron's leader. The pass at 426 s re-seats every plane that died
  before it.
  - `remove_member_unit_007f3970` now also finds the slot main already cleared, by name, and
    erases the slot with its name.
  - It touches the station flags and formation indices only when main's switch is off.
- **The departed record** for the dogfight order's target squadron (section 5).
- **The AI host reads slot 0 from the live array.**

Main's `squadron remove plane` log line carries the leader change. The `plane squadron leave` line
here is the completion, so its `promotions` count stays near 0 on this tree.

**Predictions for the merged-tree pair.** Both binaries have the range factor OFF, so the pair
measures only this switch against main. M0 (`local\mr0`) has this switch off; M1 (`local\mr1`) has
it on.
- M0 should reproduce main's E2 reference in `docs/GAME_EXECUTABLE.md`: 8 bombs, 8 torpedoes,
  Lexington survives.
- **Before 105 s the runs are identical.**
- **From 105 s, the Val #3.1 squadron differs.** In M0 its dead leader is back in slot 0. Its
  wingmen are already in their own attack runs, so the release rows for #3.1 change little: bombs
  8 ± 2.
- **Torpedo drops stay 8.**
- **After 426 s the two runs can differ more,** because every dead plane is re-seated in M0. Few
  aircraft still fly then, so the totals change little.
- **Fighter hits move by ±25%.** They are path-coupled.
- **The Lexington survives in both.**

## 8. Where the leave meets the station placement (for the follow-law packet, not this one)

The host still stands in for the plane follow law 009BFEE0/009BEE30 by PLACING a wing member on its
station every tick. It happens at three sites:
- the torpedo follow tick at 009C1FD0 (`follow_base_tick_009c1fd0`);
- the dive-bomb follow tick, where placement stays on beside the law;
- the dive-bomb done tick at 009C1FEA-009C2077 (`run_dive_bomb_done_prepare_tick_009c7270`),
  which places every wing member after its release (the `placed=` count in the `divebomb ... done`
  census).

The station is computed from `wing.front()` in `place_wing_member_on_station_007f23a0`, so it
follows the live slot 0.

When a leader dies:
- Main's clear and this packet's erase make the next member slot 0.
- The formation indices are re-dealt on the next placement.
- Every member still in one of those states is written onto the new leader's station in ONE tick.

The image flies there through the law. The first measurable case in E2 is a done-state Val
whose leader dies after the release: the member jumps by the distance between the old and the new
leader's stations. Main's seeding compaction prevents the first-step re-seed. It does not prevent
this per-tick placement.

The fix belongs to the follow-law packet: wire 009BFEE0/009BEE30 into the done and torpedo follow
ticks (`kPlaneFollowLawEnabled` already does it for the dive follow tick), and drop the placement.

## 9. The merged-tree pair, measured

Logs: `local\M0_9000.log` (this switch off) and `local\M1_9000.log` (on). Both are main f00ccfebf
plus this packet with the range factor OFF, E2 9000, `BSP_GUNNERY_RNG_STREAMS=1` on both sides.

**M0 is not directly comparable with main's reference row.** That row in `docs/GAME_EXECUTABLE.md`
(10 bombs, 8 torpedoes) was taken without the stream option. M0 has 4 bombs, 8 torpedoes, damage
9216.2 against the row's 9585.8, and the Lexington alive.

| row | M0 | M1 | prediction | verdict |
| --- | --- | --- | --- | --- |
| identical before 105 s | - | no: Val #3.1\|.-3 dies at 100.80 s in M0, 102.80 s in M1 | identical | **missed** |
| bomb drops (aircraft) | 4 (#3.1\|.-2, #3.1\|.-4) | 4 (#3.1\|.-4, #7.1\|.-2) | 8 ± 2 | missed: M0 is 4, and M1 matches it |
| torpedo drops | 8 | 8 | 8 | held |
| fighter bursts / hits | 5 / 32 | 10 / 59 | hits ±25% | **missed**: +84% |
| deaths (all) | 34 | 35 | - | - |
| Lexington | alive | alive | alive | held |
| `plane squadron leave` completions | 0 | 35 (main's own removal lines: 34 and 33) | - | - |

**Why the runs part at 100.3 s, not 105 s.** The departed record takes effect as soon as Val #3.1
dies. In M0 the fighters' target-squadron lookup finds nothing once main has cleared the ordered
plane's slot. In M1 it still reaches the squadron. So the fighter rows move first, and fighter hits
rise, before the resolver re-seat I predicted around. That is the section 5 artefact, now shown on
main's own removal.

**Switch state: `kPlaneSquadronLeaveOnDeathBound` ON,** narrowed as section 7 says.
