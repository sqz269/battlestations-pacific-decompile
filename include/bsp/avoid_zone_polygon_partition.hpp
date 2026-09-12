#pragma once

#include "bsp/avoid_zone_draft_layers.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Ordered output of004F6F20. Semantic C++ storage, not the native68h object
// or its MSVC vector/SEH/allocator ABI. Indices refer to the borrowed input;
// winding, repeated coordinates and native piece order are preserved.
struct AvoidZonePolygonPartition {
    std::vector<std::vector<std::uint32_t>> pieces; // native+44h vector
    std::vector<std::array<std::uint32_t, 2>> cuts; // native+54h vector
    std::int32_t remaining_count{}; // native+00h, may remain>=3 on seed failure
};

// Complete valid-container algorithm of004F6F20..004F7173 and its geometric
// stages. Empty input is native-invalid (004F65F0 indexes previous[0]); this
// interface throws rather than entering the CRT invalid-parameter path.
// One/two points yield no pieces. Native seed failure preserves prior pieces
// and returns remaining_count>=3; no fallback triangulation is performed.
// `angular_tolerance` is the actual constructor argument, not a merge radius.
// The caller must bind the existing LegacyCrtMathRuntime for0042CF10 and
// supply the same process-owned sqrt CRT access as other geometry kernels.
AvoidZonePolygonPartition avoid_zone_polygon_partition_004f6f20(
    const std::vector<AvoidZoneDraftPoint>& points, float angular_tolerance,
    const CameraAxesCrtAccess& crt);
}
