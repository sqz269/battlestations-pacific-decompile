#include "bsp/camera_position_modes.hpp"
#include "bsp/unit_timer_pose.hpp"
#include "bsp/vector_helpers.hpp"

extern "C" double __cdecl _CIsqrt();
extern "C" double __cdecl _CIatan();
extern "C" double __cdecl _CIatan2();

namespace bsp {
namespace {
const float one = 1.0f;
const float minus_one = -1.0f;
const float half_pi = 1.57079637050628662109375f;
const float minus_half_pi = -1.57079637050628662109375f;
const double double_one = 1.0;
const double distance_squared_cutoff = 1e-10;

// Each primitive has exactly one native x87 float store. Keep the per-vector
// stages separate: combining multiply/divide or add/subtract changes rounding.
void copy_float(float& out, const float& value) {
    const float* input = &value;
    float* output = &out;
    __asm {
        mov eax, input
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
void add_float(float& out, const float& a, const float& b) {
    const float* left = &a;
    const float* right = &b;
    float* output = &out;
    __asm {
        mov eax, left
        mov ecx, right
        mov edx, output
        fld dword ptr [eax]
        fadd dword ptr [ecx]
        fstp dword ptr [edx]
    }
}
void subtract_float(float& out, const float& a, const float& b) {
    const float* left = &a;
    const float* right = &b;
    float* output = &out;
    __asm {
        mov eax, left
        mov ecx, right
        mov edx, output
        fld dword ptr [eax]
        fsub dword ptr [ecx]
        fstp dword ptr [edx]
    }
}
void multiply_float(float& out, const float& a, const float& b) {
    const float* left = &a;
    const float* right = &b;
    float* output = &out;
    __asm {
        mov eax, left
        mov ecx, right
        mov edx, output
        fld dword ptr [eax]
        fmul dword ptr [ecx]
        fstp dword ptr [edx]
    }
}
void divide_float(float& out, const float& a, const float& b) {
    const float* left = &a;
    const float* right = &b;
    float* output = &out;
    __asm {
        mov eax, left
        mov ecx, right
        mov edx, output
        fld dword ptr [eax]
        fdiv dword ptr [ecx]
        fstp dword ptr [edx]
    }
}

// Interior 00794196..007941E4 differs from 00419440: squared components
// remain in x87 until the single sum store, followed by the 1e-10 cutoff.
__declspec(naked) float __fastcall distance_kernel(const float*) {
    __asm {
        push ecx
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 8]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        fld qword ptr [distance_squared_cutoff]
        fld dword ptr [esp]
        fcomi st(0), st(1)
        fstp st(1)
        jbe zero_result
        call _CIsqrt
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        pop ecx
        ret
    zero_result:
        fstp st(0)
        fldz
        pop ecx
        ret
    }
}

void normalize(std::array<float, 3>& direction, const CameraAxesCrtAccess& crt) {
    const float vector_length = camera_vector_length_00419440(direction.data(), &crt);
    float inverse;
    __asm {
        fldz
        fld vector_length
        fcomi st(0), st(1)
        fstp st(1)
        jbe zero_inverse
        fld1
        fdivrp st(1), st(0)
        fstp inverse
        jmp done
    zero_inverse:
        fstp st(0)
        xorps xmm0, xmm0
        movss inverse, xmm0
    done:
    }
    for (float& component : direction) multiply_float(component, component, inverse);
}

void store_angles(CameraPositionView& camera, const std::array<float, 3>& direction) {
    const float* components = direction.data();
    float* rho = &camera.rho_3c8;
    float* theta = &camera.theta_3c4;
    float rounded_angle;
    __asm {
        mov eax, components
        fld dword ptr [eax]
        fld dword ptr [eax + 8]
        call _CIatan2
        fstp rounded_angle
        fld rounded_angle
        fchs
        mov eax, rho
        fstp dword ptr [eax]
        mov eax, components
        push dword ptr [eax + 4]
        call camera_asin_clamped_0042cf10
        mov eax, theta
        fstp dword ptr [eax]
    }
}
} // namespace

__declspec(naked) float __stdcall camera_asin_clamped_0042cf10(float) {
    __asm {
        movss xmm0, dword ptr [esp + 4]
        sub esp, 8
        comiss xmm0, dword ptr [one]
        jbe check_low
        fld dword ptr [half_pi]
        add esp, 8
        ret 4
    check_low:
        movss xmm1, dword ptr [minus_one]
        comiss xmm1, xmm0
        jbe interior
        fld dword ptr [minus_half_pi]
        add esp, 8
        ret 4
    interior:
        fld dword ptr [esp + 12]
        fst qword ptr [esp]
        fmul st(0), st(0)
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        fld1
        fsubrp st(1), st(0)
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        call _CIsqrt
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        fadd qword ptr [double_one]
        fdivr qword ptr [esp]
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        call _CIatan
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        fadd st(0), st(0)
        fstp dword ptr [esp + 12]
        fld dword ptr [esp + 12]
        add esp, 8
        ret 4
    }
}

float* matrix_angles_006e47a0(const CameraMatrix& matrix, float* output,
    CameraPositionHost& host) {
    host.extract_matrix_angles_0042d2e0(matrix, output[0], output[1], output[2]);
    return output;
}

void aim_camera_at_point_00794070(CameraPositionView& camera,
    std::array<float, 3> point, const CameraAxesCrtAccess& crt) {
    std::array<float, 3> direction;
    for (std::size_t i = 0; i != direction.size(); ++i)
        subtract_float(direction[i], point[i], camera.position_394[i]);
    normalize(direction, crt);
    store_angles(camera, direction);
}

void aim_camera_at_target_00794130(CameraPositionView& camera, void* actual_target,
    float scale, CameraPositionHost& host, const CameraAxesCrtAccess& crt) {
    if (scale < 0.0f)
        scale = host.resolve_configuration_scale_54(camera.configuration_4d0);
    PoseRefreshView& pose = host.resolve_pose(actual_target);
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    std::array<float, 3> direction;
    for (std::size_t i = 0; i != direction.size(); ++i)
        subtract_float(direction[i], pose.world_cc[12 + i], camera.position_394[i]);
    const float distance = distance_kernel(direction.data());
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    // 007941F4 captures this flag before basis/scale/divisor arithmetic. Its
    // later JNZ at 00794277 uses the saved condition, not a fresh flag read.
    const bool refresh_again = pose.world_valid_c8 == 0;
    for (std::size_t i = 0; i != direction.size(); ++i)
        multiply_float(direction[i], pose.world_cc[8 + i], distance);
    for (float& component : direction) multiply_float(component, component, scale);
    float divisor;
    copy_float(divisor, camera.divisor_460);
    for (float& component : direction) divide_float(component, component, divisor);
    if (refresh_again) refresh_pose_00414db0(pose);
    std::array<float, 3> point;
    for (std::size_t i = 0; i != point.size(); ++i)
        add_float(point[i], pose.world_cc[12 + i], direction[i]);
    for (std::size_t i = 0; i != direction.size(); ++i)
        subtract_float(direction[i], point[i], camera.position_394[i]);
    normalize(direction, crt);
    store_angles(camera, direction);
}

void apply_camera_position_mode_007954a0(CameraPositionView& camera,
    std::uint32_t index, CameraPositionHost& host, const CameraAxesCrtAccess& crt) {
    void** const begin = camera.records_begin_47c;
    if (begin == nullptr || index >= static_cast<std::uint32_t>(camera.records_end_480 - begin))
        host.range_error_00bf6713();
    CameraPositionRecordView& record =
        host.resolve_position_record(camera.records_begin_47c[index]);
    const std::uint32_t kind = record.kind_48;
    switch (kind) {
    case 0: {
        std::array<float, 3> sampled;
        float parameter;
        copy_float(parameter, camera.parameter_468);
        host.sample_path_007b04c0(camera.path_464, parameter,
            camera.position_394, sampled, 0);
        std::array<float, 3> point;
        // Native captures Y, then Z, then X after the path call.
        add_float(point[1], camera.position_394[1], sampled[1]);
        add_float(point[2], camera.position_394[2], sampled[2]);
        add_float(point[0], camera.position_394[0], sampled[0]);
        aim_camera_at_point_00794070(camera, point, crt);
        return;
    }
    case 1:
        aim_camera_at_target_00794130(camera, record.target_24, -1.0f, host, crt);
        return;
    case 2: {
        std::array<float, 3> point;
        for (std::size_t i = 0; i != point.size(); ++i) copy_float(point[i], record.point_28[i]);
        aim_camera_at_point_00794070(camera, point, crt);
        return;
    }
    case 3: {
        PoseRefreshView& pose = host.resolve_pose(record.parent_14);
        if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
        std::array<float, 3> point;
        transform_point_copy_00414cd0(point.data(), record.point_28, pose.world_cc);
        aim_camera_at_point_00794070(camera, point, crt);
        return;
    }
    case 4:
        copy_float(camera.theta_3c4, record.theta_34);
        copy_float(camera.rho_3c8, record.rho_38);
        return;
    case 5: {
        PoseRefreshView& pose = host.resolve_pose(record.parent_14);
        const CameraMatrix& world = pose_world_matrix_0042d7e0(pose);
        std::array<float, 3> angles;
        matrix_angles_006e47a0(world, angles.data(), host);
        add_float(camera.theta_3c4, record.theta_34, angles[0]);
        add_float(camera.rho_3c8, record.rho_38, angles[1]);
        return;
    }
    default:
        return;
    }
}
} // namespace bsp
