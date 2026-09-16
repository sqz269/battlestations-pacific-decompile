#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::game {

// Canonical source owner for the loader-zero renderer scalar domain researched
// at 0108D6DC, 0108D6E8, 0108DAF8 and 0108DAFC. This object does not create or
// publish the actual renderer at 00F8D394.
class GameNativeRendererScalarProcess final {
public:
    GameNativeRendererScalarProcess(const GameNativeRendererScalarProcess&) = delete;
    GameNativeRendererScalarProcess& operator=(const GameNativeRendererScalarProcess&) = delete;

    NativeRendererSynchronizationGlobals& synchronization_0108d6dc() noexcept;
    std::uint32_t& logical_texture_serial_0108d6e8() noexcept;
    std::uint32_t& texture_tracking_counter_0108daf8() noexcept;
    std::uint32_t& surface_tracking_counter_0108dafc() noexcept;

private:
    friend GameNativeRendererScalarProcess& game_native_renderer_scalar_process();
    GameNativeRendererScalarProcess() = default;
    ~GameNativeRendererScalarProcess() = default;

    NativeRendererSynchronizationGlobals synchronization_0108d6dc_{};
    std::uint32_t logical_texture_serial_0108d6e8_{};
    std::uint32_t texture_tracking_counter_0108daf8_{};
    std::uint32_t surface_tracking_counter_0108dafc_{};
};

// The function-local process object is value-initialized once. Its accessors
// return stable borrowed references; there is no reset, native CRT callback or
// renderer/device lifecycle hidden behind this entry point.
GameNativeRendererScalarProcess& game_native_renderer_scalar_process();

static_assert(sizeof(NativeRendererSynchronizationGlobals) == 0x08);
static_assert(offsetof(NativeRendererSynchronizationGlobals, mode_00) == 0x00);
static_assert(offsetof(NativeRendererSynchronizationGlobals, observed_mode_01) == 0x01);
static_assert(offsetof(NativeRendererSynchronizationGlobals, preserved_02) == 0x02);
static_assert(offsetof(NativeRendererSynchronizationGlobals, nesting_04) == 0x04);

} // namespace bsp::game
