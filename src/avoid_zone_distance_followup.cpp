#include "bsp/avoid_zone_distance_followup.hpp"

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_offset.hpp"
#include "bsp/avoid_zone_segment_math.hpp"

#include <limits>

namespace bsp {
namespace {
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

bool expanded_bounds_contain(const AvoidZoneNativeStorage& native,
    const std::array<float, 2>& point, float margin) noexcept {
    const auto* source = &native;
    const float* query = point.data();
    float min_x, min_z, max_x, max_z, coordinate;
    unsigned char admitted;
    // 004174A7..00417530: retain the shared x87 margin operand, every
    // binary32 bound/coordinate spill, FCOMI ordering and unordered rejection.
    // The offsets reuse AvoidZoneNativeStorage's producer-backed layout.
    __asm {
        mov eax, source
        fld dword ptr [eax + 14h]
        fstp min_x
        fld dword ptr [eax + 18h]
        fstp min_z
        fld dword ptr [eax + 1ch]
        fstp max_x
        fld dword ptr [eax + 20h]
        fstp max_z
        fld min_x
        fld margin
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp min_x
        fld max_x
        fadd st(0), st(1)
        fstp max_x
        fld min_z
        fsub st(0), st(1)
        fstp min_z
        fadd max_z
        fstp max_z
        mov eax, query
        fld dword ptr [eax]
        fstp coordinate
        fld min_x
        fld coordinate
        fcomi st(0), st(1)
        fstp st(1)
        jc rejected_pop
        fld max_x
        fcomip st(0), st(1)
        fstp st(0)
        jbe rejected
        fld dword ptr [eax + 4]
        fstp coordinate
        fld min_z
        fld coordinate
        fcomi st(0), st(1)
        fstp st(1)
        jc rejected_pop
        fld max_z
        fcomip st(0), st(1)
        fstp st(0)
        jbe rejected
        mov admitted, 1
        jmp finished
    rejected_pop:
        fstp st(0)
    rejected:
        mov admitted, 0
    finished:
    }
    return admitted != 0;
}
} // namespace

float segment_pair_distance_004f49f0(const std::array<float, 2>& a0,
    const std::array<float, 2>& a1, const std::array<float, 2>& b0,
    const std::array<float, 2>& b1, const CameraAxesCrtAccess& crt) {
    std::array<float, 2> ignored_crossing;
    if (native_segment_crossing_004f3730(a0.data(), a1.data(), b0.data(),
            b1.data(), ignored_crossing.data())) { // 004F4A0A; RET0Ch.
        return 0.0f; // 004F4A13 FLDZ; RET8.
    }
    // Native FSTPs at 004F4A29/37/45/53 round every dependency result before
    // any comparison. Keep the original call order and borrowed point objects.
    const float to_a_from_b0 = avoid_zone_segment_distance_00419ab0(
        a0, a1, b0, crt); // 004F4A24.
    const float to_a_from_b1 = avoid_zone_segment_distance_00419ab0(
        a0, a1, b1, crt); // 004F4A32.
    const float to_b_from_a0 = avoid_zone_segment_distance_00419ab0(
        b0, b1, a0, crt); // 004F4A40.
    const float to_b_from_a1 = avoid_zone_segment_distance_00419ab0(
        b0, b1, a1, crt); // 004F4A4E.
    const float best_b = improves(to_b_from_a0, to_b_from_a1)
        ? to_b_from_a0 : to_b_from_a1; // 004F4A5F/63.
    const float best_a = improves(to_a_from_b0, to_a_from_b1)
        ? to_a_from_b0 : to_a_from_b1; // 004F4A81/85.
    return improves(best_a, best_b) ? best_a : best_b; // 004F4AA3/A7.
}

float avoid_zone_segment_boundary_distance_004171f0(
    const ShipAiPathLateralRecordList& zone,
    const std::array<float, 2>& segment_start,
    const std::array<float, 2>& segment_end, const CameraAxesCrtAccess& crt) {
    float best = std::numeric_limits<float>::max(); // 00D7A248 = 7F7FFFFFh.
    const auto start = segment_start;
    const auto end = segment_end;
    const auto* last = zone.records[zone.count - 1]; // Unconditional native read.
    std::array<float, 2> previous{last->x, last->z};
    auto* cursor = zone.records;
    const auto* finish = zone.records + zone.count;
    while (cursor != finish) {
        const std::array<float, 2> current{(*cursor)->x, (*cursor)->z};
        // 00417295; the float result retains the 0041729A binary32 boundary.
        const float candidate = segment_pair_distance_004f49f0(
            previous, current, start, end, crt);
        if (improves(candidate, best)) best = candidate; // 004172A6/A8/AA.
        ++cursor;
        previous = current;
    }
    return best; // 004172D3 FLD; 004172DC RET8.
}

bool avoid_zone_expanded_boundary_query_004174a0(
    const AvoidZoneNativeStorage& native, const AvoidZonePolygon& polygon,
    const std::array<float, 2>& point, float margin,
    AvoidZoneExpandedBoundaryQuery& output) noexcept {
    if (!expanded_bounds_contain(native, point, margin)) {
        output.within_bounds = 0; // 0041756E; every other output stays intact.
        return false;
    }
    output.within_bounds = 1; // 00417538, before either geometry call.
    output.contained = static_cast<std::uint8_t>(
        avoid_zone_contains_point_00416b50(polygon, point)); // 0041753C/47.
    avoid_zone_closest_offset_point_00416f30(native.corners, point, output.point,
        output.edge, output.endpoint, 0.0f); // 00417556; +0 from FLDZ/FSTP.
    return output.within_bounds != 0; // 0041755B; AL, RET0Ch at 00417564.
}
} // namespace bsp
