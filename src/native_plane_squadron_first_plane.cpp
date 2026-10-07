#include "bsp/native_plane_squadron_first_plane.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace bsp {

static_assert(sizeof(void*) == 4, "native squadron pointer cell requires Win32");

NativePlaneSquadronFirstPlaneView native_plane_squadron_first_plane_view(
    const void* squadron, std::size_t backing_bytes,
    const void* const volatile& first_plane) {
    const auto root = reinterpret_cast<std::uintptr_t>(squadron);
    if (squadron == nullptr || backing_bytes < 0x3d4 ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x3d4 ||
        root % alignof(void*) != 0 ||
        reinterpret_cast<std::uintptr_t>(&first_plane) != root + 0x3d0) {
        throw std::logic_error("native first plane requires live SAME squadron+3D0 cell");
    }
    return {squadron, first_plane};
}

const void* native_plane_squadron_first_plane_007ed010(
    const NativePlaneSquadronFirstPlaneView& squadron) noexcept {
    return squadron.first_plane_3d0; // ONE actual pointer observation; no count.
}

} // namespace bsp
