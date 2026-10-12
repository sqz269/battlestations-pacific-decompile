# Audit of the remaining recent ON flips: flak, tail gunner, profile cadence, strafe, dive pins (cc13-audit13)

Addresses: 0070C370 0070C387 0070C3AA 0070C3C8 0070C210 0070C247 0070C26E 0070C280 006E6490 006E6584 0084BC60
0084BE4D 0084BE5F 0084BE74 0084BEE3 0084BAD0 00864FE0 00865833 00727F10 008FFA20 008FFA6C 008FFAE7 008FFB35
008FFB62 008FFB73 008FFC3D 008FFCBD 008FFCD9 008FFCDE 008FFCEC 008FFD05 008FFD37 00CE3D20 00CE3D28 0099AEB3
009C8920 009C8931 009C899B 009C89E4 009C8A0E 009C8A17 009C8A7C 008A24E0 009C3650 009C367C 009C36A3 009C36BF
009C36CA 009C375A 009C3906 009C1D9D 009BEBA0 009CB780 009CB879 009CB87E 009CB650 009CB6F8 009CA870 009CAB35
009CAB7B 00D05858 00D05850 00D057D8 00CEC730 00BD2F10 00BD2E60 009C83E0 009C8515 009C8531 009C855B 009C8587
009C859E 009C85C1 009C7800 009C7839 009C5180 009C579E 009C58D0 009C6114 009C6270 009C62B0 009C6B75 009C6B86
009C6CAE 009C6D1A 009C6D29 009C67C7 009C67CB 009C680E 009C6826 009C683A 009C6875 009C64D7 009C654A 009CD020
009CD0AA 009CD0BB 009CA3B0 009CA41E 009CA459

Packet `cc13-audit13`, branch `agent/cc13-audit13`, 2026-10-12. Requested by the cc13 lead for cc9. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files on this branch. Base: `origin/main` at `e13bd4550`; re-checked against `355b60167`, which leaves every cited file unchanged.
The two diffs below were applied in this tree, built with `./scripts/build.ps1` and reverted. Only this doc and
`reports/cc13_audit13_recent_flips.json` are committed.

**Selection.** A script over `git log origin/main --since=2026-10-10 -p -- src include` lists every
`k...Bound` whose `= false` line was replaced by `= true` in one commit: 52 switches. Those that
`docs/CC13_AUDIT12_RECENT_FLIPS.md` audited, and those an earlier audit doc (`CC13_AUDIT_CC9_SWITCHES`,
`CC13_AUDIT_CC9_RECENT*`, `CC13_AUDIT_LUA51_FLIPS`, `CC13_AUDIT_PLANNER_ROLL`, `CC13_AUDIT_TORPEDO_BOMBER`,
`CC13_AUDIT_REATTACK_AND_SKILL`) covers, were dropped. The remaining switches are ranked as audit12 did: live on
every mission first, then x87, stack-slot and RNG paths. Most of them came from a cc13 *proposal* doc. The question
here is whether the landed code matches the image, not whether it matches the proposal.

| # | switches | why this rank |
| --- | --- | --- |
| A1 | `kFlakFlyTimeBurstBound`, `kFlakBlastIgnoreFalloffBound` | every AA mission; RNG (stream 1) |
| A2 | `kTailGunnerAimErrorBound`, `kTailGunnerOwnAimBound` | every bomber mission; RNG and x87 |
| A3 | `kTaskProfileActivationOnlyBound` | every plane task |
| A4 | `kMoveToActivationDrawsBound` with `kMoveToLuaAltZeroOffsetBound` | every moveto; RNG |
| A5 | `kStrafeDoubleConstantsBound` | operand widths |
| A6 | the six dive draw pins | RNG, stack slots |
| A7 | `kDiveFlyAboveCarriedFlagsBound`, `kDiveFlyAboveBankArmsBound`, `kDiveFlyAboveCrossTrackBound` | x87 form, stack slots |

These switches were not audited and remain uncovered by any audit doc: `kLuaHitTargetSquadronBound` and
`kLuaHitSquadronSecondPassBound` (both are in `CC13_AUDIT_LUA51_FLIPS` as proposals only), and
`kEngineMovieStopRosterBound`.

**Method.** I read each switch's ON path on `origin/main` and compared it with the disk listing
(`bsp.py disasm-raw`). Every stack slot that feeds a switched value was traced back to its writer. Every constant was
read from the image (`tools/pe_const_read.py`) together with its load width. The x87 forms were checked with
`tools/x87_dc_form_scan.py --include-st0 --context 6` over `009C6270-009C6A40`. The draw sites were checked against
`docs/CC13_RNG_STREAM_CENSUS.md` sections 10-13. Landing evidence is GUNNERY_OPEN_ITEMS 187, 188, 190, 192 and the
193 verdicts, read by line range only. Script reach was counted over this installation's loose `scripts/**/*.lua`,
excluding commented lines.

**Checklist.**
- Rule 2: `tools/verify_report_calls.py reports/cc13_audit13_recent_flips.json` checks 33 call rows, with 0 failures.
- Callee-pop widths: `00BD2F10` `RET 8`, `00419010` `RET 14h`, `0070C210` `RET 4`. Each slot offset below was
  rebased with them.
- `00BD2F10` returns a value rounded to float: `00BD2EC2 FSTP [ESP+0Ch]`, then `FLD` back. Every switched draw
  therefore enters its consumer as a float, as the host's `float` draws do.

**Status vocabulary.** Every switch audited is reconstructed and build-tested on main, and cc9 game-validated it
with pairs. This audit is a static second reading and adds no runs (RUN SLOTS rule). The two diffs are build-tested
only.

## Verdicts

| # | switch | verdict | one line |
| --- | --- | --- | --- |
| A1 | `kFlakFlyTimeBurstBound` + `kFlakBlastIgnoreFalloffBound` (GUNNERY 188/193) | FAITHFUL | The order is base tick, then expiry, then lock (`0070C387`, `0070C3AA`). The expiry test is the same `+1C4h > FlyTime` as `006E6584`. The burst is at `proj+FCh` with no back-off and `PUSH 1`. The impact flag is `classDesc+8h == 10h` on the round's own class, which the host reaches through `round_bullet_class` |
| A2 | `kTailGunnerAimErrorBound` + `kTailGunnerOwnAimBound` (190/193) | FAITHFUL | Draw order is horz, vert, period. `pi` is the float-widened qword `00CE3D28`, so float arithmetic is exact at 24 bits. The countdown runs only on aiming ticks (`008FFB35` after the `008FFAE9`/`008FFAFE` gates). The reset cadence is the library `00864FE0` call site. The two in-tick `vtable[38h](0)` clears (`008FFA75`, `008FFAE7`) are moot, because a cleared gun gets its next target through `00727F10` |
| A3 | `kTaskProfileActivationOnlyBound` (187/193) | **DEFECT (A3a)** | `009C8920`'s third block, the `+39Ch` release-altitude gate (`009C89E4-009C8A17`), is in the same `009C8938` leader branch as the `+398h` block. The host still runs it every think. A non-forced `SquadronSetReleaseAlt` therefore survives one think instead of the task. Diff A3 |
| A4 | `kMoveToActivationDrawsBound` with `kMoveToLuaAltZeroOffsetBound` (187, 190/193) | **DEFECT (A4a), an interaction** | The image refreshes at `009C36BF` **before** the leader's `+3ADh = 1` store (`009C375A`). The host stores it after the install's refresh `009C1D9D`, which comes before the activation draws. So when the activation retires a script altitude, the image's second refresh zeroes the fresh `+2Ch` and the host's keeps it. Diff A4 |
| A5 | `kStrafeDoubleConstantsBound` (190/193) | FAITHFUL | The three qwords are `0.3490658700466156`, `0.1745329350233078` and `0.6108652551968893`, loaded `FMUL`/`FADD`/`FLD qword` at `009CB87E`, `009CB6F8` and `009CAB7B`. The alignment test's `|err|` is `009CAB3E-009CAB77`'s `-0.0 - err` fold, and `0.61 > |err|` is strict |
| A6 | the six dive draw pins (190/193) | FAITHFUL | The coin is `trunc(U(0,1024)) >= 200h -> +1`, with bank > 20 deg `+1` and bank < -20 deg `-1` tested first (strict `COMISS`), and `+798h != 0` bypassing all three. The magnitude is `U(0.8,1)*side`. The re-arms are `U(0.2,0.5)` and `U(0.5,1)`, all with `ECX = 1`. The latch's `|a0|` is `-a0` from `009C6B86 FCHS/FSTP [ESP+10h]` |
| A7 | the fly-over trio (192/193) | FAITHFUL on the parts read | The enter seeds `+18h/+19h/+1Ch = 0`, `+1Ah = (approach+D1h == 0)` and `+20h = 0`. The dive test, the `+19h` copy and the 1.4 store (`009C683A`) match. The cross-track is the `DC C9` form, `(0.5 + 0.5 sin) R`. The `[ESP+27h]` bypass at `009C67D3` is `squadron+3A8h` (OldStyleBombing): no script in this installation sets it, and the host labels it |

Outside the switch list, one observation is recorded as O1 after A7: the strafe profile `009CD020` makes two stream-1
draws at each leader activation, and the host never makes them.

## A1. `kFlakFlyTimeBurstBound` and `kFlakBlastIgnoreFalloffBound` (GUNNERY 188, ON in 193)

**Host.** `src/game_hosts_gunnery.cpp` 9095-9113. The expiry block sits after the direct-strike sweep (8948-9041)
and before the proximity lock (9114). It calls `apply_impact_blast(owner, gun_row, team, position, {0,0,0}, true)`
with `round_bullet_class` set. `apply_impact_blast` (10407) draws `Draw::blast_damage` first and then gathers.
`ignore_falloff || (kFlakBlastIgnoreFalloffBound && weapon->type == "Flak")` goes into the record (10536).

**Image.**

```
0070C387 CALL 006E6490           ; base tick: +1C4h += step*class+5Ch (006E64B8-006E64BF),
                                 ; sweep 0084C430, then 006E6584 FLD +1C4h / FLD class+54h / FCOMPI / JBE
0070C3AA FLD [ecx+54h]           ; FlyTime
0070C3AD FLD [esp+14h]           ; +1C4h, copied at 0070C3A3 after the base tick
0070C3B1 FCOMI ST(1) / JBE 0070C3DA   ; strict: flight time > FlyTime
0070C3B7 PUSH 0 ; 0070C3C1 MOV [esi+50h],0 ; 0070C3C8 CALL 0070C210 ; then RET 4 (no lock search)
0070C210: 0070C247 CALL 00BD2F10 (ECX=1, U(+B4h,+B8h)) ; 0070C26C PUSH esi / PUSH edi(+238h) / PUSH 1 / PUSH &dmg
          ECX = proj+FCh, EDX = class+70h ; 0070C280 CALL 0084BAD0
```

- **Order.** In both, the sweep comes first, then the expiry, then the lock. The host `continue`s after the burst,
  as `0070C3D7` returns.
- **Centre.** `0070C27E MOV ECX,EBX` with `EBX = proj+FCh`. That is the round's position with no back-off. The
  host's zero direction gives a zero `speed`, so no back-off is applied (10427).
- **Impact flag.** `0084BE59 MOV EAX,[ESI+8]` / `0084BE5F CMP [EAX+8],10h` / `0084BE74 SETE` reads the class of
  the **round** in the impact buffer. The host's `weapon` is `bullet(round_bullet_class)`, which every impact path
  sets (8789, 8990, 9014, 9105, 9187), so it is the round's class there too.
- **Falloff.** `blast_falloff_fraction_004705c0` only lifts a positive fraction (`00470602`), and the switch only
  sets the record byte.
- **Draw.** Stream 1 at `0070C247` and at `0084BE4D`. This matches RNG census section 10 row `0070C247`.
- **Landing.** GUNNERY 193 records `flak expiry: bursts 0 -> 126` on E2 and a LOMP10 flak-blast kill as the first
  divergence. Both are the switches' own paths, as predicted. This supports the flips.

**Verdict: FAITHFUL.** Two labelled substitutions are inherited from the proposal and unchanged: the life bound is
`life*V0 > range` rather than `+1C4h > FlyTime`, and `+238h` is taken as the owner unit. One edge case is not
modelled: a round that strikes a unit and expires on the same tick bursts twice in the image, because `0070C370`
does not test death after `006E6490`. The host `continue`s after the strike.

## A2. `kTailGunnerAimErrorBound` and `kTailGunnerOwnAimBound` (GUNNERY 190, ON in 193)

**Host.** `game_hosts_gunnery.cpp` 5776 sets the reset in `set_bot_fire_target_00727f10`. 6937 aims at the origin.
7125-7140 holds the pair. 7173 skips the halving for plane-owned guns.

**Image** (`disasm-raw 008FFA20`, `008FFB2A`, `008FFC90`).

```
008FFB35 FLD [esi+6Ch] ; FSUB [esp+48h] (dt) ; FSTP/FLD ; 008FFB62 FST [esi+6Ch]
008FFB6B FLDZ ; FCOMPI ST(1) ; 008FFB73 JBE 008FFD3F     ; recompute only when 0 > countdown
008FFBC6 CALL target->vtable[44h] ; 008FFC03 CALL [vt+100h] (plane: 0042D810, zeros)
008FFC3D CALL 00414D10 (moved by the target pose) ; 008FFCBD CALL 008FDAF0 -> [esi+70h],[esi+74h]
008FFCD9 CALL 00BD2F10 (0, +84h) ; FMUL qword [00CE3D28] ; FDIV qword [00CE3D20] ; FADD [edi] ; FSTP [edi]
008FFD05 CALL 00BD2F10 (0, +84h) ; ... ; FADD [esi+74h] ; FSTP [esi+74h]
008FFD37 CALL 00BD2F10 (+88h, +8Ch) ; FSTP [esi+6Ch]
```

- **Constants.** `00CE3D28` is `3.1415927410125732`, the float-widened `pi`. `00CE3D20` is `180.0`. So
  `gun_bot_apply_aim_error_008ffa20`'s `angle + draw * pi_f / 180` is bit-exact at 24-bit precision. At 53 bits it
  is within 1 ulp, as the proposal says. I did not establish the thread that runs it.
- **Gating.** The decrement happens only after the side test (`008FFA8D-008FFAA0`), the live-target test and
  `vtable[44h]() != 0` (`008FFAE9-008FFAFE`). The host decrements inside the same aiming branch
  (`gun_bot_turret_recompute_due_008ffa20`). The strict `0 > countdown` matches the helper.
- **Sign.** `+70h` is `008FDAF0`'s output, `-0.0 - atan2`, which is the convention `kGunHorzImageSignBound` gives
  `want_horz` (687, 7014). The draw is added unwrapped in both. The vertical is the asin of the up component in
  both.
- **Resets.** `008FEA40` (the class's `+38h`) stores `-1.0` even for `SetFireTarget(0)`. The tick calls it with 0
  at `008FFA75` (dead or friendly target) and `008FFAE7` (the owner's `vtable[114h]()+221h == 0`). Both clear the
  target, so no aim happens until `00865833` assigns one, and that assignment resets anyway. The host's single reset
  site gives the same countdown on every aiming tick. `unit_gunnery_pass.cpp` 473 calls the hook for the first
  accepted candidate of each gun, as `00865833` does in its candidate walk (`00865822 SUB EBX,1 / JNS`).
- **Swing.** The AA gunner's swing (7056) excludes plane-owned guns. The image's `008FFA20` has no `00902920` call.
- **Landing.** In 193 only USN04 4500 rolls the error (`rolls=103`), and the change is the Kates' tail fire. This is
  the predicted direction.

**Verdict: FAITHFUL.** Inherited and labelled: a ship target's `00816650` hull-box offset. When a tail gun's target
is a ship, `008FFC03` passes the skill-row floats `E199A0[+34h*24h]+20h..+2Ch` to it. I did not check whether
`00816650` draws, so a stream-1 draw may be missing there. Reach for a tail gun with a ship target is likely nil.

## A3. `kTaskProfileActivationOnlyBound` (GUNNERY 187, ON in 193)

**Host.** `game_hosts_units.cpp` `update_dive_bomb_approach` (3962):
- 4010-4013 consume `db_profile_activation_pending` and gate the dive draw and the `+398h` gate on it (4014-4041);
- 4078-4089 run the `+39Ch` gate (`sq_release_alt_set`, `sq_lock_3ab`, `sq_timer_expiry_384`) **every think**,
  under a comment that says "this per-tick update stands for the profile's call".

**Image** (`disasm-raw 009C8920`):

```
009C8931 CALL 007B8AD0 ; 009C8938 JE 009C8A1B        ; leader only, for ALL three blocks
009C895E..009C8971   +394h block (CruisingAlt +4C0h)
009C897C..009C89DD   draw U(0,15) + BeginAltRange/1, +398h block, 009C89DD MOV [edi+3AAh],0
009C89E4 MOV EAX,[ESI+404h] ; ADD EAX,37Ch ; 009C89EF CMP [EAX+12h],0 (+38Eh) ; JNE 009C8A1B
009C89F7 XORPS ; COMISS xmm0,[EAX+8] (+384h) ; JBE 009C8A17      ; 0 > countdown
009C8A00 CMP [EAX+2Fh],0 (+3ABh) ; JNE 009C8A17
009C8A06 +39Ch = [00CE4C04] (9999) ; 009C8A13 +3ADh = 1
009C8A17 MOV [EAX+2Fh],0                                         ; lock cleared on every leader call
009C8A1B ... 009C8A5E +4B0h clamp ; 009C8A7C CALL 009C7A80 ; JMP 0099B740
```

The `+39Ch` block is part of the profile body that `0099AEB3` runs once per activation. The switch moved the
`+398h` block to activation only and left this one per think.

**Effect (A3a).** `008A24E0 SquadronSetReleaseAlt` stores `+384h = 0.5`, `+39Ch`, `+3ABh = 1` and `+3ADh = 0`
(host 16118-16136).
- **Image.** The first dive activation after the call clears the lock and keeps the value. The value then holds for
  the whole dive task, until the next activation finds the countdown expired.
- **Host.**
  1. The first leader think clears the lock.
  2. The first think after `now + 0.5` resets `+39Ch` to 9999.
  3. A call made before the dive task (`bsm_09` sets it at spawn) is therefore used for **one** think.

`control_alt_39c` caps the approach's dive altitude `+A8h` through `009C7A94`: `min(A8, 39C - k*A8)` (4090,
`dive_bomb_decay_dive_altitude_009c7a94`). With 9999 the cap is inert below about 9523 m. With the script's 220 m
it binds, so the host flies that dive at the default altitude instead. This is the `+39Ch` twin of the problem the switch fixed for `+394h`/`+398h` (cadence doc
section 0, point 1).

**Reach.** This installation's loose scripts have 10 live non-forced `SquadronSetReleaseAlt` calls:
- `bsm/bsm_09_battle_of_the_coral_sea.lua` 321: 220 m, a squadron that attacks the Lexington with
  `SquadronSetAttackAlt` 1000/1100;
- `COTP-USN/usn_04_defend_guadalcanal.lua` 2580: level bombers;
- `COTP-USN/usn_07_invasion_of_tarawa.lua` 1708: the Petes when not torpedo-armed, 400 m;
- `usn_12_indochina.lua` (both copies): B-29 and B-17, level bombers;
- `multi/competitive09.lua` 1136.

`+39Ch` is read only by the dive path (host 4080-4088 is the only reader), so only dive-bomber squadrons move: the
bsm_09 and usn_07 cases. No 193 row covers either.

**Verdict: DEFECT** (an incomplete scope, not a wrong reading). Diff A3 adds `kDiveReleaseAltActivationOnlyBound`,
OFF. ON, the block runs only on the activation think. The flag is read before 4012 consumes it. Prediction for a
pair: identical on every 193 row (none calls the native), and a lower dive pull-out on bsm_09's Lexington strike.
Not checked: the dive profile's `+394h` block (`009C895E`, CruisingAlt `+4C0h`) has no host counterpart at all, so a
script travel altitude is never retired by a dive activation. I did not trace whether any dive-task reader uses
`+394h`.

## A4. `kMoveToActivationDrawsBound` with `kMoveToLuaAltZeroOffsetBound` (187 and 190, ON in 193)

**Host.** `game_hosts_units.cpp`, the moveto install (27486-27554), in order:
1. 27503 `moveto_refresh_009beba0()` for the install's `009C1D9D`. This consumes the activation gate (27619).
2. 27504-27516 (`kMoveToLuaAltZeroOffsetBound`): the leader's `sq_lua_alt_3ad = false` when `!sq_alt_set`. This
   stands for the profile's `+3ADh = 1`.
3. 27533-27554 (`kMoveToActivationDrawsBound`): `#mt68`, `#mt424` (`moveto_alt_offset_2c`), `+464h = 0`, then
   `moveto_refresh_009beba0()` for `009C36BF`.

`moveto_refresh_009beba0` zeroes `moveto_alt_offset_2c` while `sq_lua_alt_3ad` is set (27582).

**Image** (`disasm-raw 009C3650`):

```
009C367C CALL 00BD2F10 -> +460h ; 009C36A3 CALL 00BD2F10 -> +424h (approach+2Ch) ; +464h = 0
009C36BF CALL 009BEBA0           ; refresh: +2Ch = 0 while ctl+3ADh == 0 (009BEBAA-009BEBB7)
009C36CA CALL 007B8AD0 ; JE 009C3947                       ; leader only from here
009C372E +38Dh ; 009C373A 0 > +380h ; 009C3744 +3A9h ; 009C3757 +394h = ... ; 009C375A +3ADh = 1 ; +3A9h = 0
```

The refresh that sees the fresh `+2Ch` runs **before** the store that marks the script altitude retired. The host
makes the store at step 2, before the draws and their refresh.

**Effect (A4a).** Take a moveto activation whose gate retires a script altitude: `+3ADh == 0`, the countdown
expired and the lock already cleared by an earlier activation.
- **Image.** `009C36BF` still sees `+3ADh == 0` and zeroes the `+2Ch` just drawn. `009C375A` then sets `+3ADh`.
  `+2Ch` stays 0 for the task, because it is a store and later refreshes see `+3ADh == 1`. The task flies at the
  default cruise altitude exactly.
- **Host.** Step 2 sets `sq_lua_alt_3ad = false` first. The activation's refresh then keeps the drawn
  `U(0, TravelAltRandom)`, so the task flies 0..TravelAltRandom lower.
- **No divergence** in either of the other cases:
  - the lock still set: `sq_alt_set` stays true, so step 2 does nothing;
  - no script altitude: both have `+3ADh == 1`.

The draws are made in both, so the RNG streams do not move. Only the altitude does.

The interaction exists only with both switches ON. `kMoveToLuaAltZeroOffsetBound`'s comment at 27505-27510
("LABELLED: the refresh above stands for that call") was written when the install's refresh was the only one.

**Reach.** A second moveto activation after a non-forced `SquadronSetTravelAlt` (or `SetAttackAlt`) is required.
The first activation clears the lock and the second retires the value. The cadence doc counts 428 non-forced calls
in 86 files. 193's USNRM01 and JM08 re-pairs are the likeliest rows.

**Verdict: DEFECT** (an ordering between two switches that were each faithful alone). Diff A4 adds
`kMoveToActivationLuaAltOrderBound`, OFF. ON, step 2 moves after the activation's refresh when the activation
draws are bound. Prediction: identical on 187's rows, and on USNRM01/JM08 some second-task movetos fly lower by up
to TravelAltRandom. Not checked: the image's moveto profile also has a `007B93F0` arm and a tuning `+43Ch` test
(`009C36F6`, `009C370C`) that the host's `cruise` does not model. That predates both switches.

## A5. `kStrafeDoubleConstantsBound` (GUNNERY 190, ON in 193)

`tools/pe_const_read.py d:00d05858 d:00d05850 d:00d057d8` gives `0.3490658700466156`, `0.1745329350233078` and
`0.6108652551968893`, the host's ON literals (`game_hosts_units.cpp` 21233, 21264, 21432).

- `009CB879 CALL 00BD2F10` / `009CB87E FMUL qword [00D05858]` / `009CB884 FMUL qword [00D7A250]` (-1.0) /
  `009CB88A FSTP [edi+1Ch]`. The host computes `double(draw) * c * -1.0` and rounds once.
- `009CB6F8 FADD qword [00D05850]` / `FSTP [esp+0Ch]`, then the min with `class+1ECh`. The host rounds
  `float(p + c)` and takes the min.
- `009CAB35 CALL 00438B10`.
  - `009CAB3E-009CAB77` builds `|err|`: if `err > 0` it stores `err`, else `[00D7A208] - err`, where
    `00D7A208` is `-0.0f`.
  - `009CAB7B FLD qword [00D057D8]` / `FCOMPI` / `JBE` sets the byte when `c > |err|`. The host's
    `fabs(err) < c` is the same, and promotes to double.
- `00CEC730`, the `pi/6` divisor at 21225, is `0.5235987901687622` as a double. The host's literal matches.

Precision: at 24 or 53 bits, each x87 step rounds no finer than the host's double, then once to float. A
double-rounding tie is the only possible difference.

**Verdict: FAITHFUL.**

## A6. The six dive draw pins (GUNNERY 190, ON in 193)

**Image** (`disasm-raw 009C8500`, `009C7800`, `009C5786`, `009C6100`):

```
009C8515 MOV EAX,[ESI+798h] ; TEST ; JNE 009C85B8      ; latched side wins
009C8531 COMISS bank,[00CE398C] (0.34906587) ; JBE     ; bank > 20 deg -> +1
009C8563 COMISS [00D1FBC0] (-0.34906587),bank ; JBE    ; bank < -20 deg -> -1
009C8587 FLD [00D05B54] (1024.0) ; ECX=1 ; 009C859E CALL 00BD2F10 ; CALL 00BF7420 ; CMP EAX,200h ; SETGE
009C85B2 LEA EDX,[EDX+EDX-1]   ; +1 / -1
009C7800: side >= 0 -> [00D7A24C] else [00D7A260] ; 009C7839 U([00CE74F8]=0.8, 1.0) ; FMUL side ; FSTP +18h
009C579E U([00CE54A0]=0.2, [00CE3800]=0.5), ECX = EBX ; 009C6114 U(0.5, 1.0), ECX = 1
```

- **Host.** `write_turn_direction` (19058) flips `roll` when `trunc(coin) < 0x200` under the same three-way gate
  (`!(bank > hi) && !(bank < lo)` with `+798h == 0`). The sign product is exact. The bank constants (4716-4717)
  equal the image's floats.
- **Latch** (24855-24871). `009C6B86 FCHS / FSTP [ESP+10h]` stores `-a0`. The two `SUB ESP,14h ... CALL 00419010`
  windows between `009C6B86` and `009C6CCA` write `[ESP+10h]` only in their shifted frames (base + 24h), and
  `00419010` pops 14h. So `009C6CE0`'s else arm reloads `-a0`, and the value is `|a0|`.
  - `009C6CF0 FSUB [ESP+74h]` rounds once to float. `009C6D1A COMISS [00D7A238]` (0.01f) and `JBE` latch when
    `|d| <= 0.01`.
  - `009C6D29 JB` gives `+1` for `a0 < 0`.
  - The gate `00D7A270` is the qword `0.05000000074505806`, which is `double(0.05f)`, as the host compares.
- **Order.** Unkeyed, the image draws the coin before the magnitude, and the host draws them the other way round.
  Keyed streams make this moot, and the host's comment at 19075 says so.
- **Landing.** 193 records dive-row-only divergence, as predicted.

**Verdict: FAITHFUL.**

## A7. The fly-over trio (GUNNERY 192, ON in 193)

**Image.**
- **Enter** `009C6270`: `009C6276 CALL 009C3DA0`, `[unit+844h] = 0`, `+20h = 0`, `+1Bh = +19h = +18h = +1Ch = 0`,
  and `+1Ah = (approach+D1h == 0)` (`009C62A1 SETE`). The host's enter (23658-23677) seeds the same four bytes and
  the side. `+844h` is not modelled. I did not trace its readers.
- **Dive test** `009C67BF-009C6842`:
  - `BL == 0` ends the step.
  - `DL = [ESP+27h]`. With `DL == 0` the step needs `+1Ah != 0` and an entry `+1Ah` (`[ESP+4Bh]`, from `009C653C`)
    of 0.
  - `+18h = (B > approach+D4h)` (strict `FCOMIP`, `JBE`).
  - `+1Ah = 0`, `+19h = [ESP+43h]` (the entry `+19h`, `009C6540`), and plan `+2E8h = [00D06874] = 1.4`.
  - `dive_bomb_flyabove_flags_009c659f` (`src/dive_bomb_task.cpp` 979-1026) has the same sequence, and the host's
    1.4 store follows `reached_dive_test` (25105).
- **`[ESP+27h]`.**
  - It is written at `009C64D7` from `ctl+3A8h` (or 0 with no control block, `009C64DD`). The `PUSH 14h` /
    `CALL vtable[5Ch]` pair after it is balanced.
  - `DL != 0` bypasses the `+1Ah` tests (`009C67D3`) and also the `009C6554` skip.
  - The host's `kOldStyleBombing1b = false` (`dive_bomb_task.hpp` 882-889) labels this. No loose script in this
    installation contains `OldStyleBombing` (0 matches, commented or not).
- **Cross-track.** `x87_dc_form_scan` finds three `DC`-form sites in `009C62B0`:
  - `009C6875 DC C9`: `ST1 = sin * 0.5`, `ST0 = 0.5`, then `FADDP`, then `FMUL R`. That is `(0.5 + 0.5 sin) R`.
    The host (`dive_bomb_task.cpp` 915-919) stores `sin` as a float first, as `009C685D` does.
  - `009C6328 DC C9`: the three-second lead, `3 dx` / `3 dz`, already bound (host 4567-4568).
  - `009C637D DC C8`: a square in the range.
- **Landing.** 193 records the trio's divergence on dive rows only.

**Verdict: FAITHFUL** on the parts read. Not re-derived:
- the `009C67B0` roll-in rule's operands;
- the `009C6929` latched arm's yaw;
- `+2C8h` (`009C6678`).

## O1. The strafe profile `009CD020` draws at activation; the host never does (outside the switch list)

```
009CD02E CALL 007B8AD0 ; 009CD035 JE 009CD0AF         ; leader only
009CD03C..009CD0A0    +394h / +398h gates with Strafe CruisingAlt (tuning+654h)
009CD0A4 LEA ECX,[ESI+3F8h] ; 009CD0AA CALL 009CA3B0   ; glide seed: 009CA41E U(0.8,1.2), 009CA459 U(-10,40)
009CD0AF FLDZ ; 009CD0BB CALL 009CCED0(0)              ; every unit
```

`docs/CC13_TASK_PROFILE_CADENCE.md` section 9 lists the strafe profile as not hosted, but mentions only its
altitude gate. The leader branch also re-seeds the glide (`+2Ch` angle, `+40h` distance) with two stream-1 draws.
It reads `ctl+398h`, which this same call has just rewritten.

The image runs that branch at the strafe install and every time the strafe resurfaces as head. The second case
follows each tightturn or flikflak that `kStrafeBreakoffBound` front-pushes and retires. The host's
`st_glide_seed_009ca3b0` (`game_hosts_units.cpp` 20925-20955) runs only from the go-towards enter sites.

So the leader's glide angle and offset after an activation are the profile's draws in the image and the enter's in
the host. Keyed streams keep the other draws in place. No diff is offered here: the activation hooks for strafe do
not exist yet, and it is not one of the recent flips.

## Diffs

### Diff A3: `kDiveReleaseAltActivationOnlyBound` (OFF)

Applied in this tree and built with `./scripts/build.ps1` (pass, 0 compiler errors; ctest 2/2), then reverted. `git apply --check` against
`origin/main` is clean.

```diff
diff --git a/include/bsp/game_hosts_units.hpp b/include/bsp/game_hosts_units.hpp
index 2c4efb53e..d3bdf1f60 100644
--- a/include/bsp/game_hosts_units.hpp
+++ b/include/bsp/game_hosts_units.hpp
@@ -459,6 +459,13 @@ inline constexpr bool kSquadronTravelAltBound = true;  // ON: pairs held (docs/L
 // once per install; the dive draw is made then. False: they run per think (the
 // moveto gate per 0.5 s refresh), as before.
 inline constexpr bool kTaskProfileActivationOnlyBound = true;  // GUNNERY 193: ON by pair verdict
+// Packet cc13-audit13 (docs/CC13_AUDIT13_RECENT_FLIPS.md, A3). The dive profile's
+// third block, the +39Ch release-altitude gate (009C89E4-009C8A17: +38Eh, +384h < 0,
+// +3ABh, the 9999 write 009C8A0E, the lock clear 009C8A17), is in the same leader
+// branch as the +398h block. True: with kTaskProfileActivationOnlyBound it runs at
+// activation only, so a non-forced SquadronSetReleaseAlt lasts for the task. False:
+// per think, which ends the script value one think after its 0.5 s countdown.
+inline constexpr bool kDiveReleaseAltActivationOnlyBound = false;
 
 inline constexpr bool kPlaneRowPositionBound = true;  // ON: mechanism held, one premise miss (docs/CONTROLLED_UNIT.md)
 
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 69e209f45..7a9fd15d4 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -3973,6 +3973,8 @@ struct GameUnitsHost::Impl {
         const bsp::OrdnanceKindSet set{slot.ordnance_mask};
         slot.db_has_bomb_d1 = bsp::ordnance_has_general_bomb_2ah(set) &&
                               dive_bomb_rounds_remaining(slot) > 0;
+        // Read before the +398h block below consumes it (0099AEB3).
+        const bool db_profile_activation = slot.db_profile_activation_pending;
         // 009C7A96: approach+ACh = ctl+398h, which the cruise profile 009C8920
         // holds at Pilot/DiveBomb/BeginAltRange/1.
         slot.db_begin_alt_ac = kPilotDiveBombBeginAltRange1;
@@ -4078,7 +4080,14 @@ struct GameUnitsHost::Impl {
         float control_alt_39c = bsp::dive_bomb_constant::kCruisingAltitudeThird;
         if (GameUnitSlot* rb = squadron_slot_of(slot.process_index);
             rb != nullptr && rb->sq_release_alt_set) {
-            if (unit_is_flight_leader_007b8ad0(slot.process_index) && !rb->sq_freeze_38e) {
+            // kDiveReleaseAltActivationOnlyBound: the block is 009C8920's (009C89E4-
+            // 009C8A17, inside the 009C8938 leader branch), so with
+            // kTaskProfileActivationOnlyBound it runs at activation only, as the
+            // +398h block above does.
+            const bool profile_call = !kDiveReleaseAltActivationOnlyBound
+                || !kTaskProfileActivationOnlyBound || db_profile_activation;
+            if (profile_call && unit_is_flight_leader_007b8ad0(slot.process_index)
+                && !rb->sq_freeze_38e) {
                 if (summary.simulated_seconds >= rb->sq_timer_expiry_384 && !rb->sq_lock_3ab) {
                     rb->sq_release_alt_set = false;
                     rb->sq_lua_alt_3ad = false;   // 009C8A13 +3ADh = 1
```

### Diff A4: `kMoveToActivationLuaAltOrderBound` (OFF)

Applied in this tree together with diff A3, built with the same `./scripts/build.ps1` run, then reverted.
`git apply --check` against `origin/main` is clean for this diff alone and for both together.

```diff
diff --git a/include/bsp/game_hosts_units.hpp b/include/bsp/game_hosts_units.hpp
index 2c4efb53e..0bbd19ef2 100644
--- a/include/bsp/game_hosts_units.hpp
+++ b/include/bsp/game_hosts_units.hpp
@@ -470,6 +470,13 @@ inline constexpr bool kSubmarineDiveTeleportBound = true;  // ON: pairs held (do
 // True: the steer altitude drops the random offset while a script altitude is
 // live. False: cruise - offset always.
 inline constexpr bool kMoveToLuaAltZeroOffsetBound = true;  // GUNNERY 193: ON by pair verdict
+// Packet cc13-audit13 (docs/CC13_AUDIT13_RECENT_FLIPS.md, A4). With the moveto
+// activation draws ON, the leader's +3ADh = 1 store (009C375A / 009C3906) belongs
+// after the activation's own refresh 009C36BF, not after the install's 009C1D9D.
+// True: the store moves there, so a script altitude that the activation retires
+// still zeroes the freshly drawn +2Ch. False: the store follows the install's
+// refresh, and the activation's +2Ch survives.
+inline constexpr bool kMoveToActivationLuaAltOrderBound = false;
 
 class GameUnitsHost {
 public:
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 69e209f45..1cc64aaf7 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -27501,7 +27501,10 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 n + "#mt54c", 0.0f, 1.0f);
                             unit_.moveto_dwell_60 = 0.0f;       // 009C1CFD
                             moveto_refresh_009beba0();          // 009C1D9D
-                            if constexpr (kMoveToLuaAltZeroOffsetBound) {
+                            if constexpr (kMoveToLuaAltZeroOffsetBound
+                                          && !(kMoveToActivationLuaAltOrderBound
+                                               && GameUnitsHost::Impl::kMoveToActivationDrawsBound
+                                               && bsp::kMoveToTaskTickBound)) {
                                 // The activation profile 009C3650 refreshes first
                                 // (009C36BF), then, for the flight leader only
                                 // (009C36CA 007B8AD0), stores +3ADh = 1 with its
@@ -27551,6 +27554,18 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 n + "#mt424", 0.0f, rnd);               // 009C36A3, +424h
                             unit_.moveto_approach_6c = 0.0f;            // 009C36B7, +464h
                             moveto_refresh_009beba0();                  // 009C36BF
+                            if constexpr (kMoveToLuaAltZeroOffsetBound
+                                          && kMoveToActivationLuaAltOrderBound) {
+                                // 009C36CA..009C375A: the leader's +3ADh = 1 follows
+                                // the 009C36BF refresh, so that refresh still sees a
+                                // live script altitude and zeroes the +2Ch drawn at
+                                // 009C36A3.
+                                GameUnitSlot* blk =
+                                    owner_.squadron_slot_of(unit_.process_index);
+                                if (leader && blk != nullptr && !blk->sq_alt_set) {
+                                    blk->sq_lua_alt_3ad = false;
+                                }
+                            }
                         }
                         owner_.log.notef("  moveto task %s: installed kind 7, leader=%d state=%s "
                             "range=%.1f (009C3000)", unit_.row.name.c_str(), leader ? 1 : 0,
```

## Summary for cc9

- A1, A2, A5, A6 and A7 are faithful. Their 193 divergences are each switch's own path.
- **A3a (DEFECT).** `kTaskProfileActivationOnlyBound` left `009C8920`'s `+39Ch` block (`009C89E4-009C8A17`) per
  think. A non-forced `SquadronSetReleaseAlt` therefore lasts one think instead of the dive task. Pair diff A3 on
  bsm_09 (the Lexington strike, release altitude 220 m) or usn_07 (the Petes, 400 m). No 193 row reaches it.
- **A4a (DEFECT, an interaction).** With `kMoveToActivationDrawsBound` and `kMoveToLuaAltZeroOffsetBound` both ON,
  the leader's `+3ADh` store runs before the activation's own refresh instead of after it (`009C36BF` then
  `009C375A`). A moveto whose activation retires a script altitude keeps its random offset in the host and drops it
  in the image. Pair diff A4 on USNRM01 or JM08 `l50_j8_r7` (the 190 re-pair rows).
- O1: the strafe profile's two activation draws (`009CA41E`, `009CA459`) are missing in the host. This adds to the
  cadence doc's open item.

## Handoff

Nothing is in flight. Possible next steps:
- trace `00816650` for a draw (A2);
- trace the readers of `+394h` during a dive task (A3);
- trace the readers of `unit+844h` (A7);
- give the strafe task activation hooks (O1).
