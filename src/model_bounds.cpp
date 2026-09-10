#include "bsp/model_bounds.hpp"
#include "bsp/camera_affine.hpp"
#include <cstring>

namespace bsp {
namespace {
// Native 007BB620 instruction order, stores and FCOMI carry branches. Its
// decompiler's ordinary comparisons lose the unordered behavior.
__declspec(naked) float __fastcall maximum_basis_kernel(const float*) {
    __asm {
        sub esp, 0x10
        fld dword ptr [ecx + 0x4]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x4]
        fld dword ptr [ecx + 0x14]
        fld dword ptr [ecx + 0x10]
        fld dword ptr [ecx + 0x18]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0x8]
        fld dword ptr [ecx + 0x24]
        fld dword ptr [ecx + 0x20]
        fld dword ptr [ecx + 0x28]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 0xc]
        fld dword ptr [esp + 0x8]
        fld dword ptr [esp + 0x4]
        fcomi st(0), st(1)
        jc choose_second
        fstp st(1)
        fld dword ptr [esp + 0xc]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        jc choose_third
        movss xmm0, dword ptr [esp + 0x4]
        jmp return_selected
    choose_second:
        fstp st(0)
        fld dword ptr [esp + 0xc]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        jc choose_third
        movss xmm0, dword ptr [esp + 0x8]
        jmp return_selected
    choose_third:
        movss xmm0, dword ptr [esp + 0xc]
    return_selected:
        movss dword ptr [esp], xmm0
        fld dword ptr [esp]
        add esp, 0x10
        ret
    }
}

void copy_sphere_x87(const float* source, float* result) {
    __asm {
        mov ecx, source
        mov eax, result
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 12]
        fstp dword ptr [eax + 12]
    }
}
}

float maximum_basis_length_squared_007bb620(const CameraMatrix& matrix) {
    return maximum_basis_kernel(matrix.data());
}

void transform_sphere_007c1180(const std::array<float, 4>& source,
    const CameraMatrix& matrix, std::array<float, 4>& result) {
    const float squared = maximum_basis_length_squared_007bb620(matrix);
    const float* source_values = source.data();
    float scale;
    std::array<float, 4> transformed;
    float* transformed_values = transformed.data();
    // 00BF7030 -> 00BF704D uses this FSQRT on finite nonnegative input with
    // CW=007F/027F. The former's masked-inexact __87except path also retains the
    // result/CW, skips matherr and leaves errno alone. See the CRT audit for the
    // exact branches; full status-word and other CRT environments remain open.
    __asm {
        fld squared
        fsqrt
        fstp scale                    // 007C1197: round square root to float
        fld scale
        mov ecx, source_values
        fmul dword ptr [ecx + 12]      // 007C11A0: no absolute value or clamp
        mov edx, transformed_values
        fstp dword ptr [edx + 12]      // 007C11AA: before affine center call
    }
    std::array<float, 3> center;
    std::memcpy(center.data(), source.data(), sizeof(center));
    std::array<float, 3> world_center;
    transform_point_004142e0(center, matrix, world_center);
    std::memcpy(transformed.data(), world_center.data(), sizeof(world_center));
    copy_sphere_x87(transformed.data(), result.data());
}

const std::array<float, 4>& get_model_world_sphere_00b6e8c0(
    CameraTransform& transform, ModelBounds& bounds) {
    if ((transform.auxiliary_flags & 0x30u) == 0) {
        if ((transform.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(transform);
        std::array<float, 4> transformed;
        transform_sphere_007c1180(bounds.local_sphere, transform.world, transformed);
        copy_sphere_x87(transformed.data(), bounds.world_sphere.data());
        transform.auxiliary_flags |= 0x30u;
    }
    return bounds.world_sphere;
}
}
