#pragma once

#include "bsp/native_frame_target_vector.hpp"
#include "bsp/native_surface_owner.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 40h Win32 group storage. No host vtable or secondary ownership.
// Constructor 00B1FBB0 leaves exactly bytes 3D..3F untouched.
struct NativeFrameTargetOwnerStorage {
    std::uint32_t vtable_00;
    std::int32_t references_04;
    NativeSurfaceOwnerStorage* colors_08[4];
    NativeSurfaceOwnerStorage* depth_18;
    NativeFrameTargetVectorStorage records_1c;
    IDirect3DSurface9* color_com_28[4];
    IDirect3DSurface9* depth_com_38;
    std::uint8_t srgb_write_3c;
    std::byte untouched_3d[3];
};
static_assert(sizeof(NativeFrameTargetOwnerStorage) == 0x40);
static_assert(offsetof(NativeFrameTargetOwnerStorage, references_04) == 4);
static_assert(offsetof(NativeFrameTargetOwnerStorage, colors_08) == 8);
static_assert(offsetof(NativeFrameTargetOwnerStorage, depth_18) == 0x18);
static_assert(offsetof(NativeFrameTargetOwnerStorage, records_1c) == 0x1c);
static_assert(offsetof(NativeFrameTargetOwnerStorage, color_com_28) == 0x28);
static_assert(offsetof(NativeFrameTargetOwnerStorage, depth_com_38) == 0x38);
static_assert(offsetof(NativeFrameTargetOwnerStorage, srgb_write_3c) == 0x3c);
static_assert(offsetof(NativeFrameTargetOwnerStorage, untouched_3d) == 0x3d);

struct NativeFrameTargetOwnerContext {
    NativeSurfaceOwnerContext& actual_surface_context;
    // Borrow the immutable original D619A0 profile. Current object profile
    // loads resolve here; slots 0/4 must be BD30E0/B3F5B0 respectively.
    // Other surface-owner profiles are outside this concrete contract.
    const volatile std::uint32_t* actual_surface_profile_00d619a0;
};

// ECX raw storage, EAX original storage, RET. Stores in original order;
// sets count one and clears members without touching the trailing padding.
NativeFrameTargetOwnerStorage* construct_native_frame_target_owner_00b1fbb0(
    void* actual_storage) noexcept;

// ECX owner, RET. Interleave each intrusive color release with its separate
// current COM Release, then depth. Clear each field after its call returns.
// Full vector disposal and base restoration include the native omitted tail.
// EH state1 disposes the vector then base; state0 restores only the base.
void destroy_native_frame_target_owner_00b1fc00(
    NativeFrameTargetOwnerStorage&, NativeFrameTargetOwnerContext&);

// ECX owner, stack flags, EAX original address, RET4. Free via shared CRT
// only after normal destruction and only if flags&1. Result may be freed.
NativeFrameTargetOwnerStorage* delete_native_frame_target_owner_00b1fcf0(
    NativeFrameTargetOwnerStorage&, std::uint32_t flags, NativeFrameTargetOwnerContext&);

// ECX group, stack slot/pointer, RET8. Unchecked native 32-bit slot indexing.
// On change: publish, retain incoming, release captured outgoing at final zero.
void set_native_frame_target_color_00b1fab0(NativeFrameTargetOwnerStorage&,
    std::uint32_t slot, NativeSurfaceOwnerStorage* incoming, NativeFrameTargetOwnerContext&);
// ECX group, stack pointer, RET4. Same ordering for depth at +18.
void set_native_frame_target_depth_00b1fb00(NativeFrameTargetOwnerStorage&,
    NativeSurfaceOwnerStorage* incoming, NativeFrameTargetOwnerContext&);
// ECX group, stack low byte, RET4. Preserve the exact byte, no normalization.
void set_native_frame_target_srgb_byte_00b1f700(
    NativeFrameTargetOwnerStorage&, std::uint8_t value) noexcept;

} // namespace bsp
