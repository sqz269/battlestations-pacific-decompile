#include "bsp/native_mesh_section.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native mesh section pool storage targets MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeMeshSectionPoolStorage) == 0x38);
static_assert(offsetof(NativeMeshSectionPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeMeshSectionPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeMeshSectionPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeMeshSectionPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeMeshSectionPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeMeshSectionPoolStorage, first_free_slab_34) == 0x34);
constexpr std::size_t free_indices = 0x1900;
constexpr std::size_t free_count = 0x1980;
constexpr std::uint32_t no_free_slab = 0xffffffffu;
struct SlabBytes { std::byte bytes[NativeMeshSectionPool::slab_bytes]; };

CRITICAL_SECTION* section(NativeMeshSectionPoolStorage& storage) noexcept {
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
void adjust_recursion(NativeMeshSectionPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte* initialize_slab_00b85650(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 64);
    for (std::uint32_t i = 0; i < 64; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2, static_cast<std::uint16_t>(63 - i));
        write<std::uint32_t>(bytes + i * 0x64 + 0x60, index);
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
} // namespace

NativeMeshSectionPool::NativeMeshSectionPool(AllocatorListDomain& list,
    NativeMeshSectionPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Establish the host dispatch binding before native construction makes this
    // element visible; a real allocation new-handler may traverse that list.
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}

void NativeMeshSectionPool::initialize_00b85bb0() {
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
            if (unwind_state >= 2) free_table_unwind_00b85720();
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* NativeMeshSectionPool::allocate_slot_00b85d30() {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    // Keep the native publication and failure semantics: no catch, scope guard,
    // rollback or automatic unlock if either real allocation throws.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_00b85650(raw, storage_.first_free_slab_34) : nullptr;
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

std::uint32_t NativeMeshSectionPool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + slot_slab_index_offset);
}

void NativeMeshSectionPool::return_slot_00b859b0(void* slot) noexcept {
    EnterCriticalSection(section(storage_));
    adjust_recursion(storage_, 1);
    const auto slab_index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[slab_index];
    // Assembly's signed magic multiply is exact signed division by 0x64.
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

void NativeMeshSectionPool::trim_empty_slabs_00b85c90() {
    std::uint32_t index = 0;
    while (index < storage_.slab_count_2c) {
        auto* slab = storage_.slabs_28[index];
        if (read<std::uint16_t>(slab + free_count) != 64) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Native continuation 00B85CB6 copies even when removing the last item.
        storage_.slabs_28[index] = storage_.slabs_28[storage_.slab_count_2c - 1u];
        --storage_.slab_count_2c;
        if (index < storage_.slab_count_2c) {
            auto* moved = storage_.slabs_28[index];
            for (std::uint32_t slot = 0; slot < 64; ++slot)
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

void NativeMeshSectionPool::invoke_trim(void* context) {
    static_cast<NativeMeshSectionPool*>(context)->trim_empty_slabs_00b85c90();
}

void NativeMeshSectionPool::destroy_critical_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        --storage_.recursion_24;
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}

void NativeMeshSectionPool::free_table_unwind_00b85720() noexcept {
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    // Native leaf does not clear table/count/capacity fields.
}

void NativeMeshSectionPool::destroy_00b858f0() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    free_table_unwind_00b85720();
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

namespace {
static_assert(sizeof(NativeMeshSectionStorage) == 0x60);
static_assert(offsetof(NativeMeshSectionStorage, references_04) == 4);
static_assert(offsetof(NativeMeshSectionStorage, primitive_08) == 8);
static_assert(offsetof(NativeMeshSectionStorage, range_words_0c) == 0x0c);
static_assert(offsetof(NativeMeshSectionStorage, instance_count_1c) == 0x1c);
static_assert(offsetof(NativeMeshSectionStorage, material_20) == 0x20);
static_assert(offsetof(NativeMeshSectionStorage, bounds_bits_24) == 0x24);
static_assert(offsetof(NativeMeshSectionStorage, word_34) == 0x34);
static_assert(offsetof(NativeMeshSectionStorage, next_section_38) == 0x38);
static_assert(offsetof(NativeMeshSectionStorage, vertex_streams_3c) == 0x3c);
static_assert(offsetof(NativeMeshSectionStorage, vertex_stream_count_4c) == 0x4c);
static_assert(offsetof(NativeMeshSectionStorage, vertex_layout_50) == 0x50);
static_assert(offsetof(NativeMeshSectionStorage, uninterpreted_54) == 0x54);
static_assert(offsetof(NativeMeshSectionStorage, indexed_58) == 0x58);
static_assert(offsetof(NativeMeshSectionStorage, instance_generator_binding_5c) == 0x5c);
static_assert(sizeof(NativeMeshSectionLayoutKey) == 0x14);
constexpr std::uint32_t section_profile = 0x00d63194;
constexpr std::uint32_t reference_profile = 0x00ceb130;

void retain_actual(void* raw) noexcept {
    auto* count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    count->fetch_add(1, std::memory_order_seq_cst);
}
void assign_actual(void*& field, void* incoming, NativeRenderActualOwners& owners) {
    void* const old = field;
    if (old == incoming) return;
    field = incoming;
    if (incoming) retain_actual(incoming);
    if (old) release_native_render_actual_owner(owners, old);
}
void release_then_clear(void*& field, NativeRenderActualOwners& owners) {
    void* const old = field;
    if (old) {
        release_native_render_actual_owner(owners, old);
        field = nullptr;
    }
}
void release_streams(NativeMeshSectionStorage& section, NativeRenderActualOwners& owners) {
    for (std::int32_t i = 0; i < section.vertex_stream_count_4c; ++i)
        release_then_clear(section.vertex_streams_3c[i], owners);
}
class SectionBaseCleanup final {
public:
    explicit SectionBaseCleanup(NativeMeshSectionStorage& section) noexcept : section_(section) {}
    ~SectionBaseCleanup() { section_.native_vtable_00 = reference_profile; }
private:
    NativeMeshSectionStorage& section_;
};
} // namespace

void* allocate_native_mesh_section_slot_00b85ee0(NativeMeshSectionPool& pool) {
    return pool.allocate_slot_00b85d30();
}
void return_native_mesh_section_slot_00b85b20(NativeMeshSectionPool& pool, void* slot) {
    pool.return_slot_00b859b0(slot);
}

NativeMeshSectionStorage* construct_native_mesh_section_00b857f0(
    void* raw, const volatile std::uint32_t& bounds_w) noexcept {
    const auto w = bounds_w;
    auto* section = ::new (raw) NativeMeshSectionStorage;
    section->native_vtable_00 = reference_profile;
    section->native_vtable_00 = section_profile;
    section->material_20 = nullptr;
    section->references_04.store(1, std::memory_order_relaxed);
    section->bounds_bits_24[0] = 0;
    section->bounds_bits_24[1] = 0;
    section->bounds_bits_24[2] = 0;
    section->bounds_bits_24[3] = w;
    section->word_34 = 0;
    section->next_section_38 = nullptr;
    section->vertex_stream_count_4c = 0;
    section->vertex_layout_50 = nullptr;
    section->uninterpreted_54 = 0;
    section->indexed_58 = 1;
    section->instance_generator_binding_5c = nullptr;
    section->primitive_08 = 4;
    for (auto& word : section->range_words_0c) word = 0;
    section->instance_count_1c = 0;
    return section;
}
NativeMeshSectionStorage* create_native_mesh_section_00533fa0(
    NativeMeshSectionPool& pool, const volatile std::uint32_t& bounds_w) {
    auto* raw = allocate_native_mesh_section_slot_00b85ee0(pool);
    return raw ? construct_native_mesh_section_00b857f0(raw, bounds_w) : nullptr;
}

void set_native_mesh_section_material_00b864c0(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners, void* material) {
    assign_actual(section.material_20, material, owners);
}
void set_native_mesh_section_vertex_stream_00b86500(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,
    std::int32_t index, void* stream) {
    assign_actual(section.vertex_streams_3c[index], stream, owners);
}
void set_native_mesh_section_vertex_layout_00b86650(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners, void* layout) {
    assign_actual(section.vertex_layout_50, layout, owners);
}
void set_native_mesh_section_instance_generator_binding_00b417e0(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners, void* binding) {
    assign_actual(section.instance_generator_binding_5c, binding, owners);
}
void append_native_mesh_section_vertex_stream_00b85b80(
    NativeMeshSectionStorage& section, void* stream) {
    retain_actual(stream);
    section.vertex_streams_3c[section.vertex_stream_count_4c] = stream;
    ++section.vertex_stream_count_4c;
}
void set_native_mesh_section_indexed_00b85600(
    NativeMeshSectionStorage& section, std::uint8_t value) noexcept {
    section.indexed_58 = value;
}
void clear_native_mesh_section_vertex_streams_00b86550(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners) {
    release_streams(section, owners);
    section.vertex_stream_count_4c = 0;
}
void clear_native_mesh_section_resources_00b86390(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners) {
    release_then_clear(section.material_20, owners);
    release_streams(section, owners);
    release_then_clear(section.vertex_layout_50, owners);
}
void rebuild_native_mesh_section_vertex_layout_00b865a0(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,
    void* mesh, NativeMeshSectionLayoutServices& services) {
    if (section.vertex_layout_50) {
        clear_native_mesh_section_vertex_streams_00b86550(section, owners);
        release_then_clear(section.vertex_layout_50, owners);
        if (section.vertex_layout_50) return;
    }
    if (section.vertex_stream_count_4c == 0) {
        // B73260 index0 reads the actual mesh+64 identity, without retaining.
        void* stream;
        std::memcpy(&stream, static_cast<std::byte*>(mesh) + 0x64, sizeof(stream));
        append_native_mesh_section_vertex_stream_00b85b80(section, stream);
    }
    NativeMeshSectionLayoutKey key;
    key.count = 0;
    for (std::int32_t i = 0; i < section.vertex_stream_count_4c; ++i) {
        const auto descriptor = services.current_stream_descriptor_virtual24(section.vertex_streams_3c[i]);
        key.descriptors[key.count] = descriptor;
        ++key.count;
    }
    section.vertex_layout_50 = services.current_renderer_layout_virtual40(key);
}
void destroy_native_mesh_section_00b86420(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners) {
    section.native_vtable_00 = section_profile;
    const SectionBaseCleanup base(section);
    release_then_clear(section.next_section_38, owners);
    release_then_clear(section.instance_generator_binding_5c, owners);
    clear_native_mesh_section_resources_00b86390(section, owners);
}
NativeMeshSectionStorage* delete_native_mesh_section_00b86690(
    NativeMeshSectionStorage* section, NativeMeshSectionEnvironment& environment, std::uint32_t flags) {
    destroy_native_mesh_section_00b86420(*section, environment.retained_owners);
    if (flags & 1) environment.pool_010901d4.return_slot_00b859b0(section);
    return section;
}

NativeMeshSectionReference::NativeMeshSectionReference(NativeMeshSectionStorage& section,
    NativeMeshSectionEnvironment& environment, NativeMeshSectionCompanionDisposal disposal)
    : RenderCommandReference(section.references_04), storage_(section), environment_(environment),
      disposal_(disposal) {
    if (!disposal.retire || section.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("section reference requires live actual storage and explicit retirement");
    require_current_profile();
}
NativeMeshSectionReference::~NativeMeshSectionReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeMeshSectionReference::require_current_profile() const noexcept {
    const auto* table = environment_.vtable_00d63194;
    if (storage_.native_vtable_00 != section_profile || !table ||
        table[0] != 0x00bd30e0 || table[1] != 0x00b86690)
        std::terminate();
}
void NativeMeshSectionReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_mesh_section_00b86690(&storage_, environment_, 1);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
