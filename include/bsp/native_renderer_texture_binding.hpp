#pragma once

#include "bsp/native_cube_texture_owner.hpp"
#include "bsp/native_texture_2d_owner.hpp"
#include "bsp/native_volume_texture_owner.hpp"

namespace bsp {

// Borrow at least eight immutable original-token DWORDs for each reached
// profile. All have slot0=BD30E0; +04 selects B3F590/B3F410/B3F430 and +1C
// selects B3CEA0/B3CF90/B3D0C0 respectively. Tokens select established source
// operations; none is a callable host pointer or an injected callback.
struct NativeRendererTextureBindingProfiles {
    const volatile std::uint32_t* actual_texture_2d_00d61948;
    const volatile std::uint32_t* actual_cube_00d61870;
    const volatile std::uint32_t* actual_volume_00d618b0;
};

// All lifetime providers borrow their actual application storage. They share
// the real notification publication, synchronization and string allocation
// domains. The initialized canonical texture/cube/volume/surface pools,
// retained-memory providers and 2D support/lifetime services remain those
// admitted by the owner APIs. No renderer, pool or reference count is created.
struct NativeRendererTextureBindingContext {
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativeTexture2DOwnerContext& actual_texture_2d_owner;
    NativeCubeTextureOwnerContext& actual_cube_owner;
    NativeVolumeTextureOwnerContext& actual_volume_owner;
    const NativeRendererTextureBindingProfiles& actual_profiles;
};

// Complete four-byte native ECX-input/EAX-result/RET leaves. Read borrowed
// actual COM pointer +10, without validation, AddRef, Release or creation.
void* __fastcall native_cube_texture_get_com_00b3cf90(const void* actual_owner) noexcept;
void* __fastcall native_volume_texture_get_com_00b3d0c0(const void* actual_owner) noexcept;

// Full B24710: native ECX renderer, stack sampler/logical texture, RET8;
// no semantic EAX result. Enter the optional guard before outer identity at
// renderer+DWORD(sampler*ACh+504h). On change, reread/capture old, publish and
// retain new before decrementing/releasing old through its CURRENT profile.
// Original incoming owner selects its CURRENT +1C borrowed COM getter after
// old destruction. Then reload device+1A10 and its current SetTexture+104.
// Null input still calls SetTexture(NULL). Unsigned sampler>=16 adds F1h
// modulo2^32. Only changed, returning COM work increments current +1BC4.
void bind_native_renderer_texture_00b24710(void* actual_renderer,
    std::uint32_t sampler, void* actual_input_texture,
    NativeRendererTextureBindingContext&);

// Reached raw slot/owner/profile/COM/renderer storage must remain valid at
// native accesses, including wrapped sampler addresses. No HRESULT gate,
// rollback, bounds repair or unsupported-profile fallback is added. Skipped
// guard entry leaves native local storage uninitialized: disabled-entry to
// enabled-exit mode transitions are outside its valid domain. Normal leave
// is disarmed; exceptional cleanup uses full B21110 and terminates a second
// C++ cleanup exception. New Win32 source interface, not a game ABI replacement.

} // namespace bsp
