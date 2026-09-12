#include "bsp/directional_light_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native directional light ownership requires MSVC Win32 widths.
#endif

namespace bsp {
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeLightTailStorage) == 0x78);
static_assert(offsetof(NativeLightTailStorage, shadow_174) == 0);
static_assert(offsetof(NativeLightTailStorage, scenes_178) == 4);
static_assert(offsetof(NativeLightTailStorage, diffuse_184) == 0x10);
static_assert(offsetof(NativeLightTailStorage, specular_194) == 0x20);
static_assert(offsetof(NativeLightTailStorage, base_diffuse_1a4) == 0x30);
static_assert(offsetof(NativeLightTailStorage, diffuse_mode3_1b4) == 0x40);
static_assert(offsetof(NativeLightTailStorage, base_specular_1c4) == 0x50);
static_assert(offsetof(NativeLightTailStorage, scalar_1d4) == 0x60);
static_assert(offsetof(NativeLightTailStorage, diffuse_scale_1d8) == 0x64);
static_assert(offsetof(NativeLightTailStorage, specular_scale_1dc) == 0x68);
static_assert(offsetof(NativeLightTailStorage, direction_1e0) == 0x6c);

namespace {
constexpr std::uint32_t one = 0x3f800000u;
constexpr std::uint32_t sixty_four = 0x42800000u; // actual constant 00CE7820
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void enter_light_phase(NativeLightBaseOwnerView& owner) noexcept {
    owner.node.storage.vtable_00 = 0x00d62f58u;
    auto& dispatch = owner.node.scene_attachment;
    dispatch.is_type = owner.light_virtual_0c;
    dispatch.world_changed = native_node_world_changed_00b6dbe0;
    dispatch.attach_scene = dispatch_light_scene_attach_00b7c020;
    dispatch.remove_scene = dispatch_light_scene_remove_00b7bd60;
    dispatch.context = &owner.retained_scenes;
}
void destroy_scene_array(NativeLightBaseOwnerView& owner) {
    resize_system_ambient_backlinks_00b7bc70(owner.scenes_178, 0);
    singleton_lifetime_free(owner.scenes_178.begin_00);
    // Native leaves the pointer/capacity words and pointed scene state intact.
}
void finish_node_base(NativeLightBaseOwnerView& owner) {
    try {
        destroy_native_node_00b6f440(owner.runtime, owner.node);
    } catch (...) {
        // The node destructor completes its own cleanup unwind before throwing.
        owner.end_tail(owner.tail);
        owner.runtime.scenes.forget_destroyed_binding(owner.node.scene_attachment);
        throw;
    }
    owner.end_tail(owner.tail);
    owner.runtime.scenes.forget_destroyed_binding(owner.node.scene_attachment);
}
}

NativeLightStorageView construct_native_light_00b7c4c0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings) {
    auto& node = construct_native_node_00b6f5a0(actual_slot, slot_bytes, name, strings);
    auto* raw = static_cast<std::byte*>(actual_slot);
    // Read the actual allocation preimage before starting typed word lifetimes.
    // These copies preserve native-unwritten values; no default colors or
    // direction are substituted for them.
    SystemLightingWords4 specular, mode3;
    std::uint32_t specular_scale;
    SystemLightingWords3 direction;
    std::memcpy(specular.data(), raw + 0x194, sizeof(specular));
    std::memcpy(mode3.data(), raw + 0x1b4, sizeof(mode3));
    std::memcpy(&specular_scale, raw + 0x1dc, sizeof(specular_scale));
    std::memcpy(direction.data(), raw + 0x1e0, sizeof(direction));
    node.vtable_00 = 0x00d62f58u;
    auto& light = *::new (raw + 0x174) NativeLightTailStorage{
        nullptr, {nullptr, 0, 0}, {one, one, one, one}, specular,
        {one, one, one, one}, mode3, {0, 0, 0, one}, sixty_four, one,
        specular_scale, direction};
    return {node, light};
}
NativeLightStorageView construct_native_directional_light_00b7c6b0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings) {
    auto storage = construct_native_light_00b7c4c0(actual_slot, slot_bytes, name, strings);
    storage.node.vtable_00 = 0x00d62fb0u; // the entire directional-only constructor
    return storage;
}

DirectionalLightOwner::DirectionalLightOwner(NativeLightStorageView storage,
    DirectionalLightPool& actual_pool, NativeNodeDestructionRuntime& node_runtime,
    SceneTypePredicate actual_directional_virtual_0c, SceneTypePredicate actual_light_virtual_0c,
    SystemShadowOwnerResolver& shadow_owners)
    : pool(actual_pool), runtime(node_runtime), light(storage.light),
      node(storage.node, actual_directional_virtual_0c, dispatch_light_scene_attach_00b7c020, nullptr),
      retained_scenes{node.scene_attachment, light.scenes_178},
      lighting(light.shadow_174, shadow_owners, light.diffuse_184, light.specular_194,
          light.diffuse_mode3_1b4, light.direction_1e0),
      directional_virtual_0c(actual_directional_virtual_0c), light_virtual_0c(actual_light_virtual_0c) {
    if (!light_virtual_0c || node.storage.vtable_00 != 0x00d62fb0u ||
        reinterpret_cast<std::byte*>(&light) !=
            reinterpret_cast<std::byte*>(&node.storage) + 0x174)
        throw std::invalid_argument("directional owner requires one actual light slot and actual phase predicates");
    node.scene_attachment.context = &retained_scenes;
    node.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    node.scene_attachment.remove_scene = dispatch_light_scene_remove_00b7bd60;
    node.scene_attachment.system_directional_light = &lighting;
}

void destroy_native_light_00b7c5b0(DirectionalLightOwner& owner) {
    destroy_native_light_00b7c5b0({owner.runtime, owner.node, owner.light.shadow_174,
        owner.light.scenes_178, owner.retained_scenes, owner.light_virtual_0c,
        &owner.light, [](void* tail) noexcept {
            static_cast<NativeLightTailStorage*>(tail)->~NativeLightTailStorage();
        }});
}
void destroy_native_light_00b7c5b0(NativeLightBaseOwnerView owner) {
    // Validate the supplied companion before entering native destruction/unwind.
    if (!owner.light_virtual_0c || !owner.tail || !owner.end_tail ||
        &owner.runtime.scenes.resolve(owner.node.transform) != &owner.node.scene_attachment)
        throw std::logic_error("light destruction requires its existing scene dispatch binding");
    enter_light_phase(owner);
    auto& array = owner.scenes_178;
    try {
        while (array.count_04 > 0) {
            SceneResource* scene = array.begin_00[array.count_04 - 1];
            remove_scene_node_if_type_00b83ec0(owner.runtime.scenes, *scene, owner.node.scene_attachment);
            // B83EC0 may change both array allocation and logical count. Native
            // then reads the NEW back element unconditionally, before decrement.
            scene = array.begin_00[array.count_04 - 1];
            if (scene->references.fetch_sub(1, std::memory_order_seq_cst) == 1)
                scene->destroy_on_zero(*scene);
            // A terminal scene callback may also change the array and count.
            if (array.count_04 != 0)
                array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) - 1u);
        }
        if (void* shadow = owner.shadow_174) {
            owner.runtime.release_retained_owner(shadow);
            owner.shadow_174 = nullptr; // clear after the real terminal callback
        }
    } catch (...) {
        // CC1EE6 / DFAFC4 / DFAFB4: state1 array B7C1C0, state0 node B6F440.
        // There is no invented shadow release or scene drain during this unwind.
        try {
            destroy_scene_array(owner);
            finish_node_base(owner);
        } catch (...) {
            std::terminate(); // second exception during native destructor unwind
        }
        throw;
    }
    // Valid array capacity is nonnegative: resize(0) cannot allocate or throw.
    destroy_scene_array(owner);
    finish_node_base(owner); // returning tail absent from the truncated pseudocode
}
void* delete_native_directional_light_00b7c820(DirectionalLightOwner& owner,
    std::uint32_t flags) {
    void* actual_slot = &owner.node.storage;
    owner.node.storage.vtable_00 = 0x00d62fb0u;
    owner.node.scene_attachment.is_type = owner.directional_virtual_0c;
    destroy_native_light_00b7c5b0(owner);
    if (flags & 1u) owner.pool.return_raw_slot_00b7b2f0(actual_slot);
    return actual_slot;
}

} // namespace bsp
