#pragma once

#include "bsp/d3d9_texture2d_pool.hpp"

#include <mutex>

namespace bsp::game {

// Canonical source owner for the actual 38h D3D9 texture2D pool at 0108DB38.
// It borrows the application's existing E188B4 allocator-list domain and lets
// native CD7B60 register the real CE0CA0 CRT exit callback.
class GameNativeTexturePoolProcess final {
public:
    GameNativeTexturePoolProcess(const GameNativeTexturePoolProcess&) = delete;
    GameNativeTexturePoolProcess& operator=(const GameNativeTexturePoolProcess&) = delete;

    // One attempt. Preserve the original atexit status; registration failure
    // retains native initialization. A thrown attempt cannot be repeated.
    int initialize_once_00cd7b60();
    D3D9Texture2DPool& texture2d_pool_0108db38();

private:
    friend GameNativeTexturePoolProcess& game_native_texture_pool_process();
    GameNativeTexturePoolProcess();
    ~GameNativeTexturePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    D3D9Texture2DPoolStorage storage_0108db38_{};
    D3D9Texture2DPool pool_;
    std::mutex startup_mutex_;
    StartupState startup_state_{StartupState::unattempted};
    int registration_status_{};
};

// Construct the shared allocator owner and this bookkeeping object before
// CD7B60 registers CE0CA0. C++ destruction does not repeat native destruction.
GameNativeTexturePoolProcess& game_native_texture_pool_process();

} // namespace bsp::game
