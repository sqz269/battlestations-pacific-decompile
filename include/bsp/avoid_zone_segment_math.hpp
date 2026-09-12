#pragma once

#include "bsp/system_camera_axes.hpp"
#include <array>

namespace bsp {

// Complete native instruction schedules; names are hypotheses. Evidence and ABI
// adaptations: docs/AVOID_ZONE_SEGMENT_MATH.md. These are Win32 implementations.
// Four stack pointers, RET 10h, AL success. Solves a0+t*da = b0+u*db;
// near-zero denominator returns false without touching either output. Reads all
// inputs before either output store; if t and u alias, u is written last.
bool __fastcall native_segment_parameters_004f3630(const float* a0, const float* da,
    const float* b0, const float* db, float* t, float* u) noexcept;

// Original ECX=start, EDX=end, stack query, ST0 distance, RET4. This typed
// interface adds required borrowed native CRT access. It preserves the native
// ignored solver-failure path: t retains the query POINTER's binary32 bits and
// u retains normalized dx, including for zero/short segments. No repair policy.
float avoid_zone_segment_distance_00419ab0(const std::array<float, 2>& start,
    const std::array<float, 2>& end, const std::array<float, 2>& query,
    const CameraAxesCrtAccess& crt);

// Original ECX points to [start.x,start.y,end.x,end.y]; stack output/query;
// EAX=output, RET8. Preserves ignored-failure t bits, unordered clamp behavior,
// and binary32 interpolation spills. Inputs are read before output writes, so
// output may alias query or either endpoint pair. CRT binding is explicit.
std::array<float, 2>& avoid_zone_segment_closest_point_004f4b50(
    const std::array<float, 4>& endpoints, std::array<float, 2>& output,
    const std::array<float, 2>& query, const CameraAxesCrtAccess& crt);

} // namespace bsp
