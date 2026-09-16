#include "bsp/game_native_resource_pools.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_resource_hierarchy_pool.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native resource pools require MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeMaterialParameterPoolStorage) == 0x38);
}

GameNativeResourcePoolProcess::GameNativeResourcePoolProcess()
    : hierarchy_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          hierarchy_storage_0109022c_) {}

GameNativeResourcePoolProcess& game_native_resource_pool_process() {
    static GameNativeResourcePoolProcess process;
    return process;
}

int GameNativeResourcePoolProcess::initialize_hierarchy_once_00cd82d0() {
    std::lock_guard lock(startup_mutex_);
    if (hierarchy_state_ == StartupState::returned) return hierarchy_registration_status_;
    if (hierarchy_state_ == StartupState::threw)
        throw std::logic_error("resource hierarchy pool startup previously threw");

    hierarchy_state_ = StartupState::threw;
    bind_static_native_hierarchy_pool_0109022c(hierarchy_);
    hierarchy_registration_status_ = initialize_static_native_hierarchy_pool_00cd82d0();
    hierarchy_state_ = StartupState::returned;
    return hierarchy_registration_status_;
}

NativeMaterialParameterPool& GameNativeResourcePoolProcess::hierarchy_pool_0109022c() {
    std::lock_guard lock(startup_mutex_);
    if (hierarchy_state_ != StartupState::returned)
        throw std::logic_error("resource hierarchy pool requires completed explicit startup");
    return hierarchy_;
}

} // namespace bsp::game
