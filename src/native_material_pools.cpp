#include "bsp/native_material_pools.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_gui_layer_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native material pools target MSVC Win32.");
static_assert(sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeMaterialPoolStorage) == 0x38);
static_assert(offsetof(NativeMaterialPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeMaterialPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeMaterialPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeMaterialPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeMaterialPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeMaterialPoolStorage, first_free_slab_34) == 0x34);
static_assert(sizeof(NativeMaterialStorage) == NativeMaterialPool::object_bytes);
static_assert(sizeof(NativeGuiLayerPoolStorage) == 0x38);
static_assert(NativeGuiLayerPool::slot_bytes * NativeGuiLayerPool::slots_per_slab ==
    NativeGuiLayerPool::free_indices_offset);
static_assert(NativeGuiLayerPool::free_indices_offset +
    NativeGuiLayerPool::slots_per_slab * sizeof(std::uint16_t) ==
    NativeGuiLayerPool::free_count_offset);
static_assert(NativeGuiLayerPool::free_count_offset + sizeof(std::uint32_t) ==
    NativeGuiLayerPool::slab_bytes);
constexpr std::uint32_t no_free_slab = 0xffffffffu;

CRITICAL_SECTION* section(NativeMaterialPoolStorage& storage) noexcept {
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
void adjust_recursion(NativeMaterialPoolStorage& storage, std::uint32_t delta) noexcept {
    const auto bits = static_cast<std::uint32_t>(storage.recursion_24) + delta;
    std::memcpy(&storage.recursion_24, &bits, sizeof(bits));
}
std::byte** allocate_table(std::uint32_t capacity) {
    // BF55BE is a trampoline to the existing BF681B new-handler allocator.
    const auto bytes = capacity * 4u;
    return static_cast<std::byte**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes}));
}
// B17630/B17670/B402B0 receive ECX=pool+28 and free only the current pointer. They
// preserve pointer/count/capacity fields, including during constructor unwind.
void free_table(NativeMaterialPoolStorage& storage) noexcept {
    if (storage.slabs_28) singleton_lifetime_free(storage.slabs_28);
}
void destroy_section(NativeMaterialPoolStorage& storage) noexcept {
    while (storage.recursion_24 > 0) {
        --storage.recursion_24;
        LeaveCriticalSection(section(storage));
    }
    DeleteCriticalSection(section(storage));
}
template<class Pool, auto Reserve = &native_gui_pool_table_reserve_00aa7740> void initialize_pool(
    AllocatorListDomain& list, NativeMaterialPoolStorage& storage) {
    list.prepend_base_element(storage.allocator_00);
    storage.allocator_00.native_vtable_00 = Pool::native_vtable;
    unsigned unwind_state = 0;
    // B17FA0/B18340/B41090: states 0/1/2 unwind list, section, pointer table. As in
    // the actual constructor, allocation failure does not reset native fields.
    __try {
        ::new (storage.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage));
        storage.recursion_24 = 0;
        unwind_state = 1;
        storage.slabs_28 = nullptr;
        storage.slab_count_2c = 0;
        storage.table_capacity_30 = 0;
        storage.first_free_slab_34 = no_free_slab;
        unwind_state = 2;
        // Source reuse of 00AA7740, not an original CALL claim: bsp.py show
        // 00AA79A0 has the same reserve(32) at pool+28h, inside unwind state 2.
        // The common pool engine retains the actual published table header.
        Reserve(native_gui_pool_slab_table(storage), 32);
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_table(storage);
            if (unwind_state >= 1) destroy_section(storage);
            list.unlink_base_element_00403970(storage.allocator_00);
        }
    }
}
template<class Pool> struct SlabBytes { std::byte bytes[Pool::slab_bytes]; };
// B17440/B17510/B401D0 initialize metadata only: free stack in descending order,
// identical slab ID in every slot, untouched object payload and tail padding.
template<class Pool> std::byte* initialize_slab(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes<Pool>)->bytes;
    write<std::uint16_t>(bytes + Pool::free_count_offset,
        static_cast<std::uint16_t>(Pool::slots_per_slab));
    for (std::uint32_t i = 0; i < Pool::slots_per_slab; ++i) {
        write<std::uint16_t>(bytes + Pool::free_indices_offset + i * 2u,
            static_cast<std::uint16_t>(Pool::slots_per_slab - 1u - i));
        write<std::uint32_t>(bytes + i * Pool::slot_bytes + Pool::slot_slab_index_offset, index);
    }
    return bytes;
}
template<class Pool, auto Initialize = &initialize_slab<Pool>>
void* allocate_slot(NativeMaterialPoolStorage& storage) {
    EnterCriticalSection(section(storage));
    adjust_recursion(storage, 1);
    // These allocators have no unwind region. Preserve early first-free and
    // capacity publication, without rollback or automatic unlock on failure.
    if (storage.first_free_slab_34 == no_free_slab) {
        storage.first_free_slab_34 = storage.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, Pool::slab_bytes, Pool::slab_bytes});
        auto* slab = raw ? static_cast<std::byte*>(Initialize(raw, storage.first_free_slab_34)) : nullptr;
        // bsp.py show 00AA7A80: append the actual newly initialized slab AFTER
        // publishing first_free_slab, under this same lock. 00AA76C0 is its
        // out-of-line equivalent; it replaces the inline growth/copy/append.
        native_gui_pool_table_push_back_00aa76c0(native_gui_pool_slab_table(storage), slab);
    }
    auto* slab = storage.slabs_28[storage.first_free_slab_34];
    const auto count = static_cast<std::uint16_t>(
        read<std::uint16_t>(slab + Pool::free_count_offset) - 1u);
    write<std::uint16_t>(slab + Pool::free_count_offset, count);
    auto* slot = slab + read<std::uint16_t>(
        slab + Pool::free_indices_offset + count * 2u) * Pool::slot_bytes;
    if (count == 0) {
        auto index = storage.first_free_slab_34 + 1u;
        storage.first_free_slab_34 = no_free_slab;
        for (; index < storage.slab_count_2c; ++index) {
            if (read<std::uint16_t>(storage.slabs_28[index] + Pool::free_count_offset) != 0) {
                storage.first_free_slab_34 = index;
                break;
            }
        }
    }
    adjust_recursion(storage, 0xffffffffu);
    LeaveCriticalSection(section(storage));
    return slot;
}
template<class Pool> void return_slot(NativeMaterialPoolStorage& storage, void* slot) noexcept {
    EnterCriticalSection(section(storage));
    adjust_recursion(storage, 1);
    // The material return, B193FA parameter fragment and B40A40 load the live ID
    // here, under the real lock. Trimming may have moved its slab table entry.
    const auto slab_index = read<std::uint32_t>(
        static_cast<const std::byte*>(slot) + Pool::slot_slab_index_offset);
    auto* slab = storage.slabs_28[slab_index];
    const auto offset_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slot)
        - reinterpret_cast<std::uintptr_t>(slab));
    std::int32_t offset;
    std::memcpy(&offset, &offset_bits, sizeof(offset));
    // Signed IMUL/SAR/SHR sequences implement signed division toward zero by
    // 114h (magic 76B981DB), 88h (78787879) or 8Ch (EA0EA0EB).
    // Valid slots lie in this slab.
    const auto index = static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(Pool::slot_bytes));
    const auto count = read<std::uint16_t>(slab + Pool::free_count_offset);
    write<std::uint16_t>(slab + Pool::free_indices_offset + count * 2u, index);
    write<std::uint16_t>(slab + Pool::free_count_offset,
        static_cast<std::uint16_t>(read<std::uint16_t>(slab + Pool::free_count_offset) + 1u));
    if (slab_index < storage.first_free_slab_34) storage.first_free_slab_34 = slab_index;
    adjust_recursion(storage, 0xffffffffu);
    LeaveCriticalSection(section(storage));
}
template<class Pool> void trim_empty_slabs(NativeMaterialPoolStorage& storage) {
    std::uint32_t index = 0;
    while (index < storage.slab_count_2c) {
        auto* slab = storage.slabs_28[index];
        if (read<std::uint16_t>(slab + Pool::free_count_offset) != Pool::slots_per_slab) {
            ++index;
            continue;
        }
        singleton_lifetime_free(slab);
        // Full free continuations B18186/B18527/B41196 copy even the last entry before
        // decrementing count, rewrite EVERY moved slot ID, then retry the index.
        storage.slabs_28[index] = storage.slabs_28[storage.slab_count_2c - 1u];
        --storage.slab_count_2c;
        if (index < storage.slab_count_2c) {
            auto* moved = storage.slabs_28[index];
            for (std::uint32_t slot = 0; slot < Pool::slots_per_slab; ++slot)
                write<std::uint32_t>(moved + slot * Pool::slot_bytes + Pool::slot_slab_index_offset, index);
        }
    }
    storage.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage.slab_count_2c; ++i) {
        if (read<std::uint16_t>(storage.slabs_28[i] + Pool::free_count_offset) != 0) {
            storage.first_free_slab_34 = i;
            break;
        }
    }
}
template<class Pool, void (*FreeTable)(void*) noexcept = nullptr>
void destroy_pool(AllocatorListDomain& list, NativeMaterialPoolStorage& storage) {
    storage.allocator_00.native_vtable_00 = Pool::native_vtable;
    for (std::uint32_t i = 0; i < storage.slab_count_2c; ++i)
        singleton_lifetime_free(storage.slabs_28[i]);
    if constexpr (FreeTable != nullptr) {
        FreeTable(&storage.slabs_28);
    } else {
        free_table(storage);
    }
    destroy_section(storage);
    list.unlink_base_element_00403970(storage.allocator_00);
}
} // namespace

NativeMaterialPool::NativeMaterialPool(AllocatorListDomain& list, NativeMaterialPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialPool::initialize_00b17fa0() { initialize_pool<NativeMaterialPool>(allocator_list_, storage_); }
void* NativeMaterialPool::allocate_00b18200() { return allocate_slot<NativeMaterialPool>(storage_); }
void NativeMaterialPool::return_slot_00b17a80(void* slot) noexcept { return_slot<NativeMaterialPool>(storage_, slot); }
void NativeMaterialPool::trim_empty_slabs_00b18160() { trim_empty_slabs<NativeMaterialPool>(storage_); }
void NativeMaterialPool::destroy_00b180d0() { destroy_pool<NativeMaterialPool>(allocator_list_, storage_); }
void NativeMaterialPool::invoke_trim(void* context) {
    static_cast<NativeMaterialPool*>(context)->trim_empty_slabs_00b18160();
}

NativeMaterialParameterPool::NativeMaterialParameterPool(
    AllocatorListDomain& list, NativeMaterialParameterPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialParameterPool::initialize_00b18340() {
    initialize_pool<NativeMaterialParameterPool>(allocator_list_, storage_);
}
void* NativeMaterialParameterPool::allocate_slot_00b185a0() {
    return allocate_slot<NativeMaterialParameterPool>(storage_);
}
void NativeMaterialParameterPool::return_slot_00b193fa_fragment(void* slot) noexcept {
    return_slot<NativeMaterialParameterPool>(storage_, slot);
}
void NativeMaterialParameterPool::trim_empty_slabs_00b18500() {
    trim_empty_slabs<NativeMaterialParameterPool>(storage_);
}
void NativeMaterialParameterPool::destroy_00b18470() {
    destroy_pool<NativeMaterialParameterPool>(allocator_list_, storage_);
}
void NativeMaterialParameterPool::invoke_trim(void* context) {
    static_cast<NativeMaterialParameterPool*>(context)->trim_empty_slabs_00b18500();
}

NativeMaterialPassPool::NativeMaterialPassPool(AllocatorListDomain& list, NativeMaterialPassPoolStorage& storage)
    : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeMaterialPassPool::initialize_00b41090() {
    initialize_pool<NativeMaterialPassPool>(allocator_list_, storage_);
}
void* NativeMaterialPassPool::allocate_slot_00b41210() {
    // 00B41210's nonnull allocation arm calls 00B401D0 with the newly
    // allocated 11C4h slab and the live, already-published first-free ID.
    return allocate_slot<NativeMaterialPassPool,
        &initialize_native_material_pass_slab_00b401d0>(storage_);
}
void NativeMaterialPassPool::return_slot_00b40a40(void* slot) noexcept {
    return_slot<NativeMaterialPassPool>(storage_, slot);
}
void NativeMaterialPassPool::trim_empty_slabs_00b41170() {
    trim_empty_slabs<NativeMaterialPassPool>(storage_);
}
void NativeMaterialPassPool::destroy_00b40980() {
    // bsp.py show 00B40980 and 00B402B0 --asm: both release the current
    // pool+28h table without clearing its pointer/count/capacity. Pass the
    // actual table header after slab destruction and before CS/list teardown.
    destroy_pool<NativeMaterialPassPool,
        &destroy_native_material_pass_pool_table_00b402b0>(allocator_list_, storage_);
}
void NativeMaterialPassPool::invoke_trim(void* context) {
    static_cast<NativeMaterialPassPool*>(context)->trim_empty_slabs_00b41170();
}
// Keep recovered slab entries out of line so production allocation retains
// their call boundaries; the metadata implementation remains shared once.
__declspec(noinline) void* initialize_native_material_pass_slab_00b401d0(void* raw, std::uint32_t index) noexcept {
    return initialize_slab<NativeMaterialPassPool>(raw, index);
}
__declspec(noinline) void destroy_native_material_pass_pool_table_00b402b0(void* header) noexcept {
    std::byte** table;
    std::memcpy(&table, header, sizeof(table));
    if (table) singleton_lifetime_free(table);
}
namespace { NativeMaterialPassPool* canonical_pass_pool; }
void bind_static_native_material_pass_pool_0108fbf8(NativeMaterialPassPool& pool) noexcept {
    canonical_pass_pool = &pool;
}
bool is_static_native_material_pass_pool(const NativeMaterialPassPool& pool) noexcept {
    return canonical_pass_pool == &pool;
}
void* allocate_static_native_material_pass_slot_00b41820() {
    return canonical_pass_pool->allocate_slot_00b41210();
}
void return_static_native_material_pass_slot_00b41040(void* slot) {
    canonical_pass_pool->return_slot_00b40a40(slot);
}
int initialize_static_native_material_pass_pool_00cd7bc0() {
    canonical_pass_pool->initialize_00b41090();
    return std::atexit(&destroy_static_native_material_pass_pool_00ce0cd0);
}
void destroy_static_native_material_pass_pool_00ce0cd0() noexcept {
    canonical_pass_pool->destroy_00b40980();
}

NativeShaderStateListPool::NativeShaderStateListPool(AllocatorListDomain& list,NativeShaderStateListPoolStorage& storage)
    :allocator_list_(list),storage_(storage) {
    list.bind_virtual0(storage.allocator_00,{native_vtable,native_virtual0,this,&invoke_trim});
}
void NativeShaderStateListPool::initialize_00b623d0(){initialize_pool<NativeShaderStateListPool>(allocator_list_,storage_);}
void* NativeShaderStateListPool::allocate_slot_00b62630(){
    // bsp.py show 00B62630: its actual 244h allocation and current +34 ID
    // feed 00B620F0 before the slab table/count publication.
    return allocate_slot<NativeShaderStateListPool,
        &initialize_native_shader_state_list_slab_00b620f0>(storage_);
}
void NativeShaderStateListPool::return_slot_00b62280(void* slot) noexcept {
    // Original SAR4 divides the signed byte offset. Valid slots are16-byte
    // aligned in this actual slab, so the common division is exactly equal.
    return_slot<NativeShaderStateListPool>(storage_,slot);
}
void NativeShaderStateListPool::trim_empty_slabs_00b62590(){trim_empty_slabs<NativeShaderStateListPool>(storage_);}
void NativeShaderStateListPool::destroy_00b62500(){
    // bsp.py show 00B62500 and 00B621B0 --asm: the same nonnull table
    // release, over this pool's retained +28h header, preserving stale fields.
    destroy_pool<NativeShaderStateListPool,
        &destroy_native_shader_state_list_pool_table_00b621b0>(allocator_list_, storage_);
}
void NativeShaderStateListPool::invoke_trim(void* context){static_cast<NativeShaderStateListPool*>(context)->trim_empty_slabs_00b62590();}
__declspec(noinline) void* initialize_native_shader_state_list_slab_00b620f0(void* raw,std::uint32_t index) noexcept {
    return initialize_slab<NativeShaderStateListPool>(raw,index);
}
__declspec(noinline) void destroy_native_shader_state_list_pool_table_00b621b0(void* header) noexcept {
    void* data;std::memcpy(&data,header,sizeof(data));if(data)singleton_lifetime_free(data);
}
namespace {NativeShaderStateListPool* canonical_shader_state_list_pool;}
void bind_static_native_shader_state_list_pool_0108fee4(NativeShaderStateListPool& pool) noexcept{canonical_shader_state_list_pool=&pool;}
void* allocate_static_native_shader_state_list_slot_00b62760(){return canonical_shader_state_list_pool->allocate_slot_00b62630();}
void return_static_native_shader_state_list_slot_00b623c0(NativeShaderDescriptorArray& slot){canonical_shader_state_list_pool->return_slot_00b62280(&slot);}
void return_static_native_shader_state_list_slot_00b623c0(void* slot){
    // 00B623C0: forward raw storage to its typed view; OFF keeps the original global-pool call.
    return_static_native_shader_state_list_slot_00b623c0(*static_cast<NativeShaderDescriptorArray*>(slot));
}
int initialize_static_native_shader_state_list_pool_00cd7cc0(){
    canonical_shader_state_list_pool->initialize_00b623d0();return std::atexit(&destroy_static_native_shader_state_list_pool_00ce0d50);
}
void destroy_static_native_shader_state_list_pool_00ce0d50() noexcept{canonical_shader_state_list_pool->destroy_00b62500();}

NativeGuiLayerPool::NativeGuiLayerPool(AllocatorListDomain& list,
    NativeGuiLayerPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00,
        {native_vtable, native_virtual0, this, &invoke_trim});
}
NativeGuiLayerPool::~NativeGuiLayerPool() {
    allocator_list_.unbind_virtual0(storage_.allocator_00);
}
void NativeGuiLayerPool::initialize_00ac4d70() {
    initialize_pool<NativeGuiLayerPool>(allocator_list_, storage_);
}
void* NativeGuiLayerPool::allocate_00ac4e50() {
    return allocate_slot<NativeGuiLayerPool>(storage_);
}
void NativeGuiLayerPool::return_00ac47f0(void* slot) noexcept {
    return_slot<NativeGuiLayerPool>(storage_, slot);
}
void NativeGuiLayerPool::trim_00ac4750() {
    trim_empty_slabs<NativeGuiLayerPool>(storage_);
}
void NativeGuiLayerPool::destroy_00ac4670() {
    destroy_pool<NativeGuiLayerPool>(allocator_list_, storage_);
}
void NativeGuiLayerPool::invoke_trim(void* context) {
    static_cast<NativeGuiLayerPool*>(context)->trim_00ac4750();
}
} // namespace bsp

// Group specialization only; the shared native pool engine above is unchanged.
#include "bsp/native_gui_group_storage.hpp"
namespace bsp {
static_assert(sizeof(NativeGuiGroupPoolStorage)==0x38);
static_assert(NativeGuiGroupPool::slot_bytes*NativeGuiGroupPool::slots_per_slab==
    NativeGuiGroupPool::free_indices_offset);
static_assert(NativeGuiGroupPool::free_indices_offset+2*NativeGuiGroupPool::slots_per_slab==
    NativeGuiGroupPool::free_count_offset);
static_assert(NativeGuiGroupPool::free_count_offset+4==NativeGuiGroupPool::slab_bytes);
NativeGuiGroupPool::NativeGuiGroupPool(AllocatorListDomain& list,
    NativeGuiGroupPoolStorage& storage) : allocator_list_(list),storage_(storage) {
    list.bind_virtual0(storage.allocator_00,{native_vtable,native_virtual0,this,&invoke_trim});
}
NativeGuiGroupPool::~NativeGuiGroupPool() {
    allocator_list_.unbind_virtual0(storage_.allocator_00);
}
void NativeGuiGroupPool::initialize_00ac7410() {
    initialize_pool<NativeGuiGroupPool>(allocator_list_,storage_);
}
void* NativeGuiGroupPool::allocate_00ac7590() {
    // bsp.py show 00AC7590: initialize the actual new 1E44h slab through
    // 00AC6FF0, retaining the native lock, null arm and publication order.
    return allocate_slot<NativeGuiGroupPool,
        &initialize_native_gui_group_slab_00ac6ff0>(storage_);
}
void NativeGuiGroupPool::return_00ac7260(void* slot) noexcept {
    return_slot<NativeGuiGroupPool>(storage_,slot);
}
void NativeGuiGroupPool::trim_00ac74f0() {
    trim_empty_slabs<NativeGuiGroupPool>(storage_);
}
void NativeGuiGroupPool::destroy_00ac71a0() {
    // bsp.py show 00AC71A0/00AC70D0: the destroy body's inline table
    // free and the out-of-line helper both free only the current +28h
    // pointer, preserving count/capacity and the dangling pointer. Pass
    // the actual pool header after slab retirement, before CS/list teardown.
    destroy_pool<NativeGuiGroupPool,
        &free_native_gui_group_pool_table_00ac70d0>(allocator_list_,storage_);
}
void NativeGuiGroupPool::invoke_trim(void* context) {
    static_cast<NativeGuiGroupPool*>(context)->trim_00ac74f0();
}
__declspec(noinline) void* initialize_native_gui_group_slab_00ac6ff0(void* raw,std::uint32_t index) noexcept {
    return initialize_slab<NativeGuiGroupPool>(raw,index);
}
__declspec(noinline) void free_native_gui_group_pool_table_00ac70d0(void* header) noexcept {
    void* const table=read<void*>(static_cast<const std::byte*>(header));
    if (table) singleton_lifetime_free(table);
}
} // namespace bsp

// GUI widget pool family (cc13-gui1, docs/CC13_GUI_1.md): fourteen more instances
// of the same native engine, parameterized by include/bsp/native_gui_widget_pools_cc13.hpp.
#include "bsp/terrain_bush_pool_cc13.hpp"
#include "bsp/cc13_seg87.hpp"
namespace bsp {
template<class T> NativeGuiWidgetPool<T>::NativeGuiWidgetPool(AllocatorListDomain& list,
    NativeGuiWidgetPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    list.bind_virtual0(storage.allocator_00, {T::native_vtable, T::native_virtual0, this, &invoke_trim});
}
template<class T> NativeGuiWidgetPool<T>::~NativeGuiWidgetPool() {
    allocator_list_.unbind_virtual0(storage_.allocator_00);
}
template<class T> void NativeGuiWidgetPool<T>::initialize() {
    initialize_pool<T>(allocator_list_, storage_);
}
template<> void NativeGuiWidgetPool<GuiIconPoolTraits>::initialize() {
    // 00AB2850 inlines 00AB2420's reserve(32) at the same pool+28h header,
    // in unwind state 2. CD7180 starts this retained F8BDA0 process pool.
    initialize_pool<GuiIconPoolTraits,
        &reserve_native_gui_icon_pool_table_00ab2420>(allocator_list_, storage_);
}
template<class T> void* NativeGuiWidgetPool<T>::allocate() {
    return allocate_slot<T>(storage_);
}
template<class T> void NativeGuiWidgetPool<T>::free(void* slot) noexcept {
    return_slot<T>(storage_, slot);
}
template<class T> void NativeGuiWidgetPool<T>::trim() {
    trim_empty_slabs<T>(storage_);
}
template<class T> void NativeGuiWidgetPool<T>::destroy() {
    destroy_pool<T>(allocator_list_, storage_);
}
template<class T> void* NativeGuiWidgetPool<T>::initialize_slab(void* raw, std::uint32_t index) noexcept {
    return bsp::initialize_slab<T>(raw, index); // the engine's, not this member
}
template<class T> void NativeGuiWidgetPool<T>::invoke_trim(void* context) {
    static_cast<NativeGuiWidgetPool*>(context)->trim();
}
#define BSP_GUI_WIDGET_POOL_INSTANCE(T)                                                     \
    static_assert(T::free_count_offset + 4 == T::slab_bytes);                               \
    static_assert(T::slots_per_slab <= 0xffffu);                                            \
    template class NativeGuiWidgetPool<T>;
BSP_GUI_WIDGET_POOL_INSTANCE(GuiWidgetPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiButtonPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiMoviePoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiIconPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiCurvePoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiSectionPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiLinePoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiScrollbarPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiGridPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiModelPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiSoundPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiClipBoxPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiFrameBoxPoolTraits)
BSP_GUI_WIDGET_POOL_INSTANCE(GuiAnimIconPoolTraits)
// cc13-terrain2: the BushGroup model pool (include/bsp/terrain_bush_pool_cc13.hpp).
BSP_GUI_WIDGET_POOL_INSTANCE(BushGroupModelPoolTraits)
// cc13-seg87: the Listbox pool at 00F8BC10 (include/bsp/cc13_seg87.hpp).
BSP_GUI_WIDGET_POOL_INSTANCE(GuiListboxPoolTraits)
#undef BSP_GUI_WIDGET_POOL_INSTANCE
} // namespace bsp
// The out-of-line Mit pointer-vector members of the slab table (ECX = pool+28h:
// data, count, capacity). Widget 00AA6FA0/00AA6FF0/00AA76C0/00AA7740, Button
// 00AAB8E0/00AAB930/00AABC70/00AABD00, Movie 00AACDF0. The native pool
// members inline the same code; the reconstructed common engine now reuses
// these bodies on its retained pool+28h headers. operator new is 00BF55BE
// and the free is 00BF6989, as in the native inlined copies.
namespace bsp {
__declspec(noinline) void native_gui_pool_table_set_capacity_00aa6fa0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    table.capacity = capacity;
    auto* replacement = allocate_table(capacity);
    // bsp.py show 00AA6FA0: every destination slot is null-tested; unsigned
    // Win32 address arithmetic also retains its modulo32 stepping on null.
    auto destination = reinterpret_cast<std::uintptr_t>(replacement);
    for (std::uint32_t i = 0; i < table.count; ++i, destination += 4u)
        if (destination != 0)
            ::new (reinterpret_cast<void*>(destination)) std::byte*(table.data[i]);
    if (table.data) singleton_lifetime_free(table.data);
    table.data = replacement;
}
__declspec(noinline) void native_gui_pool_table_grow_00aa6ff0(NativeGuiPoolSlabTable& table) {
    // 00AA6FF0's inline capacity/copy/free schedule is the existing 00AA6FA0 body.
    native_gui_pool_table_set_capacity_00aa6fa0(table, table.capacity * 2u + 2u);
}
__declspec(noinline) void native_gui_pool_table_push_back_00aa76c0(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    if (table.count == table.capacity) native_gui_pool_table_grow_00aa6ff0(table);
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0) // 00AA76C0's append also tests the actual slot address.
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}
__declspec(noinline) void native_gui_pool_table_reserve_00aa7740(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    if (table.capacity < capacity) native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}
} // namespace bsp
