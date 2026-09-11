#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_node_pool_payload_bytes = 0x174;
inline constexpr std::size_t native_node_pool_slot_bytes = 0x178;
inline constexpr std::size_t native_node_pool_slab_bytes = 0x2f44;

// B6DD10: ECX actual2F44h slab, stack slab index; EAX same slab; RET4.
// Initialize32 trailing slot IDs/free indices and the free-count WORD. Preserve
// every174h payload and the slab's last two padding bytes. No owner construction.
void* initialize_native_node_pool_slab_00b6dd10(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// B6EB00: ECX actual initialized38h pool; EAX raw178h slot; RET.
// Real Win32 critical section+0C; depth+24; table/count/capacity+28/+2C/+30;
// earliest nonfull slab+34. Uses the shared native CRT allocation service.
// Native allocation failures retain the lock/depth/publications; no rollback.
void* allocate_native_node_pool_slot_00b6eb00(void* actual_initialized_pool);

// B6ED70: replace incoming ECX (ignored allocation size) with global0108FF58;
// tail JMP B6EB00. This interface borrows that actual initialized global owner.
void* allocate_native_node_00b6ed70(void* actual_pool_0108ff58);

// B6E490: ECX actual pool, stack actual raw slot; RET4. Push its slot index on
// the slab's free stack and lower earliest if necessary. No validation, payload
// clearing, owner destruction, or slab reclamation is present in native code.
void return_native_node_pool_slot_00b6e490(
    void* actual_initialized_pool, void* actual_raw_slot);

// B6E670: ECX actual raw slot; select0108FF58; CALL B6E490; RET.
// Used for constructor-failure cleanup; the owner was not fully constructed.
void return_native_node_00b6e670(
    void* actual_raw_slot, void* actual_pool_0108ff58);

// New MSVC Win32 interfaces, not binary entry replacements. B6E980/CD7D10 pool
// startup and B6E3D0/CE0E20 shutdown remain separate dependencies. Never replace
// the required actual pool with zeroed storage. B6F5A0 constructs the174h node
// prefix in this raw allocation; NativeNodeBinding/SceneNodeAttachment are
// companions referencing that owner and cannot substitute for physical storage.
} // namespace bsp
