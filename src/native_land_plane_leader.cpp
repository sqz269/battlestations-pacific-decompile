#include "bsp/native_land_plane_leader.hpp"

#include <limits>
#include <stdexcept>

namespace bsp {

NativeLandPlaneLeaderView native_land_plane_leader_view(const void* plane,
    std::size_t backing_bytes, const volatile std::int32_t& field) {
    const auto root = reinterpret_cast<std::uintptr_t>(plane);
    const auto cell = reinterpret_cast<std::uintptr_t>(&field);
    if (plane == nullptr || backing_bytes < 0x9dc ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x9dc ||
        root % alignof(std::int32_t) != 0 || cell != root + 0x9d8) {
        throw std::logic_error("native leader requires live SAME plane+9D8 cell");
    }
    return {plane, field};
}

bool native_land_plane_is_leader_007b8ad0(const NativeLandPlaneLeaderView& plane) noexcept {
    return plane.field_9d8 == 0; // 007B8AD2: ONE actual DWORD observation.
}

namespace {
bool probe_actual_plane(const void* plane, NativeLandPlaneLeaderViews& views) {
    const auto view = views.plane_leader_view(plane); // required PURE alias mapping
    if (view.actual_plane != plane) {
        throw std::logic_error("native leader mapping changed plane identity");
    }
    return native_land_plane_is_leader_007b8ad0(view);
}
} // namespace

bool NativeLandTaskPlaneLeaderConstructorCalls::plane_is_leader_007b8ad0(const void* plane) {
    return probe_actual_plane(plane, *this);
}

bool NativeLandPlaneLeaderCruiseHost::plane_is_leader_007b8ad0(const void* plane) {
    return probe_actual_plane(plane, *this);
}

} // namespace bsp
