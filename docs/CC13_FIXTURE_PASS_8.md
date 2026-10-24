# cc13 leaf fixture, pass 8 (packet cc13-fixture8)

This pass adds 16 small float routines to the harness of `docs/CC13_FIXTURE_PASS_2.md`. The
harness is `tools/verify_cc13_leaf_reference.py` (the CODE and DATA tables) and
`tests/cc13_leaf_fixture_tests.cpp`. CTest runs it as `cc13_leaf_differential`. The cases are
`run_fixture8` at the end of the test file. They use the same edge-value draw (about one in six)
as pass 2, with their own seed `0x5eed0f08`. They run once at 53-bit and once at 24-bit precision
control on the same inputs.

The test now holds up to 512 cases (`kMaxCases`). Opening one more prints an error and exits
non-zero, where the old array of 200 would have been overrun silently.

## Threads

Every routine runs on the main thread, so the 24-bit pass decides. Two routines,
`009C6220` and `006EB870`, have no callers: rel32 and absolute-dword scans both came back empty.
They are judged by their family (seg67 bot tasks and the torpedo class), which also runs on the
main thread.

| Address | C++ | Caller evidence | 24-bit | 53-bit |
|---|---|---|---|---|
| 00809DE0 | `bsp::rocket_aim_lift_00809de0` | 007B4390, 007BD120 | 2000/2000 | 2000/2000 |
| 007B5390 | `bsp::rocket_projtime_007b5390` | 007B5430 | 2000/2000 | 2000/2000 |
| 00791640 | `bsp::cc13cam::orbit_input_track_tick_00791640` | vtable 00D04448 slot 4 | 2000/2000 | 2000/2000 |
| 0059AB10 | `bsp::cc13_fe1::hud_map_animate_zoom_0059ab10` | 005B45C0 | 3000/3000 (remodelled) | 2862/3000 |
| 00790460 | `bsp::cc13cam2::movcam_snapshot_00790460` | vtable 00D04B1C slot 3 | 1000/1000 | 1000/1000 |
| 00414130 | `bsp::engine_seg1_cc13::math_lerp_00414130` | 00599DB0 | 3000/3000 (remodelled) | 2397/3000 |
| 009C6220 | `bsp::math_clamp_symmetric_009c6220` | none | 3000/3000 | 3000/3000 |
| 0088A0C0 | `bsp::cc13_left55::scored_med3_0088a0c0` | 0088A650 | 3000/3000 | 3000/3000 |
| 0098A710 | `bsp::cc13_seg62::node_bounds_0098a710` | 00903420 | 1000/1000 | 1000/1000 |
| 00638B20 | `bsp::ingame_gui_cc13::hud_marker_endpoints_00638b20` | 00640D70 | 1000/1000 | 1000/1000 |
| 0042C450 | `bsp::cc13_sweepA::camera_clamp_zoom_0042c450` | 00609BD0 | 3000/3000 | 3000/3000 |
| 007B5BE0 | `bsp::cc13_seg47::unit_largest_extent_007b5be0` | 007B5F10, 009B5680, 009C4950, 009D0D90 | 3000/3000 | 3000/3000 |
| 006EB870 | `bsp::cc13_seg33a::ratio_abs_dc_over_8c_006eb870` | none | 3000/3000 (remodelled) | 3000/3000 |
| 009B93F0 | `bsp::bot_task_tight_turn_slot68_009b93f0` | vtable 00D205E0 slot 26 | 3000/3000 | 2972/3000 |
| 00B0A3F0 | `bsp::particle_emitters::tracer_bounding_sphere_00b0a3f0` | vtable 00D5E048 slot 9 | 1000/1000 | 1000/1000 |
| 006EA530 | `bsp::cc13_seg33a::torpedo_class_scaled_e4_006ea530` | vtable 00CFA56C slot 7 | 3000/3000 | 3000/3000 |

The 53-bit mismatches belong to routines that are modelled at 24-bit. They are informational
only.

## Case notes

- **007B5390.** The original 00809E10 runs in the fixture to get the flight time. The case then
  compares the clamp and the +9Ch store. The caller (`rocket_task_cc13.cpp`) skips a null
  `[approach+D4h]`, so the case does not test that.

## Fixes

Three reconstructions computed a chain of x87 operations in double and rounded once at the end.
At 24-bit precision control, the image rounds after every operation. Each now uses the
`include/bsp/cc13_x87_model.hpp` helpers. None of the three is live, so the fixes are direct and
not behind a switch:

- **00414130 `math_lerp_00414130`.** FSUBP, FMULP and FADDP each round. Before: 603/3000. Its host
  use in `game_hosts_mission.cpp` runs only under `kMissionDetailOffsetLerpBound`, which is OFF.
- **0059AB10 `hud_map_animate_zoom_0059ab10`.** FMUL by 1.5 and FADD/FSUBP each round. Before:
  138/3000. The host HUD map update does not call it.
- **006EB870 `ratio_abs_dc_over_8c_006eb870`.** The FDIV rounds to 24 bits and then the FSTP rounds
  again, which differs from one float division only for a denormal quotient. Before: 4/3000. It
  has no C++ caller.

The "before" counts come from the first probe run, whose input stream differed slightly.

## Follow-up

Seven more routines were probed in this pass and land in a second commit. 009A5390,
006F22F0, 006F22B0, 00727D30 and 00729A40 were bit-exact, but they live in bsp_game files that
the test target does not build yet. 009D5AE0 and 009D5B90 need a fix in
`src/cc13_seg67b.cpp`, and that fix is live. The walks decide an undecided node with their own inline copy
of the direction rule (009D5B06-009D5B56 and 009D5BB6-009D5C06). Its FCOMIP/JC gives +1 when the
two route costs are unordered (a NaN). The shared rule 009D9E6C, which the C++ walk calls, gives -1
there. The walk is live through `kShipAiArcCentrePlanTestCc13Bound` (ON). The follow-up puts the
inline rule behind an OFF switch, `kShipAiPathSideWalkNanCc13Bound`, and adds the fixture cases.

## Status

Promotion means the ledger row moves to `semantic_native_fixture_tested`. All 16 rows were
promoted, three of them after a remodel. Every row now carries `cpp_symbol` and
`abi_compatible: false`. Nothing here is game-validated.
