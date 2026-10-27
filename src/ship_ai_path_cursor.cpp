#include "bsp/ship_ai_path_cursor.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_shipai_cc13.hpp"

namespace bsp {

// 007ADC60-007ADCBD.
bool ship_ai_path_on_final_leg_007adc60(const ShipAiPathCursor& cursor,
                                        bool has_path, int point_count) {
    if (!has_path) return true;                     // 007ADC63/007ADC71, the null test
    if (point_count <= 0) return true;              // 007ADC78/007ADC7C, JG
    switch (cursor.follow_mode_0c) {                // 007ADC82, the mode at cursor+0Ch
    case kShipAiPathFollowSimple:                   // 007ADC87, CMP ECX,1
        // 007ADC96: the forward test comes first and 007ADCAE re-reads the byte,
        // so a backwards cursor can only end at index 0.
        if (cursor.forward_10) {
            return cursor.index_08 == point_count - 1;   // 007ADCA9, JZ 007ADC7E
        }
        return cursor.index_08 == 0;                     // 007ADCB4, JZ 007ADC7E
    case kShipAiPathFollowPingPong:                 // 007ADC8F, (mode-2) <= 1 unsigned
    case kShipAiPathFollowCircle:
        return false;                               // 007ADC94, AL already zero
    default:
        return false;                               // 007ADC92 JA 007ADCBC, same AL
    }
}

// 007ADCC0-007ADD4A.
int ship_ai_path_next_index_007adcc0(ShipAiPathCursor& cursor, int point_count) {
    const int index = cursor.index_08;              // 007ADCCA
    switch (cursor.follow_mode_0c) {                // 007ADCC4, the three SUB EAX,1
    case kShipAiPathFollowCircle:                   // 007ADCD7 JNZ falls here on 3
        if (cursor.forward_10) {                    // 007ADCD9, CMP byte,AL with AL=0
            if (index == point_count - 1) return 0; // 007ADCEB/007ADCEF, the wrap
            return index + 1;                       // 007ADCED -> 007ADD43
        }
        if (index == 0) return point_count - 1;     // 007ADCF6/007ADCFA/007ADD06
        return index - 1;                           // 007ADD06
    case kShipAiPathFollowPingPong:                 // 007ADCD2, JZ 007ADD0E
        if (index == 0) {                           // 007ADD0E
            cursor.forward_10 = true;               // 007ADD17
            return 1;                               // 007ADD12
        }
        if (index == point_count - 1) {             // 007ADD2D
            cursor.forward_10 = false;              // 007ADD34
            return index - 1;                       // 007ADD31
        }
        break;                                      // 007ADD2F, JNZ 007ADD3D
    case kShipAiPathFollowSimple:                   // 007ADCCD, JZ 007ADD3D
        break;
    default:
        return index;                               // 007ADCD7, JNZ 007ADD09
    }
    // 007ADD3D, the plain step both SIMPLE and the middle of a PINGPONG take.
    return cursor.forward_10 ? index + 1 : index - 1;
}

// 007ADFAC-007ADFD4.
bool ship_ai_path_advance_allowed_007adfac(const ShipAiPathCursor& cursor,
                                           bool has_path, int point_count) {
    if (!has_path) return false;                    // 007ADF90/007ADF9F
    if (point_count <= 0) return false;             // 007ADFA6/007ADFAA
    if (cursor.follow_mode_0c != kShipAiPathFollowSimple) {
        return true;                                // 007ADFB0, JNZ 007ADFD6
    }
    if (cursor.forward_10) {
        return cursor.index_08 != point_count - 1;  // 007ADFC5, JZ 007ADFFE
    }
    return cursor.index_08 != 0;                    // 007ADFD0, JZ 007ADFFE
}

// 007B1C50-007B1D2E.
void ship_ai_path_cursor_start_007b1c50(ShipAiPathCursor& cursor, int follow_mode,
                                        int start_mode, int joined_index,
                                        bool random_forward) {
    cursor.follow_mode_0c = follow_mode;            // 007B1C97
    cursor.forward_10 = true;                       // 007B1C9A
    if (start_mode == kShipAiPathStartBegin) {      // 007B1C90, CMP EAX,6
        cursor.index_08 = 0;                        // 007B1CA1
        return;                                     // 007B1CA9
    }
    if (start_mode == kShipAiPathStartJoinBackwards) {
        cursor.forward_10 = false;                  // 007B1CBB
    } else if (start_mode == kShipAiPathStartJoinRandomDir) {
        cursor.forward_10 = random_forward;         // 007B1CF5 / 007B1CFF
    } else {
        cursor.forward_10 = true;                   // 007B1D06, start mode 5
    }
    cursor.index_08 = joined_index;                 // 007B1D26
}

// A HYPOTHESIS for 007B1100, which was not read. See the header.
int ship_ai_path_nearest_index_007b1100(const float* points_xyz, int point_count,
                                        float x, float z) {
    if (points_xyz == nullptr || point_count <= 0) return 0;
    int best = 0;
    float best_d2 = -1.0f;
    for (int i = 0; i < point_count; ++i) {
        const float dx = points_xyz[i * 3 + 0] - x;
        const float dz = points_xyz[i * 3 + 2] - z;
        const float d2 = dx * dx + dz * dz;
        if (best_d2 < 0.0f || d2 < best_d2) {
            best_d2 = d2;
            best = i;
        }
    }
    return best;
}

// 007ADD70-007AE01A, ported from cc13_sweepB4_path_cursor_follow_step_007add70
// (its comment carries the per-instruction reading). The point count is read
// once (007ADD84); the path cannot change inside the call here.
ShipAiPathFollowStep ship_ai_path_follow_step_007add70(ShipAiPathCursor& cursor,
    const float* points_xyz, int point_count, float pos_x, float pos_z, float radius,
    std::int32_t count, float heading, float factor, bool force) {
    ShipAiPathFollowStep out;
    const bool has_path = points_xyz != nullptr;
    // 007ADF90 / 007AE002: null path, count <= 0, or the SIMPLE end point.
    auto at_end = [&]() {
        return !ship_ai_path_advance_allowed_007adfac(cursor, has_path, point_count);
    };
    int index = cursor.index_08;
    const bool forward = cursor.forward_10;
    int step = 0;   // the image leaves it uninitialised outside modes 1..3
    int limit;
    if (cursor.follow_mode_0c == kShipAiPathFollowSimple
        || cursor.follow_mode_0c == kShipAiPathFollowPingPong) {
        step = forward ? 1 : -1;
        limit = forward ? point_count - index : index;
    } else if (cursor.follow_mode_0c == kShipAiPathFollowCircle) {
        limit = point_count - 2;
        step = forward ? 1 : -1;
    } else {
        limit = count;
    }
    const float radius_sq = static_cast<float>(static_cast<double>(radius) * radius);
    if ((count & 0xFF) == 0 && limit > 1) limit = 1;     // 007ADDD9 / 007ADDE8
    out.limit = limit;
    if (limit <= 0) {                                    // 007ADDFA
        out.answer = at_end();
        return out;
    }
    bool hit = false;   // BL
    int advances = 0;
    for (int pass = 0; pass < limit; ++pass) {
        // cursor vtable +4h(out, index). LABELLED: the host clamps an index
        // outside the point vector; the image reads whatever the path answers.
        int at = index;
        if (at < 0) at = 0;
        if (point_count > 0 && at >= point_count) at = point_count - 1;
        const float px = has_path && point_count > 0 ? points_xyz[at * 3 + 0] : 0.0f;
        const float pz = has_path && point_count > 0 ? points_xyz[at * 3 + 2] : 0.0f;
        const float dx = px - pos_x;
        const float dz = pz - pos_z;
        float dist_sq;
        if constexpr (kPc24ShipAiCc13Bound) {
            // 007ADE4D FMUL ST0, 007ADE51 FMULP, 007ADE53 FADDP, each rounded at PC24.
            dist_sq = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(dz, dz),
                                                            cc13x87::x87_mul24(dx, dx)));
        } else {
            dist_sq =
                static_cast<float>(static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
        }
        hit = radius_sq > dist_sq;
        index += step;
        if (index >= point_count) {
            index = 0;
        } else if (index < 0) {
            index += point_count;
        }
        if (!hit) continue;
        if (factor > 0.0f && 1.0f > factor) {            // 007ADEAF-007ADF44
            float a = static_cast<float>(std::atan2(static_cast<double>(dz),
                static_cast<double>(dx)));
            a = static_cast<float>(1.5707963705062866 - a);              // 00CE3830
            if (0.0f > a) a = static_cast<float>(a + 6.2831854820251465);  // 00CE3828
            const float turn = wrapped_angle_subtract_00438b10(heading, a);
            const float s = static_cast<float>(std::sin(static_cast<double>(turn)));
            if constexpr (kPc24ShipAiCc13Bound) {
                // 007ADF19 FSUBRP 1 - f, 007ADF24 FMUL |s|, 007ADF28 FADDP f, 007ADF2A
                // FMUL radius, stored; then 007ADF3C FMULP reach * reach is rounded before
                // the 007ADF40 FCOMIP against the stored distance.
                const float reach = static_cast<float>(cc13x87::x87_mul24(
                    cc13x87::x87_add24(cc13x87::x87_mul24(cc13x87::x87_sub24(1.0, factor),
                                                          std::fabs(s)),
                                       factor),
                    radius));
                hit = cc13x87::x87_mul24(reach, reach) > dist_sq;
            } else {
            const float reach = static_cast<float>(
                ((1.0 - factor) * std::fabs(s) + factor) * static_cast<double>(radius));
            hit = static_cast<double>(reach) * reach > dist_sq;
            }
            if (!hit) continue;
        }
        out.hit_pass = pass;
        advances = pass + 1;                             // 007ADF6B
        break;
    }
    if (!hit) {
        if (!force) return out;
        advances = 1;
        hit = true;
    }
    for (;;) {                                           // 007ADF90-007ADFF0
        if (at_end()) {
            out.answer = hit ? at_end() : false;         // 007ADFFE / 007AE002
            return out;
        }
        hit = false;
        cursor.index_08 = ship_ai_path_next_index_007adcc0(cursor, point_count);
        ++out.advances;
        if (--advances <= 0) return out;
    }
}

}  // namespace bsp
