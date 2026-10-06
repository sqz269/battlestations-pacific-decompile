#pragma once
// The plane `retreat` bot task (kind 9), packet cc9_plane_retreat_task,
// docs/SQUADRON_LAND_TASK.md section 5ea. Every descriptive name here is a
// hypothesis, not a recovered symbol; the state names are the strings the
// approach registers ("moveto (retreat)", "enterzone (retreat)", "leave
// (retreat)", "follow (retreat)", 009C9C6B..009C9CA5).
//
// The image, read whole except where marked:
//  * factory 009CA2B0, constructor 009C9D00 __thiscall(task, bot), size 564h:
//    the base kind 9, then the approach 009C9BB0 at task+3F8h; +55Ch = 1.0
//    (the arm period, 00D7A24C), +560h = -U(0, 1) (stream 1, 009C9D7E). A
//    flight leader (007B8AD0) starts in moveto +478h, a wing member in follow
//    +4C4h, then that state's enter, vtable[4].
//  * approach 009C9BB0 -> 009C91B0: +2Ch = U(-50, 100) (00CECA0C, 00CE3D08) +
//    tuning+53Ch Pilot/General/CruisingAlt, the flown altitude; +30h =
//    U(500, 800) (00CE397C, 00CE3950), the edge margin; +34h = 1.0, +38h =
//    -U(0, 1), the refresh period and phase; +60h = +61h = 0; then 009C8D40
//    and 009C8F40. Then the follow state 009C2980 (its own draw).
//  * the per-tick slot +64h 009CA130: the approach countdown (refresh through
//    009C8D40 then 009C8F40 when it runs out), the arm 009C9FB0, then the
//    current state's vtable[0Ch](dt).
//  * states: moveto 00D20EA8 (enter 009C92F0, tick 009C9310), enterzone
//    00D20EC4 (tick 009C98C0), leave 00D20EE0 (tick 009C9990), follow 00D20AB8
//    (tick 009C1FD0, the generic follow). Enter/exit of the last three are the
//    empty 007B3DB0/007B3DC0. Transitions go through 009C9F70 (old exit, then
//    new enter).
//  * still-valid, slot +40h 009C9E30: the squadron's current command (squadron
//    vtable[114h] -> 0071BE40) is `retreat` 00E08F90; 2 without a squadron.
//  * the plane leaves the map in 007C6C30 (the out-of-action countdown, plane
//    vtable[1D8h]): outside the map with a squadron whose 007EE2E0 says it is
//    leaving on purpose (its command is `retreat`), unit+6F0h counts down from
//    tuning+34Ch Retreat/ExitTime; 0059C9B0 (more than tuning+348h
//    Retreat/ExitDist outside) or the countdown expiring sends 007EE410's
//    message 50h, whose receiver 007CA790 sets Lua ExitZoneParty and runs
//    007F31A0, which ends with BSP_MissionEntity_Kill(4).
#include <array>
#include <cstdint>

#include "bsp/world_map_bounds.hpp"

namespace bsp {

// Packet cc9_plane_retreat_task. True: `retreat` (00E08F90) delivered to a
// squadron's planes installs this task (0099A170's arm at 0099A45B -> 009CA2B0)
// and the planes fly it to the map edge and are removed there (007C6C30 ->
// 007F31A0 -> Kill(4)). False: the command is placed and the bot intake drops
// it, as before. Committed OFF with the predictions (5ea), flipped ON after
// the JM05l / USN02 pairs (5ea.1).
inline constexpr bool kPlaneRetreatTaskBound = true;   // ON: SQUADRON_LAND_TASK 5ea.1

inline constexpr std::uint32_t kRetreatCommandClass = 0x00E08F90u;   // 009C9E20
inline constexpr float kRetreatAltitudeDrawLow = -50.0f;     // 00CECA0C, 009C91D6
inline constexpr float kRetreatAltitudeDrawHigh = 100.0f;    // 00CE3D08
inline constexpr float kRetreatMarginDrawLow = 500.0f;       // 00CE397C, 009C91FB
inline constexpr float kRetreatMarginDrawHigh = 800.0f;      // 00CE3950
inline constexpr float kRetreatRefreshPeriod = 1.0f;         // 00D7A24C, approach+34h
inline constexpr float kRetreatArmPeriod = 1.0f;             // 00D7A24C, task+55Ch
inline constexpr double kRetreatEnterPointScale = 3.0;       // 00D7A2B0 (double)
inline constexpr double kRetreatHalf = 0.5;                  // 00D7A280 (double)
inline constexpr double kRetreatQuarter = 0.25;              // 00D7A348 (double)
inline constexpr double kRetreatTinySquare = 1.0e-10;        // 00CE3820 (double)

enum class RetreatState : std::uint8_t {
    kNone,
    kMoveTo,     // +478h
    kEnterZone,  // +494h
    kLeave,      // +4ACh
    kFollow,     // +4C4h
};

// The approach's own fields (task+3F8h + offset). The zone is the record
// 004C7730 returned (approach+3Ch); the host's records live in the Lua
// host's BorderZoneSet for the whole mission, so a pointer is stable.
struct RetreatApproach {
    float altitude_2c{0.0f};
    float margin_30{0.0f};
    float period_34{kRetreatRefreshPeriod};
    float countdown_38{0.0f};
    const BorderZoneRecord* zone_3c{nullptr};
    float edge_40[4]{0.0f, 0.0f, 0.0f, 0.0f};   // B.x, B.z, C.x, C.z
    float enter_point_50[2]{0.0f, 0.0f};
    float direction_58[2]{0.0f, 0.0f};
    bool leave_now_60{false};
    bool lined_up_61{false};
    float target_64[2]{0.0f, 0.0f};
};

// 009C8D40 __fastcall(approach), complete. 004C7730(unit+FCh, side unit+54h,
// 0, 0) picks the zone; +40h..+4Ch its B..C edge; the direction is the unit
// vector from the edge's midpoint to the mean of the four corners, and the
// enter point lies 3.0 x TurnCircleRadius (class+268h) along it. The image
// dereferences a null zone; the host keeps the previous fields and answers
// false (labelled).
bool retreat_refresh_zone_009c8d40(RetreatApproach& approach, const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side, float turn_circle_radius);

// 00489C40 -> 004C74C0, complete for a point outside the map (its only use
// here): the edge whose outside distance is strictly the largest, then the
// first record along that edge whose running length passes the point (or the
// last one). Inside the map the image indexes with a stale value; null here.
const BorderZoneRecord* border_zone_at_point_00489c40(const WorldMapBounds& bounds,
    const BorderZoneSet& zones, const std::array<float, 3>& point);

// 009C8F40 __fastcall(approach), complete: the moveto target +64h/+68h on the
// zone's edge nearest the unit (an x-constant edge clamps z to the edge; a
// z-constant edge clamps x to the edge inset by 2 x margin), the lined-up byte
// +61h (within the clamp range widened by the margin), and +60h, set when the
// unit is already outside the map in this very zone.
void retreat_refresh_target_009c8f40(RetreatApproach& approach, const WorldMapBounds& bounds,
    const BorderZoneSet& zones, float x, float z);

// The countdown shape 009CA130 (approach+38h/+34h) and 009C9FB0
// (task+560h/+55Ch) share: dt < c -> c - dt; else c = (period - dt) + c and
// the caller acts. x87 order kept.
bool retreat_countdown_step(float& countdown, float period, float dt) noexcept;

// 009C92F0, the moveto state's enter: state+18h = class+268h / class+18Ch.
float retreat_moveto_timer_009c92f0(float turn_circle_radius, float travel_speed) noexcept;

// 009C9FB0's transition rule (after its countdown fired), complete.
struct RetreatArmInputs {
    RetreatState state{RetreatState::kNone};
    bool flight_leader{false};       // 007B8AD0
    bool leave_now_60{false};
    bool lined_up_61{false};
    float moveto_timer{0.0f};        // moveto state+18h (task+490h)
    bool inside_enter_zone{false};   // 009C9EA0
};
RetreatState retreat_arm_009c9fb0(const RetreatArmInputs& in) noexcept;

// 009C9EA0 __fastcall(task), complete: planar distance to the enter point
// (0 when its square is at most 1e-10) below TurnCircleRadius.
bool retreat_inside_enter_zone_009c9ea0(const RetreatApproach& approach, float x, float z,
    float turn_circle_radius) noexcept;

// 009C9310, the moveto tick, complete: the four edge weights
// 00419010(edge, 1, edge -+ margin, 0, coordinate), their sum and the push
// vector, the steer heading pi/2 - atan2 of the blend of the unit vector to
// the target and the push, the commanded speed
// 00419010(0, squadron+3A0h, 1, class+188h MaxSpd, w), and whether the state
// timer runs (planar distance below TurnCircleRadius).
struct RetreatMoveToInputs {
    WorldMapBounds bounds{};
    float margin_30{0.0f};
    float x{0.0f};
    float z{0.0f};
    float target_x{0.0f};
    float target_z{0.0f};
    float turn_circle_radius{0.0f};  // class+268h
    float squadron_speed_3a0{0.0f};  // squadron+3A0h, class+190h
    float max_spd_188{0.0f};         // class+188h
};
struct RetreatMoveToSteer {
    float weight{0.0f};          // local_78
    float heading_2c0{0.0f};
    float desired_speed_2b4{0.0f};
    float distance{0.0f};
    bool timer_runs{false};
};
RetreatMoveToSteer retreat_moveto_steer_009c9310(const RetreatMoveToInputs& in) noexcept;

// 009F9D90, the enterzone tick's steer (009C990A), complete: pi/2 -
// atan2(point.z - z, point.x - x), wrapped into [0, 2 pi).
float retreat_heading_to_point_009f9d90(float x, float z, const float point[2]) noexcept;

// 009C9990, the leave tick's heading, complete: pi/2 - atan2(dir.z, dir.x),
// wrapped.
float retreat_leave_heading_009c9990(const RetreatApproach& approach) noexcept;

// 0059C9B0 __thiscall(world)(point, margin), complete: true when the point is
// more than `margin` outside the map box on x or z.
bool point_beyond_map_margin_0059c9b0(const WorldMapBounds& bounds, float x, float z,
    float margin) noexcept;

}  // namespace bsp
