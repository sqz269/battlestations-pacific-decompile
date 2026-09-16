#include "bsp/game_native_renderer_scalars.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native renderer scalar process requires MSVC Win32.
#endif

namespace bsp::game {

GameNativeRendererScalarProcess& game_native_renderer_scalar_process() {
    static GameNativeRendererScalarProcess process;
    return process;
}

NativeRendererSynchronizationGlobals&
GameNativeRendererScalarProcess::synchronization_0108d6dc() noexcept {
    return synchronization_0108d6dc_;
}

std::uint32_t& GameNativeRendererScalarProcess::logical_texture_serial_0108d6e8() noexcept {
    return logical_texture_serial_0108d6e8_;
}

std::uint32_t&
GameNativeRendererScalarProcess::texture_tracking_counter_0108daf8() noexcept {
    return texture_tracking_counter_0108daf8_;
}

std::uint32_t&
GameNativeRendererScalarProcess::surface_tracking_counter_0108dafc() noexcept {
    return surface_tracking_counter_0108dafc_;
}

} // namespace bsp::game
