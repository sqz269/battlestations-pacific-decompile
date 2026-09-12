// Packet cc_ai_path_follower: 009E3C00, 009D5930, 009D6550 and 004F3970.
// Evidence, original ABI and uncertainty: docs/SHIP_AI_PATH_FOLLOWER.md.
// Names are hypotheses, not recovered symbols.
//
// Float discipline. The image is x87 throughout: every FSTP to a float slot
// rounds to binary32 and every intermediate that stays in a register keeps 80
// bits. Where the image multiplies or divides two float32 values and only then
// stores, this file computes the intermediate in double and rounds once, which
// reproduces the exact-product case and leaves a double-rounding difference of
// at most one ulp in the divide cases. The build is MSVC Win32 with SSE2 float,
// so the plain float expressions below are correctly rounded binary32.

#include "bsp/ship_ai_path_follower.hpp"

#include <cmath>

#include "bsp/vector_helpers.hpp"

namespace bsp {

std::int32_t ship_ai_path_node_decide_direction_009d5930(ShipAiPathNode& node) noexcept {
    if (node.direction != 0) {  // 009D5931 CMP [ECX+4],0 / 009D5935 JNZ 009D5986
        return node.direction;
    }
    // 009D5937-009D5986 is the rule bsp/ship_ai_path_planner.hpp projects from
    // 009D9E72-009D9ED4, instruction for instruction: the same three cases and
    // the same FCOMIP at 009D5970, which sets the carry when the plus route is
    // strictly cheaper.
    node.direction = ship_ai_path_node_direction_009d9e6c(node);
    return node.direction;
}

ShipAiPathTangentChord ship_ai_path_tangent_chord_004f3970(
    const ShipAiCircleTangentCircle& circle,
    const std::array<float, 2>& point) noexcept {
    ShipAiPathTangentChord result{};

    const float dx = point[0] - circle.x;  // 004F3987
    const float dz = point[1] - circle.z;  // 004F399A
    // 004F39A5-004F39E1, the 00414C60 kernel inlined.
    const float len = length_2d_00414c60({{dx, dz}});
    // 004F39EF COMISS 1e-6 against len, 004F39F4 JA: a point at the centre has
    // no chord. An unordered compare falls through here, as JA is not taken.
    if (kShipAiPathFollowerBisectorEpsilon > len) {
        return result;
    }

    const float ux = dx / len;  // 004F39FE
    const float uz = dz / len;  // 004F3A08

    // 004F3A21 stores len - radius, 004F3A29-004F3A32 clears its sign bit.
    const float gap = std::fabs(len - circle.radius);
    // 004F3A36 COMISS 1e-6 against |gap|, 004F3A3B JBE continues: a point ON
    // the circle has no chord. JBE also continues on unordered (NaN radius).
    if (kShipAiPathFollowerBisectorEpsilon > gap) {
        return result;
    }

    result.valid = true;
    if (len > circle.radius) {  // 004F3A4D FCOMI / 004F3A51 JBE
        // 004F3A57-004F3B19, the point is outside the circle.
        // 004F3A6C-004F3A7A: the foot of the tangent chord, r^2/len along u.
        const float foot =
            static_cast<float>(static_cast<double>(circle.radius) * circle.radius / len);
        // 004F3A7E-004F3A96: the tangent length, sqrt(len^2 - r^2).
        const float radicand = static_cast<float>(static_cast<double>(len) * len -
                                                  static_cast<double>(circle.radius) * circle.radius);
        const float tangent = std::sqrt(radicand);
        // 004F3A9A-004F3AAC: half the chord, tangent * r / len.
        const float half = static_cast<float>(static_cast<double>(tangent) * circle.radius / len);
        // 004F3AB0-004F3AEE and 004F3AF5-004F3B19.
        result.base = {{circle.x + ux * foot, circle.z + uz * foot}};
        result.half_chord = {{-uz * half, ux * half}};
    } else {
        // 004F3B22-004F3B91, the point is inside the circle: the chord through
        // the point perpendicular to the centre direction.
        const float radicand =
            static_cast<float>(static_cast<double>(circle.radius) * circle.radius -
                               static_cast<double>(len) * len);
        const float half = std::sqrt(radicand);
        result.base = point;  // 004F3B46-004F3B62
        result.half_chord = {{-uz * half, ux * half}};
    }
    return result;
}

ShipAiPathTangentPair ship_ai_path_tangent_points_009d6550(
    const ShipAiCircleTangentCircle& circle,
    const std::array<float, 2>& point) noexcept {
    ShipAiPathTangentPair result{};
    // 009D6553-009D6562: the two out pointers are stack locals of 009D6550, and
    // 004F3970's `base` lands in the second, its `half_chord` in the first.
    const ShipAiPathTangentChord chord = ship_ai_path_tangent_chord_004f3970(circle, point);
    if (!chord.valid) {  // 009D6567 TEST AL,AL / 009D6569 JZ 009D65C9
        return result;   // neither caller buffer is written
    }
    result.valid = true;
    // 009D656B-009D659F, the sum.
    result.first = {{chord.half_chord[0] + chord.base[0], chord.half_chord[1] + chord.base[1]}};
    // 009D65A2-009D65BE, the difference, base minus half_chord.
    result.second = {{chord.base[0] - chord.half_chord[0], chord.base[1] - chord.half_chord[1]}};
    return result;
}

ShipAiPathNode* ship_ai_path_cursor_node_009e3c6c(ShipAiPathPlanBlock& plan) noexcept {
    ShipAiPathNode* node = plan.head;  // 009E3C72
    // 009E3C6C reads plan+34h, 009E3C6F/009E3C75 skip the walk at or below zero
    // and 009E3D05/009E3D0A decrement and re-enter.
    for (std::int32_t step = plan.node_count; step > 0; --step) {
        if (node == nullptr) {
            // Deviation: 009E3C80 re-reads [ESI+4] with no test, so a cursor
            // longer than the path faults in the image.
            return nullptr;
        }
        node->direction = ship_ai_path_node_direction_009d9e6c(*node);  // 009E3C80-009E3CE1
        node = ship_ai_path_node_next_009d9ee6(*node);                  // 009E3CE1-009E3D03
    }
    return node;
}

ShipAiPathFollowerResult ship_ai_path_follower_point_009e3c00(
    ShipAiPathPlanBlock& plan,
    ShipAiPathPointRecord& record,
    ShipAiPathFollowerHost& host) {
    ShipAiPathFollowerResult result{};
    const std::array<float, 2> pose{{record.query_x_00, record.query_z_04}};

    // 009E3C03-009E3C37, the prologue, which runs on every exit.
    plan.follower_60 = pose[0];
    plan.follower_64 = pose[1];
    plan.follower_4c = kShipAiPathFollowerMinLegLength;
    plan.follower_50 = pose[0];
    plan.follower_54 = pose[1];
    record.direction_1c = 0;  // 009E3C34
    record.node_18 = 0;       // 009E3C37

    if (plan.head == nullptr) {  // 009E3C3A
        plan.node_count = 0;     // 009E3C3F, one of the nine cursor writers
        record.point_x_08 = pose[0];
        record.point_z_0c = pose[1];
        record.steer_enabled_21 = false;  // 009E3C4E
        record.more_path_20 = false;      // 009E3C54
        record.next_x_10 = pose[0];
        record.next_z_14 = pose[1];
        result.exit = ShipAiPathFollowerExit::EmptyPlan;
        return result;
    }
    record.steer_enabled_21 = true;  // 009E3C68

    ShipAiPathNode* const cursor = ship_ai_path_cursor_node_009e3c6c(plan);
    if (cursor == nullptr) {
        // Deviation, see ship_ai_path_cursor_node_009e3c6c: 009E3D11 hands a
        // null cursor straight to 009D5930, which dereferences it.
        record.point_x_08 = pose[0];
        record.point_z_0c = pose[1];
        record.more_path_20 = true;
        result.exit = ShipAiPathFollowerExit::NoTargetNode;
        return result;
    }

    ship_ai_path_node_decide_direction_009d5930(*cursor);                // 009E3D13
    ShipAiPathNode* target = ship_ai_path_node_next_009d9ee6(*cursor);   // 009E3D18-009E3D34
    ShipAiPathNode* after = nullptr;
    if (target != nullptr) {                                            // 009E3D36
        ship_ai_path_node_decide_direction_009d5930(*target);           // 009E3D3C
        after = ship_ai_path_node_next_009d9ee6(*target);               // 009E3D41-009E3D5B
    }

    if (after == nullptr) {           // 009E3D5F
        record.more_path_20 = true;   // 009E3D65
        if (target == nullptr) {      // 009E3D63
            // 009E3D6B-009E3D81: the point is the pose and +10h/+14h are left
            // as the caller had them, which the consumer does not read because
            // it gates on the zero at +18h first (009EE5F9).
            record.point_x_08 = pose[0];
            record.point_z_0c = pose[1];
            record.direction_1c = 0;  // 009E3D77
            result.exit = ShipAiPathFollowerExit::NoTargetNode;
            return result;
        }
        after = target;               // 009E3D84
    } else {
        record.more_path_20 = false;  // 009E3D88
    }

    // 009E3D8C-009E3E03: the target's point, offset sideways when the node
    // carries a lateral record.
    const ShipAiPathLateralAnchor* const anchor =
        target->field_10 != 0 ? host.lateral_anchor_node_10(target->field_10) : nullptr;
    std::array<float, 2> point =
        anchor != nullptr ? std::array<float, 2>{{anchor->x, anchor->z}}
                          : std::array<float, 2>{{target->x, target->z}};
    if (anchor != nullptr) {  // 009E3DAB re-tests node+10h
        const float limit = host.order_turn_limit_at_00811d80(point);  // 009E3DCD
        point[0] = limit * anchor->dir_x + point[0];                   // 009E3DE2, 009E3DEF
        point[1] = limit * anchor->dir_z + point[1];                   // 009E3DE8, 009E3DFB
    }

    // 009E3E07-009E3E47, the two legs around the target.
    const std::array<float, 2> in_leg{{point[0] - cursor->x, point[1] - cursor->z}};
    const std::array<float, 2> out_leg{{after->x - point[0], after->z - point[1]}};
    // 009E3E4B-009E3E9B.
    const float distance_to_point =
        length_2d_00414c60({{point[0] - pose[0], point[1] - pose[1]}});
    // 009E3EA1-009E3EB9, the look-ahead radius.
    const float look_ahead =
        static_cast<float>(static_cast<double>(host.owner_class_turn_radius_0082e850()) *
                           kShipAiPathFollowerLookAheadScale);
    // 009E3EBC-009E3ECC, the floor on how far ahead the published point sits.
    float radius_step = static_cast<float>(static_cast<double>(host.owner_radius_09c8()) *
                                           kShipAiPathFollowerRadiusScale);
    const float in_len = length_2d_00414c60(in_leg);    // 009E3ED0-009E3F0C
    const float out_len = length_2d_00414c60(out_leg);  // 009E3F12-009E3F4E
    bool overshot = false;                              // 009E3F58

    std::array<float, 2> steer{{0.0f, 0.0f}};
    std::array<float, 2> aim = point;

    // 009E3F54 and 009E3F69-009E3F76: a corner needs the target to have at
    // least one link and the following leg to be at least one unit long. The
    // COMISS at 009E3F71 falls through on an unordered compare.
    const bool has_corner = (target->link_minus != nullptr || target->link_plus != nullptr) &&
                            !(kShipAiPathFollowerMinLegLength > out_len);
    if (has_corner) {
        record.more_path_20 = false;  // 009E3F88
        record.next_x_10 = point[0];  // 009E3F90
        record.next_z_14 = point[1];  // 009E3FA9

        // 009E3F7C-009E3FF8: the corner bisector, the incoming leg reversed
        // plus the outgoing leg, each divided by its own length.
        const float back_len = -in_len;  // 009E3F86 FCHS
        std::array<float, 2> bisector{{in_leg[0] / back_len + out_leg[0] / out_len,
                                       in_leg[1] / back_len + out_leg[1] / out_len}};
        float bisector_len = length_2d_00414c60(bisector);  // 009E3FFC
        if (!(kShipAiPathFollowerBisectorEpsilon <= bisector_len)) {  // 009E400F/009E4011
            // 009E4013-009E4063: a straight-through corner has no bisector, so
            // the offset is taken perpendicular to the following leg, the side
            // byte choosing which perpendicular.
            result.degenerate_bisector = true;
            if (static_cast<std::int8_t>(target->field_14) < 0) {  // 009E4013 JGE
                bisector = {{out_leg[1], -out_leg[0]}};            // 009E401B-009E4035
            } else {
                bisector = {{-out_leg[1], out_leg[0]}};            // 009E4037-009E4051
            }
            bisector_len = out_len;  // 009E4057
        }
        bisector[0] = bisector[0] / bisector_len;  // 009E406B
        bisector[1] = bisector[1] / bisector_len;  // 009E4075

        if (!record.more_path_20) {  // 009E4067 / 009E4085, always true here
            // 009E4087-009E40ED. The normal of the following leg, unnormalised.
            const std::array<float, 2> normal{{-out_leg[1], out_leg[0]}};
            const float offset_from_node = static_cast<float>(
                static_cast<double>(pose[1] - target->z) * normal[1] +
                static_cast<double>(pose[0] - target->x) * normal[0]);
            const float bisector_side = static_cast<float>(
                static_cast<double>(bisector[1]) * normal[1] +
                static_cast<double>(bisector[0]) * normal[0]);
            if (static_cast<double>(offset_from_node) * static_cast<double>(bisector_side) <
                kShipAiPathFollowerOvershootLimit) {
                overshot = true;  // 009E40ED
            }
        }

        // 009E40F2-009E4139: the steering circle, one look-ahead clear of the
        // corner along the bisector.
        const std::array<float, 2> offset{{bisector[0] * look_ahead, bisector[1] * look_ahead}};
        const std::array<float, 2> centre{{point[0] + offset[0], point[1] + offset[1]}};
        plan.follower_60 = centre[0];  // 009E4152
        plan.follower_64 = centre[1];  // 009E414D
        plan.follower_4c = look_ahead; // 009E4155

        ShipAiCircleTangentCircle circle{};
        circle.x = centre[0];
        circle.z = centre[1];
        circle.radius = look_ahead;
        // 009E413D-009E415E pre-seed the two out buffers with the centre and
        // the offset VECTOR, and 009D6550 leaves them alone when it fails.
        std::array<float, 2> first = centre;
        std::array<float, 2> second = offset;
        const ShipAiPathTangentPair pair = ship_ai_path_tangent_points_009d6550(circle, pose);
        if (pair.valid) {  // 009E4162
            first = pair.first;
            second = pair.second;
        }
        result.tangent_found = pair.valid;
        // 009E4167-009E4185: a negative side byte takes the second tangent.
        aim = static_cast<std::int8_t>(target->field_14) < 0 ? second : first;
        steer = {{aim[0] - pose[0], aim[1] - pose[1]}};  // 009E4185, 009E41AF
        result.exit = ShipAiPathFollowerExit::CornerTangent;
    } else {
        // 009E4197-009E41A7: no corner, steer straight at the target point.
        record.more_path_20 = true;  // 009E419B
        steer = {{point[0] - pose[0], point[1] - pose[1]}};
        result.exit = ShipAiPathFollowerExit::StraightAtPoint;
    }

    // 009E41AB-009E4221.
    if (target->link_minus == nullptr && target->link_plus == nullptr) {
        // 009E41CF-009E41E9: the target is the dangling goal node, so the point
        // is never placed past it. Together with the max at 009E42BF this
        // pins the step to the distance to the goal exactly; the min alone is
        // redundant and the doc records why it is kept.
        if (radius_step > distance_to_point) {
            radius_step = distance_to_point;
        }
    } else if (look_ahead > distance_to_point || overshot) {  // 009E41F3, 009E41F9
        // 009E4200-009E4221: the ship is inside the look-ahead of the target,
        // or past it, so try to skip the target. The cursor advances only when
        // the straight run from the ship to the node AFTER the target clears
        // the avoid zones. This is the only site in the image that advances
        // plan+34h.
        result.tested_shortcut = true;
        const std::uint32_t manager = host.avoid_zone_manager_004218e0();
        // The fourth argument is output only; 009E4205 hands it the aim buffer,
        // which the no-corner path never wrote.
        std::array<float, 2> hit = aim;
        const std::array<float, 2> to{{after->x, after->z}};
        if (!host.segment_hits_zone_00417ef0(manager, plan.zone_layer, pose, to, hit)) {
            plan.node_count = plan.node_count + 1;  // 009E421F
            result.advanced_cursor = true;
        }
    }

    // 009E4223-009E4246: the side code and the anchor handle, published only
    // when a corner was found.
    if (!record.more_path_20) {
        const std::int8_t side = static_cast<std::int8_t>(target->field_14);
        record.direction_1c = side > 0 ? 2 : (side != 0 ? 1 : 0);  // 009E4229-009E4240
        record.node_18 = target->field_10;                         // 009E4243
    }

    // 009E4249-009E42A5.
    float steer_len = length_2d_00414c60(steer);
    if (kShipAiPathFollowerMinSteerLength > static_cast<double>(steer_len)) {  // 009E428F
        steer_len = kShipAiPathFollowerSteerLengthFloor;
    }
    // 009E42AB-009E42CD: how far ahead the published point sits.
    float step = radius_step;
    if (distance_to_point > radius_step) {
        step = distance_to_point;
    }
    // 009E42D3-009E4320.
    const float scale = step / steer_len;
    const std::array<float, 2> published{{scale * steer[0] + pose[0], scale * steer[1] + pose[1]}};
    plan.follower_50 = published[0];  // 009E4312
    plan.follower_54 = published[1];  // 009E4319
    record.point_x_08 = published[0]; // 009E4320
    record.point_z_0c = published[1]; // 009E431D

    result.overshot = overshot;
    result.look_ahead = plan.follower_4c;
    result.distance_to_point = distance_to_point;
    result.step_length = step;
    return result;
}

}  // namespace bsp
