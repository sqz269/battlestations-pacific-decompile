#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 01090054.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct NativeModelPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// A host companion over that storage. It contributes no second list, slab table
// or slot state. Explicit initialize/destroy calls reproduce native lifecycle;
// the C++ destructor does not destroy, unregister or unlock the native owner.
class NativeModelPool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d62dd0;
    static constexpr std::uint32_t native_virtual0 = 0x00b74c60;
    // A native model occupies [0, 0x184); the trailing DWORD belongs to this
    // pool and must survive every model construction/destruction operation.
    static constexpr std::size_t slot_bytes = 0x188;
    static constexpr std::size_t slot_slab_index_offset = 0x184;
    static constexpr std::size_t slab_bytes = 0x3144;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeModelPool(AllocatorListDomain&, NativeModelPoolStorage&);
    ~NativeModelPool() = default;
    NativeModelPool(const NativeModelPool&) = delete;
    NativeModelPool& operator=(const NativeModelPool&) = delete;

    void initialize_00b74b80();
    void* allocate_raw_slot_00b74d00();
    void return_raw_slot_00b74750(void* slot);
    void trim_empty_slabs_00b74c60(); // no internal critical-section entry
    void destroy_00b74690();         // does not invoke any model destructor
    NativeModelPoolStorage& storage() noexcept { return storage_; }
    const NativeModelPoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    NativeModelPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b74530() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using NativeModelPoolAtexit = int (*)(void (*)());
void bind_static_native_model_pool_01090054(NativeModelPool&);
int initialize_static_native_model_pool_00cd7f00(
    NativeModelPoolAtexit register_atexit = &std::atexit);
void destroy_static_native_model_pool_00ce0e50();
void* allocate_native_model_slot_00b74eb0();
// Native 00B748C0 receives the slot in ECX, then calls the pool with a stack
// argument. This function exposes an ordinary new C++ interface to that action.
void return_native_model_slot_00b748c0(void* slot);

} // namespace bsp
