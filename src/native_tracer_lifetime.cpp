#include "bsp/native_tracer_lifetime.hpp"
#include "bsp/native_tracer_construction.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tracer lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeNodeStorage) == 0x174);
constexpr std::uint32_t tracer_table = 0x00d63fa0;
constexpr std::uint32_t model_table = 0x00d62de8;
constexpr std::uint32_t node_table = 0x00d62c88;
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
void* volatile& pointer(void* base, std::uint32_t offset) noexcept {
    return *static_cast<void* volatile*>(at(base, offset));
}
volatile std::int32_t& integer(void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::int32_t*>(at(base, offset));
}
const volatile std::uint32_t* current_table(const NativeTracerProfileBindings& binding) noexcept {
    const auto& owner = binding.model;
    switch (owner.storage.node.vtable_00) {
    case tracer_table: return binding.vtable_00d63fa0;
    case model_table: return owner.environment.vtable_00d62de8;
    case node_table: return owner.environment.vtable_00d62c88;
    default: return nullptr;
    }
}
void require_slot(const NativeTracerProfileBindings& binding, std::uint32_t offset,
    std::uint32_t target) {
    const auto* table = current_table(binding);
    if (!table || table[offset / 4] != target)
        throw std::logic_error("tracer requires its actual current virtual target");
}
void require_terminal_slot(const NativeTracerProfileBindings& binding,
    std::uint32_t offset, std::uint32_t target) noexcept {
    const auto* table = current_table(binding);
    if (!table || table[offset / 4] != target) std::terminate();
}
NativeTracerProfileBindings& profile(SceneNodeAttachment& node) noexcept {
    return *static_cast<NativeTracerProfileBindings*>(node.context);
}
bool is_type(SceneAttachmentRuntime& runtime, SceneNodeAttachment& node, std::uint32_t token) {
    auto& binding = profile(node);
    const auto* table = current_table(binding);
    if (!table) throw std::logic_error("tracer type dispatch requires its current profile");
    if (table[3] == 0x006ef860) return binding.model.environment.types.is_type_006ef860(token);
    if (table[3] == 0x00b6f570 && binding.model.storage.node.vtable_00 == node_table)
        return binding.model.environment.nodes.node_virtual_0c(runtime, node, token);
    throw std::logic_error("tracer current type predicate is unsupported");
}
void attach(SceneAttachmentRuntime& runtime, SceneNodeAttachment& node,
    SceneResource* scene, bool recurse) {
    require_slot(profile(node), 0x50, 0x00b6ed80);
    set_node_scene_00b6ed80(runtime, node, scene, recurse);
}
void remove(SceneAttachmentRuntime& runtime, SceneNodeAttachment& node,
    SceneResource* scene, bool recurse) {
    require_slot(profile(node), 0x54, 0x00b6ee10);
    remove_native_node_scene_00b6ee10(runtime, node, scene, recurse);
}
void world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& node) {
    require_slot(profile(node), 0x40, 0x00b6dbe0);
    native_node_world_changed_00b6dbe0(runtime, node);
}
void require_live(const NativeTracerProfileBindings& binding) {
    if (binding.model.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("tracer lifetime requires its live canonical model owner");
}
void release_then_clear(void* slot, std::uint32_t offset, NativeRenderActualOwners& owners) {
    if (void* const current = pointer(slot, offset)) {
        release_native_render_actual_owner(owners, current);
        pointer(slot, offset) = nullptr;
    }
}
void finish_base(NativeTracerProfileBindings& binding) {
    restore_native_tracer_model_profile(binding);
    destroy_native_model_00b750c0(binding.model);
}
} // namespace

NativeTracerProfileBindings::NativeTracerProfileBindings(NativeModelOwner& owner,
    std::size_t bytes, void* pool, const volatile std::uint32_t* table)
    : model(owner), actual_pool_0109049c(pool), vtable_00d63fa0(table),
      model_is_type_(owner.node.scene_attachment.is_type),
      model_attach_(owner.node.scene_attachment.attach_scene),
      model_remove_(owner.node.scene_attachment.remove_scene),
      model_world_changed_(owner.node.scene_attachment.world_changed),
      model_context_(owner.node.scene_attachment.context) {
    if (bytes < native_tracer_slot_bytes || !pool || !table || model_context_ != &owner)
        throw std::invalid_argument("tracer requires actual7B0h slot, canonical model and current table/pool");
}
void publish_native_tracer_profile_00bad6f0(NativeTracerProfileBindings& binding) noexcept {
    auto& node = binding.model.node.scene_attachment;
    binding.model.storage.node.vtable_00 = tracer_table;
    node.context = &binding;
    node.is_type = is_type;
    node.attach_scene = attach;
    node.remove_scene = remove;
    node.world_changed = world_changed;
}
void restore_native_tracer_model_profile(NativeTracerProfileBindings& binding) noexcept {
    auto& node = binding.model.node.scene_attachment;
    node.context = binding.model_context_;
    node.is_type = binding.model_is_type_;
    node.attach_scene = binding.model_attach_;
    node.remove_scene = binding.model_remove_;
    node.world_changed = binding.model_world_changed_;
}
void retire_failed_native_tracer_profile(NativeTracerProfileBindings& binding) noexcept {
    restore_native_tracer_model_profile(binding);
}
void reserve_native_tracer_pointer_array_00bac130(void* header, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (integer(header, 8) >= capacity) return;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 4u;
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    void* destination = replacement;
    for (std::int32_t index = 0; index < integer(header, 4); ++index) {
        if (destination) pointer(destination, 0) = pointer(pointer(header, 0),
            static_cast<std::uint32_t>(index) * 4u);
        destination = at(destination, 4);
    }
    singleton_lifetime_free(pointer(header, 0));
    pointer(header, 0) = replacement;
    integer(header, 8) = capacity;
}
void resize_native_tracer_pointer_array_00bac370(void* header, std::int32_t count) {
    if (count > integer(header, 8)) reserve_native_tracer_pointer_array_00bac130(header, count);
    for (std::int32_t index = integer(header, 4); index < count; ++index) {
        void* const destination = at(pointer(header, 0), static_cast<std::uint32_t>(index) * 4u);
        if (destination) pointer(destination, 0) = nullptr;
    }
    while (count < integer(header, 4)) integer(header, 4) = integer(header, 4) - 1;
    integer(header, 4) = count;
}
void destroy_native_tracer_point_array_00bac860(void* header) {
    resize_native_tracer_points_00bac310(*static_cast<NativeTracerPointArrayStorage*>(header), 0);
    singleton_lifetime_free(pointer(header, 0));
}
void destroy_native_tracer_pointer_array_00bac880(void* header) {
    resize_native_tracer_pointer_array_00bac370(header, 0);
    singleton_lifetime_free(pointer(header, 0));
}
void destroy_native_tracer_00bac970(NativeTracerProfileBindings& binding) {
    require_live(binding);
    void* const slot = &binding.model.storage.node;
    publish_native_tracer_profile_00bad6f0(binding);
    unsigned unwind_state = 2;
    try {
        auto& owners = binding.model.environment.retained_owners;
        release_then_clear(slot, 0x248, owners);
        release_then_clear(slot, 0x1bc, owners);
        release_then_clear(slot, 0x190, owners);
        if (void* const child = pointer(slot, 0x254)) {
            auto* const lifetime = binding.model.environment.nodes.attachments.find_actual_node(
                reinterpret_cast<std::uint32_t>(child));
            if (!lifetime) throw std::logic_error("tracer child requires its existing actual node lifetime");
            unlink_and_release_render_model_00b6dfa0(*lifetime);
            pointer(slot, 0x254) = nullptr;
        }
        unwind_state = 1;
        destroy_native_tracer_pointer_array_00bac880(at(slot, 0x1a0));
        unwind_state = 0;
        destroy_native_tracer_point_array_00bac860(at(slot, 0x194));
    } catch (...) {
        try {
            if (unwind_state >= 2) destroy_native_tracer_pointer_array_00bac880(at(slot, 0x1a0));
            if (unwind_state >= 1) destroy_native_tracer_point_array_00bac860(at(slot, 0x194));
            finish_base(binding);
        } catch (...) { std::terminate(); }
        throw;
    }
    finish_base(binding); // native state-1 before the call: never retry a throwing base
}
void* delete_native_tracer_00bacb90(NativeTracerProfileBindings& binding, std::uint32_t flags) {
    void* const slot = &binding.model.storage.node;
    destroy_native_tracer_00bac970(binding);
    if (flags & 1u) return_native_tracer_slot_00babf70(binding.actual_pool_0109049c, slot);
    return slot;
}

NativeTracerReference::NativeTracerReference(NativeTracerProfileBindings& binding,
    NativeTracerCompanionDisposal disposal)
    : RenderCommandReference(binding.model.storage.node.references_04), profile_(binding),
      runtime_(binding.model.environment.nodes.attachments), disposal_(disposal) {
    require_live(binding);
    if (!disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("tracer reference requires live native count and companion retirement");
    require_terminal_slot(binding, 0, 0x00bd30e0);
    require_terminal_slot(binding, 4, 0x00bacb90);
    runtime_.bind(*this);
}
NativeTracerReference::~NativeTracerReference() {
    if (phase_ != Phase::retired) std::terminate();
}
std::uint32_t NativeTracerReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeTracerReference*>(context)->profile_.model.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin)) std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
GeneratedModelPointLightLinks& NativeTracerReference::light_element(void* context, std::uint32_t index) noexcept {
    const auto& array = static_cast<NativeTracerReference*>(context)->profile_.model.storage.node.point_lights_164;
    auto* const light = array.begin[index];
    if (!light) std::terminate();
    return *light;
}
void NativeTracerReference::shrink_lights(void* context) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(
        static_cast<NativeTracerReference*>(context)->profile_.model.storage.node.point_lights_164);
}
void NativeTracerReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || profile_.model.phase != NativeModelOwner::Phase::live) std::terminate();
    require_terminal_slot(profile_, 0x18, 0x00b6f310);
    release_node_logical_00b6f310({runtime_, profile_.model.node.transform,
        profile_.model.storage.node.released_44, *this,
        {this, light_count, light_element, shrink_lights}});
}
void NativeTracerReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    if (phase_ == Phase::retired || profile_.model.phase == NativeModelOwner::Phase::dead) std::terminate();
    require_terminal_slot(profile_, 0x54, 0x00b6ee10);
    remove_native_node_scene_00b6ee10(profile_.model.environment.nodes.scenes,
        profile_.model.node.scene_attachment, scene, recurse);
}
void NativeTracerReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || profile_.model.phase != NativeModelOwner::Phase::live) std::terminate();
    require_terminal_slot(profile_, 0, 0x00bd30e0);
    require_terminal_slot(profile_, 4, 0x00bacb90);
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_tracer_00bacb90(profile_, 1);
    runtime.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
