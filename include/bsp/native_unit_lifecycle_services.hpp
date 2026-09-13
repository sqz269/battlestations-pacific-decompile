#pragma once

#include <cstdint>

namespace bsp {

// Borrow one actual world selected from E188A8. These are existing field
// lvalues, not a world layout, snapshot or replacement publication.
struct NativeReconDirtyWorldView {
    void* canonical_world;
    const volatile std::int32_t& local_player_index_18ec;
    volatile std::uint8_t& unit_lists_built_193c;
};

class NativeReconDirtyAccess {
public:
    virtual ~NativeReconDirtyAccess() = default;
    // Side-effect-free alias lookups at their native read points. No allocation,
    // normalization or bounds fallback. The input index must address readable
    // F874BC storage, just as native ECX does; the native routine does not bound it.
    // Null means the actual pointer-table entry is null, not an absent binding.
    virtual volatile std::uint8_t* current_slot_dirty_25(std::int32_t index) = 0;
    virtual NativeReconDirtyWorldView current_world_00e188a8() = 0;
    // Same captured world; resolve [world+18CC+player_index*4]->+28 at this point.
    // The original code has no null-player check on an in-range index.
    virtual const volatile std::int32_t& player_context_28(
        void* canonical_world, std::int32_t player_index) = 0;
};

// Complete 00803BA0..00803BD7, 56 bytes. Native ECX=signed recon index, RET,
// no calls or stack arguments. Mark a present slot, then capture the world.
// Signed player indices 0..7 alone allow context comparison and cache clear.
void mark_native_recon_slot_dirty_00803ba0(std::int32_t index,
    NativeReconDirtyAccess&);

struct NativeControlledListenerPublication {
    void* volatile& controlled_listener_00e188dc;
    void* volatile& renderer_00f8d39c;
};

class NativeControlledListenerRenderer {
public:
    virtual ~NativeControlledListenerRenderer() = default;
    // Required complete B0D7B0 provider: write captured renderer+1C0, then read
    // its current +30 and, if nonnull, call B4EC90 on that object. B4EC90 marks
    // +250 and repeatedly propagates root registration from the current list.
    // Native ECX=renderer, one stack handle, RET4. No successful default exists.
    virtual void set_listener_00b0d7b0(void* captured_renderer, void* handle) = 0;
};

// Complete 004BCA80..004BCA92, 19 bytes. Native ECX=handle, no stack inputs,
// bare RET. Store E188DC before loading F8D39C and invoking the required update.
// No native scene owner, global, receiver cast, null gate or rollback is added.
void publish_native_controlled_listener_004bca80(void* handle,
    NativeControlledListenerPublication, NativeControlledListenerRenderer&);

// New C++ interfaces. Callers must supply actual aliases/providers; native
// exceptions, asynchronous mutation and gameplay integration remain unproved.
} // namespace bsp
