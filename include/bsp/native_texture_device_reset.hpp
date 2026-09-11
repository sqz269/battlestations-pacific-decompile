#pragma once

#include <cstdint>

namespace bsp {

// Borrow the application's actual D619A0 profile words through byte +3F.
// Every reached cached wrapper is actual 34h NativeSurfaceOwnerStorage with
// current profile token D619A0, +14=B3CC80 and +3C=B3D510. Current profile/slot
// words select those existing complete source providers; numeric game code
// words are never invoked as host function pointers. No owned wrapper, virtual
// service callback, registry snapshot or replacement COM object is supplied.
struct NativeTextureResetSurfaceProfile {
    const volatile std::uint32_t* actual_surface_profile_00d619a0;
};

// Complete B3DD30: native ECX actual 50h texture-2D owner, RET, no own EH or
// semantic return. The signed level loop reloads +40h and +44h at native
// accesses and calls current surface+3Ch without a null guard. Afterward the
// captured texture+10h gets AddRef/Release, followed by a freshly loaded
// texture+10h Release and clear only after that call returns.
void release_native_texture_2d_for_reset_00b3dd30(
    void* actual_owner, const NativeTextureResetSurfaceProfile&);

// Complete B3DD90: native ECX owner, stack device, RET4, no own EH or semantic
// result. The source API explicitly borrows its two four-byte stack cells:
//
// device_argument_and_texture_output: caller seeds the actual input device
// pointer bits. Native entry ESP S places it at S+4. The SAME cell is passed as
// ppTexture, reread after COM callbacks, and conditionally released/cleared.
// Unsupported low-nibble pools use the original device argument bits as pool.
//
// reused_surface_output: source seeds this with actual_owner bits at entry,
// matching native PUSH ECX at S-4. The SAME cell is used for every surface-level
// output, reread after the surface bind callback, and never cleared per level.
//
// The cells are writable, disjoint native four-byte extents, disjoint from
// owner/array/profile storage, and live throughout the call. Their addresses
// and contents remain observable to the actual COM methods. The new C++ API
// does not require a host stack layout; callers needing native relative cell
// positions supply surface=S-4 and device=S+4 themselves.
//
// All accessed owners, current raw records, profile words and COM outputs must
// remain valid at their native accesses. No preflight, HRESULT gate, rollback,
// null repair, stable array or changed reference ownership is introduced.
void restore_native_texture_2d_after_reset_00b3dd90(
    void* actual_owner, void* device_argument_and_texture_output,
    void* reused_surface_output, const NativeTextureResetSurfaceProfile&);

// Complete genuine cube/volume callbacks: B33F10 is RET; B33F20 is RET4.
// Neither accesses the incoming owner/device or any application storage.
void native_texture_reset_noop_00b33f10(void* actual_owner) noexcept;
void native_texture_restore_noop_00b33f20(
    void* actual_owner, void* actual_device) noexcept;

} // namespace bsp
