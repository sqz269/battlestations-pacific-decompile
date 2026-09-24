#pragma once
#include "bsp/native_runtime_texture_creation.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include "bsp/native_render_resources_surfaces.hpp"

namespace bsp {
// The original B3D640..B3D642 is exactly RET4. ECX and its one stack word
// are unused. The extra fastcall EDX argument gives the new interface the
// original ECX/stack/RET4 placement without reading or changing any register.
void __fastcall native_texture_noop_00b3d640(void*,void*,const void*) noexcept;

struct NativeRenderTextureSurfaceOwnerArguments {
    std::uint32_t width,height,format,multisample,mode;
    NativeSurfaceOwnerStorage* external_surface;
};
static_assert(sizeof(NativeRenderTextureSurfaceOwnerArguments)==0x18);
// Pure immutable metadata over six initialized live DWORD cells, in native
// width/height/format/multisample/mode/external-pointer order. The actual cells
// and their addresses survive all callbacks; no aggregate is placed over them.
struct NativeRenderTextureSurfaceOwnerArgumentView {
    const volatile std::uint32_t* const words;
};
struct NativeRenderTextureSurfaceOwnerContext {
    NativeRuntimeTextureCreationContext& textures;
    NativeTextureSurfaceGetterContext& levels;
    NativeRendererSurfaceFactoryContext& render_targets;
    NativeTextureSurfaceReferenceIncrement const volatile& actual_decrement_00ce2220;
};
struct NativeRenderTextureSurfaceOwnerAcquired {
    enum class Phase { fresh,running,complete,failed };
    Phase phase{Phase::fresh};
    int unwind_state{-1};
    void* owner{};
    NativeRuntimeTextureCreationArguments texture_arguments{};
    NativeRuntimeTextureCreationAcquired texture_creation;
    NativeTextureSurfaceGetterAcquired surface_getter;
    NativeRendererSurfaceFactoryAcquired target_creation;
    alignas(4) std::uint32_t zero_vector[4];
};

// B4E020..B4E13A: ECX actual18h owner; width,height,format,multisample,
// low mode byte, optional external surface; EAX owner; RET18h. Mode zero
// publishes the texture's retained level0 at+C. Nonzero publishes it at+10,
// then either retains external at+C or creates an actual separate target.
// Texture+54 finally dispatches the genuine B3D640 no-op with four zero words.
// Only refcounted base cleanup is armed; no partial-resource rollback is added.
void* construct_native_render_texture_surface_owner_00b4e020(void* actual_owner,
    const volatile NativeRenderTextureSurfaceOwnerArguments&,
    NativeRenderTextureSurfaceOwnerContext&,NativeRenderTextureSurfaceOwnerAcquired&);
// Same shared body with actual caller argument cells. Read mode BYTE first,
// then format/height, write owner14, then read initial width. After the genuine
// texture/level callbacks, reread external; the separate-target path rereads
// multisample then width, retaining the original height/format. View metadata
// is disjoint from all mutable native storage. No argument snapshot is made.
void* construct_native_render_texture_surface_owner_00b4e020(void* actual_owner,
    const NativeRenderTextureSurfaceOwnerArgumentView&,
    NativeRenderTextureSurfaceOwnerContext&,NativeRenderTextureSurfaceOwnerAcquired&);

// B4E140..B4E1E7: ECX owner, RET. StampD61EB8; capture the actual decrement
// IAT target once; release current+C,+10,+8 in order and clear each nonnull
// slot after its call. Current BD30E0/deleting-slot dispatch uses the full
// actual surface/texture lifetimes. Base-only C++ unwind; no cleanup retries.
void destroy_native_render_texture_surface_owner_00b4e140(void*,
    NativeRenderTextureSurfaceOwnerContext&);

// Full B4E410..B4E42D including repaired post-free tail: ECX owner, stack
// flags, EAX original pointer, RET4. Destroy, free iff bit0, return original.
void* delete_native_render_texture_surface_owner_00b4e410(void*,std::uint32_t flags,
    NativeRenderTextureSurfaceOwnerContext&);

// All nested contexts view the SAME actual renderer publication, canonical
// pools, strings, support manager, counters and synchronization domain. Actual
// profiles admitted here are rendererD5F0A8, textureD61948 and surfaceD619A0;
// borrowed native table views must cover renderer+88h and texture+54h. These
// source contexts/acquired frames do not claim native ABI, FH3/SEH, arbitrary
// profiles or private-stack aliases. No owner field beyond+14 is initialized.
} // namespace bsp
