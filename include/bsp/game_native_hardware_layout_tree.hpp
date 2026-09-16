#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <array>
#include <cstddef>
#include <mutex>

namespace bsp::game {

// Canonical process storage for native 0108D530. The original CRT initializer
// CD7960 constructs the sentinel and registers CE0C50. The header and current
// source-CRT invalid-parameter domain outlive that real callback.
class GameNativeHardwareLayoutTreeProcess final {
public:
    GameNativeHardwareLayoutTreeProcess(const GameNativeHardwareLayoutTreeProcess&) = delete;
    GameNativeHardwareLayoutTreeProcess& operator=(const GameNativeHardwareLayoutTreeProcess&) = delete;

    // One attempt; preserve the actual atexit result and initialized storage
    // after a nonzero result. An exception is not retried or rolled back.
    int initialize_once_00cd7960();
    void* tree_0108d530();
    const SingletonLifetimeCallbacks& invalid_parameters() const noexcept { return invalid_; }

private:
    friend GameNativeHardwareLayoutTreeProcess& game_native_hardware_layout_tree_process();
    GameNativeHardwareLayoutTreeProcess() = default;
    ~GameNativeHardwareLayoutTreeProcess() = default;
    static void invalid_parameter(void*);
    enum class StartupState { unattempted, returned, threw };

    alignas(4) std::array<std::byte, 12> storage_0108d530_{};
    const SingletonLifetimeCallbacks invalid_{nullptr, nullptr, &invalid_parameter};
    std::mutex startup_mutex_;
    StartupState state_{StartupState::unattempted};
    int registration_status_{};
};

// Function-local bookkeeping construction completes before native atexit
// registration. Its C++ destructor never repeats native tree destruction.
GameNativeHardwareLayoutTreeProcess& game_native_hardware_layout_tree_process();

} // namespace bsp::game
