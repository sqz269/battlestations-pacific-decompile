#pragma once

// cc13-tier1 (Tier 3 row A): 006C5050's flag-1 arm, the airborne air-ops launch.
// Lua LaunchAirBaseSlot(base, slot, true) (00896750) -> 006CA8FA -> 006C7490(slot, 1)
// -> 006C5050 with its char flag set. Only chg_6_forcez.lua:174-177 (CHG06, at
// FirstRun) reaches it in this installation. The other flag-true calls,
// scripts/missions/COTP-USN/usn_01_battle_of_eastern_solomons.lua:1172/1176 (the
// Wildcat respawn) and usn_04_defend_guadalcanal.lua:741, are in scripts no mission
// loads: the menu ids USN01 and USN04 run USN/usn_1_marshall.lua and
// USN/usn_19_coralus.lua (missiontree.lua via tools/mission_script_reach.py); the
// COTP-USN usn_01/usn_04 .scn files name StageScript "USN\\usn_01_battle_of_
// eastern_solomons" / "USN\\usn_04_Defend_Guadalcanal", which do not exist under
// scripts/missions/USN, and no missiontree row or other text file names those
// scenes; the PRCPUS copies that do run (prcp_01_eastern, prcp_04_defend) have no
// LaunchAirBaseSlot call.
//
// Image, flag set (006C5050):
//  - 006C51C2-006C51DD: bag `State` = (-(flag != 0) & 6) + 1 = 7;
//  - 006C51E2-006C5207: bag `VelocitySI` = vehicle class+18Ch (TravelSpeed);
//  - 006C5294: REP MOVSD copies the owner's world matrix (owner+CCh, 10h dwords);
//  - 006C5298-006C52C5: 0042E740 (tuning singleton), then
//    00BD2F10(ECX = 1, 100.0 [00CE3D08], 300.0 [00CE3AE8]), FADD dword [tuning+210h]
//    (Dynamics/Ceiling), FSTP dword into the copy's matrix[13]. The world y is
//    REPLACED by draw + Ceiling; the owner's own height is not added;
//  - 006C5314: the block+38h brake store is skipped (flag set).
// 007F4C0B then reads State 7 (not < 2), so 007F1C00 leaves +408h clear and 007F4DB0
// runs 007F2920, the airborne station placement.
//
// The host's creator ignored request.state: bag_state 1, owner-origin height, no draw.
// The squadron then started inside its base (007CC820 state 1).

namespace bsp {

// True: the creator takes request.state. On 7 the squadron record's bag_state is 7,
// so pass C skips the inside start and runs place_squadron_members_007f2920. The
// world y is Dynamics/Ceiling plus the draw below. `VelocitySI` needs no
// field: create_units already seeds every plane at class+18Ch TravelSpeed along its
// nose (labelled: the identity axes stand for the owner's rotation rows, as on
// the flag-0 path). False: bag_state 1 and the owner-origin height, as before.
// ON since cc9-physics26 (docs/OCEAN_WAVE_FIELD.md section 115, CHG06 pair).
inline constexpr bool kAirOpsLaunchFlagAirborneBound = true;

// True: the 00BD2F10(stream 1, 100.0f, 300.0f) draw on the shared stream (the
// gunnery host's stream-1 generator), taken where the image takes it: after the
// bag and owner-matrix copy, before 004F0AD0 (create_units). Then y =
// float(draw + Ceiling) (24-bit FADD, FSTP float). False: the draw is not taken and
// is counted. y is Ceiling with NO random offset (SUBSTITUTION, labelled; no
// midpoint is substituted because the image has no constant there).
// ON since cc9-physics26 (section 115).
inline constexpr bool kAirOpsLaunchFlagAltitudeDrawBound = true;

static_assert(!kAirOpsLaunchFlagAltitudeDrawBound || kAirOpsLaunchFlagAirborneBound,
    "the 006C52BA draw belongs to the airborne arm; switch that on first");

inline constexpr float kAirOpsLaunchAltitudeDrawLow = 100.0f;    // 00CE3D08
inline constexpr float kAirOpsLaunchAltitudeDrawHigh = 300.0f;   // 00CE3AE8

}  // namespace bsp
