#include "bsp/avoid_zone_offset.hpp"

#include <limits>

namespace bsp {
namespace {
// Keep the native x87 operations and the binary32 stores between them. These
// private helpers are inlined source arithmetic, not invented native callees.
float store_x87(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
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
float dot(float delta_x, float delta_z, float outgoing_x, float outgoing_z) noexcept {
    float result;
    __asm {
        fld outgoing_z
        fmul delta_z
        fld delta_x
        fmul outgoing_x
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
float squared_length(float delta_x, float delta_z) noexcept {
    float result;
    __asm {
        fld delta_z
        fld delta_x
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
bool clamp_to_start(float parameter) noexcept {
    unsigned char result;
    __asm {
        fld parameter
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        setae result
    }
    return result != 0;
}
bool clamp_to_end(float parameter) noexcept {
    const float one = 1.0f;
    unsigned char result;
    __asm {
        movss xmm0, parameter
        movss xmm1, one
        comiss xmm0, xmm1
        setae result
    }
    return result != 0;
}
bool improves(float candidate, float best) noexcept {
    unsigned char result;
    __asm {
        fld candidate
        fld best
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool has_push(float amount) noexcept {
    unsigned char result;
    __asm {
        movss xmm0, amount
        xorps xmm1, xmm1
        ucomiss xmm0, xmm1
        setne al
        setp dl
        or al, dl
        mov result, al
    }
    return result != 0;
}
} // namespace

void avoid_zone_closest_offset_point_00416f30(
    const ShipAiPathLateralRecordList& zone, const std::array<float, 2>& point,
    std::array<float, 2>& out_point, std::array<float, 4>& out_edge,
    std::uint8_t& out_endpoint, float push) noexcept {
    float best_squared = std::numeric_limits<float>::max(); // 00D7A248
    if (zone.count == 0) return; // 00416F54/56/5C; no record dereference yet
    std::int32_t previous = zone.count - 1;
    for (std::int32_t current = 0; current != zone.count; ++current) {
        const auto& start = *zone.records[previous];
        const auto& end = *zone.records[current];
        const float query_x = store_x87(point[0]);
        const float query_z = store_x87(point[1]);
        const float dx = subtract(query_x, start.x);
        const float dz = subtract(query_z, start.z);
        const float along = dot(dx, dz, start.outgoing_x, start.outgoing_z);
        float parameter = divide(along, start.outgoing_length);
        std::array<float, 2> candidate;
        std::uint8_t endpoint;
        // 00416FD5 FCOMIP(0,t); JC includes unordered. Then 00417004
        // COMISS(t,1); JC again includes unordered: NaN must interpolate.
        if (clamp_to_start(parameter)) {
            candidate = {start.x, start.z};
            parameter = 0.0f;
            endpoint = 1;
        } else if (clamp_to_end(parameter)) {
            candidate = {end.x, end.z};
            parameter = 1.0f;
            endpoint = 1;
        } else {
            const float edge_x = subtract(end.x, start.x);
            const float edge_z = subtract(end.z, start.z);
            const float scaled_x = multiply(edge_x, parameter);
            const float scaled_z = multiply(edge_z, parameter);
            candidate = {add(start.x, scaled_x), add(start.z, scaled_z)};
            endpoint = 0;
        }
        const float distance_x = subtract(store_x87(candidate[0]), query_x);
        const float distance_z = subtract(store_x87(candidate[1]), query_z);
        const float distance_squared = squared_length(distance_x, distance_z);
        if (improves(distance_squared, best_squared)) { // reject tie/NaN
            // 004170BF..C6 skips both signed zeroes, but NOT unordered push.
            if (has_push(push)) {
                const float end_x = multiply(parameter, end.offset_dir_x);
                const float end_z = multiply(end.offset_dir_z, parameter);
                const float complement = subtract(1.0f, parameter);
                const float start_x = multiply(start.offset_dir_x, complement);
                const float start_z = multiply(complement, start.offset_dir_z);
                const float offset_x = add(start_x, end_x);
                const float offset_z = add(start_z, end_z);
                const float pushed_x = multiply(offset_x, push);
                const float pushed_z = multiply(offset_z, push);
                candidate = {add(candidate[0], pushed_x), add(candidate[1], pushed_z)};
            }
            best_squared = distance_squared; // rank before applying push
            out_point[0] = candidate[0];
            out_point[1] = candidate[1];
            out_endpoint = endpoint;
            out_edge[0] = store_x87(start.x);
            out_edge[1] = store_x87(start.z);
            out_edge[2] = store_x87(end.x);
            out_edge[3] = store_x87(end.z);
        }
        previous = current; // native reloads the array/count before advancing
    }
}
} // namespace bsp
