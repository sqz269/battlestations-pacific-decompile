#pragma once

#include <memory>

namespace bsp::game {
class GameNativeReadOnlyData;
class GameNativeRendererApplication;

// Persistent Source binding for original B24DD0 -> B19000 over this SAME
// application's effect registry, Lua publication, texture cache and owners.
// Construct/call only while the renderer is ready, before actual BeginScene.
// The caller requests one refresh after each completed model-loading batch;
// this class does not load models or invent missing environment parameters.
class GameNativeMissionMaterialRefresh final {
public:
    GameNativeMissionMaterialRefresh(GameNativeRendererApplication&,
        GameNativeReadOnlyData& same_readonly_data);
    ~GameNativeMissionMaterialRefresh() noexcept;
    GameNativeMissionMaterialRefresh(const GameNativeMissionMaterialRefresh&) = delete;
    GameNativeMissionMaterialRefresh& operator=(const GameNativeMissionMaterialRefresh&) = delete;
    GameNativeMissionMaterialRefresh(GameNativeMissionMaterialRefresh&&) = delete;
    GameNativeMissionMaterialRefresh& operator=(GameNativeMissionMaterialRefresh&&) = delete;

    // Fresh persistent native operation per call, published before entry.
    // Failure is retained and cannot be replayed; the complete renderer,
    // helper, Lua services, mapped data and reached owners must survive it.
    void refresh_after_model_load();
    bool requires_process_retention() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
