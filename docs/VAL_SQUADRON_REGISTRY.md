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
