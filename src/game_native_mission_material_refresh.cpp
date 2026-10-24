#include "bsp/game_native_mission_material_refresh.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_material_texture_refresh.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {
NativeMaterialTextureNameLiterals name_literals(GameNativeReadOnlyData& data) {
    const auto* const empty = static_cast<const char*>(data.data_at(0x00ce3a0c, 1));
    if (*empty != '\0')
        throw std::logic_error("Material texture refresh requires actual CE3A0C empty literal");
    return {game_native_renderer_scalar_process().material_texture_nil_00f8d438(), empty};
}
}

struct GameNativeMissionMaterialRefresh::Impl {
    GameNativeRendererApplication& application;
    NativeMaterialTextureRefreshContext context;
    std::vector<std::unique_ptr<NativeRendererMaterialTextureRefreshOperation>> operations;
    bool running{}, failed{};

    Impl(GameNativeRendererApplication& renderer, GameNativeReadOnlyData& data)
        : application(renderer), context(renderer.material_texture_refresh_context(name_literals(data))) {}

    void refresh() {
        if (running || failed)
            throw std::logic_error("Mission material texture refresh cannot reenter or replay failed work");
        auto operation = std::make_unique<NativeRendererMaterialTextureRefreshOperation>();
        auto* const current = operation.get();
        operations.push_back(std::move(operation)); // persistent before any native entry
        running = true;
        try {
            // Recheck the application's existing ready gate and identities;
            // this metadata borrow performs no native load or reference effect.
            const auto expected = application.material_texture_refresh_context(context.literals);
            if (&expected.cache != &context.cache || &expected.actual_owners != &context.actual_owners
                || &expected.current_lua_owner_00f8d434 != &context.current_lua_owner_00f8d434
                || expected.renderer_profile_00d5f0a8 != context.renderer_profile_00d5f0a8)
                throw std::logic_error("Mission material refresh requires the same application domains");
            void* const renderer = application.publication_00f8d394();
            if (!renderer || *static_cast<const volatile std::uint32_t*>(renderer) != 0x00d5f0a8
                || !context.renderer_profile_00d5f0a8
                || context.renderer_profile_00d5f0a8[0x120 / 4] != 0x00b24dd0)
                throw std::logic_error("Mission material refresh requires current D5F0A8 slot120 B24DD0");
            NativeLuaServiceBindings::Activation activation(application.lua_service_binding());
            refresh_native_renderer_material_textures_00b24dd0(renderer, context, *current);
            running = false;
        } catch (...) {
            failed = true;
            running = false;
            throw;
        }
    }
    bool retain() const noexcept {
        if (running || failed) return true;
        for (const auto& operation : operations)
            if (operation->phase != NativeRendererMaterialTextureRefreshOperation::Phase::complete)
                return true;
        return false;
    }
};

GameNativeMissionMaterialRefresh::GameNativeMissionMaterialRefresh(
    GameNativeRendererApplication& application, GameNativeReadOnlyData& data)
    : impl_(std::make_unique<Impl>(application, data)) {}
GameNativeMissionMaterialRefresh::~GameNativeMissionMaterialRefresh() noexcept {
    if (impl_ && impl_->retain()) std::terminate();
}
void GameNativeMissionMaterialRefresh::refresh_after_model_load() {
    impl_->refresh();
}
bool GameNativeMissionMaterialRefresh::requires_process_retention() const noexcept {
    return impl_->retain();
}
} // namespace bsp::game
