#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_hardware_layout_slab_bytes = 0x944;

// B5FF50: native ECX raw944h slab, stack slab index, EAX same slab, RET4.
// Set WORD free count32, free indices31..0, and each48h slot's DWORD+44 index.
// Preserve all44h owner payloads and the slab's final two padding bytes.
void* initialize_native_hardware_layout_slab_00b5ff50(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// B605B0: native ECX actual initialized38h pool, EAX raw48h slot, RET.
// Use its real Win32 critical section+0C and current depth/table/count/capacity/
// earliest fields+24/+28/+2C/+30/+34. Grow through the existing shared native
// allocation service boundary. No lock cleanup or rollback on allocation throw.
void* allocate_native_hardware_layout_slot_00b605b0(void* actual_pool);

// B606F0: native incoming ECX size is ignored; replace ECX with global108FE9C,
// then tail-call B605B0. This new interface borrows that actual initialized pool.
void* allocate_native_hardware_layout_00b606f0(void* actual_pool_0108fe9c);

// B60260: native ECX rawslot, RET; select actual pool108FE9C and call complete
// B60110. This is the constructor-failure slot return, with no owner destruction.
void return_native_hardware_layout_00b60260(
    void* actual_raw_slot, void* actual_pool_0108fe9c);

// New MSVC Win32 interfaces; no substitute pool, global initializer, native
// calling-convention replacement, original CRT/exception identity, or game proof.
} // namespace bsp
