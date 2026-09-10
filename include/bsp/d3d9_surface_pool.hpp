#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Caller supplies the one canonical owner representing native 0108db00.
// The critical-section bytes hold an actual Win32 CRITICAL_SECTION initialized
// by the implementation. No field initializers replace native storage preimages.
struct D3D9SurfacePoolStorage {
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
class D3D9SurfacePool {
public:
    static constexpr std::uint32_t native_vtable = 0x00d6193c;
    static constexpr std::uint32_t native_virtual0 = 0x00b3e390;
    // A native surface occupies [0, 0x34); the trailing DWORD belongs to this
    // pool and must survive every surface construction/destruction operation.
    static constexpr std::size_t slot_bytes = 0x38;
    static constexpr std::size_t slot_slab_index_offset = 0x34;
    static constexpr std::size_t slab_bytes = 0x744;
    static constexpr std::uint32_t slots_per_slab = 32;

    D3D9SurfacePool(AllocatorListDomain&, D3D9SurfacePoolStorage&);
    ~D3D9SurfacePool() = default;
    D3D9SurfacePool(const D3D9SurfacePool&) = delete;
    D3D9SurfacePool& operator=(const D3D9SurfacePool&) = delete;

    void initialize_00b3ec60();
    void* allocate_raw_slot_00b3ed40();
    void return_raw_slot_00b3d860(void* slot);
    void trim_empty_slabs_00b3e390(); // no internal critical-section entry
    void destroy_00b3e2b0();         // does not invoke any surface destructor
    D3D9SurfacePoolStorage& storage() noexcept { return storage_; }
    const D3D9SurfacePoolStorage& storage() const noexcept { return storage_; }
    static std::uint32_t live_slab_index(const void* slot) noexcept;

private:
    AllocatorListDomain& allocator_list_;
    D3D9SurfacePoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
    void free_table_unwind_00b3d370() noexcept;
};

// Explicit adapter binding for the actual static owner. Storage, list/domain
// and companion must outlive every registered callback. An unbound adapter fails;
// it never creates another pool or allocator-list head. Startup is called once.
using D3D9SurfacePoolAtexit = int (*)(void (*)());
void bind_static_d3d9_surface_pool_0108db00(D3D9SurfacePool&);
int initialize_static_d3d9_surface_pool_00cd7b40(
    D3D9SurfacePoolAtexit register_atexit = &std::atexit);
void destroy_static_d3d9_surface_pool_00ce0c90();
void* allocate_d3d9_surface_slot_00b3f2a0();
// Native 00B3DCC0 receives the slot in ECX, then calls the pool with a stack
// argument. This function exposes an ordinary new C++ interface to that action.
void return_d3d9_surface_slot_00b3dcc0(void* slot);

} // namespace bsp
