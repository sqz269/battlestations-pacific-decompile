#include "bsp/game_native_texture_pool.hpp"

#include "bsp/game_native_physical_pool.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native texture pool requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(D3D9Texture2DPoolStorage) == 0x38);
}

GameNativeTexturePoolProcess::GameNativeTexturePoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          storage_0108db38_) {}

GameNativeTexturePoolProcess& game_native_texture_pool_process() {
    static GameNativeTexturePoolProcess process;
    return process;
}

int GameNativeTexturePoolProcess::initialize_once_00cd7b60() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ == StartupState::returned) return registration_status_;
    if (startup_state_ == StartupState::threw)
        throw std::logic_error("texture2D pool startup previously threw");

    startup_state_ = StartupState::threw;
    bind_static_d3d9_texture2d_pool_0108db38(pool_);
    registration_status_ = initialize_static_d3d9_texture2d_pool_00cd7b60();
    startup_state_ = StartupState::returned;
    return registration_status_;
}

D3D9Texture2DPool& GameNativeTexturePoolProcess::texture2d_pool_0108db38() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ != StartupState::returned)
        throw std::logic_error("texture2D pool requires completed explicit startup");
    return pool_;
}

} // namespace bsp::game
