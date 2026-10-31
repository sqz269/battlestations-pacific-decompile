#include "bsp/native_matrix_indexed_model_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/native_resource_instance_bones.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>
#include <mutex>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native model pool storage targets MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeMatrixIndexedModelPoolStorage) == 0x38);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeMatrixIndexedModelPoolStorage, first_free_slab_34) == 0x34);
constexpr std::size_t free_indices = 0x3280;
constexpr std::size_t free_count = 0x32c0;
static_assert(NativeMatrixIndexedModelPool::slot_bytes * 32 == free_indices);
static_assert(free_indices + 32 * sizeof(std::uint16_t) == free_count);
static_assert(NativeMatrixIndexedModelPool::slot_slab_index_offset + 4 ==
    NativeMatrixIndexedModelPool::slot_bytes);
constexpr std::uint32_t no_free_slab = 0xffffffffu;
struct SlabBytes { std::byte bytes[NativeMatrixIndexedModelPool::slab_bytes]; };

CRITICAL_SECTION* section(NativeMatrixIndexedModelPoolStorage& storage) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(storage.critical_section_0c);
}
template<class T> T read(const std::byte* address) noexcept {
    T value;
    std::memcpy(&value, address, sizeof(value));
    return value;
}
template<class T> void write(std::byte* address, T value) noexcept {
    std::memcpy(address, &value, sizeof(value));
}
void adjust_recursion(NativeMatrixIndexedModelPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte* initialize_slab_00b8fb50(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 32);
    for (std::uint32_t i = 0; i < 32; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2, static_cast<std::uint16_t>(31 - i));
        write<std::uint32_t>(bytes + i * 0x194 + 0x190, index);
    }
    return bytes;
}
std::byte** allocate_table(std::uint32_t capacity) {
    // Native 00BF55BE jumps to the same 00BF681B new-handler allocator.
    const auto native_bytes = capacity * 4u;
    return static_cast<std::byte**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, native_bytes,
        static_cast<std::size_t>(capacity) * sizeof(std::byte*)}));
}
NativeMatrixIndexedModelPool* static_pool_01090390;
NativeMatrixIndexedModelPool& require_static_pool() {
    if (!static_pool_01090390)
        throw std::logic_error("native 01090390 Native model pool has no actual storage binding");
    return *static_pool_01090390;
}
} // namespace

NativeMatrixIndexedModelPool::NativeMatrixIndexedModelPool(AllocatorListDomain& list,
    NativeMatrixIndexedModelPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Establish the host dispatch binding before native construction makes this
    // element visible; a real allocation new-handler may traverse that list.
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}

void NativeMatrixIndexedModelPool::initialize_00b90940() {
    allocator_list_.prepend_base_element(storage_.allocator_00);
    storage_.allocator_00.native_vtable_00 = native_vtable;
    unsigned unwind_state = 0;
    // Native constructor has these three unwind leaves. Allocation itself has
    // no such region. MSVC finally also preserves cleanup during Win32 unwind.
    __try {
        ::new (storage_.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage_));
        storage_.recursion_24 = 0;
        unwind_state = 1;
        storage_.slabs_28 = nullptr;
        storage_.slab_count_2c = 0;
        storage_.table_capacity_30 = 0;
        storage_.first_free_slab_34 = no_free_slab;
        unwind_state = 2;
        if (storage_.table_capacity_30 < 32) {
            storage_.table_capacity_30 = 32;
            auto** replacement = allocate_table(32);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            storage_.slabs_28 = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table_unwind_00b8fc70();
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* NativeMatrixIndexedModelPool::allocate_raw_slot_00b90ac0() {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    // Keep the native publication and failure semantics: no catch, scope guard,
    // rollback or automatic unlock if either real allocation throws.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_00b8fb50(raw, storage_.first_free_slab_34) : nullptr;
        if (storage_.slab_count_2c == storage_.table_capacity_30) {
            storage_.table_capacity_30 = storage_.table_capacity_30 * 2u + 2u;
            auto** replacement = allocate_table(storage_.table_capacity_30);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            storage_.slabs_28 = replacement;
        }
        ::new (storage_.slabs_28 + storage_.slab_count_2c) std::byte*(slab);
        ++storage_.slab_count_2c;
    }
    auto* slab = storage_.slabs_28[storage_.first_free_slab_34];
    const auto count = static_cast<std::uint16_t>(read<std::uint16_t>(slab + free_count) - 1u);
    write<std::uint16_t>(slab + free_count, count);
    auto* slot = slab + read<std::uint16_t>(slab + free_indices + count * 2u) * slot_bytes;
    if (count == 0) {
        auto index = storage_.first_free_slab_34 + 1u;
        storage_.first_free_slab_34 = no_free_slab;
        for (; index < storage_.slab_count_2c; ++index) {
            if (read<std::uint16_t>(storage_.slabs_28[index] + free_count) != 0) {
                storage_.first_free_slab_34 = index;
                break;
            }
        }
    }
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
    return slot;
}

std::uint32_t NativeMatrixIndexedModelPool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void NativeMatrixIndexedModelPool::return_raw_slot_00b90150(void* slot) {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // Assembly's signed magic multiply is exact signed division by 0x194.
    // Preserve the low-32-bit subtraction; valid slots belong to this slab.
    const auto offset_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slot)
        - reinterpret_cast<std::uintptr_t>(slab));
    std::int32_t offset;
    std::memcpy(&offset, &offset_bits, sizeof(offset));
    const auto index = static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(slot_bytes));
    const auto count = read<std::uint16_t>(slab + free_count);
    write<std::uint16_t>(slab + free_indices + count * 2u, index);
    // Native increments the memory word after the index store. Read it again
    // rather than imposing different alias behavior on malformed free stacks.
    write<std::uint16_t>(slab + free_count,
        static_cast<std::uint16_t>(read<std::uint16_t>(slab + free_count) + 1u));
    if (slab_index < storage_.first_free_slab_34) storage_.first_free_slab_34 = slab_index;
    adjust_recursion(storage_, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
}

void NativeMatrixIndexedModelPool::trim_empty_slabs_00b90a20() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (read<std::uint16_t>(slab + free_count) != 32) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native copies the replacement even when removing the last item.
        storage_.slabs_28[index] = storage_.slabs_28[storage_.slab_count_2c - 1u];
        --storage_.slab_count_2c;
        if (index < storage_.slab_count_2c) {
            auto* moved = storage_.slabs_28[index];
            for (std::uint32_t slot = 0; slot < 32; ++slot)
                write<std::uint32_t>(moved + slot * slot_bytes + slot_slab_index_offset, index);
        }
        // Retry this same index: the replacement can also be fully empty.
    }
    storage_.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
        if (read<std::uint16_t>(storage_.slabs_28[i] + free_count) != 0) {
            storage_.first_free_slab_34 = i;
            break;
        }
    }
}

void NativeMatrixIndexedModelPool::invoke_trim(void* context) {
    static_cast<NativeMatrixIndexedModelPool*>(context)->trim_empty_slabs_00b90a20();
}

void NativeMatrixIndexedModelPool::destroy_critical_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}

void NativeMatrixIndexedModelPool::free_table_unwind_00b8fc70() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    // Native leaf does not clear table/count/capacity fields.
}

void NativeMatrixIndexedModelPool::destroy_00b90090() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table_unwind_00b8fc70();
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

void bind_static_native_matrix_indexed_model_pool_01090390(NativeMatrixIndexedModelPool& pool) {
    if (static_pool_01090390 && static_pool_01090390 != &pool)
        throw std::logic_error("native 01090390 already has a different native-model-pool binding");
    static_pool_01090390 = &pool;
}

int initialize_static_native_matrix_indexed_model_pool_00cd8550(NativeMatrixIndexedModelPoolAtexit register_atexit) {
    if (!register_atexit) throw std::invalid_argument("CRT atexit registration is required");
    require_static_pool().initialize_00b90940();
    // Native returns CRT registration's result; failure does not undo the pool.
    return register_atexit(&destroy_static_native_matrix_indexed_model_pool_00ce0f20);
}

void destroy_static_native_matrix_indexed_model_pool_00ce0f20() {
    require_static_pool().destroy_00b90090();
}
void* allocate_native_matrix_indexed_model_slot_00b90c20() {
    return require_static_pool().allocate_raw_slot_00b90ac0();
}
void return_native_matrix_indexed_model_slot_00b90480(void* slot) {
    require_static_pool().return_raw_slot_00b90150(slot);
}
} // namespace bsp

namespace bsp {
namespace {
struct MatrixTypeDescriptor {
    std::uint32_t ids[4]; // own, model, node, root
    std::uint32_t native_name_address;
};
static_assert(sizeof(MatrixTypeDescriptor) == 0x14);
static_assert(sizeof(NativeNodeStorage) + sizeof(NativeModelTailStorage) == 0x184);
static_assert(0x184 + sizeof(NativeRenderPointerArrayStorage) == 0x190);
volatile std::uint8_t matrix_type_guard_01090341{};
volatile MatrixTypeDescriptor matrix_type_010903c8{};
TypeIdCounterLifetime* matrix_type_counter{};
const volatile ModelTypeDescriptor* matrix_base_type{};
constexpr std::uint32_t matrix_table = 0x00d63590;

const volatile std::uint32_t* matrix_current_table(
    const NativeMatrixIndexedModelOwner& owner, bool allow_base_phase = false) noexcept {
    const auto profile = owner.base.storage.node.vtable_00;
    if (profile == matrix_table) return owner.environment.vtable_00d63590;
    if (allow_base_phase && profile == 0x00d62de8u)
        return owner.environment.base.vtable_00d62de8;
    if (allow_base_phase && profile == 0x00d62c88u)
        return owner.environment.base.vtable_00d62c88;
    return nullptr;
}
void require_matrix_slot(const NativeMatrixIndexedModelOwner& owner,
    std::uint32_t offset, std::uint32_t expected, bool allow_base = false) {
    const auto* table = matrix_current_table(owner, allow_base);
    if (!table || table[offset / 4] != expected)
        throw std::logic_error("matrix-indexed model current virtual profile is unsupported");
}
void require_matrix_terminal_slot(const NativeMatrixIndexedModelOwner& owner,
    std::uint32_t offset, std::uint32_t expected, bool allow_base = false) noexcept {
    const auto* table = matrix_current_table(owner, allow_base);
    if (!table || table[offset / 4] != expected) std::terminate();
}
NativeRenderPointerArrayStorage& prepare_matrix_bones(void* slot, std::size_t bytes,
    NativeMatrixIndexedModelEnvironment& environment) {
    if (bytes < NativeMatrixIndexedModelPool::slot_bytes ||
        reinterpret_cast<std::uintptr_t>(environment.vtable_00d63590) != matrix_table ||
        environment.type_ids_010903c8 != native_matrix_indexed_model_type_ids_010903c8() ||
        &environment.base.types.storage().model_01090034 != matrix_base_type ||
        matrix_type_guard_01090341 == 0)
        throw std::invalid_argument("matrix-indexed owner requires194h storage and canonical initialized type/profile cells");
    // Default initialization begins only the header's C++ lifetime; no native
    // word is initialized until the B91220 continuation runs.
    return *::new (static_cast<std::byte*>(slot) + 0x184) NativeRenderPointerArrayStorage;
}
bool matrix_is_type(SceneAttachmentRuntime&, SceneNodeAttachment& binding,
    std::uint32_t token) {
    auto& owner = *static_cast<NativeMatrixIndexedModelOwner*>(binding.context);
    require_matrix_slot(owner, 0x0c, 0x00b91250);
    // B91250 compares exactly four live cells; no guessed base fallback.
    return native_matrix_indexed_model_is_type_00b91250(
        token, owner.environment.type_ids_010903c8) != 0;
}
void matrix_attach_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeMatrixIndexedModelOwner*>(binding.context);
    require_matrix_slot(owner, 0x50, 0x00b6ed80);
    set_node_scene_00b6ed80(runtime, binding, scene, recurse);
}
void matrix_remove_scene(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding,
    SceneResource* scene, bool recurse) {
    auto& owner = *static_cast<NativeMatrixIndexedModelOwner*>(binding.context);
    require_matrix_slot(owner, 0x54, 0x00b6ee10);
    remove_native_node_scene_00b6ee10(runtime, binding, scene, recurse);
}
void matrix_world_changed(SceneAttachmentRuntime& runtime, SceneNodeAttachment& binding) {
    auto& owner = *static_cast<NativeMatrixIndexedModelOwner*>(binding.context);
    require_matrix_slot(owner, 0x40, 0x00b6dbe0);
    native_node_world_changed_00b6dbe0(runtime, binding);
}
void publish_matrix_callbacks(NativeMatrixIndexedModelOwner& owner) noexcept {
    auto& binding = owner.base.node.scene_attachment;
    binding.context = &owner;
    binding.is_type = matrix_is_type;
    binding.attach_scene = matrix_attach_scene;
    binding.remove_scene = matrix_remove_scene;
    binding.world_changed = matrix_world_changed;
}
void finish_matrix_base(NativeMatrixIndexedModelOwner& owner) {
    // B750C0 stamps its base profile/callbacks before any outgoing operation.
    // Restore only the corresponding host context before entering that body.
    owner.base.node.scene_attachment.context = &owner.base;
    try { destroy_native_model_00b750c0(owner.base); }
    catch (...) { owner.phase = NativeMatrixIndexedModelOwner::Phase::dead; throw; }
    owner.phase = NativeMatrixIndexedModelOwner::Phase::dead;
}
}

void initialize_native_matrix_indexed_model_type_00cd8570(TypeIdCounterLifetime& counter,
    ModelTypeBootstrap& model) {
    const auto storage = model.storage();
    if ((matrix_type_counter && matrix_type_counter != &counter) ||
        (matrix_base_type && matrix_base_type != &storage.model_01090034))
        throw std::logic_error("matrix-indexed type IDs require the same application counter/model storage");
    matrix_type_counter = &counter;
    matrix_base_type = &storage.model_01090034;
    if (matrix_type_guard_01090341 == 0) {
        matrix_type_guard_01090341 = 1;
        matrix_type_010903c8.native_name_address = 0x00d63574;
        model.initialize_00b74f90(storage.model_01090034);
        // CD8594..CD85B0 captures all three parents before any destination store.
        const auto model_id = storage.model_01090034.own_id;
        const auto node_id = storage.model_01090034.node_id;
        const auto root_id = storage.model_01090034.root_id;
        matrix_type_010903c8.ids[1] = model_id;
        matrix_type_010903c8.ids[2] = node_id;
        matrix_type_010903c8.ids[3] = root_id;
        volatile auto* actual_counter = counter.get_006fac20();
        const auto own_id = actual_counter->next_id_04;
        actual_counter->next_id_04 = own_id + 1u;
        matrix_type_010903c8.ids[0] = own_id;
    }
}
const volatile std::uint32_t* native_matrix_indexed_model_type_ids_010903c8() noexcept {
    return matrix_type_010903c8.ids;
}

std::uint8_t native_matrix_indexed_model_is_type_00b91250(
    std::uint32_t token, const volatile std::uint32_t* actual_type_ids_010903c8) {
    for (unsigned i = 0; i != 4; ++i)
        if (actual_type_ids_010903c8[i] == token) return std::uint8_t{1};
    return std::uint8_t{0};
}

NativeMatrixIndexedModelOwner::NativeMatrixIndexedModelOwner(void* slot, std::size_t bytes,
    NativeMatrixIndexedModelEnvironment& access)
    : base(slot, bytes, access.base), environment(access),
      bones(prepare_matrix_bones(slot, bytes, access)) {}

NativeMatrixIndexedModelOwner::~NativeMatrixIndexedModelOwner() {
    if (phase == Phase::prepared) bones.~NativeRenderPointerArrayStorage();
    else if (phase != Phase::dead) std::terminate();
}

void* construct_native_matrix_indexed_model_00b91220(NativeMatrixIndexedModelOwner& owner,
    const void* actual_name_header, const NativeNodeRawConstants& constants) {
    if (owner.phase != NativeMatrixIndexedModelOwner::Phase::prepared)
        throw std::logic_error("matrix-indexed construction requires a prepared194h slot");
    try { construct_native_model_00b75030(owner.base, actual_name_header, constants); }
    catch (...) {
        if (owner.base.phase == NativeModelOwner::Phase::dead) {
            owner.bones.~NativeRenderPointerArrayStorage();
            owner.phase = NativeMatrixIndexedModelOwner::Phase::dead;
        }
        throw;
    }
    owner.base.storage.node.vtable_00 = matrix_table; // B9122F
    owner.bones.data_00 = nullptr;                    // B91235
    owner.bones.count_04 = 0;                         // B9123B
    owner.bones.capacity_08 = 0;                      // B91241
    publish_matrix_callbacks(owner);
    owner.phase = NativeMatrixIndexedModelOwner::Phase::live;
    return &owner.base.storage.node;
}

void destroy_native_matrix_indexed_model_00b91280(NativeMatrixIndexedModelOwner& owner) {
    if (owner.phase != NativeMatrixIndexedModelOwner::Phase::live)
        throw std::logic_error("matrix-indexed destruction requires its live owner");
    owner.phase = NativeMatrixIndexedModelOwner::Phase::destroying;
    try {
        // B90630 is the normalized-identical B1C770 specialization already
        // used by B90E30. It changes count only on shrink; no node release.
        resize_native_model_bone_nodes_00b90e30(&owner.base.storage.node, 0);
        singleton_lifetime_free(owner.bones.data_00); // reload after resize
    } catch (...) {
        owner.bones.~NativeRenderPointerArrayStorage();
        try { finish_matrix_base(owner); } catch (...) { std::terminate(); }
        throw;
    }
    owner.bones.~NativeRenderPointerArrayStorage();
    finish_matrix_base(owner); // B912CA; do not repeat this if it throws.
}

void* delete_native_matrix_indexed_model_00b915a0(NativeMatrixIndexedModelOwner& owner,
    std::uint32_t flags) {
    void* const slot = &owner.base.storage.node;
    destroy_native_matrix_indexed_model_00b91280(owner);
    if (flags & 1u) owner.environment.pool_01090390.return_raw_slot_00b90150(slot);
    return slot;
}

NativeMatrixIndexedModelReference::NativeMatrixIndexedModelReference(
    NativeMatrixIndexedModelOwner& owner, NativeMatrixIndexedModelCompanionDisposal disposal)
    : RenderCommandReference(owner.base.storage.node.references_04), owner_(owner),
      runtime_(owner.environment.base.nodes.attachments), disposal_(disposal) {
    if (owner.phase != NativeMatrixIndexedModelOwner::Phase::live || !disposal.retire ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("matrix-indexed reference requires live owner and explicit retirement");
    require_matrix_terminal_slot(owner, 0, 0x00bd30e0);
    require_matrix_terminal_slot(owner, 4, 0x00b915a0);
    runtime_.bind(*this);
}
NativeMatrixIndexedModelReference::~NativeMatrixIndexedModelReference() {
    if (phase_ != Phase::retired) std::terminate();
}
std::uint32_t NativeMatrixIndexedModelReference::light_count(void* context) noexcept {
    const auto& array = static_cast<NativeMatrixIndexedModelReference*>(context)
        ->owner_.base.storage.node.point_lights_164;
    if (array.count < 0 || array.capacity < array.count || (array.count && !array.begin))
        std::terminate();
    return static_cast<std::uint32_t>(array.count);
}
void NativeMatrixIndexedModelReference::remove_light_backlink(void* context,
    std::uint32_t index, CameraTransform&) noexcept {
    auto& reference = *static_cast<NativeMatrixIndexedModelReference*>(context);
    auto& node = reference.owner_.base.storage.node;
    auto& light = reference.owner_.environment.base.nodes.point_lights.light(node.point_lights_164.begin[index]);
    remove_native_point_light_backlink_00b7c1a0(light, node);
}
void NativeMatrixIndexedModelReference::shrink_lights(void* context) noexcept {
    auto& array = static_cast<NativeMatrixIndexedModelReference*>(context)
        ->owner_.base.storage.node.point_lights_164;
    shrink_native_node_point_lights_to_zero_00b6ec70(array);
}
void NativeMatrixIndexedModelReference::release_model_virtual18_00b6f310() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeMatrixIndexedModelOwner::Phase::live)
        std::terminate();
    require_matrix_terminal_slot(owner_, 0x18, 0x00b6f310);
    release_node_logical_00b6f310({runtime_, owner_.base.node.transform,
        owner_.base.storage.node.released_44, *this,
        {this, light_count, remove_light_backlink, shrink_lights}});
}
void NativeMatrixIndexedModelReference::remove_scene_virtual54(SceneResource* scene,
    bool recurse) noexcept {
    if (phase_ == Phase::retired || owner_.phase == NativeMatrixIndexedModelOwner::Phase::dead)
        std::terminate();
    require_matrix_terminal_slot(owner_, 0x54, 0x00b6ee10, true);
    remove_native_node_scene_00b6ee10(owner_.environment.base.nodes.scenes,
        owner_.base.node.scene_attachment, scene, recurse);
}
void NativeMatrixIndexedModelReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || owner_.phase != NativeMatrixIndexedModelOwner::Phase::live)
        std::terminate();
    require_matrix_terminal_slot(owner_, 0, 0x00bd30e0);
    require_matrix_terminal_slot(owner_, 4, 0x00b915a0);
    phase_ = Phase::destroying;
    auto& runtime = runtime_;
    const auto disposal = disposal_;
    delete_native_matrix_indexed_model_00b915a0(owner_, 1);
    runtime.unbind(*this); // host identity only, as in the existing base reference
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}

namespace game {
namespace {
struct MatrixPoolProcess {
    NativeMatrixIndexedModelPoolStorage storage{};
    NativeMatrixIndexedModelPool pool;
    std::mutex mutex;
    enum class Startup { unattempted, returned, threw };
    Startup startup{Startup::unattempted};
    int status{};
    MatrixPoolProcess() : pool(game_native_physical_pool_process()
        .allocator_list_domain_00e188b4(), storage) {}
};
MatrixPoolProcess& matrix_pool_process() {
    static MatrixPoolProcess process;
    return process;
}
}
int initialize_matrix_indexed_model_pool_once_00cd8550() {
    auto& process = matrix_pool_process();
    std::lock_guard<std::mutex> lock(process.mutex);
    if (process.startup == MatrixPoolProcess::Startup::threw)
        throw std::logic_error("matrix-indexed model pool startup cannot retry a thrown attempt");
    if (process.startup == MatrixPoolProcess::Startup::unattempted) {
        process.startup = MatrixPoolProcess::Startup::threw;
        bind_static_native_matrix_indexed_model_pool_01090390(process.pool);
        process.status = initialize_static_native_matrix_indexed_model_pool_00cd8550();
        process.startup = MatrixPoolProcess::Startup::returned;
    }
    return process.status;
}
NativeMatrixIndexedModelPool& matrix_indexed_model_pool_01090390() {
    auto& process = matrix_pool_process();
    std::lock_guard<std::mutex> lock(process.mutex);
    if (process.startup != MatrixPoolProcess::Startup::returned)
        throw std::logic_error("matrix-indexed model pool requires explicit returned startup");
    return process.pool;
}
}
} // namespace bsp
