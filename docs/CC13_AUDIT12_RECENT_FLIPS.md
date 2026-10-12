# Audit of cc9's recent ON flips: cc9's priority list, then the dive and strafe paths (cc13-audit12)

Addresses: 0084B380 0084B3F8 0084B405 0084B485 0078CF20 0078D1B0 00856420 00856530 00C4DE40 00C4DFBA 00C4E1F7
00C4E5C6 00C4E602 00C4E63B 00C4E655 00C4E671 00C4E67C 00C4E68A 00C4E6BE 00C4E74F 00C4E751 00C4E773 00C42622
00C4264F 00C4265B 00C42683 00C549D0 00C5756C 006DEF30 006DEF4C 006DEFA8 006DEFAD 006DEFC8 006DEFDD 00955630
00955752 0095576A 009557F8 0085A8B0 0085A9A0 009C4F80 009C5080 009C5090 009C50B7 009C5101 009C5144 009C5149 009C5163 009C5180 009C53D8 009C5400
009C5420 009C542C 009C5473 009C547E 009C5563 009C55B0 009C57C4 009C57D3 009C57E8 009C57F1 009C57FF 009C58D0 009C58DE
009C5DB8 009C5E63 009C5F4C 009C5F75 009C5F79 009C5F9E 009C5FA7 009C5FCB 009C6E10 009C64A6 009C64A8 009C6EB1 009C6F09
009C6F33 009C6F3B 009C6F4D 009C7AA4 009C7AAD 009CAC80 009CACDA 009CACDF 00419010 00419030 00438B10 0099D300 0099D369
0099E756 00928F50 00923B80

Packet `cc13-audit12`, branch `agent/cc13-audit12`, 2026-10-12. Requested by the cc13 lead for cc9. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files on this branch. Base: `origin/main` at `9f5120c95`.
The one diff below was applied in this tree, built with `./scripts/build.ps1` and reverted. Only this doc and
`reports/cc13_audit12_recent_flips.json` are committed.

**Selection.** cc9's priority list, as the lead relayed it, in order:
- P1 `kSubmergedTorpedoLaunchSwimBound`;
- P2 `kDynListingAssociationBound` and `kDynWorkerPrecisionBound`;
- P3 `kAaKind6GateBound`;
- P4 `kAimDiveBrakeRatioBound`, against cc13's earlier brake-floor finding;
- P5 the live-sea gunnery crossing.

Before that list arrived I had audited seven switches of my own choosing. They are kept as extra sections E1-E7.
E1 is the same switch as P4.

**Method.** I read each switch's ON path on `origin/main`. I compared it with the image listing (`bsp.py disasm-raw`
on disk bytes, `bsp.py ghidra disasm` for the stored listing) and followed every stack slot and x87 stack entry from
its write to its read. Landing evidence is GUNNERY_OPEN_ITEMS 178.1, 181, 182, 183, 184 and 193 and SHIP_AI 213,
read by line range only. For P2, the per-thread control words come from `docs/CC13_FPU_PRECISION.md` and the
thread of each phase from `docs/CC13_HOST_PRECISION_CENSUS.md`.

**Checklist.**
- Rule 2 (boundary before attribution): `tools/verify_report_calls.py reports/cc13_audit12_recent_flips.json`
  checked every call row with 0 failures (the count is in the commit message).
- x87 register forms: `tools/x87_dc_form_scan.py --include-st0` over the dive ranges:
  - `009C4F80-009C5177` and `009C5DB8-009C6080` have no DC-form sites;
  - `009C6E10-009C6F91` has one, `009C6F33 dc c0 FADD ST0,ST0`, which is the term `kDiveFlyAboveTermsBound` binds
    (E5);
  - `009C5180-009C580A` has three `FMUL ST0,ST0` squares upstream of the audited arms (`009C528D/52CF/5311`).

  For `00C4DE40` and `00C42530`, I walked the x87 stack by hand over the chains named in P2.
- Callee-pop widths: `00419010` `RET 14h`, `00438B10` `RET 8`, `006DEF30` `RET 4`. Every slot offset below was
  rebased with them.
- Operand widths: every constant the switched paths load was read from the image bytes. For example `00D7A270`,
  the 0.05 that `00C4E67C` loads with `FADD qword`, and the dive constants in E5.

**Status vocabulary.** Every switch audited is reconstructed and build-tested on main, and cc9 game-validated it
with pairs. This audit is a static second reading and adds no runs (RUN SLOTS rule). The P3 diff is build-tested
only.

## Verdicts

| # | switch | verdict | one line |
| --- | --- | --- | --- |
| P1 | `kSubmergedTorpedoLaunchSwimBound` (GUNNERY 182) | FAITHFUL; superseded on main by P5 (P1a) | `0084B380` tests only the segment END (`0084B3FD-0084B405`), and `00856420` makes no medium choice at spawn. With `kGunneryLiveWaterSurfaceBound` ON, the live trace replaces `crossed`, so this term now moves only a counter |
| P2 | `kDynWorkerPrecisionBound` + `kDynListingAssociationBound` (GUNNERY 183) | FAITHFUL on the chains read | Dyn tasks run only on 53-bit workers. The host's scope covers the narrow phase and the LCP and leaves `ManifoldUpdate` (main thread, 24-bit) outside. The spill points match `00C4DE40` and `00C42530` |
| P3 | `kAaKind6GateBound` (GUNNERY 184) | gate order and refusal FAITHFUL; **DEFECT in the angles** (P3a) | `00955630` carries the gravity-lifted world direction into the mount frame. The host adds the world lift to the unlifted hull-frame elevation and keeps the unlifted bearing. On a rolling hull the bearing is off by up to about 2.4 deg. Diff P3 |
| P4 | `kAimDiveBrakeRatioBound` (GUNNERY 181.2) | FAITHFUL; it is exactly cc13's fix | This is `docs/CC13_AUDIT_PLANE_BOT_STATES.md` row 6, diff 6. The host comment cites "cc13 plane-bot audit row 6", and the read is E1 |
| P5 | `kGunneryLiveWaterSurfaceBound` (the crossing wiring) | FAITHFUL | `0084B380` samples `0078CF20(end.x, end.y)` (the image's axis defect) and compares `end.y <= h`. The crossing is `0078D1B0(from, to)`. The host's `static_trace_0084b380` matches all three, and runs only in medium 0 |
| E1 | `kAimDiveBrakeRatioBound` (GUNNERY 181.2) | FAITHFUL | `009C5F79 FSTP [ESP+38h]` under `SUB ESP,14h` is the frame's `[ESP+24h]`. `00419010` pops 14h, so `009C5FA7 FLD [ESP+24h]` reloads `AimPitchRatio x |pitch|`, as the host passes |
| E2 | `kDiveAimHeadingBlendBound` (GUNNERY 178.1) | FAITHFUL | `009C5090` gate, the `+pi` correction, `SubtractWrapped(B,E)`, `IC(-60 deg,0,-40 deg,1,pitch)`, `FMUL d` and `SubtractWrapped(B,t*d)` all match, slot for slot and argument for argument |
| E3 | `kDiveYawModeZeroBound` (GUNNERY 189 B / 193) | FAITHFUL | `009C5420` stores the immediate 0 and `009C5E63` stores EBX, which is 0 from `009C58DE XOR EBX,EBX`. The host sets the flag before `0099E756`'s read in the same think and clears it there |
| E4 | `kDiveAimGlideArmsBound` (189 A, C / 193) | FAITHFUL | Bank scale: `IC(100,1,250,1.5,[ESP+1Ch])` on the heading arm only (`009C542C`). Arm B: `0.6 > ratio`, `0 > pitch`, `p2 > p1`, with `p1 = [ESP+6Ch]` from `009C5563` and `p2 = [ESP+28h]` from `009C55B0`, neither rewritten before `009C57F1` |
| E5 | `kDiveFlyAboveTermsBound` (189 C2, D, E / 193) | FAITHFUL; one pre-existing gap noted (E5a) | `C = ctl+398h` (`009C64A8`) equals `approach+ACh` this think (`009C7AA7-7AAD`). The numerator is `FADD ST0,ST0` of the error (`009C6F33`). `2E8h = 1.5` with `+18h` clear |
| E6 | `kStrafeAttackrunSeedDrawBound` (190 / 193) | FAITHFUL | `009CACDA` draws `U(0,1)` with `ECX = 1`, and `009CACDF FCHS` then `FSTP +1Ch`. The host's keyed draw follows the host-wide convention for 00BD2F10 |
| E7 | `kPartyChangeRefreshesGunneryBound` (SHIP_AI 213) | FAITHFUL (host consolidation) | The image has one field, written by `00923B80` (`00928F7D`). The gunnery host's copy is the only split copy, and every other host reads `unit_side_0054` live |

The one defect diff (P3) is OFF and reverted here. It was built with `./scripts/build.ps1` and checked with
`git apply --check` against `origin/main`. It is listed in the Diffs section, after E7.

## P1. `kSubmergedTorpedoLaunchSwimBound` (GUNNERY 182)

**Host** (`src/game_hosts_gunnery.cpp` ~9200):
- `crossed = end.y <= 0 && (from.y > 0 || (ON && medium 0))`, where medium 0 is `!swimming && !depth_charge_in_water`;
- ON also counts `submerged_launch_entries`.

**Image** (`disasm-raw 0084B380`, `BSP_Projectile_TraceWaterSurfaceEntry`). Each 18h-byte segment record is
`from` at `-14h..-0Ch` and `to` at `-8..0` from ESI.
- `0084B3A1..0084B3F2` push `(to.x, to.y)` and call `0078CF20`.
- `0084B3FD FLD [ESP+10h]` (`to.y`), `FCOMIP` against the height, then `0084B405 JBE` to the hit. A segment hits
  when its END is at or below the sampled surface.
- The start is never read for the test. It is read only as `0078D1B0`'s first vector at `0084B485`.

So a round in medium 0 that starts below the surface enters on its first step, which is what ON allows.

**The spawn-time medium.**
- `00856420` (`BSP_TorpedoClass_CreateProjectile`) has ten calls: nine direct ones (`operator_new`, `_memset`,
  `00856050`, `0085DC80`, `0042ED50`, `006952A0` x2, `00694A60`, `00414DB0`) and one indirect one, `00856530
  CALL EDX`. That indirect call is `vtable[98h]` (`008564AB`), not `vtable[24h]`/`[28h]`.
- There is no `0078CF20` call.

So a torpedo is created in medium 0, as GUNNERY 182 says.

**P1a. Superseded on main.** Since `ecb1c55d1`, `kGunneryLiveWaterSurfaceBound` is ON. In medium 0 its block
recomputes `crossed = static_trace_0084b380(...).hit`, an end-only test against the live surface, and in the water
it sets `crossed = false`. Either way it overwrites the value this switch shapes. On main the switch therefore
changes only the `submerged_launch_entries` counter, which counts flat-surface (`y = 0`) crossings rather than the
live ones. This is a diagnostic inaccuracy, not a gameplay one. It matters for behaviour only if the live surface is
switched OFF again.

**Verdict: FAITHFUL.**

## P2. `kDynWorkerPrecisionBound` and `kDynListingAssociationBound` (GUNNERY 183)

**The precision control word.**
- `docs/CC13_FPU_PRECISION.md` section 3: the Dyn tasks run only on the `_beginthreadex` Dyn workers, whose
  `_FPmtinit` is a `RET`, so they run at `027Fh` (53-bit). The submitting thread waits (`00C33140`).
- `ManifoldUpdate` `00C549D0` is not a task. It is called directly from `Dyn_Scene_RunCollisionPass` at
  `00C5756C`, on the main thread at 24 bits (CC13_HOST_PRECISION_CENSUS row 24).
- The host's `DynWorkerPrecisionScope` (`src/hull_terrain_contact.cpp:33`) saves the word with
  `_controlfp_s(&saved,0,0)`, sets `_PC_53` under `_MCW_PC`, and restores `saved & _MCW_PC` on scope exit (RAII, so
  every return path restores it).
- In `HullTerrainContactSolver::step` the scope opens after the `refresh_native_dyn_manifold_00c4b9b0` loop, at
  the comment "From here on the image is on a Dyn worker". The manifold refresh therefore stays at the caller's
  24 bits, which matches the image's thread split.
- The world-step entry splits the span in two: the narrow phase in one scope (line 1491, `IntersectTask2`
  `00403CC0`) and each group's solve in another (line 1627, `LCPSolverTask` `00403720`). Group formation
  (`00C4B610`) runs between them at 24 bits, which matches the census's main-thread phases (broad phase,
  `ManifoldUpdate`, group formation, contact report).
- `_MCW_PC` touches only the x87 word. The build is `/fp:strict` with SSE2 (`CMakeLists.txt:223`), so the C++
  `double` template arm (`R = double`) is IEEE double. It equals the x87 at PC 53 except at the extended exponent
  range, which these magnitudes never reach.

**The association** (`build_contact_rows_listing<R>`, `src/dyn_lcp_impulse_math.cpp:72`). I checked these chains
against the disk bytes, following the x87 stack entry by entry:

| chain | listing | spills | host |
| --- | --- | --- | --- |
| `to_world`, x | `00C4DFBA..00C4DFE6`: `((m14*L1 + m8*L0) + m20*L2) + pos`, one `FSTP` | 1 | `((row0*l0 + row1*l1) + row2*l2) + pos`, one `sp()`: the same sum (the first `+` is commutative) |
| relative velocity | `00C4E568..00C4E63B`: `+vb` spilled (`110h..118h`), `-va` spilled (`B4h..BCh`), `-w x ra` spilled (`68h..70h`) | 3 per axis | three float operations, each rounding |
| normal velocity | `00C4E655..00C4E671`: `(r0*n0 + r1*n1) + r2*n2`, one spill | 1 | `sp((R(r0)*n0 + R(r1)*n1) + R(r2)*n2)` |
| bounce | `00C4E675 FLD nv` / `FMUL [eax+4]` (float) / `00C4E67C FADD qword [00D7A270]` / `FSTP` float | 1 | `float(double(R(nv)*rest) + 0.05)`; at R = double the product is exact, so a single rounding to 53 bits and then to float, as the listing |
| target clamp | `00C4E68A FLDZ` / `FCOMIP` (ST0 = 0) / `JBE` takes XMM1 (0) | - | `if (!(target < 0)) target = 0`; NaN goes to 0 on both |
| rhs velocity | `00C4E6B2 FLD` / `FCHS` keeps `-target` on the stack. Four group dots are each spilled (`00C4E6E0`, `00C4E700`, `00C4E721`, `00C4E745`) and summed on the stack, `((G1 + G0) + G2) + G3`. `00C4E74F FSUBRP` gives `-target - sum`, and `00C4E751` spills | 4 + 1 | `sp(R(-0 - target) - (((R(G1) + G0) + G2) + G3))` |
| effective mass | `00C4E773..00C4E7AE`: `dot3(j+3, m+3)` spilled, `dot3(j, m)` spilled, `FADD` on the stack | per group | `R(1) / (((R(G1') + G0') + G2') + G3')` |
| LCP residual | `00C42622..00C4265B`: the group dot is spilled at `00C4264F`, then `acc - G` is spilled at `00C4265B`. The same holds at `00C42683` / `00C42687` and onwards | every op | `dyn_row_residual`: float `acc` minus each spilled `dyn_row_group_dot` |

Two details of the stack walk:
- The `FLD ST0 / FADDP ST2 / FXCH / FSTP` idiom at `00C4E568..00C4E5CA` leaves one velocity component on the x87
  stack each time. These are `vb0..vb2` and `va0`, and the rhs chain consumes them later (`00C4E6F1 FMULP ST4`,
  `00C4E70D FMULP ST4`, `00C4E712 FMULP ST5`, `00C4E71B FMULP ST2`). They are loads of float spills, so they are
  exact, and the host's reload of the same floats is equivalent.
- The OFF-precision arm (`R = float`) rounds the bounce twice: first to double, then to float. The image at 24 bits
  rounds once. A tie could differ by 1 ulp. Both switches are ON together, so this arm is not live.

**Evidence.** GUNNERY 183's pairs moved the hull-terrain rows as predicted. I did not re-read them beyond the
section header.

**Verdict: FAITHFUL on every chain read.** I did not walk all of the friction-direction chain (`00C4E828..00C4EF99`)
step by step. Its host form uses the same `dot3` / `jacobian_dot` / `effective_mass` lambdas, which do match.

## P3. `kAaKind6GateBound` (GUNNERY 184)

**Gate order and refusal: FAITHFUL.** From `006DEF30` (stored listing, `RET 4`):
- `006DEF36` and `006DEF44..006DEF50` refuse when the target, `[bot+50h]` or `[[bot+50h]+3Ch]` is null. The host
  takes the last as present, which is labelled.
- `006DEFA8 CALL 00955630` with `ECX = [[bot+50h]+3Ch]`, `EDX = target+FCh` and stack `(gun+FCh, speed
  [[[gun+3F8h]+34h]+50h], &[E+18h], &[E+10h])`, where E is ESP after the four pushes. `006DEFAD TEST AL` refuses on
  0. The host's `reach = arc.solved` matches.
- `006DEFB3..006DEFC8` call `0085A8B0([E+10h], [E+18h])`, that is (horz, vert), and refuse on 0. The host's
  `gun_traverse_allowed_007f5fc0(arcs, wrap(horz), wrap(vert))` matches.
- Only then does `006DEFDD` call `vtable[1D4h](target)` = `0072F6E0`, the line of fire. The host makes its
  line-of-fire query and expiry draw only past both gates, and a refusal becomes a window refusal.
- The plane arm goes to the window `0085A9A0` and then the line of fire, as kind 5.
- No RNG draw sits in either gate. The only draw is the line-of-fire expiry inside `0072F6E0`, and both builds
  reach it on the same condition.

**P3a. DEFECT: the angles handed to `0085A8B0`.**
- `00955630` (`src/gun_gravity_arc.cpp` 200-252, the image's own walk) builds the solved world direction
  `004B4D80(pitch, yaw)` at `00955752..00955765`.
- With a mount, it then transforms that vector into the mount frame (`0095576A..009557F3`: `0042D0D0`, `asin`
  of local y, `atan2(local x, local z)`) and negates the yaw at `009557F8`.
- The mount is `[gun+3Ch]`, the gun's parent entity. It is oriented as the hull: `docs/AA_FIRE_WINDOW_MOUNT.md`
  section 2, 336 slot frames on USN02 and USN04 with `forward = +z` in model space.
- So the image's (horz, vert) are the hull-frame bearing and elevation of the gravity-lifted direction.

The host asks the solve with `mount_frame = nullptr`. It then sets:
- `horz` to the hull-frame bearing of the unlifted line of sight `u`;
- `vert = asin(u . up) + (arc.vert - world elevation of u)`, the world lift added to the hull-frame elevation.

Both are exact for a level hull. On a heeled or pitched hull, lifting the line in the world vertical moves its
hull-frame bearing. The live sea has been ON since `ecb1c55d1`, so every hull rolls. Worked numbers
(`local/a12_frame.py`, a hull rolled by phi about its forward axis, target bearing b, lift L):

| phi | b | L | horz error | vert error |
| --- | --- | --- | --- | --- |
| 3 deg | 60 deg | 10 deg | 0.26 deg | 0.00 deg |
| 5 deg | 45 deg | 15 deg | 0.93 deg | -0.03 deg |
| 8 deg | 45 deg | 20 deg | 2.00 deg | -0.10 deg |
| 5 deg | 30 deg | 30 deg | 2.44 deg | -0.09 deg |

So a target within about 1-2 deg of a mount's traverse limit is accepted or refused on the wrong side. That changes
which surface targets a dual-purpose mount keeps, and so when the line-of-fire cache and its expiry draw are first
reached.
- **Which missions it moves:** every row with kind-6 mounts that see ships and a non-zero sea state. GUNNERY 184.3
  counted 15-1576 `kind6_gate_rejects` per row on USN01, USN02 and USN13. USN04 E2 has no surface contact.
- **What it does not touch:** the plane arm and kinds 1 and 5.
- **Remaining labels:** the fix takes unit scale for the mount (`docs/MOUNT_FRAME_SCALE.md`: the image does not
  normalise the transformed direction). It also takes the hull's axes for `[gun+3Ch]`, as the fire window does.

**The fix** (diff P3, `kAaKind6GateMountFrameBound`, OFF):
1. Rebuild the world direction from the solve's own angles. With mount NULL these are `(pitch, -0 - yaw)`, and
   `004B4D80` gives `(sin y cos p, sin p, cos y cos p)`.
2. Project it on the hull's `right` / `up` / `forward`, with the existing `kGunHorzSign` for the image's negate.

**Verdict: the gate is faithful; the angle pair is a DEFECT (small, and live on every rolling hull).**

Two things I left unresolved:
- In the host, a non-plane kind-6 slot with `row.arcs.empty()` or `len <= 0` returns without refusing and without a
  line-of-fire query, "as kind 5 below". The image has no such early return in `006DEF30`, and `0085A8B0` with an
  empty arc list was not read here. UNCERTAIN, and probably unreachable (every dual-purpose mount has arcs).
- `[[bot+50h]+3Ch] == 0` refuses in the image (`006DEF50`). The host never refuses on it. This is labelled by cc9.

## P4. `kAimDiveBrakeRatioBound` and cc13's brake-floor finding

`docs/CC13_AUDIT_PLANE_BOT_STATES.md` row 6 ("aimdive tail, air brake `009C5FA7`: DEFECT ... the brake floor's x is
`AimPitchRatio x |pitch|` (slot `[ESP+24h]`, overwritten at `009C5F79`), not `|pitch|`", diff 6) is the finding.
`kAimDiveBrakeRatioBound` binds it. The switch's comment in `src/dive_bomb_aimdive_tail.cpp` reads "GUNNERY 181.2
(cc13 plane-bot audit row 6): 009C5F79's FSTP float writes AimPitchRatio x |pitch| over [ESP+24h]". ON passes
`x1`, which is that diff's change. The independent re-read of the bytes is E1: FAITHFUL.

## P5. The live-sea gunnery crossing (`kGunneryLiveWaterSurfaceBound`)

**Prior audits.** `docs/CC13_AUDIT_CC9_RECENT.md` item 7 and `docs/CC13_AUDIT_CC9_RECENT_2.md` item 1 covered the
wave-damp region and the amplitude side of `kWaveFieldLiveAmplitudeBound`, so I skipped them. No CC13 audit had
read the gunnery crossing wiring, so I read only that.

**Image.** `0084B380`, read in P1:
- The sample is `0078CF20(to.x, to.y)`. These are the end point's first two components, the image's axis defect
  (`docs/PROJECTILE_HELPERS.md` 1).
- The test is `to.y <= h`.
- On a hit, the two vectors pushed at `0084B42A..0084B474` are `to` first (the higher address) and `from` last, so
  the call is `0078D1B0(from, to)`. The result is stored to the record's `+0/+4/+8` with `+0Ch = 1`
  (`0084B48A..0084B496`).
- On a miss, `0084B41D` clears `+0Ch` and leaves the point untouched.

**Host.**
- `static_trace_0084b380` (`src/projectile_helpers.cpp`) = `walk_segments(..., true)`. It samples
  `water_height_0078cf20(to.x, to.y)`, tests `to.y <= surface`, and computes the crossing with
  `water_crossing_point_0078d1b0(from, to)`. On a miss the point is not written.
- The gunnery wiring (~9205) runs the trace only in medium 0 (`!swimming && !depth_charge_in_water`), the image's
  `0084C11C` choice. Its sampler is `ocean_live_field_water_height_0078cf20`. On a hit it replaces the segment end
  in all three host copies of the position (`0084C15F..0084C16B`). In the water it sets `crossed = false`.
- The crossing draws nothing from the RNG.

**Verdict: FAITHFUL.** P1a records the one interaction (the swim switch is now only a counter).

# Extra sections (my own selection, before cc9's list)

## E1. `kAimDiveBrakeRatioBound` (GUNNERY 181.2)

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

## E2. `kDiveAimHeadingBlendBound` (GUNNERY 178.1)

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

## E3. `kDiveYawModeZeroBound` (GUNNERY 189 B, ON in 193)

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

## E4. `kDiveAimGlideArmsBound` (189 A and C, ON in 193)

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

## E5. `kDiveFlyAboveTermsBound` (189 C2, D and E, ON in 193)

**5.1 C (`009C64A6-009C64C9`).** With a control block, `C = [EAX+398h]` (`009C64A8`). Without one,
`C = approach+ACh + approach+50h`. `009C7A80` (`009C7AA4-009C7AAD`) runs every think before the flyabove tick and
copies `[approach+0Ch]+398h` to `approach+ACh`, so ON's `cruise_altitude_398 = db_begin_alt_ac` is the right value
whenever the host's `db_begin_alt_ac` follows `ctl+398h`.

**E5a. Gap noted, pre-existing.** The host updates `db_begin_alt_ac` from the squadron block (`game_hosts_units.cpp`
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

**Verdict: FAITHFUL**, with E5a as an inherited gap.

## E6. `kStrafeAttackrunSeedDrawBound` (190, ON in 193)

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

## E7. `kPartyChangeRefreshesGunneryBound` (SHIP_AI 213)

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

## Diffs

### Diff P3: `kAaKind6GateMountFrameBound` (OFF)

Applied in this tree, built with `./scripts/build.ps1` (pass), reverted. `git apply --check` against `origin/main`
is clean.

```diff
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index 09d6fa565..34e97df76 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -617,6 +617,15 @@ constexpr bool kTailGunnerNoArmourTestBound = true;   // ON: GUNNERY_OPEN_ITEMS
 //    and a gate refusal refuses the slot (as a window refusal). OFF: kind 6
 //    goes straight to the line-of-fire query (the host before section 184).
 constexpr bool kAaKind6GateBound = true;   // ON: GUNNERY_OPEN_ITEMS 184.3
+//  * kAaKind6GateMountFrameBound (cc13-audit12 section P3): 00955630 builds the
+//    solved WORLD direction (004B4D80 of its pitch and yaw, 00955752..00955765) and
+//    carries that vector into the mount frame [[bot+50h]+3Ch] (0095576A..009557F3:
+//    0042D0D0, then asin of local y and atan2(local x, local z), negated at
+//    009557F8). [gun+3Ch] is oriented as the hull (docs/AA_FIRE_WINDOW_MOUNT.md 2);
+//    unit scale is taken (docs/MOUNT_FRAME_SCALE.md). ON: horz and vert are the
+//    lifted direction's hull-frame pair. OFF: the unlifted hull-frame bearing, and
+//    the world lift added to the hull-frame elevation (184.1's substitution).
+constexpr bool kAaKind6GateMountFrameBound = false;
 //  * kLandscapeSpatialAttachBound: packet cc9_landscape_spatial_attach
 //    (docs/SCENE_CONTENTS_HOSTS.md section 8). Each Landscape joins the segment
 //    query as a loose entry after the units (00884078 attaches its collision
@@ -5688,9 +5697,18 @@ public:
                 if (reach) {
                     const float u[3] = {d[0] / len, d[1] / len, d[2] / len};
                     const float direct = std::atan2(d[1], std::sqrt(d[0] * d[0] + d[2] * d[2]));
-                    const float horz = kGunHorzSign * std::atan2(dot3(u, right), dot3(u, forward));
-                    const float vert = std::asin(std::max(-1.0f, std::min(1.0f, dot3(u, up))))
+                    float horz = kGunHorzSign * std::atan2(dot3(u, right), dot3(u, forward));
+                    float vert = std::asin(std::max(-1.0f, std::min(1.0f, dot3(u, up))))
                         + (arc.angles.vert - direct);
+                    if constexpr (kAaKind6GateMountFrameBound) {
+                        // The mount-NULL solve returns (pitch, -0 - yaw) in the world frame.
+                        const float p = arc.angles.vert;
+                        const float y = -arc.angles.horz;
+                        const float w[3] = {std::sin(y) * std::cos(p), std::sin(p),
+                            std::cos(y) * std::cos(p)};
+                        horz = kGunHorzSign * std::atan2(dot3(w, right), dot3(w, forward));
+                        vert = std::asin(std::max(-1.0f, std::min(1.0f, dot3(w, up))));
+                    }
                     const bsp::GunPlatformArcs arcs{row.arcs.data(), row.arcs.size()};
                     reach = bsp::gun_traverse_allowed_007f5fc0(arcs,           // 0085A8B0
                         bsp::gun_wrap_angle_0085abbd(horz), bsp::gun_wrap_angle_0085abbd(vert));
```

## Summary for cc9

- P1, P2, P4 and P5 are faithful.
- P1a: on main, `kSubmergedTorpedoLaunchSwimBound` changes only the `submerged_launch_entries` counter, because the
  live surface trace overwrites `crossed`. The counter counts flat-surface crossings. This is diagnostic only.
- P3a is the one DEFECT. The kind-6 surface gate hands `0085A8B0` the world lift added to the hull-frame elevation,
  plus the unlifted bearing. The image uses the lifted direction's hull-frame pair. On a rolling hull the bearing
  is wrong by up to about 2.4 deg, so targets within about 1-2 deg of a traverse limit flip. To validate it, pair
  diff P3 on USN02 and USN13, the rows with the most `kind6_gate_rejects` in 184.3. The prediction is that
  `kind6_gate_rejects` moves in both directions while the plane arm stays identical.
- E1-E7 (my own selection) are faithful. E5a, the unmodelled `009C899B` profile draw behind an unfed squadron's
  `ctl+398h`, is inherited and predates the switch.
