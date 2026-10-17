# CC13 fix6/fix7: image-bit constant corrections behind OFF switches

cc13-fix67, 2026-10-17, under the cc9 gameplay lead's grant for the constants listed in
`docs/CC13_CONST_EXACT.md`. Every change selects at the declaration,
`kSwitch ? <image bits> : <old value>`, keeping the declared type. With every switch OFF the
compiled values are the old ones, bit for bit. Nothing was flipped; no game run was made.

## Switches

| switch | header | scope | default |
|---|---|---|---|
| `kPilotPitchInputScaleImageBound` | `include/bsp/pilot_controls.hpp` | fix6a, one constant | OFF |
| `kGuiCameraTargetZImageBound` | `include/bsp/gui_render_order.hpp` | fix6b, one constant | OFF |
| `kConstExactImageBitsBound` | `include/bsp/const_exact_image_bits.hpp` (new; included by the 16 headers below) | fix7, 32 constants | OFF |

## fix6a: `kPilotPitchInputScale` (pilot_controls.hpp)

Old: float `40000000` (2.0). New: float `3F060A92` (0.5235987901687622), the exact value of the
qword at 00CEC730 (`3FE0C15240000000`, pi/6 as a widened float). The 2.0 was the qword's low
dword `40000000` read as a float.

Image, in BSP_PlanePilotView_BuildPlayerCommand (00519520):

- 00519697 `FMUL qword [00CEC730]` scales a stick value by pi/6; the float result and
  `[ecx+0C64h]` go to 00438B10 (wrapped subtract), whose result is the pitch error.
- 005196B5 / 005196D1 gate the error against `[00CEC728]` (-pi/6, `BF060A92`) and
  `[00CEC724]` (pi/6, `3F060A92`); outside the gate the command is -1 / +1.
- 005196E6 `FDIV qword [00CEC730]`, then 005196EC `FADD dword [esp+8]` (the tangent term the
  existing comment describes as multiplied by 00D7A258 = 0.0), store as float, clamp to [-1, 1].

Host: `player_pitch_command_00519520` (src/pilot_controls.cpp) divides the error by
`kPilotPitchInputScale`, so the operation was already the divide; only the divisor was wrong.
The ON value is a float whose value equals the qword exactly. Under the main thread's 24-bit
precision control (docs/CC13_FPU_PRECISION.md) the image's FDIV rounds to a float significand,
which is what the float divide gives. Assumed main thread: 00519520 builds the player's command
from the pilot view. If it ran at 53 bits, the image would round the quotient twice
(53-bit then float store) where the host rounds once; that can differ by one float ulp at
rounding ties only. The 00519697 FMUL is upstream of the host function's `error` parameter and
is not part of this change. `player_pitch_command_00519520` has no caller in `src/` at present,
so ON has no effect on a current build until it is wired.

## fix6b: `kGuiCameraTargetZ` (gui_render_order.hpp)

Old: float `C61C2000` (-9992.0). New: float `C61C4000` (-10000.0), loaded from 00D19620 by
00AA3E7F `MOVSS xmm0,[00D19620]` and stored as the target z at 00AA3E87. Used by
`gui_camera_placement_00aa3e00` (src/gui_render_order.cpp). Presentation only.

## fix7: `kConstExactImageBitsBound`

"widened float" = the image holds a float value in a qword; the old host value was the
decimal. Literals are C++17 hex floats; the trailing comment of each declaration names the
image bits and the address.

| header | constant | type | old bits | new (image) bits | image address |
|---|---|---|---|---|---|
| pilot_controls.hpp | `kPilotPitchHalfRange` | float | 3F060AA6 (0.5236) | 3F060A92 | 00CEC724 |
| torpedo_aim_tick.hpp | `kProbeGateY0` | float | 3F5F66F9 | 3F5F66F3 | 00D05B40 |
| torpedo_aim_tick.hpp | `kProbeGateY1` | float | 3EB2B8C7 | 3EB2B8C3 | 00CE398C |
| torpedo_aim_tick.hpp | `kPitchClampLo` | float | BFB2B8C2 | BFB2B8C3 | 00D21318 |
| torpedo_aim_tick.hpp | `kPitchClampHi` | float | 3F5F66F9 | 3F5F66F3 | 00D057E0 m64 3FEBECDE60000000 (compare 009D1EBC) and 00D05B40 (store 009D1ECA) |
| plane_ground_ops.hpp | `kPlaneGroundYawTurnNumerator` | float | 3F5F66F9 | 3F5F66F3 | 00D057E0 m64 3FEBECDE60000000 |
| pilot_command_path.hpp | `kPilotCmdPitchZeroAngle` | float | 3E32B8C1 | 3E32B8C3 | 00CE3990 |
| ship_ai_approach_update.hpp | `kApproachThrottleSeedX1` | float | 3F9C61AA | 3F9C61AB | 00CE3988 |
| ship_ai_sector_scan.hpp | `kShipAiSectorSideBias` | float | 3EDF66F4 | 3EDF66F3 | 00D1F6C0 m64 3FDBECDE60000000 |
| dive_bomb_task.hpp | `dive_bomb_flyabove_constant::kHeightScale` | double | 3FE6666668000000 | 3FE6666660000000 | 00CEFFA0 |
| ship_ai_navigation.hpp | `kShipAiNavQuarterTurn` | double | 3FF921FB60004154 | 3FF921FB60000000 | 00CE3830 |
| ship_ai_navigation.hpp | `kShipAiNavBearingDeadzone` | double | 3FB999999999999A (0.1) | 3FB99999A0000000 | 00D7A3A0 |
| press_start_screen.hpp | `kPromptPulsePeriod` | double | 401921FB54442D18 (2*pi) | 401921FB60000000 | 00CE3828 |
| ship_ai_approach_update.hpp | `kApproachTargetRangeHigh` | double | 3FEB333333333333 (0.85) | 3FEB333340000000 | 00CF0B58 |
| ship_ai_approach_update.hpp | `kApproachLeaderShrink` | double | 3FE999999999999A (0.8) | 3FE99999A0000000 | 00CE3D40 |
| ship_ai_approach_update.hpp | `kApproachLeaderFloor` | double | 3FECCCCCCCCCCCCD (0.9) | 3FECCCCCC0000000 | 00D7A390 |
| ship_ai_approach_update.hpp | `kApproachCruiseLow` | double | 3FED70A3D70A3D71 (0.92) | 3FED70A3E0000000 | 00D04308 |
| ship_ai_approach_update.hpp | `kApproachCruiseHigh` | double | 3FEF5C28F5C28F5C (0.98) | 3FEF5C2900000000 | 00D219C8 |
| ship_ai_approach_update.hpp | `kApproachPhaseRate` | double | 3FC999999999999A (0.2) | 3FC99999A0000000 | 00CE3D10 |
| ship_ai_approach_update.hpp | `kApproachSlotKeepThreshold` | double | 3FEB333333333333 (0.85) | 3FEB333340000000 | 00CF0B58 |
| ship_ai_approach_update.hpp | `kApproachTurnDeadband` | double | BFB999999999999A (-0.1) | BFB99999A0000000 | 00CE3928 |
| hud_widgets.hpp | `kHudScoringRowStep` | double | 3F9EB851EB851EB8 (0.03) | 3F9EB851E0000000 | 00CEB690 |
| hud_widgets.hpp | `kHudScoringTeamGap` | double | 3F8EB851EB851EB8 (0.015) | 3F8EB851E0000000 | 00CEE6C8 |
| lua_spawn_cc13.hpp | `kSpawnQuarterTurn` | double | 3FF921FB54442D18 (pi/2) | 3FF921FB60000000 | 00CE3830 |
| plane_level_bomb_task.hpp | `level_bomb::kInRangeScale` | double | 3FF6666666666666 (1.4) | 3FF6666660000000 | 00D045F0 |
| scene_contents_hosts.hpp | `kSceneCloudDistanceEpsilon` | double | 3DDB7CDFD70A3D71 (float 1e-10 widened) | 3DDB7CDFD9D7BDBB (the double 1e-10) | 00CE3820 |
| ship_ai_nav_block_ctor.hpp | `kShipAiNavBlockStopHullShare` | double | 3FD999999999999A (0.4) | 3FD99999A0000000 | 00CE65D0 |
| ship_ai_nav_block_ctor.hpp | `kShipAiNavBlockYawFloorCompare` | double | 3FB1DF46A2666667 | 3FB1DF46A0000000 | 00D218B8 |
| ship_ai_states.hpp | `kShipAiThrottleDeadzone` | double | 3FA999999999999A (0.05) | 3FA99999A0000000 | 00D7A270 |
| ship_ai_throttle_ring.hpp | `kShipAiRudderAuthorityScale` | double | 3FF3333333333333 (1.2) | 3FF3333340000000 | 00CEC160 |
| ship_ai_throttle_ring.hpp | `kShipAiRudderSpeedFloor` | double | 3FD999999999999A (0.4) | 3FD99999A0000000 | 00CE65D0 |
| ship_ai_throttle_ring.hpp | `kShipAiRingHopThrottleDeadband` | double | 3FA999999999999A (0.05) | 3FA99999A0000000 | 00D7A270 |

The approach constants (`ship_ai_approach_update.hpp`) had no commit since the sweep doc was
written (last change b7b94ebf3/affcc5419, 2026-10-12), so cc9-ships56 had not touched them.

## Skipped

- Judged intentional in the sweep doc (each use casts to float first, which gives the image's
  float exactly): `kDiveAltitudeDecay`, `kImpactPointFallTimeBias`,
  `kFixedSimulationStepSeconds`, `kShipAiDangerBand`.
- Unused in `src/`: `kSpawnDegreesToRadians` (a false hit), `kGenerateObjectHeadingSentinelFloor`,
  `kTerrainGridDefaultAngleRadians`.
- `kHudRootRowGap`, `kHudRootCircleYOffset` (hud_updates.hpp): the opposite class. The image
  adds a true double (1/72, 1/120) and the host declares a float. Correcting them changes the
  declared type and the arithmetic at the use sites in src/hud_updates.cpp, which cannot be
  done with a selection at the declaration alone and keep OFF bit-identical; left for the
  owning lane. Effect: at most one float ulp in HUD row positions.

## Verification

- `python tools/const_width_sweep.py --exact` on a copy of `include/bsp/*.hpp` with all three
  switches resolved ON (local scripts fix67_resolve.py / fix67_sweep.py): 1471 parsed, 9
  mismatches, exactly the nine skipped above; all 34 targeted mismatches clear. Resolved OFF:
  43 mismatches, line-for-line identical to the sweep before the edits.
- The tool does not evaluate a switch identifier, so a plain `--all --exact` run on the edited
  headers now counts these 34 declarations as unparsed (1437 parsed, 34 unparsed, 9
  mismatches). Resolve the switches as above to re-check them.
- `./scripts/build.ps1`, both test suites passed, before each commit.

## What a pair should show

- fix6a ON: the player pitch command inside the +/-30 degree gate becomes error/(pi/6)
  instead of error/2, about 3.8 times the old response, saturating at |error| = pi/6 instead of
  never reaching +/-1 inside the gate. No effect on a current build: the host function has no
  caller yet.
- fix6b ON: the GUI camera target moves from z = -9992 to -10000; the view direction is
  unchanged up to float rounding of the look-at, so GUI output should be pixel-identical or
  differ at most by rounding.
- fix7 ON: ulp-level changes only. Expect compare-boundary flips to be rare; anything that
  accumulates (the `kPromptPulsePeriod` fmod phase drifts by about 1.7e-7 per period; the
  approach and ship-AI doubles feed double arithmetic) can move later values by rounding and,
  through the shared RNG 00BD2F10, can decouple a pair after the first flipped compare.
  `kInRangeScale` is on the level-bomb path (game_hosts_units.cpp:24327), so judge any
  level-bomb deltas there with that in mind.
