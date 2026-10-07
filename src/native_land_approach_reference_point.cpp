#include "bsp/native_land_approach_reference_point.hpp"
#include "bsp/point_effect_owner.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace bsp {

NativeLandApproachReferencePointView native_land_approach_reference_point_view(
    const void* actual_root, const std::size_t bytes,
    const std::array<float, 3>& actual_xyz) {
    static_assert(sizeof(actual_xyz) == 12);
    const auto root = reinterpret_cast<std::uintptr_t>(actual_root);
    if (!actual_root || bytes < 0x44 ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x44 ||
        reinterpret_cast<std::uintptr_t>(&actual_xyz) != root + 0x38 ||
        reinterpret_cast<std::uintptr_t>(actual_xyz.data()) != root + 0x38)
        throw std::logic_error("native approach reference requires live ROOT+38 array");
    return {actual_root, actual_xyz};
}

float* copy_native_land_approach_reference_point_009afad0(
    const NativeLandApproachReferencePointView& approach, float* actual_output) {
    copy_point_record_xyz_0049c1db(actual_output, approach.xyz_38);
    return actual_output;
}

} // namespace bsp
