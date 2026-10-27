# cc13 fixture pass 9

Packet cc13-fixture9 adds native fixtures for 16 leaf routines: vector helpers, exact
compares and float copies. The method is the same as in passes 2 to 8:

- `tools/verify_cc13_leaf_reference.py` copies each routine's original bytes, relocated,
  into the test image.
- `tests/cc13_leaf_fixture_tests.cpp` (`run_fixture9`) runs the native bytes and the C++ on
  the same inputs: 3000 pseudo-random cases mixed with edge values (signed zeros, NaNs,
  denormals, FLT_MIN, FLT_MAX, infinities). The copy routines take raw bit patterns,
  signalling NaNs included.
- Each case runs under both 53-bit and 24-bit x87 precision control.

The deciding precision is that of the calling thread (docs/CC13_FPU_PRECISION.md). Every row
here is called from main-thread game code, or has no call site in the image, so every row is
judged at 24-bit. Rows that match bit for bit at 24-bit are promoted to
`semantic_native_fixture_tested`.

## Picks

The candidates came from `local/kept/fixture_candidates.tsv` in the cc13 tree. The filters
were:

- the row is still `reconstructed_build_tested` on origin/main;
- the routine is not already in the reference tool;
- neither its address nor its source file is leased by another lane.

From what was left, the highest-scoring pure leaves were taken (no calls out, no data
references).

## Results

The figures are matches out of runs.

| Address | C++ symbol | Thread (callers) | 24-bit (deciding) | 53-bit (informational) | Outcome |
|---|---|---|---|---|---|
| 0042AE80 | `bsp::vector3_divide_in_place_0042ae80` | rocket air step, HUD movie camera, plane state messages, replication (main) | 3000/3000 (the 24-bit arm) | 2991/3000 | OFF switch; not promoted |
| 0042AE50 | `bsp::profile_seg2_cc13::vector3_scale_in_place_0042ae50` | plane wanderer fixed step (as 00C37F60 in pass 3) | 3000/3000 | 2991/3000 | fixed, promoted |
| 0042AE20 | `bsp::profile_seg2_cc13::vector3_divide_scalar_to_0042ae20` | MovCam (main) | 3000/3000 | 2991/3000 | fixed, promoted |
| 00414260 | `bsp::engine_seg1_cc13::vector3_scale_to_00414260` | HUD gun sight, landing ship, MovCam, gun bots (main) | 3000/3000 | 2991/3000 | fixed, promoted |
| 004142A0 | `bsp::engine_seg1_cc13::vector3_add_to_004142a0` | as 00414260 (main) | 3000/3000 | 3000/3000 | promoted |
| 0042B220 | `bsp::profile_seg2_cc13::vector3_dot_0042b220` | MovCam step (main) | 3000/3000 | 2197/3000 | fixed, promoted |
| 00709AC0 | `bsp::band_0067_cc13::vec3_sub_00709ac0` | no call site | 3000/3000 | 3000/3000 | promoted |
| 00B50020 | `bsp::cc13_scene2::vector4_add_00b50020` | data pointer only | 3000/3000 | 3000/3000 | promoted |
| 0042AEE0 | `bsp::profile_seg2_cc13::vector3_subtract_in_place_0042aee0` | MovCam step (main) | 3000/3000 | 3000/3000 | promoted |
| 0042AEB0 | `bsp::profile_seg2_cc13::vector3_add_in_place_0042aeb0` | fish school, plane state, aim ray (main) | 3000/3000 | 3000/3000 | promoted |
| 00427C90 | `bsp::cc13_left61::vector3_subtract_to_00427c90` | HUD screens, air ops (main) | 3000/3000 | 3000/3000 | promoted |
| 00475DF0 | `bsp::band_0045_cc13::exact_equals_00475df0` | HUD minimap, tagged value equals (main) | 3000/3000 | 3000/3000 | promoted |
| 00453F40 | `bsp::band_0045_cc13::exact_equals_00453f40` | movie entity track, tagged value equals (main) | 3000/3000 | 3000/3000 | promoted |
| 00533D30 | `bsp::frontend_seg15_22_cc13::vec2f_equal_00533d30` | HUD formation, tagged value equals (main) | 3000/3000 | 3000/3000 | promoted |
| 006FDD00 | `bsp::band_006b_cc13::matrix4x4_transpose_in_place_006fdd00` | gun shot spawn (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00B40400 | `bsp::cc13_render1::float2x4_from_matrix_transposed_00b40400` | no call site | 3000/3000 | 3000/3000 | fixed, promoted |

There are no pins. Every deciding pass matches outright, with no NaN-only differences.

## Fixture-found defects

**Double rounding at 24 bits.** At 24-bit precision control each FMUL or FDIV rounds to a
24-bit significand but keeps the extended exponent. The FSTP DWORD that follows rounds the
value again, and that second rounding changes the result when it is a float denormal.

- 0042AE20, 0042AE50 and 00414260 now use `cc13x87::x87_div24` and `x87_mul24`.
- 0042B220 keeps the whole dot product on the stack: three products and two sums, each
  rounded to 24 bits. The earlier model summed at 53 bits.
- The adds and subtracts (004142A0, 00B50020, 0042AEB0, 0042AEE0, 00427C90, 00709AC0)
  already matched. The exact sum of two floats is exact whenever it lands in the denormal
  range, so these never round twice.

**Signalling NaN on copies.**

- 00B40400 copies through FLD DWORD / FSTP DWORD, which quiets a signalling NaN (bit 22).
  It now uses `cc13x87::x87_fld_fstp_bits`, as pass 6 did for 00B40440.
- 006FDD00 swaps each pair asymmetrically. The lower element moves up through FLD/FSTP and
  is quieted; the upper element moves down through MOVSS and is left unchanged. `std::swap`
  quieted neither.

**0042AE80 is live.** The rocket air step reaches it from `game_hosts_gunnery.cpp`.

- The 24-bit model is `vector3_divide_in_place_pc24_0042ae80`, and the fixture tests it.
- The live function keeps the earlier double model until the OFF switch
  `kVector3DivideInPlacePc24Cc13Bound` (include/bsp/rocket_homing_cc13.hpp) is flipped.
- With the switch ON, the only results that change are quotients in the float denormal
  range, which the image rounds twice.
- `tools/off_switch_on_build.py --names kVector3DivideInPlacePc24Cc13Bound` builds the ON
  state cleanly.
- The ledger row stays `reconstructed_build_tested` until the flip.

The other fixed bodies have no live caller. Their callers in `src/` are unbound cc13 host
interfaces or cc13 functions that nothing outside cc13 calls. cc13_sweepB5.cpp keeps a private copy of
00414260 and 004142A0, which this pass does not change.
