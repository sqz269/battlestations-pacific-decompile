#include "bsp/native_model_owner.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_model_resource_storage.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native model ownership requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeModelTailStorage) == 0x10);
static_assert(std::is_trivially_destructible_v<NativeNodeStorage>);
static_assert(std::is_trivially_destructible_v<NativeModelTailStorage>);
static_assert(offsetof(NativeModelTailStorage, retained_174) == 0);
static_assert(offsetof(NativeModelTailStorage, scalar_178) == 4);
static_assert(offsetof(NativeModelTailStorage, scalar_17c) == 8);
static_assert(offsetof(NativeModelTailStorage, geometry_180) == 12);
static_assert(sizeof(NativeNodeStorage) + sizeof(NativeModelTailStorage) == 0x184);
static_assert(NativeModelPool::slot_slab_index_offset == 0x184);
static_assert(NativeModelPool::slot_bytes == 0x188);

// No public receipt/progress field can create or revive this raw-return fact.
// Only the real raw continuation and existing end/failure paths call these.
class NativeModelRawFactsAccess final {
public:
    static void entered(NativeModelOwner& owner) {
        auto& facts = owner.raw_facts_;
        if (facts.entered || facts.returned || facts.destruction_entered ||
            facts.typed_end_entered || facts.failed)
            throw std::logic_error("raw Model constructor fact cannot replay");
        facts.entered = true;
    }
    static void returned(NativeModelOwner& owner,
        const NativeNodeRawConstants& constants) noexcept {
        auto& facts = owner.raw_facts_;
        facts.node = &owner.storage.node;
        facts.tail = &owner.storage.model;
        facts.count = &owner.storage.node.references_04;
        facts.environment = &owner.environment;
        facts.binding = owner.raw_lifetime;
        facts.constants = &constants;
        facts.sentinel = &owner.environment.constants.unchanged_00d7a260;
        facts.returned = true;
    }
    static void destroying(NativeModelOwner& owner) noexcept {
        owner.raw_facts_.destruction_entered = true;
    }
    static void ended(NativeModelOwner& owner) noexcept {
        owner.raw_facts_.typed_end_entered = true;
    }
    static void failed(NativeModelOwner& owner) noexcept {
        owner.raw_facts_.failed = true;
    }
};

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
void require_raw_terminal_fresh(const NativeModelRawLifetimeBinding& raw) {
    const auto& a = raw.terminal;
    if (raw.host.typed_storage_ended || raw.host.failed || raw.host.stopped_site || raw.host.exception ||
        a.started || a.complete || a.scalar_started || a.scalar_complete ||
        a.flags_read || a.pool_returned || a.node.started || a.node.complete ||
        a.node.base_cleanup_completed || a.geometry_unwind.started || a.geometry_unwind.complete)
        throw std::logic_error("canonical raw Model requires fresh retained terminal backing");
}
struct Region { const void* pointer; std::size_t bytes; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(left.pointer));
    const auto r = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(right.pointer));
    return l + left.bytes <= r || r + right.bytes <= l;
}
void validate_raw_binding(void* slot, NativeModelEnvironment& env,
    NativeModelRawLifetimeBinding& raw) {
    auto& c = raw.storage;
    if (env.actual_names || !env.nodes.uses_raw_name_pool() ||
        &env.nodes.require_raw_name_pool() != &c.node.strings ||
        &env.pool_01090054 != &c.pool_01090054 ||
        &env.retained_owners != static_cast<NativeRenderActualOwners*>(&c.node.scenes.owners) ||
        &env.retained_owners != &c.node.trees.owners ||
        &env.nodes.point_lights != &c.node.trees.point_lights ||
        &c.node.scenes.decrement_00ce2220 != &c.node.trees.decrement_00ce2220 ||
        &env.constants.minimum_00ce4adc != &c.constants.negative_bound_00ce4adc ||
        &env.constants.maximum_00ce4970 != &c.constants.positive_bound_00ce4970 ||
        &env.constants.negative_zero_00d7a208 != &c.negative_zero_00d7a208)
        throw std::invalid_argument("canonical raw Model requires SAME pool/node/string/owner/import/constant providers");
    require_raw_terminal_fresh(raw);
    const Region payload{slot, NativeModelPool::slot_bytes};
    const Region records[] = {{&raw, sizeof(raw)}, {&c, sizeof(c)},
        {&c.node, sizeof(c.node)}, {&c.node.scenes, sizeof(c.node.scenes)},
        {&c.node.trees, sizeof(c.node.trees)}, {&raw.terminal_frame, sizeof(raw.terminal_frame)},
        {&raw.terminal, sizeof(raw.terminal)}, {&raw.host, sizeof(raw.host)}};
    for (const auto& record : records)
        if (!disjoint(payload, record))
            throw std::invalid_argument("canonical raw Model payload must be disjoint from retained lifetime metadata");
    const Region mutable_records[] = {{&raw.terminal_frame, sizeof(raw.terminal_frame)},
        {&raw.terminal, sizeof(raw.terminal)}, {&raw.host, sizeof(raw.host)}};
    for (std::size_t i = 0; i != 3; ++i)
        for (std::size_t j = i + 1; j != 3; ++j)
            if (!disjoint(mutable_records[i], mutable_records[j]))
                throw std::invalid_argument("canonical raw Model terminal frames and diagnostics must be disjoint");
}
NativeModelStorageView prepare_bound_storage(void* slot, std::size_t bytes,
    NativeModelEnvironment& env, NativeModelRawLifetimeBinding* raw) {
    if (raw) validate_raw_binding(slot, env, *raw);
    return prepare_storage(slot, bytes, env);
}
void require_legacy_transport(const NativeModelOwner& owner) {
    if (owner.raw_lifetime)
        throw std::logic_error("raw Model normal calls require explicit throwing raw transport");
}
void require_legacy_virtual(const NativeModelOwner& owner, std::uint32_t site) noexcept {
    if (auto* raw = owner.raw_lifetime) {
        // Wrong C++ transport is an integration violation, not a newly admitted
        // raw Native failure route. Actual Page normal calls use throwing APIs.
        raw->host.failed = true;
        raw->host.stopped_site = site;
        NativeModelRawFactsAccess::failed(owner);
        std::terminate();
    }
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
    require_legacy_transport(owner);
    require_slot(owner, 0x50, 0x00b6ed80u, true);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void model_remove_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    require_legacy_transport(owner);
    require_slot(owner, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void model_world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding) {
    auto& owner = *static_cast<NativeModelOwner*>(binding.context);
    require_legacy_transport(owner);
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
    NativeModelRawFactsAccess::ended(owner);
    owner.storage.model.~NativeModelTailStorage();
    owner.environment.nodes.scenes.forget_destroyed_binding(owner.node.scene_attachment);
    owner.phase = NativeModelOwner::Phase::dead;
}
void end_raw_typed_storage(void* context) noexcept {
    auto& owner = *static_cast<NativeModelOwner*>(context);
    auto& raw = *owner.raw_lifetime;
    if (raw.host.typed_storage_ended) std::terminate();
    // Full raw B6F440 already performed all Native cleanup/profile stores.
    // These existing typed extents/association end without further Native work.
    NativeModelRawFactsAccess::ended(owner);
    owner.storage.node.~NativeNodeStorage();
    end_tail(owner);
    raw.host.typed_storage_ended = true;
}
void record_raw_failure(NativeModelOwner& owner) noexcept {
    NativeModelRawFactsAccess::failed(owner);
    auto& raw = *owner.raw_lifetime;
    raw.host.failed = true;
    raw.host.stopped_site = raw.terminal.active_call_site;
    raw.host.exception = std::current_exception();
}
void finish_node(NativeModelOwner& owner) {
    try {
        if (owner.environment.actual_names)
            destroy_native_node_00b6f440(owner.environment.nodes, owner.node,
                *owner.environment.actual_names);
        else if (owner.environment.nodes.uses_raw_name_pool())
            destroy_native_node_00b6f440(owner.environment.nodes, owner.node,
                owner.environment.nodes.require_raw_name_pool());
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
    : NativeModelOwner(slot, bytes, access, static_cast<NativeModelRawLifetimeBinding*>(nullptr)) {}
NativeModelOwner::NativeModelOwner(void* slot, std::size_t bytes, NativeModelEnvironment& access,
    NativeModelRawLifetimeBinding& raw)
    : NativeModelOwner(slot, bytes, access, &raw) {}
NativeModelOwner::NativeModelOwner(void* slot, std::size_t bytes, NativeModelEnvironment& access,
    NativeModelRawLifetimeBinding* raw)
    : storage(prepare_bound_storage(slot, bytes, access, raw)), environment(access),
      node(storage.node, NativeNodePreconstructionBinding{}, model_is_type, model_attach_scene, this),
      raw_lifetime(raw) {
    node.scene_attachment.world_changed = model_world_changed;
    node.scene_attachment.remove_scene = model_remove_scene;
    try {
        access.nodes.scenes.bind(node.scene_attachment);
    } catch (...) {
        NativeModelRawFactsAccess::ended(*this);
        storage.model.~NativeModelTailStorage();
        storage.node.~NativeNodeStorage();
        throw;
    }
}
NativeModelOwner::~NativeModelOwner() {
    if (phase == Phase::prepared) {
        NativeModelRawFactsAccess::ended(*this);
        environment.nodes.scenes.forget_destroyed_binding(node.scene_attachment);
        storage.model.~NativeModelTailStorage();
        storage.node.~NativeNodeStorage();
    } else if (phase != Phase::dead) std::terminate();
}
void* construct_native_model_00b75030(NativeModelOwner& owner, const NativeString& name) {
    if (owner.environment.raw_name_services) {
        // NativeString is the source caller's SAME live8B header, not a copied
        // name. The raw overload enforces phase, raw cleanup and bound cells.
        return construct_native_model_00b75030(owner, static_cast<const void*>(&name),
            owner.environment.raw_name_services->constants);
    }
    if (owner.phase != NativeModelOwner::Phase::prepared)
        throw std::logic_error("model constructor requires its unused prepared slot");
    SizedStoragePool* name_pool = owner.environment.actual_names ? nullptr :
        &owner.environment.nodes.require_semantic_name_pool();
    owner.phase = NativeModelOwner::Phase::constructing;
    try {
        if (owner.environment.actual_names)
            construct_native_node_00b6f5a0(&owner.storage.node, NativeModelPool::slot_bytes,
                name, *owner.environment.actual_names);
        else
            construct_native_node_00b6f5a0(&owner.storage.node, NativeModelPool::slot_bytes,
                name, *name_pool);
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
void* construct_native_model_00b75030(NativeModelOwner& owner, const void* actual_name_header,
    const NativeNodeRawConstants& constants) {
    if (owner.phase != NativeModelOwner::Phase::prepared)
        throw std::logic_error("model constructor requires its unused prepared slot");
    auto& name_pool = owner.environment.nodes.require_raw_name_pool();
    if (owner.environment.actual_names ||
        &constants.positive_bound_00ce4970 != &owner.environment.constants.maximum_00ce4970 ||
        &constants.negative_bound_00ce4adc != &owner.environment.constants.minimum_00ce4adc)
        throw std::logic_error("raw model requires its persistent raw cleanup and shared actual bound cells");
    NativeModelRawFactsAccess::entered(owner);
    owner.phase = NativeModelOwner::Phase::constructing;
    try {
        construct_native_node_00b6f5a0(&owner.storage.node, NativeModelPool::slot_bytes,
            actual_name_header, name_pool, constants); // B75038; no copied header
    } catch (...) {
        NativeModelRawFactsAccess::failed(owner);
        end_tail(owner); // B6F5A0 already consumed its own prefix cleanup states.
        throw; // Caller still owns the physical model slot.
    }
    // Keep this continuation literal, including volatile load/alias order.
    const auto minimum = owner.environment.constants.minimum_00ce4adc; // B7503D
    publish_model_phase(owner); // B75045
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
    NativeModelRawFactsAccess::returned(owner, constants);
    return &owner.storage.node;
}
void destroy_native_model_geometry_member_00b74f20(
    NativeModelTailStorage& model, NativeRenderActualOwners& owners) {
    release_then_clear(model.geometry_180, owners);
}
void destroy_native_model_00b750c0(NativeModelOwner& owner) {
    require_live(owner);
    if (auto* raw = owner.raw_lifetime) {
        require_raw_terminal_fresh(*raw);
        NativeModelRawFactsAccess::destroying(owner);
        try {
            destroy_native_model_storage_00b750c0(&owner.storage.node, raw->terminal_frame,
                raw->storage, raw->terminal, {&owner, end_raw_typed_storage});
        } catch (...) { record_raw_failure(owner); throw; }
        return;
    }
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
    if (auto* raw = owner.raw_lifetime) {
        require_live(owner);
        require_raw_terminal_fresh(*raw);
        raw->terminal_frame.scalar_flags_argument = flags;
        return delete_native_model_raw_00b75290(owner, raw->terminal_frame.scalar_flags_argument);
    }
    void* const slot = &owner.storage.node;
    destroy_native_model_00b750c0(owner);
    if (flags & 1u) owner.environment.pool_01090054.return_raw_slot_00b74750(slot);
    return slot;
}
void* delete_native_model_raw_00b75290(NativeModelOwner& owner, const volatile std::uint32_t& flags) {
    require_live(owner);
    auto* raw = owner.raw_lifetime;
    if (!raw) throw std::logic_error("raw Model scalar requires its immutable raw lifetime binding");
    require_raw_terminal_fresh(*raw);
    NativeModelRawFactsAccess::destroying(owner);
    try {
        return delete_native_model_storage_00b75290(&owner.storage.node, flags, raw->terminal_frame,
            raw->storage, raw->terminal, {&owner, end_raw_typed_storage});
    } catch (...) { record_raw_failure(owner); throw; }
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
void NativeModelReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<NativeModelReference*>(context);
    auto& node = reference.owner_.storage.node;
    auto& light = reference.owner_.environment.nodes.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void NativeModelReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativeModelReference*>(context)->owner_.storage.node.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void NativeModelReference::release_model_virtual18_00b6f310() noexcept {
    require_legacy_virtual(owner_, 0x00b6f310);
    if (phase_ != Phase::bound || owner_.phase != NativeModelOwner::Phase::live)
        std::terminate();
    require_terminal_slot(owner_, 0x18, 0x00b6f310u);
    release_node_logical_00b6f310({runtime_, owner_.node.transform,
        owner_.storage.node.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
}
void NativeModelReference::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    require_legacy_virtual(owner_, 0x00b6ee10);
    if (phase_ == Phase::retired || owner_.phase == NativeModelOwner::Phase::dead)
        std::terminate();
    require_terminal_slot(owner_, 0x54, 0x00b6ee10u, true);
    remove_native_node_scene_00b6ee10(owner_.environment.nodes.scenes,
        owner_.node.scene_attachment, scene, recurse);
}

namespace {
NativeModelRawLifetimeBinding& require_raw_reference(NativeModelReference& reference) {
    auto& owner = reference.model_owner();
    auto* raw = owner.raw_lifetime;
    if (!raw || owner.phase != NativeModelOwner::Phase::live ||
        raw->host.failed || raw->host.typed_storage_ended || raw->terminal.node.base_cleanup_completed)
        throw std::logic_error("raw Model normal call requires its SAME still-live canonical owner");
    if (owner.environment.actual_names ||
        raw->storage.node.scenes.owners.find(&owner.storage.node) != &reference ||
        static_cast<void*>(&reference.reference_count) != static_cast<void*>(&owner.storage.node.references_04))
        throw std::logic_error("raw Model normal call lost its SAME canonical actual+04 binding");
    return *raw;
}
void require_call_regions(void* actual, Region frame, Region acquired,
    const NativeModelRawLifetimeBinding& raw) {
    const Region payload{actual, NativeModelPool::slot_bytes};
    if (!disjoint(payload, frame) || !disjoint(payload, acquired) || !disjoint(frame, acquired))
        throw std::invalid_argument("raw Model normal call requires disjoint persistent invocation backing");
    for (const Region terminal : {Region{&raw.terminal_frame, sizeof(raw.terminal_frame)},
        Region{&raw.terminal, sizeof(raw.terminal)}, Region{&raw.host, sizeof(raw.host)}})
        if (!disjoint(frame, terminal) || !disjoint(acquired, terminal))
            throw std::invalid_argument("raw Model normal invocation cannot borrow its terminal scratch");
}
} // namespace

void NativeModelReference::release_raw_virtual18_00b6f310(std::uint32_t captured_target,
    NativeNodeTreeRetirementFrame& frame, NativeNodeTreeRetirementAcquired& acquired) {
    if ((phase_ != Phase::bound && phase_ != Phase::destroying) || acquired.started || acquired.complete)
        throw std::logic_error("raw Model current18 requires a fresh retained invocation");
    auto& raw = require_raw_reference(*this);
    if (captured_target != 0x00b6f310) throw std::logic_error("raw Model current18 target is not genuine B6F310");
    require_slot(owner_, 0x18, captured_target, true);
    void* const actual = &owner_.storage.node;
    auto& trees = raw.storage.node.trees;
    require_call_regions(actual, {&frame, sizeof(frame)}, {&acquired, sizeof(acquired)}, raw);
    release_native_node_tree_00b6f310(actual, frame, trees, acquired);
    // No self/binding access: the terminal callback may have disposed this.
}
void NativeModelReference::remove_raw_scene_virtual54_00b6ee10(std::uint32_t captured_target,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentAcquired& acquired) {
    if ((phase_ != Phase::bound && phase_ != Phase::destroying) || acquired.started || acquired.complete)
        throw std::logic_error("raw Model current54 requires a fresh retained invocation");
    auto& raw = require_raw_reference(*this);
    if (captured_target != 0x00b6ee10) throw std::logic_error("raw Model current54 target is not genuine B6EE10");
    require_slot(owner_, 0x54, captured_target, true);
    void* const actual = &owner_.storage.node;
    auto& scenes = raw.storage.node.scenes;
    require_call_regions(actual, {&frame, sizeof(frame)}, {&acquired, sizeof(acquired)}, raw);
    remove_native_node_scene_storage_00b6ee10(actual, frame, scenes, acquired);
    // The Page-owned frame/provider records an escaping normal exception.
}

class NativeModelReference::RawDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    explicit RawDeleteCalls(NativeModelReference& reference) noexcept : reference_(reference) {}
    void delete_vslot04(void* actual, std::uint32_t profile, std::uint32_t flags) override {
        auto& owner = reference_.owner_;
        if (actual != &owner.storage.node || profile != model_table || flags != 1)
            throw std::logic_error("canonical raw Model terminal requires SAME actual/current profile/flags1 call");
        require_slot(owner, 4, 0x00b75290);
        auto& raw = *owner.raw_lifetime;
        require_raw_terminal_fresh(raw);
        raw.terminal_frame.scalar_flags_argument = flags;
        reference_.delete_raw_scalar_00b75290(raw.terminal_frame.scalar_flags_argument);
    }
private:
    NativeModelReference& reference_;
};

void NativeModelReference::delete_raw_scalar_00b75290(const volatile std::uint32_t& flags) {
    if (phase_ != Phase::bound || !owner_.raw_lifetime || owner_.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("canonical raw Model scalar requires its same live bound reference");
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    phase_ = Phase::destroying;
    try { delete_native_model_raw_00b75290(owner_, flags); }
    catch (...) {
        phase_ = Phase::failed;
        record_raw_failure(owner_);
        throw; // No failed-path lifetime/registry unbind or record disposal.
    }
    runtime.unbind(*this); // identity metadata only, after possible physical return
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this); // flags0 callback retains original physical-slot receipt
}
void NativeModelReference::release_zero_references() noexcept {
    if (owner_.raw_lifetime) {
        try {
            auto& raw = require_raw_reference(*this);
            if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
                throw std::logic_error("canonical raw Model terminal requires SAME observed-zero count");
            require_raw_terminal_fresh(raw);
            require_slot(owner_, 0, 0x00bd30e0);
            RawDeleteCalls calls(*this);
            invoke_native_ref_counted_delete_00bd30e0(&owner_.storage.node, calls);
        } catch (...) {
            record_raw_failure(owner_);
            // Existing canonical terminal boundary only. Normal raw18/54 do
            // not enter this catch and preserve their throwing Page transport.
            std::terminate();
        }
        return; // this may have been disposed; no further self/actual access
    }
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
NativeModelRawLifetimeBinding& NativeModelReference::require_checked_raw_live() {
    const auto& facts = owner_.raw_facts_;
    if (phase_ != Phase::bound || !facts.entered || !facts.returned ||
        facts.destruction_entered || facts.typed_end_entered || facts.failed ||
        owner_.phase != NativeModelOwner::Phase::live || !owner_.raw_lifetime)
        throw std::logic_error("checked raw Model requires its private still-live raw return");
    auto& raw = *owner_.raw_lifetime;
    auto* const actual = &owner_.storage.node;
    if (facts.node != actual || facts.tail != &owner_.storage.model ||
        facts.count != &actual->references_04 || facts.environment != &owner_.environment ||
        facts.binding != &raw || facts.constants != &raw.storage.constants ||
        facts.sentinel != &owner_.environment.constants.unchanged_00d7a260)
        throw std::logic_error("checked raw Model lost its exact constructor/provider identities");
    validate_raw_binding(actual, owner_.environment, raw);
    if (raw.storage.node.scenes.owners.find(actual) != this ||
        static_cast<void*>(&reference_count) != static_cast<void*>(facts.count))
        throw std::logic_error("checked raw Model requires its original SAME canonical/count");
    NativeModelGeometryKernelAccess::require_live_model_slot(actual, raw.storage.pool_01090054);
    if (reference_count.load(std::memory_order_relaxed) <= 0 || actual->vtable_00 != model_table)
        throw std::logic_error("checked raw Model requires its positive current live allocation");
    return raw;
}

void NativeModelReference::record_checked_raw_failure(std::uint32_t active_site) noexcept {
    record_raw_failure(owner_);
    owner_.raw_lifetime->host.stopped_site = active_site;
}

void* NativeModelReference::capture_current_raw_scene_170() {
    (void)require_checked_raw_live();
    SceneResource* captured;
    _ReadWriteBarrier();
    std::memcpy(&captured, &owner_.storage.node.scene_170, 4);
    _ReadWriteBarrier();
    return static_cast<void*>(captured); // opaque actual identity, never semantic follow
}
} // namespace bsp
