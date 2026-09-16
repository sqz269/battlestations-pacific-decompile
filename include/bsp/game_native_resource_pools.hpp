#pragma once

#include "bsp/native_material_pools.hpp"

#include <mutex>

namespace bsp::game {

// Canonical source storage for the distinct 0109022C hierarchy pool. Its 88h
// slots use the existing E188B4 allocator domain; F8D3E4 remains a different
// material-parameter pool. Native startup owns the real CRT exit callback.
class GameNativeResourcePoolProcess final {
public:
    GameNativeResourcePoolProcess(const GameNativeResourcePoolProcess&) = delete;
    GameNativeResourcePoolProcess& operator=(const GameNativeResourcePoolProcess&) = delete;

    // One attempt. Preserve the original atexit status; registration failure
    // retains native initialization. A thrown attempt cannot be repeated.
    int initialize_hierarchy_once_00cd82d0();
    NativeMaterialParameterPool& hierarchy_pool_0109022c();

private:
    friend GameNativeResourcePoolProcess& game_native_resource_pool_process();
    GameNativeResourcePoolProcess();
    ~GameNativeResourcePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    NativeMaterialParameterPoolStorage hierarchy_storage_0109022c_{};
    NativeMaterialParameterPool hierarchy_;
    std::mutex startup_mutex_;
    StartupState hierarchy_state_{StartupState::unattempted};
    int hierarchy_registration_status_{};
};

// Construct the shared allocator owner and this process object before native
// startup registers its exit callback. Every payload must die before CRT exit.
// C++ destruction does not repeat native pool destruction.
GameNativeResourcePoolProcess& game_native_resource_pool_process();

} // namespace bsp::game
