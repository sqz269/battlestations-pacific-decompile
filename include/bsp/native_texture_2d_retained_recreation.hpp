#pragma once

#include "bsp/d3d9_texture.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/native_texture_device_reset.hpp"

namespace bsp {

// Borrow the actual renderer publication, immutable native profile words,
// existing memory ownership domain and reset surface profile. The import cell
// must contain the real, live D3DXCreateTextureFromFileInMemoryEx export.
// No resource, retained-source, renderer, COM or destructor owner is copied.
struct NativeTexture2DRetainedRecreationContext {
    const void* volatile& actual_renderer_00f8d394;
    NativeRetainedMemoryOwnerContext& retained_memory;
    const volatile std::uint32_t* actual_texture_profile_00d61948;
    const NativeTextureResetSurfaceProfile& reset_surfaces;
    const volatile CreateTextureFromMemory& actual_d3dx_create_texture_from_memory_ex;
};

// Complete raw B3D7B0..B3D7B6: native __thiscall, ECX actual 50h owner,
// no stack arguments; JMP current owner table+20, no semantic result. Current
// D61948/+20=B3DD30 selects the existing complete actual reset-release provider.
// The reset provider's actual surface/COM/storage domain applies unchanged.
void release_native_texture_2d_retained_00b3d7b0(
    void* actual_owner, const NativeTexture2DRetainedRecreationContext&);

// Complete B3E190..B3E1EC: native __thiscall, ECX actual 50h owner, RET,
// no semantic result. Reads current renderer+1A10, maps format20 to21 (all
// other values to0), captures dimensions/mips, gets low32 length through the
// current retained stream+30 slot, RELOADS owner+4C for the direct data leaf,
// and passes the actual owner+10 output cell directly to real D3DX.
// HRESULT is ignored; no pre-release, clear, output temporary or retry is added.
//
// Supported length profile: actual D642C0/+30=BEF600, using the established
// 14h stream and 10h backing. Data uses the freshly loaded stream/backing
// directly. All accessed actual storage and imports must remain live at their
// native accesses; invalid pointers, other profiles and concurrent mutation
// are outside this interface's domain. These are new C++ context interfaces,
// not binary entry points, and do not establish gameplay or whole-reset parity.
void recreate_native_texture_2d_retained_00b3e190(
    void* actual_owner, const NativeTexture2DRetainedRecreationContext&);

} // namespace bsp
