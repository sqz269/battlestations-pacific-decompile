# CC13_VERIFY6: labelled substitutions in the commands host

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify6 on 2026-10-17 against
base f30aa7714. It covers `src/game_hosts_commands.cpp` and the switch and label comments in
`include/bsp/game_hosts_commands.hpp`: the scene-command chain (0046AAB0 -> 0077D600 -> 00816E30 ->
0071ECF0 -> 00721A40 -> 008358D0 / 0071E6C0), the director step 00836920, the slot clears, the
release delivery 0071DDB0 and the path cursor. No code was changed and no ledger records were added.
Every line number below is against that base. "Image" means battlestationspacific.exe, read with
`bsp.py show`, `ghidra disasm` and `disasm-raw` on disk bytes. Mission scripts and scenes are this
installation's `scripts\` and `universe\Scenes\missions\` trees (a modded install). Run evidence is
from existing cc9 logs (no game was run): `cc9-gunnery36\local\g51_r*_*.log`,
`cc9-lua52\local\l65rt_*.log`, `cc9-ships44\local\s56_*.log`, all written on 2026-10-17.

Excluded, as briefed (cc9-ships56 and cc9-physics15 are editing them): 0071D980, 0071E7F0, 0071E610,
0071BE40, 0071EB60 and the AutoTarget / free-attack gate code. In this file that is `:2584-2617`,
`:3648-3730` and the override arm `:3078-3100`. Two findings below touch that layer from outside it
(R5 and the 0071DDB0 override arm) and are reported for its owners, not resolved here.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  or RNG draw count only.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does, or the premise no longer holds.
- UNRESOLVED.

Every switch this file and its header read is ON at this base: `kAiRetaskBound`,
`kDirectorCommandArmsBound`, `kFireTargetObjectIdBound`, `kSquadronSetCommandBound`,
`kSceneCommandFindCaseInsensitiveBound`, `kEntityCommandSelfKindBound`, `kDirectorTargetChecksBound`,
`kCommandExtraTestsBound`, `kSetCommandQueueDelayBound`, `kSetCommandClearAllMessageBound`,
`kClearOrdersSendBound`, `kPathObjectDefaultPairBound`, `kAutoTargetOverrideBound`, and the imported
`kWeaponDirectorFireTargetBound`, `kPathFollowFinishBound`, `kPathCursorFollowStepBound`,
`kSquadronPathFollowStepBound`, `kFormationJoinFollowBound` (units host). So no OFF switch with an
existing body was found in this file; every `if (!k...Bound) record(...)` arm is dead code.

## Census

The census matched
`substitut|approximat|stub|simplif|deterministic|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|LABELLED|fixed value|unresolved`
(37 lines), then `not modell?ed|host (keeps|holds|has no)|instead of|rather than`, then every
`record(` / `record_slot(` call (about 110 sites). Each record was mapped to its call site in the image
and classed as dead (behind an ON switch), trace-only (the bound path also runs), or live (the record
is the only thing that happens). The live records and the labels are grouped by image function below.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `:3584-3620` plane target death ends the slot; `:721-743` 0077D600 plane->squadron retarget is a record | 0071DDB0 -> vt[70h] 00835B40 / 0084DA60 -> 0071EDD0 member walk; 0077D6CB vt[5Ch](0Fh) -> plane+9D4h | yes: `plane_matches` USN01 14, ESMP 08_engano 87, PRCP usn_03 1 | none |
| R2 (URGENT) | `:1315-1335` scene `follow` arm: kind test false, 0077C8D0 and 0064A8E0 records | 00816EA6..00816EE2: 0077C8D0 RequestJoinFormation | yes: 920 authored Follow tokens (ESMP 08_engano, usn_5/14/17, LOMP 02/03/05/07/08/09/11 ...) | none |
| R3 (URGENT) | `:3242-3261` path join index = nearest point alone | 007B1C50 -> 007B11F0 projection test picks nearest+-1 | yes: every PATH_SM_JOIN moveonpath that starts past its nearest point | none in modes 5/8 |
| R4 (URGENT) | `:313-317`, `:2812-2818` `command` channel never has a subscription; `:2546` 0071F600 "start" reports missing | 008C6760 -> 00980C10 -> 0097E360; 0071F79B / 0071F8C0 / 00836D51 -> 00984300 | yes: jm02.lua BettyRetreat / NellRetreat | none |
| R5 (probable) | `:1177-1184` override_command() = 0, echo gate and echo records | 0071E7B4..0071E7D4 -> 0071E2E0 ends the override on a category 1/2 push | where AutoTarget installs an override (us_osumi: 66 and 60 installs) and an attack order follows | none |

Latent REAL (wrong if reached, not reached on any logged row): the `clearorders` arm's 00835860(0, 1)
and 0080DC70 / 0071BF20 records (`:1353-1384`) and the attack-move building conversion
(`:3026-3052`). See the table at the end.

### R1 A dying plane target ends the slot instead of moving to its squadron

`release_observed_target_0071ddb0` (`:3544-3625`) rewrites every slot descriptor that resolves to the
dead unit to the position branch and then ends slot 0 (0071D810(2) plus the 5Dh clear) or clears a
category 1/2 slot i > 0. Its label (`:3584-3586`) says the host has no squadron member list.

Image: 0071DDB0 makes an indirect `CALL EAX` through the director's own vtable[70h] at 0071DE57; the slot is 00835B40 for the ship director
(vtable 00D09F58) and 0084DA60 for the squadron command block. Both delegate to 0071EDD0. For a target
with vt[5Ch](0Fh) and a squadron at +9D4h, 0071EDD0 walks the members at +3D0h (first five, count
+3CCh), keeps the LAST one whose +5Dh is clear (the gate `d^2 < [00D7A278]` is FLT_MAX), re-registers
the observer (006952A0, 00694A60), rewrites the descriptor to that member (kind 1, id +174h) and
answers 0, so `0071DE5B JE 0071DE96` skips both the stage raise and the clear: the command continues
on the next wingman. Only with no live member does it take the position branch the host always takes.

Upstream, 0077D600 (`EntityIssueBinding`, `:721-743`) answers `object_is_kind_of` false and
`retarget_object` null, both records. The image at 0077D6CB asks vt[5Ch](0Fh) of the resolved target
and on true rewrites the descriptor to the plane's squadron (plane+9D4h, id +174h, the zero vector at
00F87574). So an order issued through 0077D600 at a plane targets the squadron in the image and is not
released when one plane dies; only descriptors that reach a slot without 0077D600 keep a plane and take
the 0071EDD0 walk. The host keeps the plane in both cases.

Live: the summary line `director release ... plane_matches` reads 14 of 62 slot matches on USN01
(`l65rt_u01g_j3`), 87 of 331 on ESMP 08_engano (`g51_r1_e8`), 1 on PRCP usn_03 (`l65rt_p01_ref`). Each
is a head end where the image would usually keep attacking. Fix area: the units host's squadron
member list and +9D4h exposed to the commands host. RNG: none.

### R2 A scene-authored `follow` never requests the formation join

`EntityCommandArmsBinding` (`:1315-1335`): `target_is_kind_of_vtable5c` answers false (record),
`request_join_formation_0077c8d0` and `call_0064a8e0` are records. The cascade's `follow` arm
(`src/entity_command_arms.cpp:50`) therefore passes target 0 to a record and returns without queueing.

Image: `00816EA6 CMP EBP,0E08F60`, `00816EB6` 00521EA0, `00816ECA` vt[5Ch](2), `00816ED5 CALL 0077C8D0`
(RequestJoinFormation: 008162B0 -> 00779D50, then the type-76h message to 0077F940), `00816EE2`
0064A8E0 (a HUD ping, presentation). The 0077C8D0 binding already exists in the AI host
(`src/game_hosts_ai.cpp:2977` `tick_request_join_formation`, with the 00779D50 facts and the
`kPlannerJoinLoopbackBound` post to `formation_join_0077f940`); the commands host never calls it.

Live: 920 authored `Command = Follow` tokens, mostly ships following ships at mission start:
05_philippine_sea 117, usn_14_phil_sea 88, LOMP 03_defending_midway 60, LOMP 05_santa_cruz 58,
usn_17_engano 54, LOMP 02_holding_ceylon 54, usn_5_midway 44 (Hornet and Pensacola follow Enterprise),
ESMP 08_engano 30 (every destroyer and light carrier follows Iowa or Zuikaku). The ESMP 08 log
(`g51_r1_e8`) shows the record (`EntityCommandArm::request_join_formation [0077c8d0] UNIMPLEMENTED`),
`formation join ... joined=0`, and the script's `formation leader calls=446 found=0`: the mission
script reads GetFormationLeader every tick and always gets nothing. In the image the followers get
unit+284h, so the director's idle tail issues `follow` (00836E38 PUSH 0E08F60) instead of cruise or
stop. Test rows: the JM08, USN01, USN04, USN13 and LOMP10 scenes author no Follow token. RNG: none.
Squadron (PlaneSquadronGen) follows go to 007F1940, in the struck range, not this arm.

### R3 The `moveonpath` join takes the nearest point without 007B11F0's projection test

`build_path_object_0071f600` (`:3242-3261`) says the projection test from 007B1263 was not read and
starts the cursor at the nearest point (`ship_ai_path_nearest_index_007b1100`, a 2D hypothesis).

Image: 007B1C50 (start modes 5, 7 and 8) makes an indirect call through the source's vtable 10h at 007B1D24 (the slot is 007B11F0) with the
direction byte. 007B11F0 takes the nearest index (vt 14h); with two or more points the candidate is
nearest+1 forward (none when nearest is last) or nearest-1 backward (none at 0). With d = pos - P[nearest]
and e = P[candidate] - P[nearest] on (x, z), u = e times the 00419260 reciprocal length, s = u.d:
`007B1303 JA` keeps the candidate when s > 0, else the nearest (NaN keeps the nearest). A unit already
past its nearest point joins at the next one. The host sends it back to the nearest point, and
007ADD70 only advances on a radius hit of the cursor point, so the ship turns back. The host also
ignores the direction for PATH_SM_JOIN_BACKWARDS (8). The body is already reconstructed:
`src/gameplay_seg46_cc13.cpp:386` `start_index_for_position_007b11f0`.

Smaller, MINOR: 007B1100 -> 007B0900 -> 007AED10 is a 3D root distance in path-local space with a
strict `<`; the host's is 2D squared. Start mode 7 draws 00BD2F10 (007B1CE5); the host does not, but no
installed script passes PATH_SM_JOIN_RANDOM_DIR (only `scripts\global\luamw_init.lua:231` defines it).

Live: every NavigatorMoveOnPath with PATH_SM_JOIN (the default 5; shogo_one, shogo_three, jm03, jm05,
jm14 including its submarine patrols at `jm14.lua:1584`) and the 32 scene MoveOnPath tokens, whenever
the unit starts past its nearest point. Path completion is script-visible through the `finished`
report (00836D51) and the Lua waits on arrival. USN04's carrier `moveonpath` is in scope; not measured.

### R4 Lua `command` listeners never fire, and no "start" report is made

The host's `command_event_subscriptions` (`:313-317`) is never filled. Its label says the parser
0097E360 is not reconstructed and usn_2_java authors no `command` block; the premise is stale for other
missions. `GameMissionLuaHost::run_add_listener_008c6760` (`src/game_hosts_lua.cpp:5139`) stores every
listener, but only kill, hit, recon, entityKilled and exitzone are dispatched, and
`report_command_event_00984300` (`:2801`) reads its own empty vector. The "start" reports are missing
too: 0071F600 is a record in this host (`QueueClearBinding::begin_command_base`, `:2546`), and the only
report the host makes is the path `finished` (`:3415`), with `target_id` always 0 (`:2809`).

Image: 008C6760 -> 00980C10 -> 0097E360 builds a 0097C8A0 subscription (four std::set filters, empty =
accept all; 00968AF0 ANDs entity, target, command name and status). 00984300 walks the `command`
channel and calls each matching callback through 00887E50. Report sites: 0071F768 PUSH 00CFDB1C
("start") -> 0071F79B CALL 00984300 when [director+4Ch] is set (0071F75E), then 0071F7E8 00984800;
0071F8C0; 00836D51 (path finish); 0084E431 (squadron block, struck range); 009E595C and 009E5C35 (ship
AI states, reported through the units host).

Live: `jm02.lua:1561` and `:1601` (both copies) add `BettyRetreat` and `NellRetreat` with command
{"retreat"} and status {} for the player's G4M / G3M bombers; `luaJM2BettyRetreat` (narrative
`ijn02.betty_home`, SetRoleAvailable) never runs in the host. Not live in either build: jm06 `submove`
(`jm06.lua:1205`), jm07 `fleet1move` / `atlantamove` and jm13 `junklistener` filter status {"finish"},
and the image sends "finished" (00D09FD8 = `66 69 6E 69 73 68 65 64`), so the exact set match never
holds (the set's comparator was not read; an exact or case-folded compare both miss). Test rows: no
command listener on JM08, USN01, USN04, USN13 or LOMP10. RNG: none.

### R5 A category 1/2 push does not end an installed AutoTarget override (probable)

`DirectorBinding::override_command()` (`:1177`) answers 0, `session_field_f4h_is_0_or_1` (`:1178`) is a
record answering false, and `echo_command` (`:1182`) is a record. The push
(`src/cruise_command.cpp:318-321`) therefore never echoes. The premise is stale: with
`kAutoTargetOverrideBound` ON, director+188h is written by 0071E7F0. GUNNERY_OPEN_ITEMS 15574 called
006E38E0 unreachable only because +188h answered 0.

Image: 0071E6C0's tail `0071E7A8` category == 1 or 2, `0071E7B4` [director+188h] != 0, `0071E7C9`
006E38E0 ([[00E188A8]+1EF0h]+F4h is 0 or 1, the MMultiPlayer mode), `0071E7D4` 0071E2E0: when
[director+50h] < 2 it stores 2 and, unless [[00E188A8]+1FE4h] == 2, routes 0071C730(0, 0) with flags 7,
the 5Dh clear with +20h = 0 that 0071E610 receives. A scripted or AI attack order pushed while an
override is active ends the override in the image; the host keeps it. UNCERTAIN: the +F4h value in a
single-player mission was not read (no MP session is expected to give 0). Live where overrides are
installed (us_osumi `s56_a_c9` / `s56_aj_c9`: 66 / 60 installs) and an attack push follows. Routed to the override owners.

## Other findings

| Host | Image | Class | Note |
|---|---|---|---|
| `:58-66` kFireTargetObjectIdBound: the handle stands in for the entity pointer | 00835930 -> 00835860 | MATCH | Same entity by object id; representation only. |
| `:201-203` one user path per director | per slot object (0071FDE0) | MISMATCH MINOR | User paths come from the cautious route; `cautious route builds=0 clears=0` on all 34 logged rows. |
| `:233-236` +5Dh set at the gunnery kill, not the destroy flush | 00926390 at 00875EC9 | MATCH | Same fixed step, after every director step (docs/CC13_VER_13.md section 2). |
| `:264-267` torpedo test: unknown class id accepted | 009229F0 on the resolved entity | MISMATCH MINOR | Only a unit with no VehicleClass row (class_id -1). |
| `:287-300` moveonpath test: a descriptor naming no unit is taken as a Path | 007AC9D0 refuses a null entity (007AC9D5) | MISMATCH MINOR | Correct for authored paths; differs only for a stale handle. |
| `:640-647` owner pose always current | 0046ABA8 +C8h, 00414DB0 | MATCH | Created instances publish a valid matrix. |
| `:662-684` registry walk order not modelled | 00925A90 / 009251F0 | MATCH | Exact hits only on every logged row (GUNNERY 62 census). |
| `:717-743` 0077D600 resolve / kind / retarget records | 0077D676..0077D73A | MISMATCH REAL | Part of R1. |
| `:735-738` pooled name string | 0041E870, freed unread | MATCH | No observable effect. |
| `:755-779`, `:866-902` "nothing writes entity+16Ch" | 00A2D8E0 writes it (docs/AI_GROUP_THINK.md) | STALE LABEL | The AI host builds groups now. No effect: 00A2BD90 ends in an indirect tail jump through group+564Ch vt[24h], and that slot is the `RET 8` stub 00A0FC90 in all sixteen classes. |
| `:780-786` session player slot 0 | 0075B454 | MATCH | No session world in SP. |
| `:817-844` header `:30-32` "00720CA0 not projected" | 00721BA8 -> 00720CA0 | STALE LABEL | Bound (kSetCommandClearAllMessageBound); the OFF branch is dead. |
| `:845-861` 00835E90 "not reached" | 007207CC | STALE LABEL | Reached now by the user-path arm (`:2015`); the projection is used. |
| `:1034-1079` slots 1..9 path weight 1 | 0071D7AF..0071D7F9 per-slot path objects | MISMATCH MINOR | Only the queue-full test; counted by `queue_full_tests_queued_path`. |
| `:1084-1104` 0071E200 compares object pointers | resolved objects | MISMATCH MINOR | `register_units` (`:1607`) moves a new vector in on every spawn batch, so a slot's stored `&units[i]` dangles and a later push naming the same unit is not seen as a repeat. Repeats come mostly from scripted orders, which carry opaque handles, so the reach is small. Comparison only, never dereferenced. |
| `:1136-1147` 00836040 "endpoint is null" | 0083604F..008360B3: target resolving to [director+34h] (the unit, GUNNERY 133.2) is emptied for cruise/stop; 0071E742 refuses when index >= 1 | STALE LABEL, MISMATCH MINOR | The idle tail's cruise/stop name the unit itself (`:2313`), so the host stores a self descriptor where the image stores an empty one. The index >= 1 refusal does not arise, because the 5Dh clear is delivered before the SETCMD. |
| `:1177-1184` override / echo | 0071E7B4..0071E7D4 | MISMATCH REAL | R5. |
| `:1192-1198` heading from the pose | unit vt[50h] | MATCH | Same quantity. |
| `:1305-1314` resolve = object_id | 00521EA0 | MATCH | object_id = index+1 by construction (`game_hosts_units.cpp:17094`). |
| `:1315-1335` follow / leave / disband arm records | 00816EA6..00816F4B | MISMATCH REAL | R2 (follow). Leave and disband: no installed producer reaches this arm. |
| `:1342-1352` 007AC9D0 answers 0 | 00816FA7 | MATCH | Right for unit targets; a Path entity never reaches this host (NavigatorMoveToPos with a Path is MP-only: competitive07). |
| `:1353-1357` `settarget` arm record ("not reached") | 00816FFD 00835860(target, 1) | MISMATCH MINOR | Ship SetTarget tokens exist only in MP `multi\scene3.scn`; bsm_04 / bsm_09 SetTarget rows are squadrons, which apply through 007F1940 (struck). |
| `:1353-1384` `clearorders` arm: 00835860(0, 1), 007788B0, 0080DC70, 0071BF20 records | 008171CF, 008171D6, 008171F6, 00817205 | MISMATCH REAL, latent | The image clears the fire target (force), skips the clear for a formation follower, and calls FreeFire when +3Ch and +3Dh are both set. Only producer: the cautious route (`clears=0` on every logged row). |
| `:1385-1411` throwaway target records | 00817243..0081732E | MISMATCH MINOR | Reached only by an attackmove with no entity target; every scene AttackMove names a target, and NavigatorAttackMove passes entities. |
| `:1484-1487`, `:1525-1527` session route records | 0077C2A0, 0076C600 | MATCH | The queue is bound; the records are trace names. 00780670's tick widening is MP bookkeeping. |
| `:1761-1794` header `:33-38` where 00835C70 / 009E1170 run | vt[78h] caller not found in 00836920 | UNRESOLVED | Header's "009e1170 once per unit per fixed step" is stale: `:1809` says it runs on the re-plan tick. The vt[78h] caller is still unknown; the full 00835C70 (`command_execution.cpp:310`) is not used by this host. |
| `:1995-1996`, `:2003-2009`, `:2045-2047`, `:2059-2061` user-path labels | 0071DC80, 004BBDD0, 0071FFBF, 0071D2E0 | MISMATCH MINOR | Not reached on any logged row (cautious route). |
| `:2431-2557` QueueClearBinding records (0071D6D0, 00694A60, 0071FB90, 00984300, 00984800, 007788D0, 00521E70, 0071F600, 0071E6C0, 008358D0) | 00720850 / 00720CA0 | MATCH except 0071F600 | The clear path does not reach the others on a normal clear. 0071F600 belongs to R4. |
| `:2902-2906` log note "follow / attackmove / moveonpath arms projected nowhere" | 00836ADC..00836D66 | STALE LABEL | Log text only; the arms are bound. |
| `:3026-3052` attack-move building conversion record | 00836B80..00836BAD | MISMATCH REAL, latent | `converts=0` on every logged row (hits up to 90321 on us_osumi, 10199 on JM08); a destroyed (+5Eh) or same-side building would keep the attack-move forever in the host. |
| `:3254-3261` start mode 7 RNG | 007B1CE5 00BD2F10 | MATCH | Dead: no script passes mode 7. |
| `:3368-3370` count byte upper bytes | 00836C42..00836C76 | MISMATCH MINOR | Read only outside modes 1..3. |
| `:3419-3442` "NOT PROJECTED" one leg per step | 007ADD70 | STALE LABEL | Dead code: kPathCursorFollowStepBound and kSquadronPathFollowStepBound are ON. |
| `:3559-3563` "nothing writes director+188h" in 0071DDB0 | 0071DDD5..0071DE0D: override descriptor -> vt[70h](desc, 0) -> 0071D9E0(2) | STALE LABEL | The override is written now. The host ends it one step later through the override arm (00836D67), except for a plane target with live wingmen, which the image re-points (R1 mechanism). For the override owners. |
| `:3584-3586` no squadron member list | 0071EDD0 | MISMATCH REAL | R1. |
| `:3789-3805` player-controlled cruise arm records | 009E11E8..009E1262 | MATCH | Forwards the ring's confirmed pair, which stays 0 with no human input (docs/CONTROLLED_UNIT_HELM.md). |
| `:4030-4034` "inert until a caller wires it" | 00720CD0 from 0077FAB8 | STALE LABEL | Wired: `kFormationJoinFollowBound` ON (`game_hosts_units.cpp:6572`, `:37180`). |
| header `:196-198` one path pair per director | 00720850 re-constructs the slot's path object | MISMATCH MINOR | Only a `moveonpath` issued without 0071C1B0 after another path inherits the previous pair. |

## Not covered

- The excluded override layer: `:2584-2617`, `:3078-3100`, `:3488-3527` (0071BE40, 0071EB60),
  `:3648-3730` (0071D980, 0071D9E0, override accessors).
- The squadron apply 007F1940 and the squadron block step 0084E010 / 0084E3B5 (cc13-struck), which
  the scene tokens of PlaneSquadronGen entities reach in the image. This host routes those rows through
  00816E30 instead; that routing difference is outside this pass.
- `report()` `:3873-4029` (summary output only).
