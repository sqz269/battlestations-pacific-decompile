# CC13 exact-bit constant sweep

Generated 2026-10-17 by `python tools/const_width_sweep.py --all --exact` (cc13-tool1, requested by the
cc9 gameplay lead after cc13-verify9). No header or source constant was edited; this is a list for the
owning lanes.

## What the mode checks

For every `constexpr float|double NAME = <literal expression>; // <8-hex address>` in `include/bsp/*.hpp`
whose address has load sites (absolute `[disp32]` operands found by disassembling from every known
function start), it compares, with no tolerance:

- host: the bits the compiler stores for the declared type. A float literal is rounded once from its
  decimal (not via a double); hex literals, `static_cast<float|double>(...)` and literal arithmetic are
  evaluated with C++ typing (float op float stays float).
- image: the bytes at the constant's address read at each load width found (m32 or m64), widened as the
  load widens them (FLD m32 gives the float's exact value).

Run: 1471 constants parsed (0 unparsed), 23 without load sites, 1448 width checks, **43 mismatches**
in 21 headers. The calibration cases from cc13-verify9 are all reported: the seven approach doubles
(0.85 twice, 0.8, 0.9, 0.92, 0.98, 0.2, -0.1), `kApproachThrottleSeedX1`, `kProbeGateY0/Y1`,
`kPilotPitchHalfRange`, `kShipAiSectorSideBias`. Full machine output: `local/output/const_exact.txt`.

Classes: "decimal of a widened float" = a double declared as the short decimal (0.85) where the image
holds the float's exact widened value (0.8500000238418579). "declared float, loads read a double" = the
type is narrow; the float ulp figure is given when the image double holds a float value.

Caveats. The load-site column lists the first three instructions in the whole image that read the
address; a shared constant (00D7A270 = 0.05f widened) is read by many functions, so a listed site is not
necessarily the one the constant's own function uses. The assessment column was checked by reading the
host use (`git grep -w NAME -- src`) for every row and the image instruction for the large ones;
"no effect" means the use converts the constant to float before any arithmetic, which yields exactly the
image's float. Signed zeros (-0.0f at 00D7A208) are handled and none mismatch.

## Top items (largest effect first)

| header:line | constant | host bits | image bits | note |
|---|---|---|---|---|
| pilot_controls.hpp:85 | kPilotPitchInputScale | float 40000000 (2.0) | 00CEC730 m64 3FE0C15240000000 (0.5235988) | 005196E6 FDIV qword: the binary divides by pi/6, not 2 |
| gui_render_order.hpp:185 | kGuiCameraTargetZ | float C61C2000 (-9992) | 00D19620 m32 C61C4000 (-10000) | 00AA3E7F MOVSS stores -10000 |
| pilot_controls.hpp:84 | kPilotPitchHalfRange | float 3F060AA6 | 00CEC724 m32 3F060A92 | 20 ulp, gate compare |
| torpedo_aim_tick.hpp:66/88 | kProbeGateY0, kPitchClampHi | float 3F5F66F9 | 00D05B40 m32 / 00D057E0 m64, both 3F5F66F3 | 6 ulp |
| torpedo_aim_tick.hpp:68 | kProbeGateY1 | float 3EB2B8C7 | 00CE398C m32 3EB2B8C3 | 4 ulp |
| plane_ground_ops.hpp:105 | kPlaneGroundYawTurnNumerator | float 3F5F66F9 | 00D057E0 m64 3FEBECDE60000000 | 6 float ulp |
| scene_contents_hosts.hpp:86 | kTerrainGridDefaultAngleRadians | float 3E32B8BE | 00CE3990 m32 3E32B8C3 | 5 ulp, unused in src |
| pilot_command_path.hpp:108 | kPilotCmdPitchZeroAngle | float 3E32B8C1 | 00CE3990 m32 3E32B8C3 | 2 ulp |
| dive_bomb_task.hpp:848 | kHeightScale | double 3FE6666668000000 | 00CEFFA0 m64 3FE6666660000000 | neither 0.7f nor 0.7 |
| ship_ai_navigation.hpp:173 | kShipAiNavQuarterTurn | double 3FF921FB60004154 | 00CE3830 m64 3FF921FB60000000 | truncated literal |
| press_start_screen.hpp:50 | kPromptPulsePeriod | double 401921FB54442D18 (2*pi) | 00CE3828 m64 401921FB60000000 | fmod period; drift grows |
| ship_ai_approach_update.hpp:253 | kApproachThrottleSeedX1 | float 3F9C61AA | 00CE3988 m32 3F9C61AB | 1 ulp |
| torpedo_aim_tick.hpp:87 | kPitchClampLo | float BFB2B8C2 | 00D21318 m32 BFB2B8C3 | 1 ulp |
| ship_ai_sector_scan.hpp:235 | kShipAiSectorSideBias | float 3EDF66F4 | 00D1F6C0 m64 3FDBECDE60000000 | 1 float ulp |
| ship_ai_approach_update.hpp:200-273 | seven approach doubles | decimal doubles | widened floats | used in double arithmetic |

Probably intentional (no effect on the result): `kDiveAltitudeDecay`, `kImpactPointFallTimeBias`,
`kFixedSimulationStepSeconds`, `kShipAiDangerBand` (each use casts to float first). Not used by any
source: `kSpawnDegreesToRadians` (a quotient of two image constants, a false hit),
`kGenerateObjectHeadingSentinelFloor` (comment reference only), `kTerrainGridDefaultAngleRadians`.

## Full list by header

### include/bsp/dive_bomb_task.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 165 | `kDiveAltitudeDecay` | double `3FA999999999999A` (0.05) | `00D7A270` m64 `3FA99999A0000000` (0.05000000074505806) | 107374182 double | 004C6F3D, 0059CB4A, 0059D44B | decimal of a widened float | INTENTIONAL/no effect: the use casts to float first, which gives the image's float exactly. |
| 170 | `kImpactPointFallTimeBias` | double `3FB999999999999A` (0.1) | `00D7A3A0` m64 `3FB99999A0000000` (0.10000000149011612) | 107374182 double | 004226C1, 0042C364, 0042F787 | decimal of a widened float | INTENTIONAL/no effect: cast to float before use. |
| 848 | `kHeightScale` | double `3FE6666668000000` (0.7000000029802322) | `00CEFFA0` m64 `3FE6666660000000` (0.699999988079071) | 134217728 double | 0059ABEA, 00681FE7, 00682D0A | decimal of a widened float | REAL: neither the float 0.7f nor 0.7; used directly in a double multiply-add. |

### include/bsp/gui_render_order.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 185 | `kGuiCameraTargetZ` | float `C61C2000` (-9992.0) | `00D19620` m32 `C61C4000` (-10000.0) | 8192 float | 0092D248, 0092D2AC, 00935CEF |  | REAL: 00AA3E7F MOVSS [00D19620] stores -10000.0 to target z; the host stores -9992.0. |

### include/bsp/hud_updates.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 50 | `kHudRootRowGap` | float `3C638E39` (0.013888888992369175) | `00CEBDD0` m64 `3F8C71C71C71C71C` (0.013888888888888888) | 59652324 double | 00511E08, 005160AA, 00516154 | declared float, loads read a double | Minor: the image adds a true double 1/72 under 24-bit PC; the host adds float(1/72). 1-ulp results possible. |
| 51 | `kHudRootCircleYOffset` | float `3C088888` (0.00833333283662796) | `00CF0DE8` m64 `3F81111111111111` (0.008333333333333333) | 286331153 double | 005B6786, 005B8644, 005B8AFE | declared float, loads read a double | Minor: same as kHudRootRowGap (1/120). |

### include/bsp/hud_widgets.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 71 | `kHudScoringRowStep` | double `3F9EB851EB851EB8` (0.03) | `00CEB690` m64 `3F9EB851E0000000` (0.029999999329447746) | 193273528 double | 005068F6, 005069F5, 00506AF6 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 72 | `kHudScoringTeamGap` | double `3F8EB851EB851EB8` (0.015) | `00CEE6C8` m64 `3F8EB851E0000000` (0.014999999664723873) | 193273528 double | 0055E341, 005D3E3C, 0060361B | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/in_mission_subsystem_tick.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 82 | `kFixedSimulationStepSeconds` | double `3FA999999999999A` (0.05) | `00D7A270` m64 `3FA99999A0000000` (0.05000000074505806) | 107374182 double | 004C6F3D, 0059CB4A, 0059D44B | decimal of a widened float | INTENTIONAL/no effect: every use casts to float first. |

### include/bsp/lua_binding_entity_lookup.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 157 | `kGenerateObjectHeadingSentinelFloor` | double `401921FB5FFFFFFF` (6.283185482025146) | `00CE3828` m64 `401921FB60000000` (6.2831854820251465) | 1 double | 00414E98, 00414ED3, 00415C95 | decimal of a widened float | Unused in src (comment reference only); 1 double ulp. |

### include/bsp/lua_spawn_cc13.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 76 | `kSpawnDegreesToRadians` | double `3F91DF46A2529D39` (0.017453292519943295) | `00CE3D28` m64 `400921FB60000000` (3.1415927410125732) | 33568866272371399 double | 0042FAEB, 0042FCD8, 0042FD85 |  | False hit, unused: the value is the quotient 00CE3D28/00CE3D20 and no source uses it. |
| 77 | `kSpawnQuarterTurn` | double `3FF921FB54442D18` (1.5707963267948966) | `00CE3830` m64 `3FF921FB60000000` (1.5707963705062866) | 196858600 double | 00414E84, 00414EC1, 00415C81 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/pilot_command_path.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 108 | `kPilotCmdPitchZeroAngle` | float `3E32B8C1` (0.1745329052209854) | `00CE3990` m32 `3E32B8C3` (0.1745329350233078) | 2 float | 0042478A, 007D131D, 0080AFDE |  | REAL at the compare boundary (2 ulp). |

### include/bsp/pilot_controls.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 84 | `kPilotPitchHalfRange` | float `3F060AA6` (0.5235999822616577) | `00CEC724` m32 `3F060A92` (0.5235987901687622) | 20 float | 005196D1, 006C3E0C, 006C5FB4 |  | REAL: 0.5236f is not pi/6; gate compare at 005196D1 COMISS. |
| 85 | `kPilotPitchInputScale` | float `40000000` (2.0) | `00CEC730` m64 `3FE0C15240000000` (0.5235987901687622) | 8794640249520128 double | 00519697, 005196E6, 007B5078 | declared float, loads read a double holding a float, 16381294 float ulp | REAL, large: 005196E6 is FDIV qword [00CEC730] = /0.5235988 (and 00519697 FMUL by it); the host divides by 2.0, the float in the low dword. |

### include/bsp/plane_ground_ops.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 105 | `kPlaneGroundYawTurnNumerator` | float `3F5F66F9` (0.8726649880409241) | `00D057E0` m64 `3FEBECDE60000000` (0.8726646304130554) | 3221225472 double | 007B4BA3, 007DA586, 009A2306 | declared float, loads read a double holding a float, 6 float ulp | REAL: float numerator 6 ulp off the widened image value. |

### include/bsp/plane_level_bomb_task.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 60 | `kInRangeScale` | double `3FF6666666666666` (1.4) | `00D045F0` m64 `3FF6666660000000` (1.399999976158142) | 107374182 double | 0078FB42, 0079166A, 007E048D | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/press_start_screen.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 50 | `kPromptPulsePeriod` | double `401921FB54442D18` (6.283185307179586) | `00CE3828` m64 `401921FB60000000` (6.2831854820251465) | 196858600 double | 00414E98, 00414ED3, 00415C95 | decimal of a widened float | REAL and growing: fmod by 2*pi double instead of the widened float; the phase drifts by n*1.7e-7 after n periods. |

### include/bsp/scene_contents_hosts.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 86 | `kTerrainGridDefaultAngleRadians` | float `3E32B8BE` (0.17453286051750183) | `00CE3990` m32 `3E32B8C3` (0.1745329350233078) | 5 float | 0042478A, 007D131D, 0080AFDE |  | Unused in src; 5 ulp off the image ten-degree float. |
| 166 | `kSceneCloudDistanceEpsilon` | double `3DDB7CDFD70A3D71` (9.999999939225291e-11) | `00CE3820` m64 `3DDB7CDFD9D7BDBB` (1e-10) | 47022154 double | 00414B71, 00414BC1, 00414C71 |  | Compare boundary only: the host holds float(1e-10) widened, the image the double 1e-10. |

### include/bsp/ship_ai_approach_update.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 200 | `kApproachTargetRangeHigh` | double `3FEB333333333333` (0.85) | `00CF0B58` m64 `3FEB333340000000` (0.8500000238418579) | 214748365 double | 005AEE9B, 007E4155, 009B2583 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 201 | `kApproachLeaderShrink` | double `3FE999999999999A` (0.8) | `00CE3D40` m64 `3FE99999A0000000` (0.800000011920929) | 107374182 double | 00430C4A, 00536BAD, 0059ABD6 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 202 | `kApproachLeaderFloor` | double `3FECCCCCCCCCCCCD` (0.9) | `00D7A390` m64 `3FECCCCCC0000000` (0.8999999761581421) | 214748365 double | 0059D1DF, 0059D1FF, 005AEB67 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 203 | `kApproachCruiseLow` | double `3FED70A3D70A3D71` (0.92) | `00D04308` m64 `3FED70A3E0000000` (0.9200000166893005) | 150323855 double | 0078636B, 009E714E | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 204 | `kApproachCruiseHigh` | double `3FEF5C28F5C28F5C` (0.98) | `00D219C8` m64 `3FEF5C2900000000` (0.9800000190734863) | 171798692 double | 009E715A | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 240 | `kApproachPhaseRate` | double `3FC999999999999A` (0.2) | `00CE3D10` m64 `3FC99999A0000000` (0.20000000298023224) | 107374182 double | 0042F7AD, 00458B16, 00458C3F | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 243 | `kApproachSlotKeepThreshold` | double `3FEB333333333333` (0.85) | `00CF0B58` m64 `3FEB333340000000` (0.8500000238418579) | 214748365 double | 005AEE9B, 007E4155, 009B2583 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 253 | `kApproachThrottleSeedX1` | float `3F9C61AA` (1.2217304706573486) | `00CE3988` m32 `3F9C61AB` (1.2217305898666382) | 1 float | 00424842, 009A80AC, 009A8495 |  | REAL: 1 ulp (seed value). |
| 273 | `kApproachTurnDeadband` | double `BFB999999999999A` (-0.1) | `00CE3928` m64 `BFB99999A0000000` (-0.10000000149011612) | 107374182 double | 00422CC2, 0042C6B6, 0071D070 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/ship_ai_nav_block_ctor.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 47 | `kShipAiNavBlockStopHullShare` | double `3FD999999999999A` (0.4) | `00CE65D0` m64 `3FD99999A0000000` (0.4000000059604645) | 107374182 double | 00487E39, 004D8D10, 004D8D6D | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 60 | `kShipAiNavBlockYawFloorCompare` | double `3FB1DF46A2666667` (0.06981317009776832) | `00D218B8` m64 `3FB1DF46A0000000` (0.06981316953897476) | 40265319 double | 009E4587 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/ship_ai_navigation.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 169 | `kShipAiNavBearingDeadzone` | double `3FB999999999999A` (0.1) | `00D7A3A0` m64 `3FB99999A0000000` (0.10000000149011612) | 107374182 double | 004226C1, 0042C364, 0042F787 | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 173 | `kShipAiNavQuarterTurn` | double `3FF921FB60004154` (1.57079637051) | `00CE3830` m64 `3FF921FB60000000` (1.5707963705062866) | 16724 double | 00414E84, 00414EC1, 00415C81 | decimal of a widened float | REAL: truncated decimal 1.57079637051, 16724 double ulp; used in double arithmetic. |

### include/bsp/ship_ai_obstacle_tables.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 163 | `kShipAiDangerBand` | double `3FA999999999999A` (0.05) | `00D7A270` m64 `3FA99999A0000000` (0.05000000074505806) | 107374182 double | 004C6F3D, 0059CB4A, 0059D44B | decimal of a widened float | INTENTIONAL/no effect: cast to float before use. |

### include/bsp/ship_ai_sector_scan.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 235 | `kShipAiSectorSideBias` | float `3EDF66F4` (0.4363323450088501) | `00D1F6C0` m64 `3FDBECDE60000000` (0.4363323152065277) | 536870912 double | 009A3DEA, 009A8ECC, 009AC68C | declared float, loads read a double holding a float, 1 float ulp | REAL: 1 float ulp; widened and added in double at the use. |

### include/bsp/ship_ai_states.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 208 | `kShipAiThrottleDeadzone` | double `3FA999999999999A` (0.05) | `00D7A270` m64 `3FA99999A0000000` (0.05000000074505806) | 107374182 double | 004C6F3D, 0059CB4A, 0059D44B | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/ship_ai_throttle_ring.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 97 | `kShipAiRudderAuthorityScale` | double `3FF3333333333333` (1.2) | `00CEC160` m64 `3FF3333340000000` (1.2000000476837158) | 214748365 double | 005165BB, 0051693C, 0060CF1E | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 100 | `kShipAiRudderSpeedFloor` | double `3FD999999999999A` (0.4) | `00CE65D0` m64 `3FD99999A0000000` (0.4000000059604645) | 107374182 double | 00487E39, 004D8D10, 004D8D6D | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |
| 177 | `kShipAiRingHopThrottleDeadband` | double `3FA999999999999A` (0.05) | `00D7A270` m64 `3FA99999A0000000` (0.05000000074505806) | 107374182 double | 004C6F3D, 0059CB4A, 0059D44B | decimal of a widened float | REAL but small: used in double arithmetic or a double compare, so the ~1e-8 relative difference reaches the result at rounding or compare boundaries. |

### include/bsp/torpedo_aim_tick.hpp

| line | constant | host (declared type, bits, value) | image (constant, load width, bits, value) | ulp | load sites (first 3) | class | assessment |
|---|---|---|---|---|---|---|---|
| 66 | `kProbeGateY0` | float `3F5F66F9` (0.8726649880409241) | `00D05B40` m32 `3F5F66F3` (0.8726646304130554) | 6 float | 007C5EB0, 009A836A, 009D19EE |  | REAL: probe gate value 6 ulp off. |
| 68 | `kProbeGateY1` | float `3EB2B8C7` (0.34906598925590515) | `00CE398C` m32 `3EB2B8C3` (0.3490658700466156) | 4 float | 004247E6, 006C5DAF, 007C5EC4 |  | REAL: probe gate value 4 ulp off. |
| 87 | `kPitchClampLo` | float `BFB2B8C2` (-1.3962633609771729) | `00D21318` m32 `BFB2B8C3` (-1.3962634801864624) | 1 float | 009D1EA6, 009D1EB0 |  | REAL: clamp bound and stored value (009D1EA6/009D1EB0) 1 ulp off. |
| 88 | `kPitchClampHi` | float `3F5F66F9` (0.8726649880409241) | `00D057E0` m64 `3FEBECDE60000000` (0.8726646304130554) | 3221225472 double | 007B4BA3, 007DA586, 009A2306 | declared float, loads read a double holding a float, 6 float ulp | REAL: 009D1EBC compares against the m64 at 00D057E0 and 009D1ECA stores the float at 00D05B40; both are 0x3F5F66F3, the host uses 0x3F5F66F9. |
