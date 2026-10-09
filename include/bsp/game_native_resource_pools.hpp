#pragma once

#include "bsp/native_material_pools.hpp"
#include "bsp/native_mesh_pool.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_camera_pool.hpp"
#include "bsp/native_model_pool.hpp"
#include "bsp/native_group_pool.hpp"

#include <mutex>

namespace bsp::game {

// Canonical source storage for the distinct 0108FFB0 camera, 0108FFF8 mesh, 010901D4 section
// and 0109022C hierarchy pools, plus the separate 01090054 model and 0109008C
// model-base node pools, 0108FF58 plain-node and 010902F4 group pools.
// All use the existing E188B4 allocator domain;
// F8D3E4 remains a different material-parameter pool. Native startup owns
// each real CRT exit callback.
class GameNativeResourcePoolProcess final {
public:
    GameNativeResourcePoolProcess(const GameNativeResourcePoolProcess&) = delete;
    GameNativeResourcePoolProcess& operator=(const GameNativeResourcePoolProcess&) = delete;

    // One attempt. Preserve the original atexit status; registration failure
    // retains native initialization. A thrown attempt cannot be repeated.
    int initialize_camera_once_00cd7dd0();
    int initialize_mesh_once_00cd7e40();
    int initialize_section_once_00cd8250();
    int initialize_hierarchy_once_00cd82d0();
    int initialize_model_once_00cd7f00();
    int initialize_model_base_once_00cd7f20();
    int initialize_plain_node_once_00cd7d10();
    int initialize_group_once_00cd8460();
    NativeCameraPool& camera_pool_0108ffb0();
    NativeMeshPool& mesh_pool_0108fff8();
    NativeMeshSectionPool& section_pool_010901d4();
    NativeMaterialParameterPool& hierarchy_pool_0109022c();
    NativeModelPool& model_pool_01090054();
    // Actual initialized 38h pool, distinct from the plain-node 0108FF58 pool.
    // Return every 178h slot after its 174h owner/companions finish and before
    // CE0E60. No model object, type identity or plain-node binding is implied.
    void* model_base_pool_storage_0109008c();
    // Distinct actual initialized 38h owners. Plain nodes use 178h slots;
    // groups use 18Ch slots. Finish every payload/companion and return its slot
    // before CE0E20/CE0F00. These accessors do not construct either owner type.
    void* plain_node_pool_storage_0108ff58();
    NativeGroupPool& group_pool_010902f4();

private:
    friend GameNativeResourcePoolProcess& game_native_resource_pool_process();
    GameNativeResourcePoolProcess();
    ~GameNativeResourcePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    NativeCameraPoolStorage camera_storage_0108ffb0_{};
    NativeCameraPool camera_;
    NativeMeshPoolStorage mesh_storage_0108fff8_{};
    NativeMeshPool mesh_;
    NativeMeshSectionPoolStorage section_storage_010901d4_{};
    NativeMeshSectionPool section_;
    NativeMaterialParameterPoolStorage hierarchy_storage_0109022c_{};
    NativeMaterialParameterPool hierarchy_;
    std::mutex startup_mutex_;
    StartupState camera_state_{StartupState::unattempted};
    StartupState mesh_state_{StartupState::unattempted};
    StartupState section_state_{StartupState::unattempted};
    StartupState hierarchy_state_{StartupState::unattempted};
    int camera_registration_status_{};
    int mesh_registration_status_{};
    int section_registration_status_{};
    int hierarchy_registration_status_{};

    // Append to retain the existing process members' layout. Both cells and
    // the model companion survive their CRT callbacks; neither C++ destructor
    // invokes native teardown. The base header is cold until explicit startup.
    NativeModelPoolStorage model_storage_01090054_{};
    NativeModelPool model_;
    alignas(4) std::byte model_base_storage_0109008c_[0x38]{};
    StartupState model_state_{StartupState::unattempted};
    StartupState model_base_state_{StartupState::unattempted};
    int model_registration_status_{};
    int model_base_registration_status_{};

    // Keep the plain-node cell distinct from model-base even though both use
    // B6E980/B6E3D0. Their static bindings and CRT callbacks are separate.
    alignas(4) std::byte plain_node_storage_0108ff58_[0x38]{};
    NativeGroupPoolStorage group_storage_010902f4_{};
    NativeGroupPool group_;
    StartupState plain_node_state_{StartupState::unattempted};
    StartupState group_state_{StartupState::unattempted};
    int plain_node_registration_status_{};
    int group_registration_status_{};
};

// Construct the shared allocator owner and this process object before native
// startup registers its exit callback. Every payload must die before CRT exit.
// C++ destruction does not repeat native pool destruction.
GameNativeResourcePoolProcess& game_native_resource_pool_process();

} // namespace bsp::game
