# cc13 fixture pass 7

Packet cc13-fixture7 adds native fixtures for 19 leaf routines: Dyn shape supports and
transforms, plus main-thread vector, colour and GUI helpers.

The method is the same as in passes 2 and 3. `tools/verify_cc13_leaf_reference.py` copies
each routine's original bytes into the test image, relocated. Then
`tests/cc13_leaf_fixture_tests.cpp` runs the native bytes and the C++ on the same
pseudo-random inputs mixed with edge values: signed zeros, NaNs (signalling ones included
for the copy routines), denormals, FLT_MIN, FLT_MAX and infinities. Each case runs under
both 53-bit and 24-bit x87 precision control.

The deciding precision is that of the calling thread (docs/CC13_FPU_PRECISION.md):

- A routine reached only from Dyn workers is judged at 53-bit.
- Every other routine is judged at 24-bit, the main thread's precision.
- A routine with no call site in the image is also judged at 24-bit, as passes 2 and 3 did.

Rows that match bit for bit at the deciding precision are promoted to
`semantic_native_fixture_tested`. Results at the other precision are informational.

## Rows

| Address | C++ symbol | Thread evidence | Deciding |
|---|---|---|---|
| 00C48080 | `bsp::cc13_dyn1::sphere_shape_support_00c48080` | Dyn::SphereShape slot 2 (float). Only the vtable entry refers to it, and no slot-2 call site was found, as for 00C47AA0 in pass 3 | 24 |
| 00C38970 | `bsp::cc13_dyn1::sphere_shape_support_00c38970` | slot 3 (double), reached through the GJK chain like 00C38470 and 00C38810 (narrow phase, Dyn workers) | 53 |
| 00C377F0 | `bsp::cc13_dyn1::ccw_2d_00c377f0` | callers 00C41190 and 00C41230 are reached only from 00C4D5A0, which a rel32 and absolute scan finds unreferenced | 24 |
| 00C33650 | `bsp::cc13_dyn1::transform_expand_3x4_to_4x4_00c33650` | GameDynamicsList_UpdateFrame, UnitInstance_Update (main thread) | 24 |
| 00C336C0 | `bsp::cc13_dyn1::transform_from_4x4_00c336c0` | main thread, load and Lua paths | 24 |
| 00C37F10 | `bsp::cc13_dyn1::body_inertia_00c37f10` | UnitInstance_OnWrecked (main thread) | 24 |
| 00709B60 | `bsp::band_0067_cc13::vec3_add_scaled_in_place_00709b60` | main-thread helper | 24 |
| 006E0540 | `bsp::band_006b_cc13::vec3_add_scaled_in_place_006e0540` | main-thread helper | 24 |
| 0059C920 | `bsp::cc13_fe1::game_world_bounds_center_0059c920` | front end (main thread) | 24 |
| 004B4850 | `bsp::cc13_band004b::game_map_box_center_004b4850` | game map (main thread) | 24 |
| 004193E0 | `bsp::engine_seg1_cc13::vector3_length_squared_004193e0` | main-thread helper | 24 |
| 00427E30 | `bsp::cc13_left61::vector3_length_squared_00427e30` | HUD callers (main thread) | 24 |
| 004191D0 | `bsp::engine_seg1_cc13::vector2_length_squared_004191d0` | main-thread helper | 24 |
| 00BEE140 | `bsp::engine_services_cc13::scale_vector4_00bee140` | the HUD host (src/game_hosts_hud_world.cpp, main thread). Live | 24 |
| 004E5D20 | `bsp::scene_weather_cc13::scale_rgba_004e5d20` | weather reader, caller 004EC99E (main thread) | 24 |
| 00AB6BD0 | `bsp::cc13_gui1::text_context_get_normalized_text_height_00ab6bd0` | GUI text (main thread) | 24 |
| 007B8140 | `bsp::cc13_seg47::float_window_set_sample_and_sum_007b8140` | main-thread game code | 24 |
| 007BA450 | `bsp::cc13_seg47::max3_007ba450` | main-thread game code | 24 |
| 00AB0FE0 | `bsp::cc13_gui1::gui_color_equal_00ab0fe0` | GUI (main thread) | 24 |

## Results

The figures are matches out of runs. Every row matches bit for bit at its deciding
precision, so every row except 00BEE140 is promoted (see below).

| Address | Deciding precision | Other precision (informational) | Outcome |
|---|---|---|---|
| 00C48080 | 24: 2000/2000 | 53: 1999/2000 | fixed, promoted |
| 00C38970 | 53: 2000/2000 | 24: 202/2000 | promoted |
| 00C377F0 | 24: 4000/4000 | 53: 3998/4000 | fixed, promoted |
| 00C33650 | 24: 1000/1000 | 53: 1000/1000 | fixed, promoted |
| 00C336C0 | 24: 1000/1000 | 53: 1000/1000 | fixed, promoted |
| 00C37F10 | 24: 2000/2000 | 53: 2000/2000 | promoted |
| 00709B60 | 24: 3000/3000 | 53: 2999/3000 | fixed, promoted |
| 006E0540 | 24: 3000/3000 | 53: 2999/3000 | fixed (through 00709B60), promoted |
| 0059C920 | 24: 3000/3000 | 53: 3000/3000 | promoted |
| 004B4850 | 24: 3000/3000 | 53: 3000/3000 | promoted |
| 004193E0 | 24: 3000/3000 | 53: 2668/3000 | fixed, promoted |
| 00427E30 | 24: 3000/3000 | 53: 2429/3000 | fixed, promoted |
| 004191D0 | 24: 3000/3000 | 53: 2958/3000 | fixed, promoted |
| 00BEE140 | 24: 3000/3000 (the 24-bit arm) | 53: 2971/3000 | OFF switch; not promoted |
| 004E5D20 | 24: 3000/3000 | 53: 2971/3000 | fixed, promoted |
| 00AB6BD0 | 24: 3000/3000 | 53: 3000/3000 | promoted |
| 007B8140 | 24: 3000/3000 | 53: 3000/3000 | promoted |
| 007BA450 | 24: 3000/3000 | 53: 3000/3000 | fixed, promoted |
| 00AB0FE0 | 24: 3000/3000 | 53: 3000/3000 | promoted |

## Fixture-found defects

**Double rounding at 24 bits.** At 24-bit precision control every FADD, FSUB and FMUL
rounds to a 24-bit significand but keeps the extended exponent. The FSTP DWORD that
follows rounds the value again, and that second rounding changes the result when it is a
float denormal. The C++ had rounded once.

These now use `cc13x87::x87_mul24`, `x87_add24` and `x87_sub24` from
include/bsp/cc13_x87_model.hpp:

- 00709B60 (006E0540 forwards to it)
- 004E5D20
- 00C48080
- 00C377F0, whose differences and cross product round to 24 bits
- 004191D0 and 004193E0. In 004193E0 the x*x + y*y sum stays on the stack while the
  squares are stored.

00427E30 keeps the whole chain (x*x + y*y) + z*z on the stack. In float arithmetic tiny
squares underflowed, so the fixture now adds tiny (2^-72) and huge (2^54) vectors.

**Signalling NaN on copies.** 00C33650 and 00C336C0 copy through FLD DWORD / FSTP DWORD,
which sets bit 22 of a signalling NaN. The C++ copied words. Each copy now goes through
`fld_fstp_word` in src/cc13_dyn1.cpp.

**MAXSS fold.** MSVC compiled `!(x >= y) ? y : x` in 007BA450 to MAXSS, even under
`/fp:strict`. MAXSS keeps its second operand on a tie, so max3(+0, -0, ..) returned -0
where the image returns +0. The selections now index a two-element array.

**00BEE140 is live.** The HUD host calls it on the main thread. The 24-bit model is
`scale_vector4_pc24_00bee140`, which the fixture tests. The live function keeps the
earlier float model until the OFF switch `kScaleVector4Pc24Cc13Bound`
(include/bsp/engine_services_cc13.hpp) is flipped. `tools/off_switch_on_build.py --names
kScaleVector4Pc24Cc13Bound` builds the ON state cleanly. The ledger row stays
`reconstructed_build_tested` until the flip.

004E5D20 was an anonymous helper. It is now public as
`bsp::scene_weather_cc13::scale_rgba_004e5d20`, and the weather reader's `scale_rgba`
calls it.

## Reference data

The reference tool gains the 19 routines and the double 720.0 at 00CEF1B8, the divisor in
00AB6BD0. None of the routines calls out.
