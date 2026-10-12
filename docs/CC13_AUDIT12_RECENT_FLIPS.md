# Audit of cc9's recent ON flips, dive and strafe paths (cc13-audit12)

Addresses: 009C4F80 009C5080 009C5090 009C50B7 009C5101 009C5144 009C5149 009C5163 009C5180 009C53D8 009C5400
009C5420 009C542C 009C5473 009C547E 009C5563 009C55B0 009C57C4 009C57D3 009C57E8 009C57F1 009C57FF 009C58D0 009C58DE
009C5DB8 009C5E63 009C5F4C 009C5F75 009C5F79 009C5F9E 009C5FA7 009C5FCB 009C6E10 009C64A6 009C64A8 009C6EB1 009C6F09
009C6F33 009C6F3B 009C6F4D 009C7AA4 009C7AAD 009CAC80 009CACDA 009CACDF 00419010 00419030 00438B10 0099D300 0099D369
0099E756 00928F50 00923B80

Packet `cc13-audit12`, branch `agent/cc13-audit12`, 2026-10-12. Requested by the cc13 lead for cc9. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files on this branch. Base: `origin/main` at `33b553214`.
No defect was found, so no diff was applied, built or reverted. Only this doc and
`reports/cc13_audit12_recent_flips.json` are committed.

**Selection.** I listed every `k*Bound` that went from `false` to `true` on `origin/main` since 2026-10-10
(`git log -G"Bound = true"` over `src/` and `include/`, 36 commits). I dropped these:
- switches already audited in a `docs/CC13_*.md`: the CC9_SWITCHES, CC9_RECENT, LUA51_FLIPS and MG_AA_LETHALITY
  audits;
- switches that a cc13 doc proposed and cc9 only flipped: the torpedo-aim group and `kRollArmBaseNumeratorBound`
  (CC13_AUDIT_TORPEDO_BOMBER / PLANNER_ROLL), the six draw pins (UNLABELLED_DRAWS_BATCH), the cadence switches
  (TASK_PROFILE_CADENCE), the flak and tail-gunner switches, and `kStrafeDoubleConstantsBound` / `kMoveToLuaAltZeroOffsetBound`
  (SMALL_DIFFS_BATCH_2);
- cc13's own dyn and submerged-launch flips.

Of the rest I ranked the x87 and stack-slot paths first. I took six:
- the two GUNNERY 193 "mechanism verdict" dive groups. cc13-audit7 described their defects, but cc9 wrote the
  code, and no cc13 doc had read the code;
- `kDiveYawModeZeroBound`;
- the two older dive arithmetic flips (178.1, 181.2);
- the one RNG flip.

`kPartyChangeRefreshesGunneryBound` was read as a light sixth (plumbing).

**Method.** I read each switch's ON path on `origin/main` (`src/dive_bomb_aimdive_tail.cpp`, `src/dive_bomb_task.cpp`,
`src/game_hosts_units.cpp`, `include/bsp/dive_bomb_task.hpp`, `include/bsp/strafe_task.hpp`). I compared it with
the image listing (`bsp.py disasm-raw` on disk bytes, `bsp.py ghidra disasm` for the stored listing) and tracked
every stack slot from its write to its read across each `SUB ESP` and callee-pop `RET`. Landing evidence is
GUNNERY_OPEN_ITEMS 178.1, 181 and 193 and SHIP_AI 213. I read those rows by line range only.

**Checklist.**
- Rule 2 (boundary before attribution): `tools/verify_report_calls.py reports/cc13_audit12_recent_flips.json`
  checked 14 call rows, 0 failed.
- x87 register forms: `tools/x87_dc_form_scan.py --include-st0` over the four ranges. `009C4F80-009C5177` and
  `009C5DB8-009C6080` have no DC-form sites. `009C6E10-009C6F91` has one, `009C6F33 dc c0 FADD ST0,ST0`, which is
  the term `kDiveFlyAboveTermsBound` binds (section 5). `009C5180-009C580A` has three `FMUL ST0,ST0` squares at
  `009C528D/52CF/5311`; these are the planar-distance squares upstream of the audited arms, not under any switch.
- Callee-pop widths: `00419010` ends `RET 14h` (`00419030`) and `00438B10` ends `RET 8` (`00438B4C`). Every slot
  offset below was rebased with those widths.
- Operand widths: every constant the switched paths load was read from the image. The width of each load (FLD
  dword or qword, MOVSS) matches the host's `float` or `double` (section 5 lists them).

**Status vocabulary.** Every switch audited is reconstructed and build-tested on main. cc9 game-validated each one
with pairs: 178.1, 181.2, 193 and SHIP_AI 213. This audit is a static second reading and adds no runs (RUN SLOTS
rule). There are no diffs.

## Verdicts

| # | switch | verdict | one line |
| --- | --- | --- | --- |
| 1 | `kAimDiveBrakeRatioBound` (GUNNERY 181.2) | FAITHFUL | `009C5F79 FSTP [ESP+38h]` under `SUB ESP,14h` is the frame's `[ESP+24h]`. `00419010` pops 14h, so `009C5FA7 FLD [ESP+24h]` reloads `AimPitchRatio x |pitch|`, as the host passes |
| 2 | `kDiveAimHeadingBlendBound` (GUNNERY 178.1) | FAITHFUL | `009C5090` gate, the `+pi` correction, `SubtractWrapped(B,E)`, `IC(-60 deg,0,-40 deg,1,pitch)`, `FMUL d` and `SubtractWrapped(B,t*d)` all match, slot for slot and argument for argument |
| 3 | `kDiveYawModeZeroBound` (GUNNERY 189 B / 193) | FAITHFUL | `009C5420` stores the immediate 0 and `009C5E63` stores EBX, which is 0 from `009C58DE XOR EBX,EBX`. The host sets the flag before `0099E756`'s read in the same think and clears it there |
| 4 | `kDiveAimGlideArmsBound` (189 A, C / 193) | FAITHFUL | Bank scale: `IC(100,1,250,1.5,[ESP+1Ch])` on the heading arm only (`009C542C`). Arm B: `0.6 > ratio`, `0 > pitch`, `p2 > p1`, with `p1 = [ESP+6Ch]` from `009C5563` and `p2 = [ESP+28h]` from `009C55B0`, neither rewritten before `009C57F1` |
| 5 | `kDiveFlyAboveTermsBound` (189 C2, D, E / 193) | FAITHFUL; one pre-existing gap noted (5a) | `C = ctl+398h` (`009C64A8`) equals `approach+ACh` this think (`009C7AA7-7AAD`). The numerator is `FADD ST0,ST0` of the error (`009C6F33`). `2E8h = 1.5` with `+18h` clear |
| 6 | `kStrafeAttackrunSeedDrawBound` (190 / 193) | FAITHFUL | `009CACDA` draws `U(0,1)` with `ECX = 1`, and `009CACDF FCHS` then `FSTP +1Ch`. The host's keyed draw follows the host-wide convention for 00BD2F10 |
| 7 | `kPartyChangeRefreshesGunneryBound` (SHIP_AI 213) | FAITHFUL (host consolidation) | The image has one field, written by `00923B80` (`00928F7D`). The gunnery host's copy is the only split copy, and every other host reads `unit_side_0054` live |

No defect, so there is no diff section.

## 1. `kAimDiveBrakeRatioBound` (GUNNERY 181.2)

**Host.** `src/dive_bomb_aimdive_tail.cpp`: ON passes `x1 = aim_pitch_ratio_54 * |pitch_command|` as the brake
floor's argument. OFF passes `|pitch_command|`.

**Image** (`disasm-raw 009C5F1C`). Let E be ESP at `009C5F4C`. The fold writes `|pitch|` to `[E+24h]`
(`009C5F57` / `009C5F66`). Then:

```
009C5F6F FLD  [EBP+54h]            ; AimPitchRatio
009C5F72 SUB  ESP,14h              ; ESP = E-14h
009C5F75 FMUL [ESP+38h]            ; [E+24h] = |pitch|
009C5F79 FSTP [ESP+38h]            ; [E+24h] := x1   (overwrites |pitch|)
009C5F7D FLD  [ESP+38h] / FSTP [ESP+10h]   ; arg5 = x1
...  [ESP+0Ch]=+48h, [ESP+8]=1, [ESP+4]=+44h, [ESP]=0
009C5F9E CALL 00419010             ; RET 14h -> ESP = E
009C5FA3 FSTP [ESP+1Ch]            ; power cap
009C5FA7 FLD  [ESP+24h]            ; = x1
009C5FAB SUB ESP,14h / FSTP [ESP+10h]      ; arg5 = x1
...  [ESP+0Ch]=+4Ch, [ESP+8]=1, [ESP+4]=+50h, [ESP]=0
009C5FCB CALL 00419010
```

- The argument order is `(0, MinBrake +50h, 1, MaxBrake +4Ch, x1)` for the brake and `(0, MaxPower +44h, 1,
  MinPower +48h, x1)` for the power cap. The host passes the same order.
- Precision: `x1` is a float times a float, rounded once to float at `009C5F79`. The host's single-precision
  product rounds once as well, so the two are bit-equal.
- The fold's zero: `XMM0` is the zero `009C5EF2` rebuilds, and `XMM2` is the `-0.0` from `009C5EEA`. The branch
  form matches `fold()`.

**Verdict: FAITHFUL.** One limit on scope: the fold's source is the frame's `[ESP+10h]`, which the host maps to
`r.pitch_29c`. That mapping predates this switch (packet `cc9_aimdive_response`), and I did not re-derive it.

## 2. `kDiveAimHeadingBlendBound` (GUNNERY 178.1)

**Host.** `dive_bomb_aim_heading_009c4f80` (`src/dive_bomb_task.cpp`), the `mid_pitch_blend` arm. All four call
sites set it: the approach feed (`game_hosts_units.cpp` 4730 and 4858), aimdive (23024) and aimglide (23924).

**Image** (`disasm-raw 009C5040`). Let S be ESP after the two pushes. The prologue's fold writes `|bank|` to
`[S+0Ch]` (`009C4F9D` / `009C4FB1`), and `009C506E` / `009C5082` write the body bearing B to `[S+8]`.
- `009C5090 COMISS [00D20338] (-60 deg), pitch` then `009C509F JBE`: B returns only when -60 deg is greater than
  pitch. Otherwise the code blends. The host's `!(kVertical > pitch)` is the same, and both blend on a NaN (the
  unordered COMISS takes the JBE).
- `009C50B7 vtable[50h]()` gives E into `[S+10h]`. `009C50C2-50CE` compare `|bank|` against pi/2 (qword
  `00CE3830`). Past pi/2, `009C50E6` computes `00438AA0(E, pi)` (pi at `00D7A264`, the second argument).
- `009C5101 00438B10(B, E)`: the argument at `[ESP]` is B (`009C50FA FLD [ESP+10h]` = `[S+8]`). The result goes
  to `[S+0Ch]` (`009C5109`).
- `009C5144 00419010(-60 deg, 0, [00CE7D1C] = -40 deg, 1, pitch)`, then `009C5149 FMUL [S+0Ch]`, then
  `009C5150 FSTP` float, then `009C5163 00438B10(B, t*d)`.
- `00419010`'s interpolating arm rounds through `FSTP [ESP+0Ch]` (`0041905F`) before it returns, so t is a float.
  `t*d` is then rounded once to float at `009C5150`, which matches the host's `float * float`.

**Evidence.** 178.1 moves all three rows, both ways: USN04 dive releases 3/19 -> 0/19, JM08 5/27 -> 10/15. cc9
flipped it on the mechanism, not on outcomes, and recorded the USN04 loss as open. The mechanism reads correctly.
The USN04 loss is therefore a consequence of other paths (175(a)'s roll response), not of this arm.

**Verdict: FAITHFUL.**

## 3. `kDiveYawModeZeroBound` (GUNNERY 189 B, ON in 193)

**Host.** The aimglide yaw arm (`game_hosts_units.cpp` 23947) and the aimdive tail (24546) set
`gl_yaw_mode_2d4_zero`. `plan_yaw_0099d300` (30798) reads the flag and clears it. Both yaw-arm writers (30919,
31183) skip the slot write when it is set.

**Image.**
- `009C5420 MOV dword [EAX+2D4h], 0` sits on the yaw arm (`009C53E7-542A`), whose `009C53D8 COMISS [00D04A24],
  [ESP+1Ch]` / `JBE` is the arm selector the host's `st.yaw_arm` models.
- `009C5E63 MOV [EAX+2D4h], EBX`. EBX is 0: `009C58DE XOR EBX,EBX` in the aimdive prologue, and nothing between
  `009C5DB8` and `009C5E63` writes EBX (`009C5DBE` only compares BL).
- The reader is `0099E756 MOV ECX,[ESI+2D4h]`, and `0099E75E JNZ` gates the yaw arm `0099E81A`.
- The planner's own `+2D4h` stores (`0099D369`, `0099D410`, `0099D64A`) all store EDI = 0 (`0099D307 XOR EDI,EDI`).
  None of them re-arms it.

**Order.** In `pilot_think_and_commit` the re-seed (30392, `0099B450`) runs first, then the dive task arm (30477),
then the planner (30673). There is no `return` between 30400 and 30673, so the flag set this think is consumed
this think, which matches the image's seed -> state tick -> planner order. The width is a dword with 0/non-zero
semantics, and a host bool is equivalent.

**Evidence.** 193: USN04 phase 2 dive releases 0/32 -> 12/32, the predicted direction. The first divergence is
the yaw command surviving the planner.

**Verdict: FAITHFUL.**

## 4. `kDiveAimGlideArmsBound` (189 A and C, ON in 193)

**4.1 The bank scale (`009C542C-009C547E`).**
- The selector `009C53D8` `JBE 009C542C` takes the heading arm when `[00D04A24] <= [ESP+1Ch]`. The bank-scale
  store is on that arm only. The yaw arm jumps from `009C542A` straight to `009C5484`.
- The host writes `turn_scale_2e8` inside `if (r.wrote_heading && !glide_yaw_arm)`, which is the same arm.
- Arguments: `[ESP]=[00CE3D08]` 100, `[ESP+4]=1`, `[ESP+8]=[00CE77B0]` 250, `[ESP+0Ch]=[00CE380C]` 1.5,
  `[ESP+10h]=[ESP+1Ch]` before the `SUB` (`009C5431` / `009C543E`). `009C547E FSTP [EDX+2E8h]`.
- The host passes `db_impact_planar_5c`, the same value it feeds the selector (`dive_bomb_aimglide_steer_009c53d0`),
  so the two reads of `[ESP+1Ch]` stay one quantity. The re-seed to 1.0 each think is `pilot_plan_slots.cpp:42`.

**4.2 The pull-out latch arm B (`009C57D3-009C57FF`).**

```
009C57C4 MOVSS XMM0,[ESP+18h] / 009C57CA COMISS [00CE380C] / JA 009C57FF   ; arm A
009C57D3 MOVSS XMM0,[00CE3D30] (0.6) / 009C57DB COMISS [ESP+24h] / JBE exit
009C57E8 COMISS XMM2(0),[unit+C64h] / JBE exit
009C57F1 FLD [ESP+6Ch] / FLD [ESP+28h] / FCOMIP / JBE exit                    ; [ESP+28h] > [ESP+6Ch]
009C57FF MOV byte [ESI+18h],BL
```

- `[ESP+6Ch]` is written last at `009C5563` (p1, after `RET 14h` restores ESP) and `[ESP+28h]` at `009C55B0` (p2).
- I listed every ESP-relative store from `009C55B4` to `009C57C4`. The throttle block, the release block
  (`009C5689-009C57BE`) and the drop draw (`009C578C` `SUB ESP,8`) write `+1Ch`, `+20h`, `+2Ch` and their own
  argument slots, never `+28h` or `+6Ch`.
- `[ESP+24h]` is the 009C534B/5353 ratio, read but never rewritten after `009C5522`.
- `XMM2` is 0 on both paths into `009C57C4`: from `009C562D`, and re-zeroed at `009C57AF`.
- The host's `dive_bomb_aimglide_pull_out_arm_b_009c57d3(ratio_24, pitch_c64, p1, p2)` returns `p2 > p1` after the
  same two strict gates.

**Observation (pre-existing, not this switch).** The host evaluates arm A at the top of the tick, from
`db_glide_bearing_abs_18`. The image evaluates it at the tail from `[ESP+18h]`. Nothing in the host's tick reads the
latch after it is set (its only other reader is the transition at 4631), so the order does not change a value.
The feed's bearing uses the same blended aim heading (4730).

**Verdict: FAITHFUL.**

## 5. `kDiveFlyAboveTermsBound` (189 C2, D and E, ON in 193)

**5.1 C (`009C64A6-009C64C9`).** With a control block, `C = [EAX+398h]` (`009C64A8`). Without one,
`C = approach+ACh + approach+50h`. `009C7A80` (`009C7AA4-009C7AAD`) runs every think before the flyabove tick and
copies `[approach+0Ch]+398h` to `approach+ACh`, so ON's `cruise_altitude_398 = db_begin_alt_ac` is the right value
whenever the host's `db_begin_alt_ac` follows `ctl+398h`.

**5a. Gap noted, pre-existing.** The host updates `db_begin_alt_ac` from the squadron block (`game_hosts_units.cpp`
3998-4020: the Lua attack altitude or `sq_profile_398`). Otherwise it keeps the `009C3EA0` seed, tuning `+4CCh`
(22625). The image's `ctl+398h` for an unfed squadron is the profile's value, whose `00BD2F10` draw at `009C899B`
is labelled unmodelled in that block. This is the same gap that already applied to the OFF build's `ACh`. The
switch inherits it and does not add to it.

**5.2 The descent reference (`009C6F09-009C6F69`).**
- After `009C6F11 FCOMI` / `009C6F13 FSTP ST1`, ST0 is the error.
- `009C6F17 FLD A` / `FLD1` / `FCOMIP` / `FSTP ST0` leave ST0 the error again. The divisor `max(A, 1)` goes to the
  slot through `XMM0`.
- `009C6F33 FADD ST0,ST0` (the one DC-form site in the range) then `009C6F3B FDIV dword`, then `FSTP` float.
- `009C6F4D` compares v with qword `0.8f` (`00CE3D40` = `3FE99999A0000000`). Above it the store is float `0.8f`
  (`00CE74F8`).

The host's ON is `(err + err) / divisor`, then `v <= kLeaveSpanScale ? v : kDiveReferenceCap`. The constants
match the image bytes: `kLeaveSpanScale` double `0.800000011920929`, `kDiveReferenceCap` float `0.8f`,
`kClimbGain` double 3.0 (`00D7A2B0`) and `kClimbReferenceCap` float 1.0 (`00D7A24C`).

**5.3 The bank scale.** The host sets `2E8h = 1.5f` when `db_flyabove_can_dive_18` is clear, after the
`reached_dive_test` 1.4 store, so 1.5 wins whenever +18h is clear, as `009C6DF5-009C6E08` does.

**Evidence.** 193: JM08's first divergence is an `aimglide -> goaway` exit by arm B, and USN01's is the
`ScoutDauntless` aim lead, both on the switched paths. USN04 is identical, and cc9 explains why (the turn-roll
cap). The USN01 convoy loss is the 193 "convoy strike" casualty that every plane-path change re-rolls. The lead
ruled it a plan casualty, so the pair cannot attribute it to this switch.

**Verdict: FAITHFUL**, with 5a as an inherited gap.

## 6. `kStrafeAttackrunSeedDrawBound` (190, ON in 193)

**Image** (`ghidra disasm 009CAC80`).

```
009CACB2 FLD1 ; 009CACBC SUB ESP,8 ; 009CACBF FSTP [ESP+4]   ; hi = 1
009CACC9 FLDZ ; 009CACCB LEA ECX,[EAX+1] (EAX = 0 from 009CACA0) ; 009CACCE FSTP [ESP]  ; lo = 0, stream 1
009CACD5 MOVSS [ESI+18h],XMM0 (1.0, 00D7A24C)
009CACDA CALL 00BD2F10 ; 009CACDF FCHS ; 009CACE4 FSTP [ESI+1Ch]
```

**Host.** `st_ar_countdown_1c = -st_draw("ar1c", 0, 1)`. `st_draw` is `release_altitude_draw_00bd2f10` keyed on
`<name>#star1c` (FNV-1a, one generator per key when the release streams are enabled). The image's stream is 1, not
the gameplay stream 0. The host does not reproduce the image's stream-1 call order: that is the host-wide
convention for keyed 00BD2F10 draws (dive, goaway, strafe), not something this switch introduces. A new key does
not move any other key's sequence.

**+18h.** The constructor stores 1.0, and the enter `009CAD90` overwrites `+18h` and `+20h` (`009CADA0`,
`009CAD93`). The host seeds the period at install (0.4), the enter value, so the constructor's 1.0 is never seen.
This is consistent.

**Evidence.** 193: no reach on IJN01, JM08 or USN01. ESMP08 and USNOS, re-paired where strafers fly, exit 0 and 1.
The verdict there was "image-faithful and inert here", which agrees with this reading.

**Verdict: FAITHFUL.**

## 7. `kPartyChangeRefreshesGunneryBound` (SHIP_AI 213)

**Host.** `GameUnitsHost::set_unit_side_0054` writes `row.party`, then
`GameGunneryHost::refresh_unit_side` copies `unit_side_0054` into the gunnery `UnitState.row.side`.

**Image.** `00928F50` SetPartyRace writes the party through `00923B80` (`00928F7D`). The image keeps one field
(`+54h`), and the gunnery readers read it live.

**Other host copies.** I looked for every other host copy of the party. `game_hosts_ai.cpp` and the other
`game_hosts_gunnery.cpp` sites read `unit_side_0054` at use. The init copy at `game_hosts_gunnery.cpp:4165` is the
one this switch refreshes. `shot.owner_side` (7938) is taken at fire time, which is the image's read time.

**Evidence.** The callers are the script party write (`game_hosts_script_orders.cpp:5315`) and the
command-building neutralize and flip (`game_hosts_ship_ai.cpp` 12482, 12928, 12973). The SHIP_AI 213 pairs are
gameplay-identical, which is what you would expect where no captured unit fires after its flip inside the window.

**Verdict: FAITHFUL (host consolidation).**

## Summary for cc9

- All seven switches read faithful against the listing. No diff is proposed.
- Two notes need no action from this audit:
  - 5a: `ctl+398h` for an unfed squadron is the unmodelled `009C899B` profile draw. This is pre-existing.
  - Section 1: the aimdive tail's `|pitch|` source mapping predates 181.2 and was not re-derived here.
- The dive-group outcome changes in 193 (JM08 release counts, the USN01 convoy re-roll) follow from faithful
  mechanisms. If a dive row is still wrong, the cause lies elsewhere: the latched `approach+D8h` aim point, the
  175(a) roll response, or the unmodelled profile draw.
