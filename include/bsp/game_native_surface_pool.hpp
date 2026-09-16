#pragma once

#include "bsp/d3d9_surface_pool.hpp"

#include <mutex>

namespace bsp::game {

// Canonical source owner for the actual 38h D3D9 surface pool at 0108DB00.
// It borrows the application's existing E188B4 allocator-list domain and lets
// native CD7B40 register the real CE0C90 CRT exit callback.
class GameNativeSurfacePoolProcess final {
public:
    GameNativeSurfacePoolProcess(const GameNativeSurfacePoolProcess&) = delete;
    GameNativeSurfacePoolProcess& operator=(const GameNativeSurfacePoolProcess&) = delete;

    // One attempt. Preserve the original atexit status; registration failure
    // retains native initialization. A thrown attempt cannot be repeated.
    int initialize_once_00cd7b40();
    D3D9SurfacePool& surface_pool_0108db00();

private:
    friend GameNativeSurfacePoolProcess& game_native_surface_pool_process();
    GameNativeSurfacePoolProcess();
    ~GameNativeSurfacePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    D3D9SurfacePoolStorage storage_0108db00_{};
    D3D9SurfacePool pool_;
    std::mutex startup_mutex_;
    StartupState startup_state_{StartupState::unattempted};
    int registration_status_{};
};

// Construct the shared allocator owner and this bookkeeping object before
// CD7B40 registers CE0C90. C++ destruction does not repeat native destruction.
GameNativeSurfacePoolProcess& game_native_surface_pool_process();

} // namespace bsp::game
