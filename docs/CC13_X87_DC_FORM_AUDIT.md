# CC13 x87 DC-form audit (cc13-x87dc1)

A read-only audit of how Ghidra renders and models the x87 register forms whose destination is
ST(i), and of every reconstruction that contains one. It was prompted by cc13-radial1's note that
Ghidra prints `FMUL ST1` for both `D8 C9` and `DC C9`. The report is
`reports/cc13_x87_dc_form_audit.json` (census, one verdict record per site, the WRONG list). The
tool is `tools/x87_dc_form_scan.py`. Ghidra was not modified.

## 1. The defect: the listing, not the p-code

Ghidra 12.0.4 (`J:/tools/ghidra_12.0.4_PUBLIC`, `Processors/x86/data/languages/ia.sinc`):

| Encoding | Intel meaning | Ghidra listing | Ghidra p-code (ia.sinc) |
|---|---|---|---|
| `D8 C8+i` | ST0 = ST0 * ST(i) | `FMUL STi` (5684) | `ST0 = ST0 f* freg` |
| `DC C8+i` | ST(i) = ST(i) * ST0 | `FMUL STi` (5690), **the same text** | `freg = freg f* ST0` |
| `D8 C0/E0/E8/F0/F8+i` | ST0 = ST0 op ST(i) | `FADD ST0,STi`, `FSUB ST0,STi`, ... | correct |
| `DC C0+i` FADD | ST(i) = ST(i) + ST0 | `FADD STi,ST0` | correct |
| `DC E0+i` FSUBR | ST(i) = ST0 - ST(i) | `FSUBR STi,ST0` | `freg = ST0 f- freg` |
| `DC E8+i` FSUB | ST(i) = ST(i) - ST0 | `FSUB STi,ST0` | `freg = freg f- ST0` |
| `DC F0+i` FDIVR | ST(i) = ST0 / ST(i) | `FDIVR STi,ST0` | `freg = ST0 f/ freg` |
| `DC F8+i` FDIV | ST(i) = ST(i) / ST0 | `FDIV STi,ST0` | `freg = freg f/ ST0` |
| `DE C8+i` FMULP | ST(i) *= ST0, pop | `FMULP STi` (only one meaning) | correct |
| `DE xx+i` other pops | as Intel | `FADDP STi,ST0`; bare `FADDP`/`FSUBP`/`FSUBRP`/`FDIVP`/`FDIVRP` for i = 1 | correct |
| `DC D0-DF`, `DE D0-D7`, `DE DA-DF` | undocumented FCOM/FCOMP aliases | no constructor | n/a: none occur in .text |

Listing checks: `00401885` (D8 C9) and `00401CD6` (DC C9) both read `FMUL ST1`; `0040201E` (D8 CA)
and `00404BCE` (DC CA) both read `FMUL ST2`; `00415EE9` reads `FADD ST0,ST1` (D8 C1), `0042FD6D`
reads `FADD ST1,ST0` (DC C1), `00417FD6` `FSUB ST1,ST0` (DC E9), `00C08394` `FSUBR ST1,ST0` (DC E1),
`00405B48` `FDIV ST1,ST0` (DC F9), `0040C989` `FSUBR ST0,ST1` (D8 E9), `00447A8C` `FDIVR ST0,ST1`.

Decompiler checks, each against a hand trace of the bytes: `00401CD0` (DC C9: all three products
present), `00668D40` (DC C9 at `00668D6C`: `val - dt*4.0`, the 4.0 kept), `00417FA0` (DC E9:
`n - 2.0`), `00C08390` (DC E1: `f2xm1(x - round(x))`), `00405AB0` (DC F9: `sum / 3.0`), `00438AA0`
(DC C2: the 2*pi correction on the angle), `00401D30` (DE F1: `1 / sqrt`). All correct.

So the scope is exactly one thing: **a reader of the listing cannot tell `D8 C8+i` from `DC C8+i`.**
The pseudocode is trustworthy for every D8/DC/DE register form, and capstone 5.0.7 prints all of
them correctly (`fmul st(1)` is D8, `fmul st(1), st(0)` is DC; DC E0+i is `fsubr`, DC E8+i `fsub`).

cc13-radial1's note that the pseudocode for `00668D40` is wrong is right for a different reason: the
loop at `00668E40-00668ED6` reads `[ESP+14h]` after `CALL EAX` sites, and the pseudocode shows that
slot as `unaff_ESI`. That is Ghidra's stack-depth tracking across indirect calls, not the DC form.
`00661B50` contains no DC-form instruction.

## 2. Census (whole .text, PE on disk)

`tools/x87_dc_form_scan.py` decodes from every function start in the bsp index up to the next
start, so a desync cannot cross a function. A sample of 16 FMUL sites checked against the live
Ghidra listing: 15 are instruction starts reading `FMUL STi`; one (`00C324C9`) lies outside every
Ghidra function.

DC register forms with i != 0 (i = 0 is `ST0 op ST0`, the same as D8): **835 sites in 470
functions**: `FMUL` 488 (306 functions), `FADD` 157, `FSUB` 113, `FDIV` 76, `FSUBR` 1, `FDIVR` 0. Of
these, 40 sites lie past a RET/JMP + INT3 in the sweep, inside functions the index does not know
(`00414FE0`, `009BE590`, `009D6D60` and `00B052F0` were confirmed). The tool prints those with
`?`. For scale: D8 forms 4,733 sites and DE forms 14,906 sites across the image.

## 3. Reconstructions containing a `DC C8+i` site

Only the FMUL sites are at risk, since every other DC form prints explicitly. Of the 306 functions,
210 are named in `src/` or `include/`; each site was classified (322 records, 4 parallel verifiers
plus cc13-x87dc1 for the cc13 rocket and fly-to functions):

| Verdict | Sites |
|---|---|
| CORRECT | 238 |
| NOT_RECONSTRUCTED (named only as a callee, host hook, counter or doc pointer) | 75 |
| NOT_COVERED (reconstructed, but the site's branch is excluded) | 4 |
| WRONG | 5 (4 DC misreads in 3 functions, plus 1 unrelated sign error) |

Many CORRECT sites are inline-asm or naked copies that spell the DC form explicitly (shadow, terrain,
particle, DYN) or carry an earlier DC note (`0043019A`, `009C6328`, `0081008C`, `007B02D0`,
`00B4CCD4`, `0099E14C`).

### WRONG

All three are in cc9's gameplay files. The corrections are given as diffs for cc9; cc13-x87dc1
did not edit them.

**W1. `00834F03` in `00834E90` UnitInstance propellers/steering, `src/unit_motion.cpp:304-306`.**
The bytes are `FSUB qword [00D7A250]` (-1.0), `FLD qword [00CEDCD0]` (K), `DC C9`, `DE E9`. The DC C9
leaves (s+1)*K in ST1 and K in ST0, and `FSUBP ST1,ST0` gives **(s+1)*K - K**. Ghidra's pseudocode
agrees. The C++ computes (s+1) - K*(s+1): at s = 0 the game gives 0 and the C++ gives 0.2146.
```
--- src/unit_motion.cpp
-    // 00834EF5..00834F05: (s - bias) then subtract blend * (s - bias).
-    const double biased = static_cast<double>(steering_source) - kUnitSteeringBias;
-    return static_cast<float>(biased - kUnitSteeringBlend * biased);
+    // 00834EF5..00834F05: DC C9 leaves blend in ST0 and (s - bias) * blend in ST1;
+    // DE E9 (FSUBP ST1,ST0) then gives (s - bias) * blend - blend.
+    const double biased = static_cast<double>(steering_source) - kUnitSteeringBias;
+    return static_cast<float>(biased * kUnitSteeringBlend - kUnitSteeringBlend);
--- include/bsp/unit_motion.hpp:282
-// expression is literally (s - bias) - blend * (s - bias).
+// expression is literally (s - bias) * blend - blend.
```
The same wrong formula is in the Ghidra plate comment of `00834E90` and in its record in
`config/names/00830000.jsonl`.

**W2. `009E7451` and `009E7463` in `009E6E80` ShipAi approach standoff, `src/ship_ai_approach_update.cpp:810-816`.**
Trace: q = c + c*c; `FLD 0.5`; DC C9 gives ST1 = 0.5q, ST0 = 0.5; the FXCH/FSTP float-stores h = 0.5q;
`FLD h`, `FLD pi` (`00CE3D28`), DC C9 gives ST1 = h*pi, ST0 = pi; FADDP gives h*pi + pi; FMULP by the
retained 0.5. Result **0.5 * (h*pi + pi)**, h = float(0.5 * (c + c*c)). Ghidra's pseudocode
agrees: `(pi + (c*c+c)*0.5*pi)*0.5`. The C++ computes (0.5q) * (q + pi*q).
```
--- src/ship_ai_approach_update.cpp
-            // 009E7441..009E7469: q = c + c*c, float-stored at 009E7455, then
-            // (0.5 * q) * (q + pi * q).
-            const float q = static_cast<float>(static_cast<double>(capped) +
-                                               static_cast<double>(capped) * capped);
-            side_weight = static_cast<float>(
-                (kApproachSideCurveScale * q) *
-                (static_cast<double>(q) + kApproachHalfTurn * q));
+            // 009E7441..009E7469: h = (c + c*c) * 0.5, float-stored at 009E7455
+            // (DC C9 leaves 0.5 in ST0); 009E7463 DC C9 gives h*pi with pi left
+            // in ST0, FADDP gives h*pi + pi, FMULP by the retained 0.5.
+            const float h = static_cast<float>(
+                (static_cast<double>(capped) + static_cast<double>(capped) * capped) *
+                kApproachSideCurveScale);
+            side_weight = static_cast<float>(
+                (static_cast<double>(h) * static_cast<double>(kApproachHalfTurn) +
+                 static_cast<double>(kApproachHalfTurn)) *
+                kApproachSideCurveScale);
```
This is a behaviour change on a gameplay path. cc9 should land it behind a switch, following its
pair practice.

**W3. `009C6875` in `009C62B0` dive-bomb fly-above, `src/dive_bomb_task.cpp:921-925`.** This one is
already known: cc13-audit9 found it (`docs/CC13_AUDIT_PLANE_BOT_STATES_3.md`), and GUNNERY 192 ported
the correct arm behind `kDiveFlyAboveCrossTrackBound` (`include/bsp/dive_bomb_task.hpp:238`). That
switch is OFF, so the default arm still computes sin*1.5*R where the image computes (0.5 sin + 0.5)*R.
The fix is cc9's switch flip.

**W4 (not a DC misread). `00934D42` in `00934150` unit part detach, `src/unit_parts.cpp:245`.**
The x87 sequence stores spin = (vz - 0*vy, 0*vx - 0*vz, 0*vy - vx) = **(vz, 0, -vx)**; the C++ has
`-velocity[2]` for x. The DC CA itself was read correctly.
```
-            float spin[3] = {-velocity[2], 0.0f, -velocity[0]};
+            float spin[3] = {velocity[2], 0.0f, -velocity[0]};
```
The verifier also notes, without checking it, that the C++ velocity at lines 230-240 omits a random
scaling the pseudocode applies before `00934D00`.

### cc13 task reconstructions

These cover kamikaze, rocket, depth charge, moveonpath, leg manager, homing and the opportunistic
rocket. Their own functions contain no DC FMUL. The helpers they call that do contain one:

- `00809E10` rocket flight time (`src/rocket_task_cc13.cpp:431`): CORRECT. The DC C9 at `00809E7D`
  leaves 0.5*v0 in ST1 and 0.5 in ST0 for 0.5*VMax^2. While checking it I fixed a rounding nit. The
  image float-rounds (VMax - v0)/a at `00809EB7` and keeps the cruise quotient unrounded; the C++
  had it the other way round. Build-tested only.
- `0080A000` rocket air step (`src/rocket_homing_cc13.cpp:84-89`): CORRECT. DC CB at `0080A130`
  computes vy - g*dt, then subtracts g*(-credit) using the retained g.
- `009FD570` fly-to solver (`src/plane_fly_to_solver.cpp:115`): CORRECT at `009FD5B7` (3.0 scales all
  three lead components). At `009FDDB9` (0.5*(min+max)) the site is NOT_COVERED, because every host
  passes `near_edge = false`.
- The other DC forms in these helpers (`007B43DA`, `007B5B8E`, `009F9F02` FADD; `009FBEB4` FSUB;
  `007F06EF` FADD in the near-field probe; `00438AC6`/`00438AF7` wrapped angle) print explicitly and
  are outside the defect. `00438AA0`/`00438B10` are also original-byte differential tested (CC11).
  One rounding nit, not a DC error: `near_field_probe.cpp:31` computes (w*|x|)/e, where
  `007F06E1-007F06EB` computes w*(|x|/e).

### Rounding and width nits found along the way (operands correct)

These are listed in the report records. `007BB6E0` (float instead of extended in `pilot_controls.cpp`
and `plane_flight.cpp`), `008206F0` (double instead of float-rounded intermediates), `0099E14C`
(0.3 vs `(double)0.3f`), `009CA4A0` (1.4 vs 1.399999976), `00A14CDB`/`00A14D03`, and
`post_effects_cc13.cpp:263` (double box centre).

## 4. Tool

`python tools/x87_dc_form_scan.py [--range LO-HI] [--forms dc,de,d8] [--ops fmul] [--by-function]
[--reconstructed] [--include-st0] [--context N] [--limit N] [--json PATH]` prints a one-line census
and is capped at `--limit` lines (default 60); `--json` writes every site with its Intel semantics,
capstone text, owner function, `past_end` flag and recon records. `--context N` prints the capstone
listing around each site, which is the quickest way to settle a `FMUL STi`. It runs from the PE on
disk and needs no Ghidra.

## 5. Status and open items

- Exported: nothing new. Reconstructed: no new functions. Build-tested: the one rounding change in
  `src/rocket_task_cc13.cpp`. Not game-validated.
- Open for cc9: W1, W2, W4 diffs; W3 switch. Also the stale formula in the `00834E90` Ghidra comment
  and in its ledger record (W1).
- Not checked by the verifiers: the branch condition at `src/ship_ai_approach_update.cpp:796`
  against Ghidra's `(iVar3 == 0 || +48Dh != 0)`, and the `trunc(d * 1.99)` comment at
  `src/hull_terrain_contact.cpp:1012` (`00C358C7`).
