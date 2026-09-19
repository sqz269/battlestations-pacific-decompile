# How a plane ENTERS the follow state (packet `cc8_follow_enter`)

Worker `cc8-follow-enter`, 2026-09-19, branch `agent/cc8-follow-enter`. Fourth on the plane follow
law, after `cc8-follow-law`, `cc8-follow-steer` and `cc8-follow-regimes`. Those three read and bound
the law; `cc8-follow-regimes` then measured its switch as a NULL, because nothing in this host ever
enters `BotStateFollow`. This document is about the entry, not the law.

## 1. `007B8AD0` is the flight-leader test, and its ledger name is a misreading

Four instructions, body `007b8ad0-007b8adb`, bytes `33 c0 39 81 d8 09 00 00 0f 94 c0 c3`:

```
007b8ad0  XOR EAX,EAX
007b8ad2  CMP dword ptr [ECX+9D8h],EAX
007b8ad8  SETE AL
007b8adb  RET
```

`__fastcall(ECX = unit) -> bool`, i.e. `return unit->+9D8h == 0`. That much was never in doubt.

The ledger name `BSP_Unit_LacksFollowTarget` and `docs/BOT_TASK_STATES.md` read `+9D8h` as the
unit's follow target, explicitly on the reader alone: that section ends `No writer of +9D8h was
read: contract: unread`. **The writers refute it.** A census of both store forms, `--limit 4000`
with the enclosing function:

| site | in | what it stores |
| --- | --- | --- |
| `007CFE72` | `BSP_PlaneUnitInstance_Construct` | **-1**. `007CFD69 OR EDI,0FFFFFFFFh`, and a filter of the whole 207-line listing for `EDI` shows writes only at `007CFD50`/`58`/`63`/`69` - none between that `OR` and the store, and `EDI` is non-volatile across the calls in between |
| `007F4B43` | `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | `squadron+3CCh`, the member count *before* the append at `007F4B55` |
| `007CDF7C` | `FUN_007CDF20` | an index from `[[squadron+8]+120h]`, beside `007CDF6C` `plane+9D4h = squadron`, then `CALL 007ED0D0` |
| `007ED0EC` | `BSP_PlaneSquadron_InsertPlaneSorted` | the same `+9D4h`/`+9D8h` pair |
| `007ED220` | an unnamed append helper | `plane+9D4h = squadron; plane+9D8h = squadron->3CCh; members[count] = plane; count++` |
| `007ED292` | `BSP_PlaneSquadron_AssignFormationIndices` | `member+9D8h = i`, the live walk index |
| `00A41519`, `00A4161D` | `FUN_00A414D0`, `FUN_00A415B0` | immediates, far outside unit code - a different class with a field at the same offset |

`007ED220` is worth a line of its own: `scan-bytes` attributed it to `FUN_007ED1D0`, which actually
ends at `007ED20C RET 4`. Ghidra has no function at `007ED210`. The nearest *preceding* function is
not the containing one.

Every plane-side writer stores an **index**, never a pointer. And slot 0 is the leader:
`007ED610 BSP_PlaneSquadron_PromoteFlightLeader` rotates the promoted member to the **front** of
`+3D0h` and only then calls `007ED260`, whose second walk hands slot 0 formation index 0, the
leader's own station (`007ED2D2`, `docs/PLANE_FORMATION.md` section 3). `007F4BFA` runs that
assignment in the spawn tail, so `+9D8h` equals the live array position from the first assignment
onward - the host record comment "stamped at 007F4B43 and never rewritten" is wrong on that point.

**So `007B8AD0` asks "am I my squadron's flight leader".** The leader takes moveto; every wing
member takes follow. Three regimes, not two: `-1` unattached, `0` leader, `> 0` wing member.

Corroboration from the consumers. `tools/callsite_census.py` gives **85** call sites where
`bsp.py ghidra callers` gives 67. Most are bot-task `*_Construct` / `*_UpdateCruiseProfile` /
transition bodies, but the outliers decide it: three calls in
`BSP_InGameHudMarkers_BuildUnitMarker`, one each in `BSP_MissionScoring_NotifyEntity`,
`BSP_AiParty_AvailableResources`, `BSP_UnitGunneryAi_ScoreCandidate` and
`BSP_PlaneSquadron_TickAdvance`. A HUD marker asking "is this unit its squadron's representative"
is sensible; a HUD marker asking "does this unit have a follow target" is not.

`docs/PLANE_FORMATION.md` section 2, `docs/BOMBER_AFTER_TASK.md` section 6 and
`docs/AIRFIELD_TAXI.md` already carried the slot reading. `docs/BOT_TASK_STATES.md` is the one that
does not, and section 5 below withdraws it.

## 2. What decides moveto vs follow, in one sentence

The task's unit's position in its squadron's `+3D0h` array: `007B8AD0` is true only at slot 0, the
flight leader, which takes the moveto substate; every other member takes follow.

Both bomber tasks make that choice in three places, always with the identical shape
`CALL 007B8AD0 / TEST AL,AL / LEA <moveto> / JNZ over / LEA <follow>`:

| task | construct | `!engaged` edges | moveto | follow |
| --- | --- | --- | --- | --- |
| dive bomb `009C7710` | `009C7777` | `009C841F`, `009C8744` | `+4F0h` | `+52Ch` |
| torpedo `009D3050` | `009D30B7` | `009D4082`, `009D41E4` | `+544h` | `+580h` |

`BSP_BotTaskRocket_Construct 007B6A40` calls it at `007B6AA7` and
`BSP_BotTaskRocket_UpdateCruiseProfile` at `007B7AFE`, the same pattern; so does every other
`BSP_BotTask*_Construct` in the census.

## 3. What a wing member does about attacking

The engaged test is what takes a member back out of follow.

* **Dive bomb**, `009C83E0`: `009C8401 MOV EAX,[ESI+404h]` / `009C8407 CMP [EAX+370h],2` /
  `009C8410 CMP [ESI+440h],0`. Both `!engaged` edges (`009C8419-009C845E`, `009C873E-009C8783`)
  are reached only when that fails; when it holds, the branch at `009C8461` runs the attack
  instead. So a member leaves follow exactly when its **squadron's** attack mode `+370h` is 2 and
  the task has a latched target `+440h`.
* **Torpedo**, `009D3210 BSP_BotTaskTorpedo_IsEngaged`, read whole:

```
009d3213  CMP byte ptr [ESI+529h],0 / JNZ -> 1      ; the aim latch
009d321c  CMP [ESI+4C4h],0         / JZ  -> 0      ; no target, not engaged
009d322b  CMP [EAX+370h],2         / JZ  -> 1      ; squadron under attack order
009d323a  CALL 007B8AD0            / JZ  -> 0      ; NOT the leader: cannot self-engage
009d3243  FLD [ESI+488h] / FLD [ESI+484h] / FMUL qword [00D05AC8] / FCOMIP ST0,ST1 / FSTP ST0
009d3259  JA  -> 1                                 ; leader only: 484h * 2.2 > 488h
```

`00D05AC8` reads **2.2** as a double (`pe_const_read.py d:00d05ac8`), which is the constant the
host already carries as `kTorpedoEngageRangeScale_00d05ac8`.

That `009D3241 JZ` is the cleanest single witness in this packet. **A wing member cannot decide to
attack on its own**: only the flight leader reaches the range test. Members become engaged only
when the squadron's mode reaches 2, which is the attack order. A "follow target" reading cannot
explain why the range test is gated on it; a flight-leader reading is the whole point of it.

## 4. The feed, and the prediction

`src/game_hosts_units.cpp` gained one helper,
`unit_is_flight_leader_007b8ad0(process_index)`: the first live entry of the squadron record's
`member_units` is slot 0. The host already computed exactly this rule 170 lines above the
dive-bomb feed, for the spent-member arm (`0099B757` compares the unit against `[sqn+3D0h]`), so
this is the same test the host already trusted, given a name.

Seven hardcoded `true`s were replaced: the dive-bomb transition feed, both task **initial states**,
the torpedo engaged feed in the arm and in `read_transition_inputs`, that function's own transition
field, and the two `unit_has_no_follow_target()` overrides.

**One labelled substitution.** A unit with no squadron record answers `true`, the leader answer,
which is the previous hardcode. The image's own no-squadron state is the constructor's `-1`, which
is not slot 0 and would take follow - but "this host's registry has no record" is not evidence of
"the image would have left `-1` here". The registry holds `PlaneSquadronGen`/`007F4580` spawns
only; USN04 registers **one** squadron of three. Answering `false` for every other aircraft would
put planes whose membership is merely unknown into a follow state that `009BFD70` then declines to
produce a station for - commanded nothing, on no evidence.

### Prediction, written before run B

1. **Exactly two aircraft enter follow**: array slots 1 and 2 of the single registered
   `PlaneSquadronGen` squadron (`WingCount=3`, 3 members resolved). Every other aircraft in USN04
   has no squadron record, answers "leader", and is bit-for-bit unchanged.
2. **They stay there for about one tick each, not for a cruise.** `src/game_hosts_units.cpp:1580`
   hardcodes `in.engaged.control_mode_370 = 2` for the dive-bomb transition, and `+440h` is fed
   from `command_target_plus_one`, which is already non-zero when the task is installed. The
   engaged test of section 3 is therefore already satisfied on the first transition tick, and the
   member leaves follow for the attack branch immediately. So I expect `follow law` to appear in
   the log for the first time - the entry defect is real and fixed - and to appear only a handful
   of times.
3. **If that is what the run shows, the predicate was one of two gates**, and the second is that
   hardcoded attack mode, which belongs to another packet's hunk. The torpedo side does not share
   it: `torpedo_attack_mode_370` is a real maintained field there, not a constant.
4. Nothing else moves: no change to releases, water contacts, or the `D3A Val #1.1|.-2` descent,
   because placement is still ON in run B.

### Amendment: prediction 1 was already falsified by run A, before run B landed

Written after run A finished and while run B was in flight. **Prediction 1 is wrong, and it is worth
recording why, because the mistake is a reusable one.** I took "1 squadron(s) over 3 member
plane(s)" from a `plane squadron members:` line in a predecessor's log. That line is printed
*repeatedly* as squadrons launch, and I read the first one. Run A's own log prints it fourteen
times, ending at **`39 of 39 wing record(s) resolved to units over 13 squadron(s)`**. So the
registry holds 13 squadrons, not one: 13 leaders and **26 wing members**.

Run A corroborates that from a second, independent column. Every `done 009C7240/009C7270` line
carries `placed=`, and `place_wing_member_on_station_007f23a0` returns false - `placed=0` - exactly
when the squadron record is missing *or* the unit is `wing.front()`. In run A the first aircraft of
each `|.-N` group reads `placed=0` and its two siblings read `placed=598`, `placed=624` and so on.
Those siblings could only be placed through a non-null squadron record. The `D3A Val #N.1` groups
are therefore registered squadrons too, not unregistered singletons.

So the corrected expectation for run B is **26 wing members answering "not leader"**, not two - of
which the ones carrying a dive-bomb or torpedo task construct into follow. Prediction 2 (that they
leave again almost at once, because `control_mode_370` is hardcoded to 2) is untouched by this and
still stands; so does the substitution's reasoning, which is about units with no record at all.

The general lesson: **a census line that a run prints more than once has no meaning until you take
the last one.** `Select-Object -First` on a growing census is the same class of error as reading a
constructor store and calling it the value.

## 6. Measured

USN04, `--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800
--mission-frame-seconds 0.05`, both runs on this tree, same binary apart from the feed.

| | A: before (placement ON, predicate hardcoded) | B: predicate fed, placement ON |
| --- | --- | --- |
| `follow law` lines | **0** | **0** |
| task census rows | 27 | 27, of which **8 differ** |
| releases, all rows summed | **35** | **35** |
| gunnery step 4800 deaths | 14 | **10** |
| gunnery step 4800 hits / damage | 151 / 14042.2 | 109 / 14926.0 |
| `D3A Val #1.1\|.-2` done alt | 274.5 -> 31.3 | 274.5 -> 31.3 (identical) |
| `BotStateFollow::station_keeping` | 795 | 697 |
| `BotStateDiveBombDone::station_keeping` | 7035 | 7035 (identical) |

**The entry defect is real and it is fixed - on the torpedo side.** All eight changed rows are
torpedo wing members, the `|.-2` and `|.-3` of four `B5N Kate` squadrons, and they change like this:

```
A  torpedo B5N Kate #2.1|.-2 arm_ticks=2143 transitions=3 states[moveto=646 attackrun=320 goaway=995 aim=182] releases=1
B  torpedo B5N Kate #2.1|.-2 arm_ticks=2143 transitions=2 states[follow=1003            goaway=935 aim=205] releases=1

A  torpedo B5N Kate #4.1|.-2 arm_ticks=2132 transitions=4 states[moveto=529 done=255 attackrun=262 goaway=932 aim=154] releases=1
B  torpedo B5N Kate #4.1|.-2 arm_ticks=2132 transitions=3 states[follow=817 done=207            goaway=932 aim=176] releases=1
```

A wing member that used to fly 646 ticks of moveto and **320 ticks of its own attack run** now
spends 1003 ticks in **follow** and never enters attackrun at all. That is exactly what section 3
says the image does: a member cannot self-engage, so it holds formation until the squadron's mode
reaches 2. `transitions` falls 3 -> 2 because the moveto -> attackrun edge is gone.

**And it still releases.** `releases=1` on every changed row, and the run total is **35 in both
runs**, so criterion (a) holds exactly. Deaths fall 14 -> 10, so criterion (b) holds - four fewer
aircraft lost, not more. Criterion (c) is unchanged and could not have changed: see below.

**Prediction 2 is confirmed, on the side it was made about.** Not one dive-bomber row differs. All
27 dive-bomb census rows, every `done` line, and `BotStateDiveBombDone::station_keeping` at 7035
are identical between A and B. The reason is the one predicted: `control_mode_370` is hardcoded to
`2` for the dive-bomb transition, `+440h` is already non-zero when the task is installed, so the
engaged test of section 3 is satisfied on the first transition tick and the member leaves follow
before its state tick is ever dispatched. Two facts in the host make that invisible rather than
merely brief: `dive_bomb_state_bucket(ctx.current)` and the state dispatch both run **after** the
transition rule, so a state entered and left within one arm tick records zero ticks and runs no
tick. That is also why run A shows no `moveto=` entry although every aircraft constructs into
moveto. **The predicate was one of two gates. The second is that constant, and it is not this
packet's hunk.**

### Why run C (placement OFF) was NOT run, and what should replace it

Not a context decision - an evidence one. `follow law` is **0 in run B** even though eight aircraft
now spend about a thousand ticks each in follow. The law is wired into exactly one place,
`run_dive_bomb_follow_tick_009c1fd0`, which is dispatched only when a **dive bomber's**
`ctx.current == kFollow` - and no dive bomber reaches that, for the reason above. The torpedo follow
state reaches its station through the torpedo arm's own `follow_base_tick_009c1fd0` seam, which
records `BotStateFollow::station_keeping` and does not call the law.

So with placement OFF, those eight aircraft would receive **no station-keeping at all** - not the
law, and no longer the placement. Run C would not measure "the law flies the wing members"; it
would measure removing station-keeping from eight aircraft that are now holding formation for a
thousand ticks, and criteria (a)-(d) would be judging that instead. `kPlaneFormationPlacementEnabled`
is therefore left **ON**, and B is the landing.

The two things that would make a real run C possible, in order:

1. Wire the follow law into the torpedo follow seam as well, so the aircraft that actually reach
   follow are the ones the law flies; or feed `control_mode_370` so dive bombers reach their own
   follow tick, which already has the law wired.
2. Read `009BEE30`'s HOLD arm. With the good-position gate substituted to "fly-to arm always", a
   member the law holds on station is commanded by the wrong arm, and criterion (d) cannot be
   judged honestly until that is read.

Criterion (d) has a baseline either way. The `plane formation geometry` line's `pairwise=` column is
sampled at the top of `place_wing_member_on_station_007f23a0`, **before** the placement gate, so it
is genuine drift: run A settles at `0-1=94.9 0-2=92.0 1-2=128.3` at tick 5200 for squadron
`movieval`, against a station geometry of `local=(-60.0 -25.0 70.0)` for seat 1.

### Criterion (e), the mutual torpedo kills: answered by run B

`entity_impact ... life=0.05` rows naming a `B5N Kate`: **A = 4, B = 0.** In run A the two aircraft
of `#4.1` torpedo each other and the two of `#8.1` do the same, impact points metres apart at a
common altitude. In run B, with the predicate fed and **placement unchanged**, all four are gone.

That discriminates the three candidates the integrator listed. It is **(i)**: the wing members were
flying their own attack run and converging on the leader's track, because every aircraft in this
host steers at the same aim point with no lateral offset. The census shows the mechanism with no
inference needed - `attackrun=320` in A becomes no `attackrun` entry at all in B, on exactly the
eight `|.-2`/`|.-3` rows. It is not a duplicate formation index and not a collapsed station
displacement: the station producer is untouched by this change. Four of twelve torpedoes recovered,
with `releases` unchanged at 35.

## 7. Run D: the second gate, and its prediction

The integrator granted this packet the one line it had pinned: `in.engaged.control_mode_370`, now
fed from `slot.db_attack_mode_370`. Its pin rested on two measurements
(`docs/BOMBER_AFTER_TASK.md` 10.9, `docs/DIVE_BOMB_APPROACH.md`'s B' run) in which wiring the real
mode was a net regression - but both were taken when every aircraft answered `007B8AD0` as a leader
and flew its own moveto, with **no follow state to fall back to**. This packet removed that premise,
so mode 1 no longer means "commanded nothing"; it means "hold formation".

### Prediction for D, written before the run

The integrator's prediction is that wing members stay in follow through the whole approach, engage
when their own latch sets inside 2080 m, and that the `#1.1|.-2` descent in `done` disappears.
**Mine differs on one point**, and that is the point worth measuring:

1. Dive-bomber wing members enter follow and **stay** for hundreds of ticks, as the torpedo members
   already do in B. `follow law` becomes non-zero for the first time in this chain, because the law
   IS wired into the dive-bomb follow tick (unlike the torpedo seam). This is the first run in which
   the law executes at all.
2. **Leaders change too, and that is the risk.** The mode is not a per-member field: `0099B740` ->
   `007ED3F0` has the leader set the squadron's mode to 1 every think, so feeding it drops the
   LEADER out of `engaged` as well, and the leader's own `engaged` then collapses to the in-range
   latch at `R = approach+B8h = 2080 m`. So I expect the leader's `attackrun` to start LATER (nearer
   the target), not merely the members' to vanish. If total `releases` falls below 35, that is where
   it will have gone, and it is the same effect that made the two earlier verdicts a regression.
3. Criterion (c) judged against **31.3 m**, not 0.0: `#1.1|.-2` ends `done` at 31.3 m on this tree.
   I expect it to stay at or above that, and in D also to sit inside the leader-relative band.
4. Mutual kills stay at 0 - they are already fixed by B and the dive-bomb mode cannot reintroduce
   them on the torpedo side.

Falsifier for the whole packet: if `releases` falls and criterion (c) worsens, the pin was right for
a reason this packet did not remove, and the line goes back with D's table in its comment.

## 8. Two items owed by the altitude band, both closed

**`Pilot/Follow/LeaderFollowAlt` is authored `10`.** From this installation's
`I:\SteamLibrary\steamapps\common\Battlestations Pacific\scripts\datatables\planeglobals.lua`,
mtime **2024-10-29 12:54:18**, 46647 bytes, line 497: `["LeaderFollowAlt"] = 10`. That mtime matches
neither the locally modified `vehicleclasses.lua` (2026-05-09) nor the untouched bulk
`reconclasses.lua` (2024-07-13), so it is this installation's shipped value and not a local edit.
Two neighbours corroborate the existing reading rather than adding to it: line 492
`["LeaderHeadingSpdTime"] = { 0.5, 4.0 }`, which is §5.10's 0.5 s to 4.0 s lag ramp, and line 496
`["FollowedPointDist"] = 250`, which is the `+250` of §5.12's lead-pursuit point.

So the band's floor is `min(leaderY + 10.0, state+88h)`, with 10 m the authored offset.

**The writer of `state+88h` is the follow state's own Enter, and there is only one.**
`009BEDBB FSTP float ptr [ESI+88h]`, inside `009BED80 BSP_BotStateFollow_Enter_Provisional`:

```
009beda0  MOVSS [ESI+94h],XMM0        ; 0.0
009beda8  MOVSS [ESI+90h],XMM0        ; 0.0
009bedb0  FLD float ptr [EAX+8]       ; EAX = [ESI+6Ch], loaded at 009BED9A
009bedb3  MOVSS XMM0,[00D7A24C]       ; 1.0f
009bedbb  FSTP float ptr [ESI+88h]    ; state+88h = *(float*)([state+6Ch] + 8h)
009bedc3  MOV  [ESI+84h],BL           ; 0
009bedc9  MOVSS [ESI+8Ch],XMM0        ; 1.0f
009bedda  MOV  [ECX+3E4h],1           ; psFormation = 1
009bede4  CALL 007ED260               ; AssignFormationIndices
```

So `state+88h` is **latched once, at the moment the member enters follow**, from `[state+6Ch]+8h` -
it is not maintained per tick. That matters for the substitution: the host currently supplies `1e30`
so that "the named half of the band floor wins", and the honest replacement is a value sampled at
entry, not a running one. `state+8Ch` is `1.0f` from the same `[00D7A24C]` §5.2 already names, and
`state+84h` is cleared beside it.

Census scope, stated: `scan-bytes 'd9 9e 88 00 00 00'` (the ESI form) returns exactly this one site
image-wide. I did not enumerate the other seven ModRM register forms, so "only writer" is proved for
the ESI form and is an inference for the rest; `009BED80` being the state's Enter, and `+88h` being
read as a latched band floor, is what makes the inference reasonable.

### A retraction

I told the integrator that run A contained no `torpedo ... arm_ticks=` rows and that the torpedo arm
was not exercised on this mission. That was wrong: the rows are indented, and my anchored pattern
`^torpedo` did not match them. There are eight such rows in both runs and they are the whole of this
packet's measured effect.

## 5. Withdrawals owed by this chain

* `docs/BOT_TASK_STATES.md` section "`unit+9D8h`": the follow-target reading, and the claim that
  the two consumers agree on it. They agree on the *shape* (`moveto` when true), not on what the
  field is. Corrected by section 1 above.
* The ledger name and comment on `007B8AD0`.
* `docs/PLANE_FORMATION.md` section 6 and `docs/BOMBER_AFTER_TASK.md` section 6 still call
  `009BFEE0` the law that flies the member; `cc8-follow-regimes` established that `009BFEE0` is
  pure geometry and `009BEE30` is what commands.
* `include/bsp/plane_squadron_host.hpp`'s comment that `member_spawn_index` (`plane+9D8h`) is
  "stamped at `007F4B43` and never rewritten": `007ED292` rewrites it, and `007F4BFA` runs that in
  the spawn tail.
