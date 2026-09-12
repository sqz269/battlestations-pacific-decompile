// 009E3C00: the walk that turns a finished plan into the point the navigation
// arm steers at.
//
// Addresses: 009E3C00 (body 009E3C00-009E432B), 009D5930, 009D9E6C's rule,
// 00811D80, 0082E850, 009D6550, 00417EF0, 004218E0.
//
// Packet cc_exe_2q. Every descriptive name here is a hypothesis, not a
// recovered symbol. docs/SHIP_AI_PATH_PLANNER.md established the walk and the
// node layout and read this routine as far as 009E3D81;
// docs/SHIP_AI_NAVIGATION_ARM.md established what the caller does with the
// record. Neither is redefined here.
//
// Shape of the routine, `__thiscall(plan)(PathPointRecord* record)`, RET 4:
//
//   009E3C11..009E3C37  five plan fields and two record fields are seeded from
//                       the query point the caller wrote at +00h/+04h.
//   009E3C3A            no head: the point is the query point, and the record
//                       says so with +20h and +21h clear.
//   009E3C6C..009E3D0A  the walk, at most plan->node_count steps from the head,
//                       one step short of 009D9E50's.
//   009E3D10..009E3D5D  three nodes: the one the walk stopped on, its successor
//                       and the one after that.
//   009E3D8C..009E3E01  the successor's own point, or its lateral corner record
//                       offset by 00811D80's answer when it carries one.
//   009E3E07..009E3F14  the three leg lengths, the class turn radius and the
//                       owner radius.
//   009E3F1A..009E4222  THE CORNER ARM, NOT PROJECTED HERE. It runs when the
//                       successor still has a link and the outgoing leg is at
//                       least one unit long, and it rounds the corner through
//                       009D6550 and the avoid-zone probe 00417EF0.
//   009E4223..009E432B  the record's direction code and lateral record, then
//                       the point: the query point advanced along the chosen
//                       direction by max(owner_radius * 0.75, |successor -
//                       query|).
//
// Coverage: partial. The walk, the three-node pick, the successor point and the
// final store are projected; 009E3F1A..009E4222 is a host record with its
// address. On an open sea the plan is a two-node ship-to-goal chain whose goal
// node carries neither link, so 009E3F14's test takes the projected arm and the
// point handed back is the goal node's own position.
#pragma once

#include <array>
#include <cstdint>

#include "bsp/ship_ai_goal_vector.hpp"
#include "bsp/ship_ai_path_planner.hpp"
#include "bsp/ship_ai_path_search.hpp"

namespace bsp {

// 00CEC9D8, the double the owner radius at [plan+3Ch]+9C8h is scaled by
// (009E3EC6).
inline constexpr double kShipAiPathPointOwnerRadiusScale = 0.75;
// 00D7A2F0, the length the final normalise falls back to when the chosen
// direction is shorter than the 0.1 at 00D7A3A0 (009E4295, 009E429B).
inline constexpr float kShipAiPathPointMinDirectionLength = 0.1f;
// 00D7A24C, the one-unit floor the outgoing leg has to clear before the corner
// arm runs (009E3F14), and the 1.0f 009E3C23 seeds plan+4Ch with.
inline constexpr float kShipAiPathPointCornerLegFloor = 1.0f;
// 00CE3D78, the multiplier on 0082E850's turn radius (009E3EB3).
inline constexpr double kShipAiPathPointTurnRadiusScale = 1.5;

struct ShipAiPathPointHost {
    virtual ~ShipAiPathPointHost() = default;

    // 009E3DCD..009E3E01: 00811D80 BSP_UnitAiOrder_TurnLimitAt(&point) on the
    // successor's lateral corner record, times that record's +10h / +14h pair.
    // The corner record belongs to the avoid-zone packet, so the host returns
    // the product rather than the two halves. Reached only when the successor
    // carries such a record, which needs an avoid zone; no node of an unzoned
    // plan does.
    virtual std::array<float, 2> lateral_point_offset_00811d80(
        std::uint32_t corner_record, const std::array<float, 2>& point) = 0;

    // 009E3EAE: 0082E850 BSP_ShipClass_GetTurnRadius on [[plan+3Ch]+538h]. The
    // call is unconditional in the image, so the host is asked whether or not
    // the answer is used.
    virtual float class_turn_radius_0082e850() = 0;

    // 009E3EC0: [plan+3Ch]+9C8h, the owner's radius. The same field
    // 009E3ADB reads in the planner and 009F4174 reads in the drive.
    virtual float owner_radius_09c8() = 0;

    // 009E3F1A..009E4222, the corner arm. Not projected: the host is told the
    // routine needed it and answers nothing.
    virtual void corner_arm_009e3f1a() = 0;
};

struct ShipAiPathPointResult {
    bool has_head{false};             // 009E3C3A, plan+20h
    int walk_steps{0};                // how many links the walk followed
    bool walk_ran_out{false};         // the walk hit a node with no matching link
    ShipAiPathNode* stopped{nullptr}; // ESI after the walk
    ShipAiPathNode* successor{nullptr};  // EDI
    ShipAiPathNode* after{nullptr};      // EBX at 009E3D5D
    bool no_successor{false};         // the 009E3D6B exit: the point is the query
    bool corner_arm{false};           // the unprojected 009E3F1A arm was required
    float direction_length{0.0f};     // the length the final normalise used
    float advance{0.0f};              // local_40 after 009E42BD
};

// 009E3C00 over the record the caller seeded with the query point. Deviation,
// not an image behaviour: the image's walk keeps stepping after the link test
// returns null and faults on the next node; this stops instead, the same
// deviation docs/SHIP_AI_PATH_SEARCH.md records for the search.
ShipAiPathPointResult ship_ai_path_point_009e3c00(ShipAiPathPlanBlock& plan,
                                                  ShipAiPathPointRecord& record,
                                                  ShipAiPathPointHost& host);

}  // namespace bsp
