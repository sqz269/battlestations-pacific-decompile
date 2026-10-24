# cc13 fixture pass 5: game-logic float leaves

Packet `cc13-fixture5` (2026-10-24). This pass uses the harness of docs/CC13_FIXTURE_PASS_2.md:
`tools/verify_cc13_leaf_reference.py` audits the original bytes and relocations, and
`tests/cc13_leaf_fixture_tests.cpp` (CTest `cc13_leaf_differential`, function `run_fixture5`) runs
each original body next to its reconstruction on a fixed pseudo-random input set mixed with edge
values (signed zeros, a quiet NaN, denormals, FLT_MIN, FLT_MAX, infinities, 1e30) and branch
boundaries, at 53-bit and at 24-bit x87 precision control.

## Selection and audit

Nineteen game-logic float leaves whose reconstructions have no unresolved calls. 009B94E0 calls
00438B10 (already in the audit); the test passes `bsp::wrapped_angle_subtract_00438b10`.
The audit now covers 25107 bytes and 131 relocations. The pass adds 14 doubles and 9 floats to the
data list. 00D7A264 lies inside the existing 00D7A260 range.

## Deciding precision

Every routine here runs on the main thread or has no recorded caller, so the 24-bit pass decides
(docs/CC13_FPU_PRECISION.md).

| Address | Routine | Thread evidence | 24-bit | 53-bit | Result |
| --- | --- | --- | --- | --- | --- |
| 00BD4BF0 | matrix4x4 equals | TaggedValue_Equals | 3000/3000 | 3000/3000 | promoted |
| 009FA510 | world edge overrun | Bot_FlyToPointHeading | 4000/4000 | 3606/4000 | promoted |
| 00681F40 | game near world edge | HudWarningScreen, Bot_FlyToPoint | 4000/4000 | 4000/4000 | promoted |
| 0059C9B0 | world point beyond map margin | HUD map arrow, plane and ship countdowns | 4000/4000 | 4000/4000 | promoted |
| 006FBCE0 | elevator platform start travel | MotherShipSite elevator message | 3945/4000 | 3098/4000 | pinned, 55 |
| 0078FAF0 | movie camera approach offset | HudMovieCamera_Update | 3995/4000 +1 NaN-only | 3584/4000 | pinned, 4 + 1 |
| 0078FCF0 | movie camera ease in-out quad | keyframe weights, camera path | 4000/4000 | 4000/4000 | promoted |
| 0080E000 | unit AI order slot step (ON arm) | ShipAi_ControlsStep | 4000/4000 | 4000/4000 | OFF switch |
| 004F3030 | segment lerp | no recorded caller | 3998/4000 | 4000/4000 | pinned, 2 |
| 004F40C0 | is left turn | no recorded caller | 3966/4000 | 3966/4000 | pinned, 34 |
| 004F2FD0 | circle contains point | undefined code 004F4AE0 | 3995/4000 | 3953/4000 | pinned, 5 |
| 004439B0 | time of day hours | Lua SetDayTime | 4000/4000 | 2631/4000 | promoted |
| 006E2490 | projectile shot speed | vtable slot | 4000/4000 | 4000/4000 | promoted |
| 009B94E0 | split-S roll step | bot task tick (segment 67) | 3960/4000 | 3168/4000 | pinned, 40 |
| 009B9680 | maneuver roll completed | bot task tick (segment 67) | 4000/4000 | 4000/4000 | promoted |
| 00786030 | average peer quality | session add/remove peer | 4000/4000 | 4000/4000 | promoted |
| 00815440 | build unit order record | UnitInstance_IssueOrder | 4000/4000 | 4000/4000 | promoted |
| 007B9620 | plane altitude sink predicate | no recorded caller | 4000/4000 | 4000/4000 | promoted |
| 00727C30 | index of readiest barrel | HudWeaponGroup layout | 4000/4000 | 4000/4000 | promoted |

Each pinned row matches on all inputs with `BSP_FIXTURE_NO_EDGES` set. The mismatches come only
from edge inputs. The 24-bit model holds intermediates in float, so they overflow to infinity or
denormalize (underflow, for 004F2FD0's r*r) where the x87 stack keeps its extended exponent. This is
the same deviation that was pinned for 004F3DA0 in pass 2.

## Changes made to reach a match

- 006FBCE0 (bsp_game, not wired): it read its constants by dereferencing the absolute image
  addresses 00CE65D0, 00D7A358 and 00D7A238, which would fault outside the image. It is now
  modelled per operation in float, with the constants as values.
- 0078FAF0 (not wired): now modelled per operation in float. It takes the subtract arm when
  `!(target >= current)`, so an unordered compare also takes it (JC at 0078FB50). The double model
  differed on 492 of 4000 inputs.
- 004F2FD0 (not wired): `r*r > dx*dx + dy*dy` in float. The old model squared r in double.
- 004439B0 (Lua SetDayTime): each FDIV, FIADD and FADDP now rounds to float. The double sum
  differed on 1362 of 4000 inputs.
- 0078FCF0: a public wrapper `bsp::movie_camera_ease_in_out_quad_0078fcf0` was added; the
  behaviour is unchanged.
- 009B94E0 and 009B9680: host-free overloads were added in src/cc13_seg67b.cpp, and the host
  versions now delegate to them. The behaviour is unchanged.
- 0080E000 is live (game_hosts_ship_ai.cpp `prologue_0080e000`, called from 009ED6DE). A new OFF
  switch, `kUnitAiOrderResetFieldsCc13Bound` (include/bsp/ship_ai_navigation.hpp), selects the
  image's behaviour:
  - The reset (0080E03D..0080E064) stores only value_00, field_04, field_08 = 1000, flag_0c and
    field_18 of each sub-record. field_10 and field_14 (+18h/+1Ch, +34h/+38h) keep their values;
    the OFF arm clears the whole sub-record.
  - The NaN order follows the image:
    - an unordered timer skips the countdown (COMISS / JC);
    - an unordered gap takes the negative arm;
    - an unordered step snaps to the target.

  The fixture calls the ON arm. The switch compiles ON (`tools/off_switch_on_build.py`).
