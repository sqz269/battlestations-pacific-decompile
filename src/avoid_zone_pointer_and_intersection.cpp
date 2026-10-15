#include "bsp/avoid_zone_pointer_and_intersection.hpp"

#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_copy_lifetime.hpp"

#include <array>
#include <cstring>

namespace bsp {
void avoid_zone_owner_destroy_0041c490(AvoidZoneNativeStorage*& owner,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    auto* const zone = owner; // 0041C494; capture before destruction/free calls.
    if (zone) {
        // Exact semantic reuse of 0041C49C..0041C4B0: counted records clear,
        // then nonnull buffer free/pointer=0; capacity and metadata preserved.
        avoid_zone_destroy_owned_storage_0041a1e0(*zone, allocation);
        allocation.free_record_00bf65ac(allocation.context, zone); // 0041C4B7: OBJECT.
        owner = nullptr; // 0041C4BF, only after object deallocation returns.
    }
}

Cc13AvoidZoneOptionalVector& segment_intersection_optional_point_0041a4e0(
    Cc13AvoidZoneOptionalVector& output, const float* a0, const float* a1,
    const float* b0, const float* b1) noexcept {
    std::array<float, 2> point;
    if (native_segment_crossing_004f3730(a0, a1, b0, b1, point.data())) { // 0041A4FD.
        // 0041A508..0041A518 MOVSS: preserve result bits without another x87
        // float load/store or default-initializing the optional output/padding.
        std::memcpy(&output.x, &point[0], sizeof(float));
        std::memcpy(&output.z, &point[1], sizeof(float));
        output.valid = 1; // 0041A51D; after both coordinate writes.
    } else {
        output.valid = 0; // 0041A528; coordinates and padding unchanged.
    }
    return output; // Native EAX=ESI at0041A504 before hit/miss branch.
}
} // namespace bsp
