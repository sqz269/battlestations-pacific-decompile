#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_cube_texture_slot_bytes = 0x34;
inline constexpr std::size_t native_cube_texture_slab_bytes = 0x6c4;

// B3D2A0: ECX actual6C4h slab, stack slab index, EAX same slab, RET4.
// Set WORD freecount+6C0 to32, then ordered free indices31..0 at+680 and
// DWORD slab index at each34h slot+30. Preserve owner bytes and WORD+6C2.
void* initialize_native_cube_texture_slab_00b3d2a0(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// B3F170: ECX actual initialized38h pool, EAX actual34h raw slot, RET.
// Borrow real Win32 CRITICAL_SECTION+0C and current depth+24, table+28,
// count+2C, capacity+30 and earliest+34. Grow through actual shared CRT
// allocation/free services. No owner construction, lock cleanup or rollback
// on an allocation/new-handler exception; earlier publications remain.
void* allocate_native_cube_texture_slot_00b3f170(void* actual_pool);

// B3D940: ECX actual pool, stack raw slot, RET4. Read slot+30 slab index;
// signed wrapped32-bit slot-minus-slab displacement divided by52 selects
// the WORD free index. Increment the current freecount after the store,
// including metadata aliases, and lower earliest using unsigned comparison.
void return_native_cube_texture_slot_00b3d940(
    void* actual_pool, void* actual_raw_slot);

// B3F2C0: no native stack arguments; select canonical108DB70 then tailcall
// B3F170. This new source adapter borrows that actual initialized pool.
void* allocate_native_cube_texture_00b3f2c0(void* actual_pool_0108db70);

// B3DCE0: ECX raw slot, RET; call B3D940 with canonical108DB70. This is
// raw-slot return used by constructor unwind, without owner destruction.
void return_native_cube_texture_00b3dce0(
    void* actual_raw_slot, void* actual_pool_0108db70);

// New MSVC Win32 interfaces. Reached fields/cells/slabs require valid backing
// storage and a real initialized critical section. Actual new-handler list
// traversal requires the genuine cube-pool trim binding before publication.
// No substitute pool, initializer, owner payload repair, native ABI or game proof.
} // namespace bsp
