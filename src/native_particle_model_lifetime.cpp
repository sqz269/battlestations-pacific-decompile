#include "bsp/native_particle_model_lifetime.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_particle_model_manager.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle model lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
NativeParticleModelTailStorage& tail(NativeModelOwner& owner) noexcept {
    return *std::launder(reinterpret_cast<NativeParticleModelTailStorage*>(
        reinterpret_cast<std::byte*>(&owner.storage.node) + 0x184));
}
void destroy_cookie(void* data, std::uint32_t stride, bool record) noexcept {
    if (!data) return;
    auto* cookie = static_cast<std::byte*>(data) - 4;
    std::int32_t count;
    std::memcpy(&count, cookie, sizeof(count));
    // BF7C6E captures cookie/count and walks backwards. These two native
    // element destructors cannot throw; neither reads nor changes its element.
    auto* cursor = static_cast<std::byte*>(data) + static_cast<std::size_t>(count) * stride;
    for (std::int32_t remaining = count; remaining > 0; --remaining) {
        cursor -= stride;
        if (record) destroy_native_particle_temporary_00afd9f0(cursor);
        else destroy_native_particle_array_byte_00afcda0(cursor);
    }
    singleton_lifetime_free(cookie);
}
template<class T> void release_then_clear(T*& field, NativeRenderActualOwners& owners) {
    if (auto* captured = field) {
        release_native_render_actual_owner(owners, captured);
        field = nullptr;
    }
}
} // namespace

std::uint32_t native_particle_model_type_id_00af5af0(
    const volatile NativeParticleModelTypeDescriptor& types) noexcept { return types.own_id; }
std::uint32_t native_particle_model_type_name_00af5b00(
    const volatile NativeParticleModelTypeDescriptor& types) noexcept { return types.native_name_address; }
bool native_particle_model_is_type_00af6da0(
    const volatile NativeParticleModelTypeDescriptor& types, std::uint32_t token) noexcept {
    return types.own_id == token || types.model_id == token ||
        types.node_id == token || types.root_id == token;
}
void resize_native_particle_emitter_pointers_00af6180(
    NativeRenderPointerArrayStorage& array, std::int32_t count) {
    if (count > array.capacity_08) reserve_native_particle_emitter_pointers_00af6120(array, count);
    for (std::int32_t i = array.count_04; i < count; ++i) {
        const auto cell = reinterpret_cast<std::uintptr_t>(array.data_00) +
            static_cast<std::uint32_t>(i) * 4u;
        if (cell) *reinterpret_cast<void**>(cell) = nullptr;
    }
    while (count < array.count_04) --array.count_04;
    array.count_04 = count;
}
void destroy_native_particle_emitter_pointers_00af6b70(NativeRenderPointerArrayStorage& array) {
    resize_native_particle_emitter_pointers_00af6180(array, 0);
    singleton_lifetime_free(array.data_00);
}
void destroy_native_particle_array_byte_00afcda0(void*) noexcept {}
void destroy_native_particle_model_byte_array_00afd0f0(NativeParticleArrayStorage& array) noexcept {
    destroy_cookie(array.data_00, 1, false);
    array.count_04 = 0;
    array.data_00 = nullptr;
}
void destroy_native_particle_model_record_array_00afd1e0(NativeParticleArrayStorage& array) noexcept {
    destroy_cookie(array.data_00, 0x108, true);
    array.count_04 = 0;
    array.data_00 = nullptr;
}
void destroy_native_particle_model_arrays_00afd370(NativeParticleModelArraysStorage& arrays) noexcept {
    destroy_cookie(arrays.records_0c.data_00, 0x108, true);
    arrays.records_0c.data_00 = nullptr; // AFD3BC precedes count store AFD3BF.
    arrays.records_0c.count_04 = 0;
    destroy_native_particle_model_byte_array_00afd0f0(arrays.bytes_04);
}
void destroy_native_particle_model_00af6c50(
    NativeModelOwner& base, NativeParticleModelLifetimeAccess& access) {
    if (base.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("particle destruction requires the existing live model owner");
    auto& model = tail(base);
    auto& owners = base.environment.retained_owners;
    base.storage.node.vtable_00 = 0x00d5da50;
    unsigned unwind_state = 1;
    try {
        if (auto* captured = model.arrays_190) {
            destroy_native_particle_model_arrays_00afd370(*captured);
            singleton_lifetime_free(captured); // AF6C97 resumes;190 is not cleared.
        }
        for (std::int32_t i = 0; i < model.emitters_194.count_04; ++i) {
            // The emitter is required nonnull; reload current data/count after
            // each canonical current-virtual0 terminal callback.
            void* const emitter = model.emitters_194.data_00[i];
            release_native_render_actual_owner(owners, emitter);
        }
        release_then_clear(model.variant_18c, owners);
        release_then_clear(model.mesh_1c8, owners);
        release_then_clear(model.mesh_1b4, owners);
        void* const manager = access.manager_00f8c274; // AF6D3F BEFORE SUB global.
        access.live_count_00f8d2c8 = access.live_count_00f8d2c8 - 1u;
        static_cast<void>(unregister_native_particle_model_00af0ae0(manager, &base.storage.node));
        unwind_state = 0;
        destroy_native_particle_emitter_pointers_00af6b70(model.emitters_194);
    } catch (...) {
        try {
            if (unwind_state == 1) destroy_native_particle_emitter_pointers_00af6b70(model.emitters_194);
            destroy_native_model_00b750c0(base);
        } catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_model_00b750c0(base); // Native state-1; no repeated cleanup.
}
void* delete_native_particle_model_00af7f40(
    NativeModelOwner& base, NativeParticleModelLifetimeAccess& access, std::uint32_t flags) {
    void* const slot = &base.storage.node;
    destroy_native_particle_model_00af6c50(base, access);
    if (flags & 1u) return_native_particle_model_slot_00af60b0(access.actual_pool_00f8d2d0, slot);
    return slot;
}

NativeParticleModelReference::NativeParticleModelReference(NativeModelOwner& owner,
    NativeParticleModelLifetimeAccess& access, NativeParticleModelCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.node.references_04), owner_(owner), access_(access),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    if (owner.phase != NativeModelOwner::Phase::live || !access.actual_pool_00f8d2d0 ||
        !access.vtable_00d5da50 || !disposal.retire ||
        owner.storage.node.vtable_00 != 0x00d5da50 ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("particle reference requires the existing live derived model and actual bindings");
    require_slot(0, 0x00bd30e0);
    require_slot(4, 0x00af7f40);
    runtime_.bind(*this);
    auto& binding = owner.node.scene_attachment;
    binding.is_type = is_type;
    binding.attach_scene = attach_scene;
    binding.remove_scene = remove_scene;
    binding.world_changed = world_changed;
}
NativeParticleModelReference::~NativeParticleModelReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeParticleModelReference::require_slot(std::uint32_t offset,
    std::uint32_t expected, bool allow_base) const noexcept {
    const volatile std::uint32_t* table = nullptr;
    const auto profile = owner_.storage.node.vtable_00;
    if (profile == 0x00d5da50) table = access_.vtable_00d5da50;
    else if (allow_base && profile == 0x00d62de8) table = owner_.environment.vtable_00d62de8;
    else if (allow_base && profile == 0x00d62c88) table = owner_.environment.vtable_00d62c88;
    if (!table || table[offset / 4] != expected) std::terminate();
}
NativeParticleModelReference& NativeParticleModelReference::from_binding(SceneNodeAttachment& binding) {
    // Context MUST remain NativeModelOwner: B750C0 reinstalls base callbacks.
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    auto* result = dynamic_cast<NativeParticleModelReference*>(
        &owner.environment.nodes.attachments.resolve(owner.node.transform));
    if (!result) throw std::logic_error("particle profile requires its existing canonical node lifetime");
    return *result;
}
bool NativeParticleModelReference::is_type(SceneAttachmentRuntime&, SceneNodeAttachment& binding,
    std::uint32_t token) {
    auto& self = from_binding(binding);
    self.require_slot(0x0c, 0x00af6da0);
    return native_particle_model_is_type_00af6da0(self.access_.types_00f8d308, token);
}
void NativeParticleModelReference::attach_scene(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& binding, SceneResource* scene, bool recurse) {
    auto& self = from_binding(binding);
    self.require_slot(0x50, 0x00b6ed80);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void NativeParticleModelReference::remove_scene(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& binding, SceneResource* scene, bool recurse) {
    auto& self = from_binding(binding);
    self.require_slot(0x54, 0x00b6ee10);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void NativeParticleModelReference::world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding) {
    auto& self = from_binding(binding);
    self.require_slot(0x40, 0x00b6dbe0);
    native_node_world_changed_00b6dbe0(runtime, binding);
}
std::uint32_t NativeParticleModelReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeParticleModelReference*>(context)->owner_.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin)) std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
GeneratedModelPointLightLinks& NativeParticleModelReference::light_element(void* context, std::uint32_t index) noexcept {
    auto* light = static_cast<NativeParticleModelReference*>(context)->owner_.storage.node.point_lights_164.begin[index];
    if (!light) std::terminate();
    return *light;
}
void NativeParticleModelReference::shrink_lights(void* context) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(
        static_cast<NativeParticleModelReference*>(context)->owner_.storage.node.point_lights_164);
}
void NativeParticleModelReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live) std::terminate();
    require_slot(0x18, 0x00b6f310);
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.storage.node.released_44, *this, {this, light_count, light_element, shrink_lights}});
}
void NativeParticleModelReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeModelOwner::Phase::dead) std::terminate();
    require_slot(0x54, 0x00b6ee10, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes, owner_.node.scene_attachment, scene, recurse);
}
void NativeParticleModelReference::retire() noexcept {
    runtime_.unbind(*this);
    phase_ = Phase::retired;
    const auto disposal = disposal_;
    disposal.retire(disposal.context, *this);
}
void NativeParticleModelReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live) std::terminate();
    require_slot(0, 0x00bd30e0);
    require_slot(4, 0x00af7f40);
    phase_ = Phase::destroying;
    delete_native_particle_model_00af7f40(owner_, access_, 1);
    retire();
}
void NativeParticleModelReference::retire_after_failed_construction() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::dead) std::terminate();
    retire();
}
} // namespace bsp
