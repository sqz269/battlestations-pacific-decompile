# cc13 leaf fixture, pass 2 (packet cc13-fixture2)

Pass 1 is `docs/CC13_FIXTURE_PASS_1.md`. This pass adds 32 leaf routines to the same harness. The
harness is `tools/verify_cc13_leaf_reference.py` (the CODE and DATA tables) and
`tests/cc13_leaf_fixture_tests.cpp`, run by CTest as `cc13_leaf_differential`. Each routine's
original bytes run beside its reconstruction and the outputs are compared bit for bit.

## Selection

The candidates are the `reconstructed_build_tested` and `*_documented_deviation` function rows
whose bodies are pure leaves. A pure leaf has:

- no calls, except to `00BF7030` sqrt (bound to `FSQRT; RET`) or to a routine already in CODE;
- no FS-segment use, indirect jump or branch out of the body;
- no reads of writable globals, no global writes, and no address immediates outside `.text`.

That gave 1735 candidates. 274 of them use x87 and one indexes a table. They were ranked with
trig, FYL2X/F2XM1, FISTP, FMUL/FDIV and sqrt first, and this pass took the top x87, sqrt and
table routines whose C++ interface can be called directly. The scan script was a one-off and is
not kept.

## Inputs and precision

Each routine runs on a fixed pseudo-random input set. About one draw in six is replaced by an
edge value: +/-0, +/-1, 0.5, a quiet NaN, +/-1e-40 (denormal), +/-FLT_MIN, +/-1e30, +/-inf or
FLT_MAX. The cases also seed branch boundaries:

- degenerate and parallel inputs;
- equal points;
- a point on a vertex and on an edge;
- determinant 0;
- s at 0, 0.01 and so on up to 1;
- the -1000 height sentinel;
- every vertex type index.

Every case runs twice on the same inputs, once at 53-bit and once at 24-bit x87 precision
control.

The deciding precision is the precision of the thread the image runs the routine on
(`docs/CC13_FPU_PRECISION.md`):

- The main thread runs at 24-bit after device creation. This covers the frame, the HUD, Lua and
  the frontend.
- The job-pool and Dyn worker threads run at 53-bit.
- A routine reached from worker threads only is judged at 53-bit. Only 00C37860 is: the Dyn
  solver 00C410E0 and 00C4D050, plus the undefined Dyn chunk at 00C41070.
- Every other routine is judged at 24-bit. That includes routines the fixed-step pool can reach
  from either thread, and routines with no reference in the image. For those, the rel32 and
  absolute scans are empty, and they are judged by their family.

The other pass is printed for information only. The first version of this pass judged each
routine at "the precision the reconstruction models" instead. rev9 failed that rule as circular,
and the follow-up below replaced it.

## The 24-bit model

`include/bsp/cc13_x87_model.hpp` models 24-bit precision control.

Arithmetic:

- Under 24-bit control, FADD, FSUB, FMUL, FDIV and FSQRT round each result once to a 24-bit
  significand. The exponent keeps the extended range.
- `x87_add24`, `x87_sub24`, `x87_mul24` and `x87_div24` compute the exact operation in double
  and round it once to 24 bits. The exact error (from TwoSum, an FMA product residual or a
  division remainder) breaks ties, so there is no double rounding.
- The FSTP DWORD that follows is a `static_cast<float>`. It changes the value only on overflow,
  underflow and denormal results, as on the x87.
- Plain float arithmetic is not a substitute. It overflows where the stack does not, and it
  rounds a denormal product once where the x87 rounds twice.

Transcendentals:

- FYL2X, F2XM1, FSCALE, FRNDINT, FSIN and FCOS ignore precision control.
- The power tail FRNDINT / FSUB / F2XM1 / FLD1 / FADDP / FSCALE rounds its FSUB and FADDP to
  24 bits. `x87_pow2_tail24` models that.
- The CRT's x87 `log2`, `expm1`, `sin` and `cos` lose precision under 24-bit control.
  `x87_fyl2x`, `x87_pow2_tail24`, `x87_sin` and `x87_cos` therefore call them under
  `X87Precision53`, which sets 53-bit for the call and restores the caller's control word.
- A probe confirmed the power tail. A 24-bit FYL2X operand gave 7164 mismatches in 20000.
  The extended FYL2X result read as a double gave none.

FSIN and FCOS (`x87_sin`, `x87_cos`; N1 narrowed):

- An operand of magnitude 2^63 or more is returned unchanged, an infinity becomes the default
  NaN, and a NaN passes through.
- Inside that range the claim is only that the CRT value, rounded to float, matched for the
  fixture's inputs: |x| <= 7 and the edge values.
- FSIN reduces with a 66-bit pi, so a large finite operand below 2^63 (1e5, 1e15) can differ.
  This pass does not claim those operands.
- `post_effects_cc13.cpp` and `seg6_cc13.cpp` now share this one copy (N3).

## Results (the deciding precision, edge-value input set)

| Routine | Thread | 24-bit | 53-bit | Outcome |
| --- | --- | --- | --- | --- |
| 00467050 | main (HUD free camera, scene database, plane control) | 2000/2000 | 813/2000 | remodelled, stays promoted |
| 004E5C00 | main (frontend tactical library, scene readers) | 3000/3000 | 2712/3000 | remodelled, stays promoted |
| 00452B00 | main (HUD unit view) | 3000/3000 | 2926/3000 | remodelled, stays promoted |
| 00B62A10 | main (GUI ortho, shadow fits) | 2000/2000 | 1260/2000 | ON arm `ortho_off_center_lh_pc24`; live arm behind OFF switch; `reconstructed_build_tested_off_switch` |
| 0046F7B0 | unreferenced (scene geometry) | 2000/2000 | 1289/2000 | remodelled, stays promoted |
| 0046F550 | main (0046F940 from 007AE930) | 2000/2000 | 1284/2000 | remodelled, stays promoted |
| 0046F6D0 | unreferenced (scene geometry) | 2000/2000 | 1244/2000 | remodelled, stays promoted |
| 00AFB7F0 | unreferenced (particles) | 3000/3000 | 2900/3000 | remodelled, stays promoted |
| 00AFBA40 | unreferenced (particles) | 3000/3000 | 2913/3000 | remodelled, stays promoted |
| 00793730 | main (MovCam) | 3000/3000 | 2763/3000 | remodelled, stays promoted |
| 00793800 | main (MovCam) | 3000/3000 | 2777/3000 | remodelled, stays promoted |
| 007BB4C0 | unreferenced (segment 47 geometry) | 3000/3000 | 2880/3000 | remodelled, stays promoted |
| 00B75580 | main (animation tracks) | 2999/3000 (+1 NaN-only) | 1746/3000 (+1) | remodelled; one NaN-sign case, reverted to `reconstructed_build_tested` |
| 00B75520 | main (animation sampler) | 3000/3000 | 1516/3000 | remodelled, stays promoted |
| 00BBF280 | main (sky gradient, fog colour) | 3000/3000 | 2999/3000 | remodelled, stays promoted |
| 00C37860 | Dyn workers only | 1305/3000 | 3000/3000 | judged at 53-bit, stays promoted |
| 00428090 | not worker-only (projectile trace) | 3000/3000 | 3000/3000 | remodelled at 24-bit, promoted |
| 00B64470 | main (HUD map icon, GUI model) | 1993/2000 (+7 NaN-only) | 804/2000 (+7) | remodelled, NaN-only, not promoted |
| 00B62DD0 | unreferenced (camera block) | 1946/2000 (+54 NaN-only) | 1524/2000 (+54) | remodelled, NaN-only, not promoted |
| 00941AA0 | main (Lua spawn placement) | 2990/3000 (+10 NaN-only) | 2950/3000 (+10) | remodelled, NaN-only, not promoted |
| 00A8B860 | render (shadow homography) | 2999/3000 (+1 NaN-only) | 1314/3000 (+1) | remodelled, NaN-only, not promoted |
| 004F3DA0 | main | 3943/4000 | 3927/4000 | documented exponent-range deviation, not promoted |

Unchanged and bit-exact at both precisions, staying promoted: 00B65560, 00B8C520, 00ADFA90,
00ADC580, 009064D0, 00B8B940, 00B478E0 and 00AED820. 009D7780 and 009D7930 are bit-exact at
24-bit and stay promoted. Each row's ledger evidence records its thread, both results and its
outcome (N2).

Without edge values every deciding pass matches outright, the NaN-only rows and 004F3DA0
included.

NaN-only means that both sides are NaN and only the sign or payload differs. The x87 propagates
NaNs by its own rules (an input NaN, or the larger significand of two), while the SSE code
produces or propagates a different one.

004F3DA0 keeps its documented exponent-range deviation:

- Its 24-bit model does the intermediate arithmetic in plain float, so an operand near 1e30
  overflows to infinity.
- On the x87 stack the same value is still in range, so the boolean result flips.
- The `x87_*24` helpers would remove the deviation. It was left as instructed.

### Pinned counts (F3)

The test pins the deciding pass of each documented finding, using the default input set:

- 004F3DA0: 57 mismatches.
- NaN-only: 00B64470 7, 00B62DD0 54, 00941AA0 10, 00A8B860 1, 00B75580 1.

A deciding pass fails when its mismatch or NaN-only count differs from the pinned count, which
is 0 for an unlisted routine. A new mismatch and a newly fixed one both show up. With
`BSP_FIXTURE_NO_EDGES` set, every deciding pass must match outright.

### Live code

`ortho_off_center_lh` is live: `game_native_gui_render_application.cpp`
`ortho_off_center_00b62a10` uses it in the native GUI pass.

- It keeps the earlier double model while `kOrthoOffCenterPc24Cc13Bound` (in
  `include/bsp/post_effects_cc13.hpp`) is OFF.
- `tools/off_switch_on_build.py --names kOrthoOffCenterPc24Cc13Bound` built the ON arm cleanly.
- cc9 owns the flip.

Every other routine changed here is unwired. Their only callers are cc13 host virtuals that
nothing implements, or cc13 code that no host reaches.

## Fixture-found fixes

- 00B64470 (`compose_euler_scale_translation`):
  - Before multiplying by the scale, the image stores pair products to float: c2*c0, -c2*s0,
    s2*s0 and -s2*c0, then c1*s2, c1*c2, c1*s0 and c1*c0.
  - Only 159 of 2000 cases matched before the fix.
  - The remaining chains are now per-operation 24-bit.
- 00B75580 (`bezier_00b75580`): the result is rounded to float (`FSTP DWORD` / `FLD` at
  00B755F7) before the return. 2 of 3000 cases matched before.
- 00941AA0 (`spawn_ray_sphere_00941aa0`):
  - The projection and the squared miss are summed in the order x, y, z on the x87 stack.
  - An unordered (NaN) compare intersects, because `JB` is taken on unordered.
  - The "53-bit control word" note is gone. The model is now 24-bit.
- 00B64470, 00B65560 and 00467050: FSIN and FCOS return an operand of magnitude 2^63 or more
  unchanged, and turn an infinity into the default NaN.
- 004E5C00 and 00452B00: an infinite exponent product gives inf - inf after FRNDINT, so the
  image stores the default NaN, not inf or 0.
- `x87_power_response` in `src/engine_seg4_6_9_cc13.cpp`:
  - It is used by 00452BD0 at 00452FB7 and 00453068, and has the same sequence. It now uses
    `x87_fyl2x` and `x87_pow2_tail24`.
  - 00452BD0's callers (the HUD binoculars raise at 0051F00C and the HUD unit view at 006515F9)
    are on the main thread.
  - 00452BD0 is unwired, since only the unimplemented `cc13_sweepB` host virtual names it.
  - It is not fixture-tested, so its status is unchanged.
- 00428090 (`segment_intersects_sphere_00428090`):
  - Only the `FSTP DWORD` stores round to float, and the sums and products between them stay on
    the stack.
  - The first follow-up (60e1a5dd7) held them in double. This follow-up holds them at 24-bit,
    because the caller is not worker-only.
  - Orders: a = (dx*dx + dy*dy) + dz*dz; half_b = (dy*fy + dx*fx) + dz*fz;
    c = ((fx*fx + fy*fy) + fz*fz) - r*r; disc = b*b - (a*4)*c. q comes from `_CIsqrt`
    (00BF7030).
  - It is unwired: `cc13_big1` reaches it only through a host virtual that the null host leaves
    unreached.
- 00467050: the three-factor products are (sc*sa)*sb and (sc*sa)*cb (0046709E, 004670BF,
  004670D8). A 53-bit test cannot tell those orders apart.
- 00B62A10, 00B62DD0, 00A8B860, 0046F550/6D0/7B0, 00AFB7F0/BA40, 00793730/800, 007BB4C0,
  00B75520 and 00BBF280: remodelled per operation at 24-bit, in listing order.
- `include/bsp/render_fx_cc13.hpp`: the `vertex_element_type_size_00b478e0` declaration was
  inside a comment line that held literal `\n` sequences. It is now split onto real lines.
