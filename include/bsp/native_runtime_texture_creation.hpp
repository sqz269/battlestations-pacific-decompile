#pragma once
#include "bsp/native_texture_2d_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
struct NativeRendererDeviceRecreationContext;

// Same canonical serial used by all named/unnamed texture producers. Full
// B33FC0..B34006: ECX owner; COM/flags stack words; EAX owner; RET8. Does not
// retain COM or touch +18, and preserves pool metadata after the 24h prefix.
void* construct_native_logical_texture_unnamed_00b33fc0(void* actual_owner,
    void* borrowed_com, std::uint32_t flags, std::uint32_t& actual_serial_0108d6e8);

struct NativeRuntimeTextureConstructionContext {
    NativeTexture2DOwnerContext& owners;
    // Readable original literal D619E4, including the seventeenth NUL byte.
    const void* actual_handmade_texture_literal_00d619e4;
};
struct NativeRuntimeTextureConstructionAcquired {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    int unwind_state{-1};
    void* owner{};
    // Actual private COM-output/diagnostic preimages, never zero fallbacks.
    alignas(4) unsigned char descriptor[32];
    alignas(4) unsigned char first_name[8];
    alignas(4) unsigned char diagnostic[12];
};

// Full B3F7B0..B3F92E, including native four-state C++ cleanup projection.
// Original ECX raw50h owner, COM/saved-width/saved-height/flags, EAX owner,
// RET10h. Retains the actual COM texture, reads current tables and ignored
// GetLevelDesc output, and preserves both diagnostic-string lifetimes. Retain
// acquired storage after a failure; no COM release or owner rollback is added.
void* construct_native_runtime_texture_2d_00b3f7b0(void* actual_owner,
    IDirect3DTexture9* input, std::uint32_t saved_width, std::uint32_t saved_height,
    volatile std::uint32_t flags, NativeRuntimeTextureConstructionContext&,
    NativeRuntimeTextureConstructionAcquired&);

struct NativeRuntimeTextureCreationContext {
    NativeRuntimeTextureConstructionContext& construction;
    NativeRendererSynchronizationGlobals& synchronization;
    NativeRendererDeviceRecreationContext* recreation; // Required when native retry is reached.
    volatile std::uint32_t& allocation_bytes_0108d4bc;
    const volatile float& unsigned_dword_fix_00ce3978;
    const volatile double& unsigned_counter_fix_00d57da0;
};
// Retained mutable five-DWORD call cells. B2A20B replaces levels with raw
// slot bits before state1; cleanup reads that CURRENT word. When accounting
// is reached, format/flags also become the original float/CW scratch words.
// Keep all cells live/address-stable through callbacks and cleanup; no stack ABI claim.
struct NativeRuntimeTextureCreationArguments {
    std::uint32_t width, height, levels, format, flags;
};
struct NativeRuntimeTextureCreationAcquired {
    enum class Phase { fresh, running, constructing, complete, failed };
    Phase phase{Phase::fresh};
    int unwind_state{-1};
    NativeRendererOptionalGuardStorage guard;
    // The original switch leaves its stack slot untouched for low nibbles>3.
    // Supply a readable preimage if exercising that domain; never infer DEFAULT.
    std::uint32_t native_pool_slot;
    // One live typed output cell, passed directly to the genuine COM entry.
    // Source current accesses use volatile views; the object itself is not volatile.
    IDirect3DTexture9* com_output{};
    // Phase/state/result/raw_slot/owner/return diagnostics are implementation-owned
    // across callbacks. raw_slot is captured evidence, not current cleanup authority.
    HRESULT create_result{};
    void* raw_slot{};
    void* owner{};
    bool raw_slot_returned{};
    NativeRuntimeTextureConstructionAcquired construction;
};

// Full B2A070..B2A360 plus original four-entry switch table. ECX renderer,
// width/height/levels/format/flags stack words; EAX creator owner, RET14h.
// Current real CreateTexture; exact retry predicate and real B29670; canonical
// pool/constructor; x87 accounting iff flags&10h; append borrowed owner to
// renderer+1B00; diagnostic COM pair then release creator COM; optional leave.
// The mutable argument block supplies current source words at their native
// read sites and retains the overwritten levels word for current slot cleanup.
// Accounting preserves native format/flags scratch-byte writes for later observers.
// CreateTexture admits only its genuine MSVC Win32 native stdcall target; raw
// DWORD format/pool call types do not admit ISO-compatible SDK-enum stand-ins.
// No failed-HRESULT null substitute, owner cleanup, retry timeout,
// alternate pool or native ABI/SEH claim. All contexts/publications belong to
// the same actual renderer/string/owner/pool/synchronization domain.
void* create_native_runtime_texture_2d_00b2a070(void* actual_renderer,
    volatile NativeRuntimeTextureCreationArguments&,
    NativeRuntimeTextureCreationContext&, NativeRuntimeTextureCreationAcquired&);
} // namespace bsp
