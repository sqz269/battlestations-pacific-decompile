#pragma once

#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Borrow at least the first two DWORDs of each observed immutable native table;
// creation additionally needs private_index[5] for actual current14 Attach.
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

class NativeLogicalVertexDeviceRecreation;
// Creation borrows the SAME existing lifetime context. The shared B29670
// provider is already required by the vertex owner; no second renderer or
// recreation implementation is introduced here. It must execute actual device
// recreation if reached, never return success from a no-op.
struct NativeLogicalIndexCreationContext {
    NativeLogicalIndexOwnerContext& lifetime;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_logical_profile_00d61de0;
    NativeLogicalVertexDeviceRecreation& actual_device_recreation;
    // B4BF30's unwritten pool local at entry ESP-20h, consumed only when the
    // flags low nibble exceeds3. These are supplied preimage bits, not a default.
    std::uint32_t native_pool_stack_bits;
};

// Complete actual24h ctor, preserving trailing slab index+24 in its28h slot.
// ECX owner; stack count,format,flags; EAX owner; RET0Ch. Always creates a
// private physical index buffer, including dynamic flags. Uses actual D3D9
// CreateIndexBuffer, native retry predicate, required recreation, current14
// Attach, unconditional temporary COM Release, and optional guard unwind.
void* construct_native_logical_index_stream_00b4bf30(void*, std::uint32_t count,
    std::uint32_t format, std::uint32_t flags, NativeLogicalIndexCreationContext&);

// Full32-slot slab initialization and actual locked pool allocation. B48F70:
// ECX slab, stack index, EAX slab, RET4. B4B0A0: ECX pool, EAX slot, RET.
// Slab544h contains28h slots, WORD free list+500/count+540; no repaired faults.
void* initialize_native_logical_index_slab_00b48f70(void*, std::uint32_t index) noexcept;
void* allocate_native_logical_index_slot_00b4b0a0(void* actual_pool);
// B4B380 discards incoming ECX and tail-jumps with canonical pool0108FE50.
void* allocate_native_logical_index_stream_00b4b380(void* actual_pool_0108fe50);

// B22D70, ECX actual12-byte header, stack signed capacity bits, RET4. Returning
// free is followed by pointer/capacity publication, including Ghidra's9-byte gap.
void reserve_native_renderer_index_pointer_array_00b22d70(void*, std::uint32_t capacity);
// Full B288B0, ECX actual renderer, stack count,flags,format, EAX raw owner,
// RET0Ch. Appends actual+1AB8, then iff current58 returns AL1 actual+19C4;
// neither registration retains. Optional caller-owned creator publication is
// filled immediately after construction and before either append, so a later
// host exception cannot hide the acquired raw identity. It must begin null.
void* create_native_registered_index_stream_00b288b0(void* actual_renderer,
    std::uint32_t count, std::uint32_t flags, std::uint32_t format,
    NativeLogicalIndexCreationContext&, void** acquired_before_registration = nullptr);

class NativeLogicalIndexReference;
struct NativeLogicalIndexCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeLogicalIndexReference&) noexcept;
};
// One canonical companion borrowing the SAME actual+04. Binding does not
// initialize or retain. CURRENT D61DE0/BD30E0/B4C1F0 terminal only; retire the
// companion after actual destruction/pool return. No semantic stream wrapper.
class NativeLogicalIndexReference final : public RenderCommandReference {
public:
    NativeLogicalIndexReference(void* actual_stream, NativeLogicalIndexCreationContext&,
        NativeLogicalIndexCompanionDisposal);
    ~NativeLogicalIndexReference() override;
    NativeLogicalIndexReference(const NativeLogicalIndexReference&) = delete;
    NativeLogicalIndexReference& operator=(const NativeLogicalIndexReference&) = delete;
    void* storage() const noexcept { return actual_stream_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    void* actual_stream_;
    NativeLogicalIndexCreationContext& context_;
    NativeLogicalIndexCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
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
