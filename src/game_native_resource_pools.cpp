#include "bsp/game_native_resource_pools.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_model_base_bootstrap.hpp"
#include "bsp/native_node_pool_owner.hpp"
#include "bsp/native_resource_hierarchy_pool.hpp"

#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native resource pools require MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeMaterialParameterPoolStorage) == 0x38);
static_assert(sizeof(NativeModelPoolStorage) == 0x38);
static_assert(sizeof(NativeGroupPoolStorage) == 0x38);
static_assert(sizeof(AllocatorListElement) == 0x0c && alignof(AllocatorListElement) <= 4);
}

GameNativeResourcePoolProcess::GameNativeResourcePoolProcess()
    : camera_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          camera_storage_0108ffb0_),
      mesh_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          mesh_storage_0108fff8_),
      section_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          section_storage_010901d4_),
      hierarchy_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          hierarchy_storage_0109022c_),
      model_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          model_storage_01090054_),
      group_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          group_storage_010902f4_) {
    // Binding takes an AllocatorListElement reference. Establish only that
    // trivial header's lifetime here; CD7F20/B6E980 later constructs the pool,
    // publishes the list links and initializes the real critical section.
    ::new (model_base_storage_0109008c_) AllocatorListElement;
    // CD7D10 constructs this separate plain-node pool through its own binding.
    ::new (plain_node_storage_0108ff58_) AllocatorListElement;
}

GameNativeResourcePoolProcess& game_native_resource_pool_process() {
    static GameNativeResourcePoolProcess process;
    return process;
}

int GameNativeResourcePoolProcess::initialize_camera_once_00cd7dd0() {
    std::lock_guard lock(startup_mutex_);
    if (camera_state_ == StartupState::returned) return camera_registration_status_;
    if (camera_state_ == StartupState::threw)
        throw std::logic_error("resource camera pool startup previously threw");
    camera_state_ = StartupState::threw;
    bind_static_native_camera_pool_0108ffb0(camera_);
    camera_registration_status_ = initialize_static_native_camera_pool_00cd7dd0();
    camera_state_ = StartupState::returned;
    return camera_registration_status_;
}

NativeCameraPool& GameNativeResourcePoolProcess::camera_pool_0108ffb0() {
    std::lock_guard lock(startup_mutex_);
    if (camera_state_ != StartupState::returned)
        throw std::logic_error("resource camera pool requires completed explicit startup");
    return camera_;
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

int GameNativeResourcePoolProcess::initialize_model_once_00cd7f00() {
    std::lock_guard lock(startup_mutex_);
    if (model_state_ == StartupState::returned) return model_registration_status_;
    if (model_state_ == StartupState::threw)
        throw std::logic_error("resource model pool startup previously threw");
    model_state_ = StartupState::threw;
    bind_static_native_model_pool_01090054(model_);
    model_registration_status_ = initialize_static_native_model_pool_00cd7f00();
    model_state_ = StartupState::returned;
    return model_registration_status_;
}

int GameNativeResourcePoolProcess::initialize_model_base_once_00cd7f20() {
    std::lock_guard lock(startup_mutex_);
    if (model_base_state_ == StartupState::returned) return model_base_registration_status_;
    if (model_base_state_ == StartupState::threw)
        throw std::logic_error("resource model-base pool startup previously threw");
    model_base_state_ = StartupState::threw;
    bind_static_model_base_node_pool_0109008c(model_base_storage_0109008c_,
        game_native_physical_pool_process().allocator_list_domain_00e188b4());
    model_base_registration_status_ = initialize_static_model_base_node_pool_00cd7f20();
    model_base_state_ = StartupState::returned;
    return model_base_registration_status_;
}

NativeModelPool& GameNativeResourcePoolProcess::model_pool_01090054() {
    std::lock_guard lock(startup_mutex_);
    if (model_state_ != StartupState::returned)
        throw std::logic_error("resource model pool requires completed explicit startup");
    return model_;
}

void* GameNativeResourcePoolProcess::model_base_pool_storage_0109008c() {
    std::lock_guard lock(startup_mutex_);
    if (model_base_state_ != StartupState::returned)
        throw std::logic_error("resource model-base pool requires completed explicit startup");
    return model_base_storage_0109008c_;
}

int GameNativeResourcePoolProcess::initialize_plain_node_once_00cd7d10() {
    std::lock_guard lock(startup_mutex_);
    if (plain_node_state_ == StartupState::returned) return plain_node_registration_status_;
    if (plain_node_state_ == StartupState::threw)
        throw std::logic_error("resource plain-node pool startup previously threw");
    plain_node_state_ = StartupState::threw;
    bind_static_native_node_pool_0108ff58(plain_node_storage_0108ff58_,
        game_native_physical_pool_process().allocator_list_domain_00e188b4());
    plain_node_registration_status_ = initialize_static_native_node_pool_00cd7d10();
    plain_node_state_ = StartupState::returned;
    return plain_node_registration_status_;
}

int GameNativeResourcePoolProcess::initialize_group_once_00cd8460() {
    std::lock_guard lock(startup_mutex_);
    if (group_state_ == StartupState::returned) return group_registration_status_;
    if (group_state_ == StartupState::threw)
        throw std::logic_error("resource group pool startup previously threw");
    group_state_ = StartupState::threw;
    bind_static_native_group_pool_010902f4(group_);
    group_registration_status_ = initialize_static_native_group_pool_00cd8460();
    group_state_ = StartupState::returned;
    return group_registration_status_;
}

void* GameNativeResourcePoolProcess::plain_node_pool_storage_0108ff58() {
    std::lock_guard lock(startup_mutex_);
    if (plain_node_state_ != StartupState::returned)
        throw std::logic_error("resource plain-node pool requires completed explicit startup");
    return plain_node_storage_0108ff58_;
}

NativeGroupPool& GameNativeResourcePoolProcess::group_pool_010902f4() {
    std::lock_guard lock(startup_mutex_);
    if (group_state_ != StartupState::returned)
        throw std::logic_error("resource group pool requires completed explicit startup");
    return group_;
}

} // namespace bsp::game
