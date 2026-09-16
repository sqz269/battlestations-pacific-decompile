#include "bsp/game_native_surface_pool.hpp"

#include "bsp/game_native_physical_pool.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native surface pool requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(D3D9SurfacePoolStorage) == 0x38);
}

GameNativeSurfacePoolProcess::GameNativeSurfacePoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          storage_0108db00_) {}

GameNativeSurfacePoolProcess& game_native_surface_pool_process() {
    static GameNativeSurfacePoolProcess process;
    return process;
}

int GameNativeSurfacePoolProcess::initialize_once_00cd7b40() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ == StartupState::returned) return registration_status_;
    if (startup_state_ == StartupState::threw)
        throw std::logic_error("surface pool startup previously threw");

    startup_state_ = StartupState::threw;
    bind_static_d3d9_surface_pool_0108db00(pool_);
    registration_status_ = initialize_static_d3d9_surface_pool_00cd7b40();
    startup_state_ = StartupState::returned;
    return registration_status_;
}

D3D9SurfacePool& GameNativeSurfacePoolProcess::surface_pool_0108db00() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ != StartupState::returned)
        throw std::logic_error("surface pool requires completed explicit startup");
    return pool_;
}

} // namespace bsp::game
