#pragma once

#include "bsp/native_material_pools.hpp"
#include "bsp/native_mesh_pool.hpp"
#include "bsp/native_mesh_section.hpp"

#include <mutex>

namespace bsp::game {

// Canonical source storage for the distinct 0108FFF8 mesh, 010901D4 section
// and 0109022C hierarchy pools. All use the existing E188B4 allocator domain;
// F8D3E4 remains a different material-parameter pool. Native startup owns
// each real CRT exit callback.
class GameNativeResourcePoolProcess final {
public:
    GameNativeResourcePoolProcess(const GameNativeResourcePoolProcess&) = delete;
    GameNativeResourcePoolProcess& operator=(const GameNativeResourcePoolProcess&) = delete;

    // One attempt. Preserve the original atexit status; registration failure
    // retains native initialization. A thrown attempt cannot be repeated.
    int initialize_mesh_once_00cd7e40();
    int initialize_section_once_00cd8250();
    int initialize_hierarchy_once_00cd82d0();
    NativeMeshPool& mesh_pool_0108fff8();
    NativeMeshSectionPool& section_pool_010901d4();
    NativeMaterialParameterPool& hierarchy_pool_0109022c();

private:
    friend GameNativeResourcePoolProcess& game_native_resource_pool_process();
    GameNativeResourcePoolProcess();
    ~GameNativeResourcePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    NativeMeshPoolStorage mesh_storage_0108fff8_{};
    NativeMeshPool mesh_;
    NativeMeshSectionPoolStorage section_storage_010901d4_{};
    NativeMeshSectionPool section_;
    NativeMaterialParameterPoolStorage hierarchy_storage_0109022c_{};
    NativeMaterialParameterPool hierarchy_;
    std::mutex startup_mutex_;
    StartupState mesh_state_{StartupState::unattempted};
    StartupState section_state_{StartupState::unattempted};
    StartupState hierarchy_state_{StartupState::unattempted};
    int mesh_registration_status_{};
    int section_registration_status_{};
    int hierarchy_registration_status_{};
};

// Construct the shared allocator owner and this process object before native
// startup registers its exit callback. Every payload must die before CRT exit.
// C++ destruction does not repeat native pool destruction.
GameNativeResourcePoolProcess& game_native_resource_pool_process();

} // namespace bsp::game
