#include "bsp/camera_pose.hpp"
#include "bsp/camera_inverse.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void require_world_access(const CameraPoseAccess& access) {
    if (!access.load_vtable || !access.load_slot || !access.invoke_world_matrix_34)
        throw std::invalid_argument("camera pose requires actual owner table, slot and world dispatch");
}
// Called with the native input.z still in ST0. The two required readers are
// restricted to integer raw loads and may neither throw nor disturb x87 state.
__declspec(noinline) std::uintptr_t __cdecl resolve_world_entry(
    const CameraPoseAccess* access, CameraState* camera) noexcept {
    const auto table = access->load_vtable(access->context, *camera);
    return access->load_slot(access->context, table, 0x34);
}
void write_world_position(CameraState& camera, const CameraAxis& position,
    const CameraPoseAccess& access) {
    const float* source = position.data();
    float* world = camera.transform.world.data();
    const auto* adapter = &access;
    auto* state = &camera;
    std::uintptr_t entry;
    __asm {
        mov eax,source
        mov ecx,world
        fld dword ptr [eax] // B6DAE4
        fstp dword ptr [ecx+48] // B6DAEC
        fld dword ptr [eax+4] // B6DAF6
        fstp dword ptr [ecx+52] // B6DAF9
        fld dword ptr [eax+8] // B6DAFF; preserve ST0 across integer-only loads
        push state
        push adapter
        call resolve_world_entry // B6DB02..B6DB04
        add esp,8
        mov entry,eax
        mov ecx,world
        fstp dword ptr [ecx+56] // B6DB07
    }
    access.invoke_world_matrix_34(access.context, entry, camera, camera.transform.world);
}
}

void set_transform_world_position_00b6dae0(CameraState& camera, const CameraAxis& position,
    const CameraPoseAccess& access) {
    require_world_access(access);
    write_world_position(camera, position, access);
}

void set_camera_world_position_00b71400(CameraState& camera, const CameraAxis& position,
    const CameraPoseAccess& access) {
    require_world_access(access);
    camera.projection.valid_flags &= 0xfffffe4bu;
    write_world_position(camera, position, access);
    refresh_camera_direction_00b70660(camera);
}

void set_camera_look_at_00b700e0(CameraState& camera, const CameraAxis& eye,
    const CameraAxis& target, const CameraPoseAccess& access, const CameraAxesCrtAccess& crt) {
    require_world_access(access);
    if (!access.invoke_position_30 || !crt.dispatch_bypass_0109dd78 || !crt.except_00c27489)
        throw std::invalid_argument("camera look-at requires actual position dispatch and CRT access");
    const auto position_table = access.load_vtable(access.context, camera); // B700F1
    const auto position_entry = access.load_slot(access.context, position_table, 0x30); // B700F3
    camera.projection.valid_flags &= 0xfffffe4bu; // B700F6
    access.invoke_position_30(access.context, position_entry, camera, eye); // B70102

    const float* input = target.data();
    const float* position = eye.data();
    float* actual_target = camera.target.data();
    float* actual_direction = camera.direction.data();
    const auto* actual_crt = &crt;
    float difference[3], reciprocal, scaled[3];
    __asm {
        mov eax,input
        mov edi,actual_target
        fld dword ptr [eax] // B7010B
        fstp dword ptr [edi] // B70113
        fld dword ptr [eax+4] // B70119
        fstp dword ptr [edi+4] // B7011C
        fld dword ptr [eax+8] // B7011F
        fstp dword ptr [edi+8] // B70122
        mov ebx,position
        fld dword ptr [edi] // B70125
        fsub dword ptr [ebx] // B70127
        fstp dword ptr difference[0] // B70129
        fld dword ptr [edi+4] // B7012D
        fsub dword ptr [ebx+4] // B70130
        fstp dword ptr difference[4] // B70133
        fld dword ptr [edi+8] // B70137
        fsub dword ptr [ebx+8] // B7013A
        fstp dword ptr difference[8] // B7013D
        lea ecx,difference
        mov edx,actual_crt
        call camera_vector_length_00419440 // B70141
        xorps xmm0,xmm0 // B70146
        fstp dword ptr reciprocal // B70149
        fldz // B7014D
        fld dword ptr reciprocal // B7014F
        fcomi st(0),st(1) // B70153
        fstp st(1) // B70155
        jbe zero_reciprocal // includes unordered
        fld1 // B70159
        fdivrp st(1),st(0) // B7015B
        fstp dword ptr reciprocal // B7015D
        jmp multiply_direction
    zero_reciprocal:
        fstp st(0) // B70163
        movss dword ptr reciprocal,xmm0 // B70165
    multiply_direction:
        fld dword ptr difference[0] // B7016B
        fld dword ptr reciprocal // B70177
        fld st(0) // B7017E
        fmulp st(2),st(0) // B70182
        fxch st(1) // B70185
        fstp dword ptr scaled[0] // B7018D
        fld dword ptr difference[4] // B70191
        fmul st(0),st(1) // B70195
        fstp dword ptr scaled[4] // B70197
        fmul dword ptr difference[8] // B7019B
        fstp dword ptr scaled[8] // B7019F
        mov esi,actual_direction
        fld dword ptr scaled[0] // B701A3
        fstp dword ptr [esi] // B701A7
        fld dword ptr scaled[4] // B701AD
        fstp dword ptr [esi+4] // B701B1
        fld dword ptr scaled[8] // B701B7
        fstp dword ptr [esi+8] // B701BB
    }
    // The native up argument is raw (+0,+1,+0), loaded after the first length.
    const CameraAxis up{0.0f, 1.0f, 0.0f};
    CameraMatrix view, world;
    build_camera_look_at_00b63f10(view, eye, camera.target, up, crt); // B701CF
    const auto world_table = access.load_vtable(access.context, camera); // B701D4
    invert_camera_affine_00b63b30(world, view); // B701DC
    const auto world_entry = access.load_slot(access.context, world_table, 0x34); // B701E2
    access.invoke_world_matrix_34(access.context, world_entry, camera, world); // B701E7
}
}
