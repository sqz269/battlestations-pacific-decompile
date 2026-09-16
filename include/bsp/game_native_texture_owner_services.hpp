#pragma once

#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_loading_cache.hpp"

#include <cstdint>
#include <memory>

namespace bsp::game {

// Borrow one application's already-existing native pools, publication cells,
// counters and immutable original tables. Every reference must outlive the
// owner-service bundle and every raw companion created through it.
//
// The renderer notification and surface contexts must view the same renderer
// publication and string/lifetime domains. native_strings is that same actual
// string-storage object. This bundle does not manufacture a renderer, pool,
// retained-memory owner, surface owner, callback or native table.
struct GameNativeTextureOwnerServiceInputs {
    NativeMeshPool& mesh_pool_0108fff8;
    NativeMeshSectionPool& mesh_section_pool_010901d4;
    NativeStringStorage& native_strings;
    NativeMeshConstants mesh_constants;
    const volatile std::uint32_t* mesh_profile_00d62d60;
    const volatile std::uint32_t* mesh_section_profile_00d63194;

    NativeRendererTextureNameNotificationContext& renderer_notification;
    NativeRetainedMemoryOwnerContext& retained_memory;
    NativeSurfaceOwnerContext& surfaces;
    D3D9Texture2DPool& texture_2d_pool_0108db38;
    std::uint32_t& shared_serial_0108d6e8;
    volatile std::uint32_t& texture_tracking_counter_0108daf8;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    const volatile std::uint32_t* surface_profile_00d619a0;
    const volatile std::uint32_t* texture_2d_profile_00d61948;
};

// Frame-independent owner metadata and terminal routes for native geometry and
// 2D textures. Construct exactly one canonical bundle for an application's
// supplied pool family and route every raw object admitted by this graph through
// its actual_owners() registry. The registry rejects a duplicate live identity;
// separate competing registries over the same pools are outside this contract.
//
// Construction performs no native allocation, publication, retain/release,
// renderer-device work, VFS access or texture import. Destruction does not drain
// native owners: every texture and geometry creator must already have reached
// its existing native terminal, unbound its companion and left the registry
// empty. Violating that lifetime contract terminates in the contained owners.
// The class is compiled into bsp_core but no production startup host constructs
// it yet; mapped actual tables and all borrowed contexts remain prerequisites.
class GameNativeTextureOwnerServices final {
public:
    explicit GameNativeTextureOwnerServices(
        const GameNativeTextureOwnerServiceInputs&);
    ~GameNativeTextureOwnerServices();
    GameNativeTextureOwnerServices(const GameNativeTextureOwnerServices&) = delete;
    GameNativeTextureOwnerServices& operator=(const GameNativeTextureOwnerServices&) = delete;
    GameNativeTextureOwnerServices(GameNativeTextureOwnerServices&&) = delete;
    GameNativeTextureOwnerServices& operator=(GameNativeTextureOwnerServices&&) = delete;

    NativeRenderActualOwnerRegistry& actual_owners() noexcept;
    GuiNativeGeometryOwners& geometry() noexcept;
    NativeTexture2DOwnerContext& texture_2d() noexcept;
    NativeTextureLoadOwners& texture_load_owners() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp::game
