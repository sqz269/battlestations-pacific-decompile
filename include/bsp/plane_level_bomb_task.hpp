#ifndef BSP_PLANE_LEVEL_BOMB_TASK_HPP
#define BSP_PLANE_LEVEL_BOMB_TASK_HPP

// The level-bomb bot task (kind 4): factory 009B9030 -> constructor 009B7990
// (operator_new 6F8h), approach 009B75E0 at task+3F8h over the approach base
// 009B44F0, per-tick arm 009B8B50 (vtable 00D20210 slot +64h), the transition
// rule 009B88F0 and the seven states 009B42D0 names. docs/SQUADRON_LAND_TASK.md
// section 5ee carries the evidence; docs/BOT_TASKS.md owns the task object and
// docs/BOT_TASK_STATES.md the state family.
//
// Every name is a hypothesis, not a recovered symbol. The structs are host
// shapes, not binary layouts; the offset in each comment is the native one
// (task-relative for states, approach-relative for approach fields).
//
// Coverage: complete for the arm 009B8B50, the rule 009B88F0 and 009B8820, the
// break-off 009B8D80, the cruise profile 009B8C90, the approach update 009B7C90
// (paratrooper branch 009B7D7D-009B7E1F excepted), the approach seeds 009B44F0 /
// 009B4400, attackrun 009B4BD0 / 009B4D00, aim 009B5C60 / 009B5C80, release
// 009B5B00 / 009B59F0, goaway 009B5680 / 009B5760 and the moveto tick 009B6D20.
// Partial: prepare (enter 009B6600 read, tick 009B6670 unread: the host runs the
// follow tick there) and the issue branch of 007C0D90 for level bombers
// (007C0E67-007C0ECE; the paratrooper target point 006E3F90 is not modelled).

#include <cstddef>

#include "bsp/approach_target_ref.hpp"
#include "bsp/near_field_probe.hpp"

namespace bsp {

// Packet cc9_plane_level_bomb_task. Committed OFF with predictions in
// docs/SQUADRON_LAND_TASK.md 5ee. True: an aircraft whose 007EEC50 class is
// levelbomb (00E08F28) flies this task, and a level bomber's release issue
// fires every rack with the 007C0E67 delays behind the 007CC8E0 level gate.
// False: no level-bomb task (the aircraft keeps whatever it flew before).
inline constexpr bool kPlaneLevelBombTaskBound = false;

// The seven states, as task-relative offsets (approach+F0h .. +2E0h).
enum class LevelBombState : int {
    kNone = 0,
    kMoveTo = 0x4E8,     // "moveto (LevelBomb)", 009B6C10, vtable 00D201E0
    kFollow = 0x534,     // "follow (LevelBomb)", 009C2980
    kAttackRun = 0x5CC,  // "LevelBomb/attackrun", 009B49B0, vtable 00D20100
    kAim = 0x5F4,        // "LevelBomb/aim", vtable 00D2015C
    kPrepare = 0x610,    // "LevelBomb/prepare", 009C2980 + vtable 00D201A0
    kRelease = 0x6B4,    // "LevelBomb/release", vtable 00D20140
    kGoAway = 0x6D8,     // "LevelBomb/goaway", vtable 00D2011C
};

const char* level_bomb_state_name(LevelBombState s) noexcept;

namespace level_bomb {
// 009B44F0, the approach base.
inline constexpr float kFallTimeSeed = 16.0f;          // 00CE6454 -> +A4h
inline constexpr float kReleaseDelayLow = 0.3f;        // 00CE69C8, U() -> +A8h
inline constexpr float kReleaseDelayHigh = 0.65f;      // 00D07FC4
inline constexpr float kDropAltSeed = 1200.0f;         // 00CFD714 -> +B0h
inline constexpr float kAttackDistLow = 1.6f;          // 00D06BB4, x class+268h -> +B4h
inline constexpr float kAttackDistHigh = 1.8f;         // 00CF4848
inline constexpr double kInRangeScale = 1.4;           // 00D045F0 -> +B8h, +BCh
// 009B7C90, the approach update.
inline constexpr double kFallTimeBias = 0.15000000596046448;   // 00CE6618
inline constexpr double kLatchRelease = 1.100000023841858;     // 00CE3DF0
inline constexpr double kProjTimeBias = 2.0;                   // 00D7A308
inline constexpr double kProjTimeCap = 30.0;                   // 00CE7630, 00CE38C8
// 009B8D80.
inline constexpr float kSafeDistCap = 2500.0f;         // 00D20278
// 009B8B50: moveto ranges 009BDE80(+B0h - 100, +B0h, +B4h).
inline constexpr double kMoveToNearBelow = 100.0;      // 00D7A220
// 009B7A80 (prepare): +98h > -0.5.
inline constexpr float kPrepareWaitFloor = -0.5f;      // 00CE69D0
// The moveto / attackrun spacing law (009B6D20, 009B4D00).
inline constexpr float kPeerTargetRadiusSq = 90000.0f; // 00CFD404
inline constexpr double kPeerOrderBand = 15.0;         // 00CF3F20
inline constexpr double kPeerGapFloor = 10.0;          // 00CE3DC0
inline constexpr float kPeerGapFloorValue = 10.0f;     // 00CE38B8
inline constexpr float kSpeedFracPeriod = 1.0f;        // 00D7A24C (moveto +44h, +40h)
inline constexpr double kSpeedMaxMul = 1.0499999523162842;   // 00CF2510
inline constexpr float kSpeedFracTop = 1.05f;          // 00CE780C
// attackrun 009B4BD0 / 009B4D00.
inline constexpr float kRunPeriodLow = 0.4f;           // 00CE7804, stream 0
inline constexpr float kRunPeriodHigh = 0.7f;          // 00CE3E18
inline constexpr float kRunProbeX = 100.0f;            // 00CE3D08
inline constexpr float kRunProbeY = 60.0f;             // 00CEB4B0
inline constexpr float kRunProbeZ = 120.0f;            // 00D05804
inline constexpr double kRunOffsetGain = 0.5235987901687622;  // 00CEC730
inline constexpr double kRunDistCap = 2000.0;          // 00CF0DD8 / 00CFFD60
inline constexpr double kRunCeiling = 1400.0;          // 00D1F8D0
inline constexpr double kRunHeightFloor = 50.0;        // 00CE3938 / 00CEB4D4
// aim 009B5C80.
inline constexpr float kAimCloseRatio = 1.4f;          // 00D06874
inline constexpr float kAimPullOut = 1.5f;             // 00CE380C
inline constexpr float kAimNearImpact = 300.0f;        // 00CE3AE8
inline constexpr float kAimReleaseBearing = 1.483529806137085f;   // 00D20178, 85 deg
inline constexpr float kAimAbortBearing = 2.094395160675049f;     // 00D2017C, 120 deg
inline constexpr float kAimBayClose = 650.0f;          // 00D20180
inline constexpr double kAimLevelBelow = 240.0;        // 00D20190
inline constexpr double kAimDenomBias = 299.0;         // 00D20188
inline constexpr float kAimPitchLow = 200.0f;          // 00CE386C
inline constexpr double kAimProbeTrigger = 0.05000000074505806;   // 00D7A270
inline constexpr double kAimProbeVert = 3.0;           // 00D7A2B0
inline constexpr double kAimHeadingNudge = 0.2617993950843811;    // 00D19628
inline constexpr double kAimAltOffsetGain = 600.0;     // 00D20198
inline constexpr float kAimAltOffsetLo = -80.0f;       // 00D1FAEC
inline constexpr float kAimAltOffsetHi = 80.0f;        // 00CE5444
inline constexpr float kAimProbeYZ = 400.0f;           // 00CFD710
// release 009B5B00 / 009B59F0.
inline constexpr float kReleaseHold = 5.0f;            // 00CE3850
inline constexpr float kReleaseLevelAngle = 0.2617993950843811f;  // 00D05AA8, 15 deg
// goaway 009B5680 / 009B5760.
inline constexpr float kGoAwayCap = 3500.0f;           // 00D046A0 / 00D04698
inline constexpr float kGoAwayJitterLow = 1.05f;       // 00CE780C
inline constexpr float kGoAwayJitterHigh = 1.2f;       // 00CE3814
inline constexpr float kGoAwayOffsetScale = 0.6f;      // 00CE3D30
inline constexpr float kGoAwayProbeX = 100.0f;         // 00CE3D08
inline constexpr float kGoAwayProbeY = 200.0f;         // 00CE386C
inline constexpr float kGoAwayProbeZ = 120.0f;         // 00D05804
inline constexpr float kGoAwayTurnLo = -0.1745329350233078f;      // 00CECA08
inline constexpr float kGoAwayTurnHi = 0.1745329350233078f;       // 00CE3990
inline constexpr double kGoAwayNudge = 0.13962633907794952;       // 00D20138
// 007C0E67-007C0E9A, the level bomber's next-rack delay.
inline constexpr float kRackDelayLow = 0.9f;           // 00CE3860
inline constexpr float kRackDelayHigh = 1.1f;          // 00CE6448
}  // namespace level_bomb

// 009B4400, the level bomber's aim-error redraw, reads approach+14h (the
// PilotBot row): h = U(-row+ACh, row+ACh), v = U(-row+B0h, row+B0h) into
// 009FA380 (target ref +34h..+3Ch, +41h = 1), the spread row+B8h into the ref's
// +78h..+80h with +71h = 1, then approach+C4h = U(-row+B4h, row+B4h). Those row
// fields are LevelBombTargetHError, ...VError, ...CalcTargetPosError and
// ...TargetPointSelectPrec (include/bsp/robot_config.hpp _0b8.._0c4). Values:
// this installation's scripts/datatables/robots.lua (mtime 2025-06-01) PilotBot
// rows in kTorpedoAimErrorRows' order: Stun :1280, SPNormal :590, SPVeteran
// :728, MPNormal :866, MPVeteran :1004, Elite :1142. SUBSTITUTION, labelled, as
// for the torpedo and dive rows: the PilotBotConfig registry is out of reach.
inline constexpr ApproachAimErrorRow kLevelBombAimErrorRows[6] = {
    {100.0f, 200.0f, 30.0f, 0.1f},   // Stun
    {20.0f, 50.0f, 10.0f, 0.9f},     // SPNormal
    {0.0f, 0.0f, 5.0f, 0.7f},        // SPVeteran
    {15.0f, 40.0f, 5.0f, 0.4f},      // MPNormal
    {8.0f, 18.0f, 4.0f, 0.4f},       // MPVeteran
    {0.0f, 0.0f, 5.0f, 0.7f},        // Elite
};

// The approach controller's fields the rules read (task+3F8h).
struct LevelBombApproach {
    float speed_ratio_24 = 1.0f;     // +24h = task+41Ch, 009F9CE0
    int rounds_2c = 0;               // +2Ch, 009B4537 zero, -1 per release
    float projtime_74 = 0.0f;        // +74h = target ref +44h
    float fall_time_a4 = level_bomb::kFallTimeSeed;   // +A4h
    float release_delay_a8 = 0.0f;   // +A8h, U(0.3, 0.65) x 007C1FB0
    float max_speed_ac = 0.0f;       // +ACh, class+188h
    float drop_alt_b0 = level_bomb::kDropAltSeed;     // +B0h = ctl+398h each tick
    float attack_dist_b4 = 0.0f;     // +B4h
    float in_range_b8 = 0.0f;        // +B8h
    float planar_bc = 0.0f;          // +BCh
    float bearing_c0 = 0.0f;         // +C0h
    float time_error_c4 = 0.0f;      // +C4h, 009B4400's third draw
    int step_c8 = 0xFF;              // +C8h = task+4C0h
    bool close_cc = false;           // +CCh
    bool in_range_cd = false;        // +CDh = task+4C5h
    bool has_ordnance_ce = true;     // +CEh = task+4C6h
    float impact[3] = {0.0f, 0.0f, 0.0f};   // +D0h..+D8h
};

// 009B7C90 (009B7CA6-009B80F4), after its 009FADA0 head. The host refreshes the
// aim point first and passes it in.
struct LevelBombApproachInputs {
    float control_drop_alt_398 = 0.0f;   // ctl+398h
    bool release_pending_c25 = false;    // unit+C25h
    int issue_requests_c20 = 0;          // unit+C20h
    bool has_general_bomb = false;       // 007B9320(0)
    bool has_paratroopers = false;       // 007B9500(0)
    bool has_target = false;             // +48h
    float aim[3] = {0.0f, 0.0f, 0.0f};   // approach->vtable[0]
    float unit_position[3] = {0.0f, 0.0f, 0.0f};
    float unit_velocity[3] = {0.0f, 0.0f, 0.0f};   // unit->vtable[34h]
    bool reload_369_e17bf2 = false;      // ctl+369h && [00E17BF2]
    bool unit_is_leader = true;          // ctl+3D0h[0] == unit
    float leader_position[3] = {0.0f, 0.0f, 0.0f};
};
void level_bomb_approach_update_009b7c90(LevelBombApproach& a,
                                         const LevelBombApproachInputs& in) noexcept;

// 009B7B70: attackrun, aim, prepare, release or goaway.
bool level_bomb_is_attacking_009b7b70(LevelBombState s) noexcept;

struct LevelBombEntryInputs {
    bool leader = false;               // 007B8AD0
    bool release_pending_c25 = false;  // unit+C25h
    int mode_370 = 1;                  // ctl+370h
    bool has_ordnance_ce = false;
    bool in_range_cd = false;
};
// 009B8820.
LevelBombState level_bomb_entry_state_009b8820(const LevelBombEntryInputs& in) noexcept;

struct LevelBombTransitionInputs {
    LevelBombState current = LevelBombState::kNone;
    LevelBombEntryInputs entry;
    bool has_latched_target = false;   // task+440h
    bool target_disabled_5d = false;   // (task+440h)+5Dh
    bool break_off = false;            // vtable[1Ch], 009B8D80
    bool prepare_released_a0 = false;  // prepare+A0h (task+6B0h)
    bool prepare_waits = false;        // 009B7A80(prepare)
    bool aim_release_18 = false;       // aim+18h
    bool aim_abort_19 = false;         // aim+19h
    bool force_release_3ae = false;    // ctl+3AEh, SquadronSetForceRelease
    float release_timer_1c = 0.0f;     // release+1Ch (task+6D0h)
    bool goaway_ready = false;         // 009B7AB0(goaway)
};
// 009B88F0, __fastcall(task).
LevelBombState level_bomb_next_state_009b88f0(const LevelBombTransitionInputs& in) noexcept;

struct LevelBombBreakOffInputs {
    bool base_0099c230 = true;
    bool has_latched_target = false;
    bool target_disabled_5d = false;
    bool reload_369_e17bf2 = false;
    bool attacking = false;            // 009B7B70(+310h)
    bool has_ordnance_ce = false;
    float distance = 0.0f;             // |aim point - unit|, 3D
    float safe_dist_450 = 1000.0f;     // Pilot/LevelBomb/SafeDist
    float speed_ratio_41c = 1.0f;
};
// 009B8D80, vtable slot +1Ch.
bool level_bomb_should_break_off_009b8d80(const LevelBombBreakOffInputs& in) noexcept;

// 009B7AB0 on goaway: the standoff +18h is behind and the ordnance holds.
bool level_bomb_goaway_ready_009b7ab0(float standoff_18, float planar_bc,
                                      bool reload_369_e17bf2,
                                      bool has_ordnance_ce) noexcept;

// --- aim, 009B5C80 ---------------------------------------------------------
struct LevelBombAimGeometry {
    float bearing_error = 0.0f;   // wrap(+C0h - heading)
    float impact_bearing_error = 0.0f;   // wrap(bearing(impact -> aim) - heading)
    float impact_distance = 0.0f;        // planar |aim - impact|
    float ratio = 0.0f;                  // impact_distance / class+268h
    float probe_length = 0.0f;           // 00419010(0.1, 120, 0.6, 300, ratio)
};
LevelBombAimGeometry level_bomb_aim_geometry_009b5c86(float bearing_c0, float heading,
                                                      const float aim[3],
                                                      const float impact[3],
                                                      float turn_radius_268) noexcept;
struct LevelBombAimInputs {
    LevelBombAimGeometry g;
    float bearing_c0 = 0.0f;       // approach+C0h
    float heading = 0.0f;          // unit->vtable[50h]
    bool has_ordnance_ce = false;
    bool force_release_3ae = false;
    bool probe_ran = false;        // 007F0280 mode 1 at 009B5F17
    NearFieldProbeResult probe{};
    float aim_y = 0.0f;
    float drop_alt_b0 = 0.0f;
    float unit_y = 0.0f;
    float max_speed_ac = 0.0f;
    float level_flight_speed = 0.0f;   // 007C47F0
    float attack_dist_b4 = 0.0f;
};
struct LevelBombAimCommand {
    bool writes_heading = false;   // false while aim+19h holds
    float heading_2c0 = 0.0f;
    float bank_limit_2c8 = 0.0f;
    bool pitch_level = false;      // plan+2BCh = 0, +2D0h = 2
    float pitch_base = 0.0f;       // 009FBA50 arguments otherwise
    float pitch_low = 0.0f;
    float pitch_high = 0.0f;
    float pitch_scale = 0.0f;
    float speed_2b4 = 0.0f;
    bool close_cc = false;
    int step_c8 = 1;
};
// 009B5DC3-009B5E76, which runs before the probe and decides whether it runs.
void level_bomb_aim_close_009b5dc3(const LevelBombAimGeometry& g, bool has_ordnance_ce,
                                   bool force_release_3ae, bool& flag_18,
                                   bool& flag_19) noexcept;
// The tick after the probe; `flag_18` / `flag_19` are the state's +18h / +19h.
LevelBombAimCommand level_bomb_aim_tick_009b5c80(const LevelBombAimInputs& in,
                                                 bool& flag_18, bool& flag_19) noexcept;

// --- attackrun, 009B4D00 -----------------------------------------------------
struct LevelBombRunInputs {
    float bearing_c0 = 0.0f;
    float offset_20 = 0.0f;
    float speed_frac_24 = 0.0f;
    float planar_bc = 0.0f;
    float unit_y = 0.0f;
    float aim_y = 0.0f;
    float drop_alt_b0 = 0.0f;
    float attack_dist_b4 = 0.0f;
    float max_speed_188 = 0.0f;
    float level_flight_speed = 0.0f;
};
struct LevelBombRunCommand {
    float heading_2c0 = 0.0f;
    float pitch_base = 0.0f, pitch_low = 0.0f, pitch_high = 0.0f, pitch_scale = 0.0f;
    float speed_2b4 = 0.0f;
};
LevelBombRunCommand level_bomb_attackrun_009b5320(const LevelBombRunInputs& in) noexcept;

// The speed both the moveto and the attackrun tails write:
// 00419010(0, class+188h x 1.05, 1.05, 007C47F0, frac).
float level_bomb_spacing_speed(float max_speed_188, float level_flight_speed,
                               float frac) noexcept;
// The moveto's inverse of that map on the current speed (009B6D7E).
float level_bomb_speed_fraction(float max_speed_188, float level_flight_speed,
                                float speed) noexcept;

// The spacing pass both refreshes share (009B6E20-009B7400 / 009B4E00-009B530F):
// the squadrons of the same side whose target lies within 300 m of this aim
// point, split into ahead and behind of this one by their distance to it.
struct LevelBombPeer {
    float distance = 0.0f;   // planar, peer squadron -> this aim point
    unsigned id = 0;         // +174h, the tie break
};
struct LevelBombSpacing {
    float ahead_gap = -1.0f;
    float behind_gap = -1.0f;
};
LevelBombSpacing level_bomb_spacing_gaps(float own_distance, unsigned own_id,
                                         const LevelBombPeer* peers,
                                         std::size_t count) noexcept;
// The fraction both states derive (009B527B-009B5307); clamped [0, 1].
float level_bomb_spacing_fraction(const LevelBombSpacing& s) noexcept;
// The moveto's tolerance +40h (009B6FxxX): updated only when a gap exists.
float level_bomb_spacing_tolerance(const LevelBombSpacing& s, float current_40) noexcept;

// --- release, 009B5B00 / 009B59F0 --------------------------------------------
struct LevelBombReleaseState {
    bool requested_18 = false;
    bool bank_hold_19 = false;
    bool pitch_hold_1a = false;
    float timer_1c = level_bomb::kReleaseHold;
    float speed_20 = 0.0f;
};
void level_bomb_release_enter_009b5b00(LevelBombReleaseState& r, bool release_pending_c25,
                                       float current_speed, float level_flight_speed,
                                       float bank_c68, float pitch_c64) noexcept;

// --- goaway, 009B5680 / 009B5760 ---------------------------------------------
// The standoff +18h before its U(1.05, 1.2) draw and the +24h ratio.
float level_bomb_goaway_standoff_base(float safe_dist_450, bool has_extent_target,
                                      float target_extent) noexcept;
// 009B5845-009B58DD: the turn, then the heading (unit heading + turn).
float level_bomb_goaway_heading_009b5845(float heading, float fly_to_heading,
                                         float planar_bc, float probe) noexcept;

// --- the release issue, 007C0D90 for a level bomber ---------------------------
// 007CC8E0's level arm after the bay test: |bank| and |pitch| <= tuning+550h.
bool level_bomb_attitude_gate_007cc8e0(float bank_c68, float pitch_c64,
                                       float angle_max_550) noexcept;

}  // namespace bsp

#endif  // BSP_PLANE_LEVEL_BOMB_TASK_HPP
