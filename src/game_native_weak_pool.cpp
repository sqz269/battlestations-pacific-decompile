#include "bsp/game_native_weak_pool.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native weak pool requires MSVC Win32.
#endif

namespace bsp::game {
static_assert(sizeof(void*) == 4 && sizeof(NativeWeakPoolStorage) == 0x38);

GameNativeWeakPoolProcess::GameNativeWeakPoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          storage_0109ce94_) {}

GameNativeWeakPoolProcess& game_native_weak_pool_process() {
    static GameNativeWeakPoolProcess process;
    return process;
}

int GameNativeWeakPoolProcess::initialize_once_00cd8a60() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ == StartupState::returned) return registration_status_;
    if (startup_state_ == StartupState::threw)
        throw std::logic_error("weak pool startup previously threw");
    startup_state_ = StartupState::threw;
    bind_static_native_weak_pool_0109ce94(pool_);
    registration_status_ = initialize_static_native_weak_pool_00cd8a60();
    startup_state_ = StartupState::returned;
    return registration_status_;
}

NativeWeakHandlePool& GameNativeWeakPoolProcess::pool_0109ce94() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ != StartupState::returned)
        throw std::logic_error("weak pool requires completed explicit startup");
    return pool_;
}

} // namespace bsp::game
