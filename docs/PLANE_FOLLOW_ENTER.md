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

Measured results follow in section 6 once the runs land.

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
