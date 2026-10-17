# CC13_VERIFY1: plane attack-task substitutions and the director override path

Verify-only pass for the cc9 gameplay lead (worker cc13-verify1, 2026-10-17, base fe9811ba8).
No code was changed and no ledger records were added. Every line number below is against
that base. "Image" is battlestationspacific.exe as read with `bsp.py show`, `disasm-raw` on
disk bytes and constants read from the PE (`local/verify1_const.py`).

Rubric (cc13 common brief): MATCH; MISMATCH MINOR (edge guards, error paths, width with no
effect, synchronisation, ordering with no observable effect); MISMATCH REAL (wrong result,
missing or extra call, write or side effect on a normal path). "RNG" names draws on the
shared generator 00BD2F10 (ECX = stream). The host's `release_altitude_draw_00bd2f10(key)`
streams are keyed per unit and field, so every host-side keyed draw is a deliberate stream
substitution even when the distribution matches; those are listed once (A.6), not per site.

## Part B: director override path (done first; sent to team-lead)

| Function | Host | Verdict |
|---|---|---|
| 0071D980 BSP_WeaponDirector_SendOverrideCommand | `src/director_target_gate.cpp:57` `may_send_override_command`, `:79` `send_override_command` | MATCH |
| 0071D980 (second copy) | `src/gameplay_seg38_cc13.cpp:244` `send_override_command_0071d980` | gate only (see B.2) |
| 0071E7F0 BSP_WeaponDirector_SetOverrideCommand | `src/command_execution.cpp:251` `set_override_command_0071e7f0` | MISMATCH REAL (repeat test) |

### B.1 0071D980

`__thiscall(director, command, target)`, RET 8 at 0071D9DD, SEH frame 00C85108. 0071D995 XORPS
xmm0; 0071D99E `COMISS xmm0, [esi+40h]`; 0071D9A2 JBE skips the body. So the send runs only
when `0.0f > [director+40h]` and ordered; a NaN skips. 0071D9B4 `0071C830(cmd, target, 0)` on a
stack message, 0071D9B9 ECX = `[director+34h]`, 0071D9C9 `0077C2A0(msg, 7, 0)`. No callee draws
00BD2F10.

`director_target_gate.cpp` matches all of it (`kAutoTargetHoldThreshold > hold`, flag
`kSetCommandMessageOverride` = 0, route flags 7, out 0). It reads `+34h` before the build where
the image reads it after; that is a read with no side effect. This is the copy to wire.

### B.2 Disagreements between the reconstructions

- `gameplay_seg38_cc13.cpp:244` returns only the predicate (`override_delay_40 < 0.0f`); it
  neither builds nor routes the message, so it is a gate, not the function. Its comment says
  "FLD [ECX+40h]; FCOMP 0.0", but the image uses SSE COMISS. The NaN arm is the same (false).
- The two name `+40h` differently: seg38 calls it `override_delay_40`; `director_target_gate.hpp`
  calls it `auto_target_hold`, with evidence: the -1.0f sentinel stored at 00720225, 3.0f from
  the `cleartarget` arm at 00817031, and the per-frame decrement only while >= 0 (0071F2F8). The
  second reading has the evidence.
- `include/bsp/bot_fire_target.hpp:315` and `include/bsp/cc13_seg68.hpp:147` declare host
  virtuals for the call; no behaviour to compare.
- The live hosts are stubs that only record the call: `game_hosts_ship_ai.cpp:11330`
  (`AutoTarget::issue_attackmove`, 0071D980) and `game_hosts_commands.cpp:970`
  (`WeaponDirector::queue_command`, 0071E7F0).

### B.3 0071E7F0, MISMATCH REAL

`__thiscall(director, command, params)`, RET 8 at 0071E914. Reached from
00721A40 BSP_WeaponDirector_ApplyGameUnitMessage at 00721B7C when the message flag byte +20h is
0 (00721B36 CMP / JE 00721B65), i.e. from every 0071D980 send.

Repeat test, image 0071E7FD-0071E81B: `[+30h] == 2 && cmd == [+188h] &&
0071E200(ECX = new params, arg = director+18Ch)` returns. 0071E200
BSP_CommandSlotTarget_Matches requires the same object resolved by 00521EA0 on both sides AND a
squared distance <= 1.0 between the two descriptors' positions. A descriptor whose byte +1 is
clear uses the zero vector 00F87574. The distance is an x87 FSUB/FMUL chain from 0071E27B.

Host `command_execution.cpp:253-256` tests only `resolve_target(a) == resolve_target(b)`:
- Same entity with positions more than 1 unit apart: the image re-installs it. That means the
  observer unregister/register (0071E895 / 0071E8EE), mode 2, +4Ch = 0, +50h = 0 and
  vtable[6Ch](0) at 0071E90E. The host refuses it as a repeat.
- Two positional targets (both resolve to 0): the host treats every new position as a repeat
  and refuses it. The image installs it when the positions are more than 1 apart.

LIVE: yes once wired, on every override send (AutoTarget attackmove). Today both ends are
stubs. RNG: none on either side (callees 0071E200, 0071D6D0, 00521EA0, 00694A60, 006952A0).
Fix: call `slot_target_matches_0071e200` (`src/gameplay_seg38_cc13.cpp:149`) with
`same_resolved_object` taken from the two resolves.

The rest of 0071E7F0 MATCHES:
- the 0071D6D0 accept gate at 0071E825;
- the attackmove head check at 0071E832-0071E857. It compares 0xE08F78 with the class at
  `[+54h]` and compares the resolved `[+58h]` target with the resolved new one. A mismatch
  returns; a match skips the category test.
- the category test for any other non-null head at 0071E862: category 1 or 2 returns;
- the 18h-byte params copy at 0071E89E-0071E8D4. It copies the floats by FLD/FSTP, which only
  quiets an sNaN, so that part is MINOR.
- the tail at 0071E8FC-0071E90E.

Hook note: 006952A0 and 00694A60 receive EDX = director+1Ch. The host hooks take only the
target, so the implementation must supply director+1Ch itself.

## Part A: plane attack-task arms in `src/game_hosts_units.cpp`

Scope: dogfight (009AB1C0 and its gun 009FC7C0, rocket 009FD0E0), strafe (009CD170), kamikaze
(009AF510), drop-kamikaze (009AE8A0) and torpedo (009D4850 with its issue stage 007CE9FD). The
dive, level and torpedo RETIRE rules were excluded as instructed. Labelled substitutions were
found by a keyword sweep (`local/verify1_subs.py`) and then read in context.

### A.1 URGENT: torpedo and level-bomb release issue interval (sent to team-lead at once)

`game_hosts_units.cpp:28534-28537`, `run_release_issue_stage_007ce9fd`, sets
`in.interval_draw = bsp::kIssueIntervalLow_00ce3860` (0.9) with the comment "the host's
deterministic draw takes the low end".

Image 007CEA85-007CEABE (inside 007CE040, right after the 007C0D90 issue at 007CEA8D and the
+910h decrement) does `ECX = 1`, pushes 1.1f (00CE6448) and 0.9f (00CE3860), calls 00BD2F10,
then `FMUL [class+1F4h]` (BombDelay) and `FSTP [unit+918h]`. The next-issue interval is
U(0.9, 1.1) x BombDelay in the image and always 0.9 x BombDelay in the host.

- **MISMATCH REAL.** Multi-issue sequences run about 10% faster on average and never slower.
- **LIVE:** yes, for every plane whose +910h count is non-zero after an issue: level-bomb
  sticks, and multi-round racks.
- **RNG:** the image draws once per issue on stream 1; the host draws nothing here.

### A.2 Dogfight (009AB1C0, 009AAC70, 009AA630, 009FC7C0, 009FD0E0)

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| D1 | 21131-21133, 21255 | reselect timer `+E4h` fixed at 2.0, "midpoint" | 009AA630: `+E4h = 00BD2F10(1, 1.0, 3.0)` (00CE3854 = 3.0) at every select | REAL: the reselect cadence is fixed, not U(1, 3) | yes, every dogfight select | image 1 draw per select on stream 1; host none |
| D2 | 21310-21311 | non-current candidate factor fixed at 0.9 | 009AA630 tail: `score *= 00BD2F10(1, 0.8 (00CE74F8), 1.0)` for each scored candidate that is not the current `+B4h` | REAL: the target choice changes when scores are within 20% | yes, squadrons of 2 or more | image 1 draw per non-current live in-map candidate; host none |
| D3 | 21296-21302 | wingmate count from the wingmates' dogfight target | 009AA630 inner loop: 007BBC10 BSP_Unit_PilotTargetIs (the pilot target), 0.7 (00CEFFA0) per hit | label consistent; MINOR where the pilot target and the dogfight target agree, REAL otherwise (gun-target adoption 009A797F can make them differ) | yes | none |
| D4 | (unlabelled) 21283-21289 | candidates filtered by `df_slot_live` only | 009AA630 also skips candidates where 0071C4F0 BSP_Game_PointOutsideMapBounds is true | MINOR (only planes outside the map bounds) | rare | none |
| D5 | 21169-21171 | aim point `approach+48h` = target origin | 009AAC70 calls 009FADA0 BSP_BotApproachTargetRef_Update then reads approach->vtable[0] | label consistent; not resolved here (the target-ref pick for an aircraft target was not read) | yes | unknown |
| D6 | 21185-21187 | approach+24h floor 1.0 | 009AAC70 `+24h` read in `tuning+644h * +24h` | label consistent, per the torpedo/dive-bomb binding note | yes | none |
| D7 | 21933-21935 | distortion 009FA7E0 and damping "off" | superseded: `kFighterAimTermsBound = true` (7237) runs both (21966-21990) | stale label; code models it | - | - |
| D8 | 21961-21965 | cone gate uses the unit's forward, not gun+68h | 009FC829-009FC871: when gun+68h..+70h are all zero, +68h is set from row 2 of `[X+2F4h]`'s world matrix (0042D7E0); 009FCE5A-009FCE64 zero it every tick. But 26 state ticks write it through 009FABE0 on approach+1Ch, the gun object (009A71E0 dogfight attackrun at 009A747C with ECX = `[approach+1Ch]`, 009C18C0 moveto, 009BEE30 follow, 009B5760 level-bomb goaway, 009C4A40 dive goaway, the rocket states, ...). The comment "the dogfight task never writes it" is wrong for 009A71E0 | REAL where a state wrote +68h and the cone is non-zero (the task-cone path: the strafe goaway/aim cones 0x674/0x678): the image cones the lead against the commanded direction and the host against the nose | yes (strafe, any task-cone state) | none |
| D9 | 22266-22269 | fire-branch re-aim not modelled | 009FCDB6-009FCE01: when fine aim is enabled (BL), 007BA760 is false, 007B96D0 (a friendly is in line) is true and gun+49h is set, the image clears +4Ah, adds 0.25 (qword 00D7A348) to an angle and calls 009F9FC0 again | REAL: the steering differs while a friendly is in the line of fire | yes (`kFighterFriendlyInLineBound` on) | none |
| D10 | 22210-22212 | gun `+74h` from the finder (unread) | superseded: `kPlaneFinderBound = true` (6459) runs 007E2090 (22224-22227) | stale label | - | - |
| D11 | 22320-22324 | near-field probe scans every aircraft, not the 3 s-refreshed 1080 m list | 007F0280 in mode 1, box at most 140 m | MINOR (no plane closes 940 m in 3 s) | yes | none |
| D12 | 22485-22490 | finder cadence = think interval | not read (007E2010 cadence) | label consistent, unverified | yes | none |
| D13 | 22597 | follow wait `state+85h` taken as clear | 009BE3E0 returns 1.0 when +85h is set; 009BFD70 sets +85h = 1 when the member is within `[row+18h]` of its station and its forward dot product with the leader's exceeds `[row+14h]` (009BFE95), else 0 | MINOR: in station the host formula also tends to 1, but values differ at the edge | yes (dogfight follow) | none |
| D14 | 22662-22666 | 009AAA80 forced false | superseded by `kDogfightEarlyEdgeBound = true` (6471, 22670-22676) | stale label | - | - |
| D15 | 22883-22898 | avoid stand-in | dead code: `kDogfightAvoidBodiesBound = true` returns first | unreachable | no | - |
| D16 | 22911-22934 | maneuver mode-0 stand-in for attackrun/prepare | reached only for kAttackRun/kPrepare, which the label says are unreachable in mode 1 (not re-proved here) | label consistent | no (per label) | - |
| D17 | 22938-23022 | moveto stand-in | `kDogfightMovetoGenericBound = true`: the generic 009C18C0 glide (ranges 500/100/1000 from 009A955B) and 009FBA50; the heading stays 009F9E40 toward the target, which 009C18C0 also calls | label consistent; `cin.has_squadron = false` unverified | yes | none |
| D18 | 22063-22070 | rocket "attached" = rack has rounds | 00923810 is the entity-ancestor walk (`+3Ch` chain): the rocket is still parented | MINOR (a representation difference) | rocket-armed fighters | none |
| D19 | 22092-22095 | target velocity: ships give 0 | 009FD0E0 asks the target's vt[34h] | REAL against moving ships (the strafe-set rocket target, 009CA80E) | rocket-armed strafers | none |
| D20 | 22100-22110 | rocket flight time from fixed R4M/HVAR values (800/1000 AA, 500/500 otherwise) | 00809E10 reads the rocket's class | label consistent; the per-class values were not checked against the bullet classes | rocket-armed planes | none |
| D21 | 22126-22133 | 007BBBA0 spawns the rocket at once | 007BBBA0 (read) only raises device+61h/+68h/+11h and increments unit+C20h; the launch happens later through 007CE9FD -> 007C0D90 | REAL: one step or more earlier, and the rack is ASSUMED | rocket-armed planes | the image's issue stage draws (A.1) |
| D22 | 22173-22175 | rocket target cleared at the next rocket tick after the task leaves | the image clears it in the task Stop (007B7BA0 / 009AA9E0 / 009CD0D0) | MINOR (one tick late at most) | yes | none |

Superseded midpoints: the maneuver expiry, the avoid pick, the gun burst and the enter draws
are on keyed streams (`kDogfightTurnAfterDrawBound`, `kDogfightAvoidPickDrawBound`,
`kDogfightGunBurstDrawBound`, `kDogfightEnterDrawsBound`, all true). D1 and D2 are the two
dogfight draws still at midpoints. The comment at 21131 ("Draws are fixed at their midpoints")
is now true only for those two.

### A.3 Strafe (009CD170, 009CC230, 009CA3B0, ...)

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| S1 | 23096-23116 | glide seed alt = script attack alt, else Pilot/Strafe/CruisingAlt; "cadence unread" | 009CA3B0 reads `[[approach+0Ch]+398h]`. The cadence is read: 009CD020 is strafe vtable +54h (00D21134), called from 0099ACD0 at 0099AEB3 on every head-task change (0099AE92 compares old and new heads), leader only (009CD02E 007B8AD0), and it writes ctl+394h/+398h only when unset (+38Ch/+38Dh, +37Ch/+380h < 0, +3A9h/+3AAh) | the alt substitution is consistent. The cadence is already hosted (`kStrafeProfileActivationSeedBound`, 6308; the flag is re-set on manoeuvre retire at 23348), so the "unread" remark is stale. The +394h/+398h gate flags and the every-unit `009CCED0(0)` at 009CD0BB are not hosted: MINOR | yes | 2 stream-1 draws per activation in the image; keyed in the host |
| S2 | 23176-23177 | entry state's vt[4] (moveto/follow enters) not modelled | the follow enter 009BED80 writes +85h (009BEE1C) and squadron+3E4h = 1 (009BEDDA) and re-runs 007ED260 (see the DKZ formation note at 6220) | MINOR to REAL (formation shape on strafe members; not measured) | yes | unknown |
| S3 | 23213-23216 | the target-point pick runs at the next aim query; "007B7870 tail unread" | the tail is now hosted under `kOpportunisticRocketBound` (23233-23241), with the commanded target standing in for the ref | stale half; the target-ref substitution stays | yes | the host's pick seed is deterministic |
| S4 | 23244-23247 | manoeuvre suspends the arm | consistent with 00999F50 front-push / 0099ACD0 head-only tick | MATCH | yes | - |
| S5 | 23320 | 009BA1C8 `+2E4h &= ~4` not modelled | 81 A6 E4 02 00 00 FB 00 00 00 (AND +2E4h, 0FBh) in every manoeuvre tick: 009BA1CB TightTurn, 009B9C86/9D2D/9D69/9E29 FlikFlak, 009B9752 BarrelRoll, 009BA300 GainAltitude. 00999950 sets +2E4h = 0FFh, the arm runs at 0099995A, and 009A17D0 runs after it at 0099999B and tests bit 4 at 009A18BA before 0099EC40 BSP_PilotBot_AvoidGunfire | **REAL**: during every strafe breakoff manoeuvre the image skips gunfire avoidance; the host (`task_mask_2e4` stays 0FFh) runs it, inserting avoid bands (0099B790) and drawing | yes (strafe breakoff under fire; `kPilotGunfireAvoidanceBound` on) | 0099EC40 draws 00BD2F10; host-only draws during manoeuvres |
| S6 | 23386 | "GAP: the manoeuvre tasks are not modelled" | superseded by `kStrafeBreakoffBound = true` (23387-23392) | stale label | - | - |
| S7 | 23455 | moveto/follow/prepare enters not modelled | as S2 | as S2 | yes | unknown |
| S8 | 24881-24887 | a new task per order (0099A170); the host re-installs only on a new target | label consistent | MINOR to REAL (a repeated order on the same target resets state and redraws in the image) | depends on scripts | the image redraws the install draws |
| S9 | 24936 | target+5Dh replaced by host liveness | the image reads only +5Dh; the host `!df_slot_live` is also true for +5Ch clear or +5Eh/+60h set | MINOR (destruction edge) | yes | none |
| S10 | 23031 | every stream-1 draw on keyed per-field streams | by design | RNG stream substitution | yes | all strafe draws |

### A.4 Kamikaze (009AF510) and drop-kamikaze (009AE8A0)

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| K1 | 25004-25006, 25129-25131 | gun block +40h write not carried | the cc13 reconstruction only CLEARS +40h (`clears_gun_block_40`, 009AD58C), and the host zeroes `gun_cone_40` every tick (34134, as 009FCE69) | MATCH in effect | - | - |
| K2 | 25004-25005, 25265 | gun block +34h/+38h clear is an empty hook | `kamikaze_task_cc13.cpp:524`: the image zeroes (approach+1Ch)+34h/+38h at install. Those are the finder's near (`+34h * 0.3`) and inner (`+38h`) arguments and the fire envelope's shoot distance. The host's kamikaze is not in `has_task`, but `kTaskGunUntaskedPlanesBound = true` (5799) still runs 009FC7C0 for it with the `df_row` defaults | **REAL candidate**: the host kamikaze may acquire and fire where the image's zeroed envelope would not. The fire envelope with +34h = 0 was not read; confirm before fixing | kamikaze missions | none |
| K3 | 25297-25306 | profile +54h (009AF480) "run once" at install | 009AF480 is vtable +54h, so the indirect call in 0099ACD0 reaches it on every head change (as S1). Its own callees are 009AB850 BSP_BotApproachKamikaze_RedrawAltitudes, 009AF0A0(0) and 0099B740 | MINOR to REAL: it re-runs only after a front-pushed task retires (not measured for kamikaze) | when kamikazes get pushed manoeuvres | the image redraws in 009AB850 on re-activation |
| K4 | 25343-25346 | new task per order | as S8 | as S8 | | |
| K5 | 25419-25423 | Ohka/Funryu release collapsed into one think; kinematic round | 007BBBA0 (read) only raises the request; the launch is 007CE9FD -> 007C0D90 later | REAL (release at least one step early; no gunnery projectile or water blast) | Ohka missions (USNOS) | the issue stage's U(0.9, 1.1) draw (A.1) is skipped |
| K6 | 25679-25680 | unit+C25h is one think wide | the only byte writer found is 007C0EE2 (`= 1`, in 007C0D90). The host's torpedo binding models the clear through the issue stage (`clear_release_pending_c25`, 28554). Readers: 009AD668 (DKZ release tick), 009AE539 (DKZ EnterEngagedState), 009C19A1 (moveto), 009C1FFD (follow), 007EFFE6 (PromoteArmedLeader), level-bomb 009B5AC9/009B7CBF/009B8835 | MINOR to REAL: the DKZ pulse is shorter than the image's pending window; the issue-stage clear condition was not traced for DKZ | Ohka missions | none |
| K7 | 25203-25205 | stream-1 draws on keyed `#kz` streams | by design | RNG stream substitution | yes | all kamikaze draws |

### A.5 Torpedo (009D4850, 009D0380, 009D15F0, 007CE9FD)

| # | Host line | Label / behaviour | Image evidence | Verdict |
|---|---|---|---|---|
| T1 | 28534-28537 | issue interval draw at the low end | see A.1 | **REAL, URGENT** |
| T2 | 28538-28545 | "class+1F4h unread, scale 1" | superseded: `kPlaneFixedStepPredicatesBound` feeds `plane_bomb_delay_1f4` (BombDelay) | stale label; MATCH |
| T3 | 28484-28490 | app state 1FE4h = 0 | single player, value not stand-in | MATCH |
| T4 | 27954-27986 | robots row by skill index; ReferenceSpeed divisor authored KMH(300) | 009D0484-009D0497 read the PilotBot row and 009F9D30 the ratio | label consistent; the values are authored, so it matches when the row index is right |
| T5 | 6234-6237 (`kTorpedoDoneDisarmedArmBound`) | `[approach+1Ch]+40h = 0` has no host cell | the host zeroes `gun_cone_40` every tick (34134) | MATCH in effect |
| T6 | 6233 | done-disarmed arm: aim point = target position | approach->vtable[0] | label consistent; not resolved |
| T7 | 29816-29828 | desc+26Ch "stays 0, unread" | 007C4850 BSP_PlaneClass_DeriveFlightConstants: 007C4C27 `+26Ch = r` (007DB4D0) and 007C4C4B `+268h = max(1.1 r (00CE3DF0), tuning+54Ch)` (00415550 max). 009D1946-009D1970 take `max(+26Ch, +268h)`, which is always +268h for r >= 0 | MATCH in effect (the label's premise is wrong: +26Ch is not 0, but it never wins) |
| T8 | 28050-28140 | reset draws (`#t70/#t78/#t7c/#t80/#t88/#t12c`) | 009D03D9-009D0625 draw on stream 1 | RNG stream substitution only |
| T9 | 28458-28459 | "both gates fed by the unread 009D3420" | not re-read here | open (see docs/TORPEDO_* contracts) |

No mismatch was found in the torpedo goaway or the aim tick beyond T1. The goaway range
29564-29756 carries no labelled substitution.

### A.6 RNG stream summary

Image draws on 00BD2F10 that the host takes on keyed per-unit streams (deliberate):
- the dogfight expiry, avoid pick, gun burst and enter draws;
- all strafe, kamikaze, drop-kamikaze, level-bomb and torpedo-reset draws.

Image draws the host does not take at all:
- the dogfight select timer and non-current factors (D1, D2);
- the issue-interval draw (A.1);
- the redraws on profile re-activation after a pushed task (K3);
- the draws inside 0099EC40 that the image skips during manoeuvres while the host takes them (S5).

## Mismatch list

REAL:
1. A.1 / T1, 007CEAB3: the issue interval fixed at 0.9 instead of U(0.9, 1.1). URGENT (torpedo, level-bomb).
2. B.3, 0071E7F0: the repeat test lacks 0071E200's position check.
3. D1, 009AA630: the reselect timer fixed at 2.0 instead of U(1, 3).
4. D2, 009AA630: the non-current factor fixed at 0.9 instead of U(0.8, 1.0).
5. D8, 009FCCA0: the gun cone is measured against the nose, not the state-written gun+68h direction.
6. D9, 009FCDB6: the friendly-in-line re-aim (+0.25, second 009F9FC0) is missing.
7. D19, 009FD0E0: the rocket lead uses zero velocity for ship targets.
8. D21 and K5, 007BBBA0: the rocket and Ohka launch at once instead of through 007CE9FD -> 007C0D90.
9. S5, 009BA1CB and siblings: manoeuvre ticks do not clear +2E4h bit 4, so the host runs gunfire avoidance during strafe breakoffs.
10. K2, 009AF720 install: the gun block +34h/+38h is not cleared for kamikazes (candidate; confirm the envelope).

MINOR, or MINOR to REAL and not measured: D3, D4, D11, D13, D18, D22, S1 (the gate flags and
`009CCED0(0)`), S2/S7, S8/K4, S9, K3, K6.

Stale labels (code already binds what the comment says is missing): D7, D10, D14, S3 (half), S6,
T2, and the "unread cadence" in S1.

Not resolved here: D5, D12, D16, D17 (`has_squadron`), D20, T6, T9.
