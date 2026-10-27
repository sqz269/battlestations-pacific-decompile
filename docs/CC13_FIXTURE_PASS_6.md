# cc13 leaf fixture, pass 6 (packet cc13-fixture6)

This pass adds 14 render and geometry float leaves to the leaf fixture. The method is pass 2's
(`docs/CC13_FIXTURE_PASS_2.md`):

- `tools/verify_cc13_leaf_reference.py` copies each routine's original bytes and constants.
- `tests/cc13_leaf_fixture_tests.cpp` (`run_fixture6`) runs them beside the reconstruction. The
  CTest name is `cc13_leaf_differential`.
- Each case runs at 53-bit and at 24-bit x87 precision control, on the same inputs.
- The deciding pass is the precision of the calling thread. Every routine here is judged at
  24-bit, since each one is reached from the main thread or belongs to a main-thread family.

## Inputs

- The pass 2 edge values are mixed in: signed zeros, a quiet NaN, denormals, FLT_MIN, 1e30,
  infinities and FLT_MAX.
- The routines that only move floats get raw dwords: any bit pattern, with one draw in eight
  forced to a signalling NaN. These are 007AE330/3E0, 00B75DE0, 00B40440 and 00ADB280/BD00.
- Boundaries are seeded:
  - plane sets: the last plane's s - d near 0;
  - the box test: s - d near +r and -r;
  - the extents: 0, powers of two, odd values above 2^24 and values of 2^31 and up;
  - equal operands for min/max;
  - absent tiles and empty grids for the height range.

## Results (edge-value input set)

| Routine | Thread | 24-bit | 53-bit | Outcome |
| --- | --- | --- | --- | --- |
| 00AE05E0 | main (bush cell cull 00AE1A10) | 3000/3000 | 2964/3000 | remodelled, promoted |
| 00B65140 | main (camera classify point 00B714D0) | 3000/3000 | 2985/3000 | remodelled, promoted |
| 00B4CC50 | main (luminance pass 00B50050) | 2000/2000 | 1933/2000 | remodelled, promoted |
| 00B4CDE0 | main (luminance pass 00B50050) | 2000/2000 | 1734/2000 | remodelled, promoted |
| 00B8B350 | main (flare update 00B8D150) | 3000/3000 | 2255/3000 | remodelled, promoted |
| 00481510 | main (traffic cell tree 00498C40) | 3000/3000 | 3000/3000 | unchanged, promoted |
| 00B9A3F0 | main (ocean optics 0078C9B0, load) | 3000/3000 | 3000/3000 | unchanged, promoted |
| 00B999A0 | unreferenced (render family) | 3000/3000 | 2293/3000 | remodelled, promoted |
| 00B75DE0 | main (animator pose 00B75E90) | 2000/2000 | 2000/2000 | signalling NaN quieted, promoted |
| 00B40440 | unreferenced caller 00B405E0 (render family) | 2000/2000 | 2000/2000 | signalling NaN quieted, promoted |
| 00ADBD00 | main (height TGA dump 00ADC1C0) | 1000/1000 | 1000/1000 | signalling NaN quieted, promoted |
| 00ADB280 | main (height field vtable 00D5D358) | 1000/1000 | 1000/1000 | signalling NaN quieted, promoted |
| 007AE330 | main (path / MovCam 007AE49F, 007AF335) | 3000/3000 | 3000/3000 | signalling NaN quieted, promoted |
| 007AE3E0 | main (path / MovCam 007AE4CF, 007AF421) | 3000/3000 | 3000/3000 | signalling NaN quieted, promoted |

With `BSP_FIXTURE_NO_EDGES` set, every deciding pass also matches. Nothing is pinned.

## Findings

Before this pass, the 24-bit results were as follows. These counts were measured before the
merge with fixture3 and fixture5, which now draw from the same generator first, so the input
draws differ from the table above.

24-bit precision:

- 00B999A0 matched 2286/3000. The C++ used double sums.
- 00B8B350 matched 2293/3000, also because of double sums.
- 00B4CC50 matched 1933/2000 and 00B4CDE0 1716/2000.
  - The extent is FILD (exact), then FADD 2^32 (CE3978) for a negative int, which rounds to 24
    bits.
  - 00B4CDE0 also rounds width * 3 to 24 bits before the FDIV.
  - The C++ formed both exactly and divided in double.
- 00B65140 matched 2941/3000 once the boundary inputs were added (3000/3000 without them).
  - The C++ summed c*z + b*y + a*x in double and never stored the dot to float.
  - The image sums (a*x + b*y) + c*z, FSTPs it to float, then FSTPs s - d to float.
- 00AE05E0 matched 2999/3000.
  - The C++ summed both terms in plain float, in the wrong order.
  - The image forms r = (|b| e.y + |a| e.x) + |c| e.z and s = (a p.x + b p.y) + c p.z.
  - r and s are FSTP'd to float, then s - d is stored to float. -r > s returns Outside (JA),
    and r > s gives Intersecting.

Signalling NaNs:

- Six routines only move floats through FLD / FSTP DWORD: 007AE330/3E0, 00B75DE0, 00B40440 and
  00ADB280/BD00. FLD turns a signalling NaN into the quiet NaN with the same sign and payload.
- The C++ copies kept the signalling NaN: 819, 805, 19 and 156 NaN-only differences per pass.
- The new `x87_fld_fstp` / `x87_fld_fstp_bits` (`include/bsp/cc13_x87_model.hpp`) model the
  load.

Already exact:

- 00481510 (centre = float(min + max) * 0.5) is a single rounding of exact operands.
- 00B9A3F0 (three FDIVR from 1.0) is a single rounding of float operands.
- For +, -, *, / and sqrt of float operands, rounding to 53 bits and then to 24 is the same as
  rounding to 24 once. These two needed no change.

## Live code

None of the changed routines has a live caller:

- `terrain_box_classify_planes_00ae05e0`: only `bush_cell_cull_and_emit_lod_00ae1a10`.
- `sample_offsets_*`: only `luminance_measure_and_adapt_00b50050`.
- `matrix_from_transposed_3x4_00b75de0`: only `optimized_evaluate_pose_00b75e90`.
- `float3x4_from_matrix_transposed_00b40440`: only `float4_block_set_matrix_rows_00b405e0`.

Those callers are cc13 bodies that nothing calls. The rest have no caller at all, so no switch
was needed.
