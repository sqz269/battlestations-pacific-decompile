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
control. The pass that decides is the precision the reconstruction models. That is 24-bit for
`009D7780`, `009D7930` and `004F3DA0`, and 53-bit for the rest. The other pass is printed for
information only.

## Results (the deciding precision)

Bit-exact, 26 routines, promoted to `semantic_native_fixture_tested`:

- 53-bit: 00B65560, 00B8C520, 00B62A10, 00467050, 004E5C00, 00452B00, 0046F7B0, 0046F550,
  0046F6D0, 00AED820, 00AFB7F0, 00AFBA40, 00ADFA90, 00ADC580, 009064D0, 00B8B940, 00793730,
  00793800, 007BB4C0, 00B75580, 00B75520, 00BBF280, 00C37860 and 00B478E0.
- 24-bit: 009D7780 and 009D7930.

Bit-exact except where both sides are NaN (only the sign or payload of the NaN differs), not
promoted:

| Routine | NaN-only cases |
| --- | --- |
| 00B64470 | 6/2000 |
| 00B62DD0 | 54/2000 |
| 00941AA0 | 8/3000 |
| 00A8B860 | 2/3000 |

The image propagates an input NaN's sign and payload, while the SSE code produces or propagates
a different one.

Known findings, not fixed and not promoted:

| Routine | Mismatching cases |
| --- | --- |
| 00428090 | 213/3000 |
| 004F3DA0 | 57/4000 at 24-bit |

Both reconstructions do their intermediate arithmetic in float, so an operand near 1e30
overflows to infinity. On the x87 stack the same value is still in range even at 24-bit
precision, so the boolean result flips. Without edge values both routines match.

## Fixture-found fixes

- 00B64470 (`compose_euler_scale_translation`): before multiplying by the scale, the image
  stores seven pair products to float: c2*c0, -c2*s0, s2*s0, -s2*c0, c1*s2, c1*c2, c1*s0 and
  c1*c0. Only 159 of 2000 cases matched before the fix.
- 00B75580 (`bezier_00b75580`): the result is rounded to float (`FSTP DWORD` / `FLD` at
  00B755F7) before the return. 2 of 3000 cases matched before.
- 00941AA0 (`spawn_ray_sphere_00941aa0`):
  - The projection and the squared miss are summed in the order x, y, z on the x87 stack.
  - An unordered (NaN) compare intersects, because `JB` is taken on unordered.
- 00B64470, 00B65560 and 00467050: FSIN and FCOS return an operand of magnitude 2^63 or more
  unchanged, and turn an infinity into the default NaN. These are modelled by `x87_sin` and
  `x87_cos`.
- 004E5C00 and 00452B00: an infinite exponent product gives inf - inf after FRNDINT, so the
  image stores the default NaN, not inf or 0. `x87_power_response` in
  `src/engine_seg4_6_9_cc13.cpp`, used by 00452BD0, has the same sequence. It is not fixed,
  because 00452BD0 is not in this pass.
- `include/bsp/render_fx_cc13.hpp`: the `vertex_element_type_size_00b478e0` declaration was
  inside a comment line that held literal `\n` sequences. It is now split onto real lines.
