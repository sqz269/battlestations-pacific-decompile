#include "bsp/native_mesh_section.hpp"
#include "bsp/native_mesh_section_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>
#include <exception>
#include <intrin.h>
#include <limits>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh section copying requires MSVC Win32 x87 assembly.
#endif

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
NativeMeshSectionPool* static_pool_010901d4{};
NativeMeshSectionPool& require_static_pool() {
    if (!static_pool_010901d4)
        throw std::logic_error("native mesh section pool 010901D4 is not bound");
    return *static_pool_010901d4;
}
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

void bind_static_native_mesh_section_pool_010901d4(NativeMeshSectionPool& pool) {
    if (static_pool_010901d4 && static_pool_010901d4 != &pool)
        throw std::logic_error("native mesh section pool 010901D4 is already bound");
    static_pool_010901d4 = &pool;
}

int initialize_static_native_mesh_section_pool_00cd8250(
    NativeMeshSectionPoolAtexit register_atexit) {
    if (!register_atexit)
        throw std::invalid_argument("native mesh section pool requires atexit");
    require_static_pool().initialize_00b85bb0();
    // Preserve EAX from the real registration, including failure without rollback.
    return register_atexit(&destroy_static_native_mesh_section_pool_00ce0ec0);
}

void destroy_static_native_mesh_section_pool_00ce0ec0() {
    require_static_pool().destroy_00b858f0();
}

// Private observer can be constructed only inside this translation unit's
// genuine owning entries. It changes caller metadata, not Native scheduling.
class NativeMeshSectionLifetimeIssuer final {
public:
    NativeMeshSectionLifetimeIssuer(NativeMeshSectionLifetimeFrame& frame,
        NativeMeshSectionLifetimeCall& call) noexcept : frame_(frame),call_(call) {}
    void prefix(NativeMeshSectionStorage& section) noexcept { frame_.cpp_prefix_born(section); }
    void count(NativeMeshSectionStorage& section) noexcept { frame_.native_count1_born(section); }
    void returned(NativeMeshSectionStorage& section) noexcept { frame_.native_constructor_returned(section); }
    void release(NativeRenderActualOwners& owners,void* actual,std::size_t site) {
        frame_.release_child(call_,owners,actual,site);
    }
private:
    NativeMeshSectionLifetimeFrame& frame_;
    NativeMeshSectionLifetimeCall& call_;
};

namespace {
constexpr std::size_t next_site=0,generator_site=1,material_site=2,stream_sites=3,layout_site=7;
using SectionAtomic=std::atomic<std::int32_t>;
using SectionAtomicStorage=std::remove_reference_t<decltype(std::declval<SectionAtomic&>()._Storage)>;
using SectionAtomicValue=std::remove_reference_t<decltype(std::declval<SectionAtomic&>()._Storage._Value)>;
static_assert(!std::is_trivially_copyable_v<SectionAtomic>);
static_assert(std::is_standard_layout_v<SectionAtomic> && std::is_standard_layout_v<SectionAtomicStorage>);
static_assert(std::is_trivially_copyable_v<SectionAtomicStorage>);
static_assert(std::is_same_v<SectionAtomicValue,std::int32_t>);
static_assert(std::is_same_v<std::make_unsigned_t<SectionAtomicValue>,std::uint32_t>);
static_assert(std::is_trivially_copyable_v<SectionAtomicValue> && std::has_unique_object_representations_v<SectionAtomicValue>);
static_assert(sizeof(SectionAtomic)==4 && alignof(SectionAtomic)==4);
static_assert(sizeof(SectionAtomicStorage)==4 && alignof(SectionAtomicStorage)==4);
static_assert(sizeof(SectionAtomicValue)==4 && alignof(SectionAtomicValue)==4);
static_assert(offsetof(SectionAtomic,_Storage)==0 && offsetof(SectionAtomicStorage,_Value)==0);
static_assert(std::numeric_limits<SectionAtomicValue>::digits==31);
static_assert((std::numeric_limits<SectionAtomicValue>::min)()==(-2147483647-1));
static_assert((std::numeric_limits<SectionAtomicValue>::max)()==2147483647);
static_assert(std::numeric_limits<unsigned char>::digits==8);
static_assert(std::is_standard_layout_v<NativeMeshSectionStorage>);
static_assert(!std::is_trivially_copyable_v<NativeMeshSectionStorage>);
static_assert(std::is_nothrow_default_constructible_v<NativeMeshSectionStorage>);
static_assert(std::is_trivially_destructible_v<NativeMeshSectionStorage>);
#define BSP_SECTION_MEMBER(member,offset,bytes) \
    static_assert(offsetof(NativeMeshSectionStorage,member)==offset); \
    static_assert(sizeof(decltype(NativeMeshSectionStorage::member))==bytes); \
    static_assert(std::is_trivially_copyable_v<decltype(NativeMeshSectionStorage::member)>)
BSP_SECTION_MEMBER(native_vtable_00,0x00,4);
static_assert(offsetof(NativeMeshSectionStorage,references_04)==4);
BSP_SECTION_MEMBER(primitive_08,0x08,4);
BSP_SECTION_MEMBER(range_words_0c,0x0c,16);
BSP_SECTION_MEMBER(instance_count_1c,0x1c,4);
BSP_SECTION_MEMBER(material_20,0x20,4);
BSP_SECTION_MEMBER(bounds_bits_24,0x24,16);
BSP_SECTION_MEMBER(word_34,0x34,4);
BSP_SECTION_MEMBER(next_section_38,0x38,4);
BSP_SECTION_MEMBER(vertex_streams_3c,0x3c,16);
BSP_SECTION_MEMBER(vertex_stream_count_4c,0x4c,4);
BSP_SECTION_MEMBER(vertex_layout_50,0x50,4);
BSP_SECTION_MEMBER(uninterpreted_54,0x54,4);
BSP_SECTION_MEMBER(indexed_58,0x58,1);
BSP_SECTION_MEMBER(untouched_59,0x59,3);
BSP_SECTION_MEMBER(instance_generator_binding_5c,0x5c,4);
#undef BSP_SECTION_MEMBER
#define BSP_SECTION_ADJACENT(left,right) \
    static_assert(offsetof(NativeMeshSectionStorage,right)== \
        offsetof(NativeMeshSectionStorage,left)+sizeof(decltype(NativeMeshSectionStorage::left)))
BSP_SECTION_ADJACENT(native_vtable_00,references_04);
BSP_SECTION_ADJACENT(references_04,primitive_08);
BSP_SECTION_ADJACENT(primitive_08,range_words_0c);
BSP_SECTION_ADJACENT(range_words_0c,instance_count_1c);
BSP_SECTION_ADJACENT(instance_count_1c,material_20);
BSP_SECTION_ADJACENT(material_20,bounds_bits_24);
BSP_SECTION_ADJACENT(bounds_bits_24,word_34);
BSP_SECTION_ADJACENT(word_34,next_section_38);
BSP_SECTION_ADJACENT(next_section_38,vertex_streams_3c);
BSP_SECTION_ADJACENT(vertex_streams_3c,vertex_stream_count_4c);
BSP_SECTION_ADJACENT(vertex_stream_count_4c,vertex_layout_50);
BSP_SECTION_ADJACENT(vertex_layout_50,uninterpreted_54);
BSP_SECTION_ADJACENT(uninterpreted_54,indexed_58);
BSP_SECTION_ADJACENT(indexed_58,untouched_59);
BSP_SECTION_ADJACENT(untouched_59,instance_generator_binding_5c);
#undef BSP_SECTION_ADJACENT
static_assert(sizeof(NativeMeshSectionStorage)==offsetof(NativeMeshSectionStorage,instance_generator_binding_5c)+4);
template<class T> void restore_section_subobject(T& destination,const unsigned char* preimage) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    auto* const bytes=reinterpret_cast<volatile unsigned char*>(&destination);
    for(std::size_t i=0;i!=sizeof(T);++i)bytes[i]=preimage[i];
}
NativeMeshSectionStorage* begin_section_storage(void* raw) noexcept {
    // Existing fresh/aligned/exclusive60h birth contract: no callbacks or count
    // users can observe the transient SDK default atomic0 before restoration.
    // This is Source C++ birth work, not an additional Native instruction proof.
    // All16 real born member extents are restored individually. Neither atomic
    // nor whole carrier is byte-copied as a trivially-copyable complete object.
    // Untouched streams/+59..5B retain arbitrary preimage; live pool ID+60 is OUT.
    unsigned char preimage[sizeof(NativeMeshSectionStorage)];
    auto* const source=static_cast<const volatile unsigned char*>(raw);
    _ReadWriteBarrier();
    for(std::size_t i=0;i!=sizeof(preimage);++i)preimage[i]=source[i];
    _ReadWriteBarrier();
    auto* const section=::new(raw)NativeMeshSectionStorage;
    _ReadWriteBarrier();
    restore_section_subobject(section->native_vtable_00,preimage);
    restore_section_subobject(section->references_04._Storage._Value,preimage+4);
    restore_section_subobject(section->primitive_08,preimage+0x08);
    restore_section_subobject(section->range_words_0c,preimage+0x0c);
    restore_section_subobject(section->instance_count_1c,preimage+0x1c);
    restore_section_subobject(section->material_20,preimage+0x20);
    restore_section_subobject(section->bounds_bits_24,preimage+0x24);
    restore_section_subobject(section->word_34,preimage+0x34);
    restore_section_subobject(section->next_section_38,preimage+0x38);
    restore_section_subobject(section->vertex_streams_3c,preimage+0x3c);
    restore_section_subobject(section->vertex_stream_count_4c,preimage+0x4c);
    restore_section_subobject(section->vertex_layout_50,preimage+0x50);
    restore_section_subobject(section->uninterpreted_54,preimage+0x54);
    restore_section_subobject(section->indexed_58,preimage+0x58);
    restore_section_subobject(section->untouched_59,preimage+0x59);
    restore_section_subobject(section->instance_generator_binding_5c,preimage+0x5c);
    _ReadWriteBarrier();
    return section;
}

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
void assign_actual(void*& field, void* incoming, NativeRenderActualOwners& owners,
    NativeMeshSectionLifetimeIssuer* issuer=nullptr,std::size_t site=0) {
    void* const old = field;
    if (old == incoming) return;
    field = incoming;
    if (incoming) retain_actual(incoming);
    if (old) {
        if (issuer) issuer->release(owners,old,site);
        else release_native_render_actual_owner(owners, old);
    }
}
void copy_actual(void*& destination, void* const& source, NativeRenderActualOwners& owners,
    NativeMeshSectionLifetimeIssuer* issuer=nullptr,std::size_t site=0) {
    // B85EF0 loads the destination before the current source identity.
    void* const old = destination;
    void* const incoming = source;
    if (old == incoming) return;
    destination = incoming;
    if (incoming) retain_actual(incoming);
    if (old) {
        if (issuer) issuer->release(owners,old,site);
        else release_native_render_actual_owner(owners, old);
    }
}
void copy_section_float_word(std::uint32_t& destination, const std::uint32_t& source) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
void release_then_clear(void*& field, NativeRenderActualOwners& owners,
    NativeMeshSectionLifetimeIssuer* issuer=nullptr,std::size_t site=0) {
    void* const old = field;
    if (old) {
        if (issuer) issuer->release(owners,old,site);
        else release_native_render_actual_owner(owners, old);
        field = nullptr;
    }
}
void release_streams(NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,
    NativeMeshSectionLifetimeIssuer* issuer=nullptr) {
    for (std::int32_t i = 0; i < section.vertex_stream_count_4c; ++i)
        release_then_clear(section.vertex_streams_3c[i], owners,issuer,stream_sites+i);
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

static NativeMeshSectionStorage* construct_section_body(
    void* raw, const volatile std::uint32_t& bounds_w,NativeMeshSectionLifetimeIssuer* issuer) noexcept {
    const auto w = bounds_w;
    auto* section = issuer ? begin_section_storage(raw) : ::new (raw) NativeMeshSectionStorage;
    if (issuer) issuer->prefix(*section);
    section->native_vtable_00 = reference_profile;
    section->native_vtable_00 = section_profile;
    section->material_20 = nullptr;
    section->references_04.store(1, std::memory_order_relaxed);
    if (issuer) issuer->count(*section);
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
    if (issuer) issuer->returned(*section);
    return section;
}
NativeMeshSectionStorage* construct_native_mesh_section_00b857f0(
    void* raw,const volatile std::uint32_t& bounds_w) noexcept {
    return construct_section_body(raw,bounds_w,nullptr);
}
NativeMeshSectionStorage* create_native_mesh_section_00533fa0(
    NativeMeshSectionPool& pool, const volatile std::uint32_t& bounds_w) {
    auto* raw = allocate_native_mesh_section_slot_00b85ee0(pool);
    return raw ? construct_native_mesh_section_00b857f0(raw, bounds_w) : nullptr;
}

static NativeMeshSectionStorage* copy_section_body(
    void* raw, const NativeMeshSectionStorage& source, NativeRenderActualOwners& owners,
    const volatile std::uint32_t& bounds_w,NativeMeshSectionLifetimeIssuer* issuer) {
    auto* section = issuer ? begin_section_storage(raw) : ::new (raw) NativeMeshSectionStorage;
    if (issuer) issuer->prefix(*section);
    section->native_vtable_00 = reference_profile;
    section->references_04.store(1, std::memory_order_relaxed);
    if (issuer) issuer->count(*section);
    const auto w = bounds_w; // Native copy reads CE4970 after base count=1.
    section->native_vtable_00 = section_profile;
    section->material_20 = nullptr;
    section->bounds_bits_24[0] = 0;
    section->bounds_bits_24[1] = 0;
    section->bounds_bits_24[2] = 0;
    section->bounds_bits_24[3] = w;
    section->word_34 = 0;
    section->next_section_38 = nullptr;
    section->vertex_stream_count_4c = 0;
    section->vertex_layout_50 = nullptr;
    section->instance_generator_binding_5c = nullptr;
    section->indexed_58 = source.indexed_58;
    section->primitive_08 = source.primitive_08;
    for (std::uint32_t i = 0; i < 4; ++i)
        section->range_words_0c[i] = source.range_words_0c[i];
    section->instance_count_1c = source.instance_count_1c;
    try { // Native EH state0 starts immediately before material assignment.
        copy_actual(section->material_20, source.material_20, owners,issuer,material_site);
        for (std::uint32_t i = 0; i < 4; ++i)
            copy_section_float_word(section->bounds_bits_24[i], source.bounds_bits_24[i]);
        copy_section_float_word(section->word_34, source.word_34);
        copy_actual(section->next_section_38, source.next_section_38, owners,issuer,next_site);
        for (std::int32_t i = 0; i < source.vertex_stream_count_4c; ++i)
            append_native_mesh_section_vertex_stream_00b85b80(*section, source.vertex_streams_3c[i]);
        copy_actual(section->vertex_layout_50, source.vertex_layout_50, owners,issuer,layout_site);
        section->uninterpreted_54 = source.uninterpreted_54;
        copy_actual(section->instance_generator_binding_5c, source.instance_generator_binding_5c, owners,issuer,generator_site);
    } catch (...) {
        // CC24B0 invokes only BD30F0, not the derived resource destructor.
        section->native_vtable_00 = reference_profile;
        throw;
    }
    if (issuer) issuer->returned(*section);
    return section;
}
NativeMeshSectionStorage* copy_construct_native_mesh_section_00b85ef0(
    void* raw,const NativeMeshSectionStorage& source,NativeRenderActualOwners& owners,
    const volatile std::uint32_t& bounds_w) {
    return copy_section_body(raw,source,owners,bounds_w,nullptr);
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
static void clear_section_streams_body(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,NativeMeshSectionLifetimeIssuer* issuer) {
    release_streams(section, owners,issuer);
    section.vertex_stream_count_4c = 0;
}
static void clear_section_resources_body(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,NativeMeshSectionLifetimeIssuer* issuer) {
    release_then_clear(section.material_20, owners,issuer,material_site);
    release_streams(section, owners,issuer);
    release_then_clear(section.vertex_layout_50, owners,issuer,layout_site);
}
static void rebuild_section_layout_body(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,
    void* mesh, NativeMeshSectionLayoutServices& services,NativeMeshSectionLifetimeIssuer* issuer) {
    if (section.vertex_layout_50) {
        clear_section_streams_body(section, owners,issuer);
        release_then_clear(section.vertex_layout_50, owners,issuer,layout_site);
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
static void destroy_section_body(
    NativeMeshSectionStorage& section, NativeRenderActualOwners& owners,NativeMeshSectionLifetimeIssuer* issuer) {
    section.native_vtable_00 = section_profile;
    const SectionBaseCleanup base(section);
    release_then_clear(section.next_section_38, owners,issuer,next_site);
    release_then_clear(section.instance_generator_binding_5c, owners,issuer,generator_site);
    clear_section_resources_body(section, owners,issuer);
}
void clear_native_mesh_section_vertex_streams_00b86550(
    NativeMeshSectionStorage& section,NativeRenderActualOwners& owners) {
    clear_section_streams_body(section,owners,nullptr);
}
void clear_native_mesh_section_resources_00b86390(
    NativeMeshSectionStorage& section,NativeRenderActualOwners& owners) {
    clear_section_resources_body(section,owners,nullptr);
}
void rebuild_native_mesh_section_vertex_layout_00b865a0(
    NativeMeshSectionStorage& section,NativeRenderActualOwners& owners,
    void* mesh,NativeMeshSectionLayoutServices& services) {
    rebuild_section_layout_body(section,owners,mesh,services,nullptr);
}
void destroy_native_mesh_section_00b86420(
    NativeMeshSectionStorage& section,NativeRenderActualOwners& owners) {
    destroy_section_body(section,owners,nullptr);
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
    if (!ordinary_frame_ && phase_ != Phase::retired) std::terminate();
}
void NativeMeshSectionReference::require_current_profile() const noexcept {
    const auto* table = environment_.vtable_00d63194;
    if (storage_.native_vtable_00 != section_profile || !table ||
        table[0] != 0x00bd30e0 || table[1] != 0x00b86690)
        std::terminate();
}
NativeMeshSectionReference::NativeMeshSectionReference(NativeMeshSectionStorage& section,
    NativeMeshSectionEnvironment& environment,NativeMeshSectionLifetimeFrame& frame) noexcept
    :RenderCommandReference(section.references_04),ordinary_frame_(&frame),storage_(section),environment_(environment),
     disposal_{} {}
void NativeMeshSectionReference::validate_ordinary_target() const {
    const auto* table=environment_.vtable_00d63194;
    if(storage_.native_vtable_00!=section_profile || !table ||
        table[0]!=0x00bd30e0 || table[1]!=0x00b86690)
        throw std::logic_error("ordinary Section CURRENT target/profile mismatch");
}
void NativeMeshSectionReference::release_zero_references() {
    if(ordinary_frame_){ordinary_frame_->ordinary_virtual_zero(*this);return;}
    release_zero_references_legacy();
}
void NativeMeshSectionReference::release_zero_references_legacy() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_mesh_section_00b86690(&storage_, environment_, 1);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
NativeMeshSectionStorage* NativeMeshSectionLifetimeAccess::construct(void* raw,
    const volatile std::uint32_t& bounds,NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionLifetimeCall& call) noexcept {
    NativeMeshSectionLifetimeIssuer issuer(frame,call);
    return construct_section_body(raw,bounds,&issuer);
}
NativeMeshSectionStorage* NativeMeshSectionLifetimeAccess::copy(void* raw,
    const NativeMeshSectionStorage& source,NativeRenderActualOwners& owners,
    const volatile std::uint32_t& bounds,NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionLifetimeCall& call) {
    NativeMeshSectionLifetimeIssuer issuer(frame,call);
    return copy_section_body(raw,source,owners,bounds,&issuer);
}
void NativeMeshSectionLifetimeAccess::mutate(NativeMeshSectionStorage& section,
    NativeRenderActualOwners& owners,std::uint32_t operation,void* operand,std::int32_t index,
    NativeMeshSectionLayoutServices* services,NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionLifetimeCall& call) {
    NativeMeshSectionLifetimeIssuer issuer(frame,call);
    using Operation=NativeMeshSectionLifetimeFrame::Operation;
    switch(static_cast<Operation>(operation)) {
    case Operation::material: assign_actual(section.material_20,operand,owners,&issuer,material_site);break;
    case Operation::stream: assign_actual(section.vertex_streams_3c[index],operand,owners,&issuer,stream_sites+index);break;
    case Operation::layout: assign_actual(section.vertex_layout_50,operand,owners,&issuer,layout_site);break;
    case Operation::generator: assign_actual(section.instance_generator_binding_5c,operand,owners,&issuer,generator_site);break;
    case Operation::append: append_native_mesh_section_vertex_stream_00b85b80(section,operand);break;
    case Operation::indexed: set_native_mesh_section_indexed_00b85600(section,static_cast<std::uint8_t>(index));break;
    case Operation::clear_streams: clear_section_streams_body(section,owners,&issuer);break;
    case Operation::clear_resources: clear_section_resources_body(section,owners,&issuer);break;
    case Operation::rebuild: rebuild_section_layout_body(section,owners,operand,*services,&issuer);break;
    default: throw std::logic_error("Section private mutation operation mismatch");
    }
}
void NativeMeshSectionLifetimeAccess::destroy(NativeMeshSectionStorage& section,
    NativeRenderActualOwners& owners,NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionLifetimeCall& call) {
    NativeMeshSectionLifetimeIssuer issuer(frame,call);
    destroy_section_body(section,owners,&issuer);
}
} // namespace bsp
