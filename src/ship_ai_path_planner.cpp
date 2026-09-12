// The planner the ship AI hands its goal to: 009E3780 and the node graph it
// seeds. Evidence, original ABI and uncertainty: docs/SHIP_AI_PATH_PLANNER.md.
// Names are hypotheses, not recovered symbols.

#include "bsp/ship_ai_path_planner.hpp"

#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// 00415510 BSP_Math_MinFloatByRef, __fastcall(ECX = &a, EDX = &b) -> ST0, body
// 00415510-0041554E. 00415525 FCOMIP compares b with a and 00415529 JBE returns
// b, so an unordered pair also returns b. Both 009E3780 call sites pass
// addresses of floats already in memory.
float min_float_by_ref_00415510(float a, float b) noexcept {
    return (b > a) ? a : b;
}

// The squared planar distance the routine builds and rounds to float before
// every comparison (009E39BD..009E39C5, 009E3AA1..009E3AA9, 009E3BA4..009E3BAC).
float planar_distance_sq(float ax, float az, float bx, float bz) noexcept {
    const float dx = ax - bx;
    const float dz = az - bz;
    const float sq = dx * dx + dz * dz;
    return sq;
}

}  // namespace

ShipAiPathNode ship_ai_path_node_init_009d9150(const std::array<float, 2>& xz,
                                               std::uint32_t zone_layer) noexcept {
    ShipAiPathNode node{};
    node.x = xz[0];          // 009D9163
    node.z = xz[1];          // 009D9168
    node.zone_layer = zone_layer;  // 009D91A9
    return node;
}

void ship_ai_path_node_link_009d9230(ShipAiPathNode& node, ShipAiPathNode* next) noexcept {
    if (next == nullptr) {  // 009D9237 TEST EAX,EAX / 009D923C JZ
        return;
    }
    node.link_minus = next;  // 009D9242
    next->back = &node;      // 009D9248
    // 009D9245..009D9272 is the 00414C60 kernel inline: the rounded sum of
    // squares against the double 1e-10 at 00CE3820, then the CRT square root.
    const float length =
        length_2d_00414c60({node.x - next->x, node.z - next->z});
    node.link_minus_length = length;  // 009D92AF
    // 009D92AC COMISS 1.0, length / 009D92B4 JBE: unordered also clears it.
    node.short_link = (kShipAiPathShortLinkLimit > length) ? std::uint8_t{1} : std::uint8_t{0};
}

bool ship_ai_path_goal_within_tolerance_009e399e(float goal_x, float goal_z,
                                                 float latched_x, float latched_z,
                                                 float drift_sq) noexcept {
    const float sq = planar_distance_sq(goal_x, goal_z, latched_x, latched_z);
    // 009E39D0 FCOMIP drift_sq against sq / 009E39D4 JBE: the plan dies when the
    // tolerance is not strictly greater, and an unordered pair dies too.
    return drift_sq > sq;
}

float ship_ai_path_revalidate_delay_009e3abf(float owner_radius_09c8,
                                             float owner_class_max_speed_0500,
                                             bool has_owner) noexcept {
    float delay = kShipAiPathRevalidateCap;  // 009E3ACC
    if (!has_owner) {                        // 009E3AD1 JZ
        return delay;
    }
    // 009E3AEB min(100.0f, owner+9C8h), 009E3AF6 FDIV class+500h, 009E3B06
    // min(delay, quotient). The divide runs in the x87 stack and is rounded to
    // float by the store at 009E3B02.
    const float capped = min_float_by_ref_00415510(kShipAiPathRevalidateRadiusCap,
                                                   owner_radius_09c8);
    const float quotient = capped / owner_class_max_speed_0500;
    delay = min_float_by_ref_00415510(delay, quotient);
    return delay;  // 009E3B0B FSTP [EBP]
}

ShipAiPathCorridorResult ship_ai_path_corridor_009e3a57(const std::array<float, 2>& pose,
                                                        const std::array<float, 2>& node,
                                                        const std::array<float, 2>& next,
                                                        float capture_radius_sq) noexcept {
    ShipAiPathCorridorResult result{};

    // 009E3A5B..009E3AB9: the ship is on plan while it is inside the head node's
    // own circle, whatever the segment does.
    const float to_node_sq = planar_distance_sq(pose[0], pose[1], node[0], node[1]);
    if (!(to_node_sq > capture_radius_sq)) {  // 009E3AB5 FCOMIP / 009E3AB9 JBE
        result.verdict = ShipAiPathCorridorVerdict::InsideCapture;
        result.plan_still_valid = true;
        return result;
    }

    // 009E3A6E..009E3A8E the segment, 009E3B12 its length, 009E3B25..009E3B31
    // the unit direction. A zero length divides by zero here as it does there.
    const float seg_x = next[0] - node[0];
    const float seg_z = next[1] - node[1];
    const float length = length_2d_00414c60({seg_x, seg_z});
    result.segment_length = length;
    const float unit_x = seg_x / length;
    const float unit_z = seg_z / length;

    const float to_x = pose[0] - node[0];
    const float to_z = pose[1] - node[1];
    // 009E3B3B..009E3B4D: (unit_z * to_z) + (unit_x * to_x), in that order.
    const float projection = unit_z * to_z + unit_x * to_x;
    result.projection = projection;
    if (!(projection > 0.0f)) {  // 009E3B57 FCOMI against zero / 009E3B5B JBE
        result.verdict = ShipAiPathCorridorVerdict::BehindSegment;
        result.plan_still_valid = false;
        return result;
    }

    // 009E3B61..009E3B83, the perpendicular foot on the segment.
    const float step_x = unit_x * projection;
    const float step_z = unit_z * projection;
    const float foot_x = step_x + node[0];
    const float foot_z = node[1] + step_z;
    const float offset_sq = planar_distance_sq(pose[0], pose[1], foot_x, foot_z);
    if (!(offset_sq > capture_radius_sq)) {  // 009E3BB8 FCOMIP / 009E3BBC JBE
        result.verdict = ShipAiPathCorridorVerdict::InsideCorridor;
        result.plan_still_valid = true;
    } else {
        result.verdict = ShipAiPathCorridorVerdict::OutsideCorridor;
        result.plan_still_valid = false;
    }
    return result;
}

std::int32_t ship_ai_path_node_direction_009d9e6c(const ShipAiPathNode& node) noexcept {
    if (node.direction != 0) {  // 009D9E6C, a decided node keeps its choice
        return node.direction;
    }
    if (node.link_minus == nullptr) {  // 009D9E72
        return node.link_plus != nullptr ? 1 : 0;
    }
    if (node.link_plus == nullptr) {  // 009D9E86
        return -1;
    }
    // 009D9E96..009D9ED4: the plus side wins only when it is strictly cheaper.
    const float plus = node.cost_plus + node.link_plus_length + node.link_plus->remaining_length;
    const float minus = node.cost_minus + node.link_minus_length + node.link_minus->remaining_length;
    return plus < minus ? 1 : -1;
}

ShipAiPathNode* ship_ai_path_node_next_009d9ee6(const ShipAiPathNode& node) noexcept {
    // 009D9EE6..009D9EFE, the same test in both walks.
    if (node.link_plus != nullptr && node.direction == 1) {
        return node.link_plus;
    }
    if (node.link_minus != nullptr && node.direction == -1) {
        return node.link_minus;
    }
    return nullptr;
}

float ship_ai_path_remaining_length_009d9e50(ShipAiPathPlanBlock& plan,
                                             const std::array<float, 2>& pose) noexcept {
    if (plan.head == nullptr) {  // 009D9E53
        return 0.0f;
    }
    ShipAiPathNode* node = plan.head;  // 009D9E5E
    // 009D9E64: the walk is bounded by node_count + 1 steps.
    for (std::int32_t step = plan.node_count; step >= 0; --step) {
        if (node == nullptr) {  // 009D9E6A, leaves through 009D9F04
            break;
        }
        node->direction = ship_ai_path_node_direction_009d9e6c(*node);
        node = ship_ai_path_node_next_009d9ee6(*node);
    }
    if (node == nullptr) {  // 009D9F00 / 009D9F04
        node = plan.goal_node;
    }
    if (node == nullptr) {
        // Not reachable in the image: 009D9F04 dereferences plan+24h without a
        // test. Guarded here rather than faulting.
        return 0.0f;
    }
    // 009D9F0B..009D9F5A: the node's own remaining length plus the straight
    // line from the pose to it, through the 00414C60 kernel.
    const float gap = length_2d_00414c60({pose[0] - node->x, pose[1] - node->z});
    return node->remaining_length + gap;
}

ShipAiPathArrivalResult ship_ai_path_arrival_009da590(bool latch_2fe,
                                                      float goal_x, float goal_z,
                                                      float latched_x, float latched_z) noexcept {
    ShipAiPathArrivalResult result{};
    if (!latch_2fe) {  // 009DA593 CMP byte [ECX+2FEh],0 / 009DA59A JZ
        return result;
    }
    const float sq = planar_distance_sq(goal_x, goal_z, latched_x, latched_z);
    // 009DA5E8 FCOMIP the double 6400.0 against sq / 009DA5EC JBE.
    if (kShipAiPathArrivalRadiusSq > static_cast<double>(sq)) {
        result.reached = true;  // 009DA5EE
        return result;
    }
    result.clears_latch = true;  // 009DA5F6
    return result;
}

void ship_ai_path_plan_reset_009d9d40(ShipAiPathPlanBlock& plan,
                                      ShipAiPathPlannerHost& host) noexcept {
    if (plan.head != nullptr) {  // 009D9D49 / 009D9D4B
        host.release_node_list_vtable0(plan.head);  // 009D9D53 CALL EDX
    }
    plan.goal_node = nullptr;     // 009D9D5D
    plan.head = nullptr;          // 009D9D60
    plan.node_count = 0;          // 009D9D63
    plan.zone_layer = 0;          // 009D9D66
    plan.search_state = static_cast<int>(ShipAiPathSearchState::Empty);  // 009D9D69
    plan.goal_clearance = kShipAiPathNoClearance;  // 009D9D6D
}

ShipAiPathPlanRequestResult ship_ai_path_plan_request_009e3780(
    ShipAiPathPlanBlock& plan,
    const std::array<float, 2>& pose,
    const std::array<float, 2>& goal,
    std::uint32_t zone_layer,
    float unused_owner_radius,
    ShipAiPathPlannerHost& host) {
    // The fourth stack argument is pushed by all four call sites and read by
    // none of the body; 009E3ADB takes the same field through plan+3Ch.
    (void)unused_owner_radius;

    ShipAiPathPlanRequestResult result{};

    for (;;) {
        // 009E378D, and again at 009E37BC after the reset below.
        if (plan.search_state == static_cast<int>(ShipAiPathSearchState::Failed)) {
            result.outcome = ShipAiPathPlanOutcome::SearchFailed;
            return result;
        }
        const int state = plan.search_state;  // 009E3796
        if (state == static_cast<int>(ShipAiPathSearchState::Empty)) {
            break;  // 009E379B, the seeding branch at 009E37CC
        }
        if (plan.zone_layer != zone_layer) {  // 009E379D CMP [ESI+38h],EBX
            result.outcome = ShipAiPathPlanOutcome::LayerChanged;
            return result;
        }

        if (state > static_cast<int>(ShipAiPathSearchState::Searched)) {
            // 009E39E5: a graph exists, so the corridor decides.
            ShipAiPathNode* node = plan.head;
            ShipAiPathNode* next = node != nullptr ? node->link_minus : nullptr;
            if (node == nullptr || next == nullptr) {  // 009E39EA / 009E39F1
                ship_ai_path_plan_reset_009d9d40(plan, host);  // 009E39F5
                result.accepted = true;                        // 009E39FC MOV AL,1
                result.outcome = ShipAiPathPlanOutcome::GraphMissing;
                return result;
            }
            if (!ship_ai_path_goal_within_tolerance_009e399e(goal[0], goal[1],
                                                             plan.latched_goal_x,
                                                             plan.latched_goal_z,
                                                             plan.goal_drift_sq)) {
                result.outcome = ShipAiPathPlanOutcome::GoalDrifted;  // 009E3BD1
                return result;
            }
            // 009E3A44 COMISS 0 against plan+14h / 009E3A51 JC: the corridor is
            // only re-tested once the delay has run out.
            if (!(plan.revalidate_delay <= 0.0f)) {
                result.accepted = true;  // 009E3A4C set the byte to 1
                result.outcome = ShipAiPathPlanOutcome::RevalidateDeferred;
                return result;
            }
            const float to_node_sq =
                planar_distance_sq(pose[0], pose[1], node->x, node->z);
            if (!(to_node_sq > plan.capture_radius_sq)) {  // 009E3AB9 JBE
                result.accepted = true;
                result.outcome = ShipAiPathPlanOutcome::CorridorHeld;
                return result;
            }
            // 009E3ABF..009E3B0B: leaving the node's circle re-arms the delay.
            plan.revalidate_delay = ship_ai_path_revalidate_delay_009e3abf(
                host.owner_radius_09c8(), host.owner_class_max_speed_0500(),
                plan.owner != nullptr);
            const ShipAiPathCorridorResult corridor = ship_ai_path_corridor_009e3a57(
                pose, {node->x, node->z}, {next->x, next->z}, plan.capture_radius_sq);
            result.accepted = corridor.plan_still_valid;
            result.outcome = corridor.plan_still_valid
                                 ? ShipAiPathPlanOutcome::CorridorHeld
                                 : ShipAiPathPlanOutcome::CorridorLeft;
            return result;
        }

        if (plan.goal_node != nullptr) {  // 009E37AF
            // 009E399E: while the search runs, only the goal is watched.
            const bool ok = ship_ai_path_goal_within_tolerance_009e399e(
                goal[0], goal[1], plan.latched_goal_x, plan.latched_goal_z,
                plan.goal_drift_sq);
            result.accepted = ok;
            result.outcome = ok ? ShipAiPathPlanOutcome::SearchInProgress
                                : ShipAiPathPlanOutcome::GoalDrifted;
            return result;
        }
        ship_ai_path_plan_reset_009d9d40(plan, host);  // 009E37B7, then loop
    }

    // -----------------------------------------------------------------------
    // 009E37CC, the seeding branch
    // -----------------------------------------------------------------------
    std::array<float, 2> pose_copy = pose;  // 009E37D0, 009E37E3
    std::array<float, 2> goal_copy = goal;  // 009E37EE, 009E37D8

    plan.latched_goal_x = goal[0];  // 009E37F2
    plan.latched_goal_z = goal[1];  // 009E37FA
    plan.zone_layer = zone_layer;   // 009E37FD
    plan.goal_clearance = kShipAiPathNoClearance;                        // 009E3819
    plan.search_state = static_cast<int>(ShipAiPathSearchState::Seeded);  // 009E381E

    std::uint32_t manager = host.avoid_zone_manager_004218e0();  // 009E3821
    std::uint32_t zone =
        host.zone_containing_point_00417e40(manager, goal_copy, plan.zone_layer);  // 009E3831
    if (zone != 0) {
        // 009E3851: the goal was inside a zone, so it moves to the boundary and
        // the clearance the search would have measured is zero.
        goal_copy = host.push_point_out_of_zone_00417580(zone, goal_copy,
                                                         kShipAiPathGoalZoneMargin);
        plan.goal_clearance = 0.0f;  // 009E386B / 009E38DE
    } else {
        manager = host.avoid_zone_manager_004218e0();  // 009E3870
        const std::uint32_t group =
            host.zone_group_for_layer_004120d0(manager, plan.zone_layer);  // 009E387B
        if (group != 0) {
            const std::array<float, 2> nearest = host.nearest_zone_boundary_0041b840(
                group, goal_copy, kShipAiPathClearanceSearchRadius,
                kShipAiPathClearancePush);  // 009E38A2
            const float clearance = length_2d_00414c60(
                {nearest[0] - goal_copy[0], nearest[1] - goal_copy[1]});  // 009E38C3
            // 009E38D2 FCOMIP 1.0 against the length / 009E38D6 JC.
            if (clearance >= kShipAiPathClearanceFloor) {
                plan.goal_clearance = clearance;  // 009E38DE
            }
        }
    }

    manager = host.avoid_zone_manager_004218e0();  // 009E38E3
    zone = host.zone_containing_point_00417e40(manager, pose, plan.zone_layer);  // 009E38EF
    if (zone != 0) {
        pose_copy = host.push_point_out_of_zone_00417580(zone, pose,
                                                         kShipAiPathPoseZoneMargin);  // 009E390B
    }

    ShipAiPathNode* start = host.allocate_path_node_00bf681b(kShipAiPathNodeSize);  // 009E3927
    if (start != nullptr) {
        *start = ship_ai_path_node_init_009d9150(pose_copy, plan.zone_layer);  // 009E393E
    }
    plan.head = start;  // 009E3949

    ShipAiPathNode* target = host.allocate_path_node_00bf681b(kShipAiPathNodeSize);  // 009E394C
    if (target != nullptr) {
        *target = ship_ai_path_node_init_009d9150(goal_copy, plan.zone_layer);  // 009E3963
    }
    plan.goal_node = target;  // 009E3970

    result.accepted = true;  // 009E3995 MOV AL,BL, and BL is 1 from 009E380E
    result.seeded = true;
    if (plan.head == nullptr) {
        // The image calls 009D9230 and reads plan+20h without a test here
        // (009E396C, 009E3986), so a failed allocation faults there. Stopping
        // instead is the one deliberate deviation in this routine.
        result.outcome = ShipAiPathPlanOutcome::NodeAllocationFailed;
        return result;
    }
    ship_ai_path_node_link_009d9230(*plan.head, plan.goal_node);  // 009E3973
    plan.head->seed = host.owner_seed_vtable50();                 // 009E3980
    plan.head->is_start = 1;                                      // 009E3991
    result.outcome = ShipAiPathPlanOutcome::Seeded;
    return result;
}

}  // namespace bsp
