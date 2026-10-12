# Audit of the planner roll arm and the torpedo pursuit bearing (cc13-audit10, part 2)

Addresses: 0099DE8A 0099DEB8 0099DEBD 0099DF87 0099DF8B 0099DFB2 0099DFCC 0099E02B 0099E07E 0099E087 0099E0C1 0099E10E
0099E110 0099E127 0099E17B 0099E180 0099E19E 0099E1CC 0099E25C 0099E275 007B95E0 009D3519 009D3586 009C5408 009C5450
009C5DB2 0099E756

Packet `cc13-audit10` (follow-up), branch `agent/cc13-audit10`, 2026-10-12. cc9 requested it through the cc13 lead,
after docs/CC13_AUDIT_TORPEDO_BOMBER.md. **Read only.** This audit made no Ghidra writes, added no ledger records and
edited none of cc9's files. Its one diff is at the end, behind an OFF switch.

Base: `origin/main` at `3b265bfef`. That base has **neither** cc13-audit7's diff (defect B, `gl_yaw_mode_2d4_zero`
in aimglide/aimdive) **nor** the `pilot+264h` fix (`kBandShaperPlan264Bound`). The verdict below does not depend on
either. Where they interact, the doc says so.

**Method.** As in part 1:
- the disk listing via `bsp.py disasm-raw`, with ESP walked by hand through every `SUB ESP`, `PUSH` and `RET n`
  between a store and its reload;
- constants via `tools/pe_const_read.py`;
- displacement censuses via `local/au10_disp.py`;
- the bank arithmetic evaluated by `local/au10_rollfp.py` on this installation's values:
  - `vehicleclasses.lua` VehicleClass[112], TBD Devastator, mtime 2026-05-09;
  - `planeglobals.lua` PilotGeneral, mtime 2024-10-29.

**Status vocabulary.** Every function named here is reconstructed and build-tested on main. This audit is a static
reading and adds no runs. The numbers in finding 1 are arithmetic on the host's own law, not game measurements. The
diff was apply-checked and built (passed) in this tree with the switch OFF.

## Verdict

**The steady 0.19-0.33 rad bank on a 0.02-0.05 rad heading error is a host defect.** The image's roll arm does not
see the raw heading error. It sees the planner's yaw base numerator: the error with a 0.01 rad zone removed and,
below the soft limit, multiplied by `SoftHdgMul` = 0.35. The host feeds the raw error, so for small errors it asks for
about four times the image's bank. The law itself (`(h - s) / c * max_bank`, GUNNERY 159) is faithful. Its input is
not.

## Findings

| # | item | verdict | one line |
| --- | --- | --- | --- |
| 1 | roll arm input `h` (`0099E127`, `0099E02B`) | **DEFECT** | the image reads frame `[ESP+6Ch]`, the base numerator from `0099DF87`; the host passes the raw `00438B10` error |
| 2 | roll arm law `0099E07E`..`0099E25C` | FAITHFUL | `w = min(1.5|bank|, max_bank)`, `c = 0099D0A0(w)` with the 0.01 / -0.1 floors, `s = interp(-c, -0.3c, c, 0.3c; h)`, `clamp((h - s)/c) * max_bank`, the pitch-scheduled limit and its `[-2, 2]` clamp |
| 3 | `max_bank` source `0099DF8B`..`0099DFF5` | DEFECT (minor, not in the diff) | `007B95E0` picks TurnRollLeader (`class+260h`) for a squadron leader; the host always uses TurnRoll |
| 4 | the pursuit bearing `009D3519`..`009D35C0` | FAITHFUL (precision note) | `pi/2 - atan2(dz, dx)` wrapped by `2 pi`; the range's sum of squares is rounded to float before `sqrt` in the image |
| 5 | interaction with audit7's defect B | NO INTERACTION for the torpedo aim | aim never writes `plan+2D4h`; aimdive skips the roll arm (`plan+2CCh = 0`) |

**1, DEFECT.**
- **The slot.** `0099DE8A CMP ECX,2` / `JNE 0099E26E` admits only mode 2. Let `E` be ESP at `0099DEA0`.
  - `0099DEB8` computes `00438B10(plan+2C0h, unit->vtable[50h]())`. `0099DEBD FMUL [ESP+28h]` applies the turbo
    scale, which is 1.0 here.
  - `0099DECC..0099DF03` is the deadband `tuning+3Ch`: it subtracts or adds the zone, and the middle band is
    `XORPS`, giving 0.
  - `0099DF09..0099DF81`: below `limit = (class+1C8h + class+1ACh) * tuning+38h` the value is `x tuning+34h`.
    Otherwise it is moved toward zero by `(1 - tuning+34h) * limit`.
  - `0099DF87 FSTP [ESP+6Ch]` stores it at `E+6Ch`. The host's `yaw_base_numerator_0099de8a` is this exactly.
- **The walk to the reload.** From `0099DF87` to `0099E10E` the net ESP change is zero:
  - `0099DF8B` is a thiscall with no stack arguments, as is `0099DFCC` (`0047B880`);
  - `0099DFFF` and `0099E058` each do `SUB ESP,14h` and then call `00419010` (`RET 14h`);
  - `0099E0BB PUSH ECX` and `0099E0C1` call `0099D0A0` (`RET 4`).
  - No store in that window addresses `E+6Ch`.
  - **`0099E02B MOVSS XMM0,[ESP+6Ch]`** reads it at depth 0 for the ramp `interp(tuning+8Ch -> 0, tuning+90h -> 1;
    |base|)`, which is `PitchTurnHdgRange`.
  - **`0099E127 FLD [ESP+80h]`** reads it after `0099E110 SUB ESP,14h`, which is `E+6Ch`. It is pushed as the x of the
    deadband interpolation (`0099E140 FSTP [ESP+10h]`) and kept as a double at `0099E136 FST qword [ESP+70h]`
    (`E+5Ch`). `0099E180 FSUBR qword [ESP+5Ch]` (depth 0, `E+5Ch`) then forms `h - s` from that same copy.
- **Host.** `rin.heading_error = term.heading_error` in `game_hosts_units.cpp`'s roll-input block. That is the raw
  error, and `pilot_plan_roll_0099e2ba` uses it both for `s`/`raw` and for `hdg_ramp`.
  - `scratch.base_num` is already computed a few lines above (`bsp::yaw_base_numerator_0099de8a(term)`) and zeroed
    outside mode 2 (`kPlannerYawBaseModeGateBound`).
  - GUNNERY 159's table reads the law and writes `h` without its provenance; this is the term it missed.
- **Values.** SoftHdgZone 0.01, SoftHdgLimit 0.5, SoftHdgMul 0.35 (`planeglobals.lua` lines 362-364). For the TBD the
  limit is `(TurnRollSpd 0.087266 + PitchSpd 0.436332) x 0.5` = 0.262 rad. So for `|h| < 0.272`,
  `base = 0.35 (|h| - 0.01)`.
- **Arithmetic** (`au10_rollfp.py`; `max_bank = min(TurnRoll 1.047, TurnRollLimitLarge 56 deg)` = 0.977, pitch command
  at the 0.1 floor). The bank at which the arm asks for exactly the current bank:

  | heading error h (rad) | host (raw h) | image (base numerator) |
  | --- | --- | --- |
  | 0.020 | 0.176 | 0.040 |
  | 0.035 | 0.224 | 0.121 |
  | 0.050 | 0.261 | 0.150 |
  | 0.100 | 0.350 | 0.214 |

  - The host column reproduces 148's measured 0.19-0.33 rad on 0.02-0.05 rad of error. The image asks for half to a
    quarter of that, and inside 0.01 rad it asks for nothing.
  - These are equilibria of the arm's ask, not a flight simulation. In a pursuit the heading error adjusts until the
    turn rate matches the bearing rate. The image will therefore carry a somewhat larger error for the same turn,
    while banking much less whenever the bearing is nearly steady.
- **Consequence for the TBD hold** (part 1). Less bank shrinks the pass-0 band shaper's `g`, the band relaxes, and
  the aim descent can follow its command.
  - `kBandShaperPlan264Bound` (part 1 finding 3) works the other way, making banked bands stricter. The two should
    be paired together and apart.
  - Expected direction with both ON: TBDs bank 0.04-0.15 rad. That gives `g` of 0.11-0.43 with `pilot+264h` = 0,
    against today's 0.30-0.90, so the descent should continue below 106 m. This is a prediction, not a measurement.
- **Scope.** The roll arm runs only in mode 2 (`0099E275`). That covers every task heading hold: moveto, follow,
  attackrun, torpedo aim, the aimglide command arm (`009C5450` writes `+2CCh` = EBP = 2) and the dogfight headings.
  Pair it broadly.

**2, FAITHFUL.**
- `0099E087 FMUL qword [00CE3D78]` (1.5) and the min against `max_bank`.
- `0099E0C1 CALL 0099D0A0`. Then `0099E0CE` compares against `[00D7A238]` (0.01) and `[00D7A300]` (-0.01). The
  replacement is `[00D7A238]` or `[00CE3CB4]` (-0.1) by sign.
- The deadband (`0099E110..0099E17B`): `[00CE3DC8]` is 0.3 (double). `0099E14C FMUL ST1,ST0` writes ST1 (GUNNERY 159's
  note), so the endpoints are `(-c, -0.3c)` and `(c, 0.3c)`.
- `0099E18D FDIV [ESP+1Ch]` is `E+18h`, the floored `c`. `0099E199 CALL 00415620` clamps to `[-1, 1]`, and
  `0099E19E FMUL [ESP+10h]` is `E+10h`, `max_bank`.
- The limit: `00419010(tuning+6Ch, +78h, +70h, +74h; [E+3Ch])`, maxed with `|bank|` (`JA`). `00415690` then clamps it
  with `[00CE3958]` 2.0 and `[00CE7D7C]` -2.0. The bank target is clamped to `+-limit` and stored at `0099E25C`
  (`plan+2C4h`), with `+2CCh = 1`.

**3, DEFECT (minor).**
- `0099DF8B CALL 007B95E0` returns 1 when `unit+9D8h == 0`, `unit+9D4h` (the squadron) is non-null and
  `squadron+3BCh > 0`. That case selects `class+260h` TurnRollLeader (`0099DF9A`); otherwise `class+25Ch` TurnRoll
  (`0099DFAA`). The product is `x plan+2E8h` (`0099DFB2`) and capped by `tuning+64h`/`+68h` (`0047B880`).
- The host's `max_bank` uses `scale.turn_roll` (TurnRoll) for every aircraft. The slot already carries
  `plane_class_turn_roll_leader`.
- **TBD:** the leader's 0.873 is below the 0.977 cap, so a leader banks at most 0.873 in the image against 0.977 in
  the host. Wingmen are unaffected.
- **Part 1 correction.** For the same reason the aim tick's `plan+2E8h = 1.4` is **neutral for a TBD wingman**:
  `1.4 x 1.047` and `1.0 x 1.047` both exceed TurnRollLimitLarge 0.977. For a leader the image gives
  `min(1.4 x 0.873, 0.977)` = 0.977.
- `squadron+3BCh` was not identified in the host, so this finding is not in the diff.

**4, FAITHFUL (precision note).**
- **Range.** `009D3519..009D352A` forms `dx`, `dz` (target minus aircraft). `009D3536..009D353E` computes
  `dz^2 + dx^2` and **stores it as a float** (`FSTP dword [ESP+10h]`). It is compared with the double 1e-10 (`JBE`
  gives 0), then reloaded for `00BF7030` (`sqrt`). The host computes the sum in double and does not round. That is a
  sub-ulp difference in `approach+90h`, recorded only.
- **Bearing.** `FLD dz`, `FLD dx`, `CALL 00BF701A`, i.e. `atan2(dz, dx)`, stored as a float. Then `FSUBR qword
  [00CE3830]` (pi/2), stored as a float, and `FLDZ`/`FCOMIP`/`JBE` adds the double `2 pi` when it is negative. This
  matches `torpedo_approach_bearing_009d3586`.
- The bearing is refreshed every approach update to `approach_target_point`, the lead point. That makes it a pursuit
  of a moving point, as in the image. The lead point's producer (docs/TORPEDO_AIM_LEAD.md) was not re-read here.
- Nothing in the bearing makes it rotate with the aircraft's own turn. It moves only with the target point and the
  aircraft's position.

**5, audit7's defect B.**
- **Torpedo aim: no interaction.** A displacement census of `+2D4h` over `009CF000..009D5000` finds only `009CF2E7`
  and `009CF6E8`, neither in the aim tick. The aim therefore leaves the reset's `plan+2D4h = 1`, and `0099E756` lets
  the yaw arm `0099E81A` run in the image as well. B's fix does not change the TBD.
- **Aimdive.** It writes its own roll slot (`009C5DA3` `+290h`, `+294h = 1`) and `plan+2CCh = EBX = 0`
  (`009C5DB2`). `0099DE8D` then skips both the base numerator and the roll arm, so finding 1 does not reach the
  aimdive. The planner's interference with the aimdive's lateral steering is B's yaw overwrite, not this.
- **Aimglide.** The yaw arm (`009C5408`, mode 1, `+2C4h = 0`) skips the roll law. The command arm (`009C5450`,
  mode 2) runs it, so finding 1 affects aimglide's command arm.
- **The dive bombers' cross-track problem.** Finding 1 is a candidate only for the aimglide command arm and the
  approach states before the dive. It is not one for aimdive, where B is the planner term in play.

## The diff

Applied in this tree, built with `./scripts/build.ps1` (passed, 2/2 tests), reverted, and confirmed with
`git apply --check` on a copy extracted from this doc. It is independent of part 1's diff and of audit7's: different
hunks, and it applies with or without them.

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..7a950a5e5 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -5636,6 +5636,11 @@ struct GameUnitsHost::Impl {
     // Packet cc9_near_field_probe: 007F0280 at the dive-bomb attack run
     // (009C42B8) and the fly-over speed slot (009C6B3A). docs/NEAR_FIELD_PROBE.md.
     static constexpr bool kNearFieldProbeBound = true;
+    // cc13-audit10 (docs/CC13_AUDIT_PLANNER_ROLL.md finding 1): the roll arm's
+    // h is the yaw base numerator [ESP+6Ch] (0099DF87), not the raw heading
+    // error. 0099E127 loads it for the deadband and the bank ask, and 0099E02B
+    // for the pitch-floor ramp. Behaviour-changing; pair before ON.
+    static constexpr bool kRollArmBaseNumeratorBound = false;
     // Packet cc9_plane_gun_pass: 009AAA80's early maneuver -> aim edge, and the
     // latched gunFire (unit+BC9h) the plane's fixed step hands to every gun's
     // SetTriggerHeld (007CE9A0-007CE9F4). docs/PLANE_GUN_PASS.md.
@@ -30437,6 +30442,15 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // never leave them in.
                         bsp::PilotBotRollInputs rin;
                         rin.heading_error = term.heading_error;
+                        if constexpr (GameUnitsHost::Impl::kRollArmBaseNumeratorBound) {
+                            // 0099E127 FLD [ESP+80h] after SUB ESP,14h at
+                            // 0099E110 is frame [ESP+6Ch], which 0099DF87
+                            // stored: SoftHdgZone-deadbanded, SoftHdgMul-scaled.
+                            // 0099E02B reads the same slot for the ramp. Both
+                            // are 0 outside mode 2, and the arm then does not
+                            // run (0099E275).
+                            rin.heading_error = scratch.base_num;
+                        }
                         rin.bank = unit_.plane_bank_angle_c68;
                         rin.dt_scale = 1.0f;
                         rin.turn_scale_2e8 = unit_.plan_state.turn_scale_2e8;
```

## Handoff

- Pair `kRollArmBaseNumeratorBound` alone first on USN04 (the TBDs' bank and their minimum altitude in `aim`), on a
  fighter mission and on a dive mission. Then pair it with `kBandShaperPlan264Bound`.
- Finding 3 needs `squadron+3BCh` identified before it can be bound.
