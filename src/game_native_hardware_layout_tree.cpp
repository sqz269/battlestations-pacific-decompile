#include "bsp/game_native_hardware_layout_tree.hpp"

#include "bsp/native_hardware_layout_tree_static.hpp"

#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native hardware-layout tree requires MSVC Win32.
#endif

namespace bsp::game {

GameNativeHardwareLayoutTreeProcess& game_native_hardware_layout_tree_process() {
    static GameNativeHardwareLayoutTreeProcess process;
    return process;
}

void GameNativeHardwareLayoutTreeProcess::invalid_parameter(void*) {
    // Same current source-CRT handler domain as the existing raw singleton
    // operations. A returning handler preserves the native continuation.
    _invalid_parameter_noinfo();
}

int GameNativeHardwareLayoutTreeProcess::initialize_once_00cd7960() {
    std::lock_guard lock(startup_mutex_);
    if (state_ == StartupState::returned) return registration_status_;
    if (state_ == StartupState::threw)
        throw std::logic_error("hardware-layout tree startup previously threw");
    state_ = StartupState::threw;
    bind_static_native_hardware_layout_tree_0108d530(storage_0108d530_.data(), invalid_);
    registration_status_ = initialize_static_native_hardware_layout_tree_00cd7960();
    state_ = StartupState::returned;
    return registration_status_;
}

void* GameNativeHardwareLayoutTreeProcess::tree_0108d530() {
    std::lock_guard lock(startup_mutex_);
    if (state_ != StartupState::returned)
        throw std::logic_error("hardware-layout tree requires completed explicit startup");
    return storage_0108d530_.data();
}

} // namespace bsp::game
