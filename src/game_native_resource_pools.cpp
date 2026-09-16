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
    : mesh_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          mesh_storage_0108fff8_),
      section_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          section_storage_010901d4_),
      hierarchy_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          hierarchy_storage_0109022c_) {}

GameNativeResourcePoolProcess& game_native_resource_pool_process() {
    static GameNativeResourcePoolProcess process;
    return process;
}

int GameNativeResourcePoolProcess::initialize_mesh_once_00cd7e40() {
    std::lock_guard lock(startup_mutex_);
    if (mesh_state_ == StartupState::returned) return mesh_registration_status_;
    if (mesh_state_ == StartupState::threw)
        throw std::logic_error("resource mesh pool startup previously threw");
    mesh_state_ = StartupState::threw;
    bind_static_native_mesh_pool_0108fff8(mesh_);
    mesh_registration_status_ = initialize_static_native_mesh_pool_00cd7e40();
    mesh_state_ = StartupState::returned;
    return mesh_registration_status_;
}

int GameNativeResourcePoolProcess::initialize_section_once_00cd8250() {
    std::lock_guard lock(startup_mutex_);
    if (section_state_ == StartupState::returned) return section_registration_status_;
    if (section_state_ == StartupState::threw)
        throw std::logic_error("resource section pool startup previously threw");
    section_state_ = StartupState::threw;
    bind_static_native_mesh_section_pool_010901d4(section_);
    section_registration_status_ = initialize_static_native_mesh_section_pool_00cd8250();
    section_state_ = StartupState::returned;
    return section_registration_status_;
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

NativeMeshPool& GameNativeResourcePoolProcess::mesh_pool_0108fff8() {
    std::lock_guard lock(startup_mutex_);
    if (mesh_state_ != StartupState::returned)
        throw std::logic_error("resource mesh pool requires completed explicit startup");
    return mesh_;
}

NativeMeshSectionPool& GameNativeResourcePoolProcess::section_pool_010901d4() {
    std::lock_guard lock(startup_mutex_);
    if (section_state_ != StartupState::returned)
        throw std::logic_error("resource section pool requires completed explicit startup");
    return section_;
}

NativeMaterialParameterPool& GameNativeResourcePoolProcess::hierarchy_pool_0109022c() {
    std::lock_guard lock(startup_mutex_);
    if (hierarchy_state_ != StartupState::returned)
        throw std::logic_error("resource hierarchy pool requires completed explicit startup");
    return hierarchy_;
}

} // namespace bsp::game
