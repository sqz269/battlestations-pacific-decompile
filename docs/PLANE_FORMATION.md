# The plane squadron's formation: how a wing member gets its own station

Addresses: 007F23A0, 007ED260, 009BFD70, 009BFEE0, 009C1FD0, 009BED80, 007F2920, 007D01C3,
007D68E4, 007D690D, 007F50CF, 007F50FB, 0077C8D0, 0077F940, 0070D290, 009483D0, 00A10DC0,
00D049A8, 00D7A208, 00D05DC4, 00D08AA0, 007F2900.

Packet `cc8_formation`, from the listing. Every descriptive name here is a hypothesis, not a
recovered symbol.

This document exists because `docs/TORPEDO_AFTER_THE_DROP.md` section 15.4 named the wrong routine.
It said the missing formation offset was `0077C8D0 BSP_Entity_RequestJoinFormation` and that the
spacing was `Formation_UnitDist = 300.0`. **Both are ship machinery and neither can reach a plane.**
Section 1 is that correction; sections 2 to 5 are the machine that actually does it.

## 1. `0077C8D0` is a ship path, and a plane is never in it

`0077C8D0` is `__thiscall(entity)(entity other)`, `RET 4`, body `0077C8D0-0077C97B`. From the
listing: `0077C8F0-0077C8FE` calls `this->vtable[16Ch]` with the literal `00CFB52C` (`"follow"`) and
the other entity; on a true answer it builds session message type `76h`
(`0077C920 PUSH 76h` into `0075B430`), stamps `word [other+174h]` into it at `0077C92B/0077C951`,
and routes it with `0077C964 CALL 0077C2A0`. The receiving side is `0077FE80
BSP_Session_DispatchEntityKindMessage`, which reaches `0077F940 BSP_UnitGroup_JoinOrMerge`, and that
builds the `508h`-byte unit group at `entity+284h` through `0070DB20` and `0070EF30`. That group is
the object `docs/SHIP_AI_FORMATION.md` reads. Its station producer `0070D290
BSP_UnitGroup_StationPointFor` has exactly **one** caller, `009DF2D0
BSP_ShipAi_FollowUpdateFormationPoint`, and the station it produces is measured back along the
leader's **wake**.

Neither of the two call sites that could enrol a spawned member is open to a plane:

| Call site | In | Gate | Meaning |
| --- | --- | --- | --- |
| `0094870E` | `009483D0 BSP_SpawnRequest_CreateMembers` | `class->vtable[18h](6)` at `009486CB`, `TEST AL,AL` / `JE 00948713` at `009486DC` | class family 6, the nine ship classes |
| `00A10E67` | `00A10DC0 BSP_AiCommand_FollowerPass` | `vtable[5Ch](6)` (`docs/AI_COMMAND_TICK.md` line 71) | the same family |

`docs/LUA_SPAWN_NEW_HOST.md` line 268 already recorded the first of those as "not reached, because
no member takes the surface arm". A plane's class chain is `11 0F 05 04` for a TorpedoBomber and
`10 0F 05 04` for a LevelBomber (`docs/ATTACK_GATE_TAILS.md`); there is no `06` in any of them.

`Formation_UnitDist` (`+210h` of the AI tuning record, default `300.0` at `00CE3AE8`) belongs to the
AI command layer's own formation, `00A11690`, whose only callers are the `CAUTIOUSMOVE`,
`CAUTIOUSATTACK` and `DEFENDPOSITION` ticks. It is not the plane wing's spacing. (`00CE3AE8` is the
pooled float `300.0` and has twenty-odd readers across the image; an xref census on it proves
nothing about formations.)

## 2. The fields the plane machine runs on

| Field | Name | Evidence |
| --- | --- | --- |
| `plane+9D0h` | `formationIndex` | the property-bag pair `{1, &[ESI+9D0h]}` at `007D68E4`/`007D68EA` and `{0, 00D05DC4 "formationIndex"}` before `007D690D CALL 00BD6830` |
| `plane+9D8h` | the member's array slot | `007F4B43` stamps the current `+3CCh`; `007ED292` rewrites it |
| `squadron+3E4h` | `psFormation`, the SHAPE | the pair at `007F50CF`/`007F50FB` with `00D08AA0 "psFormation"`; `007F2C60` seeds `1` |
| `squadron+3E8h` | morale | `007F2C60` seeds `1.0f`; `009BFDA6-009BFDB2` republishes it from `task+8Ch` every follow tick |
| `squadron+35Ch` | the plane class descriptor | `docs/PLANE_SQUADRON.md` line 77 |

`BSP_PlaneUnitInstance_Construct` zeroes `+9D0h` at `007D01C3` (the same `EBX` that zeroes `+C02h`,
`+BF8h` and the rest of the block), and the attach `007F4B43` never writes it. **So every member of
a freshly spawned squadron carries formation index 0, which is the leader's own station, until
`007ED260` runs.** That one fact is the whole defect: three planes asking for station 0.

`docs/PLANE_SQUADRON.md` lines 98 and 319 list `+3E4h` as written once by the constructor with "no
reader found in range". It has one: section 4.

## 3. `007ED260`, the index assignment

`__thiscall(squadron)`, `RET`, body `007ED260-007ED374`, read complete. Two walks over the `+3CCh`
live members of `+3D0h`, with a five-entry table on the stack whose three values are 0 (never seen),
1 (a member is carrying that index now) and -1 (handed out during this pass). `007ED2C7 PUSH EDI`
shifts the frame by four between the walks, which is why the first walk writes `[ESP+ESI*4+0Ch]` and
the second reads `[ESP+EAX*4+10h]`: the same slot.

* First walk, `007ED290-007ED2B4`: `member+9D8h = i` (`007ED292`), then `taken[member+9D0h] = 1`.
* Second walk, `007ED2C8-007ED367`: slot 0 takes index 0 (`007ED2D2`) and marks `taken[0] = -1`.
  For every other slot it takes the lowest free **odd** index from 1 (`007ED2E1`/`007ED2FB`,
  `007ED300-007ED308`) and the lowest free **even** index from 2 (`007ED30A-007ED319`) and keeps the
  smaller (`007ED33E-007ED35A`). `007ED31B-007ED33A` is the one adjustment: when the two candidates
  are adjacent, a member that already carries a positive **even** index is pushed to the next odd
  one, which is how a plane keeps its side across a re-assignment.

For a fresh three-plane squadron, where every member starts at 0, that gives indices **0, 1, 2**.

Callers. `python tools/bsp.py ghidra callers 007ed260` returns four; an exhaustive rel32 and
absolute-dword scan (`tools/callsite_census.py 007ed260`) returns **nine**, and the one that matters
most is among the five it missed:

```
007ED645  in 007ED610 BSP_PlaneSquadron_PromoteFlightLeader
007EE06A  in 007EDF50
007F3A11  in 007F3970 BSP_Squadron_RemovePlane
007F4BFA  in 007F4580 BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes   <-- the spawn tail
009A48B8  in 009A47D0
009AD1AF  in 009AD110
009B662F  in 009B6590
009BEDE4  in 009BED80            (<- 009C7240 DiveBombDone_Enter, 009D2530 TorpedoDone_Enter)
009D2558  in 009D2530 BSP_BotStateTorpedoDone_Enter
```

`007F4BFA` is confirmed in the listing: `007F4BEE MOV ECX,ESI` (the squadron) then the call, sitting
in the spawn tail just past the `007F4B43..007F4B6E` attach. **So the indices ARE assigned at spawn**,
and again on every promote, leave and follow-state entry. An earlier draft of this document said
they were assigned only on the follow entry; that came from the under-reporting caller query and is
withdrawn.

## 4. `007F23A0`, the station point

`__thiscall(squadron)(int formationIndex, float out[5])`, `RET 8`, body `007F23A0-007F28FF`, 345
instructions, cyclomatic complexity 19. Two callers, `007F2920` and `009BFD70`.

`009BFD70` is the proof of the argument list and of what the routine is for. It is
`__fastcall(state)`, body `009BFD70-009BFED6`, and it:

```
009bfd76  ECX = [state+4]                       ; the task context
009bfd79  EAX = [ECX+0Ch]                       ; the squadron; JZ out if null
009bfd84  EAX = [EAX+3D0h]                      ; member array slot 0 = the flight leader
009bfd92  CMP EAX,[ECX+4] / JZ 009bfeb1         ; the leader does not follow itself
009bfda1  006CEF40(state+18h, leader)           ; the task remembers the leader
009bfdb2  [squadron+3E8h] = [state+8Ch]         ; morale republished
009bfdbe  ECX = [[ECX+4]+9D0h]                  ; the unit's OWN formationIndex
009bfdc7  EBP = state+30h                       ; the out buffer
009bfdcc  CALL 007F23A0  with ECX = squadron
009bfdee  delta = own position (unit+0FCh..104h) - station
009bfe19  0042B2F0(delta)  vs  [[state+6Ch]+18h]
```

### 4.1 The displacement triple

`007F23DE PUSH 10h` then `leader->vtable[5Ch]()`, and `007F23FF PUSH 14h` then the same. Branch
senses from the bytes, not the decompile: `007F23F0 84 c0 75 6c` and `007F2403 84 c0 75 59`, both
`JNZ` to `007F2460`. So:

* leader answers `10h` (MPlaneBomber / `Type = LevelBomber`) **or** `14h` (the ReconPlane family,
  which `15h` and `16h` also answer) -> `Pilot/Follow/BomberDisplacement`, singleton `+3DCh`,
  loaded at `007F2465`.
* otherwise -> `Pilot/Follow/SmallPlaneDisplacement`, singleton `+3D0h`, loaded at `007F240C`.

A **TorpedoBomber is `11h` and a DiveBomber is `12h`**; neither chain contains `10h` or `14h`, so
both take the *small-plane* triple. That is counter-intuitive from the key names and it is what the
image does.

### 4.2 The frame

* Small-plane arm, `007F2444-007F245B`: `00414DB0` refreshes the leader's pose if `leader+0C8h` is
  zero, then `004134F0` copies **the leader's whole world matrix, `leader+0CCh`**, into the frame.
  A wingman therefore banks with its leader.
* Bomber arm, `007F2499-007F2508`: `leader->vtable[50h]()` gives an angle, `00B646E0` builds a pure
  **yaw** rotation from it, and the leader's world position (`leader+0FCh..104h`) is written into
  the frame's translation row at `ESP+70h` (`40h + 30h`, the fourth row).

`007F23C6` and `007F23D8` also fill `out[3] = leader->vtable[50h]()` and `out[4] = [leader+0C70h]`
before any of this. Nothing in this reconstruction consumes them.

### 4.3 The spacing floor

```
007f250e  EDX = [ESI+35Ch]                 ; the plane class descriptor
007f2514  FLD  [EDX+0A4h]                  ; the Lua `Width` (00960368 stores it at class+A4h)
007f2520  FMUL qword [00D049A8]            ; a DOUBLE, 1.7999999523162842
007f2538  FCOMIP disp[0] vs width*1.8 / JA ; keep disp[0] when it is strictly greater
```

So `spacing_x = max(displacement[0], width * 1.8)`.

### 4.4 The pair number and the five shapes

`007F254A LEA EAX,[EBX+1] / CDQ / SUB EAX,EDX / SAR EAX,1` is `(formationIndex + 1) / 2`
truncating toward zero: the **pair** number, so 1 and 2 share pair 1 and 3 and 4 share pair 2.

`007F2556 MOV EAX,[ESI+3E4h] / ADD EAX,-1 / CMP EAX,4 / JA 007F2853 / JMP [EAX*4+007F2900]` is a
**five-entry jump table**, so there are five formation shapes, 1 to 5. Only case 0 - shape 1, the
value `007F2C60` seeds and the only one a `SpawnNew` squadron can have - is read here:

```
007F2576-007F25B8, shape 1:
    pair = (index + 1) / 2
    x = pair * spacing_x        * morale      ; [ESP+24h]
    y = pair * displacement[1]  * morale      ; [ESP+28h]
    z = pair * displacement[2]  * morale      ; [ESP+2Ch]
```

The other four bodies are at `007F25BD`, `007F26AD`, `007F27DE` and `007F2838`. **They are not
read. That is a hole, not a proof.** Two of them set a flag (`007F26A2 MOV BL,1` and `007F26A4 MOV
[ESP+1Bh],BL`) that the tail below tests, so their tails differ from shape 1's as well.

### 4.5 The tail: which components are mirrored

```
007f2853  TEST byte ptr [ESP+0C4h],1      ; [ESP+0C4h] is the FIRST stack argument
007f285b  JZ 007f28bf                     ; an even index is transformed as it stands
007f285f  x = (-0.0f) - x                 ; 00D7A208 is -0.0f, so this is a negation
007f2873  JNZ 007f2896                    ; BL, the per-shape flag; 0 for shape 1
007f287a  CMP byte [tuning+3E9h],0 / JNZ  ; SymmetricalAltitude: non-zero SKIPS the mirror
007f2882  y = -y
007f2896  CMP byte [ESP+1Bh],0 / JNZ      ; the same per-shape flag
007f28a2  CMP byte [tuning+3E8h],0 / JNZ  ; SymmetricalPosition: non-zero SKIPS the mirror
007f28ab  z = -z
007f28bf  004142E0(local, out, frame)     ; out[0..2]
```

All three mirrors sit inside the odd-index arm.

### 4.6 The authored values, from this installation

`I:\SteamLibrary\steamapps\common\Battlestations Pacific\scripts\datatables\planeglobals.lua`,
mtime **2024-10-29 12:54:18**, table `Follow` (the file's own comment: *"follow parancs parameterei.
wingmanek igy kovetik a leadert"*, the parameters of the follow command, this is how wingmen follow
the leader):

```
["SmallPlaneDisplacement"] = { 60, 25, 70 }
["BomberDisplacement"]     = { 100,  0, -100 }
["SymmetricalPosition"]    = true
["SymmetricalAltitude"]    = false
```

The file's own Hungarian comments confirm the two gates independently of the branch bytes:
`SymmetricalPosition` is "whether the position of the wingmen on the two sides of the leader is
symmetrical (e.g. both sides' wingmen fly in front of it), or not (left in front, right behind)",
and `SymmetricalAltitude` is the same for altitude ("left below, right above"). `true` means *not
mirrored*, which is exactly what `JNZ` past the negation does.

With those values, a three-plane squadron whose leader is a TorpedoBomber (the small-plane triple)
gets, at morale 1.0:

| index | pair | side | local offset (x, y, z) |
| --- | --- | --- | --- |
| 0 | 0 | leader | (0, 0, 0) |
| 1 | 1 | odd | (-60, -25, +70) |
| 2 | 1 | even | (+60, +25, +70) |

120 m apart laterally and 50 m vertically, in the leader's own frame. No dropped torpedo starts
inside a squadron mate's hull box at that spacing.

## 5. `009BFEE0`, which is NOT reconstructed

> **Corrected 2026-09-19, packets `cc8_follow_law` and `cc8_follow_regimes`.** The paragraph
> below (and section 6) call `009BFEE0` "the law that flies a member to the station: the
> throttle and heading". **It commands nothing** — all fifteen of its callees are math or pose
> primitives, and it is a pure producer of one float3 steer point at `state+44h/48h/4Ch`. The
> body that issues every command is `009BEE30`, listed here as an unread third step. Its steer
> point is now read and bound for the lead-pursuit and abeam regimes; see
> `docs/PLANE_FOLLOW_LAW.md` §5.10-§5.12 and its §7 block map for what remains open
> (Phase A `009C0251`-`009C0EE0`, and `009BEE30`'s hold arm). The host no longer places a wing
> member on its station: `src/game_hosts_units.cpp` runs the law behind
> `kPlaneFollowLawEnabled`, with `kPlaneFormationPlacementEnabled` as its false twin.

`009C1FD0` is the follow tick. It calls `009BFD70` (section 4) and then `009BFEE0`. Its callers are
`009C7270 BSP_BotStateDiveBombDone_Tick`, `009D2720 BSP_BotStateTorpedoPrepare_Tick`, `009AD1F0` and
`009B6670`.

`009BFEE0` is **1795 instructions, 210 basic blocks, 55 calls, cyclomatic complexity 89, body
`009BFEE0-009C1846`**. It is the law that *flies* a member to the station: the throttle and heading
handling behind `GoodPositionDist`, `GoodPositionDir`, `NearbyDist`, `WaitForHdgDiff`,
`LeaderHeadingSpdTime/Dist`, `TightTurn` and the `yf_`/`pf_`/`rf_`/`pwr_` gain block of the same
`Pilot/Follow` table. This packet did not read it and does not reconstruct it.

## 6. What this process binds, and what stands in

`src/plane_formation.cpp` reconstructs `007ED260` and `007F23A0` shape 1 as pure rules.
`src/game_hosts_units.cpp`'s `Impl::place_wing_member_on_station_007f23a0` is the binding, reached
from the existing `follow_base_tick_009c1fd0` seam that `src/torpedo_task_arm.cpp` already calls at
`009D2731`.

| Native | In this process | Kind |
| --- | --- | --- |
| `007ED260` | reconstructed, both walks | proof |
| `007F23A0` shape 1 and the whole tail | reconstructed | proof |
| `007F23A0` shapes 2 to 5 | refused: `produced = false`, no station, no placement | **hole**, stated |
| `009BFD70`'s squadron lookup, leader refusal and index read | reconstructed | proof |
| `009BFEE0`, the station-keeping law | **not reconstructed** | **hole**, stated |
| the moment `007ED260` runs | the image runs it in the spawn tail at `007F4BFA` and again on every promote, leave and follow entry; this host runs it once per squadron at the member's first step, because at `007F4580` time the members are still plans with no unit and no pose | scheduling difference, stated |
| flying the member to its station | the member is **placed** on its station instead | **hole**: the geometry is the image's, the path to it is not |
| the moment the station is APPLIED | the image spreads the wing inside the follow state's tick (`009C1FD0`); this host applies it once at the member's first plane step, and from packet `cc8_done_state` also on every tick a DIVE BOMBER spends in `kDone`/`kPrepare` | **hole**, stated |
| the bomber arm's yaw-only frame | this host passes the leader's whole published pose in both arms | difference, inactive on USN04 because its leaders are TorpedoBombers (`11h`), which take the small-plane arm |

**CORRECTION, packet `cc8_done_state` (2026-09-19).** The row above used to say "no plane in this
process ever enters that state - every USN04 aircraft reports `states[attackrun=...]` and
`prepare_entries=0`". That is no longer true and was already stale when written for the dive bomb:
a 4800-frame USN04 run on main `6d9f7064d` reports `divebomb movieval ... states[done=303 ...]` and
`divebomb movieval|.-3 ... states[done=492 ...]`. `kDone` IS a follow state
(`docs/BOMBER_AFTER_TASK.md` section 1), and `cc8_done_state` now dispatches its tick, so a dive
bomber's wing members reach the placement every tick they spend there. The placement still has no
counterpart to the image's altitude clamp `009C16D2`-`009C1846`, which is the limit of the
stand-in; `docs/BOMBER_AFTER_TASK.md` section 6e states it.

The placement is the honest name for what replaces `009BFEE0`. A wing member in the follow state is
put at its station every tick; when the state leaves (a torpedo run-in, for instance) the placement
stops and the member flies on from where it stood, with the velocity its own physics gave it.

## 7. Measurement

See section 8 for the runs. The line a run prints without any probe is

```
plane formation geometry: squadron <name> tick=<n> wing=<k> pairwise=[0-1=.. 0-2=.. 1-2=..] \
    seat1 index=<i> local=(x y z) disp=(..) bomber_triple=<0|1>
```

paced one line per squadron per 400 ticks, produced whether or not the placement is enabled, which
is what makes a before/after on one binary possible.

## 8. Runs, USN04, `--frames 3200 --press-start-frame 30 --mission-frames 3000 --mission-frame-seconds 0.05`

Three runs of the same tree. The first is the BEFORE: the station code was compiled in but bound
only to the follow-tick seam, which **no plane in this process ever reaches** - every USN04 aircraft
reports `states[attackrun=...]` and `prepare_entries=0`, and the run printed not one
`plane formation` line. The second added the once-per-member placement at the plane step. The third
added the `number[3]` loader fix of section 8.1.

| | `local/formation_after_usn04.log` (BEFORE) | `local/formation_after2_usn04.log` | `local/formation_after3_usn04.log` |
| --- | --- | --- | --- |
| `torpedo_drop drops` | 6 | 6 | 6 |
| `swims_started` | **0** | **5** | **4** |
| `torpedo_closest_approach swims` | 0 | 5 | 4 |
| `range_peak_in_goaway`, squadron `B5N Kate #2.1` | 428.2 / 428.2 / 428.2 | 428.2 / 428.7 / 430.6 | 428.2 / 427.6 / 430.0 |
| `range_peak_in_goaway`, squadron `B5N Kate #4.1` | 428.6 / 428.6 / 428.6 | 428.6 / 427.3 / 432.8 | 428.6 / 426.4 / 432.2 |
| wing pairwise distance at tick 400 | no line: the seam never ran | 103.9 / 102.8 / 167.6 m | 82.0 / 86.8 / 104.0 m |
| `seat1 local` | - | (-60, -60, 60), wrong triple | **(-60, -25, 70)** |
| `disp` | - | (60, 60, 60) | **(60, 25, 70)** |

Run 3's `seat1 local=(-60.0 -25.0 70.0)` is exactly what section 4.6's table predicts for
formation index 1 at morale 1.0 with the small-plane triple: `x` mirrored because the index is odd,
`y` mirrored because `SymmetricalAltitude` is false, `z` left alone because `SymmetricalPosition` is
true. The pairwise distances stay in the 70 to 170 m band instead of being 0.

Two of the six rounds still do not swim in run 3, and this packet does not claim to know why;
`swims_started` moving 0 -> 5 -> 4 is the zero-versus-nonzero result, and 5 against 4 is inside the
run-to-run variation the mission has.

`swims_started` is a zero-versus-nonzero count, and the three identical `range_peak_in_goaway`
values per squadron are the co-location signature section 15.4 of
`docs/TORPEDO_AFTER_THE_DROP.md` described from the other end. Both move. USN04's gunnery TOTALS are
not deterministic run to run and nothing here rests on them.

### 8.1 A `number[3]` loader defect this packet had to fix first

The first placement run printed `disp=(60.0 60.0 60.0)` where
`scripts/datatables/planeglobals.lua` authors `{60, 25, 70}`. The cause is in
`src/game_tuning_singleton.cpp`, not in anything this packet wrote: a `number[3]` key occupies
**three** consecutive rows of `kGameTuningKeys`, one per destination float and all carrying the same
path, and the loader ran `host.number_triple` - which fills three floats from the handle - on every
one of them. Each row therefore rewrote the whole triple starting at its own offset, so
`SmallPlaneDisplacement` came out `(60, 60, 60)` and `BomberDisplacement` `(100, 100, 100)`, with
the last two rows of each group also writing one and two floats past the end of it. Only the first
row of a group writes now. It affects every `number[3]` key in the block, not only this packet's
two.
