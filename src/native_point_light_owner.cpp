#include "bsp/native_point_light_owner.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native point light ownership requires MSVC Win32 widths.
#endif

namespace bsp {
static_assert(sizeof(NativePointLightTypeDescriptor) == 20);
static_assert(sizeof(NativePointLightTailStorage) == 0x78);
static_assert(offsetof(NativePointLightTailStorage, shadow_174) == 0);
static_assert(offsetof(NativePointLightTailStorage, scenes_178) == 4);
static_assert(offsetof(NativePointLightTailStorage, diffuse_184) == 0x10);
static_assert(offsetof(NativePointLightTailStorage, specular_194) == 0x20);
static_assert(offsetof(NativePointLightTailStorage, base_diffuse_1a4) == 0x30);
static_assert(offsetof(NativePointLightTailStorage, diffuse_mode3_1b4) == 0x40);
static_assert(offsetof(NativePointLightTailStorage, base_specular_1c4) == 0x50);
static_assert(offsetof(NativePointLightTailStorage, scalar_1d4) == 0x60);
static_assert(offsetof(NativePointLightTailStorage, diffuse_scale_1d8) == 0x64);
static_assert(offsetof(NativePointLightTailStorage, specular_scale_1dc) == 0x68);
static_assert(offsetof(NativePointLightTailStorage, backlinks_1e0) == 0x6c);

NativePointLightTypes::NativePointLightTypes(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& shared, NativePointLightTypeStorage storage) noexcept
    : counter_(counter), shared_(shared), storage_(storage) {}
std::uint32_t NativePointLightTypes::consume_id() {
    volatile auto* counter = counter_.get_006fac20();
    const auto id = counter->next_id_04;
    counter->next_id_04 = id + 1u;
    return id;
}
void NativePointLightTypes::initialize_00cd81a0() {
    if (storage_.guard_0109010f != 0) return;
    auto base = shared_.storage();
    const bool initialize_light = base.light_guard_0109010d == 0;
    storage_.guard_0109010f = 1;
    storage_.point_010901b0.native_name_address = 0x00d62f30u;
    // Native captures the light guard before publishing point guard/name.
    // Do not call the standalone initializer and re-read that captured guard.
    if (initialize_light) {
        base.light_guard_0109010d = 1;
        base.light_0109018c.native_name_address = 0x00d62f14u;
        shared_.initialize_node_00b6f110(base.node_0108ff90);
        const auto node = base.node_0108ff90.own_id;
        const auto root = base.node_0108ff90.root_id;
        base.light_0109018c.node_id = node;
        base.light_0109018c.root_id = root;
        base.light_0109018c.own_id = consume_id();
    }
    const auto light = base.light_0109018c.own_id;
    const auto node = base.light_0109018c.node_id;
    const auto root = base.light_0109018c.root_id;
    storage_.point_010901b0.light_id = light;
    storage_.point_010901b0.node_id = node;
    storage_.point_010901b0.root_id = root;
    storage_.point_010901b0.own_id = consume_id();
}
bool NativePointLightTypes::is_type_00b7c740(std::uint32_t token) const noexcept {
    return storage_.point_010901b0.own_id == token || storage_.point_010901b0.light_id == token ||
        storage_.point_010901b0.node_id == token || storage_.point_010901b0.root_id == token;
}

namespace {
bool matches_profile(const volatile std::uint32_t* table, std::uint32_t deleting,
    std::uint32_t predicate, std::uint32_t attach, std::uint32_t remove) noexcept {
    return table && table[0] == 0x00bd30e0u && table[1] == deleting &&
        table[0x0c / 4] == predicate && table[0x18 / 4] == 0x00b6f310u &&
        table[0x34 / 4] == 0x00b6e870u && table[0x40 / 4] == 0x00b6dbe0u &&
        table[0x50 / 4] == attach && table[0x54 / 4] == remove;
}
bool valid_profiles(NativePointLightProfiles profiles) noexcept {
    return matches_profile(profiles.point_00d63008, 0x00b7c850u, 0x00b7c740u,
            0x00b7c020u, 0x00b7bd60u) &&
        matches_profile(profiles.light_00d62f58, 0x00b7c800u, 0x00b7c580u,
            0x00b7c020u, 0x00b7bd60u) &&
        matches_profile(profiles.node_00d62c88, 0x00b6f8d0u, 0x00b6f570u,
            0x00b6ed80u, 0x00b6ee10u);
}
void require_environment(const NativePointLightEnvironment& environment) {
    if (!valid_profiles(environment.profiles) || !environment.point_virtual_0c ||
        !environment.light_virtual_0c ||
        &environment.nodes.scenes != &environment.nodes.attachments.scenes)
        throw std::invalid_argument("point light requires actual profiles, predicates and shared runtimes");
}
void end_point_tail(void* tail) noexcept {
    static_cast<NativePointLightTailStorage*>(tail)->~NativePointLightTailStorage();
}
void retire_allocated_companions(void*, NativePointLightReference& reference) noexcept {
    auto* owner = &reference.light_owner();
    delete &reference;
    delete owner;
}
void discard_unbound_fresh_storage(NativePointLightStorageView storage,
    SizedStoragePool& strings) noexcept {
    PooledStringStorage pooled(strings);
    destroy_native_string_header_0041dd20(&storage.node.name_54, pooled);
    storage.light.~NativePointLightTailStorage();
    storage.node.~NativeNodeStorage();
}
} // namespace

NativePointLightStorageView construct_native_point_light_00b7c710(void* raw,
    std::size_t bytes, const NativeString& name, SizedStoragePool& strings) {
    if (!raw || bytes < NativePointLightPool::slot_bytes)
        throw std::invalid_argument("point construction requires its actual200h pool slot");
    auto base = construct_native_light_00b7c4c0(raw, bytes, name, strings);
    // Temporary construction values only. No persistent duplicate fields and
    // no direction reference survives replacement by the point-light tail.
    const NativeLightTailStorage values = base.light;
    base.light.~NativeLightTailStorage();
    auto& tail = *::new (static_cast<std::byte*>(raw) + 0x174) NativePointLightTailStorage{
        values.shadow_174, values.scenes_178, values.diffuse_184, values.specular_194,
        values.base_diffuse_1a4, values.diffuse_mode3_1b4, values.base_specular_1c4,
        values.scalar_1d4, values.diffuse_scale_1d8, values.specular_scale_1dc,
        {nullptr, 0, 0}};
    base.node.vtable_00 = 0x00d63008u;
    initialize_native_point_light_backlinks_00b7c710_fragment(raw, bytes);
    return {base.node, tail};
}
NativePointLightOwner::NativePointLightOwner(NativePointLightStorageView storage,
    NativePointLightEnvironment& env)
    : environment(env), light(storage.light),
      node(storage.node, env.point_virtual_0c, dispatch_light_scene_attach_00b7c020, nullptr),
      retained_scenes{node.scene_attachment, light.scenes_178},
      backlinks(&storage.node, NativePointLightPool::slot_bytes, light.backlinks_1e0) {
    require_environment(env);
    if (node.storage.vtable_00 != 0x00d63008u ||
        reinterpret_cast<std::byte*>(&light) != reinterpret_cast<std::byte*>(&node.storage) + 0x174)
        throw std::invalid_argument("point companion requires the same freshly constructed native slot");
    node.scene_attachment.context = &retained_scenes;
    node.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    node.scene_attachment.remove_scene = dispatch_light_scene_remove_00b7bd60;
}
void destroy_native_point_light_00b7c770(NativePointLightOwner& owner) {
    auto& nodes = owner.environment.nodes;
    require_environment(owner.environment);
    if (owner.phase != NativePointLightOwner::Phase::live ||
        owner.node.storage.vtable_00 != 0x00d63008u ||
        &nodes.scenes.resolve(owner.node.transform) != &owner.node.scene_attachment ||
        &nodes.point_lights.light(owner.backlinks.identity) != &owner.backlinks)
        throw std::logic_error("point destruction requires its live canonical bindings");
    nodes.point_lights.require_owned_backing(owner.backlinks.backlinks.begin,
        owner.backlinks.backlinks.capacity);
    owner.phase = NativePointLightOwner::Phase::destroying;
    owner.node.storage.vtable_00 = 0x00d63008u;
    owner.node.scene_attachment.is_type = owner.environment.point_virtual_0c;
    // The supported valid-descriptor unlink/resize/free phase is nonthrowing.
    // Descriptor association ends before the Light phase and raw tail lifetime.
    destroy_native_point_light_backlinks_00b7c770_fragment(nodes.point_lights, owner.backlinks);
    nodes.point_lights.unbind_light(owner.backlinks);
    try {
        destroy_native_light_00b7c5b0({nodes, owner.node, owner.light.shadow_174,
            owner.light.scenes_178, owner.retained_scenes,
            owner.environment.light_virtual_0c, &owner.light, end_point_tail});
    } catch (...) {
        owner.phase = NativePointLightOwner::Phase::dead;
        throw; // base completed its own cleanup; native wrapper returns no slot
    }
    owner.phase = NativePointLightOwner::Phase::dead;
}
void* delete_native_point_light_00b7c850(NativePointLightOwner& owner, std::uint32_t flags) {
    void* raw = &owner.node.storage;
    destroy_native_point_light_00b7c770(owner);
    if (flags & 1u) owner.environment.pool_0109011c.return_raw_slot_00b7b1d0(raw);
    return raw;
}

NativePointLightReference::NativePointLightReference(NativePointLightOwner& owner,
    NativePointLightCompanionDisposal disposal)
    : RenderCommandReference(owner.node.storage.references_04), owner_(owner),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    require_environment(owner.environment);
    if (!disposal.retire || owner.phase != NativePointLightOwner::Phase::live ||
        owner.node.storage.vtable_00 != 0x00d63008u ||
        reference_count.load(std::memory_order_relaxed) <= 0 ||
        &owner.environment.nodes.scenes.resolve(owner.node.transform) != &owner.node.scene_attachment ||
        &owner.environment.nodes.point_lights.light(owner.backlinks.identity) != &owner.backlinks)
        throw std::invalid_argument("point reference requires its live actual owner and canonical bindings");
    runtime_.bind(*this);
}
NativePointLightReference::~NativePointLightReference() {
    if (phase_ != Phase::retired) std::terminate();
}
const volatile std::uint32_t* NativePointLightReference::current_table() const noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativePointLightOwner::Phase::dead)
        std::terminate();
    const auto profiles = owner_.environment.profiles;
    switch (owner_.node.storage.vtable_00) {
    case 0x00d63008u: return profiles.point_00d63008;
    case 0x00d62f58u:
        if (phase_ == Phase::destroying) return profiles.light_00d62f58;
        break;
    case 0x00d62c88u:
        if (phase_ == Phase::destroying) return profiles.node_00d62c88;
        break;
    default: break;
    }
    std::terminate();
}
std::uint32_t NativePointLightReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativePointLightReference*>(context)->owner_.node.storage.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void NativePointLightReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<NativePointLightReference*>(context);
    auto& node = reference.owner_.node.storage;
    auto& light = reference.owner_.environment.nodes.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void NativePointLightReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativePointLightReference*>(context)->owner_.node.storage.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void NativePointLightReference::release_model_virtual18_00b6f310() noexcept {
    if (current_table()[0x18 / 4] != 0x00b6f310u) std::terminate();
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.node.storage.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
}
void NativePointLightReference::remove_scene_virtual54(SceneResource* expected, bool recurse) noexcept {
    const auto target = current_table()[0x54 / 4];
    auto& binding = owner_.node.scene_attachment;
    if (target == 0x00b7bd60u) {
        if (binding.remove_scene != dispatch_light_scene_remove_00b7bd60 ||
            binding.context != &owner_.retained_scenes) std::terminate();
        remove_light_scene_00b7bd60(owner_.environment.nodes.scenes,
            owner_.retained_scenes, expected, recurse);
    } else if (target == 0x00b6ee10u) {
        if (binding.remove_scene != remove_native_node_scene_00b6ee10) std::terminate();
        remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes, binding, expected, recurse);
    } else {
        std::terminate();
    }
}
void NativePointLightReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.node.storage.vtable_00 != 0x00d63008u)
        std::terminate();
    const auto* table = current_table();
    if (table[0] != 0x00bd30e0u || table[1] != 0x00b7c850u) std::terminate();
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_point_light_00b7c850(owner_, 1);
    runtime.unbind(*this); // host identity only; actual slot is dead
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
NativePointLightReference* allocate_native_point_light(NativePointLightEnvironment& environment,
    const NativeString& name) {
    require_environment(environment);
    void* raw = environment.pool_0109011c.allocate_raw_slot_00b7b810();
    if (!raw) throw std::bad_alloc();
    NativePointLightOwner* owner{};
    bool constructed = false, scene_bound = false, backlinks_bound = false;
    try {
        auto storage = construct_native_point_light_00b7c710(raw,
            NativePointLightPool::slot_bytes, name, environment.nodes.strings);
        constructed = true;
        owner = new NativePointLightOwner(storage, environment);
        environment.nodes.scenes.bind(owner->node.scene_attachment);
        scene_bound = true;
        environment.nodes.point_lights.bind_light(owner->backlinks);
        backlinks_bound = true;
        return new NativePointLightReference(*owner, {nullptr, retire_allocated_companions});
    } catch (...) {
        if (backlinks_bound) {
            delete_native_point_light_00b7c850(*owner, 0);
        } else if (constructed) {
            if (scene_bound)
                environment.nodes.scenes.forget_destroyed_binding(owner->node.scene_attachment);
            discard_unbound_fresh_storage({*static_cast<NativeNodeStorage*>(raw),
                *reinterpret_cast<NativePointLightTailStorage*>(static_cast<std::byte*>(raw) + 0x174)},
                environment.nodes.strings);
        }
        delete owner;
        environment.pool_0109011c.return_raw_slot_00b7b1d0(raw);
        throw;
    }
}
} // namespace bsp
