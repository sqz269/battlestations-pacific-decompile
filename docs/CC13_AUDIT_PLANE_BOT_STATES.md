# Audit of the plane bot states in cc9's motion step (cc13-audit5)

Addresses: 009C58D0 009C5A5E 009C5AA3 009C5AF1 009C5C9B 009C5D33 009C5D60 009C5DB8 009C5F79 009C5FA7 009C60C1
009C6114 009C7B27 009C7EA0 00438B10 00438AA0 00419010 00BD2F10 009B5C80 009B4D00 009B5320 009B5B00 009CDD50 009CC690
009CC2F0 009CCED0 006C5E20

Packet `cc13-audit5`, branch `agent/cc13-audit5`, 2026-10-12. Requested by cc9 through the cc13 lead. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files. Findings that need a change are given as unified diffs
for cc9's lanes to apply. Base: `origin/main` at `5a4509b2c`.

**Method.**
- Each host reconstruction the motion step calls (`src/game_hosts_units.cpp`, `src/dive_bomb_task.cpp`,
  `src/dive_bomb_aimdive_tail.cpp`, `src/plane_level_bomb_task.cpp`, `src/strafe_task.cpp`) was compared with the disk
  listing (`bsp.py disasm-raw`).
- ESP was tracked by hand from each frame base, so `SUB ESP,14h` and `SUB ESP,8` argument blocks are not read as
  displacements. `tools/x87_sym.py` confirmed the two load-bearing slot reads.
- Constants were read from the PE at the load's own width (`local/au5_const.py`: `movss` and `fld dword` as float,
  `fld qword` as double).
- Every x87 `FCOMI(P)` and SSE `COMISS` was read with its operand order and its `Jcc`. `JBE` is taken on CF|ZF,
  which includes unordered; `JA` and `JB` are the strict forms.

**Checklist.**
- Rule 2 (boundary before attribution): `ghidra proto --brief` puts each key site in its cited function:
  - `009C5FA7` and `009C60EC` are in `BSP_BotStateDiveBombAimDive_Tick` (`009C58D0`..`009C6161`);
  - `009C7EDB` is in `BSP_BotStateDiveBombTurnDown_IsComplete` (`009C7EA0`..`009C7EFB`);
  - `009B5DC3` and `009B6394` are in `BSP_BotStateLevelBombAim_Tick` (`009B5C80`..`009B6467`);
  - `009B54D5` is in `BSP_BotStateLevelBombAttackRun_Tick` (`009B4D00`..`009B55B5`). The host's
    `level_bomb_attackrun_009b5320` is its tail from `009B5320`, not the whole function;
  - `009CDDF1` is in `BSP_TakeoffStatePrepare_Enter`;
  - `009CC7C6` is in `FUN_009CC690`, `009CCFF4` in `FUN_009CCED0` and `006C5FF2` in `FUN_006C5E20`.
- Rule 7 (argument counts): `00419010` is `RET 14h`, `00438B10` and `00438AA0` are `RET 8`, and `00BD2F10` is `RET 8`.
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table, so there is nothing for it
  to check.

**Status vocabulary.** Everything below is reconstructed and build-tested on main. This audit is a static second
reading and adds no runs.

## Verdicts

| # | state / routine | verdict | one line |
| --- | --- | --- | --- |
| 1 | aimdive roll arms `009C5D33` / `009C5D60` | **FAITHFUL** | Both arms are `InterpolateClamped(-band, +1, +band, -1, H - bearing)`, with the same `00438B10(H, b)` order and no `FCHS`. The flip at the along-track zero is the image's own arm change |
| 2 | aimdive pitch arm `009C5C9F`-`009C5CEF` and the steep gate | FAITHFUL | The gate is `JA` on `pitch > -30 deg`. The sign gate is `FCOMI err,0` / `JBE`. Clamps are `JBE` against `+1` and `-1` |
| 3 | aimdive aim error `009C5BBC`-`009C5C9B` | FAITHFUL | `(cos(err18) x dist(I,T) - lead) x gain`. `FSUBR` makes the memory operand the minuend |
| 4 | aimdive dive abort `009C5AFD`-`009C5B48` | FAITHFUL | Three strict gates, each `JBE` to skip. `0.3 x h + 150 > range` uses the first sqrt |
| 5 | aimdive tail, throttle `009C5F1C`-`009C605F` | FAITHFUL | Floor `JBE`, ramp `clamp((h - margin)/120, 0.01, 1)`, `min(v, cap)` |
| 6 | **aimdive tail, air brake `009C5FA7`** | **DEFECT** | The brake floor's x is `AimPitchRatio x |pitch|` (slot `[ESP+24h]`, overwritten at `009C5F79`), not `|pitch|`. Diff 6 |
| 7 | aimdive release `009C608C`-`009C6154` | FAITHFUL; one unlabelled substitution (7a) | Gates `a8 > y`, `timer <= 0`, `|err| < 25`, and pull-out `0.5 x a8 > y` all match |
| 8 | turndown complete `009C7EA0` | FAITHFUL | `pitch < -1.3`, or `pitch < -1.0` with `|bank| > 135 deg` |
| 9 | impact point `009C7D71` (host side only) | not re-read | Out of this packet's time; the aim error and the arms consume it correctly |
| 10 | level-bomb aim tick `009B5C80`-`009B645B` | FAITHFUL | Every compare, clamp, constant and the probe-slot layout matches. 18h/19h logic and step 3/1 match |
| 11 | level-bomb attackrun tail `009B5320`-`009B5585` | FAITHFUL | `min(bc, 2000)`, `max(1400 - y, 50)`, and the deferred fifth interp argument pushed at `009B553A` |
| 12 | level-bomb release enter `009B5B00` | FAITHFUL (first 0x98 bytes read) | `max(lfs, speed)`, then the 15 deg hold folds |
| 13 | takeoff prepare enter `009CDD50` | FAITHFUL; RNG stream substituted, labelled | Three `U()` draws on `ECX=1`, in the host's order; constants match |
| 14 | landing turn-in `006C5E20` | FAITHFUL | StandbyDist `JA`, side `JB`, threshold `InterpolateClamped(PosBehind, 80 deg, 0.6 x Standby, 30 deg, len)` |
| 15 | strafe rule `009CC690`, ready `009CC2F0`, range `009CCED0` | FAITHFUL | The aim-to-goaway arm pushes `EDX = esi+690h` (rebuilt at `009CC75A`); every compare matches |

Not covered (handoff): takeoff/landing beyond 13 and 14 (the `006C7960` mode chooser and `006C6020` path were read in
the host only), aimglide, flyabove, and the level-bomb goaway.

## 1. The aimdive roll arms: no sign error

This was reported to the lead first. Frame base `E` is ESP after the four pushes at `009C58D3`-`009C58EE`.

**Producers.**
- **The approach.** `[E+34h]`/`[E+3Ch]` is `aim - pose+FCh/+104h` (`009C598A`-`009C59AB`). `[E+40h]`/`[E+48h]` is
  `aim - approach+D8h/+E0h` (`009C594E`-`009C596F`). In both, `aim` is `approach->vtable[0]`. Here `[esi+4]` is the
  approach, the same object and slot as the approach update's `+C0h` at `009C7B27`.
- **The bearings.** Each is built as `FLD dz`, `FLD dx`, `CALL 00BF701A` (`_CIatan2`, `atan2(ST1, ST0)`). Then
  `FSUBR qword [00CE3830]` gives `pi/2 - atan2`. Then `FLDZ`, `FCOMPI ST1`, `JBE` adds `2pi` (`00CE3828`) only when
  the value is below 0.
- **The error.** `009C5AA3` and `009C5AF1` both push `[ESP]` = `[E+10h]`, the `009C4F80` heading `H` stored at
  `009C593A`, and `[ESP+4]` = the bearing. `00438B10` is `FLD [esp+4]`, `FSUB [esp+8]`, then a wrap into `[-pi, pi]`,
  so each result is `H - bearing`. The results go to `[E+24h]` (aircraft) and `[E+18h]` (impact point).
  `x87_sym.py 009C5A5E` shows the same first argument at both calls.
- **No stray stores.** Between `009C5AA8` and `009C5D60`, nothing stores to `[E+18h]` or `[E+24h]` at any depth. The
  `SUB ESP,14h` blocks at `009C5BFD` and `009C5C52` write `E+10h`, `E+28h`, `E+5Ch` and argument slots only.

**Band selection.**
- `009C5D0E` is `COMISS [E+5Ch], 0.0` / `JBE` and `009C5D2C` is `COMISS 1.0472, [E+20h]` / `JBE`.
- `[E+20h]` is `|bank|`, folded at `009C5914`-`009C592D` with `-0.0` (`00D7A208`).
- So the wide arm requires `[E+5Ch] > 0` and `|bank| < 60 deg`. That is `used_wide_band` exactly.

**The map.** Both arms push five floats: `[ESP]` = `-0.5` (`00CE69D0`) or `-0.4` (`00D1F400`), `[ESP+4]` = `+1`,
`[ESP+8]` = `+0.5` (`00CE3800`) or `+0.4` (`00CE7804`), `[ESP+0Ch]` = `-1` (`00D7A260`), and `[ESP+10h]` = x.
`00419010` takes `(x0, y0, x1, y1, x)`, so the map falls in both arms. The host passes
`(-band, 1, band, -1, used_wide_band ? bearing_error_wide_18 : bearing_error)` (`src/dive_bomb_task.cpp:1078`). The
host's `bearing_error` is `00438B10(H, db_bearing_c0)` and its `bearing_error_wide_18` is
`00438B10(H, db_impact_bearing_18)`. Their form matches the image. The host refreshes both in the
approach update it binds as `update_dive_bomb_approach_009c7a80`. I did not re-read the image's call order between
`009C7A80` and the state tick, so a one-tick lag at the crossing is not excluded.

**Why the bank still flips.**
- `sign(H - bearing(X -> T))` is the side of the target relative to the line through `X` in direction `H`.
- The wide arm measures from the impact point `I`, the default arm from the aircraft `A`.
- When `I` sits laterally off `A`'s `H`-line, the target can lie between the two parallel lines. That happens when `H`
  is not the ground track, as below -40 deg, where `H` is the body-up bearing.
- Near the along-track zero `T` is close to `I`, so the two arms then disagree in sign. That is the image's geometry,
  not a transcription error.
- The levers are `H` (`009C4F80`, whose mid-pitch arm is GUNNERY 178's `kDiveAimHeadingBlendBound`) and the roll
  response. This agrees with GUNNERY 178's own reading.

**Precision.** The along-track error that picks the arm is a host double chain against the image's x87 (24-bit
precision; docs/CC13_FPU_PRECISION.md). Within about 1 ulp of 0 the two can pick different arms. That decides one tick
at most, not a sustained flip.

## 6. DEFECT: the aimdive air-brake floor reads the scaled stick

Listing, frame base `E`:

```
009C5F66 MOVSS [ESP+24h], xmm0        ; [E+24h] = |pitch command|
009C5F6F FLD   [EBP+54h]              ; AimPitchRatio (row+60h in the host's view)
009C5F72 SUB   ESP, 14h
009C5F75 FMUL  [ESP+38h]              ; E+24h: ratio x |pitch|
009C5F79 FSTP  [ESP+38h]              ; E+24h := ratio x |pitch|   <- overwrites the slot
...      00419010(0, MaxPower, 1, MinPower, E+24h)   ; RET 14h, ESP back to E
009C5FA7 FLD   [ESP+24h]              ; E+24h: ratio x |pitch|, NOT |pitch|
...      00419010(0, MinBrake, 1, MaxBrake, that)
```

`python tools/x87_sym.py 009C5F6C --stop 009C5FD0 --fret 00419010` prints both calls with the same x:
`args=[0.0, [0x14][0x44], 1.0, [0x14][0x48], v9c5f79]` and `args=[0.0, [0x14][0x50], 1.0, [0x14][0x4c], v9c5f79]`.

**Effect.** The host feeds `|pitch|` to the brake floor (`src/dive_bomb_aimdive_tail.cpp:65-66`). With this
installation's SPNormal row (AimPitchRatio 3.0, MinBrake 0.0, MaxBrake 0.5):
- the image's floor is `0.5 x min(1, 3|p|)`, and reaches full brake at `|p| = 1/3`;
- the host's floor is `0.5 x |p|`.

The air brake is `max(floor, 1 - v)`, so the host under-brakes whenever the aircraft is high enough that `1 - v` is
small. `kAimDiveTailBound` is `true` (`src/game_hosts_units.cpp:2205`), so this is live. Expect slower, steeper-held
dives in the image, which moves release heights and the dive geometry.

```diff
--- a/src/dive_bomb_aimdive_tail.cpp
+++ b/src/dive_bomb_aimdive_tail.cpp
@@ -60,10 +60,13 @@
     const float x1 = in.aim_pitch_ratio_54 * a;   // FSTP float at 009C5F79
     const float power_cap = dive_bomb_interpolate_clamped_00419010(
         0.0f, in.max_power_44, 1.0f, in.min_power_48, x1);
-    // 009C5FA7-009C5FCB: the brake floor, min brake at a level stick rising to
-    // max brake at a full one.
+    // 009C5FA7-009C5FCB: the brake floor, min brake at a level stick rising to
+    // max brake once |pitch| x AimPitchRatio reaches 1. 009C5F79's FSTP wrote
+    // x1 over [ESP+24h], the slot that held |pitch|, and 009C5FA7 reloads that
+    // slot, so both interpolations take x1 (cc13-audit5).
     const float brake_floor = dive_bomb_interpolate_clamped_00419010(
-        0.0f, in.min_brake_50, 1.0f, in.max_brake_4c, a);
+        0.0f, in.min_brake_50, 1.0f, in.max_brake_4c, x1);
```

Because the tail is bound ON, cc9 should land this behind a switch or as a pair, following its own convention.

## 7a. Unlabelled substitution: the aimdive re-arm draw

- **The image.** `009C6106`-`009C6119` calls `00BD2F10` with `ECX = 1`, `[ESP] = 0.5` (`00CE3800`) and
  `[ESP+4] = 1.0`, and stores `U(0.5, 1.0)` to state+1Ch after each release.
- **The host.** It passes the low end, `kAimDiveRearmLow` (`src/game_hosts_units.cpp:4591`), with no substitution
  label.
- **When it matters.** Only for a second release within one aimdive, where it gates the spacing.
- **Stream.** `00BD2ED0` indexes the generator table by `ESI + ECX*2` (`009C6114` passes `ECX=1`), so this is not the
  `ECX=0` stream.

UNCERTAIN in effect. A label is the minimum; the full fix is a draw on the per-unit stream behind a switch:

```diff
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -4591 +4591,4 @@
-        in.rearm_draw = bsp::dive_bomb_constant::kAimDiveRearmLow;
+        // SUBSTITUTION, labelled: 009C6114 draws U(0.5, 1.0) (00CE3800, 1.0)
+        // from 00BD2F10 with ECX = 1 after each release; this host takes the
+        // low end and draws nothing (cc13-audit5).
+        in.rearm_draw = bsp::dive_bomb_constant::kAimDiveRearmLow;
```

## Evidence notes for the FAITHFUL rows

- **2. Pitch arm.**
  - `009C5BD4 COMISS pitch, [00CEC728]` (-0.5236) / `009C5BDB JA 009C5CEF`.
  - `009C5C9F FLDZ`, `FLD err`, `FCOMI ST0,ST1`, `JBE`, so `err <= 0` (and NaN) takes the negative arm.
  - Positive arm: `FLD1`, `FCOMPI ST1`, `JBE` gives `min(err x row+64h, 1)`. Negative arm: `FLD -1`, `FLD d`,
    `FCOMPI`, `JBE` gives `max(d, -1)`. The host matches, NaN included.
- **3. Aim error.**
  - `[E+1Ch] = cos([E+18h])` (`009C5BE5`), and `along = [E+1Ch] x [E+5Ch]` (`009C5C04`, at ESP `E-14h`).
  - Lead: `00419010(a8 + qword[00D7A220], 0, ac + 50h, row+5Ch, [E+14h])`. `009C5C4E FSUBR [E+28h]` is `along - lead`.
  - Gain: `00419010(.., 1, .., row+60h, ..)`, then `009C5C97 FMUL`.
  - `[E+14h]` is `pose+100h - aim.y`, from `009C59D2 FSUBR qword`.
  - The `00438AA0(H, pi)` at `009C5BB1` is dead (popped at `009C5BBA`), as AIMDIVE_ENTRY.md already records.
- **4. Dive abort.**
  - `009C5B0E FLD (D4h+50h)`, `FCOMPI ST1` (height), `JBE`.
  - `009C5B25 COMISS pitch, -60 deg` (`00D20338`), `JBE`.
  - `009C5B2E FLD ST1` (height), `FMUL 0.3` (qword `00CE3DC8`), `FADD 150` (`00CE3DD8`), `FCOMPI ST1` against
    `[E+1Ch]`, the aircraft range. The host's `aim_point_distance = db_planar_bc` matches.
- **5. Throttle.**
  - The floor test is `009C5F20 FLD a8`, `FCOMPI ST1` (`[E+14h]`), `JBE`. Below it: throttle `0.05` (`00CE7638`),
    brake `1.0`.
  - `v`: `(h - margin) / 120.0` (qword `00D1F3F8`), then `0.01` (`00D7A238`) by `FCOMPI`/`JBE`, then 1.0 by
    `COMISS`/`JBE`.
  - Throttle `min(v, cap)` by `FCOMI`/`JBE`; brake `max(floor, 1 - v)` by `FCOMPI`/`JA`.
  - The margin's `FSUBR qword pi` at `009C5EBC` and `FSUBRP` at `009C5EFD` (`1 - cos`) match the host.
  - The yaw gain is `8.0` (qword `00CE3DB0`) and the margin base `50.0` (qword `00CE3938`).
- **7. Release.**
  - `009C60AF FLD a8`, `FCOMPI` (y), `JBE`.
  - `009C60BB COMISS 0, [esi+1Ch]`, `JB`.
  - `|[E+5Ch]|` against `25.0` (qword `00CE3880`) by `FCOMPI`/`JBE`.
  - Pull-out: `a8 x 0.5` (qword `00D7A280`), `FSTP` to float, `FCOMPI` (y), `JBE`.
  - The host's planar-distance substitute for `[E+5Ch]` on the steep-gate path (`src/game_hosts_units.cpp`, the
    `009C60C1` comment) is correct: the gate jumps past `009C5C9B`.
- **8. Turndown complete.**
  - `COMISS -1.3 (00D1F98C), pitch` / `JA` returns true. `COMISS -1.0, pitch` / `JBE` returns false.
  - Otherwise `COMISS |bank|, 2.3562 (00D20E80)` / `JA` returns true.
  - The host's double negative is equivalent. They differ only on NaN pitch.
- **10. Level-bomb aim.** Frame `F` is ESP after `push esi`, and `G = F - 4` after `push edi`.
  - `bearing_error = 00438B10(c0, heading)` (`009B5CB9`).
  - The impact bearing is `pi/2 - atan2(aim.z - D8h, aim.x - D0h)`, then `00438B10(b, heading)`.
  - `ratio = dist / ([approach+8]+268h)`.
  - Close gate: `1.4 > ratio` (`00D06874`); limit `00419010(0.1, 0.5236, 2.0, 2.618, ratio)`; `|be| > lim`; `CEh`;
    the `3AEh` arm sets 19h, or clears 19h and sets 18h on `|be| > 1.5`.
  - Probe arguments `(unit, &G+54h, &G+3Ch, &G+60h, &G+48h, 1)`: out_a is at `G+3Ch` and out_b at `G+60h`.
  - The trigger is `|a0| > 0.05` (qword). `s = -a0 x b1 x b2`; `t = -a2 x 3.0` clamped to ±1. The `±0.5` fold is
    `(s ∓ 0.5) x 2`.
  - `w = 00419010(150, 0.15, 350, 1, dist)`; `nudge = w x float(s x 0.2618)`; `alt = clamp(a1 x 600, -80, 80)`.
  - `e = 00438B10(00438AA0(c0, nudge), heading)` is clamped by `00419010(0.3, 5 deg, 1.5, 80 deg, ratio)`; the
    heading is `00438AA0(heading, ec)` with mode 2. The bank limit is `00419010(0.1, 0.2, 0.8, 1.2, ratio)`.
  - Altitude: `dy = y - (aim.y + B0h + alt)` in x87, folded; level below 240 (qword).
  - `009FBA50(aim.y + B0h + alt, 200, dist, 00419010(0.05, 0.1, 0.35, 1, |dy| / (dist - 299)))`.
  - Speed `k x span x vert + ACh`. `close = 650 > dist`.
  - 18h/19h: `!18h && !(300 > dist)` leads to abort `|ibe| > 120 deg && 1 > ratio && !3AEh`; otherwise release on
    `|ibe| > 85 deg`.
  - Step is 3 unless `18h` or `0.5 x B4h > dist`, in which case 1 (`009B6458` stores `EDX = 1`).
  - Precision only: the image keeps `aim.y + B0h + alt` and `B4h x 0.5` in x87. The host rounds the first to float.
- **11. Level-bomb attackrun tail.**
  - `FLD qword 2000.0 (00CF0DD8)`, `FCOMPI`, `JBE` gives `min(BCh, 2000)`.
  - `1400 - y`, floored at 50 by `FCOMPI`/`JBE`. `00419010(0.1, 0.4, 0.35, 1, h/dist)`.
  - `009FBA50(aim.y + B0h, B4h, BCh, scale)`.
  - The speed interp's fifth argument (`[ebp+24h]`) is pushed at `009B553A` before `007C47F0`, which takes no
    stack argument, and the `SUB ESP,10h` completes the block.
  - `top = 188h x 1.05` (qword `00CF2510`); `x1 = 1.05f` (`00CE780C`).
- **13. Takeoff prepare enter.**
  - `U(0, 2pi)` (`00CE3D9C`), then `T` floored at 1.5 (qword `00CE3D78`, `JBE`), `p = T / 2pi`.
  - `U(0, 1)` compared with 0.5 by `FCOMPI`/`JBE` gives sign `+1`; above 0.5 it is `-1`.
  - `U(p x 0.6, 0.95 x p)` (qwords `00CEFF98`, `00CEFFB0`), times the sign.
  - All three draws are on `ECX = 1`. The host's per-unit named streams are a labelled substitution in its own
    comment.
- **14. Landing turn-in.**
  - `|T - plane|` (planar, `00414C60`) against tuning+500h (StandbyDist): `FXCH`, `FCOMPI`, `JA` returns false.
  - Side: `COMISS local.x, 0` / `JB` gives side 0.
  - `delta = |00438B10(00414EB0(v), heading)|`.
  - `00419010(tuning+4F0h PosBehind, 1.396, StandbyDist x 0.6, 0.5236, |v|)`, then `FCOMPI`/`JBE` against `delta`.
  - The host matches (offsets checked in `game_tuning_singleton.hpp:362/366`).
- **15. Strafe.**
  - Rule `009CC690`:
    - the engaged test is `448h || (mode 370h == 2 && (44Ch || 468h))`;
    - `mode == 0` gives prepare (`5B4h`);
    - prepare `5B4h` takes the engaged entry `009CC5F0`;
    - attackrun `6D8h` goes to gotowards `64Ch` on `448h`;
    - goaway `690h` goes to `64Ch` on `6B4h`;
    - gotowards `64Ch` goes to aim `670h` on `009CC2F0`;
    - aim `670h` goes to goaway `690h` on `68Dh || 68Ch`. The `MOV DL,[esi+448h]` at `009CC6D0` clobbers EDX's low
      byte, but `009CC75A` reloads EDX before `009CC7C6 PUSH EDX`;
    - not engaged gives moveto or follow.
  - Ready: `FLD 188h`, `FADD ST0`, `FADD approach+38h`, `FCOMPI` (`+18h`), `JBE` false.
  - Range `009CCED0`:
    - the countdown uses `FCOMI dt, cd` / `JB`;
    - the reload is `cd + (CCh - dt)`;
    - the threshold is `(y - aim.y) / 30h + 2 x 188h`, floored by `FCOMPI`/`JA` at `658h x (1.0 | 1.4)`;
    - in range when `threshold > 009CAD00()` (`FCOMPI`/`JBE`).

## Handoff

- **Open.** Takeoff/landing past `009CDD50` and `006C5E20`:
  - `006C7960` mode chooser;
  - `006C6020` path;
  - `006C3B10` corridor;
  - `006C5C40` on-circle.

  Also aimglide (`009C5180`-`009C580B`), flyabove (`009C6270` onward), the level-bomb goaway (`009B5845`) and the
  release enter beyond `009B5B94`.
- **For cc9.** Apply diff 6 (live, behaviour-changing) behind a switch or as a pair. Apply the 7a label. Do not bind
  a roll-arm sign change at `009C5D33`/`009C5D60`.
