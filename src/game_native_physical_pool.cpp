#include "bsp/game_native_physical_pool.hpp"

#include "bsp/native_physical_provider_pool_lifecycle.hpp"
#include "bsp/native_enum_node_pool.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native physical pool requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(alignof(std::max_align_t) >= alignof(CRITICAL_SECTION));
}

GameNativePhysicalPoolProcess::GameNativePhysicalPoolProcess() noexcept
    : list_(list_head_00e188b4_), context_{pool_0109dbf0_, list_} {}

GameNativePhysicalPoolProcess& game_native_physical_pool_process() {
    static GameNativePhysicalPoolProcess process;
    return process;
}

int GameNativePhysicalPoolProcess::initialize_once_00cd9010() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ == StartupState::returned) return registration_status_;
    if (startup_state_ == StartupState::threw)
        throw std::logic_error("physical pool startup previously threw");

    // Commit to one attempt before either binding or native construction. In
    // particular, no retry may construct a second section over a live pool.
    startup_state_ = StartupState::threw;
    bind_static_native_physical_provider_pool_0109dbf0(pool_0109dbf0_, list_);
    registration_status_ = initialize_static_native_physical_provider_pool_00cd9010();
    startup_state_ = StartupState::returned;
    return registration_status_;
}

NativePhysicalProviderPoolContext&
GameNativePhysicalPoolProcess::physical_provider_pool_context_0109dbf0() {
    std::lock_guard lock(startup_mutex_);
    if (startup_state_ != StartupState::returned)
        throw std::logic_error("physical pool requires completed explicit startup");
    return context_;
}

int GameNativePhysicalPoolProcess::initialize_property_node_pool_once_00cc8a30() {
    std::lock_guard lock(property_startup_mutex_);
    if (property_startup_state_ == StartupState::returned)
        return property_registration_status_;
    if (property_startup_state_ == StartupState::threw)
        throw std::logic_error("property node pool startup previously threw");

    // The canonical process and its C++ exit registration already exist.
    // Bind the genuine trim service before publishing the same borrowed cell.
    // Commit one attempt before either operation; do not repair a partial pool.
    property_startup_state_ = StartupState::threw;
    bind_static_native_property_record_pool_00e175b0(property_pool_00e175b0_, list_);
    property_registration_status_ = initialize_static_native_property_record_pool_00cc8a30();
    property_startup_state_ = StartupState::returned;
    return property_registration_status_;
}

void* GameNativePhysicalPoolProcess::property_node_pool_storage_00e175b0() {
    std::lock_guard lock(property_startup_mutex_);
    if (property_startup_state_ != StartupState::returned)
        throw std::logic_error("property node pool requires completed explicit startup");
    return property_pool_00e175b0_;
}

} // namespace bsp::game
