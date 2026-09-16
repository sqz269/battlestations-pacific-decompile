#include "bsp/game_native_texture_owner_services.hpp"

#include <memory>
#include <stdexcept>

namespace bsp::game {
namespace {

void require_owner_inputs(const GameNativeTextureOwnerServiceInputs& inputs) {
    if (&inputs.native_strings !=
        &inputs.renderer_notification.actual_string_storage)
        throw std::invalid_argument(
            "native texture owner services require one actual string domain");

    const auto* const notification_renderer_cell = static_cast<const volatile void*>(
        std::addressof(inputs.renderer_notification.actual_renderer_00f8d394));
    const auto* const surface_renderer_cell = static_cast<const volatile void*>(
        std::addressof(inputs.surfaces.actual_renderer_00f8d394));
    if (notification_renderer_cell != surface_renderer_cell)
        throw std::invalid_argument(
            "native texture owner services require one renderer publication cell");

    if (!inputs.mesh_profile_00d62d60 ||
        inputs.mesh_profile_00d62d60[0] != 0x00bd30e0u ||
        inputs.mesh_profile_00d62d60[1] != 0x00b74280u)
        throw std::invalid_argument(
            "native texture owner services require the actual mesh terminal table");
    if (!inputs.mesh_section_profile_00d63194 ||
        inputs.mesh_section_profile_00d63194[0] != 0x00bd30e0u ||
        inputs.mesh_section_profile_00d63194[1] != 0x00b86690u)
        throw std::invalid_argument(
            "native texture owner services require the actual mesh-section terminal table");
    if (!inputs.renderer_profile_00d5f0a8 ||
        inputs.renderer_profile_00d5f0a8[0x6c / sizeof(std::uint32_t)] != 0x00b32250u)
        throw std::invalid_argument(
            "native texture owner services require the actual renderer notification table");
    if (!inputs.surface_profile_00d619a0 ||
        inputs.surface_profile_00d619a0[0] != 0x00bd30e0u ||
        inputs.surface_profile_00d619a0[1] != 0x00b3f5b0u)
        throw std::invalid_argument(
            "native texture owner services require the actual surface terminal table");
    if (!inputs.texture_2d_profile_00d61948 ||
        inputs.texture_2d_profile_00d61948[0] != 0x00bd30e0u ||
        inputs.texture_2d_profile_00d61948[1] != 0x00b3f590u)
        throw std::invalid_argument(
            "native texture owner services require the actual texture2D terminal table");
}

} // namespace

struct GameNativeTextureOwnerServices::Impl {
    NativeRenderActualOwnerRegistry actual_owners;
    NativeMeshEnvironment meshes;
    NativeMeshSectionEnvironment sections;
    GuiNativeGeometryRegistration registration;
    GuiNativeGeometryOwners geometry;
    NativeTexture2DOwnerContext texture_2d;
    NativeTextureLoadOwners texture_load_owners;

    explicit Impl(const GameNativeTextureOwnerServiceInputs& inputs)
        : actual_owners(),
          meshes{inputs.mesh_pool_0108fff8, actual_owners, inputs.native_strings,
              inputs.mesh_profile_00d62d60},
          sections{inputs.mesh_section_pool_010901d4, actual_owners,
              inputs.mesh_section_profile_00d63194},
          registration{actual_owners, std::addressof(actual_owners),
              &NativeRenderActualOwnerRegistry::bind_callback,
              &NativeRenderActualOwnerRegistry::unbind_callback,
              &NativeRenderActualOwnerRegistry::find_callback},
          geometry(meshes, inputs.mesh_constants, sections, registration),
          texture_2d{inputs.renderer_notification, inputs.retained_memory,
              inputs.surfaces, inputs.texture_2d_pool_0108db38,
              inputs.shared_serial_0108d6e8, inputs.texture_tracking_counter_0108daf8,
              inputs.renderer_profile_00d5f0a8, inputs.surface_profile_00d619a0},
          texture_load_owners(geometry, texture_2d,
              inputs.texture_2d_profile_00d61948) {
        require_owner_inputs(inputs);
    }
};

GameNativeTextureOwnerServices::GameNativeTextureOwnerServices(
    const GameNativeTextureOwnerServiceInputs& inputs)
    : impl_(std::make_unique<Impl>(inputs)) {}

GameNativeTextureOwnerServices::~GameNativeTextureOwnerServices() = default;

NativeRenderActualOwnerRegistry&
GameNativeTextureOwnerServices::actual_owners() noexcept {
    return impl_->actual_owners;
}

GuiNativeGeometryOwners& GameNativeTextureOwnerServices::geometry() noexcept {
    return impl_->geometry;
}

NativeTexture2DOwnerContext& GameNativeTextureOwnerServices::texture_2d() noexcept {
    return impl_->texture_2d;
}

NativeTextureLoadOwners&
GameNativeTextureOwnerServices::texture_load_owners() noexcept {
    return impl_->texture_load_owners;
}

} // namespace bsp::game
