#include "bsp/scene_pose_binding.hpp"
#include "bsp/camera_multiply.hpp"

#include <stdexcept>

namespace bsp {

void add_scene_parent_world_offset_0046c6b7(void* captured_parent_identity,
    PoseRefreshResolver& poses, float& x, float& z) {
    if (!captured_parent_identity)
        throw std::invalid_argument("scene parent-pose branch requires its actual nonnull parent");
    auto& parent = poses.resolve_pose(captured_parent_identity);
    if (parent.world_valid_c8 == 0) refresh_pose_00414db0(parent);
    const float* const world = parent.world_cc.data();
    float* const x_storage = &x;
    float* const z_storage = &z;
    __asm {
        mov eax, world
        mov edx, x_storage
        fld dword ptr [eax + 0x30] // 0046C6C7: actual parent+FCh
        fadd dword ptr [edx]      // 0046C6CD: prior local X
        fstp dword ptr [edx]     // 0046C6D1: float spill before reading Z
        mov edx, z_storage
        fld dword ptr [eax + 0x38] // 0046C6D5: actual parent+104h
        fadd dword ptr [edx]       // 0046C6DB: prior local Z
        fstp dword ptr [edx]       // 0046C6DF
    }
}

void compose_scene_null_parent_offset_0046c6e5(const CameraMatrix& local,
    const CameraMatrix& parent_argument, float& x, float& z) {
    CameraMatrix product;
    multiply_camera_matrices_00413920(product, local, parent_argument);
    const float* const values = product.data();
    float* const x_storage = &x;
    float* const z_storage = &z;
    __asm {
        mov eax, values
        mov edx, x_storage
        movss xmm0, dword ptr [eax + 0x30]
        movss dword ptr [edx], xmm0
        mov edx, z_storage
        movss xmm0, dword ptr [eax + 0x38]
        movss dword ptr [edx], xmm0
    }
}

} // namespace bsp
