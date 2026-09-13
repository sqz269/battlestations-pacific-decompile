#pragma once

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Complete00415D70 normal instruction schedule, using the actual selected-edge
// run links and borrowed CRT state. Each normal gates on either endpoint's
// projection being strictly >1, then clips a negative endpoint at projection0.
// If either clipped endpoint lies strictly outside radius squared, consider
// sqrt(min(endpoint squared distances))-radius. This is an endpoint minimum,
// not the distance to the segment interior, and may be negative. Empty/no
// ordered improvement returns FLT_MAX. Inputs and links remain unchanged.
//
// Original ECX=&head, stack center,radius,normal_a,normal_b,unused,unused;
// ST0 result, RET18h. The last two arguments are never read. This new typed
// interface omits them and binds the already recovered ST0 sqrt explicitly.
// Native x87 spills, unordered branches and input reload order are retained.
// No independent CRT-library, binary replacement or gameplay parity claim.
float avoid_zone_selected_segments_clearance_00415d70(
    const AvoidZoneSelectedSegment* head,
    const std::array<float, 2>& center, float radius,
    const std::array<float, 2>& normal_a,
    const std::array<float, 2>& normal_b, const CameraAxesCrtAccess& crt);
} // namespace bsp
