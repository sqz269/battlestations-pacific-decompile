# Audit of the plane bot states, part 3 (cc13-audit9)

Addresses: 009C62B0 009C6270 009C641C 009C6493 009C64C9 009C6530 009C659F 009C65FD 009C663E 009C6674 009C6678
009C66C1 009C66E3 009C6765 009C67B0 009C67C1 009C67D5 009C67DE 009C67E8 009C680E 009C6822 009C6826 009C683A
009C6857 009C6877 009C6909 009C6929 009C69EB 009C6A30 009C6A37 009C6AA3 009C6B75 009C6D23 009C6D59 009C6D6F
009C6DC8 009C6E08 009C83E0 009C84FB 009C8508 009C85E8 00419010 00438B10 00438AA0 007F0280

Packet `cc13-audit9`, branch `agent/cc13-audit9`, 2026-10-12. Requested by cc9 through the cc13 lead. It continues
`docs/CC13_AUDIT_PLANE_BOT_STATES_2.md` (cc13-audit7, "Not covered") and does not repeat that audit's items.

**Read only.** No Ghidra writes, no ledger records, and cc9's files are not changed on this branch. Findings that
need a change are given as one unified diff (section 6) for cc9 to apply.

**Base.** The diff applies on `origin/main` at `dd31c362a` **with audit 2's section-9 diff (A, B, C, C2, D, E)
already applied**: it rewrites C2's hunk (the 2E8h write) and sits next to D's. `git apply --check` was run on that
combination (section 6).

**Method.**
- The whole tick body `009C62B0`-`009C6DCD` was read from the disk listing (`bsp.py disasm-raw`). ESP was tracked
  from frame base `S` (ESP after `PUSH EDI` at `009C62C6`); every slot read was traced back to its last store.
- `tools/x87_sym.py` settled the two arithmetic readings that decide a finding: the cross-track factor
  (`009C6842`-`009C688D`) and the avoidance gain on both `+18h` paths (`009C6C22`-`009C6CBA`).
- Constants were read from the PE at the load's own width (`local/au9_const.py`, a copy of audit 7's script).
- Call census of the range (direct `CALL rel32` only; the range also has six indirect calls, at `009C62CF`, `009C6342`, `009C6404`, `009C647B`, `009C64EC` and
  `009C6705`):
  `00419010` x8, `00438B10` x3, `00438AA0` x3, `00414DB0` x2, `00BF701A` (atan2) x2, `00BF7030` (sqrt) x1,
  `009FA2E0` x1, `007F0280` x1. **There is no `00BD2F10` draw** anywhere in `009C62B0`-`009C6DCD`. The fly-over's
  only draw is the `009C3DA0` call in the enter `009C6276`, which the host already binds.

**Checklist.**
- Rule 2 (boundary): every cited site is inside `009C62B0` (Ghidra body `009C62B0`-`009C7085`), the enter
  `009C6270`-`009C62A8`, or the transition `009C83E0`.
- Rule 7 (argument counts): `00419010` is `RET 14h` (each of the eight sites opens with `SUB ESP,14h`).
  `00438B10`/`00438AA0` are `RET 8` (`SUB ESP,8`). `007F0280` takes six pushes, `ECX` = approach+0Ch.
- Rule 8 (register provenance):
  - `EBP` = 0 from `009C67CF XOR EBP,EBP` to `009C6936 MOV [ECX+CCh],EBP`, with no write between.
  - `BL` is set only at `009C6524`/`009C652A`/`009C6530` and reloaded from `+18h` at `009C6B7F`.
  - `DL` at `009C67D5`/`009C67DE` is `[ESP+27h]`, which is 0 in this installation
    (`docs/DIVE_BOMB_FLYOVER_FLAGS.md` section 1).
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table, so there is nothing for it
  to check.

**Status vocabulary.** The audited host code is reconstructed and build-tested on main. The diff in section 6 is
**build-tested** in this worktree (`scripts/build.ps1`), applied and then reverted. It is **not run**: no pair, no
game validation. Rule 6 applies, because `bsp_game.exe` reaches every one of these paths. The diff changes when the
fly-over leaves, so it needs a same-binary pair on the dive-bomb missions (USN04, JM08) before it lands.

## Verdicts

| # | block | verdict | one line |
| --- | --- | --- | --- |
| 1 | `009C62B0`-`009C63E6` lead point, R, bearing | FAITHFUL (precision notes) | `aim - 3(v_own - v_tgt) - pos`, the `1e-10` floor, `pi/2 - atan2(z,x)` and the `+2pi` wrap all match |
| 2 | `009C63EE`-`009C6453` E, Eabs | FAITHFUL | `00438B10(b, heading)` -> `[S+1Ch]`; fold `-0.0 - E` when `E <= 0` -> `[S+2Ch]` |
| 3 | `009C6462`-`009C6493` B | FAITHFUL | `double(pos.y) - aim.y` -> `[S+38h]` |
| 4 | **`009C64EE`-`009C6530` BL, its C** | **DEFECT** (audit 2 item 11, follow-on) | Predicate matches. But C is `ctl+398h`, and the host still passes `ACh + 50h`. Diff part 1 |
| 5 | `009C658D`-`009C65FD` span | FAITHFUL | `max(R - (0.7 max(B,100) + 200), 0)` |
| 6 | `009C6603`-`009C6674` leave tolerance, T0 | FAITHFUL | `IC(0, 20 deg, 0.8 B4 - S, pi, span)` and `IC(0, 30 deg, 200, 0, span)` |
| 7 | **`009C6678`-`009C66D1` bank limit** | **DEFECT** (unbound) | Within `700 x approach+24h` of the lead point: plan+2C8h = `IC(400, 0.2, 600, 1.2, R)` and approach+CCh = 3. Diff part 2 |
| 8 | **`009C66FB`-`009C67B0` +19h roll-in error** | **DEFECT** | The 1.6 rad test is on `heading - bearing(aim - approach+D8h/E0h)`, not on the lead-point error. Diff part 3 |
| 9 | **`009C659F`, `009C66E3`-`009C6826` flag semantics** | **DEFECT** | The flags are carried bytes. `+18h` is cleared every tick and set only on the BL leave edge. With BL set, `+1Ah` is cleared, `+19h` is never cleared by the 67B0 arm, and the leave clears it. Diff part 3 |
| 10 | **flag timing vs `009C83E0`** | **DEFECT** | The transition reads the previous tick's bytes. The host recomputes them in the feed and so answers one think early. Diff part 3 |
| 11 | **enter `009C6270`** | **DEFECT** | It seeds `+18h = +19h = 0` and `+1Ah = (approach+D1h == 0)`. The host seeds only `+1Ch`. Diff part 3 |
| 12 | **audit 2 item 13: the 1.4 at `009C683A`** | **SETTLED; DEFECT (unwired)** | It is reached only on the BL leave edge, and with C2 the tick ends with `2E8h = +18h ? 1.4 : 1.5`. Diff part 4 |
| 13 | **`009C6857`-`009C6889` cross-track** | **DEFECT** | `(0.5 + 0.5 sin Eabs) R`, not `1.5 sin(Eabs) R`. The x87_sym walk confirms it. Diff part 5 |
| 14 | `009C689E`-`009C6923` rest of the bank arm | FAITHFUL (gate wrong, see 9) | CCh = 3, `cos(Eabs) R - 120 <= 0` latches, `T = IC(0, 100 deg, TCR, 10 deg, R)` |
| 15 | **`009C6929`-`009C6A33` latched arm** | **DEFECT** (unbound) | It writes the yaw slot `sign(E) x IC(40,0,120,1,R) x IC(30,0,70 deg,1,Eabs)`, bank 0 in mode 1, and `2D4h = 0`. With `Eabs > pi/2` and `cos(Eabs) R < -20` it also sets `+19h = +18h`, the only road to turndown. Diff part 6 |
| 16 | `009C6A37`-`009C6A9F` dead band | FAITHFUL (its T was wrong, see 9) | `max(E - T, 0)` / `min(E + T, 0)`, skipped when `T <= 0`. Before the diff the host fed it the bank arm's T on every BL tick; the image keeps 009C6674's T0 unless the arm ran |
| 17 | `009C6AA3`-`009C6B75` near-field extents and speed slot | FAITHFUL (one nit) | Extents and slot match. Nit: the host's `0.05` is a double literal. The image's qword is `float(0.05)` (`00D7A270`). They differ only at `|a0| == float(0.05)` |
| 18 | `009C6B7B`-`009C6D42` avoidance heading increment | DEFECT (labelled, unbound) | The host labels it unbound. Transcription in section 4. No diff: it also needs `+20h` carried |
| 19 | `009C6D42`-`009C6DC8` slew | FAITHFUL | `AddWrapped(A, inc)`, `delta = Sub(A, heading)`, clamp to `±L` with `L = pi/2`, `AddWrapped(heading, clamped)` |

## 1. Lead geometry, E, B and C (`009C62B0`-`009C64C9`)

**1, FAITHFUL.** Frame `S` is ESP after `PUSH EDI` (`009C62C6`).
- **Own velocity.** `009C62CF` `vtable[34h]` on the unit gives the own velocity: `x -> [S+10h]`, `z -> [S+18h]`.
- **Target velocity.** `009C62E8 009FA2E0(approach+30h)` writes it to `[S+50h]..[S+58h]`.
- **Difference.** `dx`/`dz` are formed and rounded to float (`009C6301`, `009C630D`).
- **Lead offset.** `009C6328 FMUL ST(1),ST(0)` with the qword 3.0 (`00D7A2B0`), stored to `[S+50h]`/`[S+58h]`.
- **Lead point.** `aim - 3d` then `- pos (+FCh/+104h)` (`009C6346`-`009C6371`).
- **Range.** `R = sqrt(x^2 + z^2)` with the `1e-10` qword floor (`00CE3820`, `009C6397 JBE` -> XORPS 0), stored to
  `[S+28h]` at `009C63A6`.
- **Bearing.** `b = pi/2 - atan2(z, x)` with `00BF701A`, ST0 = x and ST1 = z. The `FSUBR` qword at `00CE3830`
  gives `pi/2 - atan`. `+2pi` (`00CE3828`) is added when `0 > b` (`009C63DE JBE`).

The host (`game_hosts_units.cpp`, the `dive_bomb_transition_inputs` feed) matches. Precision notes only:
- the image rounds `x^2 + z^2` to float before the sqrt (`009C6385`) and `atan2` to float before the subtraction
  (`009C63C4`);
- the host keeps both in double;
- the image adds the qword `2pi` to the float b in x87, where the host adds `float(2pi)`.

At the main thread's 24-bit precision these are last-place differences.

**2, FAITHFUL.**
- **Heading.** `009C6404 vtable[50h]` on the unit, stored to `[S+4Ch]`.
- **E.** `009C641C 00438B10(b, heading)` (`[ESP] = b`, `[ESP+4] = heading`) gives E, stored to `[S+1Ch]`.
- **Fold.** `009C642B FCOMPI`/`009C642F JBE` keeps E when `E > 0`. Otherwise it takes `-0.0 (00D7A208) - E`
  into `[S+2Ch]`.

**3, FAITHFUL.** `009C6465 FLD [unit+100h]` / `009C646D FSTP qword [S+10h]`, then `009C647B vtable[0]` on the
approach. `009C6482 FSUBR qword [S+10h]` gives `pos.y - aim.y`, and `009C6493` stores it to `[S+38h]`.

**4, DEFECT.**
- **The predicate.** `009C64F2`-`009C6530` matches the host's `dive_bomb_flyabove_bank_arm_009c6530`, and the walk
  agrees with its comment:
  - ST0 = `D4h`, ST1 = C (`FCOMI`/`JA` -> 0);
  - `B4h` against `ST4` = R (`FCOMPI`/`JBE` -> 1);
  - `FXCH ST(2)`, then B against `D4h` (`FCOMI`/`JB` -> 0).
- **C.** It is `[S+3Ch]` from `009C64C9`, the value `009C64A8` loads from `ctl+398h` whenever approach+0Ch is non-null.
  Audit 2 item 11 showed that approach+0Ch is never null on this path.
- **The gap.** Audit 2's diff D corrected the altitude arm but left the BL call site passing
  `db_begin_alt_ac + db_aim_point_height_50`, the dead fall-back. The diff passes `db_begin_alt_ac`.

## 2. Span, tolerances, the bank limit (`009C658D`-`009C66D1`)

**5, FAITHFUL.**
- **Floor.** `009C659B FCOMPI` (qword 100.0 `00D7A220` against B), `009C65A9 JBE`: the floor is B when
  `100 <= B`, else the float 100 (`00CE3D08`).
- **Threshold.** `S = float(max x 0.7 (00CEFFA0) + 200 (00CE4D70))`.
- **Span.** `009C65DB FSUBP ST(2)` gives `R - S`, floored at 0 (`009C65E9 FCOMPI 0 vs x`, `JBE`). It is stored to
  `[S+30h]` at `009C65FD`, under `SUB ESP,14h`.
- **The clear.** `009C659F` writes `+18h = 0` on this path, every tick (item 9).

**6, FAITHFUL.**
- **Tolerance.** `009C663E 00419010(0, 20 deg (00CE398C), float(0.8 (00CE3D40) x B4h - S), pi (00D7A264), span)`
  gives `tol` in `[S+10h]`. The host's `dive_bomb_flyabove_leave_009c66e3` has the same arguments.
- **T0.** `009C666F 00419010(0, 30 deg (00CEC724), 200 (00CE386C), 0, span)` gives T0 in `[S+20h]`. The `FST` at
  `009C6654` supplies both zeros.

**7, DEFECT (unbound).**
- **Gate.** `009C6678 FLD R`, `009C667C FLD [approach+24h]`, `FMUL` qword 700 (`00D20D20`), `009C6685 FCOMPI`,
  `009C6687 JBE`. The arm runs when `700 x approach+24h > R`.
- **Writes.** `009C6690` writes approach+CCh = 3. Then `009C66C1 00419010(400 (00CFD710), 0.2 (00CE54A0),
  600 (00CE4BC4), 1.2 (00CE3814), R)` -> `009C66CB` plan+2C8h, through approach+18h.
- **plan+2C8h is the bank limit.**
  - `plane_ai_control.cpp:543` clamps the bank target to `±2C8h` when it is below pi.
  - `0099B55E` reseeds it to 20.0 every think.
  - So within 700 m the fly-over limits the bank to 0.2-1.2 rad.
- **The host** writes neither field. approach+24h is the speed ratio `max(MaxSpd / ref, 1)`. The dive-bomb host
  already substitutes 1.0 for it (`b.speed_ratio_41c = 1.0f` in the break-off feed), and the diff does the same,
  labelled.
- **Not modelled.** approach+CCh is not kept by this host (`dive_bomb_aimdive_tail.hpp:183`), so the `= 3` and
  item 15's `= 0` are left as contract.

## 3. The flags (`009C66D5`-`009C6A35`, the enter, the transition)

All three findings (8-11) are about the same four state bytes. The image, in this installation (`+1Bh == 0`,
`squadron+3A8h == 0`):

```
enter 009C6270   +18h = +19h = +1Bh = +1Ch = 0, +20h = 0,
                 +1Ah = (approach+D1h == 0)          009C6298 CMP / 009C62A1 SETE / 009C62A4
think N:
  009C8834 009C83E0 (transition) reads +19h 009C84FB, +18h 009C8508, +1Ah 009C85E8:
      +19h ? (+18h ? turndown : aimglide 009C85D4) : (+1Ah ? goaway 009C85F5 : stay)
  009C884C tick 009C62B0:
      009C6540  [S+43h] = +19h (entry), 009C653C [S+4Bh] = +1Ah (entry)
      009C659F  +18h = 0
      009C66E1  Eabs > tol  -> +1Ah = 1, +19h = 0            (009C66E3/009C66E7, JMP 009C67BC)
                otherwise   -> +1Ah = 0 (009C66F2); +19h = 1 if 009C67B0's rule holds on E2
      009C67C1  BL == 0 -> 009C6A37
      009C67D8  +1Ah == 0 -> 009C6A35;  009C67E2  [S+4Bh] != 0 -> 009C6A35
      009C67E8  +18h = (B > D4h); +1Ah = 0 (009C6822); +19h = [S+43h] (009C6826); 2E8h = 1.4 (009C683A)
      009C6861  +1Ch ? latched arm 009C6929 : bank arm 009C6857 (may clear +19h, may latch +1Ch)
      009C6929  latched arm, ends with +19h = +18h when Eabs > pi/2 and cos(Eabs) R < -20 (009C6A30)
```

**8, DEFECT: the roll-in error.**
- **The image's error.** `009C66FB vtable[0]` gives the aim point. `009C6709`/`009C6716` subtract approach+D8h and
  +E0h. Then `pi/2 - atan2` and the `+2pi` wrap (`009C6728`-`009C674F`) give the bearing E2b.
- **Arguments.** `009C6765 00438B10([ESP] = heading [S+4Ch], [ESP+4] = E2b)`. `[S+4Ch]` is the `009C6406` store;
  nothing writes it between.
- **The tests.** The fold `009C6778`-`009C678A` is compared with the qword 1.6 (`009C67A3 FCOMPI`, `009C67A7 JA`).
  The span test is `009C67A9 COMISS 0,[S+30h]`, `009C67AE JB`.
- **approach+D8h/E0h** is what 009C7A80 wrote this think (audit 2 section 1): the raw unit position before the range
  latch, and the predicted impact point after it. The host keeps the bearing from that point to the aim point as
  `db_impact_bearing_18` (`update_dive_bomb_approach`, the `009C5950` block).
- **The host's error.** It fed `wrapped_angle_subtract(lead_bearing, heading)`, the 3-second lead-point error
  `[S+1Ch]`. That is the same quantity as the `+1Ah` test, but not the one this test uses.

**9, DEFECT: the flag semantics.** The host recomputed all three bytes every think from three independent predicates:
- `+18h = B > D4h`;
- `+19h = 009C67B0's rule`;
- `+1Ah = Eabs > tol`.

The image differs in four ways:
- **+18h.** It is 0 at the end of every tick except a tick that reached `009C67E8`. That needs BL, `+1Ah` set this
  tick and `+1Ah` clear on entry (`009C67D5 CMP [ESI+1Ah],DL` / `JE`, `009C67DE CMP [ESP+4Bh],DL` / `JNE`, DL = 0).
  `009C6822` clears `+1Ah` again on that path, so the next tick's entry byte is 0. Every tick on which
  `BL && Eabs > tol` holds is therefore such a tick.
- **+1Ah.** With BL set it never survives the tick, so the leave edge does not send the state to goaway: it re-tests
  the height instead.
- **+19h.** It is set by `009C67B0` and never cleared on that arm. It is cleared:
  - by the leave (`009C66E7`), which the host's "ready first" order inverted;
  - by the bank arm (`009C688F`, `009C68E1`);
  - by the avoidance arm (`009C6D23`).
  `009C6826` restores it to its entry value on the BL edge.
- **Turndown.** It needs `+19h && +18h` at the next transition. `+18h` is only set on an edge tick, and on that tick
  `+19h` is the entry value; entry +19h is 0, because a set +19h makes the transition leave before the tick runs.
  So the only way to turndown is `009C6A30`'s `+19h = +18h` on the latched arm: the aircraft has latched,
  overflown the aim point (`Eabs > pi/2`), is more than 20 m past it along track, and is above approach+D4h.
  Every `009C67B0` set meets `+18h == 0` and gives aimglide.

The host's turndown fired on any think with `ready && B > D4h`. `docs/DIVE_BOMB_TASK.md` reports a turndown that
handed aimdive an aircraft pointed 179.9 degrees the wrong way; this audit does not test whether that is related.

**10, DEFECT: timing.**
- **Order.** `009C8834` (transition) precedes `009C884C` (tick) on every think (audit 2 section 1). The transition
  therefore reads the bytes the previous tick wrote.
- **The host.** It computed them inside `read_transition_inputs` from the current pose, one think earlier than the
  image.
- **The diff.** The feed returns the carried bytes, and the tick writes them.

**11, DEFECT: the enter.** The host's enter cleared only `+1Ch` ("the other three are recomputed every tick"). With
carried bytes it must seed:
- `+18h = +19h = 0`;
- `+1Ah = !approach+D1h`.

An aircraft that enters the fly-over without ordnance therefore leaves for goaway on the next think, as in the image.

**12, audit 2 item 13, SETTLED.**
- **The 1.4.** `009C6832`/`009C683A` writes plan+2E8h = 1.4 (`00D06874`) on the `009C67E8` path only, after
  `+18h` is set at `009C680E`.
- **The 1.5.** `009C6DF5 CMP [ESI+18h],0` / `JNZ` then writes 1.5 (`00CE380C`) at `009C6E08`.
- **Result.** In this installation `+18h` is non-zero only after `009C680E`, so the tick ends with
  `2E8h = +18h ? 1.4 : 1.5`, and the 1.4 survives exactly when the dive test passed.
- **Host wiring.** The host writes the 1.4 when the flag step reached the dive test, then keeps C2's 1.5 on a clear
  `+18h`. This is diff part 4, and it rewrites C2's comment.

## 4. Bank arm, latched arm, avoidance (`009C6857`-`009C6DC8`)

**13, DEFECT: the cross-track.**
```
009c685b FSIN ; 009c685d FSTP [S+10h]            s = float(sin Eabs)
009c686b FLD [S+10h] ; 009c686f FLD qword 0.5    ST0 = 0.5, ST1 = s
009c6875 FMUL ST(1),ST(0)                        ST1 = 0.5 s, ST0 = 0.5
009c6877 FADDP ST(1)                             ST0 = 0.5 s + 0.5
009c6879 FMUL [S+28h] ; 009c687d FSTP [S+10h]    (0.5 + 0.5 s) R
```
- **x87_sym.** `tools/x87_sym.py 009C6842 --stop 009C688D` prints `v9c687d := ((v9c685d * 0.5) + 0.5) * arg+0x28`.
- **Earlier readings.** The host and `docs/DIVE_BOMB_FLYOVER_FLAGS.md` section 2 read `1.5 sin`.
- **Effect.** At `Eabs = 0` the image's offset is `R/2`, not 0. The arm therefore clears `+19h` with `T = 0`
  (`009C6889` `JBE` not taken) whenever `R > 2.8 x TurnCircleRadius`, so the latch only becomes reachable inside
  that range.

**14, FAITHFUL** apart from the gate.
- **Turn circle.** `Rt = float(classDesc+268h x qword 1.4)` (`00D045F0`).
- **Weapon selector.** `009C68A0` writes CCh = 3.
- **Along-track.** `cos` is rounded to float at `009C68B0`. `009C68BE FMULP ST(2)` / `009C68C0 FXCH` /
  `009C68C2 FSUB` qword 120.0 give `cos x R - 120`.
- **Latch or T.** `JBE` at `009C68D4` latches. Otherwise `009C6909`'s window reads as the host has it.
- **Precision.** The host does not round `cos` to float; this is a precision note only.

**15, DEFECT: the latched arm (unbound).** `009C6929`-`009C6A33` runs every tick that reaches `009C6861` with `+1Ch`
set, and on the tick that sets it.
- **Weapon selector.** `009C6936` writes approach+CCh = 0 (EBP).
- **The two factors.**
  - `v1 = 00419010(40 (00CE685C), 0, 120 (00D05804), 1, R)` at `009C695B`;
  - `v2 = 00419010(30 deg (00CEC724), 0, 70 deg (00CE3988), 1, Eabs)` at `009C698E`;
  - `009C6993 FMUL` / `009C69A5 FSTP` give `m = float(v2 x v1)` into `[S+30h]`.
- **Bank.** plan+2C4h = 0 (`009C69B1`), plan+2CCh = 1 (`009C69B9`).
- **Sign.** `-1` if `0 > E` (`009C69A9 COMISS`/`JBE`), `1` if `E > 0` (`009C69CB`/`JA`), else 0 (EBP).
- **Yaw slot.** plan+288h = 1 (`009C69E1`, CL), plan+284h = `float(sign x m)` (`009C69EB`), plan+2D4h = 0
  (`009C69F1`).
- **The turndown set.** If `Eabs > pi/2` (qword `00CE3830`, `009C6A05 JBE`) and `-20 (00CF180C) > float(float(cos
  Eabs) x R)` (`009C6A2B JBE`), then `+19h = +18h` (`009C6A2D`/`009C6A30`).
- **The jump.** It goes to `009C6A3D`, the dead band with T0.

The host suppressed the heading and wrote nothing else, so the planner flew its own yaw/bank arms. The diff binds
the bank and the yaw slot the way the follow hold arm already does (`game_hosts_units.cpp`, `009BF716`-`009BF789`).

**17, nit.** `near_field_flyover_slot_009c6b75` compares `|a0| > 0.05` with a double literal. The image's qword at
`00D7A270` is `0.05000000074505806`, which is `float(0.05)`. The two disagree only when `|a0| == float(0.05)`. This
is not in the diff.

**18, the avoidance heading increment (labelled unbound in the host), transcribed.**
- **The probe.** `007F0280(unit, ext [S+5Ch], a [S+68h], b [S+74h], w [S+80h], 1)` with `ECX = approach+0Ch`. When
  `|a0| <= 0.05` (`009C6B75 JBE`), `[S+20h] = 0` and A is unchanged.
- **The gain.** Otherwise, with `f = +18h`:

```
s1 = -a0 * b1 * b2                                         009C6B9E -> [S+1Ch]
s1 = s1 > 0.5 ? 2(s1 - 0.5) : s1 < -0.5 ? 2(s1 + 0.5) : 0  009C6BCE-009C6C0A (qword 2.0 00D7A308)
if (!f) s1 *= 0.5                                          009C6C14
k1 = IC(150, 0.2, 350, 1, R)                               009C6C50
inc = float(float(s1 * 0.43633 (00D1F6C0)) * k1) * (f ? k1 : IC(500, 0, 800, 1, R))   009C6CB6
```

  The `f ? k1` arm is the compiler's reuse of ST0. x87_sym confirms both arms:
  - `v9c6cb6 := ret_9c6c50 * v9c6c77` (taken);
  - `ret_9c6ca9 * v9c6c77` (fall).
- **Side effects.** When `+19h && f && +20h == 0`:
  - if `||a0| - b0| > 0.01` (`00D7A238`), `+19h = 0` (`009C6D23`);
  - otherwise `+20h = a0 >= 0 ? -1 : +1` (`009C6D31`/`009C6D3B`).
- **Then.** `009C6D54` `A = AddWrapped(A, inc)`.
- **No diff.** `+20h` (seeded 0 by the enter) is not carried by the host, and the probe is bound only for the speed
  slot. This is left for cc9 as a packet, not a line.

## 5. For cc9

- **Behaviour change.** Diff parts 3 and 6 change when the fly-over leaves:
  - turndown only after the latched overfly;
  - aimglide on any 67B0 set;
  - no goaway while BL holds.

  This is the largest behaviour change of the three audits. Take a same-binary pair on USN04 and JM08 and compare
  the per-aircraft fly-over exits, not the aggregate. The flyabove census fields (`db_flyabove_bl_ticks`,
  `db_flyabove_latch_tick`, transition `f18`/`f19`) still count, and now count the carried bytes.
- **Switch.** If you want one, the natural seam is the feed. With the switch off, the old recomputation stays and
  the tick's flag step only drives the latched arm. The diff has no switch, as audit 2's did not.
- **Independent parts.** Parts 1 (BL's C), 2 (2C8h), 5 (cross-track) and 6 (latched arm) are separable. Part 4
  (2E8h) depends on part 3's `reached_dive_test`.
- **Not in the diff.** Item 18 (the avoidance heading increment, `+20h`) and approach+CCh.
- **Correction for docs/DIVE_BOMB_FLYOVER_FLAGS.md section 2** (cc9's or the integrator's doc, not edited here):
  - "folds it into `sin + 0.5 * sin`, i.e. `1.5 * sin`" should read `0.5 + 0.5 sin`;
  - the `009C680E` row of `docs/DIVE_BOMB_TASK.md`'s flag table lacks the `+1Ah` edge condition at
    `009C67D5`/`009C67DE`.

## 6. The diff

- **Base.** `origin/main` `dd31c362a` **plus audit 2's section-9 diff**: `git apply` of
  `docs/CC13_AUDIT_PLANE_BOT_STATES_2.md`'s block succeeds on `dd31c362a`.
- **Check.** On that combination `git apply --check` of the block below returns 0. On `origin/main` alone it fails
  at the C2 hunk, as expected.
- **Build.** With both applied, `scripts/build.ps1` passes (exit 0; the two ctest cases pass). Both diffs were then
  reverted; cc9's files are unchanged on this branch.

The diff has six parts:
1. BL's C;
2. the 2C8h arm;
3. carried flags (the feed, the enter, the tick's flag step);
4. 2E8h;
5. the cross-track;
6. the latched arm.

```diff
diff --git a/include/bsp/dive_bomb_task.hpp b/include/bsp/dive_bomb_task.hpp
index 16a04bf53..79a8d79da 100644
--- a/include/bsp/dive_bomb_task.hpp
+++ b/include/bsp/dive_bomb_task.hpp
@@ -1197,7 +1197,10 @@ bool dive_bomb_flyabove_bank_arm_009c6530(bool is_recon_plane,
 //   Rt   = classDesc+268h TurnCircleRadius * 1.4, base[ESP+30h] at 009C6853
 //
 //   009C6861  +1Ch already set        -> skip the arm entirely (the latch)
-//   009C6889  1.5 * sin(Eabs) * R > Rt-> +19h = 0, T = 0 (009C6893's XMM0)
+//   009C6889  (0.5 + 0.5 sin(Eabs)) R > Rt -> +19h = 0, T = 0 (009C6893's XMM0)
+//             CORRECTED (cc13-audit9): 009C6875 FMUL ST(1),ST(0) leaves the 0.5
+//             in ST0 and 009C6877 FADDP adds it, so the factor is 0.5 + 0.5 sin,
+//             not 1.5 sin (tools/x87_sym.py 009C6842: `((sin * 0.5) + 0.5) * R`).
 //   009C68A0  otherwise                  approach+CCh = 3, the weapon selector
 //   009C68D4  cos(Eabs) * R - 120 <= 0-> +1Ch = 1, the LATCH; T untouched
 //             otherwise               -> +19h = 0 and 009C6911's
@@ -1222,11 +1225,90 @@ struct DiveBombFlyAboveBank {
     bool weapon_select_3 = false;   // 009C68A0 approach+CCh = 3
     float dead_band_t = 0.0f;       // base[ESP+20h] as it reaches 009C6A43
     float along_track = 0.0f;       // cos(Eabs) * R, kept for the census
-    float cross_track = 0.0f;       // 1.5 * sin(Eabs) * R
+    float cross_track = 0.0f;       // (0.5 + 0.5 sin(Eabs)) * R
 };
 DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
     const DiveBombFlyAboveBankInputs& in) noexcept;
 
+// ---------------------------------------------------------------------------
+// 009C659F-009C6A33, the fly-over's flag step (cc13-audit9), for this
+// installation's +1Bh == 0 and squadron+3A8h == 0 (kOldStyleBombing1b). The
+// flags are state bytes that persist across ticks; 009C83E0 reads them at the
+// START of the next think (009C84FB +19h, 009C8508 +18h, 009C85E8 +1Ah), before
+// this tick runs again, so the transition sees the previous tick's values.
+//
+//   009C659F  +18h = 0, every tick
+//   009C66E1  Eabs > tol (009C663E)   -> +1Ah = 1, +19h = 0 (009C66E3/009C66E7)
+//             otherwise               -> +1Ah = 0 (009C66F2), and +19h = 1 when
+//             009C67B0's rule holds on the ROLL-IN error: |SubtractWrappedAngle(
+//             heading, bearing(aim - approach+D8h/E0h))| (009C6707-009C6796),
+//             not the lead-point error; +19h is never cleared on this arm
+//   009C67C1  BL == 0                 -> done
+//   009C67D8  +1Ah == 0               -> done (009C6A35)
+//   009C67E2  +1Ah was set on entry   -> done (009C6A35); [ESP+4Bh] is the
+//                                        entry byte saved at 009C653C
+//   009C67E8  +18h = B > approach+D4h (009C67F6 JBE), +1Ah = 0 (009C6822),
+//             +19h = its ENTRY value (009C6826 from [ESP+43h], 009C6540),
+//             plan+2E8h = 1.4 (009C683A, 00D06874)
+//   009C6861  +1Ch clear -> the bank arm 009C6857; done unless it latched
+//   009C6929  the latched arm: approach+CCh = 0, plan+2C4h = 0 with 2CCh = 1,
+//             yaw slot plan+284h = sign(E) * IC(40, 0, 120, 1, R)
+//             * IC(30 deg, 0, 70 deg, 1, Eabs) with +288h = 1 and +2D4h = 0,
+//             and +19h = +18h when Eabs > pi/2 and cos(Eabs) * R < -20
+//             (009C69F7-009C6A30)
+//
+// So with BL set the leave edge does not leave: it re-tests the height, and the
+// state reaches turndown only through 009C6A30 (+19h = +18h) on the latched
+// arm; 009C67B0's set always meets +18h == 0 and gives aimglide.
+// ---------------------------------------------------------------------------
+struct DiveBombFlyAboveFlags {
+    bool can_dive_18 = false;
+    bool ready_19 = false;
+    bool leave_1a = false;
+    bool latch_1c = false;
+};
+struct DiveBombFlyAboveStepInputs {
+    DiveBombFlyAboveFlags entry;   // the bytes as the previous tick left them
+    bool bank_arm_bl = false;      // 009C6530
+    bool leave_now = false;        // dive_bomb_flyabove_leave_009c66e3
+    float roll_in_error = 0.0f;    // 009C6765's result, signed
+    float span = 0.0f;             // base[ESP+30h], 009C65FD
+    float bearing_error = 0.0f;    // E, base[ESP+1Ch], 009C6421
+    float bearing_error_abs = 0.0f;  // Eabs, base[ESP+2Ch]
+    float height_above_aim_b = 0.0f; // B, base[ESP+38h]
+    float release_range_d4 = 0.0f;   // approach+D4h
+    float lead_range_r = 0.0f;       // R, base[ESP+28h]
+    float turn_circle_radius = 0.0f; // classDesc+268h
+    float span_dead_band = 0.0f;     // 009C6674's T
+};
+struct DiveBombFlyAboveStep {
+    DiveBombFlyAboveFlags flags;
+    bool reached_dive_test = false;  // 009C67E8 ran this tick
+    bool bank_arm_ran = false;       // 009C6857 ran this tick
+    DiveBombFlyAboveBank bank;       // valid when bank_arm_ran
+    float dead_band_t = 0.0f;        // base[ESP+20h] at 009C6A3D
+    bool latched_arm = false;        // 009C6929-009C6A33 ran this tick
+    float yaw_284 = 0.0f;            // plan+284h on the latched arm
+};
+inline constexpr float kLatchYawRangeNear = 40.0f;    // 00CE685C, 009C6952
+inline constexpr float kLatchYawRangeFar = 120.0f;    // 00D05804, 009C6942
+inline constexpr float kLatchYawErrorNear = 0.5235987901687622f;  // 00CEC724
+inline constexpr float kLatchYawErrorFar = 1.2217305898666382f;   // 00CE3988
+inline constexpr double kLatchBehindBearing = 1.5707963705062866; // 00CE3830, qword
+inline constexpr float kLatchBehindRange = -20.0f;    // 00CF180C, 009C6A21
+inline constexpr float kRollInTurnScale = 1.399999976158142f;     // 00D06874, 009C6832
+inline constexpr float kFlyAboveTurnScale = 1.5f;     // 00CE380C, 009C6E00
+DiveBombFlyAboveStep dive_bomb_flyabove_flags_009c659f(
+    const DiveBombFlyAboveStepInputs& in) noexcept;
+
+// 009C6678-009C66D1: within approach+24h * 700 (qword 00D20D20) of the lead
+// point the tick sets approach+CCh = 3 and the bank limit plan+2C8h =
+// IC(400 (00CFD710), 0.2 (00CE54A0), 600 (00CE4BC4), 1.2 (00CE3814), R), which
+// 0099B55E reseeds to 20.0 every think. Returns false when the arm is skipped
+// (009C6687 JBE: 700 * ratio <= R).
+bool dive_bomb_flyabove_bank_limit_009c6678(float speed_ratio_24, float lead_range_r,
+                                            float* bank_limit_2c8) noexcept;
+
 // ---------------------------------------------------------------------------
 // 009C57C4-009C57FF, the aimglide's pull-out latch, state+18h = whole-object
 // +76Ch. Packet cc8_dive_flyover. The tick has exactly one store to that byte
diff --git a/src/dive_bomb_task.cpp b/src/dive_bomb_task.cpp
index d5db78551..f86060041 100644
--- a/src/dive_bomb_task.cpp
+++ b/src/dive_bomb_task.cpp
@@ -905,10 +905,14 @@ DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
     const float turn_circle = static_cast<float>(
         static_cast<double>(in.turn_circle_radius) *
         dive_bomb_flyabove_constant::kTurnCircleMul);
-    // 009C6857-009C687D. FSIN, then (sin + 0.5 * sin), then * R, stored as a
-    // float at 009C687D and reloaded at 009C6885.
+    // 009C6857-009C687D. FSIN stored as a float at 009C685D, then 009C686F
+    // FLD 0.5, 009C6875 FMUL ST(1),ST(0) (ST1 = 0.5 sin, ST0 = 0.5) and
+    // 009C6877 FADDP: (0.5 + 0.5 sin) * R, stored as a float at 009C687D.
+    // CORRECTED (cc13-audit9); this read `1.5 * sin`.
+    const double sin_f = static_cast<double>(static_cast<float>(std::sin(eabs)));
     out.cross_track = static_cast<float>(
-        std::sin(eabs) * (1.0 + dive_bomb_flyabove_constant::kRollInSinHalf) *
+        (sin_f * dive_bomb_flyabove_constant::kRollInSinHalf +
+         dive_bomb_flyabove_constant::kRollInSinHalf) *
         range);
     // 009C6889 FCOMIP ST0,ST1 with ST0 = cross-track and ST1 = the turn circle,
     // 009C688D `76` JBE: only a strictly wider offset leaves.
@@ -940,6 +944,80 @@ DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
     return out;
 }
 
+// 009C659F-009C6A33, the flag step. Header carries the branch table.
+DiveBombFlyAboveStep dive_bomb_flyabove_flags_009c659f(
+    const DiveBombFlyAboveStepInputs& in) noexcept {
+    DiveBombFlyAboveStep out;
+    out.flags = in.entry;
+    out.dead_band_t = in.span_dead_band;  // 009C6674 -> base[ESP+20h]
+    out.flags.can_dive_18 = false;        // 009C659F
+    if (in.leave_now) {
+        out.flags.leave_1a = true;   // 009C66E3
+        out.flags.ready_19 = false;  // 009C66E7
+    } else {
+        out.flags.leave_1a = false;  // 009C66F2
+        // 009C67A7 JA / 009C67AE JB: only a set; nothing on this arm clears it.
+        if (dive_bomb_flyabove_roll_in_009c67b0(in.roll_in_error, in.span)) {
+            out.flags.ready_19 = true;  // 009C67B0
+        }
+    }
+    if (!in.bank_arm_bl) return out;  // 009C67C1
+    // 009C67D5 CMP [ESI+1Ah],DL / JE and 009C67DE CMP [ESP+4Bh],DL / JNE, DL = 0.
+    if (!out.flags.leave_1a || in.entry.leave_1a) return out;  // 009C6A35
+    out.reached_dive_test = true;
+    // 009C67EA-009C67F6: FCOMIP with ST0 = B and ST1 = approach+D4h, JBE.
+    out.flags.can_dive_18 =
+        dive_bomb_flyabove_can_dive_009c680e(in.height_above_aim_b, in.release_range_d4);
+    out.flags.leave_1a = false;           // 009C6822
+    out.flags.ready_19 = in.entry.ready_19;  // 009C6826, [ESP+43h] from 009C6540
+    if (!in.entry.latch_1c) {
+        DiveBombFlyAboveBankInputs bin;
+        bin.bank_arm_bl = true;
+        bin.bearing_error_abs = in.bearing_error_abs;
+        bin.lead_range_r = in.lead_range_r;
+        bin.turn_circle_radius = in.turn_circle_radius;
+        bin.span_dead_band = in.span_dead_band;
+        out.bank = dive_bomb_flyabove_bank_009c6857(bin);
+        out.bank_arm_ran = true;
+        if (out.bank.clear_roll_in_19) out.flags.ready_19 = false;  // 009C688F/009C68E1
+        out.flags.latch_1c = out.bank.latched_1c;
+        out.dead_band_t = out.bank.dead_band_t;
+        if (!out.flags.latch_1c) return out;  // 009C6923
+    }
+    // 009C6929-009C6A33, the latched arm.
+    out.latched_arm = true;
+    const float v1 = dive_bomb_interpolate_clamped_00419010(
+        kLatchYawRangeNear, 0.0f, kLatchYawRangeFar, 1.0f, in.lead_range_r);  // 009C695B
+    const float v2 = dive_bomb_interpolate_clamped_00419010(
+        kLatchYawErrorNear, 0.0f, kLatchYawErrorFar, 1.0f, in.bearing_error_abs);  // 009C698E
+    const float mag = static_cast<float>(static_cast<double>(v2) * v1);  // 009C6993/009C69A5
+    // 009C69A9 COMISS 0,E / JBE -> -1; 009C69CB COMISS E,0 / JA -> 1; else 0.
+    const int sign = (0.0f > in.bearing_error) ? -1 : ((in.bearing_error > 0.0f) ? 1 : 0);
+    out.yaw_284 = static_cast<float>(static_cast<double>(sign) * mag);  // 009C69D8-009C69EB
+    // 009C6A01 FCOMIP Eabs against the qword pi/2, 009C6A05 JBE.
+    if (static_cast<double>(in.bearing_error_abs) > kLatchBehindBearing) {
+        const float c = static_cast<float>(std::cos(static_cast<double>(in.bearing_error_abs)));
+        const float along = static_cast<float>(static_cast<double>(c) * in.lead_range_r);
+        // 009C6A27 FCOMIP with ST0 = -20 and ST1 = cos * R, 009C6A2B JBE.
+        if (kLatchBehindRange > along) {
+            out.flags.ready_19 = out.flags.can_dive_18;  // 009C6A2D/009C6A30
+        }
+    }
+    return out;
+}
+
+// 009C6678-009C66D1.
+bool dive_bomb_flyabove_bank_limit_009c6678(float speed_ratio_24, float lead_range_r,
+                                            float* bank_limit_2c8) noexcept {
+    // 009C667F FMUL qword 700 (00D20D20), 009C6685 FCOMPI against R, 009C6687 JBE.
+    if (!(static_cast<double>(speed_ratio_24) * 700.0 > static_cast<double>(lead_range_r))) {
+        return false;
+    }
+    *bank_limit_2c8 = dive_bomb_interpolate_clamped_00419010(
+        400.0f, 0.2000000029802322f, 600.0f, 1.2000000476837158f, lead_range_r);  // 009C66C1
+    return true;
+}
+
 // 009C57C4-009C57FF, arm A. Header carries arm B and why it is not modelled.
 bool dive_bomb_aimglide_pull_out_009c57ff(float bearing_error_abs) noexcept {
     // 009C57CA COMISS XMM0,[00CE380C] with XMM0 = [ESP+18h] and 009C57D1 `77`
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 66f49aff3..26d443e30 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -4491,28 +4491,18 @@ struct GameUnitsHost::Impl {
             const bsp::DiveBombFlyAboveSpan span =
                 bsp::dive_bomb_flyabove_span_009c65fd(height_above, lead_range);
             slot.db_flyabove_span = span.span;
-            // +18h at 009C680E: dive once higher above the aim point than
-            // approach+D4h, which is now the real 675.0 m.
-            in.flyabove_can_dive_790 = bsp::dive_bomb_flyabove_can_dive_009c680e(
-                height_above, slot.db_release_range_d4);
-            slot.db_flyabove_can_dive_18 = in.flyabove_can_dive_790;
-            // +19h at 009C67B0, BOUND. The first arm is the 1.6 rad bearing
-            // test at 00CE3D48; the second is `span <= 0` (009C67A9 with
-            // 009C67AE the byte 72, JC). RETRACTED by packet cc8_dive_heading:
-            // "with the 0.7/200.0 pair that is height <= 666.7 m, the same gate
-            // approach+D4h's 675.0 m expresses" was an artefact of feeding the
-            // span the height. The second arm is `R <= 0.7 * max(B,100) + 200`,
-            // a range-to-go test against a glide slope, and it has nothing to
-            // do with +D4h.
-            in.flyabove_ready_791 = bsp::dive_bomb_flyabove_roll_in_009c67b0(
-                lead_bearing_error, span.span);
-            slot.db_flyabove_ready_19 = in.flyabove_ready_791;
-            // +1Ah at 009C66E3, BOUND: leave when the folded bearing error
-            // beats a tolerance opening from 20 degrees at span 0 to pi at
-            // approach+B4h * 0.8 - S. 009C66E7 clears +19h on the same edge,
-            // which the transition rule already models by taking `ready` first.
-            in.flyabove_leave_792 = bsp::dive_bomb_flyabove_leave_009c66e3(
-                lead_bearing_error, span, slot.db_attack_dist_b4);
+            // +18h/+19h/+1Ah are state bytes, not per-think predicates
+            // (cc13-audit9). 009C83E0 reads them at 009C84FB/009C8508/009C85E8
+            // before this think's flyabove tick runs, so the transition sees
+            // what the PREVIOUS tick left; the tick writes them in
+            // dive_bomb_flyabove_flags_009c659f and the enter 009C6270 seeds
+            // them. Recomputing them here from the current pose answered one
+            // think early, gave +18h on every high tick (the image clears it at
+            // 009C659F and sets it only at 009C680E) and let `ready` beat the
+            // 009C66E7 clear.
+            in.flyabove_can_dive_790 = slot.db_flyabove_can_dive_18;
+            in.flyabove_ready_791 = slot.db_flyabove_ready_19;
+            in.flyabove_leave_792 = slot.db_flyabove_leave_1a;
             // Packet cc9_dive_modes, diagnostic only: the fly-over's roll-in
             // inputs per tick, to tell 009C67B0's two arms apart.
             if (kHullAimTrace && slot.dive_bomb_state == bsp::DiveBombState::kFlyAbove) {
@@ -22735,9 +22725,12 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 // the integrator's hunk arbitration of
                                 // 2026-09-19. It clears +18h/+19h/+1Bh/+1Ch
                                 // (009C6295/009C6292/009C628F/009C629E) and
-                                // sets +1Ah from `approach+D1h == 0`. Only the
-                                // +1Ch clear needs state here: the other three
-                                // are recomputed every tick by the state feed.
+                                // sets +1Ah from `approach+D1h == 0`
+                                // (009C6298 CMP / 009C62A1 SETE). All four are
+                                // carried state (cc13-audit9).
+                                unit_.db_flyabove_can_dive_18 = false;
+                                unit_.db_flyabove_ready_19 = false;
+                                unit_.db_flyabove_leave_1a = !unit_.db_has_bomb_d1;
                                 unit_.db_flyabove_bank_latch_1c = false;
                                 unit_.db_flyabove_latch_tick = -1;
                                 if constexpr (kAimErrorDrawBound) {
@@ -23793,6 +23786,99 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     // values need frame slots this body cannot resolve.
                     void run_dive_bomb_flyabove_tick_009c62b0() {
                         ++unit_.db_flyabove_tick_ticks;
+                        // 009C6421-009C6453: E = SubtractWrappedAngle(lead
+                        // bearing, heading) and its abs-fold Eabs.
+                        const float bearing_error =
+                            bsp::wrapped_angle_subtract_00438b10(
+                                unit_.db_flyabove_lead_bearing,
+                                unit_.plane_heading_c6c);
+                        const float bearing_error_abs =
+                            (bearing_error > 0.0f)
+                                ? bearing_error
+                                : (bsp::dive_bomb_constant::kNegativeZero -
+                                   bearing_error);
+                        // 009C64EE-009C6530, BL. C is ctl+398h: approach+0Ch
+                        // is never null on this path (cc13-audit7 item 11), so
+                        // 009C64A8 loads it and it is db_begin_alt_ac this
+                        // think. The 210 m clamp at 009C6580 runs after the BL
+                        // test and does not reach it.
+                        const bool bank_arm_bl =
+                            bsp::dive_bomb_flyabove_bank_arm_009c6530(
+                                /*is_recon_plane=*/false,
+                                unit_.db_release_range_d4,
+                                unit_.db_begin_alt_ac,
+                                unit_.db_attack_dist_b4,
+                                unit_.db_flyabove_lead_range,
+                                unit_.db_flyabove_height);
+                        if (bank_arm_bl) ++unit_.db_flyabove_bl_ticks;
+                        // 009C659F-009C6A33, the flag step (cc13-audit9).
+                        const bsp::DiveBombFlyAboveSpan fa_span =
+                            bsp::dive_bomb_flyabove_span_009c65fd(
+                                unit_.db_flyabove_height,
+                                unit_.db_flyabove_lead_range);
+                        bsp::DiveBombFlyAboveStepInputs fin;
+                        fin.entry.can_dive_18 = unit_.db_flyabove_can_dive_18;
+                        fin.entry.ready_19 = unit_.db_flyabove_ready_19;
+                        fin.entry.leave_1a = unit_.db_flyabove_leave_1a;
+                        fin.entry.latch_1c = unit_.db_flyabove_bank_latch_1c;
+                        fin.bank_arm_bl = bank_arm_bl;
+                        // 009C663E-009C66E1 on Eabs.
+                        fin.leave_now = bsp::dive_bomb_flyabove_leave_009c66e3(
+                            bearing_error, fa_span, unit_.db_attack_dist_b4);
+                        // 009C66FB-009C6765: the roll-in error is measured
+                        // from approach+D8h/E0h to the aim point, the bearing
+                        // 009C7A80 left in db_impact_bearing_18 this think,
+                        // against the heading: SubtractWrappedAngle(heading,
+                        // bearing). Not the lead-point error.
+                        fin.roll_in_error = bsp::wrapped_angle_subtract_00438b10(
+                            unit_.plane_heading_c6c, unit_.db_impact_bearing_18);
+                        fin.span = fa_span.span;
+                        fin.bearing_error = bearing_error;
+                        fin.bearing_error_abs = bearing_error_abs;
+                        fin.height_above_aim_b = unit_.db_flyabove_height;
+                        fin.release_range_d4 = unit_.db_release_range_d4;
+                        fin.lead_range_r = unit_.db_flyabove_lead_range;
+                        fin.turn_circle_radius = unit_.plane_turn_circle_radius;
+                        fin.span_dead_band =
+                            bsp::dive_bomb_flyabove_span_dead_band_009c6674(
+                                fa_span.span);
+                        const bsp::DiveBombFlyAboveStep fstep =
+                            bsp::dive_bomb_flyabove_flags_009c659f(fin);
+                        if (fstep.flags.latch_1c &&
+                            !unit_.db_flyabove_bank_latch_1c) {
+                            unit_.db_flyabove_latch_tick =
+                                unit_.dive_bomb_arm_ticks;
+                        }
+                        unit_.db_flyabove_can_dive_18 = fstep.flags.can_dive_18;
+                        unit_.db_flyabove_ready_19 = fstep.flags.ready_19;
+                        unit_.db_flyabove_leave_1a = fstep.flags.leave_1a;
+                        unit_.db_flyabove_bank_latch_1c = fstep.flags.latch_1c;
+                        unit_.db_flyabove_dead_band_t = fstep.dead_band_t;
+                        if (fstep.bank_arm_ran) {
+                            unit_.db_flyabove_along_track = fstep.bank.along_track;
+                            unit_.db_flyabove_cross_track = fstep.bank.cross_track;
+                        }
+                        // 009C6678-009C66D1: the bank limit plan+2C8h near the
+                        // lead point. approach+24h is taken as 1.0, the same
+                        // substitution the break-off feed makes (b.speed_ratio_41c).
+                        {
+                            float limit = 0.0f;
+                            if (bsp::dive_bomb_flyabove_bank_limit_009c6678(
+                                    1.0f, unit_.db_flyabove_lead_range, &limit)) {
+                                unit_.plan_state.bank_limit_2c8 = limit;
+                            }
+                        }
+                        if (fstep.latched_arm) {
+                            // 009C69B1/009C69B9: bank target 0 with plan+2CCh =
+                            // 1; 009C69E1-009C69F1: yaw slot desired, active,
+                            // plan+2D4h = 0.
+                            unit_.plan_state.bank_target_2c4 = 0.0f;
+                            unit_.plan_heading_mode_2cc = 1;
+                            unit_.plan_heading_2c0_written = false;
+                            unit_.plan_slots[bsp::kPilotSlotYaw].desired = fstep.yaw_284;
+                            unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;
+                            unit_.gl_yaw_mode_2d4_zero = true;
+                        }
                         // The heading arm now runs AFTER the altitude arm,
                         // because the bank predicate BL at 009C650E compares
                         // approach+D4h against C, the commanded altitude
@@ -23939,67 +24025,11 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // 007F0280. The 009C64EC query that can veto BL is NOT
                         // in that list any more - it is IsKindOf(RECON_PLANE),
                         // and these are dive bombers.
-                        const float bearing_error =
-                            bsp::wrapped_angle_subtract_00438b10(
-                                unit_.db_flyabove_lead_bearing,
-                                unit_.plane_heading_c6c);
-                        // 009C642F-009C6453, the class's abs-fold.
-                        const float bearing_error_abs =
-                            (bearing_error > 0.0f)
-                                ? bearing_error
-                                : (bsp::dive_bomb_constant::kNegativeZero -
-                                   bearing_error);
-                        // C as 009C650E sees it is the UNCLAMPED altitude of
-                        // 009C64C9, not the `limit_c` the altitude arm returns:
-                        // the 210 m release clamp at 009C6580-009C6589 sits
-                        // inside the 009C654A branch and runs AFTER the BL test
-                        // at 009C6532. Reading limit_c here would compare
-                        // approach+D4h (675 m) against 210 m and veto the bank
-                        // arm on every tick of every mission.
-                        const float unclamped_c = unit_.db_begin_alt_ac +
-                                                  unit_.db_aim_point_height_50;
-                        const bool bank_arm_bl =
-                            bsp::dive_bomb_flyabove_bank_arm_009c6530(
-                                /*is_recon_plane=*/false,
-                                unit_.db_release_range_d4,
-                                unclamped_c,
-                                unit_.db_attack_dist_b4,
-                                unit_.db_flyabove_lead_range,
-                                unit_.db_flyabove_height);
-                        if (bank_arm_bl) ++unit_.db_flyabove_bl_ticks;
-                        bsp::DiveBombFlyAboveBankInputs bin;
-                        bin.latched_1c = unit_.db_flyabove_bank_latch_1c;
-                        bin.bank_arm_bl = bank_arm_bl;
-                        bin.bearing_error_abs = bearing_error_abs;
-                        bin.lead_range_r = unit_.db_flyabove_lead_range;
-                        bin.turn_circle_radius = unit_.plane_turn_circle_radius;
-                        bin.span_dead_band =
-                            bsp::dive_bomb_flyabove_span_dead_band_009c6674(
-                                unit_.db_flyabove_span);
-                        const bsp::DiveBombFlyAboveBank bank =
-                            bsp::dive_bomb_flyabove_bank_009c6857(bin);
-                        if (bank.latched_1c &&
-                            !unit_.db_flyabove_bank_latch_1c) {
-                            unit_.db_flyabove_latch_tick =
-                                unit_.dive_bomb_arm_ticks;
-                        }
-                        unit_.db_flyabove_bank_latch_1c = bank.latched_1c;
-                        unit_.db_flyabove_dead_band_t = bank.dead_band_t;
-                        unit_.db_flyabove_along_track = bank.along_track;
-                        unit_.db_flyabove_cross_track = bank.cross_track;
-                        // 009C688F / 009C68E1 clear +19h. READ and NOT applied:
-                        // in the image +19h persists across ticks and this
-                        // host recomputes it from 009C67B0's rule alone in the
-                        // state feed, before the transition rule and before
-                        // this tick, so a clear written here would be
-                        // overwritten before anything could read it. Modelling
-                        // it means giving +19h its own carried state, which is
-                        // the 009C6A30/009C6826 pair as well, and that is a
-                        // packet rather than a line. `bank.clear_roll_in_19` is
-                        // carried for the census only.
+                        // E, Eabs, BL, the bank arm and T are computed at the
+                        // top of this tick by the flag step (cc13-audit9).
                         bsp::DiveBombFlyAboveCommandInputs in;
                         // 009C6DCD / 009C6DDA: the latch suppresses the write.
-                        in.suppress_heading_1c = bank.latched_1c;
+                        in.suppress_heading_1c = fstep.flags.latch_1c;
                         // 009C6A37-009C6A9F then 009C6D6F-009C6DC8. A is
                         // AddWrappedAngle(C, deadband) when 009C6A46's JBE is
                         // not taken and the raw bearing otherwise; the slew
@@ -24014,7 +24044,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 bsp::wrapped_angle_add_00438aa0(
                                     unit_.plane_heading_c6c,
                                     bsp::dive_bomb_flyabove_dead_band_009c6a37(
-                                        bearing_error, bank.dead_band_t)),
+                                        bearing_error, fstep.dead_band_t)),
                                 bsp::dive_bomb_flyabove_constant::
                                     kHeadingSlewLimit);
                         const bsp::DiveBombFlyAboveCommand r =
@@ -24030,11 +24060,15 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             unit_.plan_heading_2c0_written = true;
                             unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                         }
-                        // 009C6DF5-009C6E08: with flyabove+18h clear the bank
-                        // scale plan+2E8h is 1.5 (00CE380C), whether or not the
-                        // heading was written. The 1.4 (00D06874) store at
-                        // 009C683A, on the 009C67BF-009C6829 roll-in path, is
-                        // not modelled here.
+                        // 009C683A: plan+2E8h = 1.4 (00D06874) on the tick that
+                        // reached the dive test; 009C6DF5-009C6E08 then writes
+                        // 1.5 (00CE380C) whenever +18h ended the tick clear,
+                        // whether or not the heading was written. In this
+                        // installation +18h is set only at 009C680E, so the
+                        // tick leaves 1.4 exactly when +18h is set (cc13-audit9).
+                        if (fstep.reached_dive_test) {
+                            unit_.plan_state.turn_scale_2e8 = bsp::kRollInTurnScale;
+                        }
                         if (!unit_.db_flyabove_can_dive_18) {
                             unit_.plan_state.turn_scale_2e8 = 1.5f;
                         }
```

## 7. Handoff

- **Covered.** All of `009C62B0`-`009C6DC8` and the enter `009C6270`; audit 2's item 13 is settled. With audits 1
  and 2, the whole flyabove tick has now been audited.
- **Open.**
  - Item 18, the avoidance heading increment and `+20h`.
  - approach+CCh, the weapon selector, which `009C6690`, `009C68A0` and `009C6936` write.
  - The runtime pair for the section-6 diff.
  - Audit 2's open list (other plan+2E8h writers; `009B776C`/`009B7926`), which this audit did not take up.
