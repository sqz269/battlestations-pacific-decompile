#include "bsp/directional_light_reference.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
bool matches_profile(const volatile std::uint32_t* table,
    std::uint32_t deleting, std::uint32_t predicate,
    std::uint32_t attach, std::uint32_t remove) noexcept {
    return table && table[0] == 0x00bd30e0u && table[1] == deleting &&
        table[0x0c / 4] == predicate && table[0x18 / 4] == 0x00b6f310u &&
        table[0x34 / 4] == 0x00b6e870u && table[0x40 / 4] == 0x00b6dbe0u &&
        table[0x50 / 4] == attach && table[0x54 / 4] == remove;
}
bool valid_profiles(DirectionalLightReferenceProfiles profiles) noexcept {
    return matches_profile(profiles.directional_00d62fb0, 0x00b7c820u, 0x00b7c6d0u,
            0x00b7c020u, 0x00b7bd60u) &&
        matches_profile(profiles.light_00d62f58, 0x00b7c800u, 0x00b7c580u,
            0x00b7c020u, 0x00b7bd60u) &&
        matches_profile(profiles.node_00d62c88, 0x00b6f8d0u, 0x00b6f570u,
            0x00b6ed80u, 0x00b6ee10u);
}
void retire_allocated_companions(void*, DirectionalLightReference& reference) noexcept {
    auto* owner = &reference.light_owner();
    delete &reference;
    delete owner;
}
// Only host setup can reach this rollback: the fresh owner has never been
// exposed, attached, retained or inserted into any hierarchy. Native constructor
// failures clean their own partial prefix before returning to the caller.
void discard_unbound_fresh_storage(NativeLightStorageView storage,
    SizedStoragePool& strings) noexcept {
    PooledStringStorage pooled(strings);
    destroy_native_string_header_0041dd20(&storage.node.name_54, pooled);
    storage.light.~NativeLightTailStorage();
    storage.node.~NativeNodeStorage();
}
}

DirectionalLightReference::DirectionalLightReference(DirectionalLightOwner& owner,
    DirectionalLightReferenceProfiles profiles, DirectionalLightCompanionDisposal disposal)
    : RenderCommandReference(owner.node.storage.references_04), owner_(owner),
      runtime_(owner.runtime.attachments), profiles_(profiles), disposal_(disposal) {
    if (!valid_profiles(profiles) || !disposal.retire ||
        owner.node.storage.vtable_00 != 0x00d62fb0u ||
        reference_count.load(std::memory_order_relaxed) <= 0 ||
        &owner.runtime.scenes != &runtime_.scenes ||
        &owner.runtime.scenes.resolve(owner.node.transform) != &owner.node.scene_attachment)
        throw std::invalid_argument("directional reference requires actual profiles, a live owner, one scene binding and retirement");
    runtime_.bind(*this);
}
DirectionalLightReference::~DirectionalLightReference() {
    if (phase_ != Phase::retired) std::terminate();
}
const volatile std::uint32_t* DirectionalLightReference::current_table() const noexcept {
    if (phase_ == Phase::retired) std::terminate();
    switch (owner_.node.storage.vtable_00) {
    case 0x00d62fb0u: return profiles_.directional_00d62fb0;
    case 0x00d62f58u:
        if (phase_ == Phase::destroying) return profiles_.light_00d62f58;
        break;
    case 0x00d62c88u:
        if (phase_ == Phase::destroying) return profiles_.node_00d62c88;
        break;
    default: break;
    }
    std::terminate();
}
std::uint32_t DirectionalLightReference::light_count(void* context) noexcept {
    const auto& array = static_cast<DirectionalLightReference*>(context)->owner_.node.storage.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void DirectionalLightReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<DirectionalLightReference*>(context);
    auto& node = reference.owner_.node.storage;
    auto& light = reference.owner_.runtime.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void DirectionalLightReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<DirectionalLightReference*>(context)->owner_.node.storage.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void DirectionalLightReference::release_model_virtual18_00b6f310() noexcept {
    if (current_table()[0x18 / 4] != 0x00b6f310u) std::terminate();
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.node.storage.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
    // A terminal self release may have destroyed both host companions.
}
void DirectionalLightReference::remove_scene_virtual54(SceneResource* expected, bool recurse) noexcept {
    const auto target = current_table()[0x54 / 4];
    auto& binding = owner_.node.scene_attachment;
    if (target == 0x00b7bd60u) {
        if (binding.remove_scene != dispatch_light_scene_remove_00b7bd60 ||
            binding.context != &owner_.retained_scenes) std::terminate();
        remove_light_scene_00b7bd60(owner_.runtime.scenes, owner_.retained_scenes, expected, recurse);
    } else if (target == 0x00b6ee10u) {
        if (binding.remove_scene != remove_native_node_scene_00b6ee10) std::terminate();
        remove_native_node_scene_00b6ee10(owner_.runtime.scenes, binding, expected, recurse);
    } else {
        std::terminate();
    }
}
void DirectionalLightReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.node.storage.vtable_00 != 0x00d62fb0u)
        std::terminate();
    const auto* table = current_table();
    if (table[0] != 0x00bd30e0u || table[1] != 0x00b7c820u) std::terminate();
    // BD30E0 supplies deleting flag1. It adds no decrement or byte44 gate.
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_directional_light_00b7c820(owner_, 1);
    // The slot is now dead. This unbind compares host interface identity only.
    runtime.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}

DirectionalLightReference* allocate_native_directional_light(
    DirectionalLightReferenceEnvironment& environment, const NativeString& name) {
    if (!valid_profiles(environment.profiles) || !environment.directional_virtual_0c ||
        !environment.light_virtual_0c ||
        &environment.nodes.scenes != &environment.nodes.attachments.scenes)
        throw std::invalid_argument("directional allocation requires actual pool, phase predicates and shared runtimes");
    void* raw = environment.pool_01090154.allocate_raw_slot_00b7bac0();
    if (!raw) throw std::bad_alloc();
    DirectionalLightOwner* owner{};
    bool constructed = false;
    bool scene_bound = false;
    try {
        auto storage = construct_native_directional_light_00b7c6b0(raw,
            DirectionalLightPool::slot_bytes, name, environment.nodes.strings);
        constructed = true;
        owner = new DirectionalLightOwner(storage, environment.pool_01090154,
            environment.nodes, environment.directional_virtual_0c,
            environment.light_virtual_0c, environment.shadow_owners);
        environment.nodes.scenes.bind(owner->node.scene_attachment);
        scene_bound = true;
        return new DirectionalLightReference(*owner, environment.profiles,
            {nullptr, retire_allocated_companions});
    } catch (...) {
        if (scene_bound) {
            delete_native_directional_light_00b7c820(*owner, 0);
        } else if (constructed) {
            discard_unbound_fresh_storage({*static_cast<NativeNodeStorage*>(raw),
                *reinterpret_cast<NativeLightTailStorage*>(static_cast<std::byte*>(raw) + 0x174)},
                environment.nodes.strings);
        }
        delete owner;
        environment.pool_01090154.return_raw_slot_00b7b2f0(raw);
        throw;
    }
}
}
