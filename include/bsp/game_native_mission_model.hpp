#pragma once
#include "bsp/camera_projection.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

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

    // Incremental native-reader/mesh/subset/material loads share this one
    // application domain. Repeated logical names share retained native meshes;
    // each unit has distinct actual model nodes. True means newly loaded;
    // false means the same unit/name was already loaded. Rebinding an index or
    // replaying a failed operation throws. The caller brackets native loading.
    bool load_installed_unit(const std::string& logical_name, std::size_t unit_index);
    bool unit_loaded(std::size_t unit_index) const noexcept;
    std::size_t unit_count() const noexcept;
    std::size_t unit_drawable_count(std::size_t unit_index) const noexcept;
    std::size_t drawable_unit_index(std::size_t drawable_index) const;
    // Compose immutable authored asset matrices with the borrowed current unit
    // pose, then publish through actual B6DB10 (including graph invalidation).
    // No simulation pose is retained. False means that index is not loaded.
    bool update_unit_transform(std::size_t unit_index, const CameraMatrix&);
    // Publish factor 1/0 through B6DA70 for every flattened drawable. No live
    // visibility/liveness field is copied into this domain. Unknown => false.
    bool set_unit_visible(std::size_t unit_index, bool visible);

    // Compatibility diagnostic: unit zero in an otherwise empty domain.
    // Unknown/missing data receives no synthetic shader/buffer/transform/texture.
    void load_installed_ship(const char* logical_name);
    std::size_t drawable_count() const noexcept;
    void* drawable(std::size_t) const;
    NativeModelReference& drawable_reference(std::size_t) const;
    std::size_t section_count() const noexcept; // Unique loaded asset sections.
    // First loaded logical asset, retained for the single-ship diagnostic.
    const char* installed_name() const noexcept;
    bool requires_process_retention() const noexcept;

    // After ordinary commands/scene-root references have retired, release each
    // model creator through its real current native terminal and the cached
    // mesh creators, then close the shared effect caller. No failure rollback
    // or implicit destructor cleanup.
    void retire_drawables();
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
