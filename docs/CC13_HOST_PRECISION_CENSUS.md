# Host precision census against the per-thread x87 policy

Addresses: 00825F20 009C0251 009EDA28 0099F1C0 008206F0 009BFEE0 007BE060 00A11690 009BEE56
0085E4F0 00936DC0 007DF4F0 009E6E80 0099CAB0 00953CC0 00955630 0072FEFB 009B5C80 009D0F10
004F3970 0092E9E1 00C549D0 00C53630 00C535E0 00C3F760 00C44090 00403720 00403850 00403CC0
00C4F040 00C4DE40 00C42BA0 00C42530 00C42230 00C37B50 00C35020 00BF7030 00BF704D 00BF84E8
00BFA078 00BFA968 00BFE638 00BFE798 00BFEDF8 00ADF1B0 00AE9D80 00AEA2B0 004D4DF0 00431480

Packet `cc13-precision1`, worker `agent/cc13-precision1`, 2026-10-12, requested by cc9.
Read-only: no C++, ledger or Ghidra change. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; the listing quotes are disk bytes (`bsp.py disasm-raw`).
Builds on `docs/CC13_FPU_PRECISION.md` (which thread runs which x87 precision). Machine
readable rows: `reports/cc13_host_precision_census.json`. Status of everything here:
**static analysis of host source plus listing spot checks**; nothing was built differently,
run, or game-validated.

## Policy measured against

cc9's host policy: single-threaded; main-thread and fixed-step unit math at x87 PC=24 (the
image's main thread, and its `SetSingleThread(true)` mode); the Dyn solver and collision
tasks at PC=53 (the image's Dyn workers, always).

The host build (`CMakeLists.txt:223`, `cmake/startup.cmake:443`) is MSVC Win32 `/fp:strict`
with the default `/arch:SSE2`:

| Host construct | Effective precision |
| --- | --- |
| C++ `float` expression | SSE single, rounded after every operation: **24-bit** |
| C++ `double` expression (including `static_cast<double>` chains and unsuffixed literals) | SSE2 double: **53-bit**, independent of the x87 control word |
| inline `__asm` x87 (`native_*.cpp`) | the **calling thread's** x87 PC. bsp_game's main thread is at PC=24 after its own `CreateDevice` (`src/d3d9_startup.cpp:45`, no `FPU_PRESERVE`; measured `0x00020000` at the fixed step, `docs/GAME_EXECUTABLE.md:4383`) |
| stock Lua 5.1.1 (`cmake/lua.cmake`) `lua_Number` | SSE2 double: 53-bit |

Three facts decide which host code is a real mismatch:

1. **At PC=24 every x87 operation equals a float operation.** The x87 rounds each result
   straight to 24 bits, so whether the image keeps an intermediate on the stack or spills it
   to a float temporary makes no difference on the main thread (barring overflow, underflow
   and denormals, where the x87 keeps its 15-bit exponent). A plain C++ `float` expression
   therefore reproduces main-thread image arithmetic exactly, whatever the spill pattern.
2. **One double operation on two floats, narrowed at once, is exact.** 53 >= 2*24+2, so
   `static_cast<float>(double(a) op double(b))` for `+ - * /` and `sqrt` rounds to the same
   float as the float operation. Such single-op casts are not findings. A chain of two or more
   operations in double before the narrowing, a `double` local carried across statements, or a
   comparison whose arithmetic side is computed in double, is a finding.
3. **CRT calls.** `sqrt` (`00BF7030`, worker `00BF704D`) keeps the caller's PC (it calls
   `00C083A5`, `AND 300h / OR 7Fh`), so on the main thread it is a 24-bit `FSQRT`, equal to
   `sqrtf`. The transcendental group switches to 53 bits internally and leaves a 53-bit result
   on the stack: `00BF84E8` (`FPATAN`, atan), `00BFA078` (`FPTAN`), `00BFA968` and `00BFEDF8`
   (`FYL2X`, the logarithms), `00BFE638` (`FSIN`), `00BFE798` (`FCOS`). For those, a host
   `double` call narrowed straight to float is the closer model, and `sinf`/`cosf` can differ in
   the last float bit; either is a low-risk item.

A single operation of a float with a double constant that has more than 24 significant bits
(the image's `FMUL qword [const]`) rounds once in the image and twice in the host
(`static_cast<float>(double(x) * c)`); the results differ only in rare halfway cases. Many
image constants are float values stored as doubles (`3.1415927410125732`), for which this
cannot happen.

## Answer

| Class | What | Rows | Gameplay impact |
| --- | --- | --- | --- |
| **B** | Dyn worker code that the host runs at 24 bits: the narrow phase (inline x87 on the PC=24 main thread) and the LCP solve (C++ float) | 56 | **High.** Every hull-terrain, hull-hull, hull-fort and hull-draft contact, every substep |
| **A** | Main-thread / fixed-step gameplay math the host computes in double where the image rounds every op to 24 bits | 741 rows, of which 439 gameplay (cc9 lanes + Dyn host) | **Medium to high**, concentrated in plane bot AI, ship AI geometry, gunnery ballistics |
| **C** | All Lua script arithmetic: stock Lua in SSE2 double vs the image's main-thread PC=24 | 1 subsystem | **Medium**, global to mission scripts (timers, accumulators, thresholds) |

The rows are "host disagrees with the policy", not "host disagrees with every image run": in
the image's default multi-core mode a fixed-step unit job (ship motion, squadron step, gun
tick, projectiles) runs at 53 bits whenever a pool worker claims it, so class A code in those
jobs (36 rows, marked `unit_step_24_or_53`) matches the image's default mode some of the time
and its single-thread mode never.

## Class B: Dyn worker code computed at 24 bits

The image's Dyn worker set is the direct-call closure of the two task vslots forked through
`00C33140` (`00403720` `Dyn::Scene::LCPSolverTask`, `00403850` `LCPSolver2Task`, `00403CC0`
`Dyn::CollisionSystem::IntersectTask2`) plus the narrow-phase shape-pair dispatch targets,
which the call graph cannot follow through the table: 415 functions. Everything else in
`00C30000..00C6FFFF` runs on the calling thread, which is the main thread
(`00875E0C` in `00875BB0` calls `00C5C540` directly): velocity phase `00C41550`, position
phase `00C5B1B0`, broad phase, `ManifoldUpdate` `00C549D0`, group formation, contact report.

bsp_game does not use the host's own Dyn task manager for ships. `run_world_contact_phase`
(`src/game_hosts_units.cpp:16276`, `kDynWorldContactPhaseBound = true`) and
`HullTerrainContactSolver` (`src/hull_terrain_contact.cpp`, `kHullTerrainContactSolverBound =
true`) run the whole contact phase inline on the main thread:

| Host code | Image | Host precision | Image precision | Risk |
| --- | --- | --- | --- | --- |
| `native_dyn_terrain_convex.cpp` `00C53630`, called at `hull_terrain_contact.cpp:323` | `Dyn_TerrainConvexMeshIntersect_vslot0`, IntersectTask2 on a Dyn worker | inline x87 at the main thread's PC=24 | 53 | penetration depth, contact normal and point for every hull-terrain candidate; feeds the solve and the warm start (persists across steps) |
| `native_dyn_general_convex.cpp` / `native_dyn_primitive_dispatch.cpp` / `native_dyn_box_box.cpp` / `native_dyn_convex_ray.cpp` / `native_dyn_convex_simplex.cpp` / `native_dyn_convex_support.cpp` through `00C535E0` (`hull_terrain_contact.cpp:784, 831, 1138, 1314`) | hull-hull, hull-fort, hull-draft pairs on Dyn workers | inline x87 at PC=24 | 53 | GJK/EPA-style iterations: iteration counts and termination tests can flip, not just last bits |
| `native_dyn_narrow_phase.cpp` `00C3F760` (insert), `00C3F650` | contact insert / merge inside IntersectTask2 | inline x87 at PC=24 | 53 | merge-distance thresholds decide whether a point is new or replaces one |
| `dyn_lcp_impulse_math.cpp`: `dyn_build_contact_rows_00c4de40`, `dyn_solve_normal_rows_00c42530`, `dyn_solve_friction_rows_00c42230`, `dyn_write_back_velocities_00c37b50`, `dyn_store_impulses_00c35020`, `dyn_solve_group_00403720`; warm start `00C42BA0` | `LCPSolverTask` body `00403784..004037DB` on a Dyn worker | C++ float, 24 per op | 53 | 10 iterations per group per substep, with clamped impulses: the clamp branches can flip. Listing: `00C42618..00C4264F` keeps a 3-term dot (`FMUL`, `FMUL`, `FADDP`, `FMUL`, `FADDP`) on the stack before the `FSTP dword` spill, so the image rounds once where the host rounds five times |

The file's own header (`src/dyn_lcp_impulse_math.cpp:9`) and `src/dyn_contact_solver.cpp:7`
assume 24 bits; `docs/CC13_FPU_PRECISION.md` already lists those as corrections for the
solver tasks. The contact *report* in `dyn_contact_solver.cpp` runs on the calling thread and
is correctly 24 bits.

The C++ `ship_hull_shapes.cpp` bounds (`00C57C40`, `00C55FC0`) appear in the worker closure
too, but they are also reached at body build; treat them as shared, low risk. `00401170`
(`BSP_Math_AbsFloat`) and `004011D0` (`BSP_Math_SqrtFloat`) are shared helpers whose precision
is the caller's.

## Class A: gameplay math widened to double

The host's dominant reconstruction idiom models an x87 chain by widening it,
`static_cast<float>(static_cast<double>(a) * b + c)`, which reproduces a **53-bit** thread.
On the main thread the image rounds after each operation. `src/game_hosts_units.cpp:16391`
tells the log that 00825F20's expressions are computed "with SSE2 float32 under /fp:strict,
which rounds every intermediate to 24 bits"; the function that follows (`motion_step_00825f20`,
lines 16374..31726) contains 117 multi-op double chains and 118 double locals, so the log line
does not describe the code.

Lane totals (functions with a class A score of at least 3, diagnostics excluded by name only
in the top lists):

| Lane | Functions | With a multi-op chain or double local | Chains | Double locals |
| --- | --- | --- | --- | --- |
| cc9, named host files (`game_hosts_units/ship_ai/gunnery/ai/script_orders/commands/mission_frame`) | 129 | 100 | 232 | 321 |
| cc9, gameplay-area files (plane, ship AI, torpedo, gun, submarine, AI command) | 299 | 220 | 335 | 196 |
| Dyn host (calling-thread parts of `hull_terrain_contact.cpp`) | 11 | 10 | 26 | 8 |
| cc13 / presentation (camera, water, weather, GUI, scene graph) | 231 | 201 | 406 | 161 |
| other (terrain quadtree, map bounds, ocean spectrum, loaders) | 104 | 84 | 127 | 95 |

Top gameplay rows, hand-checked (H = read by hand, L = also checked against the listing).
"Image" is the precision under the policy; `fixed-step` rows run at 24 or 53 in the image's
default mode.

| # | file:function | Image address | Host evidence | Image thread / precision | Risk |
| --- | --- | --- | --- | --- | --- |
| 1 | `game_hosts_units.cpp:16374` `motion_step_00825f20` | `00825F20` and the inlined plane bot states | mixed: 117 chains, 118 double locals (H) | fixed-step / 24 | hot spots: takeoff `009CE160`/`009CE2C0` (`v*1.4 + B`, `ang*pi/180 >= a2`, lines 28874..29124), landing `009B1D70`/`009B0300`/`009B3C60`, pilot plan `0099D300`, neighbours `007E11D0`, strafe `009CA870`, dive-bomb heading `009C4F80`. Steering every step; integrated into the pose (accumulates) |
| 2 | `plane_follow_law.cpp:192` `plane_follow_phase_a_009c0251` | `009C0251` | 48 chains (`kHalfPi*iw + th*iw`, `x28 - r*2.0*F(1-cos(phi))`) and `e > p*kGuardShare` in double (H) | main / 24 | formation follow law; branch thresholds flip |
| 3 | `ship_ai_station_keeping.cpp:58` `ship_ai_station_keeping_arm_009eda28` | `009EDA28` | double locals `ref`, `dir_x`, `dir_z`; `(ref*(ref/retardation))*kHalf` (H) | main / 24 | ship station keeping; deadband compares |
| 4 | `game_hosts_units.cpp:10083` `terrain_avoidance_0099f1c0` | `0099F1C0` `BSP_PilotBot_AvoidTerrain` | `sa - w_width*pitch*4.0`, `p_now - f*10.0`, double `d2` (H) | main / 24 | plane terrain avoidance thresholds |
| 5 | `game_hosts_ship_ai.cpp:13154` `transport_launch_craft_008206f0` | `008206F0` `BSP_TroopTransport_LaunchLandingCraft` | double placement locals, `x*right + z*forward + origin` (H) | main / 24 | one-shot craft positions, then persistent |
| 6 | `game_hosts_units.cpp:3864` `update_dive_bomb_approach` | `009C7A80` `BSP_BotTaskDiveBombApproach_Update` (the scan picked `006E3500` from the comment above; corrected by hand) | RNG draw (`00BD2F10`) plus a constant in double; double `atan2`/distances (H) | fixed-step / 24 | RNG-coupled release altitude |
| 7 | `hull_terrain_contact.cpp:235` `native_narrow_phase` | calling-thread transform before `00C53630` | `pos + row0*x + row1*y + row2*z` in double (H) | main / 24 | world-space hull points fed to the class B test |
| 8 | `game_hosts_units.cpp:7758` `run_follow_law_009bfee0_009bee30` | `009BFEE0` | `double dist = sqrt(dx*dx+dy*dy+dz*dz)`; `fct_tick_*` sums are diagnostics (H) | main / 24 | follow distance |
| 9 | `plane_wanderer.cpp:65` `plane_wanderer_fixed_step_007be060` | `007BE060` | `accel + d*c`, `roll_change_max*w*sign + roll_target` (H) | main / 24 | integrated every step (accumulates) |
| 10 | `ai_command_tick.cpp:564` `ai_formation_wedge_00a11690` | `00A11690` | rotation `s*d1 + c*d0` in double (H) | main / 24 | wedge slot offsets steer every member |
| 11 | `plane_follow_hold.cpp:64` `plane_follow_hold_command_009bee56` | `009BEE56` | double `sx`, `sy`, `sz`; yaw command chains (H) | main / 24 | steering command per tick |
| 12 | `plane_angular_velocity.cpp:171` `rotate_about_axis_equivalent` | `0085E4F0` | double `cos`/`sin`, double 3x3 `x*r0 + y*r3 + z*r6` (H) | main / 24 | plane attitude update (accumulates) |
| 13 | `submarine_model.cpp:363` `submarine_dive_step_00936dc0` | `00936DC0` | `vy + (candidate - vy)*curve`, `error*gain/v` (H) | main / 24 | depth physics (accumulates) |
| 14 | `game_hosts_units.cpp:9664` `vehicle_avoidance_007df4f0` | `007DF4F0` | 3-term dots, `-(dot/(s*s))`, `(d - rr)/s > 5.0` (H) | main / 24 | collision-band admission threshold |
| 15 | `ship_ai_approach_update.cpp:641` `ship_ai_approach_choose_standoff_009e6e80` | `009E6E80` | double `base`, `floor_value`; `(goal - standoff)/turn_radius` (H) | main / 24 | standoff range choice |
| 16 | `game_hosts_units.cpp:9890` `terrain_shape_band_0099cab0` | `0099CAB0` | `(lo - 1.1)*g + 1.1` with the image's double constants (H) | main / 24 | terrain band limits |
| 17 | `game_hosts_gunnery.cpp:6496` `run_gun_aim_and_fire` | `00953CC0` | `draw(...) * 3.14159265358979 / 180.0 + e`, `atan2(dx,dz) + sign*mag*pi/180` (H) | fixed-step / 24 | **RNG-coupled** AA gunner aim error |
| 18 | `gun_gravity_arc.cpp:178` `solve_gun_gravity_arc_00955630` | `00955630` | `dz*dz + dx*dx`, `(h - root)/(drop + drop)` in double (H) | main / 24 | launch elevation of every shell |
| 19 | `gun_dispersion.cpp:128` `gun_multi_bullet_ring_0072fefb` | `0072FEFB` | `i * 6.283185307179586 / n` (H) | main / 24 | per-bullet ring angle |
| 20 | `plane_level_bomb_task.cpp:200` `level_bomb_aim_tick_009b5c80` | `009B5C80` | `dy / (impact - bias)`, `(s - 0.5) + (s - 0.5)` (H) | main / 24 | bomb release decision |
| 21 | `torpedo_goaway_tick.cpp:79` `torpedo_goaway_tick_009d0f10` | `009D0F10` | `2.0*side*elapsed`; `range < breakoff - slack` (H) | main / 24 | break-off threshold |
| 22 | `ship_ai_path_follower.cpp:33` `ship_ai_path_tangent_chord_004f3970` | `004F3970` | `r*r/len`, `len*len - r*r` in double (H, L: `004F3A76 FMUL ST0,ST0`, `004F3A78 FDIV`, `004F3A83 FMULP`, `004F3A87 FSUBP` on the stack) | main / 24 | ship path tangents |
| 23 | `ship_motion.cpp` `ship_righting_rate_0092e9e1` | `0092E9E1` | `rate - error*(dt*10.0)` (H) | fixed-step / 24 | righting rate (accumulates). The rest of `ship_apply_steering_0092e8c0` is single-op and exact |
| 24 | `hull_terrain_contact.cpp:418` `HullTerrainContactSolver::step` | `00C549D0` `ManifoldUpdate` | 8 chains | main / 24 | manifold refresh on the calling thread |
| 25 | `game_hosts_scene_contents.cpp:3469` `tile_walk_00adf1b0`, `:3567` `node_walk_00ae9d80`, `:3646` `ray_walk_00aea2b0` | `00ADF1B0`, `00AE9D80`, `00AEA2B0` | 8-10 chains each | main / 24 | terrain queries used by ships, planes and line-of-sight; lane "other" but gameplay-relevant |

Also in the top 45 (not hand-read; scores in the JSON): `ship_ai_neighbour_avoid_box_refresh_009eafc0`,
`torpedo_reset_run_time_009d0160`, `landing_spacing_006c3f80`, `solve_spawn_placement_0094a140`,
`retreat_refresh_zone_009c8d40`, `gunfire_detect_0099ec40`, `landing_corridor_006c3b10`,
`plane_class_turn_rate_270_007db4d0`, `submarine_scan_point_00855420`,
`ship_ai_arm_final_separation_turn_009debb9`, `retreat_moveto_steer_009c9310`.

Rows the scan raised that are **not** findings: `seed_wrap_0085a3d0` (a float compared with
float-valued double constants, exact), `release_bomb_drop` (double sums feed drift statistics
only), `ship_ai_sub_attack_fire_step_009e9eb0` (timers are single-op then narrowed; two small
chains remain), every `GameUnitsHost::report`/`*_trace`/`*_print` row, and the many summary
fields declared `double` in `game_hosts_gunnery.cpp` (`*_sum`, `*_max`), which are diagnostics.
`summary.simulated_seconds` is a float.

Presentation (cc13, low priority): `default_unit_camera_update_chase_00431480` (26 chains),
`chase_camera_update_offsets_0042f800`, `update_ocean_beams_00ba2ff0`, `torpedo_view_step_00430020`,
`hud_formation_update_0053a8e0`, the anim-track tangents `00B75820`/`00B75A20`.

## Class C: Lua arithmetic

The image runs Lua on the main thread, so every `lua_Number` add, multiply and divide is an
x87 operation at PC=24: each script result is a double holding a 24-bit mantissa. The host
links stock 5.1.1 compiled with SSE2, so scripts get 53 bits. Effects: a script timer
accumulated as `t = t + dt` drifts differently and can cross a threshold on another frame;
`0.1 + 0.2` prints `0.30000001192093` in the image and `0.3` in the host (`%.14g`), which
matters wherever a number is concatenated into a key or a message. Integer arithmetic below
2^24 is identical. `math.sin`/`math.cos` go through the 53-bit transcendental group in both.
This is an inference from `docs/CC13_FPU_PRECISION.md` (Lua on the main thread; no SSE2
double arithmetic in game code); no Lua VM instruction was disassembled for this packet.

## What already matches

- Inline x87 outside Dyn (953 functions with x87 operations, of 1040 in all): the host's main
  thread is at PC=24, as the image's is.
- SSE `float` C++ (2164 functions with float evidence and no double evidence): exact for
  main-thread code by fact 1.
- Single-op double-then-narrow casts (several hundred): exact by fact 2.
- The Dyn calling-thread phases in float (`dyn_body_integrate_velocity_00c41550`, the
  contact report).

## For cc9

1. Class B is the largest single lever and is one switch away for the native part: bracket the
   contact phase (`run_world_contact_phase`, `HullTerrainContactSolver::step`) with
   `_controlfp_s(&old, _PC_53, _MCW_PC)` and a restore, and the inline x87 narrow phase runs at
   the image's precision. The C++ solver needs `double` intermediates up to each spill the
   listing makes (`FSTP dword`), the opposite of the main-thread rule.
2. Class A: the mechanical fix is to drop the `static_cast<double>` widening and double
   locals in main-thread code and write plain float expressions in the listing's operation
   order (fact 1 makes the spill pattern irrelevant). Keep a double only where the image
   loads a qword constant into a single operation, and then only for that operation.
   Priority: rows 1, 17 and 18 (RNG-coupled aim, ballistics), then the plane bot states
   inside row 1, then ship AI rows 3, 15, 22.
3. Class C needs a decision rather than a patch: a Lua build whose arithmetic rounds to float
   (for example `lua_Number` ops wrapped through `float`) would match the image's main thread.
4. Expect behaviour pairs to move when any of this lands; the changes are RNG-coupled through
   the shared stream (`00BD2F10`) wherever aim or spawn draws are involved.

## Uncertainties

1. The image thread of each address is a direct-call closure. Virtual calls and table dispatch
   are invisible to it, so `main_24` also covers code that actually runs in fixed-step jobs; under
   the policy both are 24 bits, so the class is unaffected, only the "image default mode" note.
2. Address association is heuristic (name suffix, then comment above, then first body
   comment); 126 of the 741 class A rows have no address. Row 6 was one wrong association (fixed
   in the JSON by hand; its `image_thread` still reflects the scanned address).
3. The chain counter treats every `static_cast<float>(...)` (and the local narrowing helpers
   `f`, `F`, `f32`, `narrow`, `to_float`, `store_float`, `round_to_float`, `terrain_f32`) whose
   argument mentions a double as a narrowing span and counts its operators; it does not parse C++
   types, so a span over float-only operands with an integer literal can be misread. Scores
   rank; they do not prove. Only the hand-checked rows are findings in their own right.
4. Load-time code (scene contents `004D4DF0`, Lua table loaders) may run on the loading thread
   (`00501620`), which is at 53 bits in the image; there host double would match. Not traced.
5. Class B assumes bsp_game calls the native narrow phase only from the main thread. No run
   read the control word inside `HullTerrainContactSolver`.

## Reproduction

`local/` of this worktree (not committed): `pr1_census.py` (parse, classify, thread join;
writes `local/pr1_census_raw.json`), `pr1_rank.py` (classes, scores, `topA`/`B`/`detail`
views), `pr1_window.py` (per-window scan of one long function), `pr1_lines.py`,
`pr1_emit.py` (writes the report JSON with the hand-check verdicts), `pr1_top.py`,
`pr1_check.py`. Base commit `42b49f76a`. `tools/verify_report_calls.py` on the JSON: no call
rows, nothing to check.

## Handoff

Done: the census, the two mismatch classes plus Lua, 30 hand checks (two against the
listing). Open, in order of value: (a) a run that logs `_controlfp_s` inside
`HullTerrainContactSolver::step` to close uncertainty 5; (b) hand-read the remaining top-45
rows and the window hot spots of row 1 per bot state; (c) trace which thread loads scene
contents and Lua tables (uncertainty 4); (d) disassemble one Lua VM arithmetic opcode to
turn class C from inference into listing evidence.
