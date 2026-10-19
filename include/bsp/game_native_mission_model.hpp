#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeModelEnvironment;
struct NativeMaterialDestructionAccess;
struct NativeMaterialEffectCacheContext;
class NativeModelReference;
class NativeLuaServiceBindings;
struct NativeInstanceGeometryAccess;
class NativeRenderGroupModels;
class NativeTracelineRenderServices;
}
namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameInputRuntime;

// One optional mission diagnostic domain over the application's canonical
// geometry/material/effect/string/node owners. Construction binds services only;
// explicit loading performs the installed asset reads. Keep this domain before
// render-resource initialization and through command retirement/shared drain.
class GameNativeMissionModel final {
public:
    GameNativeMissionModel(GameHostLog&, GameNativeRendererApplication&, GameVfsHost&,
        GameSingletonHost&, GameNativeReadOnlyData&, GameInputRuntime&);
    ~GameNativeMissionModel() noexcept;
    GameNativeMissionModel(const GameNativeMissionModel&) = delete;
    GameNativeMissionModel& operator=(const GameNativeMissionModel&) = delete;

    NativeMaterialEffectCacheContext& effect_cache();
    NativeMaterialDestructionAccess& material_lifetime() noexcept;
    NativeModelEnvironment& model_environment() noexcept;
    NativeLuaServiceBindings& lua_services() noexcept;
    NativeInstanceGeometryAccess& instance_geometry() noexcept;
    NativeRenderGroupModels& group_models() noexcept;
    NativeTracelineRenderServices& render_services() noexcept;
    const volatile std::uint32_t* profile(const void* actual_owner) const;

    // One retained native-reader/mesh/subset/material load. The path is a VFS
    // logical name. Unknown/missing serialized data never gets a synthetic
    // shader, buffer, transform or texture. Failed operations cannot replay.
    void load_installed_ship(const char* logical_name);
    std::size_t drawable_count() const noexcept;
    void* drawable(std::size_t) const;
    NativeModelReference& drawable_reference(std::size_t) const;
    std::size_t section_count() const noexcept;
    const char* installed_name() const noexcept;
    bool requires_process_retention() const noexcept;

    // After ordinary commands/scene-root references have retired, release each
    // model creator through its real current native terminal, then close the
    // shared effect caller. No failure rollback or implicit destructor cleanup.
    void retire_drawables();
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
