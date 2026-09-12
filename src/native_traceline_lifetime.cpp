#include "bsp/native_traceline_lifetime.hpp"
#include "bsp/native_traceline_pool.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Traceline lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* raw, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(raw) + offset, sizeof(value));
    return value;
}
template<class T> void write(void* raw, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(raw) + offset, &value, sizeof(value));
}
void release_then_clear(void* raw, std::size_t offset, NativeRenderActualOwners& owners) {
    if (void* captured = read<void*>(raw, offset)) {
        release_native_render_actual_owner(owners, captured);
        write<void*>(raw, offset, nullptr);
    }
}
void destroy_payload_array(void* raw, const NativeTracelineMemory& memory) noexcept {
    if (void* captured = read<void*>(raw, 0x4c)) {
        memory.free_00bf6989(captured);
        write<void*>(raw, 0x4c, nullptr);
    }
}
void require_memory(const NativeTracelineMemory& memory) {
    if (!memory.free_00bf6989 || !memory.free_00bf65ac)
        throw std::invalid_argument("Traceline lifetime requires actual returning CRT frees");
}
} // namespace

void destroy_native_traceline_00af1980(NativeModelOwner& owner,
    const NativeTracelineMemory& memory) {
    if (owner.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("Traceline destruction requires its same live model owner");
    require_memory(memory);
    void* const segments = read<void*>(&owner.storage.node, 0x188);
    owner.storage.node.vtable_00 = 0x00d0c8c8;
    if (segments) memory.free_00bf6989(segments);
    destroy_native_model_00b750c0(owner);
}
void* delete_native_traceline_00858380(NativeModelOwner& owner,
    NativeTracelineLifetimeAccess& access, std::uint32_t flags) {
    void* const slot = &owner.storage.node;
    if (!owns_native_traceline_slot(access.actual_pool_00f8c288, slot))
        throw std::invalid_argument("Traceline scalar deletion requires its actual1BCh pool identity");
    destroy_native_traceline_00af1980(owner, access.memory);
    if (flags & 1u) return_native_traceline_00af1ea0(slot, access.actual_pool_00f8c288);
    return slot;
}
void destroy_native_traceline_payload_0086ad50(void* raw,
    NativeRenderActualOwners& owners, const NativeTracelineMemory& memory) {
    require_memory(memory);
    if (!raw) throw std::invalid_argument("Traceline payload destructor requires actual80h storage");
    write<std::uint32_t>(raw, 0, 0x00d0d4a4);
    try {
        release_then_clear(raw, 0x40, owners);
        release_then_clear(raw, 0x44, owners);
    } catch (...) {
        // State0: DC740C -> C95460 ->869B10. The unvisited reference is
        // not released, and a throwing release's captured field stays set.
        destroy_payload_array(raw, memory);
        throw;
    }
    destroy_payload_array(raw, memory);
}
void* delete_native_traceline_payload_0086ade0(void* raw,
    NativeRenderActualOwners& owners, std::uint32_t flags, const NativeTracelineMemory& memory) {
    destroy_native_traceline_payload_0086ad50(raw, owners, memory);
    if (flags & 1u) memory.free_00bf65ac(raw);
    return raw;
}

NativeTracelineReference::NativeTracelineReference(NativeModelOwner& owner,
    NativeTracelineLifetimeAccess& access, NativeTracelineCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.node.references_04), owner_(owner), access_(access),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    require_memory(access.memory);
    if (owner.phase != NativeModelOwner::Phase::live || !disposal.retire ||
        !access.vtable_00d0c928 || !access.vtable_00d0c8c8 ||
        owner.storage.node.vtable_00 != 0x00d0c928 ||
        reference_count.load(std::memory_order_relaxed) <= 0 ||
        !owns_native_traceline_slot(access.actual_pool_00f8c288, &owner.storage.node))
        throw std::invalid_argument("Traceline reference requires its actual live derived slot and bindings");
    require_slot(0, 0x00bd30e0);
    require_slot(4, 0x00858380);
    runtime_.bind(*this);
    auto& binding = owner.node.scene_attachment;
    binding.is_type = is_type;
    binding.attach_scene = attach_scene;
    binding.remove_scene = remove_scene;
    binding.world_changed = world_changed;
}
NativeTracelineReference::~NativeTracelineReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeTracelineReference::require_slot(std::uint32_t offset,
    std::uint32_t expected, bool allow_base) const noexcept {
    const volatile std::uint32_t* table = nullptr;
    const auto profile = owner_.storage.node.vtable_00;
    if (profile == 0x00d0c928) table = access_.vtable_00d0c928;
    else if (allow_base && profile == 0x00d0c8c8) table = access_.vtable_00d0c8c8;
    else if (allow_base && profile == 0x00d62de8) table = owner_.environment.vtable_00d62de8;
    else if (allow_base && profile == 0x00d62c88) table = owner_.environment.vtable_00d62c88;
    if (!table || table[offset / 4] != expected) std::terminate();
}
NativeTracelineReference& NativeTracelineReference::from_binding(SceneNodeAttachment& binding) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    auto* result = dynamic_cast<NativeTracelineReference*>(
        &owner.environment.nodes.attachments.resolve(owner.node.transform));
    if (!result) throw std::logic_error("Traceline requires its existing canonical node lifetime");
    return *result;
}
bool NativeTracelineReference::is_type(SceneAttachmentRuntime&, SceneNodeAttachment& binding,
    std::uint32_t token) {
    auto& self = from_binding(binding);
    self.require_slot(0x0c, 0x006ef860, true);
    return self.owner_.environment.types.is_type_006ef860(token);
}
void NativeTracelineReference::attach_scene(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& binding, SceneResource* scene, bool recurse) {
    from_binding(binding).require_slot(0x50, 0x00b6ed80, true);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void NativeTracelineReference::remove_scene(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& binding, SceneResource* scene, bool recurse) {
    from_binding(binding).require_slot(0x54, 0x00b6ee10, true);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void NativeTracelineReference::world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding) {
    from_binding(binding).require_slot(0x40, 0x00b6dbe0, true);
    native_node_world_changed_00b6dbe0(runtime, binding);
}
std::uint32_t NativeTracelineReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeTracelineReference*>(context)->owner_.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin)) std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void NativeTracelineReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& owner = static_cast<NativeTracelineReference*>(context)->owner_;
    auto& light = owner.environment.nodes.point_lights.light(owner.storage.node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, owner.storage.node);
}
void NativeTracelineReference::shrink_lights(void* context) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(
        static_cast<NativeTracelineReference*>(context)->owner_.storage.node.point_lights_164);
}
void NativeTracelineReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live) std::terminate();
    require_slot(0x18, 0x00b6f310);
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.storage.node.released_44, *this, {this, light_count, remove_light_backlink, shrink_lights}});
}
void NativeTracelineReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeModelOwner::Phase::dead) std::terminate();
    require_slot(0x54, 0x00b6ee10, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes,
        owner_.node.scene_attachment, scene, recurse);
}
void NativeTracelineReference::retire() noexcept {
    runtime_.unbind(*this);
    phase_ = Phase::retired;
    const auto disposal = disposal_;
    disposal.retire(disposal.context, *this);
}
void NativeTracelineReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live) std::terminate();
    require_slot(0, 0x00bd30e0);
    require_slot(4, 0x00858380);
    phase_ = Phase::destroying;
    delete_native_traceline_00858380(owner_, access_, 1);
    retire();
}
void NativeTracelineReference::retire_after_failed_construction() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::dead) std::terminate();
    retire();
}
} // namespace bsp
