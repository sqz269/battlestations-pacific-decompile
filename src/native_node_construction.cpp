#include "bsp/native_node_construction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native node construction requires MSVC Win32 field and pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);
static_assert(sizeof(NativeNodePointLightArray) == 12);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(offsetof(NativeNodeStorage, references_04) == 4);
static_assert(offsetof(NativeNodeStorage, untouched_08) == 8);
static_assert(offsetof(NativeNodeStorage, parent_30) == 0x30);
static_assert(offsetof(NativeNodeStorage, released_44) == 0x44);
static_assert(offsetof(NativeNodeStorage, name_54) == 0x54);
static_assert(offsetof(NativeNodeStorage, valid_flags_5c) == 0x5c);
static_assert(offsetof(NativeNodeStorage, view_60) == 0x60);
static_assert(offsetof(NativeNodeStorage, notification_context_a0) == 0xa0);
static_assert(offsetof(NativeNodeStorage, root_list_a4) == 0xa4);
static_assert(offsetof(NativeNodeStorage, local_b0) == 0xb0);
static_assert(offsetof(NativeNodeStorage, world_f0) == 0xf0);
static_assert(offsetof(NativeNodeStorage, retained_130) == 0x130);
static_assert(offsetof(NativeNodeStorage, enabled_134) == 0x134);
static_assert(offsetof(NativeNodeStorage, auxiliary_flags_138) == 0x138);
static_assert(offsetof(NativeNodeStorage, world_sphere_13c) == 0x13c);
static_assert(offsetof(NativeNodeStorage, bounds_min_14c) == 0x14c);
static_assert(offsetof(NativeNodeStorage, bounds_max_158) == 0x158);
static_assert(offsetof(NativeNodeStorage, point_lights_164) == 0x164);
static_assert(offsetof(NativeNodeStorage, scene_170) == 0x170);

namespace {
constexpr std::uint32_t one_word = 0x3f800000u; // native 00D7A24C
constexpr std::uint32_t positive_bound_word = 0x501502f9u; // native 00CE4970
constexpr std::uint32_t negative_bound_word = 0xd01502f9u; // native 00CE4ADC

void store_word(float& destination, std::uint32_t word) noexcept {
    std::memcpy(&destination, &word, sizeof(word));
}
void initialize_identity(CameraMatrix& destination) {
    // The native constructs a fresh stack matrix for each destination, then
    // invokes the recovered sequential x87 copy. Destination order is B0,F0,60.
    const CameraMatrix identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    copy_camera_matrix_004134f0(destination, identity);
}
CameraTransformBacking transform_backing(NativeNodeStorage& node) noexcept {
    return {node.parent_30, node.first_child_34, node.child_count_38,
        node.next_sibling_3c, node.previous_sibling_40, node.root_list_a4,
        node.valid_flags_5c, node.auxiliary_flags_138, node.notification_context_a0,
        node.view_60, node.local_b0, node.world_f0};
}
}

NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings) {
    if (!actual_slot || slot_bytes < sizeof(NativeNodeStorage) ||
        (reinterpret_cast<std::uintptr_t>(actual_slot) % alignof(NativeNodeStorage)) != 0)
        throw std::invalid_argument("native node construction requires an aligned actual 0x174-byte prefix");

    // No parentheses: value-initialization would overwrite the slot preimage.
    auto& node = *::new (actual_slot) NativeNodeStorage;
    node.vtable_00 = 0x00ceb130u;
    node.references_04.store(1, std::memory_order_relaxed);
    node.vtable_00 = 0x00d62c88u;
    node.parent_30 = nullptr;
    node.first_child_34 = nullptr;
    node.child_count_38 = 0;
    node.next_sibling_3c = nullptr;
    node.previous_sibling_40 = nullptr;
    node.mask_48 = 0;
    node.bounds_scalar_50 = positive_bound_word;
    node.notification_context_a0 = nullptr;
    node.root_list_a4 = nullptr;
    node.field_a8 = 0;
    node.retained_130 = nullptr;
    node.point_lights_164 = {nullptr, 0, 0};
    node.scene_170 = nullptr;
    try {
        PooledStringStorage pool(strings);
        node.name_54.copy_from_00be0a30_fragment(pool, name);
        node.scalar_4c = one_word;
        node.scalar_ac = one_word;
        node.valid_flags_5c = 0;
        initialize_identity(node.local_b0);
        initialize_identity(node.world_f0);
        initialize_identity(node.view_60);
        node.auxiliary_flags_138 = 0x40;
        store_word(node.world_sphere_13c[0], 0);
        store_word(node.world_sphere_13c[1], 0);
        store_word(node.world_sphere_13c[2], 0);
        store_word(node.world_sphere_13c[3], positive_bound_word);
        for (auto& value : node.bounds_min_14c) store_word(value, negative_bound_word);
        for (auto& value : node.bounds_max_158) store_word(value, positive_bound_word);
        node.released_44 = 0;
        node.enabled_134 = 1;
        node.mask_48 = 0x000fffffu;
    } catch (...) {
        // Native CC1A31 unwind states 2,1,0: array resize0/free, name pool
        // release, then AA6E10 ->BD30F0. No physical slot return belongs here.
        while (node.point_lights_164.count > 0) --node.point_lights_164.count;
        node.point_lights_164.count = 0;
        singleton_lifetime_free(node.point_lights_164.begin);
        if (node.name_54.data())
            strings.release_00bd1510(node.name_54.data(), node.name_54.length() + 1u);
        node.vtable_00 = 0x00d5c104u;
        node.vtable_00 = 0x00ceb130u;
        node.~NativeNodeStorage();
        throw;
    }
    return node;
}

NativeNodeBinding::NativeNodeBinding(NativeNodeStorage& node,
    SceneTypePredicate actual_virtual_0c, SceneAttachOverride actual_virtual_50,
    void (*actual_notify_changed)(void*), void* dispatch_context)
    : storage(node), transform(transform_backing(node), actual_notify_changed),
      scene_attachment(transform, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&node)),
          node.scene_170, actual_virtual_0c, actual_virtual_50, dispatch_context) {
    if ((node.notification_context_a0 != nullptr) != (actual_notify_changed != nullptr))
        throw std::invalid_argument("native node attachment and notification callback must be bound together");
    // Fresh construction has A0=null, so its caller explicitly passes null.
}
NativeNodeBinding::NativeNodeBinding(NativeNodeStorage& node, NativeNodePreconstructionBinding,
    SceneTypePredicate actual_virtual_0c, SceneAttachOverride actual_virtual_50,
    void* dispatch_context)
    : storage(node), transform(transform_backing(node), nullptr),
      scene_attachment(transform, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&node)),
          node.scene_170, actual_virtual_0c, actual_virtual_50, dispatch_context) {}

} // namespace bsp
