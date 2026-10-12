# Audit of the plane bot states, part 2 (cc13-audit7)

Addresses: 009C7A80 009C8790 009C87EA 009C884C 009C83E0 009C8920 009998A0 0099AF1C 009C58D0 00D20CE8 009C73A0
006C3B10 006C5C40 006C7960 006C6020 006C5E20 006CCAF4 006CCC0C 009C5180 009C53D0 009C5420 009C547E 009C57D3 009C5E63
0099E756 0099B52C 009C62B0 009C64A8 009C683A 009C6E08 009C6E10 009C6F33 009B5760 009B5845 009B5B00 00BD2F10

Packet `cc13-audit7`, branch `agent/cc13-audit7`, 2026-10-12. Requested by cc9 through the cc13 lead. It continues
`docs/CC13_AUDIT_PLANE_BOT_STATES.md` (cc13-audit5) and does not repeat that audit's items. **Read only:** no Ghidra
writes, no ledger records, no change to cc9's files. Findings that need a change are given as one unified diff
(section 9) for cc9 to apply. Base: `origin/main` at `fa36636cd`.

**Method.** Same as audit5:
- Each host routine was compared with the disk listing (`bsp.py disasm-raw`). ESP was tracked by hand from each frame
  base, and every read of a reused slot was traced back to its last store.
- `tools/x87_sym.py` confirmed the two slot readings that decide a finding: the aimglide pull-out (009C5180) and the
  flyabove descent reference (009C6E10).
- Constants were read from the PE at the load's own width (`local/au7_const.py`).
- Call censuses are byte scans for E8/E9 rel32 and absolute dwords (`local/au7_xref.py`), not `ghidra callers`.
- Displacement censuses use `local/au7_disp.py`, run over 009B0000-009D8000.

**Checklist.**
- Rule 2 (boundary): `ghidra proto --brief` places each cited site in its function:
  - 009B5845 is in 009B5760 (009B5760-009B599F);
  - 009B5B94 is in 009B5B00 (-009B5BD7);
  - 0099B52C is in 0099B450 BSP_PilotBot_SeedPlanSlots;
  - 0099DE93 is in 0099D300;
  - 009C5E65 is in 009C58D0;
  - 009C69F3 is in 009C62B0;
  - 009BFA02 is in 009BEE30.
- Rule 7 (argument counts):
  - 006C7960 ends in `RET 10h`; 006C6020, 006C3B10, 006C5C40 and 006C3E50 in `RET 4`; 006C5380 in `RET 0Ch`;
  - 00412E20 is `RET 4`, 00427C90 `RET 8` and 006BCA40 `RET 4`;
  - 00419010 is `RET 14h` and 00438B10/00438AA0 are `RET 8`.
- Rule 8 (register provenance): the listing of 009C58D0 was filtered for EBX. Its only write is `009C58DE XOR EBX,EBX`.
  In 009C5180, EBX = 1 and EBP = 2 come from `009C53DD`/`009C53E2`, both on the straight-line path before
  `009C53E5`. Neither register is written again before `009C579E`.
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table, so there is nothing for it
  to check.

**Status vocabulary.** The audited host code is reconstructed and build-tested on main. The diff in section 9 is
**build-tested** in this worktree (`scripts/build.ps1`). It is **not run**: no pair, no game validation. Every
behaviour change below needs a same-binary pair before it lands. Rule 6 applies: `bsp_game.exe` reaches all of these
paths.

## Verdicts

| # | item | verdict | one line |
| --- | --- | --- | --- |
| 1 | approach update vs aimdive tick, per think | **FAITHFUL** (no lag) | `009C87EA` 009C7A80 runs before `009C884C` state->tick on every think, unconditionally; the host keeps the order |
| 2 | landing corridor `006C3B10` | FAITHFUL | Every compare, constant and operand order matches |
| 3 | on-circle `006C5C40` | FAITHFUL | `dd > 1.6r` false; `InterpolateClamped(0.25r, 45 deg, 0.8r, 20 deg, dd) > |a - h|` |
| 4 | mode chooser `006C7960` | FAITHFUL (two notes) | Branch order and results match. squadron+3B0h is labelled; the follower's head is `[sq+3D0h]`, which the host reads as `members[0]` |
| 5 | landing path `006C6020` | FAITHFUL | Side, arc, leg and `max(D, path)` match. The `+904h` -> `-1` arm is unreachable from its one caller |
| 6 | aimglide bearings, steer split, command heading | FAITHFUL | `009C5357` takes atan2 of `aim - pos`, the same vector as approach+C0h in the same think |
| 7 | **aimglide yaw arm `009C5420`, aimdive tail `009C5E63`: plan+2D4h = 0** | **DEFECT** | Neither host arm sets `gl_yaw_mode_2d4_zero`, so the planner's yaw arm `0099E81A` overwrites the state's 284h. Diff B |
| 8 | **aimglide command arm `009C547E`: plan+2E8h** | **DEFECT** | The bank-limit scale `InterpolateClamped(100, 1, 250, 1.5, miss)` is dropped. The host's 2E8h is never written. Diff C |
| 9 | aimglide pitch, throttle, release | FAITHFUL; 9a: an unlabelled pin | The rearm draw `U(0.2, 0.5)` on `ECX = 1` is pinned to 0.2 with no label at the call site |
| 10 | **aimglide pull-out arm B `009C57D3`-`009C57FD`** | **DEFECT** | The three "unwalked" slots are ratio, p1 and p2. Latch on `0.6 > ratio && pitch < 0 && p2 > p1`. Diff A |
| 11 | **flyabove C `009C6491`-`009C64C9`** | **DEFECT** | The `ACh + 50h` fall-back is dead because 009C7A80 dereferences approach+0Ch on every think. C is ctl+398h = `db_begin_alt_ac`. Diff D |
| 12 | **flyabove descent reference `009C6F33`** | **DEFECT** | `2 x err / max(A, 1)`, not `2 x A / max(A, 1)`. The host always commands the 0.8 cap. Diff E |
| 13 | flyabove plan+2E8h `009C6E08` / `009C683A` | **DEFECT** (6E08); UNCERTAIN (683A) | 1.5 when flyabove+18h is clear (Diff C2). The 1.4 store on the 009C67BF roll-in path is not wired |
| 14 | level-bomb goaway `009B5760`-`009B599F` | FAITHFUL | Fly-to arguments, probe, clamp to ±10 deg, opposing-sign nudge x 0.1396, `H + e`, throttle/brake, `009FB800(aim.y + B0h, 1)` |
| 15 | level-bomb release enter tail `009B5B94`-`009B5BD7` | FAITHFUL | `+19h = |bank| > 15 deg`, `+1Ah = |pitch| > 15 deg`, strict |

## 1. The approach update runs before the aimdive tick, on every think

This was reported to the lead first. In the image:
- **The think gate.** `0099AEF2`-`0099AF1C` subtracts dt from bot+74h. `FLDZ / FCOMPI / JB` at `0099AF10` skips the
  think while the result is above 0. Otherwise `0099AF1C` calls `009998A0 BSP_PilotBot_Update`.
- **009998A0.** Both of its paths call `task->vtable[64h](dt)`: `009998FB` when `0099C270` is true, and `0099995A`
  otherwise, after the `bot+308h` timer fold. The only skip is the gate at `009998A4`-`009998C6`
  (`bot+270h == 2` with a flagged target), which bypasses both calls together.
- **The dive-bomb vtable[64h], `009C8790`.** There is no branch before the call:
  1. `009C87EA` CALL `009C7A80` (ECX = task+3F8h, dt, diving);
  2. `009C8825` `009BDE80` (the moveto ranges);
  3. `009C8834` `009C83E0` (the transition);
  4. `009C884C` `[task+310h]->vtable[0Ch](dt)`.
- **The aimdive state is task+734h.** `009C7547` stores vtable `00D20CE8` at ECX+33Ch inside `009C73A0`, which
  `009C7751` calls with `ECX = task+3F8h`. `00D20CE8+0Ch` is `009C58D0`. The byte scan finds exactly one reference
  to `009C58D0`, the dword at `00D20CF4`, and exactly two calls to `009C7A80`, at `009C87EA` and `009C8A7C`.
- **009C8A7C** is in `009C8920` (task vtable[54h]). It runs `009C7A80(dt = 0, diving = false)` (`FLDZ`, `PUSH 0` at
  `009C8A56`/`009C8A58`) when `0099AEB3` notifies a new front task. This has no aimdive effect.
- **The early return.** 009C7A80 returns early only on `approach+48h == 0`, the no-target case. C0h and D8h-E0h are
  then stale in both image and host.

The image's aimdive tick re-derives its bearings from the live pose and from approach+D8h/E0h, which 009C7A80 has just
written. Nothing between `009C87EA` and `009C884C` moves the pose. The host's cached `db_bearing_c0` /
`db_impact_bearing_18` therefore equal what the image computes in the tick, up to the float store of C0h (the x87
precision note in audit5 section 1 still applies).

**The host.** `game_hosts_units.cpp` calls `dive_bomb_task_arm_009c8790` (the approach update through the binding,
then the transition) and then `run_dive_bomb_aimdive_tick_009c58d0()` on `ctx.current`, after the transition. That is
the image's order. There is no one-tick lag at the along-track zero.

## 2. Landing

**Corridor `006C3B10`.** Frame `F` is ESP after `PUSH EDI` at `006C3BAE`. Locals: `l` is at F+20h, `zp` overwrites
`l[2]`, and `e` overwrites `l[0]`.
- `zp = l2 - 1.5L` (qword `00CE3D78`).
- `l1 > 5.0 - 2 tan(a) zp` returns false. The `FCOMPI`/`JA` is at `006C3C15`, `00412E20` rounds tan to float, and
  `00D7A370` is the qword 5.0.
- `e`: if `l0 > 0` (`COMISS`/`JBE`), `max(l0 - w, 0)`; otherwise `min(l0 + w, 0)`, with `w = B0h x 0.5`.
- `over = |(e, l1, zp)| - 1.5L` must not exceed `PosBehind x 1.1` (`JA` at `006C3CB6`).
- `b = 00414EB0(T - pos)`. `006BCA40` writes T to F+38h and `00427C90` subtracts the position into F+2Ch. When
  `|e| < 1` (`00D7A24C`, `JBE`) and `zp > -2L`, b is the runway heading instead.
- Then `|h - rh| < pi/2` (qword `00CE3830`, `JBE`), and `InterpolateClamped(0, 30 deg (00CEC724), PB x 1.25, pi/2
  (00CE3C64), over) > |h - b|`.

**On-circle `006C5C40`.** Frame `E0` is ESP after `PUSH ESI`. The side byte reuses the argument slot E0+28h.
- `dd` is at E0+8, and its double copy at E0+14h overwrites `vx` (the copy at E0+18h survives).
- `dd > r x 1.6` (qword `00CE3D48`) returns false.
- `off = side ? +pi/2 : -pi/2`.
- `s = 00438B10(heading(v) - off, h)`, and `|s|` is folded with -0.0.
- The five 00419010 arguments are `(r x 0.25, 0.7854, r x 0.8, 0.3491, dd)`, pushed through `[ESP+38h]`/`[ESP+3Ch]`.
  The result is true iff `thr > |s|`.

**Mode chooser `006C7960`.**
- The `+904h` landed arm writes mode 4, -1 and 1. `!airborne || held` writes `leader ? 2 : 1`, 999999 and 1.
- `squadron+3B0h` forces both the leader flag and the "skip on-circle" byte BL. The host's comment says the field is
  only ever cleared; that is not re-read here.
- **Leader pass.** If the corridor holds, the result is 4 when `006BED60 && rec+8 > 0` and 3 otherwise. If not,
  on-circle gives 3; otherwise `NEG AL / SBB / ADD 2` gives `outside ? 1 : 2`.
- **Follower pass**, on headMode:
  - 2: 1 if outside standby, else `006C5E20([sq+3D0h]) ? 2 : 1`;
  - 1: 1;
  - 3: 3;
  - else: `006BED60 && corridor && rec+8 > 0 ? 4 : 3`, with 006BED60 called first.
- The floor is `max(path, headDist + 1)` (qword `00D7A210`) for followers only, where "follower" means the possibly
  forced leader byte.
- The call sites `006CCAF4` push `(rec, 1, 4, -1.0)` and `006CCC0C` push `(rec, 0, headMode, headDist)`, the host's
  two calls. The host passes `members[0]` for `[sq+3D0h]`. That is the same plane while the array is kept compact.
  Only a null `[sq+3D0h]` could differ, and the image would then pass null into `006C5E20`.

**Path `006C6020`.** Frame `E` is ESP after `PUSH EDI`. The argument slot E+54h is reused for `dp`, `off`, `b` and
then `bp`.
- The side for modes 3/4 is `l0 >= 0` (`COMISS`/`JB`). For other modes it is the sum of `l0` over the airborne
  `[sq+3D0h..+3E0h]`, then `> 0`.
- `D = |pos - T|`, and mode 4 returns D.
- `m1 = max(1, dP)` and `h1 = 00438AA0(rh, side ? -pi/2 : +pi/2)`.
- For side 1: `bp = b + acos(min(1, r/m1))` and `theta = h1 - bp`. For side 0: `bp = b - acos(...)` and
  `theta = bp - h1`. Every argument slot was checked.
- `theta < -5 deg` (`00CF885C`) without the corridor adds 2pi.
- `path = PB + min(r, m1) x theta`, plus `|pos - (c + r x dir(bp))|` when `m1 > r`. The result is `max(D, path)`.
- The `+904h` arm (`-1`) is unreachable from `006C7AA6`, the only caller (byte scan), because 006C7960 returns on
  `+904h` first.

## 3. Aimglide `009C5180`

Frame `E` is ESP after the four pushes; the argument dt is at E+6Ch.

**Slots.**
- `[E+38h]`/`[E+40h]` is `aim - pos`.
- `[E+44h]`/`[E+4Ch]` is `impact - pos`.
- `[E+50h]`/`[E+58h]` is `aim - impact`.
- `[E+20h]` is `pos.y - aim.y` (`009C527D FSUBR`).
- `[E+10h]`, `[E+14h]` and `[E+1Ch]` are the planar lengths: range, throw and miss.
- `[E+24h]` is range / throw (`009C5353`).
- `[E+28h]` is H = `009C4F80`. It is overwritten by the atan at `009C54F7` and finally by **p2** at `009C55B0`.
- `[E+1Ch]` is overwritten by the ceiling `C = row'+40h x approach+A8h` at `009C5493`, after the steer split has read
  it as the miss.
- `[E+6Ch]` holds the bearing (`009C5372`), then the glide angle, then **p1** (`009C5563`).

**6, FAITHFUL.**
- `009C535F` is `atan2([E+40h], [E+38h])`, the `aim - pos` vector, which is approach+C0h's construction. The host's
  `db_bearing_c0` substitution is the same value in the same think.
- The error is `00438B10(b, H)` (`009C539C`), then `|e|` into `[E+18h]`.
- The split is `COMISS 140 (00D04A24), miss` / `JBE`, so the yaw arm runs on `140 > miss`.
- The yaw arm is `e x 8.0`.

**7, DEFECT.** The yaw arm writes `+284h`, `+288h = 1`, `+2C4h = 0`, `+2CCh = 1`, **`+2D4h = 0` (`009C5420`)**, and
`approach+CCh = 3` (the weapon selector, a no-op binding in the host).
- `0099E756` reads plan+2D4h. When it is zero, the planner runs only the `+2ECh` timer branch
  (`0099E764`-`0099E80D`, no store to +284h) and jumps to `0099EA57`, past the yaw arm `0099E81A` that writes +284h
  at `0099EA46`.
- `0099B450` reseeds +2D4h every think (host comment at `gl_yaw_mode_2d4_zero`).
- The host models this through `gl_yaw_mode_2d4_zero`, which `plan_yaw_0099d300()` consumes. Seven other states set
  it; the aimglide yaw arm and the aimdive tail do not.
- The aimdive's `009C5E63 MOV [EAX+2D4h],EBX` (EBX = 0 from `009C58DE`, its only write) has the same comment in the
  host and no store.
- In both states the host's planner therefore replaces the state's yaw command each think
  (`kRateLawAttitudeTermsBound` is true, so the arm after the pitch arm runs).
- Static finding: the run that would show it needs a counter at the overwrite, which is cc9's file.

**8, DEFECT.** The command arm (`miss >= 140`) writes `+2C0h = b`, `+2CCh = 2`, and
`+2E8h = 00419010(100 (00CE3D08), 1.0, 250 (00CE77B0), 1.5 (00CE380C), miss)` (`009C5444`-`009C547E`).
- plan+2E8h is the bank-limit multiplier (`plane_ai_control.cpp`: `max_bank = turn_scale_2e8 x turn_roll`), reseeded
  to 1.0 at `0099B52C`.
- No host code writes `plan_state.turn_scale_2e8`, so the glide banks at 1.0x where the image allows 1.12x-1.5x.
- **Census** (`au7_disp.py 2E8 009B0000 009D8000`). The writers are:
  - `009C5480` aimglide;
  - `009C683E` and `009C6E0C` flyabove;
  - `009BFA02` follow (`009BEE30`, from `[edx+38h]`, outside this audit);
  - `009D1D06` torpedo aim (1.4; `torpedo_aim_tick.cpp` carries it as `roll_limit_2e8`, but nothing copies it into
    the plan);
  - `009B776C` and `009B7926` (`MOV [ESI+2E8h],EBX`), not classified.

**9, FAITHFUL**, with note 9a.
- **Pitch.** `d = max(|range - throw|, 1)` and `a = atan((C - h) / d)`, floored by `JBE` at the climb angle.
  `p1 = 00419010(0.75, -1, 1.15, glide, ratio)` and `p2 = 00419010(0.7C, 0, 1.1C, -1, h)`. The result is
  `max(p1, p2)` with mode 2.
- **Throttle.** `t = 00419010(0.8, row'+48h, 1.3, row'+44h, ratio)`. Throttle is `clamp(t)` and the brake is
  `clamp(-0 - t)`, with `+27Ch = +2ACh = 1` and `+2D8h = 0`. `row' = approach+14h` is row+0Ch, so the host's
  `*_050/_054/_04c` names match.
- **Release.** The host's six-gate chain (`dive_bomb_aimglide_release_009c5777`) matches the listing, including the
  ceiling read of the overwritten `[E+1Ch]` and the window `-5 - 4w < lead < -5`. The salvo is capped by EBP = 2, and
  the travel grows by `+= A4h x 0.35`.
- **9a.** `009C579E` draws `00BD2F10(0.2, 0.5)` with `ECX = EBX = 1`, which is stream 1. The host passes
  `kAimGlideRearmLow` (0.2) with no label at `game_hosts_units.cpp` (`in.rearm_draw = ...`). That is the same class
  as audit5's 7a: label it, and if it is ever drawn, use a per-unit stream, not stream 0.

**10, DEFECT.** The header calls `[ESP+24h]`, `[ESP+28h]` and `[ESP+6Ch]` unwalked. They are the ratio, p2 and p1.
`x87_sym.py 009C5180 --take 009C53E5,009C568D` prints:

```
009C57E0 jbe 009C5802  [comiss 0.600000024 ? v9c5353]           (ratio)
009C57EF jbe 009C5802  [comiss 0.0 ? this[0x4][0x4][0xc64]]     (unit pitch)
009C57FD jbe 009C5802  [fcmp ST0=ret_9c55ab ? ret_9c555e]       (p2 ? p1)
009C57FF STORE this[0x18] = bl
```

- **The rule.** Arm B latches when `0.6 > ratio`, `pitch < 0` and `p2 > p1`: the impact point is well past the aim
  point while the height arm, not the ratio arm, sets the pitch.
- **The host.** It runs arm A only, so a low, overshooting glide keeps gliding in the host and pulls out to goaway in
  the image. The header's argument for under-approximating no longer holds now that all three slots are known.
- **Order.** In the image the latch is the tick's last act. The host samples arm A at the top of its tick and arm B
  after its pitch block. Nothing in the tick reads `+18h`, and `009C86B2` reads it at the next think's transition.

## 4. Flyabove `009C62B0`

**11, DEFECT.**
- `009C648E` loads `EAX = [approach+0Ch]`. `009C64A6 JZ` takes the fall-back `ACh + 50h` only when it is null. When
  it is not null, `009C64A8` loads `ctl+398h`.
- The host passes `has_control_block_0c = false` ("nothing fills it"). But 009C7A80 executes
  `009C7AA4 MOV EAX,[ESI+0Ch]` / `009C7AA7 FLD [EAX+398h]` with no test, on every think, before this tick (section 1).
  approach+0Ch is therefore never null on this path, and the fall-back is dead.
- 009C7A80 also copies ctl+398h into approach+ACh that think, and the host already keeps it as `db_begin_alt_ac`
  (`game_hosts_units.cpp` also passes it as `cruise_altitude_398` at two other sites). So `C = db_begin_alt_ac`.
- **Effect.** C, the target `min(ACh + 50h, C)`, the 0.15C dead band, the `1.1R` clamp and the bank predicate's C
  (`009C650E`) all move by approach+50h.

**12, DEFECT.** For `err > band`, the x87 stack is:
- `009C6F0D FLD err`, then `009C6F11 FCOMI` / `009C6F13 FSTP ST1`: ST0 = err.
- `009C6F17 FLD A`, `FLD1`, then `009C6F1D FCOMIP` pops 1 and `009C6F1F FSTP ST0` pops A: ST0 = err.
- `009C6F33 FADD ST0,ST0` / `009C6F3B FDIV [max(A,1)]`.

`x87_sym.py 009C6E10 --take 009C6EAF` prints `v9c6f3f := (v9c6e44 + v9c6e44) / 1.0`, where
`v9c6e44 := B - min(ACh, C)`, the height error. The reference is `min(2 x err / max(A, 1), 0.8)`.
- The host doubles A, so the quotient is 2 and the reference is always the 0.8 cap.
- The image starts the descent gently and reaches the cap only at `err >= 0.4 x max(A, 1)`.
- The header's paragraph ("the reference is 0.8 at every geometry") is the same misreading. The diff corrects it.

**13.** `009C6DF5 CMP [ESI+18h],0` / `JNZ`, then `009C6E00`-`009C6E08` writes plan+2E8h = 1.5 (`00CE380C`). This
runs after the heading arm whether or not the heading was written, and is dropped in the host (diff C2).
- `009C683A` writes plan+2E8h = 1.4 (`00D06874`) on the path `009C67BF` (BL set) -> `009C67D5` (+1Ah set) ->
  `009C680A`. The same path writes +18h/+19h/+1Ah.
- I did not read how the host models that path, so its 1.4 is left as UNCERTAIN (wiring).
- When +18h ends up clear, `009C6E08` overrides the 1.4 with 1.5 in the same tick anyway.

## 5. Level-bomb goaway and release enter

**14, FAITHFUL.** Frame `E` is ESP after `PUSH EDI`.
- **Fly-to.** `009FD570(aim, unit, state+18h, &state+1Ch, approach+BCh, 0.6)`; the copy of BCh at E+0Ch feeds the
  range.
- **Probe.** `007F0280(unit, ext (100, 200, 120), out_a E+34h, out_b E+28h, w 0, 1)`, then
  `probe = -out_a[0] x out_b[1] x out_b[2]`, which overwrites E+0Ch.
- **Turn.** `e = 00438B10(fly-to, H)`; `300 > BCh` zeroes it, otherwise it is clamped to ±0.1745 (`JA`/`JA`).
- **Nudge.** `e += probe x 0.1396` (qword `00D20138`) when `e >= 0 && probe < 0` or `e <= 0 && probe > 0`. The
  host's predicate covers the `e == 0` path, which enters at `009B58A1`.
- **Heading.** `00438AA0(H, e)` with a fresh `vtable[50h]`. Then throttle 1, brake 0, `+2D8h = 0`.
- **Altitude.** `009FB800(aim.y + B0h, 1.0)`. Heading mode 2 and the cone `+674h`.
- **Unmodelled.** `009FABE0(heading, 0099B630())` on approach+1Ch. The host records it as a contract only
  (`record("BotStateMoveTo::direction_009fabe0")`).

**15, FAITHFUL.** `|bank|` and `|pitch|` are each folded with -0.0 (`COMISS`/`JBE` and `JA`) and compared strictly
with 15 deg (`00D05AA8`) into `+19h` and `+1Ah`. The host's `abs_wrapped` is the same fold.

## 6. Asides (outside this packet, not verified further)

- `src/torpedo_approach_update.cpp:589` writes `cmd.roll_limit_2e8 = 0.0f`, but its comment cites `[00D06874]`, which
  is 1.4f. `torpedo_aim_tick.cpp` uses 1.4. One of the two is wrong, or the file is superseded.
- Follow `009BFA00` writes plan+2E8h from `[edx+38h]`. With no host writer of `turn_scale_2e8`, every state that
  scales the bank limit currently flies at 1.0.

## 7. For cc9

- **Rejected hypothesis.** Do not chase a one-tick lag at the aimdive along-track zero; the order is the image's.
- **Defects behind a switch.** Diffs A, C, C2, D and E are behaviour-changing. Land each behind a switch or as a
  same-binary pair: dive-bomb missions, USN04 and JM08.
- **Diff B.** It touches every dive-bomb yaw command, the aimdive's included. Take its own pair.
- **The note.** Label 9a.

## 8. Handoff

- **Covered.** Item 1, landing (`006C7960`, `006C6020`, `006C3B10`, `006C5C40`), aimglide, the flyabove altitude arm
  and its heading-arm tail, the level-bomb goaway, and the release enter tail.
- **Not covered.** The rest of the flyabove body:
  - `009C62B0`-`009C6470`;
  - the bank/roll-in path `009C6532`-`009C6A37` and the 1.4 store;
  - `009C6A37`-`009C6DC8`, the dead band and slew, which the host binds.
- **Also open.** Other plan+2E8h writers (follow, torpedo) and the two unclassified `009B776C`/`009B7926` stores.

## 9. The diff

Build-tested on `agent/cc13-audit7` with the patch applied, then reverted; cc9's files are unchanged on this branch.

```diff
diff --git a/include/bsp/dive_bomb_task.hpp b/include/bsp/dive_bomb_task.hpp
index 0c48fb9ef..56a8b54f4 100644
--- a/include/bsp/dive_bomb_task.hpp
+++ b/include/bsp/dive_bomb_task.hpp
@@ -201,6 +201,8 @@ inline constexpr float kGlideDiveAngleLimit = 0.5235987901687622f;  // 00CEC724,
 // release gate above, and it is a latch rather than a per-tick gate. Packet
 // cc8_dive_flyover.
 inline constexpr float kGlidePullOutBearing = 1.5f;                 // 00CE380C, 85.9 deg
+// 009C57D3 MOVSS, the pull-out's arm B ratio bound (cc13-audit7).
+inline constexpr float kGlidePullOutRatio = 0.6000000238418579f;    // 00CE3D30
 // 009C53DD `MOV EBP,2` / 009C53E2 `LEA EBX,[EBP-1]`, both on the straight-line
 // path into 009C53E5's branch, so both arms carry EBP=2 and EBX=1 to the salvo:
 // 009C5762 caps the loop at two rounds and 009C5782 steps it by one. This is a
@@ -877,20 +879,21 @@ inline constexpr bool kOldStyleBombing1b = false;
 //                                                           009C6EAF `76` JBE
 //   err <= band  : NO 009FB800 - cmd+2BCh = 0.0 with cmd+2D0h = 2, a dead band
 //                  that holds level flight                  009C6F15 `76` JBE
-//   otherwise    : 009FB800(target, min(2 * A / max(A, 1), 0.8))
+//   otherwise    : 009FB800(target, min(2 * err / max(A, 1), 0.8))
 //
 // The two references are the second argument 009FB800 caps its pitch demand
-// with, the same slot 009FBA50 fills for the run-in. On the dive arm A is the
-// planar range in metres and is never under 1, so `2 * A / A` is exactly 2 and
-// the cap at 00CE74F8 takes it: the reference is 0.8 at every geometry this
-// mission produces. That is transcribed rather than folded, because the divisor
-// is only pinned to A by the `1.0 <= A` test at 009C6F1D.
+// with, the same slot 009FBA50 fills for the run-in. CORRECTED (cc13-audit7):
+// the descent numerator is the height error, not A. 009C6F13 FSTP ST1 leaves
+// err in ST0, 009C6F17 FLD A is popped by 009C6F1D FCOMIP and 009C6F1F FSTP
+// ST0, so 009C6F33 FADD ST0,ST0 doubles err. The reference therefore grows with
+// the height error and reaches the 0.8 cap only at err >= 0.4 * max(A, 1).
 //
-// UNCERTAIN, and labelled at the call site: `C`. The image prefers the control
-// block's ordered cruise altitude ctl+398h and falls back to the approach's own
-// base; this host models no control block, so it takes the fall-back. The image
-// is not known to reach the other arm in this mission - approach+0Ch is
-// unit+9D4h (009F9CFC) and nothing in this reconstruction fills it.
+// `C` (SETTLED, cc13-audit7): the image prefers the control block's ordered
+// cruise altitude ctl+398h and falls back to the approach's own base. The
+// fall-back is dead: 009C7A80 dereferences approach+0Ch unconditionally at
+// 009C7AA4/009C7AA7 on every think (and copies ctl+398h to approach+ACh), and
+// the arm runs it before this tick, so the call site passes ctl+398h, which the
+// host keeps as db_begin_alt_ac.
 // ---------------------------------------------------------------------------
 struct DiveBombFlyAboveAltitudeInputs {
     float height_above_aim_b = 0.0f;    // B, 009C6493
@@ -1246,8 +1249,18 @@ DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
 // the image. Under-approximating is the safe direction: this latch fires
 // strictly less often than the image's. `bearing_error_abs` is the same
 // [ESP+18h] that DiveBombAimGlideReleaseInputs::bearing_error_18 carries.
-// coverage: partial, arm B (009C57D3-009C57FD) unbound.
+// coverage: arm A here; arm B below (cc13-audit7 walked its three slots).
 bool dive_bomb_aimglide_pull_out_009c57ff(float bearing_error_abs) noexcept;
+// 009C57D3-009C57FD, arm B, reached when arm A did not latch. The slots, walked
+// from frame base E (ESP after the four pushes; the argument slot is E+6Ch):
+//   [E+24h] the 009C5353 ratio (range / throw), not written again;
+//   [E+6Ch] p1, 009C5563's FSTP of the first 00419010 (the ratio arm);
+//   [E+28h] p2, 009C55B0's FSTP of the second 00419010 (the height arm).
+// Latch when 0.6 (00CE3D30) > ratio, unit+C64h < 0 (009C57E8 against the
+// XORPS zero of 009C562D/009C57AF), and p2 > p1 (009C57F9 FCOMIP, JBE).
+// tools/x87_sym.py 009C5180 confirms the three operands.
+bool dive_bomb_aimglide_pull_out_arm_b_009c57d3(float ratio_24, float pitch_c64,
+                                                float p1, float p2) noexcept;
 
 // 009C6A37-009C6A7F, the symmetric dead band on the signed bearing error E,
 // half-width T. 009C6A46's JBE skips the whole arm when T <= 0, and there the
diff --git a/src/dive_bomb_task.cpp b/src/dive_bomb_task.cpp
index 6bd2994e7..56d5a4be8 100644
--- a/src/dive_bomb_task.cpp
+++ b/src/dive_bomb_task.cpp
@@ -778,10 +778,14 @@ DiveBombFlyAboveAltitudeCommand dive_bomb_flyabove_altitude_009c6e10(
         out.level_arm = true;
         return out;
     }
+    // 009C6F33 FADD ST0,ST0 then 009C6F3B FDIV. ST0 there is the height error:
+    // 009C6F11 FCOMI / 009C6F13 FSTP ST1 keep err in ST0, and 009C6F17 FLD A is
+    // popped again by 009C6F1D FCOMIP and 009C6F1F FSTP ST0 (cc13-audit7,
+    // tools/x87_sym.py 009C6E10: `(v9c6e44 + v9c6e44) / divisor`).
     const float v = static_cast<float>(
-        (static_cast<double>(in.planar_distance) +
-         static_cast<double>(in.planar_distance)) /
-        static_cast<double>(divisor));  // 009C6F33 FADD ST0,ST0 then 009C6F3B FDIV
+        (static_cast<double>(out.height_error) +
+         static_cast<double>(out.height_error)) /
+        static_cast<double>(divisor));
     // 009C6F4D FCOMIP against the 0.8 at 00CE3D40, 009C6F51 `76` JBE.
     out.reference = (static_cast<double>(v) <=
                      dive_bomb_flyabove_constant::kLeaveSpanScale)
@@ -943,6 +947,17 @@ bool dive_bomb_aimglide_pull_out_009c57ff(float bearing_error_abs) noexcept {
     return bearing_error_abs > dive_bomb_constant::kGlidePullOutBearing;
 }
 
+// 009C57D3-009C57FD, arm B.
+bool dive_bomb_aimglide_pull_out_arm_b_009c57d3(float ratio_24, float pitch_c64,
+                                                float p1, float p2) noexcept {
+    // 009C57DB COMISS 0.6,[ESP+24h] / 009C57E0 JBE.
+    if (!(dive_bomb_constant::kGlidePullOutRatio > ratio_24)) return false;
+    // 009C57E8 COMISS 0,unit+C64h / 009C57EF JBE.
+    if (!(0.0f > pitch_c64)) return false;
+    // 009C57F1 FLD p1, 009C57F5 FLD p2, 009C57F9 FCOMIP / 009C57FD JBE.
+    return p2 > p1;
+}
+
 // 009C6A37-009C6A7F.
 float dive_bomb_flyabove_dead_band_009c6a37(float bearing_error,
                                             float half_width_t) noexcept {
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 4f28e3401..65264093f 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -23703,12 +23703,24 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 unit_.plan_heading_2c0_written = false;
                                 unit_.plan_slots[bsp::kPilotSlotYaw].desired = st.yaw_284;
                                 unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;
+                                // 009C5420 plan+2D4h = 0: 0099E756 then skips
+                                // the planner's yaw arm 0099E81A, which would
+                                // overwrite the 284h just written.
+                                unit_.gl_yaw_mode_2d4_zero = true;
                             }
                         }
                         if (r.wrote_heading && !glide_yaw_arm) {
                             unit_.plan_heading_2c0 = r.heading_2c0;
                             unit_.plan_heading_2c0_written = true;
                             unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
+                            // 009C5444-009C547E: the same arm writes the bank
+                            // scale plan+2E8h = InterpolateClamped(100 (00CE3D08),
+                            // 1, 250 (00CE77B0), 1.5 (00CE380C), miss [ESP+1Ch]);
+                            // 0099B52C reseeds 1.0 every think.
+                            unit_.plan_state.turn_scale_2e8 =
+                                bsp::clamped_interpolate_00419010(
+                                    100.0f, 1.0f, 250.0f, 1.5f,
+                                    unit_.db_impact_planar_5c);
                         }
                         if (kAimGlideThrottleBound) {
                             // [ESP+10h] is the planar distance to the fed aim
@@ -23750,6 +23762,16 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             unit_.plan_state.pitch_target_2bc = gp.pitch_target_2bc;
                             unit_.plan_state.pitch_mode_2d0 =
                                 bsp::dive_bomb_glide_pitch_constant::kPitchMode;
+                            // 009C57D3-009C57FD, the pull-out latch's arm B:
+                            // [ESP+24h] is the 009C5353 ratio, [ESP+6Ch] p1
+                            // (009C5563) and [ESP+28h] p2 (009C55B0).
+                            if (!unit_.db_aimglide_pull_out_76c &&
+                                bsp::dive_bomb_aimglide_pull_out_arm_b_009c57d3(
+                                    pin.ratio_24, unit_.plane_pitch_angle_c64,
+                                    gp.p1, gp.p2)) {
+                                unit_.db_aimglide_pull_out_76c = true;
+                                ++unit_.db_aimglide_pull_outs;
+                            }
                         }
                     }
 
@@ -23790,11 +23812,15 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             // point, the same quantity 009C7A80 keeps in
                             // approach+BCh; the flyabove recomputes it locally.
                             ain.planar_distance = unit_.db_planar_bc;
-                            // SUBSTITUTION, labelled: approach+0Ch is unit+9D4h
-                            // and nothing in this reconstruction fills it, so
-                            // the arm takes 009C64B2's fall-back. The ordered
-                            // cruise altitude ctl+398h is unreachable here.
-                            ain.has_control_block_0c = false;
+                            // approach+0Ch is never null here: 009C7A80 reads
+                            // [approach+0Ch]+398h unconditionally at
+                            // 009C7AA4/009C7AA7 every think, before this tick
+                            // (009C87EA precedes 009C884C), and copies it to
+                            // approach+ACh. So 009C64A8 always takes ctl+398h,
+                            // which is db_begin_alt_ac this think; the 009C64B2
+                            // fall-back (ACh + 50h) is never taken.
+                            ain.has_control_block_0c = true;
+                            ain.cruise_altitude_398 = unit_.db_begin_alt_ac;
                             const bsp::DiveBombFlyAboveAltitudeCommand a =
                                 bsp::dive_bomb_flyabove_altitude_009c6e10(ain);
                             ++unit_.db_fa_alt_calls;
@@ -23994,6 +24020,14 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             unit_.plan_heading_2c0_written = true;
                             unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                         }
+                        // 009C6DF5-009C6E08: with flyabove+18h clear the bank
+                        // scale plan+2E8h is 1.5 (00CE380C), whether or not the
+                        // heading was written. The 1.4 (00D06874) store at
+                        // 009C683A, on the 009C67BF-009C6829 roll-in path, is
+                        // not modelled here.
+                        if (!unit_.db_flyabove_can_dive_18) {
+                            unit_.plan_state.turn_scale_2e8 = 1.5f;
+                        }
                     }
 
                     // 009C58D0's steering, the part of the aimdive tick that
@@ -24133,6 +24167,9 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             // 009C5E5D, 009C5E3F, 009C5E63 (+2D4h = 0).
                             unit_.plan_slots[bsp::kPilotSlotYaw].desired = tail.yaw_284;
                             unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;
+                            // 009C5E63 is that +2D4h = 0: without it the
+                            // planner's yaw arm 0099E81A overwrites the 284h.
+                            unit_.gl_yaw_mode_2d4_zero = true;
                             // 009C605F/009C5F37 and 009C606A.
                             unit_.plan_slots[bsp::kPilotSlotThrottle].desired = tail.throttle_278;
                             unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
```
