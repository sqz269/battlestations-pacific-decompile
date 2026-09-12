// Packet cc_exe_2q. See docs/GAME_EXECUTABLE.md milestone 2q for the evidence.

#include "bsp/ship_ai_path_point.hpp"

#include <cmath>

namespace bsp {
namespace {

// The 00414C60 kernel, inlined at 009E3E67..009E3E9B, 009E3EDA..009E3F03,
// 009E3EF4.. and 009E42D3: the squared sum is rounded to float32, compared
// against the double 1e-10 at 00CE3820, and only then square-rooted.
constexpr double kCutoff = 1.0e-10;

float planar_length(float dx, float dz) noexcept {
    const float square = dz * dz + dx * dx;
    if (static_cast<double>(square) <= kCutoff) {
        return 0.0f;
    }
    return static_cast<float>(std::sqrt(static_cast<double>(square)));
}

}  // namespace

ShipAiPathPointResult ship_ai_path_point_009e3c00(ShipAiPathPlanBlock& plan,
                                                  ShipAiPathPointRecord& record,
                                                  ShipAiPathPointHost& host)
{
    ShipAiPathPointResult out{};

    // 009E3C11..009E3C31: the follower slots are seeded from the query point,
    // and plan+4Ch takes the 1.0f at 00D7A24C.
    plan.follower_60 = record.query_x_00;
    plan.follower_64 = record.query_z_04;
    plan.follower_4c = kShipAiPathPointCornerLegFloor;
    plan.follower_50 = record.query_x_00;
    plan.follower_54 = record.query_z_04;
    record.direction_1c = 0;   // 009E3C34
    record.node_18 = 0;        // 009E3C37

    // 009E3C3A CMP dword ptr [ECX+20h],EBX / JNZ 009E3C68.
    if (plan.head == nullptr) {
        plan.node_count = 0;                     // 009E3C3F
        record.point_x_08 = record.query_x_00;   // 009E3C42/45
        record.point_z_0c = record.query_z_04;   // 009E3C48/4B
        record.steer_enabled_21 = false;         // 009E3C4E
        record.more_path_20 = false;             // 009E3C54
        record.next_x_10 = record.query_x_00;    // 009E3C51/57
        record.next_z_14 = record.query_z_04;    // 009E3C5A/5D
        return out;
    }
    out.has_head = true;
    record.steer_enabled_21 = true;              // 009E3C68

    // 009E3C6C..009E3D0A: the walk. EDX counts down from plan+34h and the loop
    // test at 009E3D08 is JG, so a node_count of 0 skips it entirely.
    ShipAiPathNode* node = plan.head;
    std::int32_t budget = plan.node_count;
    while (budget > 0 && node != nullptr) {
        if (node->direction == 0) {                                   // 009E3C80
            node->direction = ship_ai_path_node_direction_009d9e6c(*node);
        }
        ShipAiPathNode* const next = ship_ai_path_node_next_009d9ee6(*node);  // 009E3CE1
        if (next == nullptr) {
            // 009E3D03 XOR ESI,ESI, and the image keeps looping on the null.
            out.walk_ran_out = true;
            break;
        }
        node = next;
        ++out.walk_steps;
        --budget;                                                     // 009E3D05
    }
    out.stopped = node;
    if (node == nullptr) {
        return out;
    }

    // 009E3D13: the stopped node's direction, then its successor.
    node->direction = ship_ai_path_node_choose_direction_009d5930(*node);
    ShipAiPathNode* successor = ship_ai_path_node_next_009d9ee6(*node);  // 009E3D18..009E3D34
    ShipAiPathNode* after = nullptr;
    if (successor != nullptr) {
        // 009E3D3C on the successor, then the same link test again.
        successor->direction = ship_ai_path_node_choose_direction_009d5930(*successor);
        after = ship_ai_path_node_next_009d9ee6(*successor);  // 009E3D41..009E3D5B
    }
    out.successor = successor;
    out.after = after;

    // 009E3D5F TEST EBX,EBX / JNZ 009E3D88.
    if (after == nullptr) {
        record.more_path_20 = true;   // 009E3D65, the byte at record+20h
        if (successor == nullptr) {   // 009E3D63 TEST EDI,EDI / 009E3D69 JNZ
            record.point_x_08 = record.query_x_00;  // 009E3D6B/6E
            record.point_z_0c = record.query_z_04;  // 009E3D71/74
            record.direction_1c = 0;                // 009E3D77, EDI is zero here
            out.no_successor = true;
            return out;
        }
        after = successor;            // 009E3D84
    } else {
        record.more_path_20 = false;  // 009E3D88
    }

    // 009E3D8C..009E3E01: the successor's point. A null lateral record takes
    // the node's own +18h/+1Ch; a non-null one is offset along its +10h/+14h
    // pair by 00811D80's answer.
    float point_x = successor->x;
    float point_z = successor->z;
    if (successor->field_10 != 0) {
        const std::array<float, 2> offset = host.lateral_point_offset_00811d80(
            successor->field_10, std::array<float, 2>{point_x, point_z});  // 009E3DCD
        point_x += offset[0];
        point_z += offset[1];
    }

    // 009E3E07..009E3F14.
    const float query_x = record.query_x_00;
    const float query_z = record.query_z_04;
    float dir_x = point_x - node->x;   // 009E3E23
    float dir_z = point_z - node->z;   // 009E3E33
    const float out_x = after->x - point_x;  // 009E3E3C
    const float out_z = after->z - point_z;  // 009E3E45
    const float to_successor = planar_length(point_x - query_x, point_z - query_z);  // 009E3E67
    const float turn_radius = static_cast<float>(
        static_cast<double>(host.class_turn_radius_0082e850())
        * kShipAiPathPointTurnRadiusScale);            // 009E3EAE, 009E3EB3
    float advance = static_cast<float>(static_cast<double>(host.owner_radius_09c8())
                                       * kShipAiPathPointOwnerRadiusScale);  // 009E3EC0, 009E3EC6
    const float incoming_length = planar_length(dir_x, dir_z);   // 009E3EDA
    const float outgoing_length = planar_length(out_x, out_z);   // 009E3EF4
    static_cast<void>(incoming_length);

    // 009E3F0A..009E3F1A: the corner arm runs only for a successor that still
    // has a link and an outgoing leg of at least one unit.
    const bool successor_is_end
        = successor->link_minus == nullptr && successor->link_plus == nullptr;
    if (successor_is_end || outgoing_length < kShipAiPathPointCornerLegFloor) {
        record.more_path_20 = true;      // 009E419B
        dir_x = point_x - query_x;       // 009E4197..009E41A3
        dir_z = point_z;                 // 009E41A7, the query z is subtracted below
    } else {
        // 009E3F1A..009E4222. Not projected; the caller is told.
        out.corner_arm = true;
        host.corner_arm_009e3f1a();
        return out;
    }
    dir_z = dir_z - query_z;             // 009E41AF

    // 009E41AB..009E41E1: the two arms of the clearance test. The successor
    // being the end of the chain takes the first, which only lowers `advance`.
    if (successor_is_end) {
        if (to_successor < advance) {
            advance = to_successor;
        }
    }
    // The `else if (to_successor < turn_radius || crossed)` arm asks the
    // avoid-zone manager through 00417EF0; it is unreachable from the branch
    // above, so nothing here calls it.
    static_cast<void>(turn_radius);

    // 009E4223..009E4246: the record's direction code and lateral record, taken
    // from the successor, only while the +20h byte is clear.
    if (!record.more_path_20) {
        const int side = static_cast<int>(ship_ai_path_node_side(*successor));
        record.direction_1c = (side > 0) ? 2 : ((side != 0) ? 1 : 0);
        record.node_18 = successor->field_10;
    }

    // 009E428B..009E4320: normalise the chosen direction, floor it at 0.1, and
    // advance the query point along it by max(advance, |successor - query|).
    float length = planar_length(dir_x, dir_z);
    if (static_cast<double>(length) < static_cast<double>(0.1)) {  // 009E428F/95/99
        length = kShipAiPathPointMinDirectionLength;
    }
    if (advance < to_successor) {  // 009E42B9/BD
        advance = to_successor;
    }
    const float scale = advance / length;      // 009E42D8
    const float px = scale * dir_x + query_x;  // 009E42E6, 009E42FA
    const float pz = scale * dir_z + query_z;  // 009E42EE, 009E4306
    plan.follower_50 = px;                     // 009E4312
    plan.follower_54 = pz;                     // 009E4319
    record.point_z_0c = pz;                    // 009E431D
    record.point_x_08 = px;                    // 009E4320
    out.direction_length = length;
    out.advance = advance;
    return out;
}

}  // namespace bsp
