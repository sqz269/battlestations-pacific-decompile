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

int GameNativePhysicalPoolProcess::initialize_property_library_node_pool_once_00cc8a70() {
    std::lock_guard lock(property_library_startup_mutex_);
    if (property_library_startup_state_ == StartupState::returned)
        return property_library_registration_status_;
    if (property_library_startup_state_ == StartupState::threw)
        throw std::logic_error("property library node pool startup previously threw");

    // Commit the one Source attempt before binding or native construction.
    // Never reconstruct a live/partially constructed critical-section owner.
    property_library_startup_state_ = StartupState::threw;
    bind_static_native_property_library_node_pool_00e17540(
        property_library_pool_00e17540_, list_);
    property_library_registration_status_ =
        initialize_static_native_property_library_node_pool_00cc8a70();
    property_library_startup_state_ = StartupState::returned;
    return property_library_registration_status_;
}

void* GameNativePhysicalPoolProcess::property_library_node_pool_storage_00e17540() {
    std::lock_guard lock(property_library_startup_mutex_);
    if (property_library_startup_state_ != StartupState::returned)
        throw std::logic_error("property library node pool requires completed explicit startup");
    return property_library_pool_00e17540_;
}

int GameNativePhysicalPoolProcess::initialize_enum_table_pool_once_00cc8a10() {
    std::lock_guard lock(enum_table_startup_mutex_);
    if (enum_table_startup_state_ == StartupState::returned)
        return enum_table_registration_status_;
    if (enum_table_startup_state_ == StartupState::threw)
        throw std::logic_error("enum table pool startup previously threw");

    // Commit before binding or construction; never reconstruct a partial pool.
    enum_table_startup_state_ = StartupState::threw;
    bind_static_native_enum_table_pool_00e175e8(enum_table_pool_00e175e8_, list_);
    enum_table_registration_status_ = initialize_static_native_enum_table_pool_00cc8a10();
    enum_table_startup_state_ = StartupState::returned;
    return enum_table_registration_status_;
}

int GameNativePhysicalPoolProcess::initialize_enum_symbol_pool_once_00cc8a50() {
    std::lock_guard lock(enum_symbol_startup_mutex_);
    if (enum_symbol_startup_state_ == StartupState::returned)
        return enum_symbol_registration_status_;
    if (enum_symbol_startup_state_ == StartupState::threw)
        throw std::logic_error("enum symbol pool startup previously threw");

    enum_symbol_startup_state_ = StartupState::threw;
    bind_static_native_enum_symbol_pool_00e17578(enum_symbol_pool_00e17578_, list_);
    enum_symbol_registration_status_ = initialize_static_native_enum_symbol_pool_00cc8a50();
    enum_symbol_startup_state_ = StartupState::returned;
    return enum_symbol_registration_status_;
}

void* GameNativePhysicalPoolProcess::enum_table_pool_storage_00e175e8() {
    std::lock_guard lock(enum_table_startup_mutex_);
    if (enum_table_startup_state_ != StartupState::returned)
        throw std::logic_error("enum table pool requires completed explicit startup");
    return enum_table_pool_00e175e8_;
}

void* GameNativePhysicalPoolProcess::enum_symbol_pool_storage_00e17578() {
    std::lock_guard lock(enum_symbol_startup_mutex_);
    if (enum_symbol_startup_state_ != StartupState::returned)
        throw std::logic_error("enum symbol pool requires completed explicit startup");
    return enum_symbol_pool_00e17578_;
}

} // namespace bsp::game
