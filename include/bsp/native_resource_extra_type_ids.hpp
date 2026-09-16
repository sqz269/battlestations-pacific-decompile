#pragma once

#include "bsp/native_mesh_subset_loading.hpp"

#include <cstdint>

namespace bsp {

// Process-owned descriptors used by the animation and bone resource profiles.
// Each descriptor is exactly [own,scene,root,name]. Binding this view does not
// reset guards or repair a descriptor left partial by an earlier exception.
struct NativeResourceExtraTypeIdStorage {
    volatile std::uint8_t& animation_guard_01090264;
    volatile std::uint32_t* animation_01090268; // exactly4 words
    volatile std::uint8_t& bone_guard_01090265;
    volatile std::uint32_t* bone_01090278; // exactly4 words
};

class NativeResourceExtraTypeIds final {
public:
    // SAME process counter and scene/root domain as the existing mesh type
    // service. Neither constructor nor storage() performs initialization.
    NativeResourceExtraTypeIds(TypeIdCounterLifetime&, NativeMeshResourceTypeIds&,
        NativeResourceExtraTypeIdStorage) noexcept;

    // Full native CRT entries, 79 bytes each, no ordinary args, RET. The guard
    // and name precede the scene call; parent IDs are captured before stores;
    // the current counter is advanced before the own-ID store.
    void initialize_animation_resource_00cd82f0();
    void initialize_bone_resource_00cd8340();

    NativeResourceExtraTypeIdStorage storage() const noexcept { return storage_; }

private:
    void initialize(volatile std::uint8_t&, volatile std::uint32_t*, std::uint32_t);

    TypeIdCounterLifetime& counter_;
    NativeMeshResourceTypeIds& scene_;
    NativeResourceExtraTypeIdStorage storage_;
};

} // namespace bsp
