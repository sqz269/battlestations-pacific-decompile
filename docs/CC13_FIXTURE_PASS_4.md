# cc13 leaf fixture, pass 4 (packet cc13-fixture4)

This pass adds 12 main-thread math leaves to the cc13 leaf harness, with the method of pass 2
(`docs/CC13_FIXTURE_PASS_2.md`).

- The harness is `tools/verify_cc13_leaf_reference.py`. Pass 4 adds a block at the end of its
  CODE table and four constants to DATA: `00D7A244`, `00D7A248`, `00D7A210` and `00D638E4`.
- The cases are the `run_fixture4` block at the end of `tests/cc13_leaf_fixture_tests.cpp`.

Every case runs on the same inputs twice, once at 53-bit and once at 24-bit precision control.
The deciding pass is the precision of the thread the image runs the routine on
(`docs/CC13_FPU_PRECISION.md`). For every routine here that is 24-bit.

## Inputs

Edge values are mixed in as in pass 2: one draw in six, from signed zeros, units, a quiet NaN,
denormals, FLT_MIN, 1e30, infinities and FLT_MAX.

The FFT grid and the flag cloth take whole arrays, where one draw in six would turn every output
into NaN or infinity. For those cases the edge value goes into one iteration in four, at one
random element.

The cases also cover these boundaries:

- equal box bounds (00B917D0);
- a degenerate segment (00B999F0);
- `life == elapsed` (0042C090);
- a negative and a zero point count (006F9EE0);
- coincident cloth points (00709EF0).

## Results

The counts are for the edge-value input set.

| Routine | Thread | 24-bit (deciding) | 53-bit | Outcome |
| --- | --- | --- | --- | --- |
| 00B6D440 matrix scale | main (animation blend 00B6DE40) | 2000/2000 | 2000/2000 | promoted |
| 00B6D630 matrix add | main (animation blend 00B6DE40) | 2000/2000 | 2000/2000 | promoted |
| 00B6D6D0 matrix subtract | main (animation blend 00B6DE40) | 2000/2000 | 2000/2000 | promoted |
| 00B917D0 quadtree clip range | main (quadtree render 00B91A50) | 4000/4000 | 4000/4000 | promoted |
| 00B94E90 ocean FFT32 rows | unreferenced, ocean family | 200/200 | 0/200 | remodelled, promoted |
| 00B950B0 ocean FFT32 columns | unreferenced, ocean family | 200/200 | 0/200 | remodelled, promoted |
| 00B65710 plane set classify AABB | main (004B48E0 via 00B71590) | 4000/4000 | 4000/4000 | fixture-found fix, promoted |
| 0042C090 random wander evaluate | main (cockpit camera 00434810) | 4000/4000 | 3999/4000 | remodelled, promoted |
| 006F9EE0 convex object box | resource load (006FAE89 in 006FAD70 ReadPayload, from 006FAF00) | 6000/6000 | 6000/6000 | promoted (exact at both precisions) |
| 00B999F0 plane / segment intersection | main (ocean projector 00B9AE00) | 3999/4000 (+1 NaN-only) | 3140/4000 | remodelled, NaN-only, not promoted |
| 00709EF0 flag cloth wind and springs | main (008E2560 via 0070AA70) | 397/400 (+3 NaN-only) | 120/400 | fixture-found fix, NaN-only, not promoted |
| 0070A450 flag cloth integrate | main (008E2560 via 0070AA70) | 398/400 (+2 NaN-only) | 398/400 (+2) | unchanged, NaN-only, not promoted |

The ocean FFT has no reference in the image. Under the pass 2 rule it is judged at 24-bit, as
the ocean family it belongs to is: the wave field update 00BA0090 runs on the main thread.
Both entries are one C++ routine: the columns entry 00B950B0 is the same `ocean_fft32` called
with the column stride (32), the rows entry 00B94E90 with stride 1.

### Pinned counts

The deciding pass of these three routines is pinned with the default input set. They are
pinned in `fixture4_pin` in the test file, next to the pass 2 table.

| Routine | NaN-only count |
| --- | --- |
| 00B999F0 | 1 |
| 00709EF0 | 3 |
| 0070A450 | 2 |

None of them has a value mismatch. With `BSP_FIXTURE_NO_EDGES` set, every deciding pass matches
outright.

## Fixes

### 00B65710

The previous model did the whole test in double, and mismatched 24 of 4000 at both precisions.
The image does the following:

- It stores `min + max` and `max - min` to float.
- It halves each with the double `[00D7A280]` into a float slot.
- It sums the centre dot product as `(b cy + a cx) + c cz`, stores it to float, and subtracts
  `d` into another float.
- It sums the radius as `(|b| hy + |a| hx) + |c| hz` and stores it to float.
- The test is `-radius > dist`, by FCHS then FCOMIP / JA, so a NaN continues to the next plane.
- `|n|` is `n > 0 ? n : -0.0f - n` (COMISS / JBE), so a NaN component takes the subtraction.

### 00709EF0

Several sums are grouped differently from the previous model:

- the wind numerator is `(ny wy + wx nx) + nz wz`;
- the normal length is `(ny ny + nx nx) + nz nz`;
- the spring length is `(dx dx + dy dy) + dz dz`;
- the damping dot is `(dvx ux + dvy uy) + dvz uz`.

A NaN squared length takes the zero-length arm, because FCOMI against 1e-10 is followed by JBE.

### Remodelled at 24-bit

00B94E90/00B950B0, 00B999F0, 0042C090 and 00709EF0 now use the `x87_*24` helpers of
`include/bsp/cc13_x87_model.hpp`. Each x87 operation rounds once to 24 bits with the extended
exponent, and each FSTP float is a cast. Plain float arithmetic and the earlier double models
differed from the image in three ways:

- on overflow of an intermediate;
- on a denormal result, which the x87 rounds twice;
- where products stay on the stack before a sum (the FFT butterflies and twiddles).

### Precision of the other passes

The 53-bit passes are informational. Each remodelled routine now mismatches there, as the
pass 2 remodels do.

## Live code

No switch was needed. None of the routines changed here has a host caller:

- `update_flag_wave_0070aa70` (flag cloth), `ocean_fft32` and `plane_set_classify_aabb` have no
  caller in `src/`.
- `random_wander_evaluate_0042c090` is reached only from `plane_cockpit_camera_step_00434810`,
  which nothing calls.
- `plane_intersect_segment_00b999f0` is reached only from `compute_projector_matrices_00b9ae00`
  and `plane_crossing_00b9a8a0`, which nothing outside `water_render.cpp` calls.

## Build note

The 0042C090 case links `src/segment2_camera_cc13.cpp` (bsp_core), which calls
`dive_bomb_interpolate_clamped_00419010`. That function used to live in the bsp_game file
`src/dive_bomb_task.cpp`, so the fixture failed to link (LNK2019). cgbuildB moved it verbatim
to `src/unit_rudder.cpp` (bsp_core), and this pass relies on that move. No CMake change is
needed.

## Sqrt

00709EF0 and the FFT call 00BF7030. As in pass 2, the harness binds that call to `FSQRT; RET`.
At 24-bit that rounds once to 24 bits. The CRT's SSE2 path (SQRTSD, then the FSTP float) gives
the same value for a float operand, because the double rounding is innocuous for a square root
when 53 >= 2 * 24 + 2.
