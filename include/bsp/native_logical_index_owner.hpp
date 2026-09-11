#pragma once

#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Borrow at least the first two DWORDs of each observed immutable native table.
// Contents remain original code-address tokens; these are not host callbacks.
struct NativeLogicalIndexPhysicalProfiles {
    const volatile std::uint32_t* private_index_00d61e10;
    const volatile std::uint32_t* pooled_index_00d61e58;
};

// Borrow the application's current renderer/global synchronization state, the
// physical-index owner's actual services, and initialized logical pool storage.
struct NativeLogicalIndexOwnerContext {
    const void* volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativePhysicalBufferOwnerContext& actual_physical;
    const NativeLogicalIndexPhysicalProfiles& actual_physical_profiles;
    void* actual_logical_index_pool_0108fe50;
};

// Complete 00B4B6F0; native ECX logical owner, RET. Borrow actual 24h storage:
// physical pointer+08 and flags+10, with a real intrusive LONG at physical+04.
// Dynamic flags unregister from the captured physical; renderer removal uses
// the current global; physical release reloads logical+08 after that removal.
// At count zero, only observed immutable index profiles D61E10 (private) and
// D61E58 (pooled) are in-domain. Their BD30E0 invoker rereads the profile before
// dispatching the exact deleting entry with flag1. No callback fallback exists.
void destroy_native_logical_index_stream_00b4b6f0(void* actual_logical,
    NativeLogicalIndexOwnerContext&);

// Complete 00B4C1F0; ECX owner, stack flags, EAX original address, RET4. Always
// destroy first; bit0 returns the raw28h slot to the actual 0108FE50 pool.
void* delete_native_pooled_logical_index_stream_00b4c1f0(void* actual_logical,
    std::uint32_t flags, NativeLogicalIndexOwnerContext&);

// Complete 00B495E0; ECX pool, stack raw28h slot, RET4. Borrow initialized
// pool+0C critical section/+24 depth/+28 slab-pointer array/+34 lowest slab.
// Slot+24 holds its slab index. Each slab has32 slots; WORD free indices start
// at+500 and the WORD free count is+540. No scope guard is added to this leaf.
void return_native_logical_index_slot_00b495e0(void* actual_pool, void* raw_slot);

// New C++ interfaces, not original game ABI replacements. The native guard is
// uninitialized when entry is disabled; an entry-disabled/exit-enabled change
// is outside the valid execution domain. Entry renderer is retained for guard
// cleanup; its current lock and the current mode still govern that cleanup.
} // namespace bsp
