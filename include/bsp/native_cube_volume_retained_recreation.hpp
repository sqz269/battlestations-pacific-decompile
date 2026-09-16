#pragma once

#include "bsp/native_memory_stream.hpp"

#include <d3d9.h>

namespace bsp {

// Concrete C2DFE0/CE2404 and C2DFDA/CE2408 import bindings. Borrow the actual
// caller-owned d3dx9_40 module for every synchronous call; resolve only the
// two named exports, with no loading/search fallback or callback injection.
// Resolution failures throw during host construction, before native work.
class NativeD3dx9CubeVolumeMemoryImports final {
public:
    using CubeEntry = HRESULT (WINAPI*)(IDirect3DDevice9*, const void*, UINT,
        IDirect3DCubeTexture9**);
    using VolumeEntry = HRESULT (WINAPI*)(IDirect3DDevice9*, const void*, UINT,
        IDirect3DVolumeTexture9**);

    explicit NativeD3dx9CubeVolumeMemoryImports(HMODULE actual_d3dx9_40);
    NativeD3dx9CubeVolumeMemoryImports(const NativeD3dx9CubeVolumeMemoryImports&) = delete;
    NativeD3dx9CubeVolumeMemoryImports& operator=(const NativeD3dx9CubeVolumeMemoryImports&) = delete;

    HRESULT create_cube(IDirect3DDevice9*, const void*, UINT,
        IDirect3DCubeTexture9** actual_output) const;
    HRESULT create_volume(IDirect3DDevice9*, const void*, UINT,
        IDirect3DVolumeTexture9** actual_output) const;
    CubeEntry cube_entry() const noexcept { return cube_; }
    VolumeEntry volume_entry() const noexcept { return volume_; }

private:
    CubeEntry cube_;
    VolumeEntry volume_;
};

// Borrow actual four-byte F8D394 publication storage (not a renderer snapshot),
// the existing actual retained-memory domain and the concrete live imports.
// Stream profile D642C0/current +30 must identify BEF600. No owner, backing,
// COM reference, renderer, profile, counter or private storage is constructed.
struct NativeCubeVolumeRetainedRecreationContext {
    const void* actual_renderer_global_f8d394;
    NativeRetainedMemoryOwnerContext& retained_memory;
    const NativeD3dx9CubeVolumeMemoryImports& imports;
};

// Full B3D7C0[52]/B3D800[52]: native ECX raw owner, no stack arguments, RET.
// If captured COM+10 is nonnull, AddRef then reload its table for Release.
// Independently reload owner+10; if nonnull, current-table Release then clear.
// A throw prevents subsequent calls/clear; neither callback has cleanup/EH.
// Retained sources and all other fields remain untouched.
void release_native_cube_texture_retained_00b3d7c0(void* actual_owner);
void release_native_volume_texture_retained_00b3d800(void* actual_owner);

// Full B3E1F0[50]/B3E230[50]: native ECX raw owner, no stack arguments, RET.
// Read current renderer publication and B1FEF0 device first; dispatch current
// stream+2C (cube) / +30 (volume) length slot+30, keeping only low DWORD; reload
// the stream for BEF610 data; invoke the simple four-argument D3DX import with
// raw owner+10 as output. Ignore HRESULT. Do not preclear/release output, alter
// retained counts/cursor, or add null guards, fallback, retry or rollback.
void recreate_native_cube_texture_retained_00b3e1f0(void* actual_owner,
    NativeCubeVolumeRetainedRecreationContext&);
void recreate_native_volume_texture_retained_00b3e230(void* actual_owner,
    NativeCubeVolumeRetainedRecreationContext&);

// All reached raw storage, current COM tables and the borrowed module remain
// valid. These are new MSVC Win32 C++ interfaces, not native ABI, SEH/FH3 or
// gameplay validation. Ordinary exceptions propagate in native call order.
} // namespace bsp
