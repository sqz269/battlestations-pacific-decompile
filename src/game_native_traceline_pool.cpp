#include "bsp/game_native_traceline_pool.hpp"
#include "bsp/game_native_physical_pool.hpp"

#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native Traceline pool requires MSVC Win32.
#endif

namespace bsp::game {

GameNativeTracelinePoolProcess::GameNativeTracelinePoolProcess()
    : list_(game_native_physical_pool_process().allocator_list_domain_00e188b4()) {
    // Binding borrows the actual base header before AF3170 publishes it.
    // Only establish that trivial header's lifetime here; native startup owns
    // the list links, CRITICAL_SECTION and pointer-table initialization.
    ::new (storage_00f8c288_) AllocatorListElement;
}

GameNativeTracelinePoolProcess& game_native_traceline_pool_process() {
    static GameNativeTracelinePoolProcess process;
    return process;
}

int GameNativeTracelinePoolProcess::initialize_once() {
    std::lock_guard lock(startup_mutex_);
    if (state_ == StartupState::returned) return registration_status_;
    if (state_ == StartupState::threw)
        throw std::logic_error("native Traceline pool startup previously threw");
    state_ = StartupState::threw;
    bind_static_native_traceline_pool_00f8c288(storage_00f8c288_, list_);
    // bsp.py show CD77F0: AF3170(F8C288), then real atexit(CE0B70).
    // This source owner supplies that same storage/domain to both bodies.
    registration_status_ = initialize_static_native_traceline_pool_00cd77f0();
    state_ = StartupState::returned;
    return registration_status_;
}

void* GameNativeTracelinePoolProcess::pool_storage() {
    std::lock_guard lock(startup_mutex_);
    if (state_ != StartupState::returned)
        throw std::logic_error("native Traceline pool requires completed explicit startup");
    return storage_00f8c288_;
}

} // namespace bsp::game
