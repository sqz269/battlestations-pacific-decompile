#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include "bsp/native_texture_loading_cache.hpp"

namespace bsp {
// Complete B189F0[80], ECX actual material; stacked slot/texture; RET8.
// Sign-extend its SHORT+34 count, compare unsigned, store low16(index+1)
// BEFORE identity testing. Slots use wrapping +10+4*index; no range check.
// Publish and retain incoming, then release captured old through its CURRENT
// canonical terminal. The older GUI helper remains a checked convenience API.
void set_native_material_texture_unchecked_00b189f0(void* actual_material,
    std::uint32_t index, void* actual_texture, NativeRenderActualOwners&);

struct NativeMeshTextureFieldContext {
    NativeResourceStreamReadContext& reads;
    NativeTextureCacheContext& textures;
    NativeRenderActualOwners& owners; // SAME canonical domain as cache and material.
    const volatile std::uint32_t* renderer_profile_00d5f0a8; // through slot64.
    // reads.strings and textures.strings must borrow the SAME current raw
    // string-pool publication, gate and lifetime; no private adapter/pool.
};
struct NativeMeshTextureFieldAcquired {
    enum class Phase { empty, name, texture, slot, assignment, child,
        child_fields, child_release, texture_release, name_return, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{}, slot{};
    NativeString name; // Stable local-header projection; bytes remain after release.
    bool name_completed{}, name_cleanup_armed{}, name_returned{};
    void* child{};
    bool child_cleanup_armed{};
    void* texture{}; // One native temporary-release obligation after return.
    void* captured_texture{}; // Borrowed audit identity, possibly stale after release.
    // Each invocation owns its own persistent cache call. Its existing child
    // frames must remain alive on provider failure; no retry or default cleanup.
    NativeTextureCacheAcquired cache;
};

// Complete B93D30[294]: ECX unused; stacked material/parent handle; RET8.
// Read local name (ignore BEA010 returned header), capture current renderer64,
// call actual B319B0(name,0), read slot, retained assignment. Recognized
// TextureAddress children discard3controlDWORDs; unknown children skip/detach.
// Release temporary texture, then captured local name/current length/current
// pool. EH owns only completed name and completed current child, never texture
// or material publication. Reset child state BEFORE normal release: no retry.
// Acquired starts empty and preserves outstanding effects on failure. Keep
// failed cache/provider frames alive until their recorded obligations resolve.
void read_native_mesh_texture_field_00b93d30(void* actual_material,
    void* actual_parent_handle, NativeMeshTextureFieldContext&,
    NativeMeshTextureFieldAcquired&);

// Numeric native renderer words select the existing complete texture-cache
// source interface; they are not callable host pointers. This does not supply
// missing effect-constructor/instance-generator/subset admission. New Win32
// source interfaces, not native ABI/FH3/SEH or gameplay replacements.
} // namespace bsp
