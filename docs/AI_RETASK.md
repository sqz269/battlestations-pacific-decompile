# AI re-task after a release: what a released escort does next

Packet cc9_ai_retask, 2026-09-23. Base: main 8af437557. Switch `kAiRetaskBound` in
`src/game_hosts_commands.cpp`. Status: reconstructed, build-tested and run-compared on USN04
4700/4500 and E2 9200/9000. Not ABI-compatible and not game-validated. Descriptive names are
hypotheses.

Addresses: 00A32D50, 00A182C0 BSP_AiParties_Think, 00A181A0 BSP_AiPartyBrain_Think, 009FFE50,
00A12A90, 00A13B60, 00A10DC0, 0077C8D0 BSP_Entity_RequestJoinFormation, 00836920 and its idle
tail 00836DC9..00836E90, 007788B0 BSP_Unit_IsFormationFollower, 007788D0
BSP_Unit_FormationLeader, 0070E4C0 BSP_UnitGroup_DetachMember, 0070ECA0, 0077BD70, 0077D600,
00521EA0.

## 1. The image

**The escorts are commanded by the coordinator.** 00A182C0 walks the eight party slots. With
modes 0 to 3 and world+61Ch set, 009FFE50 enables only party 0 and party 4, the two sides'
commander slots (docs/AI_GROUP_THINK.md section 1). On USN04 the host's group table puts the
US ships in team 0, party 0, one group of 18 led by Lexington-class01, claimed by a planner. So
the player's escorts are members of an AI-commanded group.

**The group command decides what a member receives.**
- **CLOSEATTACK** (00A13B60, reached from 00A12A90's promotion when the two leaders close within
  CloseAttack_CollectDist). This orders every served ship member an `attackmove` on an object
  (00A14A6E). It is the source of the escorts' 124.5 s attack on D3A Val #1.1.
- **MOVETOATTACK**. This is where the group stands for the rest of the run: the end-of-mission
  table shows `command=MOVETOATTACK`. It sends the leader a `moveto` through 00A02020. A ship
  follower gets only 0077C8D0 BSP_Entity_RequestJoinFormation (00A10DC0, 00A10E3E) and no scene
  command at all (docs/AI_CLOSE_ATTACK_TICK.md, docs/AI_COMMAND_TICK.md).

So while the group is not in CLOSEATTACK, the coordinator never gives a released escort a new
attack. The party think has no idle-member detection that issues orders: 00A181A0 only claims
groups for planners and ticks them.

**The re-task is the director's idle tail.** When the attackmove ends at stage 2
(docs/SHIP_AI_TARGET_RELEASE.md), the next 00836920 reaches its idle tail:

```
00836E02  MOV EDX,[ESI]; CALL [EDX+6Ch](1)        the director reset
00836E0D  MOV ECX,[ESI+24Ch]; CALL 007788B0       is the unit a formation follower?
00836E1A  JE 00836E3F                             no: the cruise / stop pair
00836E1C  FLDZ ... CALL 007788D0                  the leader
00836E32  CALL 00465080                           a command target on the leader
00836E38  PUSH 0E08F60                            the `follow` object
00836E3D  JMP 00836E90                            issue through 0071ECF0
```

007788B0 is `g = [unit+284h]; g && [g+14h] != unit`.

Nothing on the attack path clears unit+284h. The census of stores to +284h:
- `89 ?? 84 02 00 00`: 14 hits
- `C7 ?? 84 02 00 00 00 00 00 00`: 4 hits

The only unit-group clears are 0070E4C0 (via 0070ECA0 RemoveEntity and 0077BD70) and 0091D620.
0077BD70 is reached from 0077F940 JoinOrMerge and the entity-kind message 0077FE80. 0077D600, the
order issue, calls none of them.

A released escort therefore stays in Lexington's group, and the image re-tasks it with `follow`
on Lexington at the first director step after the release.

## 2. The host, and the divergent term

For the eight escorts after 162.76 s:
- The party think runs: 72 close ticks and 2 promotions over the run.
- The group sits in MOVETOATTACK, so, as in the image, no attack order reaches them.
- The director's idle tail runs and asks 007788B0.
- The commands host answers from GameCommandUnit::formation_follower. The units host pushes that
  flag when a join lands (game_hosts_units.cpp, `set_unit_formation`).
- The escorts joined at the start (log line 1961, `formation column: follower=Northampton-class01
  leader=Lexington-class01`). Four later unit-table re-registrations, one per spawn batch at log
  lines 6568..6732, each **replaced** the rows with default ones (`host.units =
  std::move(units)`), clearing the flag.
- The idle tail therefore issued `stop`: director stop +8, follow +0 in the release packet's
  treatment.

**The divergent term is the follower pair lost at re-registration.** It is not the AI
coordinator, the hostile list, the resource rule or the target-facts source.

**Bound**, behind `kAiRetaskBound`: `register_units` keeps each existing unit's formation pair
when the incoming row has none. unit+284h is per-unit state in the image, and creating another
unit does not touch it.

## 3. Predictions

These were written to local/rt_predictions.txt before the runs.

1. At 162.76 s the eight escorts would release as before and receive `follow` on Lexington
   instead of `stop`. This held.
2. Any earlier difference would be an idle-tail firing on an escort. This held: there was none.
   The first ship-AI difference is step 3260.
3. Standoffs would be unchanged, with 153 choices and first = last. This held.
4. Headings after the release would move with the column. This held. Gunnery direction and
   Lexington were not predicted.
5. E2 would show the same release and follow. This held.

## 4. Measurement

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides. The control is the same source with the
switch false. Logs:

- USN04: local/rt_ctl_usn04.log and local/rt_trt_usn04.log
- E2: local/rt_ctl_e2.log and local/rt_trt_e2.log

| Row | USN04 ctl | USN04 trt | E2 ctl | E2 trt |
|---|---|---|---|---|
| releases at 162.76 s | 8 | 8 | 8 | 8 |
| escort state after 3260 | stop x8 | follow x8 | stop x8 | follow x8 |
| director idle stop / follow counters | 39 / 17 | 31 / 17 | 39 / 17 | 31 / 17 |
| standoff choices per escort | 153 | 153 | 153 | 153 |
| queued hits | 241 | 240 | 310 | 329 |
| damage | 18217.5 | 18379.1 | 20182.2 | 22682.2 |
| deaths | 24 | 26 | 35 | 36 |
| Lexington | afloat, 141.9 HP at the 225 s end | sunk 221.81 s, credited to York-class02 | sunk 225.81 s | sunk 221.81 s |

In the treatment, `follow` keeps `last_idle_command`, so the idle counter does not rise. The
eight follows are visible in the ship-AI rows instead.

**AI command rows** are identical in both pairs: tick orders, the 54 attackmove choices, the 50
suppressed duplicates and the close ticks. The re-task does not go through the coordinator.

**Ship-AI rows** are identical up to step 3260 in both pairs. At step 3260 every escort row reads
`state=follow` instead of `state=stop`, and it stays follow to the end. That is the one term,
007788B0's answer.

**Downstream.** USN04's 4500 frames end at 225 s. The E2 control shows Lexington sinking at
225.81 s without the change, so its survival in the USN04 control was an artefact of the window.
With the escorts following, it sinks 4 s earlier.
- The only new impact on Lexington in USN04 is one bullet class 31 blast (Artillery, V0 300,
  range 1500, first used by category 6; took 15.0), credited to York-class02.
- The rest of its last 142 HP is not in any impact line and was not traced.
- The shoot-downs of Val #1.1|.-3 and |.-4 move later or away.

Decision: landed with `kAiRetaskBound` on. The image re-tasks released escorts with `follow`
through the director idle tail, and the host now does the same.

## 5. Step 4, read-only

- **The position-target attackmove.** 00521EA0 returns 0 for a descriptor whose kind byte is 0
  (`CMP byte [ECX],0; JNZ; XOR EAX,EAX; RET` at 00521EA0..00521EA7). So the director arm raises
  stage 2 on the first step for an attackmove aimed at a position. The host's bound arm does the
  same: a zero resolve is "target gone".
- **The friendly-fire credit.** Both Lexington kills in these pairs are credited to an escort,
  after a bullet class 15 or 31 blast at dist 0 (Artillery, first used by category 6).
  - The firing gun was not traced.
  - That the credit rule 0077CE60 has no side filter was established by the RNG-streams packet,
    not re-read here. On that finding, the credit is faithful whenever the blast lands.

## 6. Open

- Which gun fired the category 6 blasts onto Lexington, and whether the image's projectile path
  lets a friendly blast damage its own side.
- The part of Lexington's final damage that has no impact line.
