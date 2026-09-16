#include "bsp/native_resource_extra_type_ids.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
}

NativeResourceExtraTypeIds::NativeResourceExtraTypeIds(TypeIdCounterLifetime& counter,
    NativeMeshResourceTypeIds& scene, NativeResourceExtraTypeIdStorage storage) noexcept
    : counter_(counter), scene_(scene), storage_(storage) {}

void NativeResourceExtraTypeIds::initialize(volatile std::uint8_t& guard,
    volatile U* descriptor, U name) {
    if (guard != 0) return;
    guard = 1;
    descriptor[3] = name;
    auto* const parent = scene_.storage().scene_01090210;
    scene_.initialize_scene_resource_00b869c0(parent);
    const U scene = parent[0];
    const U root = parent[1];
    descriptor[1] = scene;
    descriptor[2] = root;
    volatile auto* const counter = counter_.get_006fac20();
    const U id = counter->next_id_04;
    counter->next_id_04 = id + 1u;
    descriptor[0] = id;
}

void NativeResourceExtraTypeIds::initialize_animation_resource_00cd82f0() {
    initialize(storage_.animation_guard_01090264, storage_.animation_01090268, 0x00d63258);
}

void NativeResourceExtraTypeIds::initialize_bone_resource_00cd8340() {
    initialize(storage_.bone_guard_01090265, storage_.bone_01090278, 0x00d6326c);
}

} // namespace bsp
