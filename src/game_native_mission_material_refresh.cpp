#include "bsp/game_native_mission_material_refresh.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_material_texture_refresh.hpp"
#include "bsp/native_sampler_parameter_write.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
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
    struct ParameterWrite {
        NativeString key, value;
        NativeSamplerParameterWriteOperation operation;
        bool key_live{}, value_live{};
        ~ParameterWrite() { if (key_live || value_live) std::terminate(); }
    };
    std::vector<std::unique_ptr<ParameterWrite>> publications;
    bool running{}, failed{};

    Impl(GameNativeRendererApplication& renderer, GameNativeReadOnlyData& data)
        : application(renderer), context(renderer.material_texture_refresh_context(name_literals(data))) {}

    void* ready_renderer() {
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
        return renderer;
    }
    void publish_reflection(const std::string& name) {
        if (running || failed)
            throw std::logic_error("Mission material publication cannot reenter or replay failed work");
        // The record's +B98h as 004EE450 wrote it (empty without an authored
        // ReflCubemap, HDR_-prefixed whole when linearizing); an empty name
        // still publishes, clearing any previous mission's Lua value.
        if (name.size() > std::numeric_limits<std::uint32_t>::max())
            throw std::logic_error("ReflectionCubeMap source exceeds native string extent");
        auto publication = std::make_unique<ParameterWrite>();
        auto* const current = publication.get();
        publications.push_back(std::move(publication));
        running = true;
        try {
            (void)ready_renderer();
            current->key_live = true;
            current->key.assign_0041e870(context.cache.strings, "ReflectionCubeMap");
            current->value_live = true;
            current->value.resize_0041dd40(context.cache.strings,
                static_cast<std::uint32_t>(name.size()), false);
            if (!name.empty()) std::memcpy(current->value.data(), name.data(), name.size());
            NativeSamplerParameterWriteContext write_context{
                game_native_lua_globals_process().empty_string_0108ff2c(), current->operation};
            NativeLuaServiceBindings::Activation activation(application.lua_service_binding());
            void* const receiver = context.current_lua_owner_00f8d434;
            if (!receiver)
                throw std::logic_error("ReflectionCubeMap requires the current native renderer Lua owner");
            set_native_sampler_parameter_00b1b830(receiver, write_context,
                &current->key, &current->value);
            destroy_native_string_header_0041dd20(&current->value, context.cache.strings);
            current->value_live = false;
            destroy_native_string_header_0041dd20(&current->key, context.cache.strings);
            current->key_live = false;
            running = false;
        } catch (...) {
            failed = true;
            running = false;
            throw;
        }
    }

    void refresh() {
        if (running || failed)
            throw std::logic_error("Mission material texture refresh cannot reenter or replay failed work");
        auto operation = std::make_unique<NativeRendererMaterialTextureRefreshOperation>();
        auto* const current = operation.get();
        operations.push_back(std::move(operation)); // persistent before any native entry
        running = true;
        try {
            void* const renderer = ready_renderer();
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
        for (const auto& publication : publications)
            if (publication->key_live || publication->value_live
                || publication->operation.phase != NativeSamplerParameterWriteOperation::Phase::complete)
                return true;
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
void GameNativeMissionMaterialRefresh::publish_reflection_cubemap(
    const std::string& record_refl_cubemap_b98) {
    impl_->publish_reflection(record_refl_cubemap_b98);
}
void GameNativeMissionMaterialRefresh::refresh_after_model_load() {
    impl_->refresh();
}
bool GameNativeMissionMaterialRefresh::requires_process_retention() const noexcept {
    return impl_->retain();
}
} // namespace bsp::game
