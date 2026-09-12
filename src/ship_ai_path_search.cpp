// The search that expands the two seed nodes into a path: 009EC680 and the five
// routines it drives. Evidence, original ABI and uncertainty:
// docs/SHIP_AI_PATH_SEARCH.md. Names are hypotheses, not recovered symbols.

#include "bsp/ship_ai_path_search.hpp"

#include <cmath>
#include <cstring>

#include "bsp/geometry_helpers.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// 00415510 BSP_Math_MinFloatByRef, __fastcall(ECX = &a, EDX = &b) -> ST0.
// 00415525 FCOMIP compares b with a and 00415529 JBE returns b, so an unordered
// pair also returns b.
float min_float_by_ref_00415510(float a, float b) noexcept {
    return (b > a) ? a : b;
}

// The squared planar distance the search builds and stores to a float before
// every comparison (009E33F0, 009E3452, 009D9702, 009D9726).
float planar_distance_sq(float ax, float az, float bx, float bz) noexcept {
    const float dx = ax - bx;
    const float dz = az - bz;
    return dx * dx + dz * dz;
}

// The far end of the edge `side` names: -1 the link_minus edge (009E3050), +1
// the link_plus edge (009E3055).
ShipAiPathNode* edge_far(const ShipAiPathNode& node, std::int32_t side) noexcept {
    return side < 0 ? node.link_minus : node.link_plus;
}

// 009E3573 / 009E3764: "this edge is resolved", the flag 009D5A20 walks across
// and 009E3330 skips over.
void mark_edge_resolved(ShipAiPathNode& node, std::int32_t side) noexcept {
    if (side < 0) {
        node.short_link = 1;
    } else {
        node.field_32 = 1;
    }
}

// 009E33B7 / 009E35BF: a candidate that is the far node's own corner, or that
// sits within five units of the node that produced it, is thrown away.
bool candidate_is_redundant(const ShipAiPathNode& node,
                            const ShipAiPathNode& far,
                            const ShipAiPathNode& candidate) noexcept {
    if (candidate.field_08 == far.field_08 &&
        ship_ai_path_node_corner_index(candidate) == ship_ai_path_node_corner_index(far)) {
        return true;  // 009E33CA / 009E342C JZ straight to the destructor
    }
    const float sq = planar_distance_sq(node.x, node.z, candidate.x, candidate.z);
    // 009E33FE FCOMIP of the double 25.0 against the sum, 009E3402 JBE keeps the
    // candidate, so an unordered pair is kept.
    return kShipAiPathSearchDuplicateRadiusSq > static_cast<double>(sq);
}

// 009E34CA / 009E3531 / 009E36D2 / 009E3723: a positional copy of an existing
// node, so a splice does not have to rewire what already points at it.
ShipAiPathNode* clone_node(const ShipAiPathNode& source, ShipAiPathSearchHost& host) {
    ShipAiPathNode* const copy = host.allocate_path_node_00bf681b(kShipAiPathNodeSize);
    if (copy == nullptr) {
        return nullptr;  // the image calls 009D9230 with a null this and faults
    }
    *copy = ship_ai_path_node_init_009d9150({source.x, source.z}, source.zone_layer);
    return copy;
}

// The two link writers, selected by the side of the edge being spliced.
void link_side(ShipAiPathNode& node, ShipAiPathNode* next, std::int32_t side) noexcept {
    if (side < 0) {
        ship_ai_path_node_link_009d9230(node, next);
    } else {
        ship_ai_path_node_link_alt_009d92e0(node, next);
    }
}

}  // namespace

// ---------------------------------------------------------------------------

void ship_ai_path_node_link_alt_009d92e0(ShipAiPathNode& node, ShipAiPathNode* next) noexcept {
    if (next == nullptr) {  // 009D92E7 TEST EAX,EAX / 009D92EC JZ
        return;
    }
    node.link_plus = next;                              // 009D92F2
    ship_ai_path_node_set_back_plus(*next, &node);      // 009D92F8
    // 009D92F5..009D9322 is the same 00414C60 kernel 009D9230 uses.
    const float length = length_2d_00414c60({node.x - next->x, node.z - next->z});
    node.link_plus_length = length;                     // 009D935F
    // 009D935C COMISS 1.0, length / 009D9364 JBE: unordered also clears it.
    node.field_32 = (kShipAiPathShortLinkLimit > length) ? std::uint8_t{1} : std::uint8_t{0};
}

std::int32_t ship_ai_path_node_choose_direction_009d5930(const ShipAiPathNode& node) noexcept {
    if (node.direction != 0) {  // 009D5931, a decided node keeps its choice
        return node.direction;
    }
    if (node.link_minus == nullptr) {  // 009D593A
        return node.link_plus != nullptr ? 1 : 0;  // 009D5940 SETNZ
    }
    if (node.link_plus == nullptr) {  // 009D594E
        return -1;                    // 009D5976
    }
    // 009D5952..009D596D: each sum is stored to a float and reloaded before the
    // comparison. 009D5974 JC takes the plus branch when the plus total is
    // strictly cheaper and also when the pair is unordered.
    const float minus = node.cost_minus + node.link_minus_length + node.link_minus->remaining_length;
    const float plus = node.cost_plus + node.link_plus_length + node.link_plus->remaining_length;
    return (plus >= minus) ? -1 : 1;
}

bool ship_ai_path_search_route_resolved_009d5a20(const ShipAiPathNode& node) noexcept {
    if (node.link_minus == nullptr && node.link_plus == nullptr) {
        return true;  // 009D5A27 / 009D5A2B, a leaf is the end of a route
    }
    bool minus_reaches = false;
    if (node.short_link != 0 && node.link_minus != nullptr) {  // 009D5A38 / 009D5A41
        minus_reaches = ship_ai_path_search_route_resolved_009d5a20(*node.link_minus);
    }
    bool plus_reaches = false;
    if (node.field_32 != 0 && node.link_plus != nullptr) {  // 009D5A4C / 009D5A55
        plus_reaches = ship_ai_path_search_route_resolved_009d5a20(*node.link_plus);
    }
    return minus_reaches || plus_reaches;  // 009D5A62 / 009D5A67
}

// ---------------------------------------------------------------------------

namespace {

// 009EC351..009EC3BC and 009EC3D1..009EC43B, the same five instructions twice:
// the bearing from the start node to the child, measured against the heading
// seed and run through the 00419010 ramp.
float turn_cost(const ShipAiPathNode& node,
                const ShipAiPathNode& child,
                const ShipAiPathSearchTurnRamp& ramp) {
    const float bearing = heading_angle_00414eb0({child.x - node.x, child.z - node.z});
    const float delta = wrapped_angle_subtract_00438b10(bearing, node.seed);
    const float magnitude = std::fabs(delta);  // 009EC389 AND EAX,0x7fffffff
    return clamped_interpolate_00419010(ramp.knee_x, 0.0f, ramp.limit_x, ramp.limit_y, magnitude);
}

// 009EC4B1 / 009EC4C7 / 009EC5EB / 009EC5F2: both side codes sign-extended and
// multiplied; a negative product means the two corners are on opposite sides.
bool side_changes(const ShipAiPathNode& node, const ShipAiPathNode& child) noexcept {
    const std::int32_t product = static_cast<std::int32_t>(ship_ai_path_node_side(node)) *
                                 static_cast<std::int32_t>(ship_ai_path_node_side(child));
    return product < 0;
}

// 009EC497 / 009EC52F / 009EC5B6 / 009EC5D1 then 00415510: a link's turn cost
// never exceeds a fifth of the route behind it.
float capped_turn_cost(float cost, float route) noexcept {
    const float cap = static_cast<float>(static_cast<double>(route) *
                                         kShipAiPathSearchTurnCostFraction);
    return min_float_by_ref_00415510(cost, cap);
}

}  // namespace

float ship_ai_path_search_cost_009ec280(ShipAiPathNode& node,
                                        float side_switch_penalty,
                                        ShipAiPathSearchHost& host) {
    node.direction = 0;       // 009EC295
    node.cost_plus = 0.0f;    // 009EC29C
    node.cost_minus = 0.0f;   // 009EC2A1
    if (node.field_30 != 0) {  // 009EC28A, the edge had no usable detour
        node.remaining_length = kShipAiPathSearchDeadEndCost;  // 009EC2B6
        return node.remaining_length;
    }
    if (node.is_start != 0) {  // 009EC2C3
        // 009EC2DC / 009EC2F7 COMISS 10.0 against the link length with JBE: the
        // seed moves on only across a link shorter than ten units.
        if (node.link_minus != nullptr &&
            kShipAiPathSearchSeedHopLimit > node.link_minus_length) {
            node.link_minus->is_start = 1;    // 009EC2E7
            node.link_minus->seed = node.seed;  // 009EC2EB
        }
        if (node.link_plus != nullptr &&
            kShipAiPathSearchSeedHopLimit > node.link_plus_length) {
            node.link_plus->is_start = 1;     // 009EC302
            node.link_plus->seed = node.seed;   // 009EC306
        }
        const ShipAiPathSearchTurnRamp ramp = host.game_settings_turn_ramp_00424c40();
        if (node.link_minus != nullptr && node.link_minus->is_start == 0) {  // 009EC33E / 009EC34B
            node.cost_minus = turn_cost(node, *node.link_minus, ramp);       // 009EC3BC
        }
        if (node.link_plus != nullptr && node.link_plus->is_start == 0) {    // 009EC3BE / 009EC3C5
            node.cost_plus = turn_cost(node, *node.link_plus, ramp);         // 009EC43B
        }
    }

    if (node.link_minus == nullptr && node.link_plus == nullptr) {
        // 009EC455..009EC46B, a leaf: no route left and nothing to choose.
        node.remaining_length = 0.0f;
        node.link_plus_length = 0.0f;
        node.link_minus_length = 0.0f;
        node.is_start = 0;
        node.direction = 0;
        return node.remaining_length;
    }

    if (node.link_minus == nullptr) {
        // 009EC476..009EC4FE, the link_plus-only branch.
        float route = ship_ai_path_search_cost_009ec280(*node.link_plus, side_switch_penalty, host) +
                      node.link_plus_length;
        node.remaining_length = route;                         // 009EC494
        node.cost_plus = capped_turn_cost(node.cost_plus, route);  // 009EC4B5
        route = node.cost_plus + route;                        // 009EC4B8
        node.remaining_length = route;                         // 009EC4C4
        node.direction = 1;                                    // 009EC4D7 / 009EC4F2
        if (side_changes(node, *node.link_plus)) {
            node.remaining_length = route + side_switch_penalty;  // 009EC4D2
        }
        return node.remaining_length;
    }

    if (node.link_plus == nullptr) {
        // 009EC501..009EC586, the link_minus-only branch.
        node.direction = -1;                                   // 009EC50F
        float route = ship_ai_path_search_cost_009ec280(*node.link_minus, side_switch_penalty, host) +
                      node.link_minus_length;
        node.remaining_length = route;                         // 009EC52C
        node.cost_minus = capped_turn_cost(node.cost_minus, route);  // 009EC54D
        route = node.cost_minus + route;                       // 009EC54F
        node.remaining_length = route;                         // 009EC55A
        if (side_changes(node, *node.link_minus)) {
            node.remaining_length = route + side_switch_penalty;  // 009EC568
        }
        return node.remaining_length;
    }

    // 009EC589..009EC67C, both links: cost each route and keep the cheaper.
    const float minus_route =
        ship_ai_path_search_cost_009ec280(*node.link_minus, side_switch_penalty, host) +
        node.link_minus_length;                                // 009EC58E
    const float plus_route =
        ship_ai_path_search_cost_009ec280(*node.link_plus, side_switch_penalty, host) +
        node.link_plus_length;                                 // 009EC5A5
    node.cost_minus = capped_turn_cost(node.cost_minus, minus_route);  // 009EC5C5
    node.cost_plus = capped_turn_cost(node.cost_plus, plus_route);     // 009EC5EF
    float minus_total = minus_route + node.cost_minus;         // 009EC5FA
    float plus_total = node.cost_plus + plus_route;            // 009EC603
    if (side_changes(node, *node.link_minus)) {
        minus_total = minus_total + side_switch_penalty;       // 009EC613
    }
    if (side_changes(node, *node.link_plus)) {
        plus_total = plus_total + side_switch_penalty;         // 009EC62D
    }
    // 009EC63D FCOMIP of the plus total against the minus total, 009EC641 JBE:
    // a tie, and an unordered pair, go to link_plus.
    if (plus_total > minus_total) {
        node.remaining_length = minus_total;  // 009EC649
        node.direction = -1;                  // 009EC652
    } else {
        node.remaining_length = plus_total;   // 009EC667
        node.direction = 1;                   // 009EC670
    }
    return node.remaining_length;
}

// ---------------------------------------------------------------------------

void ship_ai_path_node_attach_corner_record_009d58f0(ShipAiPathNode& node,
                                                     ShipAiPathSearchHost& host) {
    if (node.field_10 != 0) {  // 009D58F3, done once
        return;
    }
    if (node.field_08 == 0) {  // 009D58FC, no zone to ask
        return;
    }
    const std::int16_t corner = ship_ai_path_node_corner_index(node);
    if (corner < 0) {  // 009D5904 TEST AX,AX / 009D5907 JL
        return;
    }
    // 009D590B SETL on the signed side byte, 009D5914 ADD: the left side takes
    // the edge after the corner, the right side the edge at it.
    const std::int32_t index =
        static_cast<std::int32_t>(corner) + (ship_ai_path_node_side(node) < 0 ? 1 : 0);
    const std::uint32_t record = host.zone_corner_record_00417610(node.field_08, index);  // 009D5917
    node.field_10 = record;                                                              // 009D5920
    host.ensure_zone_corner_metric_00423190(node.field_08, record);                       // 009D5923
}

ShipAiPathEdgeProbe ship_ai_path_probe_edge_009e3040(ShipAiPathNode& node,
                                                     std::int32_t side,
                                                     float margin,
                                                     ShipAiPathSearchHost& host) {
    ShipAiPathEdgeProbe probe{};
    ShipAiPathNode* const far = edge_far(node, side);  // 009E3043..009E3055
    const std::uint32_t manager = host.avoid_zone_manager_004218e0();  // 009E3058
    std::uint32_t zone = 0;
    std::int32_t edge_index = 0;
    if (!host.segment_blocked_00417e90(manager, node.zone_layer, {node.x, node.z},
                                       {far->x, far->z}, zone, edge_index)) {
        return probe;  // 009E307C JZ 009E331C, AL zero: the edge is clear
    }
    probe.blocked = true;

    std::array<float, 2> left_point{kShipAiPathSearchCandidateSentinel,
                                    kShipAiPathSearchCandidateSentinel};  // 009E30A2 / 009E30A8
    std::array<float, 2> right_point{kShipAiPathSearchCandidateSentinel,
                                     kShipAiPathSearchCandidateSentinel};
    std::int32_t left_index = -1;   // 009E3098
    std::int32_t right_index = -1;  // 009E3094
    // 009E3091 / 009E30BE: the hints are only handed over when the node in
    // question already sits on this same zone.
    const std::int32_t far_side_hint =
        (far->field_08 == zone) ? static_cast<std::int32_t>(ship_ai_path_node_side(*far)) : 0;
    const std::int32_t near_corner_hint =
        (node.field_08 == zone) ? static_cast<std::int32_t>(ship_ai_path_node_corner_index(node)) : -1;
    const std::int32_t preference = host.zone_detour_corners_00422500(
        zone, {far->x, far->z}, edge_index, near_corner_hint, far_side_hint, margin,
        left_point, right_point, left_index, right_index);  // 009E3105

    // 009E310A..009E3156: the bearing of the straight segment this edge is.
    const float segment_bearing =
        heading_angle_00414eb0({far->x - node.x, far->z - node.z});

    if (left_index >= 0) {  // 009E3156
        const std::array<float, 3> probe_point{left_point[0], 0.0f, left_point[1]};
        bool keep = false;
        if (!host.point_outside_world_bounds_0071c4f0(probe_point)) {  // 009E3189
            const float bearing =
                heading_angle_00414eb0({left_point[0] - node.x, left_point[1] - node.z});
            const float delta = wrapped_angle_subtract_00438b10(segment_bearing, bearing);
            keep = !(0.0f > delta);  // 009E31C6 FCOMIP zero / 009E31CA JBE
        }
        if (!keep) {
            left_index = -1;  // 009E31CC
        }
    }
    if (right_index >= 0) {  // 009E31D4
        const std::array<float, 3> probe_point{right_point[0], 0.0f, right_point[1]};
        bool keep = false;
        if (!host.point_outside_world_bounds_0071c4f0(probe_point)) {  // 009E3207
            const float bearing =
                heading_angle_00414eb0({right_point[0] - node.x, right_point[1] - node.z});
            const float delta = wrapped_angle_subtract_00438b10(segment_bearing, bearing);
            keep = !(delta > 0.0f);  // 009E3246 FCOMIP / 009E324A JBE
        }
        if (!keep) {
            right_index = -1;  // 009E324C
        }
    }

    bool built_any = false;
    // 009E3256 TEST EDI / JG: a positive preference skips the left candidate.
    if (preference <= 0 && left_index >= 0) {  // 009E325A
        ShipAiPathNode* const candidate = host.allocate_path_node_00bf681b(kShipAiPathNodeSize);
        if (candidate == nullptr) {
            // 009E328D writes through the null pointer and the image faults.
            probe.allocation_failed = true;
            return probe;
        }
        *candidate = ship_ai_path_node_init_009d9150(left_point, node.zone_layer);  // 009E327A
        candidate->field_08 = zone;                                                 // 009E328D
        candidate->field_0c = static_cast<std::uint16_t>(left_index);               // 009E3297
        ship_ai_path_node_set_side(*candidate, kShipAiPathSearchSideLeft);           // 009E329D
        ship_ai_path_node_attach_corner_record_009d58f0(*candidate, host);           // 009E32A3
        probe.left = candidate;
        built_any = true;
    }
    // 009E32AA TEST EDI / JL: a negative preference skips the right candidate.
    if (preference >= 0 && right_index >= 0) {  // 009E32AE
        ShipAiPathNode* const candidate = host.allocate_path_node_00bf681b(kShipAiPathNodeSize);
        if (candidate == nullptr) {
            probe.allocation_failed = true;
            return probe;
        }
        *candidate = ship_ai_path_node_init_009d9150(right_point, node.zone_layer);  // 009E32CE
        candidate->field_08 = zone;                                                  // 009E32E1
        candidate->field_0c = static_cast<std::uint16_t>(right_index);               // 009E32EB
        ship_ai_path_node_set_side(*candidate, kShipAiPathSearchSideRight);           // 009E32F1
        ship_ai_path_node_attach_corner_record_009d58f0(*candidate, host);            // 009E32F7
        probe.right = candidate;
        return probe;  // 009E32FC returns without touching the dead-end flag
    }
    if (!built_any) {
        node.field_30 = 1;     // 009E330C
        probe.dead_end = true;
    }
    return probe;
}

// ---------------------------------------------------------------------------

namespace {

// 009E347A..009E356F and 009E3682..009E3761, one splice written once. `side`
// names the edge being replaced; `left` goes on link_minus and `right` on
// link_plus, which is how the two side codes stay consistent.
ShipAiPathExpandOutcome splice_detour(ShipAiPathNode& node,
                                      std::int32_t side,
                                      ShipAiPathNode* left,
                                      ShipAiPathNode* right,
                                      ShipAiPathSearchHost& host) {
    ShipAiPathNode* far = edge_far(node, side);
    if (side < 0) {
        far->back = nullptr;  // 009E347F
    } else {
        ship_ai_path_node_set_back_plus(*far, nullptr);  // 009E3687
    }

    if (right == nullptr) {  // 009E3482 / 009E368A
        link_side(*left, far, side);   // 009E348A / 009E3692
        link_side(node, left, side);   // 009E3492 / 009E369A
        return ShipAiPathExpandOutcome::Inserted;
    }
    if (left == nullptr) {  // 009E34A1 / 009E36A9
        link_side(*right, far, side);  // 009E34AB / 009E36B3
        link_side(node, right, side);  // 009E34B3 / 009E36BB
        return ShipAiPathExpandOutcome::Inserted;
    }

    // 009E34C5 reads the far node's link_plus back pointer, 009E36CD its
    // link_minus one: the slot the fork is about to claim.
    const bool far_already_joined =
        (side < 0) ? (ship_ai_path_node_back_plus(*far) != nullptr) : (far->back != nullptr);
    if (far_already_joined) {
        ShipAiPathNode* const copy = clone_node(*far, host);
        if (copy == nullptr) {
            return ShipAiPathExpandOutcome::AllocationFailed;
        }
        link_side(*copy, far, side);  // 009E34F2 / 009E36FA
        // 009E34F7 / 009E36FF is a raw pointer store: the length and the
        // resolved flag on this node keep the values the old link left.
        if (side < 0) {
            node.link_minus = copy;
        } else {
            node.link_plus = copy;
        }
        far = copy;
    }

    ShipAiPathNode* const join = edge_far(node, side);
    ship_ai_path_node_link_alt_009d92e0(*left, join);   // 009E3502 / 009E370A
    ship_ai_path_node_link_009d9230(*right, join);      // 009E350D / 009E3715

    const bool free_slot =
        (side < 0) ? (node.link_plus == nullptr) : (node.link_minus == nullptr);
    if (free_slot) {  // 009E3512 / 009E371A, both land on 009E3517
        ship_ai_path_node_link_009d9230(node, left);
        ship_ai_path_node_link_alt_009d92e0(node, right);
        return ShipAiPathExpandOutcome::Forked;
    }

    ShipAiPathNode* const duplicate = clone_node(node, host);  // 009E3531 / 009E3723
    if (duplicate == nullptr) {
        return ShipAiPathExpandOutcome::AllocationFailed;
    }
    ship_ai_path_node_link_009d9230(*duplicate, left);    // 009E3553 / 009E3743
    ship_ai_path_node_link_alt_009d92e0(*duplicate, right);  // 009E355B / 009E374D
    link_side(node, duplicate, side);                     // 009E3563 / 009E3755
    return ShipAiPathExpandOutcome::Forked;
}

}  // namespace

ShipAiPathExpandResult ship_ai_path_expand_009e3330(ShipAiPathNode& start,
                                                    float margin,
                                                    std::int32_t depth,
                                                    ShipAiPathSearchHost& host) {
    ShipAiPathExpandResult result{};
    ShipAiPathNode* node = &start;
    std::int32_t counter = depth;
    std::int32_t side = 0;

    for (;;) {
        if (node->link_minus == nullptr && node->link_plus == nullptr) {  // 009E3342 / 009E334A
            result.outcome = ShipAiPathExpandOutcome::LeafReached;
            result.at = node;
            result.depth = counter;
            return result;
        }
        counter += 1;  // 009E3352
        node->direction = ship_ai_path_node_choose_direction_009d5930(*node);  // 009E3355
        if (node->direction == -1) {  // 009E335A
            if (node->short_link == 0) {  // 009E335F
                side = -1;
                break;
            }
            node = node->link_minus;  // 009E3365
        } else {
            if (node->field_32 == 0) {  // 009E336A
                side = 1;
                break;
            }
            node = node->link_plus;  // 009E3374
        }
        if (node == nullptr) {
            // Guard, not the image: a decided direction always has its link, so
            // 009E3342 never reads through a null. The image would fault here.
            result.outcome = ShipAiPathExpandOutcome::LeafReached;
            result.depth = counter;
            return result;
        }
    }

    result.at = node;
    result.depth = counter;

    ShipAiPathEdgeProbe probe = ship_ai_path_probe_edge_009e3040(*node, side, margin, host);
    if (probe.allocation_failed) {
        result.outcome = ShipAiPathExpandOutcome::AllocationFailed;
        return result;
    }
    if (!probe.blocked) {  // 009E339D / 009E35A5
        mark_edge_resolved(*node, side);
        result.outcome = ShipAiPathExpandOutcome::EdgeCleared;
        return result;
    }
    if (counter >= kShipAiPathSearchDepthLimit) {  // 009E33A3 / 009E35AB
        mark_edge_resolved(*node, side);
        result.outcome = ShipAiPathExpandOutcome::BudgetExhausted;
        return result;
    }

    ShipAiPathNode* const far = edge_far(*node, side);
    if (probe.right != nullptr && candidate_is_redundant(*node, *far, *probe.right)) {
        host.release_path_node_vtable0(probe.right);  // 009E340D / 009E3615
        probe.right = nullptr;
    }
    if (probe.left != nullptr && candidate_is_redundant(*node, *far, *probe.left)) {
        host.release_path_node_vtable0(probe.left);   // 009E346E / 009E3676
        probe.left = nullptr;
    }
    if (probe.left == nullptr && probe.right == nullptr) {  // 009E3472 / 009E367A
        mark_edge_resolved(*node, side);
        result.outcome = ShipAiPathExpandOutcome::DeadEnd;
        return result;
    }

    result.outcome = splice_detour(*node, side, probe.left, probe.right, host);
    return result;
}

// ---------------------------------------------------------------------------

void ship_ai_path_prune_009d9550(ShipAiPathNode& start, ShipAiPathSearchHost& host) {
    ShipAiPathNode* const node = &start;
    ShipAiPathNode* const minus = node->link_minus;
    ShipAiPathNode* const plus = node->link_plus;

    if (minus != nullptr && plus != nullptr) {  // 009D9559 / 009D9563
        bool keep_minus;
        if (node->short_link == 0) {  // 009D9565
            keep_minus = false;
        } else if (node->field_32 == 0) {  // 009D956A
            keep_minus = true;
        } else if (!ship_ai_path_search_route_resolved_009d5a20(*minus)) {  // 009D9571
            keep_minus = false;
        } else if (!ship_ai_path_search_route_resolved_009d5a20(*plus)) {  // 009D957C
            keep_minus = true;
        } else {
            // 009D9585..009D959B, no intermediate float store: the image sums in
            // the x87 stack. 009D9599 JBE sends a tie, and an unordered pair, to
            // link_plus.
            const double minus_cost = static_cast<double>(minus->remaining_length) +
                                      node->link_minus_length + node->cost_minus;
            const double plus_cost = static_cast<double>(plus->remaining_length) +
                                     node->link_plus_length + node->cost_plus;
            keep_minus = plus_cost > minus_cost;
        }
        if (keep_minus) {  // 009D959D, drop link_plus
            host.release_path_node_vtable0(plus);
            node->field_32 = 0;              // 009D95AE
            node->link_plus = nullptr;       // 009D95B1
            node->direction = -1;            // 009D95B4
        } else {  // 009D95BD, drop link_minus
            host.release_path_node_vtable0(minus);
            node->short_link = 0;            // 009D95CE
            node->link_minus = nullptr;      // 009D95D1
            node->direction = 1;             // 009D95D4
        }
    }

    if (node->link_plus != nullptr) {  // 009D95DB, move the survivor into link_minus
        ship_ai_path_node_set_back_plus(*node->link_plus, nullptr);  // 009D95E5
        node->link_plus->back = node;                                // 009D95EB
        node->link_minus_length = node->link_plus_length;            // 009D95F4
        node->cost_plus = 0.0f;                                      // 009D95F7
        node->link_minus = node->link_plus;                          // 009D95FC
        node->link_plus = nullptr;                                   // 009D95FF
    }

    ShipAiPathNode* const next = node->link_minus;  // 009D9602
    node->cost_minus = 0.0f;                        // 009D960D
    node->short_link = 1;                           // 009D961A
    node->is_start = 0;                             // 009D961E
    node->direction = (next != nullptr) ? -1 : 0;   // 009D9621, NEG/SBB on the pointer
    node->field_32 = 0;                             // 009D9624
    node->link_plus_length = kShipAiPathSearchNoLinkLength;  // 009D9627
    if (next != nullptr) {
        ship_ai_path_prune_009d9550(*next, host);   // 009D962E
    }

    // 009D9633..009D968B: splice out every following node whose incoming link is
    // under one unit. 009D964C JBE leaves an unordered length in place.
    while (node->link_minus != nullptr &&
           kShipAiPathShortLinkLimit > node->link_minus_length) {
        ShipAiPathNode* const middle = node->link_minus;
        ShipAiPathNode* const after = middle->link_minus;  // 009D9651
        if (after == nullptr) {  // 009D9656
            break;
        }
        node->link_minus_length = middle->link_minus_length;  // 009D965B
        middle->back = nullptr;                               // 009D965E
        middle->link_minus = nullptr;                         // 009D9664
        ship_ai_path_node_set_back_plus(*middle, nullptr);    // 009D966A
        middle->link_plus = nullptr;                          // 009D9670
        host.release_path_node_vtable0(middle);               // 009D9680
        node->link_minus = after;                             // 009D9682
        after->back = node;                                   // 009D9685
    }
}

// ---------------------------------------------------------------------------

bool ship_ai_path_smooth_009d96a0(ShipAiPathNode& start, ShipAiPathSearchHost& host) {
    ShipAiPathNode* node = &start;
    for (;;) {
        node->cost_minus = 0.0f;  // 009D96AF
        node->is_start = 0;       // 009D96B4
        ShipAiPathNode* const middle = node->link_minus;
        if (middle == nullptr) {  // 009D96AD / 009D96B8
            return false;
        }
        ShipAiPathNode* const after = middle->link_minus;
        if (after == nullptr) {  // 009D96C5
            return false;
        }

        // 009D9706 / 009D972A: either leg under three units forces the removal.
        const float far_leg_sq = planar_distance_sq(after->x, after->z, middle->x, middle->z);
        bool degenerate = kShipAiPathSearchSmoothMinLegSq > far_leg_sq;  // 009D9714 JA
        if (!degenerate) {
            const float near_leg_sq = planar_distance_sq(middle->x, middle->z, node->x, node->z);
            degenerate = kShipAiPathSearchSmoothMinLegSq > near_leg_sq;  // 009D9734 JA
        }

        // 009D973E..009D97E7: how much further the path turns to reach the node
        // after the middle one than to reach the middle one.
        const float bearing_after = heading_angle_00414eb0({after->x - node->x, after->z - node->z});
        const float bearing_middle =
            heading_angle_00414eb0({middle->x - node->x, middle->z - node->z});
        const float delta = wrapped_angle_subtract_00438b10(bearing_after, bearing_middle);

        // 009D97EB..009D9842: min(10 / max(leg, 1), 0.05) radians.
        float leg = node->link_minus_length;
        if (kShipAiPathShortLinkLimit > leg) {  // 009D97F8 FCOMIP 1.0 / 009D97FC JBE
            leg = kShipAiPathShortLinkLimit;
        }
        float tolerance =
            static_cast<float>(kShipAiPathSearchSmoothToleranceScale / static_cast<double>(leg));
        if (static_cast<double>(tolerance) > kShipAiPathSearchSmoothToleranceLimit) {  // 009D9830 JBE
            tolerance = kShipAiPathSearchSmoothToleranceClamp;
        }

        bool remove = degenerate;
        if (!degenerate) {  // 009D9842 TEST BL,BL
            const std::int8_t side = ship_ai_path_node_side(*middle);
            if (delta > tolerance && side > 0) {  // 009D9856 JBE / 009D985E JG
                remove = true;
            } else if (-tolerance > delta && side < 0) {  // 009D9868 JBE / 009D9871 JL
                remove = true;
            }
            if (!remove) {
                node = middle;  // 009D9873, the tail call on the next node
                continue;
            }
        }

        // 009D9885..009D98B1
        ship_ai_path_node_link_009d9230(*node, after);  // 009D988E
        middle->back = nullptr;                         // 009D9893
        middle->link_minus = nullptr;                   // 009D989A
        host.release_path_node_vtable0(middle);         // 009D98A9
        if (degenerate) {
            node->short_link = 1;  // 009D98AF, the new link is not re-tested
        }
        return true;
    }
}

// ---------------------------------------------------------------------------

ShipAiPathSearchTickResult ship_ai_path_search_tick_009ec680(ShipAiPathPlanBlock& plan,
                                                             float frame_delta,
                                                             ShipAiPathSearchHost& host) {
    ShipAiPathSearchTickResult result{};
    result.state_before = static_cast<ShipAiPathSearchState>(plan.search_state);
    result.state_after = result.state_before;

    // 009EC683..009EC68F: the delay runs down before the state is even read, so
    // every tick pays it, including the ones that do nothing.
    plan.revalidate_delay = plan.revalidate_delay - frame_delta;
    result.revalidate_delay = plan.revalidate_delay;

    const std::int32_t state = plan.search_state;
    if (state == 0 || state == 2 || state == 7) {  // 009EC692 / 009EC69B / 009EC6A4
        return result;
    }
    if (plan.owner == nullptr) {  // 009EC6AA
        plan.search_state = static_cast<std::int32_t>(ShipAiPathSearchState::Failed);  // 009EC6B0
        result.state_after = ShipAiPathSearchState::Failed;
        result.action = ShipAiPathSearchAction::FailedNoOwner;
        return result;
    }
    if (plan.head == nullptr) {
        // Guard, not the image: every case below hands plan+20h to a __thiscall
        // and the image faults on a null head.
        result.head_missing = true;
        return result;
    }

    // 009EC6D8 FLD float ptr [ESI+0x2c]: the field is a float, not the dword the
    // planner header types it as. See the Corrections table in the doc.
    float side_switch_penalty = 0.0f;
    std::memcpy(&side_switch_penalty, &plan.search_context, sizeof(float));

    switch (state) {
    case 1:
    case 5:
        // 009EC6D8, the Seeded and ExtendSearch case share one jump-table entry.
        result.action = ShipAiPathSearchAction::Expanded;
        ship_ai_path_search_cost_009ec280(*plan.head, side_switch_penalty, host);  // 009EC6E2
        result.expand = ship_ai_path_expand_009e3330(*plan.head, kShipAiPathSearchMargin,
                                                     0, host);                    // 009EC6F6
        result.route_resolved = ship_ai_path_search_route_resolved_009d5a20(*plan.head);  // 009EC6FE
        if (result.route_resolved) {  // 009EC705
            plan.search_state = static_cast<std::int32_t>(ShipAiPathSearchState::Searched);
        }
        break;
    case 3:
        result.action = ShipAiPathSearchAction::Pruned;
        ship_ai_path_prune_009d9550(*plan.head, host);  // 009EC715
        plan.search_state = static_cast<std::int32_t>(ShipAiPathSearchState::Extracting);  // 009EC71A
        plan.node_count = 0;                                                               // 009EC721
        break;
    case 4:
        result.action = ShipAiPathSearchAction::Smoothed;
        result.node_removed = ship_ai_path_smooth_009d96a0(*plan.head, host);  // 009EC72F
        // 009EC734 NEG AL / SBB / ADD 6: a removal means another pass is needed.
        plan.search_state = static_cast<std::int32_t>(
            result.node_removed ? ShipAiPathSearchState::ExtendSearch
                                : ShipAiPathSearchState::Extracted);
        break;
    case 6:
        result.action = ShipAiPathSearchAction::Finished;
        plan.search_state = static_cast<std::int32_t>(ShipAiPathSearchState::Ready);  // 009EC742
        break;
    default:
        break;
    }
    result.state_after = static_cast<ShipAiPathSearchState>(plan.search_state);
    return result;
}

}  // namespace bsp
