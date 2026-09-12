#pragma once

#include <array>
#include <cstdint>

#include "bsp/ship_ai_approach_update.hpp"  // ShipAiCircleTangentCircle
#include "bsp/ship_ai_goal_vector.hpp"      // ShipAiPathPointRecord
#include "bsp/ship_ai_path_planner.hpp"     // ShipAiPathNode, ShipAiPathPlanBlock

namespace bsp {
// How a ship walks the plan the search built: 009E3C00 and its two helpers.
//
// Packet cc_ai_path_follower, worker agent/cc-ai-path-follower. Project
// C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra was
// READ-ONLY for this packet. Every descriptive name here is a hypothesis, not
// a recovered symbol. docs/SHIP_AI_PATH_FOLLOWER.md carries the evidence
// address by address and the Coverage table saying which bodies are partial.
//
// This header builds on bsp/ship_ai_path_planner.hpp (ShipAiPathNode,
// ShipAiPathPlanBlock and the shared direction rule), on
// bsp/ship_ai_goal_vector.hpp (ShipAiPathPointRecord, the 22h-byte record the
// caller hands in) and on bsp/ship_ai_approach_update.hpp
// (ShipAiCircleTangentCircle, the three-float circle 009D6550 takes as `this`).
// It redefines none of their types.
//
// What the routine is. 009E3C00 answers one question per navigation tick:
// given a finished plan and the ship's pose, which point should the ship steer
// at. It is not a search. It walks `plan+34h` links from the head, takes the
// node after that as the target, offsets the target sideways if the target
// carries a lateral record, and publishes a point at a computed distance along
// a steering direction. Where the path turns a corner it does not aim at the
// corner: it builds a circle of look-ahead radius centred one look-ahead clear
// of the corner along the corner's bisector and aims at one of the two tangent
// points from the ship to that circle, the node's signed byte +14h choosing
// which.
//
//   009E3C6C  the cursor walk, plan+34h steps from plan+20h
//   009E3D13  009D5930 on the node the cursor landed on
//   009E3D3C  009D5930 on the target, to find the node after it
//   009E3DCD  00811D80 the lateral offset for the target's anchor point
//   009E3EAE  0082E850 the ship class turn radius; x1.5 is the look-ahead
//   009E3EC0  owner+9C8h, the unit radius; x0.75 is the floor on the step
//   009E3FFC  00414C60 the length of the corner bisector
//   009E4162  009D6550 the two tangent points from the pose to the circle
//   009E4200  004218E0 and 009E4216 00417EF0, the shortcut clearance test
//   009E421F  plan+34h += 1, the ONLY advance of the cursor in the image
//
// The cursor. `ShipAiPathPlanBlock::node_count` is plan+34h. Every writer in
// the image is listed in docs/SHIP_AI_PATH_FOLLOWER.md; eight of the nine store
// zero and the ninth is 009E421F here, so the field is a step cursor, not a
// node count. The name in bsp/ship_ai_path_planner.hpp predates that evidence
// and is left alone by this packet; the Corrections section of the doc records
// it and the follow-up packet renames it.

// ---------------------------------------------------------------------------
// Constants, each from the instruction that loads it
// ---------------------------------------------------------------------------

// 00D7A24C, 1.0f. Stored into plan+4Ch by the prologue at 009E3C23 and used at
// 009E3F71 as the shortest following leg that still counts as a corner.
inline constexpr float kShipAiPathFollowerMinLegLength = 1.0f;
// 00CE3D78, a double, 009E3EB3: the class turn radius times this is the
// look-ahead radius, which is both the corner circle's radius and the distance
// under which the follower tries to skip the target node.
inline constexpr double kShipAiPathFollowerLookAheadScale = 1.5;
// 00CEC9D8, a double, 009E3EC6: the unit radius times this is the distance the
// published point sits ahead of the ship when the target is further than that.
inline constexpr double kShipAiPathFollowerRadiusScale = 0.75;
// 00D7A288, 009E4009: a corner bisector shorter than this is treated as
// degenerate and replaced by a perpendicular of the following leg.
inline constexpr float kShipAiPathFollowerBisectorEpsilon = 1.0e-6f;
// 00D7A250, a double, 009E40E1: the ship is called past the corner when the
// product of its lateral offset and the bisector's lateral component is below
// this. The bound is on a product of two unnormalised terms, so it is a length
// threshold in the following leg's units, not a sign test.
inline constexpr double kShipAiPathFollowerOvershootLimit = -1.0;
// 00D7A3A0, a double, 009E428F: below this the steering vector's length is not
// used as a divisor.
inline constexpr double kShipAiPathFollowerMinSteerLength = 0.1;
// 00D7A2F0, 009E429B: the divisor substituted when the steering vector is
// shorter than kShipAiPathFollowerMinSteerLength.
inline constexpr float kShipAiPathFollowerSteerLengthFloor = 0.1f;

// ---------------------------------------------------------------------------
// The lateral record at node+10h
// ---------------------------------------------------------------------------
//
// 009E3D8C dereferences ShipAiPathNode::field_10 and 009E3DD2 reads it again.
// Only these four floats are read by the follower. The producer is 009D5920
// out of 00417610 (docs/SHIP_AI_PATH_SEARCH.md, the +10h row); this packet did
// not read it, so the layout below claims only the fields 009E3C00 touches.
// The consumer 009EE63A reads the same record's +20h as a width.
struct ShipAiPathLateralAnchor {
    float x{0.0f};      // +00h, 009E3D96, the anchor point the target is offset from
    float z{0.0f};      // +04h, 009E3DA0
    float dir_x{0.0f};  // +10h, 009E3DE2, the unit the offset is taken along
    float dir_z{0.0f};  // +14h, 009E3DE8
};

// ---------------------------------------------------------------------------
// 009D5930, the out-of-line copy of the shared direction rule
// ---------------------------------------------------------------------------

// 009D5930, `void __thiscall(node)`, RET 0, body 009D5930-009D5987, complete.
// Instruction for instruction the same rule bsp/ship_ai_path_planner.hpp
// projects as ship_ai_path_node_direction_009d9e6c from 009D9E6C, which both
// walks inline (009E3C80-009E3CE1 and 009D9E80-009D9ED6). This routine is the
// call-site form: it stores the decision into node+4h and returns nothing.
// Reuses the planner's rule rather than restating it.
std::int32_t ship_ai_path_node_decide_direction_009d5930(ShipAiPathNode& node) noexcept;

// ---------------------------------------------------------------------------
// 004F3970 and 009D6550, the tangent pair
// ---------------------------------------------------------------------------

// What 004F3970 writes into its two out pointers. `valid` false means neither
// was written: the caller's buffers keep whatever they held.
struct ShipAiPathTangentChord {
    bool valid{false};
    // 004F3AE4/004F3AEA (external) or 004F3B54/004F3B62 (interior). For a point
    // outside the circle this is the foot of the tangent chord on the line from
    // the centre to the point; for a point inside it is the point itself.
    std::array<float, 2> base{{0.0f, 0.0f}};
    // 004F3B0D/004F3B13 and 004F3B84/004F3B8A. Perpendicular to the centre-to-
    // point direction, half the chord long.
    std::array<float, 2> half_chord{{0.0f, 0.0f}};
};

// 004F3970, `bool __thiscall(circle)(const float2* point, float2* base,
// float2* half_chord)`, RET 0Ch, body 004F3970-004F3B98, complete. False and
// no writes when the point is at the centre (004F39F4) or within 1e-6 of the
// circle itself (004F3A3B). Name proposed, not applied: 004F3970 belongs to the
// open `ship_ai_nav_circle_tangent` packet.
ShipAiPathTangentChord ship_ai_path_tangent_chord_004f3970(
    const ShipAiCircleTangentCircle& circle,
    const std::array<float, 2>& point) noexcept;

// What 009D6550 writes. `valid` false means neither point was written.
struct ShipAiPathTangentPair {
    bool valid{false};
    std::array<float, 2> first{{0.0f, 0.0f}};   // base + half_chord, 009D6599/009D659F
    std::array<float, 2> second{{0.0f, 0.0f}};  // base - half_chord, 009D65B8/009D65BE
};

// 009D6550, `bool __thiscall(circle)(const float2* point, float2* first,
// float2* second)`, RET 0Ch, body 009D6550-009D65D0, complete. It is 004F3970
// plus the two sums. For a point outside the circle the pair is the two tangent
// points; for a point inside it is the two ends of the chord through the point
// perpendicular to the centre direction. 009D68B0
// BSP_ShipAi_CircleTangentOrOffsetPoint is the other caller and selects between
// the two with its `side` argument (docs/SHIP_AI_APPROACH_UPDATE.md).
ShipAiPathTangentPair ship_ai_path_tangent_points_009d6550(
    const ShipAiCircleTangentCircle& circle,
    const std::array<float, 2>& point) noexcept;

// ---------------------------------------------------------------------------
// 009E3C00
// ---------------------------------------------------------------------------

// 009E3C6C-009E3D0A. Walks plan+34h links from the head, deciding each
// undecided node's direction on the way. Deviation from the image: 009E3C80
// re-enters the loop body without testing the pointer, so a cursor longer than
// the path dereferences null there; this stops and returns null instead.
ShipAiPathNode* ship_ai_path_cursor_node_009e3c6c(ShipAiPathPlanBlock& plan) noexcept;

// One method per native call site 009E3C00 makes, in the order a corner tick
// runs them. The avoid-zone objects stay opaque: this packet does not own them.
struct ShipAiPathFollowerHost {
    virtual ~ShipAiPathFollowerHost() = default;

    // 009E3D8C and 009E3DD2, a dereference of ShipAiPathNode::field_10, not a
    // call. Null when the node carries no lateral record.
    virtual const ShipAiPathLateralAnchor* lateral_anchor_node_10(std::uint32_t handle) = 0;

    // 009E3DCD, 00811D80(record)(&xz) where the record is
    // `owner + 0A98h + 54h * [owner+0B40h]`, the unit's own PUBLISHED order
    // slot (009E3DB5-009E3DC1; unit_ai_order_published_offset in
    // bsp/ship_ai_navigation.hpp). Contract from
    // bsp/ship_ai_throttle_ring.hpp: 30.0f unless a live sub-record sits within
    // 20 units of the query position.
    virtual float order_turn_limit_at_00811d80(const std::array<float, 2>& xz) = 0;

    // 009E3EAE, 0082E850 BSP_ShipClass_GetTurnRadius with
    // ECX = [[plan+3Ch]+538h], the owner's ship class (009E3EA5).
    virtual float owner_class_turn_radius_0082e850() = 0;

    // 009E3EC0, the float at owner+9C8h through plan+3Ch, a field read and not
    // a call. The same field 009E3ADB reads in 009E3780.
    virtual float owner_radius_09c8() = 0;

    // 009E4200, 004218E0 BSP_AvoidZoneManager_GetSingleton.
    virtual std::uint32_t avoid_zone_manager_004218e0() = 0;

    // 009E4216, 00417EF0(manager)(layer, from, to, &hit), RET 10h. The body
    // (00417EF0-00417F59) asks 004120D0 for the layer's zone group and forwards
    // to 004179D0; true means the segment meets a zone and `hit` receives the
    // meeting point, false leaves `hit` alone. It is the float2-out sibling of
    // 00417E90 `segment_blocked_00417e90` (docs/SHIP_AI_PATH_SEARCH.md), which
    // has the same body shape and the same polarity. 004179D0 itself was not
    // read: the polarity is from the two siblings and their call sites, not
    // from the deciding body.
    virtual bool segment_hits_zone_00417ef0(std::uint32_t manager,
                                            std::uint32_t zone_layer,
                                            const std::array<float, 2>& from,
                                            const std::array<float, 2>& to,
                                            std::array<float, 2>& hit) = 0;
};

// Which exit 009E3C00 took. The image returns nothing; the caller reads the
// record.
enum class ShipAiPathFollowerExit {
    EmptyPlan,        // 009E3C3F, plan+20h null: the point is the pose itself
    NoTargetNode,     // 009E3D6B, a head but no node past the cursor
    StraightAtPoint,  // 009E4197, no corner: steer at the target point
    CornerTangent,    // 009E4185, steer at a tangent of the corner circle
};

struct ShipAiPathFollowerResult {
    ShipAiPathFollowerExit exit{ShipAiPathFollowerExit::EmptyPlan};
    bool advanced_cursor{false};   // 009E421F ran
    bool tested_shortcut{false};   // the 009E41F7 / 009E41FE gate opened
    bool overshot{false};          // 009E40ED set the byte
    bool tangent_found{false};     // 009D6550 returned true
    bool degenerate_bisector{false};  // the 009E4011 arm ran
    float look_ahead{0.0f};        // what landed in plan+4Ch
    float distance_to_point{0.0f}; // the pose-to-target distance at 009E3E5F
    float step_length{0.0f};       // the max at 009E42AB, how far ahead the point sits
};

// 009E3C00, `void __thiscall(plan)(float* record)`, RET 4 at 009E3C65,
// 009E3D81 and 009E4328, body 009E3C00-009E432A, complete.
//
// Writes the record's +08h/+0Ch point, +10h/+14h next point, +18h anchor
// handle, +1Ch side code and the two bytes, and the plan's +4Ch, +50h/+54h and
// +60h/+64h. plan+34h is the only other plan field it writes, and only through
// the increment at 009E421F.
//
// Deviations from the image, both in degenerate input:
//   - the cursor walk stops at a null link instead of faulting (see above);
//   - when 009D6550 reports no tangent, the image leaves its two buffers
//     holding the circle centre and the offset VECTOR, and a negative side byte
//     then steers at that vector as if it were a point. This reproduces it, so
//     a caller that sees `tangent_found` false in `CornerTangent` is looking at
//     the image's own behaviour, not at a repair.
ShipAiPathFollowerResult ship_ai_path_follower_point_009e3c00(
    ShipAiPathPlanBlock& plan,
    ShipAiPathPointRecord& record,
    ShipAiPathFollowerHost& host);

}  // namespace bsp
