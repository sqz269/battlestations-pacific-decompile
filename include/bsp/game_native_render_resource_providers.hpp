#pragma once

namespace bsp {
struct NativeRuntimeTextureCreationContext;
struct NativeRendererSurfaceFactoryContext;
struct NativeTextureSurfaceGetterContext;
struct NativeRenderTextureSurfaceOwnerContext;
struct NativeRenderResourcesDirectTerminalDomain;
}
namespace bsp::game {
// Borrowed view of address-stable providers retained by one ready renderer
// application. The view owns nothing; it must not outlive that application.
// Keep every invocation's acquired storage, native survivor and required host
// companion alive through native retirement and subsequent host quiescence.
// No factory is called by borrowing this view, and no failure is rolled back.
struct GameNativeRenderResourceProviders {
    NativeRuntimeTextureCreationContext& runtime_textures;
    // Both render-target B2A7C0 and depth-surface B2A9A0 use this SAME context.
    NativeRendererSurfaceFactoryContext& surfaces;
    NativeTextureSurfaceGetterContext& texture_surfaces;
    NativeRenderTextureSurfaceOwnerContext& holders;
    // Retained for later explicit producer/lifetime admission. Borrowing does
    // NOT install it into render_resources_lifetime().direct_terminals.
    NativeRenderResourcesDirectTerminalDomain& direct_terminals;
};
} // namespace bsp::game
