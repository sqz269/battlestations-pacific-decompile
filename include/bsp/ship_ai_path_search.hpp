#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_path_planner.hpp"

namespace bsp {
// The search that expands the two seed nodes into a path: 009EC680 and the five
// routines it drives. Semantic interfaces, not native object layouts. Names are
// hypotheses, not recovered symbols. Addresses, evidence, original ABI and
// uncertainty: docs/SHIP_AI_PATH_SEARCH.md.
//
// 009E3780 (include/bsp/ship_ai_path_planner.hpp) leaves the block holding two
// nodes, the ship and the goal, linked by one segment, in state Seeded.
// 009EC680 is called once per navigation tick from 009ED3E0
// BSP_ShipAi_RefreshPathPlan (009ED59B on the plan being computed, 009ED614 on
// the plan in use) and performs exactly one state transition.
//
// The search is not A*, and it is not a visibility graph built up front. It is
// a lazy binary expansion over the node graph, the classic "split the blocked
// segment around the obstacle, twice, and keep the cheaper half":
//
//   Seeded / ExtendSearch  009EC6D8  009EC280 costs the whole graph, 009E3330
//                                    splits the first unresolved edge on the
//                                    cheapest route, 009D5A20 asks whether a
//                                    fully resolved route now exists. On yes
//                                    the state becomes Searched, on no the
//                                    same case runs again next tick.
//   Searched               009EC712  009D9550 prunes every fork down to the
//                                    cheaper branch and splices out the hops
//                                    under one unit, then Extracting with the
//                                    plan's node cursor cleared.
//   Extracting             009EC72C  009D96A0 removes one node whose corner no
//                                    longer bends the path outward. It returns
//                                    true while it removed one, which sends the
//                                    state back to ExtendSearch so the new
//                                    shortcut edge is re-tested; false gives
//                                    Extracted.
//   Extracted              009EC742  Ready.
//
// 009ED5A6 swaps the plan into use as soon as the state passes Searched, so the
// ship starts following at Extracting and the smoothing runs on the live plan.
//
// On an open sea with no avoid zone between the two seeds the whole sequence is
// four navigation ticks: Seeded on the tick 009E3780 seeds (009ED528 sets the
// computing flag and 009ED59B runs the first pass on the same tick), Searched
// on tick 2 with the swap, Extracted on tick 3, Ready on tick 4. The plan is
// usable from tick 2.

// ---------------------------------------------------------------------------
// Constants, each from the instruction that loads it
// ---------------------------------------------------------------------------

// 00E0E304, loaded at 009EC6BB into the argument slot 009EC6E9 hands 009E3330:
// the clearance the detour corners keep from the zone polygon.
inline constexpr float kShipAiPathSearchMargin = 30.0f;
// 009E33A3 / 009E35AB CMP EDI,0x96 with JGE: the expansion stops once it has
// descended 150 nodes, and marks the edge it stopped on resolved.
inline constexpr std::int32_t kShipAiPathSearchDepthLimit = 0x96;
// 00CE3880, a double compared at 009E33FE, 009E3460, 009E3606 and 009E3668: a
// candidate corner within five units of the node that produced it is dropped.
inline constexpr double kShipAiPathSearchDuplicateRadiusSq = 25.0;
// 00CE3914, a float compared at 009D9710 and 009D9730: in the smoothing pass a
// leg under three units forces the middle node out whatever the turn says.
inline constexpr float kShipAiPathSearchSmoothMinLegSq = 9.0f;
// 00CE38B8, compared at 009EC2DC and 009EC2F7: the start node's heading seed
// propagates across a link only while the link is shorter than this.
inline constexpr float kShipAiPathSearchSeedHopLimit = 10.0f;
// 00D7A248, stored at 009EC2B6: a node whose edge had no usable detour costs
// FLT_MAX, which keeps every route through it out of the comparison.
inline constexpr float kShipAiPathSearchDeadEndCost = 3.402823466e+38f;
// 00CF87D0, stored into +38h at 009D9627 once the fork is collapsed: the
// missing second link's length.
inline constexpr float kShipAiPathSearchNoLinkLength = 1.0e6f;
// 00CE3D10, a double multiplied in at 009EC497, 009EC52F, 009EC5B6 and
// 009EC5D1: a link's turn penalty is capped at a fifth of the route behind it.
inline constexpr double kShipAiPathSearchTurnCostFraction = 0.2;
// 00CE3DC0, the dividend at 009D9818, and 00D7A270 / 00CE7638, the limit and
// the clamp at 009D982C and 009D9832: the smoothing tolerance is
// min(10 / max(leg, 1), 0.05) radians.
inline constexpr double kShipAiPathSearchSmoothToleranceScale = 10.0;
inline constexpr double kShipAiPathSearchSmoothToleranceLimit = 0.05;
inline constexpr float kShipAiPathSearchSmoothToleranceClamp = 0.05f;
// 00CE4C04, stored at 009E3082 into both candidate points before 00422500 runs.
inline constexpr float kShipAiPathSearchCandidateSentinel = 9999.0f;
// The side code 009E3283 and 009E32D7 write into a fresh corner node's +14h.
inline constexpr std::int8_t kShipAiPathSearchSideLeft = -1;   // 009E329D, byte 0FFh
inline constexpr std::int8_t kShipAiPathSearchSideRight = 1;   // 009E32F1

// ---------------------------------------------------------------------------
// Node fields this packet establishes
// ---------------------------------------------------------------------------
//
// The planner header names the 54h-byte node; four of its unnamed fields and
// two of its named ones belong to the search. Accessors rather than new fields,
// because the layout is owned by include/bsp/ship_ai_path_planner.hpp.
//
//   +08h field_08   the avoid zone this node's corner belongs to, 009E328D
//   +0Ch field_0c   the corner index inside that zone, a signed short, 009E3297
//   +10h field_10   the zone's corner record, 009D5920
//   +14h field_14   the side code, a signed byte, 009E329D / 009E32F1
//   +2Ch field_2c   the back pointer of link_plus, 009D92F8
//   +30h field_30   set when the edge was blocked and no detour existed, 009E330C
//   +31h short_link "the link_minus edge is resolved": 009D92BB seeds it from
//                   the length, 009E3573 sets it when the edge proved clear
//   +32h field_32   the same for link_plus, 009D936B and 009E3764

// 009D92F8 MOV dword ptr [EAX + 0x2c],ESI, the mirror of 009D9248's +28h store.
// The field is typed std::uint32_t by the planner header; on the Win32 target a
// node pointer is exactly that wide. See the Corrections table in the doc.
inline ShipAiPathNode* ship_ai_path_node_back_plus(const ShipAiPathNode& node) noexcept {
    return reinterpret_cast<ShipAiPathNode*>(static_cast<std::uintptr_t>(node.field_2c));
}
inline void ship_ai_path_node_set_back_plus(ShipAiPathNode& node, ShipAiPathNode* back) noexcept {
    node.field_2c = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(back));
}

// 009E329D and 009E32F1 write a byte the readers sign-extend (009D590B SETL on
// a signed compare, 009D985B / 009D986D JG and JL, 009EC4B1 MOVSX).
inline std::int8_t ship_ai_path_node_side(const ShipAiPathNode& node) noexcept {
    return static_cast<std::int8_t>(node.field_14);
}
inline void ship_ai_path_node_set_side(ShipAiPathNode& node, std::int8_t side) noexcept {
    node.field_14 = static_cast<std::uint8_t>(side);
}

// 009E3297 MOV word ptr [EAX + 0xc],DX over the 0FFFFh the constructor wrote;
// 009D5904 TEST AX,AX with JL and 009E33C6 CMP CX,word both read it signed.
inline std::int16_t ship_ai_path_node_corner_index(const ShipAiPathNode& node) noexcept {
    return static_cast<std::int16_t>(node.field_0c);
}

// ---------------------------------------------------------------------------
// The host the executable must implement
// ---------------------------------------------------------------------------
//
// One pure-virtual method per native call site, in the order a Seeded tick runs
// them. reports/ship_ai_path_search.json carries the same list with
// `address` / `native` rows. The avoid-zone objects stay opaque handles: this
// packet does not own them, and 004179D0, 00422500's tangent walk and
// 00423190's body are not read here.
struct ShipAiPathSearchTurnRamp {
    float knee_x;   // settings+6F0h, 009EC310, the interpolation's x0
    float limit_x;  // settings+6F4h, 009EC323, x1
    float limit_y;  // settings+6F8h, 009EC336, y1. y0 is FLDZ at 009EC3AA.
};

struct ShipAiPathSearchHost {
    virtual ~ShipAiPathSearchHost() = default;

    // 009EC30B, 009EC31E and 009EC331: 00424C40 BSP_GameSettings_GetSingleton,
    // called once per field. The three floats feed one 00419010 ramp, so this
    // method delivers them together.
    virtual ShipAiPathSearchTurnRamp game_settings_turn_ramp_00424c40() = 0;

    // 009E3058: 004218E0 BSP_AvoidZoneManager_GetSingleton, once per edge test.
    virtual std::uint32_t avoid_zone_manager_004218e0() = 0;

    // 009E3075: 00417E90(manager)(layer, &from, &to, &out_zone, &out_edge),
    // RET 14h, body 00417E90-00417EED. The body resolves the layer's group with
    // 004120D0 and hands the segment to 004179D0; on a hit it copies the zone
    // that routine reported into *out_zone. Returns AL. 004179D0's body is not
    // read here, so which zone wins when the segment crosses several is open.
    virtual bool segment_blocked_00417e90(std::uint32_t manager,
                                          std::uint32_t zone_layer,
                                          const std::array<float, 2>& from,
                                          const std::array<float, 2>& to,
                                          std::uint32_t& out_zone,
                                          std::int32_t& out_edge_index) = 0;

    // 009E3105: 00422500(zone)(far_point, edge_index, near_corner_hint,
    // far_side_hint, margin, &left, &right, &left_index, &right_index),
    // RET 28h. Contract partial: the head 00422500-00422580 was read and shows
    // zone+0h as the corner-record array and zone+4h as its count, the hit edge
    // indexing that array, the wrap to the next corner, and the margin being
    // floored at one unit. The tangent walk 00422580-0042318F is unread, so the
    // rule that picks the two corners is not established here.
    //
    // The caller reads the return as a side preference: negative means only the
    // left corner is offered, positive only the right, zero both
    // (009E3256 TEST/JG, 009E32AA TEST/JL).
    virtual std::int32_t zone_detour_corners_00422500(std::uint32_t zone,
                                                      const std::array<float, 2>& far_point,
                                                      std::int32_t edge_index,
                                                      std::int32_t near_corner_hint,
                                                      std::int32_t far_side_hint,
                                                      float margin,
                                                      std::array<float, 2>& out_left,
                                                      std::array<float, 2>& out_right,
                                                      std::int32_t& out_left_index,
                                                      std::int32_t& out_right_index) = 0;

    // 009E3189 and 009E3207: 0071C4F0(world)(&point), RET 4, body
    // 0071C4F0-0071C547, complete. ECX is the global at 00E188A8; the body
    // compares point.x against world+711Ch and world+7128h and point.z against
    // world+7124h and world+7130h and returns 1 when the point is outside that
    // box. The caller builds the float3 as {x, 0, z}.
    virtual bool point_outside_world_bounds_0071c4f0(const std::array<float, 3>& point) = 0;

    // 009E3263, 009E32B7, 009E34CC, 009E3533, 009E36D4 and 009E3725:
    // operator new(54h). Zero on failure.
    virtual ShipAiPathNode* allocate_path_node_00bf681b(std::size_t size) = 0;

    // 009D5917: 00417610(zone)(index), RET 4, body 009D5917's callee
    // 00417610-00417623, complete: returns zone_records[index % zone_count],
    // signed IDIV at 00417619: quotient truncates toward zero; the remainder
    // keeps the index's sign and can address before the array. No fixup/guard.
    virtual std::uint32_t zone_corner_record_00417610(std::uint32_t zone,
                                                      std::int32_t index) = 0;

    // 009D5923: 00423190(zone)(record), body 00423190-004234F8. Contract
    // bounded by selected geometry: positive record+20h skips computation;
    // otherwise caches an outward clearance scale, initially 800, refined
    // against a temporary segment list (004234BC store). The full outer body
    // is now read; list membership/lifetime remain dependencies. See
    // docs/SHIP_AI_LATERAL_RECORD.md for its producer and exact refinement.
    virtual void ensure_zone_corner_metric_00423190(std::uint32_t zone,
                                                    std::uint32_t record) = 0;

    // CALL EDX on node->vtable[0] with 1: the scalar deleting destructor.
    // 009E340D, 009E346E, 009E3615 and 009E3676 discard a rejected candidate;
    // 009D95A9, 009D95C9 and 009D9680 drop a losing branch; 009D98A9 drops a
    // smoothed-away node. Callee body unread: contract unread beyond "the node
    // is gone".
    virtual void release_path_node_vtable0(ShipAiPathNode* node) = 0;
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 009D92E0, __thiscall(node)(next), RET 4, body 009D92E0-009D937E,
// complete. Instruction for instruction 009D9230 with +24h
// for +20h, +2Ch for +28h, +38h for +34h and +32h for +31h: a null argument is
// a no-op, otherwise the link is written with the back pointer on the far node,
// the segment length measured through the same x87 kernel, and the resolved
// flag set when that length is below one unit.
void ship_ai_path_node_link_alt_009d92e0(ShipAiPathNode& node, ShipAiPathNode* next) noexcept;

// 009D5930, __thiscall(node), RET 0, body 009D5930-009D5987, complete. The
// direction rule 009D9E6C and 009E3C56 inline, run for its side effect on +4h
// before the expansion reads it. A decided node keeps its choice. The image's
// JC at 009D9974 also takes the plus branch when the two costs are unordered,
// which is the one place it differs from
// ship_ai_path_node_direction_009d9e6c. The JC is at 009D5974.
std::int32_t ship_ai_path_node_choose_direction_009d5930(const ShipAiPathNode& node) noexcept;

// 009D5A20, __thiscall(node) -> bool in AL, RET 0, body 009D5A20-009D5A7A,
// complete. "Is there a route from this node to a leaf across resolved edges
// only." A node with neither link is a leaf and answers yes; otherwise each
// link is followed only when its resolved flag is set, and either side
// answering yes is enough. 009EC703 reads it as "the search is done".
bool ship_ai_path_search_route_resolved_009d5a20(const ShipAiPathNode& node) noexcept;

// 009EC280, __thiscall(node)(float side_switch_penalty) -> float in ST0,
// RET 4, body 009EC280-009EC67E, complete. A post-order pass over the whole
// graph that writes, on every node it reaches: +4h the cheaper direction, +4Ch
// and +50h the two link turn costs, +3Ch the route length from that node on.
//
// The turn cost exists only on the node the ship stands on (+45h). Its heading
// seed +48h, which 009E3980 took from the owner, is compared against the
// bearing to each child through 00414EB0 and 00438B10, run through the
// 00419010 ramp, and capped at a fifth of the route behind it by 00415510. The
// seed propagates to a child across a link shorter than ten units, so a first
// hop that short moves the penalty one node along.
//
// side_switch_penalty is plan+2Ch, which 009EC6D8 loads as a float. It is added
// to a route whose next corner sits on the opposite side of its zone from this
// one (009EC4CB IMUL of the two signed +14h bytes). 009D9CE1 writes the field
// zero and no other writer was found, so the penalty is inert in the image.
float ship_ai_path_search_cost_009ec280(ShipAiPathNode& node,
                                        float side_switch_penalty,
                                        ShipAiPathSearchHost& host);

// What 009E3040 reported about one edge.
struct ShipAiPathEdgeProbe {
    bool blocked{false};       // 009E307C JZ: false when 00417E90 found no zone
    ShipAiPathNode* left{nullptr};   // 009E328B, side -1, the +20h candidate slot
    ShipAiPathNode* right{nullptr};  // 009E32DF, side +1
    bool dead_end{false};      // 009E330C, the edge is blocked and neither corner survived
    bool allocation_failed{false};   // deviation, see below
};

// 009E3040, __thiscall(node)(int side, node** out_left, node** out_right,
// float margin) -> bool in AL, RET 10h, body 009E3040-009E3325, complete.
//
// side selects the edge: -1 the link_minus edge, +1 the link_plus edge. The
// segment from this node to the far node goes to 00417E90; a clear segment
// returns false at once and nothing else runs. On a hit the zone and the index
// of the edge that was hit go to 00422500 together with the far endpoint, the
// margin, and two hints: the near node's own corner index when it sits on this
// same zone and -1 when it does not, and the far node's side byte when it does
// and 0 when it does not. 00422500 answers with up to two corner points offset
// by the margin, their indices, and the side preference described on the host
// method.
//
// Each candidate then has to survive two tests: 0071C4F0 must not put it
// outside the world box, and its bearing from this node, compared against the
// bearing of the straight segment through 00438B10, must have the sign of its
// side (009E31CA JBE keeps a non-negative delta for the left candidate,
// 009E324A JBE a non-positive one for the right). A surviving candidate becomes
// a fresh node carrying the zone, the corner index and the side code, and
// 009D58F0 attaches the zone's corner record to it.
//
// Deviation from the image: when operator new returns null, 009E328D writes
// through the null pointer and the image faults. This reconstruction reports
// allocation_failed and leaves both slots as they stand.
ShipAiPathEdgeProbe ship_ai_path_probe_edge_009e3040(ShipAiPathNode& node,
                                                     std::int32_t side,
                                                     float margin,
                                                     ShipAiPathSearchHost& host);

// 009D58F0, __thiscall(node), RET 0, body 009D58F0-009D5929, complete. Gives a
// fresh corner node its zone corner record, once: a node that already has one,
// or has no zone, or whose corner index is negative, is left alone. The record
// index is the corner index, plus one when the side code is negative, so the
// two sides of a corner take adjacent indexed records. 00417610 applies signed
// remainder indexing; 009D5920 stores the record handle before 00423190
// refreshes its cached scale at 009D5923. Producer: SHIP_AI_LATERAL_RECORD.md.
void ship_ai_path_node_attach_corner_record_009d58f0(ShipAiPathNode& node,
                                                     ShipAiPathSearchHost& host);

// How one 009E3330 pass ended.
enum class ShipAiPathExpandOutcome {
    LeafReached,        // 009E334A, the descent ran into a node with no links
    EdgeCleared,        // 009E3573 / 009E3764, the probe found the edge clear
    BudgetExhausted,    // the same two stores after 009E33A9 / 009E35B1
    DeadEnd,            // the probe was blocked but neither corner survived
    Inserted,           // one corner was inserted into the chain
    Forked,             // both corners were inserted as a fork
    AllocationFailed,   // deviation, see 009E3040
};

struct ShipAiPathExpandResult {
    ShipAiPathExpandOutcome outcome{ShipAiPathExpandOutcome::LeafReached};
    ShipAiPathNode* at{nullptr};   // the node whose edge the pass worked on
    std::int32_t depth{0};         // the counter as the pass left it
};

// 009E3330, __thiscall(node)(float margin, int depth), RET 8, body
// 009E3330-009E376F, complete.
//
// One pass. It walks down the cheapest route, 009D5930 deciding each node's
// direction as it goes, and stops at the first edge whose resolved flag is
// clear. That edge goes to 009E3040. A clear edge, or a depth past 150, sets
// the flag and ends the pass. Otherwise the surviving corners are spliced in:
// one corner becomes a single detour node between the two ends, two corners
// become a diamond, the left corner reached through link_minus and the right
// through link_plus, both rejoining the far node. When the far node already
// carries a back pointer on the slot the splice needs, or when this node has no
// free slot, a positional copy is allocated so the existing structure is not
// rewired (009E34C2, 009E3531, 009E36CA, 009E3723).
ShipAiPathExpandResult ship_ai_path_expand_009e3330(ShipAiPathNode& node,
                                                    float margin,
                                                    std::int32_t depth,
                                                    ShipAiPathSearchHost& host);

// 009D9550, __thiscall(node), RET 0, body 009D9550-009D9690, complete. The
// prune. At every fork it keeps one branch and destroys the other: an
// unresolved edge loses, a branch 009D5A20 rejects loses, and otherwise the
// cheaper of the two route totals wins, ties going to link_plus. The survivor
// is moved into the link_minus slot so what is left is a chain, the costs are
// zeroed, link_plus_length takes the no-link sentinel, and the pass recurses
// into the next node. It then splices out every following node whose incoming
// link is under one unit.
void ship_ai_path_prune_009d9550(ShipAiPathNode& node, ShipAiPathSearchHost& host);

// 009D96A0, __thiscall(node) -> bool in AL, RET 0, body 009D96A0-009D98BB,
// complete. The smoothing pass, one removal per call, tail-recursive along the
// chain. For each triple node -> a -> c it compares the bearing to c with the
// bearing to a: a is removed when the path does not bend past a tolerance
// toward the side a's corner sits on, or when either leg is shorter than three
// units. The tolerance is min(10 / max(node's leg, 1), 0.05) radians. On a
// removal from the short-leg rule the new link is marked resolved so the next
// expansion does not re-test it; on a removal from the turn rule it is not, so
// the state machine sends the shortcut back through 009E3330. Returns false
// when the chain has fewer than three nodes left, which is what ends the loop.
bool ship_ai_path_smooth_009d96a0(ShipAiPathNode& node, ShipAiPathSearchHost& host);

// ---------------------------------------------------------------------------
// 009EC680 itself
// ---------------------------------------------------------------------------

// Which case of 009EC680's switch a tick took.
enum class ShipAiPathSearchAction {
    Idle,              // 009EC692 / 009EC69B / 009EC6A4: Empty, Failed or Ready
    FailedNoOwner,     // 009EC6B0, plan+3Ch is null
    Expanded,          // 009EC6D8, the Seeded and ExtendSearch case
    Pruned,            // 009EC712, the Searched case
    Smoothed,          // 009EC72C, the Extracting case
    Finished,          // 009EC742, the Extracted case
};

struct ShipAiPathSearchTickResult {
    ShipAiPathSearchState state_before{ShipAiPathSearchState::Empty};
    ShipAiPathSearchState state_after{ShipAiPathSearchState::Empty};
    ShipAiPathSearchAction action{ShipAiPathSearchAction::Idle};
    float revalidate_delay{0.0f};   // plan+14h after 009EC68F
    bool route_resolved{false};     // 009D5A20's answer on an Expanded tick
    bool node_removed{false};       // 009D96A0's answer on a Smoothed tick
    // Deviation, not an image exit: every working case hands plan+20h to a
    // __thiscall and the image faults when the head is null.
    bool head_missing{false};
    ShipAiPathExpandResult expand{};// the pass 009E3330 made on an Expanded tick
};

// 009EC680, __thiscall(plan)(float frame_delta), RET 4, body 009EC680-009EC74C,
// complete. The frame delta comes off plan+14h before the state is read, so the
// revalidation delay runs down on every tick including the ones that do
// nothing. The jump table at 009EC750 covers states 1 to 6; state 2 has an
// entry but the equality test at 009EC69B has already returned.
ShipAiPathSearchTickResult ship_ai_path_search_tick_009ec680(ShipAiPathPlanBlock& plan,
                                                             float frame_delta,
                                                             ShipAiPathSearchHost& host);

}  // namespace bsp
