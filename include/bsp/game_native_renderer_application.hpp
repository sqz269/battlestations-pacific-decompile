#pragma once
#include <cstdint>
#include <memory>

struct IDirect3D9;
struct IDirect3DDevice9;
struct _D3DPRESENT_PARAMETERS_;
namespace bsp {
struct RendererInitRequest;
struct NativeRendererParametersOwner;
struct SettingsRendererCapabilities;
struct ResourceLoadEventHost;
class NativeXLiveDeviceAdapter;
class XLiveLibrary;
struct NativeRendererPresentObservation;
struct NativeTextureCacheContext;
class NativeRenderActualOwnerRegistry;
struct NativeCameraEnvironment;
struct NativeNodeRawConstants;
class NativeViewportRegistry;
struct NativeCockpitViewportReleaseContext;
struct NativeRenderResourcesLifetimeContext;
class NativeRenderResourcesConstructionAcquired;
}
namespace bsp::game {
class GameSingletonHost;
class GameVfsHost;
class GameNativeReadOnlyData;
class GameNativeLuaServices;
class GameHostLog;
// Retain this source graph before native construction and through the actual
// singleton drain. Native failures/constructor-only owners require process
// retention; B32920 requires a completed device/default-surface lifetime.
class GameNativeRendererApplication final {
public:
    GameNativeRendererApplication(GameHostLog&, GameSingletonHost&, GameVfsHost&,
        GameNativeLuaServices&, GameNativeReadOnlyData&, void* const volatile& clock,
        const void* actual_platform_window_focus);
    ~GameNativeRendererApplication();
    GameNativeRendererApplication(const GameNativeRendererApplication&) = delete;
    GameNativeRendererApplication& operator=(const GameNativeRendererApplication&) = delete;
    void construct();
    void create_device(const RendererInitRequest&);
    // BECEE0's BED1E8..BED222 fragment, after actual device startup. Retain
    // the registered cache and its source bindings through the shared drain.
    void initialize_window_render_entry_cache();
    void bind_platform_services(ResourceLoadEventHost&, const volatile std::uint32_t* online,
        const NativeXLiveDeviceAdapter*, const XLiveLibrary&);
    // Full native begin/clear/end around the current frontend draw. The color
    // is a host choice; the raw frame providers retain their native branches.
    void begin_frame(std::uint32_t clear_color);
    NativeRendererPresentObservation end_frame();
    void copy_settings_capabilities(SettingsRendererCapabilities&) const;
    IDirect3D9& api() const;
    IDirect3DDevice9* device() const noexcept;
    NativeRendererParametersOwner& parameters() const;
    const _D3DPRESENT_PARAMETERS_& presentation() const;
    // The actual renderer's loading domain. Keep acquisition frames alive and
    // release ordinary returned references before the shared renderer drain;
    // its own fallback reference follows the native cache cleanup schedule.
    NativeTextureCacheContext& texture_cache() noexcept;
    NativeRenderActualOwnerRegistry& actual_owners() noexcept;
    // One camera domain shares the application's pool, type counter, node
    // lifetimes and current raw renderer. Retire native cameras/viewports and
    // forget quiescent construction records before destroying this graph.
    NativeCameraEnvironment& camera_environment() noexcept;
    const NativeNodeRawConstants& node_constants() noexcept;
    NativeViewportRegistry& viewport_registry() noexcept;
    const NativeCockpitViewportReleaseContext& cockpit_viewport_release() noexcept;
    // Full B14A10 after device startup; preserves allocation preimages.
    // Caller must complete the later native resource initialization before
    // ordinary destruction (B14A10 leaves +70 unwritten). Normal startup is
    // still gated on that B107F0 integration; this is the shared provider API.
    void* construct_render_resources();
    NativeRenderResourcesLifetimeContext& render_resources_lifetime() noexcept;
    const NativeRenderResourcesConstructionAcquired& render_resources_construction() const noexcept;
    bool requires_process_retention() const noexcept;
    void drain_singletons();
    void after_native_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
