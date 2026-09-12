#include "bsp/ship_ai_lateral_record.hpp"

#include "bsp/geometry_helpers.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include "bsp/unit_rudder.hpp"

#include <array>
#include <utility>

namespace bsp {
namespace {
// Original binary32 stores delimit these x87 operations.
float subtract(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fsub right
        fstp result
    }
    return result;
}
float add(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fadd right
        fstp result
    }
    return result;
}
float multiply(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fmul right
        fstp result
    }
    return result;
}
float divide(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fdiv right
        fstp result
    }
    return result;
}

float outgoing_length(float x, float z, const CameraAxesCrtAccess& crt) {
    // 0041A2FC..0041A33E. Unlike 00419260, neither square is spilled before
    // the sum. The sum is stored once, compared to double 1e-10, then sqrt.
    float squared;
    __asm {
        fld x
        fld z
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp squared
    }
    if (!(static_cast<double>(squared) > 1.0e-10)) return 0.0f;
    const auto* access = &crt;
    float result;
    __asm {
        fld squared
        mov ecx, access
        call native_crt_sqrt_st0_00bf7030
        fstp result
    }
    return result;
}
} // namespace

ShipAiPathLateralRecord* ship_ai_lateral_record_at_00417610(
    const ShipAiPathLateralRecordList& zone, std::int32_t index) noexcept {
    return zone.records[index % zone.count]; // IDIV at 00417619, EDX remainder
}

bool ship_ai_lateral_records_derive_0041a200(ShipAiPathLateralRecordList& zone,
    bool reverse_positive_turn, const CameraAxesCrtAccess& crt) {
    float total_turn = 0.0f;
    for (std::int32_t i = 0; i != zone.count; ++i) {
        auto& record = *zone.records[i];
        const auto& previous = *zone.records[i == 0 ? zone.count - 1 : i - 1];
        const auto& next = *zone.records[i == zone.count - 1 ? 0 : i + 1];
        std::array<float, 2> incoming{
            subtract(record.x, previous.x), subtract(record.z, previous.z)};
        const float incoming_reciprocal =
            native_vector2_reciprocal_length_00419260(incoming.data(), &crt);
        incoming[0] = multiply(incoming[0], incoming_reciprocal);
        incoming[1] = multiply(incoming[1], incoming_reciprocal);

        std::array<float, 2> outgoing{
            subtract(next.x, record.x), subtract(next.z, record.z)};
        record.outgoing_length = outgoing_length(outgoing[0], outgoing[1], crt);
        outgoing[0] = divide(outgoing[0], record.outgoing_length);
        outgoing[1] = divide(outgoing[1], record.outgoing_length);
        record.outgoing_x = outgoing[0];
        record.outgoing_z = outgoing[1];

        // Inlined 00414EB0 sequence at 0041A374..0041A3E2; no native CALL
        // to 00414EB0 exists here. Reuse its already reconstructed rule.
        const float incoming_heading = heading_angle_00414eb0(incoming);
        const float outgoing_heading = heading_angle_00414eb0(outgoing);
        record.signed_turn = wrapped_angle_subtract_00438b10(
            outgoing_heading, incoming_heading);
        const float direction_x_sum = add(incoming[0], outgoing[0]);
        const float direction_z_sum = add(incoming[1], outgoing[1]);
        record.offset_dir_x = direction_z_sum;
        record.offset_dir_z = -direction_x_sum;
        const std::array<float, 2> normal{record.offset_dir_x, record.offset_dir_z};
        const float normal_reciprocal =
            native_vector2_reciprocal_length_00419260(normal.data(), &crt);
        const float normalized_x = multiply(record.offset_dir_x, normal_reciprocal);
        const float normalized_z = multiply(record.offset_dir_z, normal_reciprocal);
        record.offset_dir_x = normalized_x;
        record.offset_dir_z = normalized_z;
        total_turn = add(record.signed_turn, total_turn);
    }
    if (!(total_turn > 0.0f)) return true; // native JBE also accepts unordered
    if (reverse_positive_turn) {
        for (std::int32_t left = 0, right = zone.count - 1; left < right;
             ++left, --right) {
            std::swap(zone.records[left], zone.records[right]);
        }
        (void)ship_ai_lateral_records_derive_0041a200(zone, false, crt);
    }
    return false; // 0041A4C5 XOR AL,AL, regardless of recursive return
}

ShipAiPathLateralAnchor ship_ai_lateral_anchor_projection(
    const ShipAiPathLateralRecord& record) noexcept {
    return {record.x, record.z, record.offset_dir_x, record.offset_dir_z};
}
} // namespace bsp
