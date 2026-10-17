# CC13_VERIFY10: torpedo aim and the remaining plane files

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify10 on 2026-10-17 against
base 3aa6d7d3e. It continues CC13_VERIFY8 ("Not covered"): the torpedo aim, approach, first-release and
release-order files, `dive_bomb_aimdive_tail.cpp`, the dive-bomb approach update 009C7A80 as GHU feeds
it, and the plane files VERIFY8 checked for labels only. The torpedo aim tick's host binding in GHU was
read with them, because the labels sit there. No code was changed and no ledger records were added.
Line numbers are against the base. "GHU" is `src/game_hosts_units.cpp`. "Image" means
battlestationspacific.exe, read with `bsp.py disasm-raw` and `tools/pe_const_read.py`.

Excluded, as briefed: `src/squadron_free_attack.cpp`. 007EE7F0 and 007EEF30 (in
`torpedo_release_orders.cpp`) lie in the struck 007EE000-007F2FFF range and were not re-read. Items
already in CC13_VERIFY1 / VERIFY8 / CC13_VER_6 are not repeated.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width with no effect, presentation, or RNG
  draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Census

The VERIFY8 keyword sweep, with `record\(|uncertain|taken as|PARTIAL` added, matched only 3 lines in
the 18 listed files. Two are labels: `torpedo_approach_update.cpp:583` and `plane_ai_control.cpp:600`.
`bot_tasks.cpp:162` is a note. The substitutions live in the GHU bindings, so the torpedo aim binding
(GHU `:29560-29724`, `:30121-30260`), the approach binding (`:20396`), the wanderer binding
(`:35917-35994`), the death-mode binding (`:18750-18800`), the wire-rope gate (`:19168-19187`) and
009C7A80's feed (`:4326-4654`) were also read. Every header in the list went through
`tools/const_width_sweep.py --load-sites`: 95 constants checked, 0 A-WRONG, and 23 A-harmless (each is
float-declared and loaded as a double, and that double is the same float widened).

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | GHU `:29663-29666`: the aim binding's sector probe returns no hit and runs no probe (URGENT, torpedo) | The image's 007F0280 probe at instruction 009D1A94 runs in mode 0 (the squadron members; the arguments are pushed from 009D1A52). The 009D1AC9-009D1B7A fold adds the sidestep to the aim heading | every torpedo aim with +AAh set (open water) and a squadron member in the box | none |
| R2 | approach+A8h is never written (`include/bsp/torpedo_approach_update.hpp:181`), so `torpedo_approach_update.cpp:478-487` never runs (URGENT, torpedo) | 009D15DE sets +A8h on aim entry. At each replan, 009D3688-009D36F3 set +A9h from 007DF360 (a ship in the line of fire), and the aim floor rises from 5 to 30 | torpedo aims with an escort or another ship between the plane and its target | none |

### R1 The torpedo aim's squadron sidestep probe is not bound

`TorpedoAimTickHost::sector_probe_009d1a94` in the aim binding (GHU `:29663-29666`) returns
`TorpedoAimSectorProbe{}`. The probe never reports a hit, and the binding has no label. The law at
`torpedo_aim_tick.cpp:209-233` is complete.

Image, 009D15F0:
- 009D19AE `CMP byte [approach+AAh],0`. When it is set, 009D1A01 computes
  cone = interp(1.5, DEG50, 3.0, DEG20, time), and 009D1A0C continues only if cone > |turn|.
- 009D1A16-009D1A46 build the box: extent = 72 x time, min(0.7 x extent, 150) (00CEFFA0, 00CE3DD8 /
  00CE3808) and 1.5 x extent (00CE3D78).
- 009D1A52 `PUSH 0` is the last argument, so this is mode 0. 009D1A94 `CALL 007F0280` has the same
  six-argument shape as the attack-run call (009D07FE-009D0857), and the weights are zero.
- 009D1A99-009D1AC9: a hit is |out x| > 0.05 (00D7A270). 009D1ACF-009D1B64 fold `-x * a * b` through
  the 0.03 / 0.97 band (00CEB690, 00D1FEC0, 00D1FEC8, all float values widened). The result is scaled
  by interp(1.5, 1, 2.5, 0.25, time) x 00D05A28 (DEG80 as a float). 009D1B7A adds it to the command.

Host: the image's out values never arrive, so `f48` stays 0 and the aim heading is the bearing plus
the sector turn alone. `nf_probe_squadron_007f0280` (GHU `:22725`) is the mode-0 host already used for
009D0857 under `kAttackRunSquadronProbeBound` (ON, `:6401`), so the binding is ready to use.

- **MISMATCH REAL.** A torpedo plane in aim never sidesteps a squadron member inside its probe box.
- **LIVE:** +AAh is set whenever all 36 sectors are clear (`torpedo_approach_update.cpp:288`, open
  water, the normal case). That exposes every torpedo aim of a squadron of two or more (USN04 and any
  torpedo row). The hit frequency was not measured.
- **RNG:** 007F0280 makes no 00BD2F10 call in its first 400h bytes. None on either side.

### R2 approach+A8h is never set, so +A9h stays 0

Image:
- The aim enter 009D15D0 runs 009D15D6 `CALL 009D02A0` (the host has it, GHU `:28643-28652`), then
  009D15DE `MOV byte [approach+A8h],1`, 009D15E7 `MOV byte [state+24h],0` (the release timer's enable
  byte) and `[state+2Ch],0`. The exit 009D1350 clears +A8h. The only other writer, 009D12F0, has no
  reference (no dword match, no Ghidra xref).
- In 009D3420, at each replan: 009D3688 `CMP byte [+A8h],0` / `JE` skips the block. Otherwise 009D36A9
  `0041BC20` terrain > 10.0 (00CE38B8) sets +A9h. Then, when unit+C50h is set, 009D36EE
  `007DF360(unit+C50h, target, aim point)` and 009D36F3 `MOV [+A9h],AL` overwrite it outright.
- 007DF360 (decompiled export) returns 1 when another kind-6 unit in the +C50h neighbour list, not the
  target, lies between the aim point and the plane, inside its +538h->A0h radius scaled by 00D06BA0.
- The aim tick then floors at 30 (00CE38C8) instead of 5 (009D1613-009D163F). The floor feeds the
  release threshold (009D1D5A) and the dive command (009D1E8E).

Host: `scan_enabled_a8` (`include/bsp/torpedo_approach_update.hpp:181`, commented "read only") has no
writer in `src` or `include`. `torpedo_approach_update.cpp:478-487` therefore never runs, and
`over_land_a9` stays 0. The block has two latent defects as well:
- GHU `:20396-20400` `target_reachable_007df360` is `unimplemented` and returns false.
- `:485-486` ORs (`? 1 : a9`) where the image overwrites, so a 0 result does not clear a terrain 1.

- **MISMATCH REAL.** With a ship screening the target, the image holds the 30 m floor through aim and
  the host descends to 5 m.
- **LIVE:** torpedo aims with an escort between the plane and its target (USN04 and other torpedo
  rows). The frequency was not measured. The terrain arm matters only over land.
- **RNG:** none.

## Other findings

| # | Host | Verdict | Note |
|---|---|---|---|
| C1 | GHU `:29688-29692` "009FA3EA draws between timer+4h and timer+8h, both unread on this object"; `reseed_draw = 0.0f` | STALE LABEL; MISMATCH MINOR (RNG) | 009D2DA0 writes the aim state's timer: 009D2F34 `[state+1Ch]` (timer+4h) = 0.5 (00CE3800) and 009D2F44 `[state+20h]` (timer+8h) = 1.0 (00D7A24C). The image re-arms U(0.5, 1.0) after each release; the host re-arms 0. That makes no difference while `torpedo_releases < 1` (`:29687`) blocks a second drop. **RNG:** the image takes one stream-1 draw per torpedo release (009FA3DF `ECX = 1`); the host takes none. |
| C2 | GHU `:29680-29687` 007BB110 stands in as "carries a torpedo and `torpedo_releases < 1`" (labelled) | UNRESOLVED | `torpedo_releases` is never reset (only `++` at `:3594`), so each host plane drops once per mission. That matters only for multi-torpedo carriers, or for a re-armed plane if rearming is ever hosted. |
| C3 | GHU `:29693-29694` `slot_byte_9c0_clear = false` (labelled) | UNRESOLVED, minor | The notify byte timer+0Dh (state+25h) is never set. No reader was found: scan-bytes for `80 7x 25 00`, `8A 4x 25` and `80 Bx 3D 03 00 00 00` in 009C/009D gave nothing, and 009D31B0 and 009D4030 do not read it. MSVC shapes outside those patterns were not scanned, so this negative is not conclusive. |
| C4 | GHU `:28641-28653` aim entry; `:30217` `state_flag_24 = false` | MINOR (sent with R2) | The host never clears `torpedo_aim_timer_enabled_0c` on aim entry (009D15E7). A plane that armed the timer, left aim without dropping and came back would drop on its first aim tick, without the five-flag chain. `state_flag_24 = false` stands for that byte. It matches in effect, because the arm refuses a re-arm (009D2279). |
| C5 | `torpedo_aim_tick.cpp:226` `v * 1.39626f` | MISMATCH MINOR | 00D05A28 is the double 1.3962634801864624 (DEG80 as a float). `1.39626f` is 29 float ulps below it. The line is dead until R1 is bound. |
| C6 | `torpedo_aim_tick.cpp:22-26` `clamp_listing` "a NaN takes the same arm" | MISMATCH MINOR (NaN only), wrong comment | At 009D1EAE / 009D1EC8 (and 009D1D82, 009D1DA8, 009D213C) `FCOMI lo,v` / `JBE` keeps v on unordered, so a NaN passes through. The host returns `lo`. |
| C7 | `torpedo_approach_update.cpp:560-607` `torpedo_aim_tick_009d15f0` "coverage: partial" | label on dead code | It has no caller (GHU runs `torpedo_aim_tick_full_009d15f0`). It is harmless, but the old floor and throttle there do not match the image. |
| C8 | `torpedo_approach_update.cpp:121` `if (closing != 0.0f)` | MISMATCH MINOR | 009D3D03-009D3D07 divide unguarded. At zero closing speed the image stores 30 (inf) or NaN; the host keeps +98h. |
| C9 | `torpedo_approach_update.cpp:30-32`, `:374` | MATCH | Both guards are unreachable: the speed is > ceiling >= 0 in one, and radius >= 450 gives a divisor >= 50 in the other. |
| C10 | `include/bsp/torpedo_release_orders.hpp:229` `kReleaseOrderLeaderBlockBound = false` | already reported (CC13_VER_6 3b) | In the struck range; not re-judged. |
| L1 | GHU `:4519-4521`, `:4564-4566`, `:4588-4590` "the aim point this host carries is the commanded target's position" (SUBSTITUTION) | STALE LABEL | Since packet cc8_hull_aim_point, `tp` is the 009FADA0 hull aim point (`:4480-4483`), which is the image's approach aim point. |
| L2 | GHU `:4613-4616` "this host draws none of 009C3DA0 ... SUBSTITUTION, the draw's mean, 0" | STALE LABEL | `kAimErrorDrawBound` is ON (`:2342`), and +C8h is drawn at `:26231` / `:26464`. |
| L3 | GHU `:4470-4483` 009FADA0 runs only with a target | MISMATCH MINOR | 009C7A9F calls 009FADA0 on approach+30h before the 009C7B03 target test. |
| L4 | GHU `:4555-4583` leader read | MATCH, edge MINOR | 009C7C5D reads `[ctl+3D0h]` directly. The host takes the first resolved registry member, so it differs only when member 0 is unresolved. 009C7AB3-009C7AF6 (the +A8h ceiling, `min(+A8h, +39Ch - 0.05 +A8h)`) and the latch 009C7C19-009C7CFE match. |
| P1 | `plane_ai_control.cpp:598-601` "0099D8DD stores XMM3. Its producer was not traced ... PARTIAL" | STALE LABEL, MATCH | XMM3 = 1.0 (00D7A24C) is loaded at 0099D618, which every path to 0099D8DD crosses: the only earlier jump past code, 0099D5BE `JNP`, lands at 0099D602, and 0099D3C2 is the centred arm's RET. The only other write before 0099D8DD, at 0099D706, is also 1.0. The host's 1.0 is right. |
| P2 | `plane_ai_control.cpp:341-343` `if (turn_roll != 0)` | MISMATCH MINOR | 0099E49A divides unguarded. TurnRoll is authored non-zero. The floor product 0099E4DC is subtracted unrounded, so the result is 1 ulp off at 53 bits only. |
| P3 | GHU `:19183` `6.944444179534912` | MISMATCH MINOR | 00D06878 is the double 6.94444465637207, 1 float ulp higher. The gates and order at 007DBF28-007DBFDE otherwise match. |
| P4 | `bot_tasks.cpp` | not live | None of its functions is called outside the file. The land profile 009B3C60 (`plane_squadron_host.cpp:419`) is a separate implementation. |
| P5 | `plane_release_ver_cc13.cpp:48-65` `host_bomb_release_velocity`, `host_torpedo_release_velocity` | not live | No callers. `rack_round_release_velocity_006e4d50` is live (`kBombReleaseVelocityFrameBound` ON, `game_hosts_gunnery.cpp:528`) and was audited in CC13_VER_20. |
| P6 | GHU `:35920-35923` wanderer draws on the keyed `#wander` stream | MATCH; RNG stream substitution | The image draws on stream 1: the construct at 007C457D / 007C4616 (`ECX = 1`), then on every timer expiry the timer (007BE186), 00439950's two draws and the roll draw. That is four stream-1 draws per active plane per expiry, which shift every later stream-1 consumer in the image. The host takes none on the shared stream. The gate at 007BE0C6-007BE0FF matches. |

## Unlabelled arithmetic spot checks

All of these were read against `disasm-raw` bytes. The constants were read with `pe_const_read.py`.

| Function | Host | Verdict |
|---|---|---|
| 009D1D54-009D1DED release threshold | `torpedo_aim_tick.cpp:55-68` | MATCH. The doubles 00CE3D48 / 00CEC9E0 / 00D7A3A0 are float values widened. The NaN arms are C6. |
| 009D18F9-009D19A9 turn-radius escape | `:175-195` | MATCH. A NaN radius picks +268h in the image and +26Ch in the host (NaN only). |
| 009D1DED-009D1EDD pitch denominator and clamp | `:267-288` | MATCH. 00D7A208 is -0.0, CF87C8 is 2.5, CE380C is 1.5. 00D057E0 is the float 00D05B40 widened. |
| 009D1F75-009D2027 aim solution | `:91-104`, `:303-305` | MATCH. The F=20h slot holds the speed, then the envelope. |
| 009D202C-009D22D9 five-flag chain, ground arm, countdown | `:308-354` | MATCH. 009D2239 also tests [00E188A8] and +19CCh before the ground probe (null guards). |
| 009FA3A0 release timer | `torpedo_first_release.cpp:7-38` | MATCH (apart from C1 and C3) |
| 009D3445-009D3487 speed ceiling | `torpedo_approach_update.cpp:29-35` | MATCH |
| 009D3950-009D3AA2 sector slope and segment tests | `:364-393` | MATCH. A NaN radius takes the slope arm in the image (NaN only). |
| 009D35FF-009D361E, 009D3770 in-range latch | `:69-81` | MATCH. 00CE3DF0 is the float 1.1 widened. |
| 009D3CB4-009D3D5F engagement ETA | `:110-127` | MATCH apart from C8 |
| 009C5DB8-009C6060 aimdive tail | `dive_bomb_aimdive_tail.cpp:25-91` | MATCH. It also checks the +288h / +2D4h writes (GHU `:28040-28047`), the x1 reload at 009C5FA7 and the constants 00CE7638 / 00CE3DB0 / 00CE3938 / 00D1F3F8 / 00D7A238. |
| 009C55E5-009C5651 aimglide throttle and brake | `:130-144` | MATCH |
| 009C7A80-009C7B0D approach head | GHU `:4329-4464` | MATCH apart from L3 |
| 007C1900-007C19D4 pitch and the cross guard | `plane_attitude_angles.cpp:23-50` | MATCH |
| 007CA8A0-007CAAB8 death mode | `plane_death_modes.cpp:19-37` | MATCH. The draw is taken before the +DFCh test. A NaN health continues in the host and returns in the image (NaN only). |
| 007DAB52-007DABE0 control rate law | `plane_control_rate.cpp:57-62` | MATCH. A, B and C are the globals 00F872FC / 00F87300 / 00F87304. |
| 00419519-0041955E normalise, 0085DDFB-0085DE32 projection | `plane_pose_commit.cpp:32-41`, `:81-85` | MATCH. The dot is kept unrounded on the stack (1 ulp at 53 bits only). |
| 007BE0B8-007BE0FF wanderer gate | `plane_wanderer.cpp:82` | MATCH |
| 007D20F8-007D2127 class accel | `plane_class_fields.cpp:207-220` | MATCH |
| 0099E490-0099E512 pitch floor | `plane_ai_control.cpp:340-371` | MATCH apart from P2 |

## Not covered

- Labels and liveness only, with no byte check: `plane_angular_velocity.cpp` (`body_to_world_007d9c80`
  is live), `plane_squadron_entity.cpp` (007F4B43, 007F4754 and 007EDA90 are live), the rest of
  `plane_ground_ops.cpp` (`water_drown_test_007cbd79` and `water_accumulator_step_007cbe7a` are live),
  `plane_class_fields.cpp` beyond 007D20F3, and `plane_bow_wave.cpp` (not live).
- Still unread in `plane_ai_control.cpp`: `pilot_plan_roll_0099e2ba`, `plan_yaw_0099e81a`,
  `pilot_roll_bank_demand_0099e2ba`, `pilot_roll_rate_limited_0099e344`, and the tail of
  `plane_control_targets_007da710`.
- In the struck range, not re-read: 007EE7F0 and 007EEF30.
