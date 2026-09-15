#pragma once

#include <cstdint>

namespace bsp {
class NativeNodeDestructionRuntime;
struct NativeTexture2DOwnerContext;
struct NativeFrameTargetOwnerContext;

using NativeDirectionalShadowInterlocked = long (__stdcall*)(volatile long*);

// Borrow the actual providers, installed import cells and original table words.
// No owner, reference count, camera association or allocation is created here.
// Bindings stay fixed; import cells and table contents are read at native sites.
// All actual storage, canonical companions and providers must survive callbacks.
struct NativeDirectionalShadowOwnerContext {
    NativeNodeDestructionRuntime& nodes;
    NativeTexture2DOwnerContext& texture_2d;
    NativeFrameTargetOwnerContext& frame_target;
    NativeDirectionalShadowInterlocked const volatile& increment_iat_00ce221c;
    NativeDirectionalShadowInterlocked const volatile& decrement_iat_00ce2220;
    const volatile std::uint32_t* const table_00d5b5d8;
    const volatile std::uint32_t* const table_00d5e5f8;
    const volatile std::uint32_t* const table_00d61948;
    const volatile std::uint32_t* const table_00d5e600;
    // Optional actual base table; existing final-profile callers need not bind it.
    const volatile std::uint32_t* const table_00d5b574 = nullptr;
};

// Complete A8DEC0 (455 bytes): native ECX=actual 508h owner, RET. New source
// ABI supplies a context. Stamp D5B574, release actual fields in native order,
// and restore BD30F0. The sole armed cleanup restores ONLY the base on a C++
// exception; it neither finishes members nor frees storage. Camera viewport
// clear is unconditional, followed by an independent current camera reload.
void destroy_native_directional_shadow_owner_00a8dec0(
    void* actual_owner, NativeDirectionalShadowOwnerContext&);

// Complete A8FCD0 (30 bytes): native ECX=owner, stacked flags, EAX=original,
// RET4. Explicit source slot preserves the late LOW BYTE flags read following
// normal destruction. Bit0 frees in the established shared CRT domain. The
// returned identity may already be freed; a throwing destructor never frees.
void* delete_native_directional_shadow_owner_00a8fcd0(void* actual_owner,
    const volatile std::uint32_t& actual_public_flags_slot,
    NativeDirectionalShadowOwnerContext&);

// Complete A8E160 (30 bytes): base-profile deleting entry, with the same native
// ECX/stack flags/EAX/RET4 contract and late borrowed LOW BYTE flags read above.
// A direct call needs no table lookup; the flags1 allocation domain still applies.
void* delete_native_directional_shadow_base_00a8e160(void* actual_owner,
    const volatile std::uint32_t& actual_public_flags_slot,
    NativeDirectionalShadowOwnerContext&);

// Complete B7BDF0 (65 bytes): ECX=actual light, stack replacement, RET4.
// Capture old +174, skip equal, publish/retain incoming, decrement captured old,
// then dispatch its CURRENT profile/slot0 on zero. No rollback or later clear.
// Old-owner domains are D5B5D8/BD30E0/A8FCD0 and explicitly bound
// D5B574/BD30E0/A8E160; unknown profiles/targets or missing bindings diagnose.
void set_native_directional_light_shadow_owner_00b7bdf0(void* actual_light,
    void* incoming_owner, NativeDirectionalShadowOwnerContext&);

// Complete CB63D0 action (8 native bytes): original loads this from [EBP-10h]
// and jumps BD30F0. The source supplies that captured owner explicitly; it does
// not manufacture a native FH3 frame or perform any unfinished member cleanup.
void __fastcall unwind_native_directional_shadow_base_00cb63d0(
    void* actual_owner) noexcept;

// Caller provides real initialized storage and compatible disposal domains.
// A8FD30/A8FA30/A8E2E0 construction uses the separate existing construction
// module and persistent construction block; this lifetime context creates none.
// Child providers retain their explicit supported-profile and noexcept limits.
// Source diagnostics/C++ cleanup are not arbitrary native FH3/SEH, ABI or game
// proof. Unsupported bindings never receive a fabricated successful callback.
} // namespace bsp
