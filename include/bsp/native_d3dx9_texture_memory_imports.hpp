#pragma once

#include "bsp/native_cube_volume_retained_recreation.hpp"
#include "bsp/native_texture_loading_cache.hpp"

namespace bsp {

// Concrete original D3DX9_40 memory-texture import set for
// NativeTextureLoadingContext. The caller owns and keeps the actual module
// loaded for every use of the returned entries. Construction resolves only the
// four exact exports and fails before texture-loading work if any is absent.
// There is no DLL search/load fallback, callback substitution or renderer.
class NativeD3dx9TextureMemoryImports final {
public:
    explicit NativeD3dx9TextureMemoryImports(HMODULE actual_d3dx9_40);
    NativeD3dx9TextureMemoryImports(const NativeD3dx9TextureMemoryImports&) = delete;
    NativeD3dx9TextureMemoryImports& operator=(
        const NativeD3dx9TextureMemoryImports&) = delete;

    ReadImageInfoFromMemory image_info_00c2dfec() const noexcept {
        return image_info_;
    }
    CreateTextureFromMemory create_texture_00c2dfe6() const noexcept {
        return create_texture_;
    }
    CreateNativeCubeTextureFromMemory create_cube_texture_00c2dfe0() const noexcept {
        return cube_volume_.cube_entry();
    }
    CreateNativeVolumeTextureFromMemory create_volume_texture_00c2dfda() const noexcept {
        return cube_volume_.volume_entry();
    }

private:
    NativeD3dx9CubeVolumeMemoryImports cube_volume_;
    ReadImageInfoFromMemory image_info_;
    CreateTextureFromMemory create_texture_;
};

} // namespace bsp
