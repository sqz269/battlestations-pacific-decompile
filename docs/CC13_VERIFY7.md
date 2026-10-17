# CC13_VERIFY7: ship and gunnery sub-files

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify7 on 2026-10-17 against
base 44242d5c7. It covers `src/ship_ai_torpedo_response.cpp`, `ship_ai_ring_scan.cpp`,
`ship_ai_path_cursor.cpp`, `ship_ai_free_bearing.cpp`, `ship_ai_hull_geometry.cpp`, `gun_bot_ticks.cpp`,
`projectile_impact.cpp`, `weapon_director.cpp`, `command_execution.cpp`, `director_target_gate.cpp` and
`ship_ai_approach_update.cpp`. No code was changed and no ledger records were added. Every line number
below is against that base. "Image" means battlestationspacific.exe, read with `bsp.py show`,
`bsp.py disasm-raw` (disk bytes) and `ghidra disasm`. Constants were read from the PE with
`tools/pe_const_read.py`. Script counts come from this installation's `scripts/` tree (a modded
install).

Excluded, as briefed: the 0071E7F0 / 0071E610 override layer in `command_execution.cpp`, and the +490h
max-range sites in `ship_ai_approach_update.cpp` (`:728`, `:1039`; cc9-ships56). Already reported and not
repeated: from_submarine / the sub addon in the torpedo response, the neighbour Y band in hull geometry,
the 0071E7F0 repeat test, and the +490h reads (CC13_VERIFY1..3).

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  NaN-only branch polarity, or 1-ulp precision.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but the premise no longer holds.
- UNRESOLVED.

## Census

These files carry almost no labels. The census pattern
`substitut|approximat|stub|simplif|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|fixed value|unresolved|unimplemented|record\(|not modell|deterministic|for now|TODO|assum|uncertain|provisional`
found only four relevant lines: `ship_ai_free_bearing.cpp:5` (x87 precision between stores not modelled),
`projectile_impact.cpp:9-10` (flak variant and narrowphase not modelled), and in
`ship_ai_path_cursor.cpp` the HYPOTHESIS at `:94` (007B1100 "not read") and the LABELLED index clamp at
`:154`. No file has a `record()` or `unimplemented()` call. Every switch in these files and their headers
is ON: `kApproachSideWeightDcFormBound`, `kTorpedoStandoffBound`, `kShipAiQueryGateBytesBound`,
`kShipAiApproachModeLatchBound`, `kShipAiApproachRetargetRingBound`, `kShipAiApproachLandingModesBound`,
`kShipAiApproachNoShipHoldBound`, `kOutlineWalkX87ProductsBound`, `kProjectileGravityImageBound`,
`kPathCursorFollowStepBound`, `kSquadronPathFollowStepBound`, `kPathFollowFinishBound`.

Most of the pass was therefore the unlabelled arithmetic. The first step was to find which entry points a
live host calls (`git grep` outside the file). These are live: the torpedo admission, the free-bearing
query, the hull pre-step, the ring-scan slot scorers, the whole 009F3090 approach chain, the path cursor
(from `game_hosts_commands.cpp`), the stance and fire-target routines of the weapon director, the clear
routines of `command_execution.cpp`, and from `gun_bot_ticks.cpp` the idle timer, the torpedo-ready test,
the lead-error span and limit, the muzzle-error envelope and the slot map. These are NOT called by any
host: the three `gun_bot_*_tick` sequences, `begin_current_command_00835c70` in `command_execution.cpp`
(`game_hosts_commands.cpp:3171` has its own), all of `director_target_gate.cpp`, and in
`projectile_impact.cpp` everything except the launch velocity.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `game_hosts_commands.cpp:3243-3253` joins the moveonpath cursor at the nearest point alone, through `ship_ai_path_nearest_index_007b1100` (`ship_ai_path_cursor.cpp:94`) | 007B1C50 -> path-source vt+10h = 007B11F0: the nearest point n, its neighbour m in the travel direction, and m when dot(normalize(p_m - p_n), pos - p_n) > 0 (007B1263..007B12E0) | yes: every moveonpath with PATH_SM_JOIN / _FORWARD / _BACKWARDS (321 + 33 + 29 script uses, including jm08.lua; the host comment counts eight USN04 sites) | image stream 1 for PATH_SM_JOIN_RANDOM_DIR only (007B1CE2); host none |

One unwired REAL-class mismatch is recorded under `command_execution.cpp` (U1). It has no effect while
nothing calls the routine.

### R1 Moveonpath join index

In the image, 007B1C50 handles start modes 5, 7 and 8 (PATH_SM_JOIN / _JOIN_FORWARD = 5, _JOIN_RANDOM_DIR
= 7, _JOIN_BACKWARDS = 8; mode 6 = BEGIN stores 0). It sets the direction byte at cursor+10h and then
cursor+8 = the path source's vt+10h (007B1D24). For an authored path that is 007B11F0:

- vt+14h gives the nearest index n. That is 007B1100 -> 007B0900 -> 007AED10: the query is put in path
  space through the inverse world matrix, the distance is 3D, and the first index wins a tie.
- With at most one point, n is the answer. Going forward, n is the answer when it is the last point;
  otherwise m = n + 1. Going backward, n is the answer when it is 0; otherwise m = n - 1.
- 007B1263..007B12E0 take dir = (p_m - p_n) scaled by 00419260's reciprocal length, and keep n when
  dot(dir, pos - p_n) <= 0. Otherwise the cursor starts at m.

The host takes only the nearest point (labelled at `:3243-3245`: "the projection test ... was not read").
A ship that is already past its nearest point toward the next one starts at m in the image. The host
starts it at n, so the ship turns back to n first, unless n happens to be inside the follow radius on
the first follow step.

Two smaller differences belong to the same call:
- PATH_SM_JOIN_RANDOM_DIR: the image draws 00BD2F10 on stream 1 at 007B1CE2, U(0, 1) against 0.5
  (00CE3800), to pick the direction. The host passes `random_forward=true` (`:3255-3260`). There is one
  script use in this installation.
- A start mode outside 5..8 makes the image return before it touches cursor+8 (007B1CB9 -> 007B1D29).
  `ship_ai_path_cursor_start_007b1c50` (`ship_ai_path_cursor.cpp:75`) stores the joined index anyway.
  This is MINOR.

Unverified: whether a kind-0 user-path source (`0071F6A5`) has the same vt+10h. The host gives it start
mode 5 too.

## Per-file results

### ship_ai_torpedo_response.cpp (URGENT class: torpedo), all MATCH

| Routine | Image | Result |
|---|---|---|
| `ship_ai_torpedo_admits` | brain pre-pass 009F1685..009F1839 | MATCH. Horizon = (obs_hi + predict_hi) + float(period + 3.0). Hull = float(len * 0.6)^2. The JBE / JBE / SETA polarity is as the bytes have it, including NaN. The squared distance is one x87 sum, modelled as one double sum. |
| `ship_ai_admit_torpedo_track_009f0ad0` | 009F0AD0 | MATCH. Capacity 80h. Lifetime CollectTimer[2] + 1.0 (double). Three stream-1 draws in image order. The predict scale applies when the reference is shorter than the hull. Own side: observation 0, and predict 25.0 when <= 25.0. The track constants 5.0 and 12.0, and max horizon (spd_err + speed) * predict, all agree. |
| `ship_ai_refresh_torpedo_track_009dc060` | 009DC060 torpedo arm | MATCH. The 414DB0 pose refresh is the caller's snapshot. The ship arm (+60h) is not in this file. |

### ship_ai_path_cursor.cpp (live through game_hosts_commands.cpp)

| Routine | Result |
|---|---|
| 007ADC60, 007ADCC0, 007ADFAC | MATCH (the branch tables were read against the bytes). |
| 007B1C50 `ship_ai_path_cursor_start_007b1c50` | MINOR: an unknown start mode still stores the index (see R1). |
| 007B1100 `ship_ai_path_nearest_index_007b1100` | STALE LABEL ("not read"). 007AED10 has now been read: it is the nearest point, with strict `<` and the first index winning, so the ordering matches. The host uses world XZ only; the image uses 3D distance in path space. MINOR when the path points share the unit's Y. What R1 is about is how the caller uses this index, not the routine itself. |
| 007ADD70 `ship_ai_path_follow_step_007add70` | MATCH, read against 007ADD70..007AE01A. The limits per mode, the `(count & 0xFF) == 0` cap at 1, radius^2 against dz^2 + dx^2, the wrap, the factor gate (0 < f < 1), atan2(dz, dx), pi/2 - a with the 2*pi wrap, subtract(heading, a), and reach = ((1 - f)*abs(sin) + f) * r with reach^2 > d^2 all agree, as do the force arm and the advance loop. The LABELLED index clamp at `:154` never fires, because the wrap keeps the index inside [0, count). The image's step is uninitialised outside modes 1..3, where the host uses 0 (MINOR). |

### ship_ai_free_bearing.cpp (live), MATCH

- 00416270, `avoid_zone_outline_walk_00416270`: MATCH. Near and far ends per direction, the normal and
  facing product, the circle-crossing choice (`d1 <= d0` -> p1), count 1 -> p0, count 0 -> the far end,
  next/previous, and the non-monotone stop all agree. NaN: the image continues only on an ordered
  `prev <= far2`, while the host stops only on `prev > far2` (MINOR).
- 009DC2E0: MATCH. The round one / round two corner logic was checked against the decompiled control flow:
  a failed walk keeps the previous dev and angle, and side 1 wins a tie. So were the clearance pass (along
  and lateral components, the clip, the arm choice `max <= 0 || max < -min`, both slope updates), the
  second pass of the same segment (it recomputes from the segment, so the claim that it changes nothing
  holds), and the final atan turn. The round-one acceptances are the only NaN-polarity difference (MINOR).
  The label at `:5` is accurate: 24- versus 53-bit only.

### ship_ai_hull_geometry.cpp (live), MATCH

The 009DE2F0 tail is inline-asm transcription. 009E0270: the class reference, the reference-speed floor
(FCOMIP and JBE as the bytes have it), the turn circle at 0.5, the three flags and the profile reset all
agree. The `throw` at `:177` is an added guard on a pointer the image re-reads; it has no effect (MINOR).

### ship_ai_ring_scan.cpp (scorers live), MATCH

009E6400 decay (FCOMIP polarity and the 1.0 ceiling), 009E6870 arc cost and 009E6640 probe (side gate,
timer, re-probe seed U, origin, clear distance) were checked against the bytes. There are two NaN-only
differences, both MINOR. At 009E6906, on an unordered pair, the image keeps `to_add` and the host takes
`to_sub`. At 009E6933, JB sends an unordered pair to the subtracting arm, while the host divides; the
comment at `:131-133` says the opposite of the bytes.

### ship_ai_approach_update.cpp (live: 009F3090 chain)

Checked against the bytes: 009F1BC0 goal range, class and turn radius max, retarget timer U(2, 3);
009E5E90 commanded-heading arms (both polarities); 009E7FC0 standoff window, mode-0 gates and running
maximum; the whole of 009E6A90; 008173E0; and 009F2AC9..009F2DF2.

| Item | Result |
|---|---|
| 008173E0 `ship_ai_torpedo_threatens_008173e0` (torpedo) | MATCH. The sub band \|dy\| <= 6.0. The non-sub depth FILD(-class+570h) JAE. dy zeroed. Seconds = len / speed. Tolerance interp(2.0, 1.3962634, 6.0, 0.7853982). Compass heading, both wraps, the SIGNED difference, and the length * 0.5 fallback. |
| 009F2AC9 `ship_ai_torpedo_standoff_009f2ac9` (torpedo) | MATCH. Health weight interp(0, 3.0, 0.4, 1.0). Hits-to-sink * weight. In-flight * 0.5 against max(., threatening / 5). The device count, accuracy and 008387B0 argument order, and max(300, curve) * weight. NaN only: at 009F2CFE, JB keeps the gate open on an unordered need, while the host closes it (MINOR). |
| 009E6A90 `ship_ai_approach_limit_throttle_009e6a90` | MATCH in control flow: early zero arms skip the gate, override arms reach it with 1.0, gate 1 / 2, astern and ahead caps, final clamp. MINOR: `kApproachThrottleSeedX1` (`ship_ai_approach_update.hpp:253`) is 1.2217304706573486f = 0x3F9C61AA, but 00CE3988 holds 0x3F9C61AB (1.2217305898666382). The value is one ulp low, and the comment's "(70 deg)" hides it; `const_width_sweep.py` does not flag it. Also MINOR: `outer = radius + reference` (`:589`) and the gate-1 threshold (`:610`) are rounded to float in the host but compared unrounded on the x87 stack in the image (009E6C67, 009E6D1C). |
| 009F1BC0 `ship_ai_approach_frame_state_009f1bc0` | MATCH for the parts read. NaN only: on an unordered retarget timer the image skips the redraw (JBE at 009F1D90), while the host redraws (MINOR). |
| 009E7FC0 `ship_ai_approach_reset_scores_009e7fc0` | MATCH. The +1294h = 20.0, +1298h = 60.0 and +12BCh / +12BDh = 1 stores (009E813D..009E8178) are not in this routine, but `game_hosts_ship_ai.cpp:2665-2666` and the ring-scan query words carry them. |
| 009E5E90 `ship_ai_approach_commanded_heading_009e5e90` | MATCH. |
| `:905`, `:922` traffic record insert and erase | Not judged (host plumbing names, not a substitution). |

Not covered in depth: 009E6E80 choose_standoff (`:652-848`, which holds the excluded `:728`), 009E9190
refresh_avoidance (`:849-964`), 009E76D0 select_slot (`:965-1063`, which holds the excluded `:1039`),
009F1F47 mode latch, and 009F2124 / 009F239A retarget (`:1196-1354`).

### gun_bot_ticks.cpp

| Routine | Live? | Result |
|---|---|---|
| 008FBCE0 `gun_bot_idle_timer_008fbce0` | yes | MATCH. The pin and its double FLT_MAX compare, the 0085AD00 rest call (the host is given the action), and the 0.25 wind-back on a failed side gate. |
| 008FFF20 `gun_bot_torpedo_ready_008fff20` (torpedo) | yes | MATCH: subtract(bot+60h, gun+480h), abs, COMISS 1 degree JBE, then gun vtable[1D0h](1). |
| 00902920 `gun_bot_lead_error_limit_00902920` | yes | MINOR: `distance == 0 -> 0` (`:233`) is a guard the image lacks. At 00902E27 the image computes 25.0 / distance (FDIVR), which is +inf at zero, so nothing clamps; the host clamps every error to 0. This only happens at zero range. |
| 008FFA20 turret tick, 009030C0 ballistic tick, 006DF520 muzzle tick | no | Not wired. The 008FFA20 draws were checked: three stream-1 U draws, (0, err) twice and then (min, max). The host converts degrees to radians in float where the image uses double pi/180, and `kAAGunnerBotRangeFraction` and `kUnitGunAimWindowKind12` are float copies of doubles. All MINOR, and none is reached. |

### projectile_impact.cpp

006E8430 launch velocity (live): MATCH. Sub-types 4 to 7 subtract 0.05f * 9.81 (FSUBP DE E9 = vy - bias),
taken as a float product (MINOR at 53-bit). The rest of the file is not wired, and its label at `:9-10`
is accurate.

### weapon_director.cpp (live), MATCH

00836210 (the store, then the target release at +238h only when forbidding), the 0071C1E0 sub-kind table
(+1CCh dword, +220h..+223h bytes, vtable 64h / 68h), 00835860 gate and route, 0071D560 / 0071D580.

### command_execution.cpp

| Routine | Live? | Result |
|---|---|---|
| 00720850 `clear_command_slot_00720850` | yes | MATCH except one edge. With index 9, the image does not reinitialise slot 9 (the reset sits inside `if (index < 9)`), but it still deletes slot 9's path object and builds a new one. The host resets slot 9 (`:157`). Slot 9 is reached only with ten commands queued (MINOR). |
| 00720CA0, 00720CD0, 008358D0 | yes / yes / via host | MATCH. |
| U1: 00835C70 `begin_current_command_00835c70` | no (`game_hosts_commands.cpp:3171` is the live one) | REAL if it were wired, and listed so nobody wires it as it stands. On the follow arm, the image calls 007788B0 when the target is null, when unit+284h is null, or when unit+284h != target+284h. The host interface has no group-equality test (`:339`). The cruise arm leaves out 00835AC0's latch, the AttackMove gate 00521E70 is behind the stub `attack_move_gate()`, and every override in `game_hosts_commands.cpp:2534-2545` is a record that returns 0 or false. |

### director_target_gate.cpp (not wired)

MATCH against 0071DF70. One error in a comment: at 0071F30A, JB skips the countdown for an unordered hold,
while `update_auto_target_hold` (`:47`) subtracts on NaN, although its comment says it does not (MINOR).

## Uncovered

The `ship_ai_approach_update.cpp` ranges listed above; `ship_ai_ring_scan.cpp:328-407` (the 009E76D0
driver body, which is not called by a host; its winner, verdict and normalise helpers were read);
`projectile_impact.cpp:140-306` (not wired); `command_execution.cpp:251-292` (excluded).
