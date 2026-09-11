#include "bsp/native_group_owner.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native group ownership requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeGroupTypeDescriptor) == 16);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeGroupTailStorage) == 0x14);
static_assert(NativeGroupPool::slot_slab_index_offset == 0x188);
static_assert(NativeGroupPool::slot_bytes == 0x18c);
namespace {
constexpr std::uint32_t group_table = 0x00d634f8u;
constexpr std::uint32_t node_table = 0x00d62c88u;
const volatile std::uint32_t* current_table(const NativeGroupOwner& owner, bool allow_node) noexcept {
    if (owner.storage.node.vtable_00 == group_table) return owner.environment.vtable_00d634f8;
    if (allow_node && owner.storage.node.vtable_00 == node_table) return owner.environment.vtable_00d62c88;
    return nullptr;
}
void require_slot(const NativeGroupOwner& owner, std::uint32_t offset,
    std::uint32_t address, bool allow_node = false) {
    const auto* table = current_table(owner, allow_node);
    if (!table || table[offset / 4] != address)
        throw std::logic_error("native group current virtual profile is unsupported");
}
void require_terminal_slot(const NativeGroupOwner& owner, std::uint32_t offset,
    std::uint32_t address, bool allow_node = false) noexcept {
    const auto* table = current_table(owner, allow_node);
    if (!table || table[offset / 4] != address) std::terminate();
}
void require_live(const NativeGroupOwner& owner) {
    if (owner.phase != NativeGroupOwner::Phase::live)
        throw std::logic_error("native group operation requires its live owner");
}
void validate_array(const NativeGroupTailStorage& group) {
    if (group.attached_count_17c < 0 || group.attached_capacity_180 < group.attached_count_17c ||
        (group.attached_capacity_180 && !group.attached_nodes_178))
        throw std::logic_error("native group attachment descriptor has invalid extent");
}
void append_actual(void* context, CameraTransform& node) {
    auto& group = static_cast<NativeGroupOwner*>(context)->storage.group;
    validate_array(group);
    if (group.attached_count_17c == group.attached_capacity_180) {
        const auto capacity = static_cast<std::uint32_t>(group.attached_capacity_180);
        if (capacity > 0x1fffffffu) throw std::length_error("native group attachment growth overflows");
        reserve_native_group_attached_nodes_0059e5e0(group, static_cast<std::int32_t>(capacity * 2));
    }
    group.attached_nodes_178[group.attached_count_17c] = &node;
    ++group.attached_count_17c;
}
bool erase_actual(void* context, CameraTransform* node) noexcept {
    auto& group = static_cast<NativeGroupOwner*>(context)->storage.group;
    validate_array(group);
    for (std::int32_t i = 0; i < group.attached_count_17c; ++i) {
        if (group.attached_nodes_178[i] != node) continue;
        const auto last = group.attached_count_17c - 1;
        if (i != last) group.attached_nodes_178[i] = group.attached_nodes_178[last];
        --group.attached_count_17c;
        return true;
    }
    return false;
}
NativeGroupStorageView prepare_storage(void* slot, std::size_t bytes, NativeGroupEnvironment& environment) {
    if (!slot || bytes < NativeGroupPool::slot_bytes ||
        reinterpret_cast<std::uintptr_t>(slot) % alignof(NativeNodeStorage) ||
        !environment.vtable_00d634f8 || !environment.vtable_00d62c88)
        throw std::invalid_argument("group requires one aligned18Ch slot and actual group/node tables");
    std::array<std::byte, 0x188> preimage;
    std::memcpy(preimage.data(), slot, preimage.size());
    auto& node = *::new (slot) NativeNodeStorage;
    auto& group = *::new (static_cast<std::byte*>(slot) + 0x174) NativeGroupTailStorage;
    std::memcpy(slot, preimage.data(), preimage.size());
    return {node, group};
}
bool group_is_type(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding, std::uint32_t token) {
    auto& owner = *static_cast<NativeGroupOwner*>(binding.context);
    const auto* table = current_table(owner, true);
    if (!table) throw std::logic_error("group type dispatch requires its current native table");
    if (table[3] == 0x00b8f650u) return owner.environment.types.is_type_00b8f650(token);
    if (table[3] == 0x00b6f570u && owner.storage.node.vtable_00 == node_table)
        return owner.environment.nodes.node_virtual_0c(runtime, binding, token);
    throw std::logic_error("group current native type predicate is unsupported");
}
void group_attach_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeGroupOwner*>(binding.context);
    require_slot(owner, 0x50, 0x00b6ed80u, true);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void group_remove_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeGroupOwner*>(binding.context);
    require_slot(owner, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void publish_group_phase(NativeGroupOwner& owner) noexcept {
    owner.storage.node.vtable_00 = group_table;
    owner.node.scene_attachment.is_type = group_is_type;
    owner.node.scene_attachment.attach_scene = group_attach_scene;
    owner.node.scene_attachment.remove_scene = group_remove_scene;
    owner.node.scene_attachment.world_changed = native_group_world_changed_00b8e6b0;
}
void end_tail(NativeGroupOwner& owner) noexcept {
    owner.environment.nodes.attachments.unbind_attachment(owner.attached_nodes);
    owner.environment.nodes.scenes.forget_destroyed_binding(owner.node.scene_attachment);
    owner.storage.group.~NativeGroupTailStorage();
    owner.phase = NativeGroupOwner::Phase::dead;
}
void finish_node(NativeGroupOwner& owner) {
    try { destroy_native_node_00b6f440(owner.environment.nodes, owner.node); }
    catch (...) { end_tail(owner); throw; }
    end_tail(owner);
}
} // namespace

NativeGroupTypes::NativeGroupTypes(TypeIdCounterLifetime& counter, LightTypeBootstrap& shared,
    NativeGroupTypeStorage storage) noexcept : counter_(counter), shared_types_(shared), storage_(storage) {}
std::uint32_t NativeGroupTypes::type_id_00b8e620() const noexcept { return storage_.group_0109032c.own_id; }
bool NativeGroupTypes::is_type_00b8f650(std::uint32_t token) const noexcept {
    return storage_.group_0109032c.own_id == token || storage_.group_0109032c.node_id == token ||
        storage_.group_0109032c.root_id == token;
}
void NativeGroupTypes::initialize_00b8f590(volatile NativeGroupTypeDescriptor& target) {
    if (storage_.guard_010902e1 == 0) {
        storage_.guard_010902e1 = 1;
        target.native_name_address = 0x00d634f0u;
        auto& node = shared_types_.storage().node_0108ff90;
        shared_types_.initialize_node_00b6f110(node);
        target.node_id = node.own_id;
        target.root_id = node.root_id;
        auto* counter = counter_.get_006fac20();
        const auto id = counter->next_id_04;
        counter->next_id_04 = id + 1u;
        target.own_id = id;
    }
}
void reserve_native_group_attached_nodes_0059e5e0(NativeGroupTailStorage& group, std::int32_t capacity) {
    validate_array(group);
    if (capacity < 1) capacity = 1;
    if (capacity <= group.attached_capacity_180) return;
    if (static_cast<std::uint32_t>(capacity) > 0x3fffffffu)
        throw std::length_error("native group attachment extent overflows");
    auto** replacement = static_cast<CameraTransform**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, static_cast<std::size_t>(capacity) * 4,
        static_cast<std::size_t>(capacity) * sizeof(CameraTransform*)}));
    for (std::int32_t i = 0; i < group.attached_count_17c; ++i)
        replacement[i] = group.attached_nodes_178[i];
    singleton_lifetime_free(group.attached_nodes_178);
    group.attached_nodes_178 = replacement; // native stores are after returning free
    group.attached_capacity_180 = capacity;
}
void resize_native_group_attached_nodes_0059fce0(NativeGroupTailStorage& group, std::int32_t count) {
    validate_array(group);
    if (count < 0) throw std::invalid_argument("negative native group attachment count");
    if (count > group.attached_capacity_180) reserve_native_group_attached_nodes_0059e5e0(group, count);
    for (auto i = group.attached_count_17c; i < count; ++i) group.attached_nodes_178[i] = nullptr;
    while (group.attached_count_17c > count) --group.attached_count_17c;
    group.attached_count_17c = count;
}
NativeGroupOwner::NativeGroupOwner(void* slot, std::size_t bytes, NativeGroupEnvironment& access)
    : storage(prepare_storage(slot, bytes, access)), environment(access),
      node(storage.node, NativeNodePreconstructionBinding{}, group_is_type, group_attach_scene, this),
      attached_nodes{slot, {}, {this, append_actual, erase_actual}} {
    node.scene_attachment.world_changed = native_group_world_changed_00b8e6b0;
    node.scene_attachment.remove_scene = group_remove_scene;
    try {
        access.nodes.scenes.bind(node.scene_attachment);
        try { access.nodes.attachments.bind_attachment(attached_nodes); }
        catch (...) { access.nodes.scenes.forget_destroyed_binding(node.scene_attachment); throw; }
    } catch (...) {
        storage.group.~NativeGroupTailStorage();
        storage.node.~NativeNodeStorage();
        throw;
    }
}
NativeGroupOwner::~NativeGroupOwner() {
    if (phase == Phase::prepared) {
        end_tail(*this);
        storage.node.~NativeNodeStorage();
    } else if (phase != Phase::dead) std::terminate();
}
void* construct_native_group_00b8f5e0(NativeGroupOwner& owner, const NativeString& name) {
    if (owner.phase != NativeGroupOwner::Phase::prepared)
        throw std::logic_error("group construction requires an unused prepared slot");
    owner.phase = NativeGroupOwner::Phase::constructing;
    try {
        construct_native_group_00b8f5e0(&owner.storage.node, NativeGroupPool::slot_bytes, name,
            owner.environment.nodes.strings, owner.environment.constants);
    } catch (...) { end_tail(owner); throw; }
    publish_group_phase(owner);
    owner.phase = NativeGroupOwner::Phase::live;
    return &owner.storage.node;
}
void destroy_native_group_00b8f680(NativeGroupOwner& owner) {
    require_live(owner);
    publish_group_phase(owner);
    try {
        resize_native_group_attached_nodes_0059fce0(owner.storage.group, 0);
        singleton_lifetime_free(owner.storage.group.attached_nodes_178);
    } catch (...) {
        try { finish_node(owner); } catch (...) { std::terminate(); }
        throw;
    }
    finish_node(owner); // EH state-1: never call base a second time on its throw.
}
void* delete_native_group_00b8f8c0(NativeGroupOwner& owner, std::uint32_t flags) {
    void* const slot = &owner.storage.node;
    destroy_native_group_00b8f680(owner);
    if (flags & 1u) owner.environment.pool_010902f4.return_raw_slot_00b8ed40(slot);
    return slot;
}
void native_group_attachment_virtual1c(NativeGroupOwner& owner, NativeNodeParentingRuntime& runtime,
    void* actual_group) {
    require_live(owner);
    if (&runtime.nodes != &owner.environment.nodes)
        throw std::invalid_argument("group attachment requires its canonical node runtime");
    require_slot(owner, 0x1c, 0x00b8f4f0u);
    set_native_group_attachment_00b8f4f0(runtime, owner.node.transform, actual_group);
}
void notify_native_group_bounds_00b6dbc0(NativeGroupOwner& owner) {
    require_slot(owner, 0x3c, 0x00b6dbc0u, true);
    auto& node = owner.node.transform;
    node.auxiliary_flags &= 0xffffffc3u;
    if (void* enclosing = node.notification_context) {
        const auto invoke = node.notify_changed;
        if (!invoke) throw std::logic_error("group enclosing owner requires actual virtual3C");
        invoke(enclosing);
    }
}
void native_group_world_changed_00b8e6b0(SceneAttachmentRuntime&, SceneNodeAttachment& binding) {
    auto& owner = *static_cast<NativeGroupOwner*>(binding.context);
    require_slot(owner, 0x40, 0x00b8e6b0u);
    binding.transform.auxiliary_flags &= 0xffffffcfu; // no enclosing-owner callback
}
NativeGroupReference::NativeGroupReference(NativeGroupOwner& owner, NativeGroupCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.node.references_04), owner_(owner),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    if (owner.phase != NativeGroupOwner::Phase::live || !disposal.retire ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("group reference requires live owner and explicit retirement");
    require_terminal_slot(owner, 0, 0x00bd30e0u);
    require_terminal_slot(owner, 4, 0x00b8f8c0u);
    runtime_.bind(*this);
}
NativeGroupReference::~NativeGroupReference() { if (phase_ != Phase::retired) std::terminate(); }
std::uint32_t NativeGroupReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeGroupReference*>(context)->owner_.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin)) std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
GeneratedModelPointLightLinks& NativeGroupReference::light_element(void* context, std::uint32_t index) noexcept {
    auto* light = static_cast<NativeGroupReference*>(context)->owner_.storage.node.point_lights_164.begin[index];
    if (!light) std::terminate();
    return *light;
}
void NativeGroupReference::shrink_lights(void* context) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(
        static_cast<NativeGroupReference*>(context)->owner_.storage.node.point_lights_164);
}
void NativeGroupReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeGroupOwner::Phase::live) std::terminate();
    require_terminal_slot(owner_, 0x18, 0x00b8eec0u);
    auto& group = owner_.storage.group;
    validate_array(group);
    while (group.attached_count_17c != 0) {
        auto* child = group.attached_nodes_178[group.attached_count_17c - 1];
        --group.attached_count_17c;
        if (!child) std::terminate();
        child->notification_context = nullptr; // native clears even a mismatched owner
        child->notify_changed = nullptr;
    }
    release_node_logical_00b6f310({runtime_, owner_.node.transform, owner_.storage.node.released_44,
        *this, {this, light_count, light_element, shrink_lights}});
}
void NativeGroupReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeGroupOwner::Phase::dead) std::terminate();
    require_terminal_slot(owner_, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes, owner_.node.scene_attachment, scene, recurse);
}
void NativeGroupReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeGroupOwner::Phase::live) std::terminate();
    require_terminal_slot(owner_, 0, 0x00bd30e0u);
    require_terminal_slot(owner_, 4, 0x00b8f8c0u);
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_group_00b8f8c0(owner_, 1);
    runtime.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this); // no access after terminal callback
}
} // namespace bsp
