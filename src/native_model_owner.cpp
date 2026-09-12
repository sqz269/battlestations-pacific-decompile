#include "bsp/native_model_owner.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native model ownership requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeModelTailStorage) == 0x10);
static_assert(offsetof(NativeModelTailStorage, retained_174) == 0);
static_assert(offsetof(NativeModelTailStorage, scalar_178) == 4);
static_assert(offsetof(NativeModelTailStorage, scalar_17c) == 8);
static_assert(offsetof(NativeModelTailStorage, geometry_180) == 12);
static_assert(sizeof(NativeNodeStorage) + sizeof(NativeModelTailStorage) == 0x184);
static_assert(NativeModelPool::slot_slab_index_offset == 0x184);
static_assert(NativeModelPool::slot_bytes == 0x188);

namespace {
constexpr std::uint32_t model_table = 0x00d62de8u;
constexpr std::uint32_t node_table = 0x00d62c88u;
void word(float& destination, std::uint32_t bits) noexcept {
    std::memcpy(&destination, &bits, sizeof(bits));
}
void pose_word(NativeNodeStorage& node, std::size_t offset, std::uint32_t bits) noexcept {
    std::memcpy(node.untouched_08.data() + offset - 8, &bits, sizeof(bits));
}
const volatile std::uint32_t* current_table(const NativeModelOwner& owner,
    bool allow_node_phase) noexcept {
    const auto profile = owner.storage.node.vtable_00;
    if (profile == model_table) return owner.environment.vtable_00d62de8;
    if (allow_node_phase && profile == node_table) return owner.environment.vtable_00d62c88;
    return nullptr;
}
void require_slot(const NativeModelOwner& owner, std::uint32_t offset,
    std::uint32_t expected, bool allow_node_phase = false) {
    const auto* table = current_table(owner, allow_node_phase);
    if (!table || table[offset / 4] != expected)
        throw std::logic_error("native model current virtual profile is unsupported");
}
void require_terminal_slot(const NativeModelOwner& owner, std::uint32_t offset,
    std::uint32_t expected, bool allow_node_phase = false) noexcept {
    const auto* table = current_table(owner, allow_node_phase);
    if (!table || table[offset / 4] != expected) std::terminate();
}
NativeModelStorageView prepare_storage(void* slot, std::size_t bytes,
    NativeModelEnvironment& environment) {
    if (!slot || bytes < NativeModelPool::slot_bytes ||
        reinterpret_cast<std::uintptr_t>(slot) % alignof(NativeNodeStorage) ||
        !environment.vtable_00d62de8 || !environment.vtable_00d62c88)
        throw std::invalid_argument("model requires one aligned188h slot and actual model/node table views");
    std::array<std::byte, 0x184> preimage;
    std::memcpy(preimage.data(), slot, preimage.size());
    auto& node = *::new (slot) NativeNodeStorage;
    auto& tail = *::new (static_cast<std::byte*>(slot) + 0x174) NativeModelTailStorage;
    std::memcpy(slot, preimage.data(), preimage.size());
    return {node, tail};
}
bool model_is_type(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    std::uint32_t token) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    const auto* table = current_table(owner, true);
    if (!table) throw std::logic_error("model type dispatch requires its current native table");
    if (table[3] == 0x006ef860u) return owner.environment.types.is_type_006ef860(token);
    if (table[3] == 0x00b6f570u && owner.storage.node.vtable_00 == node_table)
        return owner.environment.nodes.node_virtual_0c(runtime, binding, token);
    throw std::logic_error("model type dispatch has no actual current predicate");
}
void model_attach_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    require_slot(owner, 0x50, 0x00b6ed80u, true);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void model_remove_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    require_slot(owner, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void model_world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    require_slot(owner, 0x40, 0x00b6dbe0u, true);
    native_node_world_changed_00b6dbe0(runtime, binding);
}
void publish_model_phase(NativeModelOwner& owner) noexcept {
    owner.storage.node.vtable_00 = model_table;
    owner.node.scene_attachment.is_type = model_is_type;
    owner.node.scene_attachment.attach_scene = model_attach_scene;
    owner.node.scene_attachment.remove_scene = model_remove_scene;
    owner.node.scene_attachment.world_changed = model_world_changed;
}
void end_tail(NativeModelOwner& owner) noexcept {
    owner.storage.model.~NativeModelTailStorage();
    owner.environment.nodes.scenes.forget_destroyed_binding(owner.node.scene_attachment);
    owner.phase = NativeModelOwner::Phase::dead;
}
void finish_node(NativeModelOwner& owner) {
    try {
        if (owner.environment.actual_names)
            destroy_native_node_00b6f440(owner.environment.nodes, owner.node,
                *owner.environment.actual_names);
        else
            destroy_native_node_00b6f440(owner.environment.nodes, owner.node);
    } catch (...) {
        end_tail(owner);
        throw;
    }
    end_tail(owner);
}
void require_live(const NativeModelOwner& owner) {
    if (owner.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("native model operation requires its live owner");
}
void release_then_clear(void*& field, NativeRenderActualOwners& owners) {
    if (void* captured = field) {
        release_native_render_actual_owner(owners, captured);
        field = nullptr;
    }
}
void retain_raw(void* value) noexcept {
    auto* actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(value) + 4));
    actual->fetch_add(1, std::memory_order_seq_cst);
}
bool different_scalar(float value, std::uint32_t sentinel) noexcept {
    // Original UCOMISS/LAHF/TEST44/JNP: unordered values are written too.
    // Use the SSE compare itself, including its MXCSR exception behavior.
    unsigned char different;
    __asm {
        movss xmm1, dword ptr [value]
        movss xmm0, dword ptr [sentinel]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        setp al
        mov different, al
    }
    return different != 0;
}
} // namespace

NativeModelOwner::NativeModelOwner(void* slot, std::size_t bytes, NativeModelEnvironment& access)
    : storage(prepare_storage(slot, bytes, access)), environment(access),
      node(storage.node, NativeNodePreconstructionBinding{}, model_is_type, model_attach_scene, this) {
    node.scene_attachment.world_changed = model_world_changed;
    node.scene_attachment.remove_scene = model_remove_scene;
    try {
        access.nodes.scenes.bind(node.scene_attachment);
    } catch (...) {
        storage.model.~NativeModelTailStorage();
        storage.node.~NativeNodeStorage();
        throw;
    }
}
NativeModelOwner::~NativeModelOwner() {
    if (phase == Phase::prepared) {
        environment.nodes.scenes.forget_destroyed_binding(node.scene_attachment);
        storage.model.~NativeModelTailStorage();
        storage.node.~NativeNodeStorage();
    } else if (phase != Phase::dead) std::terminate();
}
void* construct_native_model_00b75030(NativeModelOwner& owner, const NativeString& name) {
    if (owner.phase != NativeModelOwner::Phase::prepared)
        throw std::logic_error("model constructor requires its unused prepared slot");
    owner.phase = NativeModelOwner::Phase::constructing;
    try {
        if (owner.environment.actual_names)
            construct_native_node_00b6f5a0(&owner.storage.node, NativeModelPool::slot_bytes,
                name, *owner.environment.actual_names);
        else
            construct_native_node_00b6f5a0(&owner.storage.node, NativeModelPool::slot_bytes,
                name, owner.environment.nodes.strings);
    } catch (...) {
        end_tail(owner);
        throw;
    }
    const auto minimum = owner.environment.constants.minimum_00ce4adc;
    publish_model_phase(owner);
    auto& model = owner.storage.model;
    model.retained_174 = nullptr;
    word(model.scalar_178, minimum);
    word(model.scalar_17c, owner.environment.constants.maximum_00ce4970);
    const auto negative_zero = owner.environment.constants.negative_zero_00d7a208;
    model.geometry_180 = nullptr;
    auto& node = owner.storage.node;
    pose_word(node, 0x18, negative_zero);
    pose_word(node, 0x1c, negative_zero);
    pose_word(node, 0x20, negative_zero);
    pose_word(node, 0x24, 0); pose_word(node, 0x28, 0); pose_word(node, 0x2c, 0);
    pose_word(node, 0x08, 0); pose_word(node, 0x0c, 0); pose_word(node, 0x10, 0);
    pose_word(node, 0x14, 0);
    owner.phase = NativeModelOwner::Phase::live;
    return &owner.storage.node;
}
void destroy_native_model_geometry_member_00b74f20(
    NativeModelTailStorage& model, NativeRenderActualOwners& owners) {
    release_then_clear(model.geometry_180, owners);
}
void destroy_native_model_00b750c0(NativeModelOwner& owner) {
    require_live(owner);
    publish_model_phase(owner);
    unsigned unwind_state = 1;
    try {
        release_then_clear(owner.storage.model.retained_174, owner.environment.retained_owners);
        unwind_state = 0; // A failing normal geometry release must not be retried.
        destroy_native_model_geometry_member_00b74f20(owner.storage.model,
            owner.environment.retained_owners);
    } catch (...) {
        try {
            if (unwind_state == 1)
                destroy_native_model_geometry_member_00b74f20(owner.storage.model,
                    owner.environment.retained_owners);
            finish_node(owner);
        } catch (...) { std::terminate(); }
        throw;
    }
    finish_node(owner); // Native state-1: no second base call if this throws.
}
void* delete_native_model_00b75290(NativeModelOwner& owner, std::uint32_t flags) {
    void* const slot = &owner.storage.node;
    destroy_native_model_00b750c0(owner);
    if (flags & 1u) owner.environment.pool_01090054.return_raw_slot_00b74750(slot);
    return slot;
}
void set_native_model_geometry_00b75170(NativeModelOwner& owner, std::uint32_t unused,
    void* geometry, float scalar_178, float scalar_17c) {
    static_cast<void>(unused);
    require_live(owner);
    auto& model = owner.storage.model;
    void* const previous = model.geometry_180;
    if (previous != geometry) {
        model.geometry_180 = geometry;
        if (geometry) retain_raw(geometry);
        if (previous) release_native_render_actual_owner(owner.environment.retained_owners, previous);
    }
    const auto sentinel = owner.environment.constants.unchanged_00d7a260;
    if (different_scalar(scalar_178, sentinel))
        std::memcpy(&model.scalar_178, &scalar_178, sizeof(scalar_178));
    if (different_scalar(scalar_17c, sentinel))
        std::memcpy(&model.scalar_17c, &scalar_17c, sizeof(scalar_17c));
}

NativeModelReference::NativeModelReference(NativeModelOwner& owner, NativeModelCompanionDisposal disposal)
    : RenderCommandReference(owner.storage.node.references_04), owner_(owner),
      runtime_(owner.environment.nodes.attachments), disposal_(disposal) {
    if (owner.phase != NativeModelOwner::Phase::live || !disposal.retire ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("model reference requires live owner and explicit companion retirement");
    require_terminal_slot(owner, 0, 0x00bd30e0u);
    require_terminal_slot(owner, 4, 0x00b75290u);
    runtime_.bind(*this);
}
NativeModelReference::~NativeModelReference() {
    if (phase_ != Phase::retired) std::terminate();
}
std::uint32_t NativeModelReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeModelReference*>(context)->owner_.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
GeneratedModelPointLightLinks& NativeModelReference::light_element(void* context,
    std::uint32_t index) noexcept {
    const auto& array = static_cast<NativeModelReference*>(context)->owner_.storage.node.point_lights_164;
    auto* light = array.begin[index];
    if (!light) std::terminate();
    return *light;
}
void NativeModelReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativeModelReference*>(context)->owner_.storage.node.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void NativeModelReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live)
        std::terminate();
    require_terminal_slot(owner_, 0x18, 0x00b6f310u);
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.storage.node.released_44, *this,
        {this, light_count, light_element, shrink_lights}});
}
void NativeModelReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeModelOwner::Phase::dead)
        std::terminate();
    require_terminal_slot(owner_, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes,
        owner_.node.scene_attachment, scene, recurse);
}
void NativeModelReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live)
        std::terminate();
    require_terminal_slot(owner_, 0, 0x00bd30e0u);
    require_terminal_slot(owner_, 4, 0x00b75290u);
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_model_00b75290(owner_, 1);
    runtime.unbind(*this); // host identity only; physical slot is already free
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
