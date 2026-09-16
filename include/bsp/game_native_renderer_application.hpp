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
        const NativeXLiveDeviceAdapter*);
    void copy_settings_capabilities(SettingsRendererCapabilities&) const;
    IDirect3D9& api() const;
    IDirect3DDevice9* device() const noexcept;
    NativeRendererParametersOwner& parameters() const;
    const _D3DPRESENT_PARAMETERS_& presentation() const;
    bool requires_process_retention() const noexcept;
    void drain_singletons();
    void after_native_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
