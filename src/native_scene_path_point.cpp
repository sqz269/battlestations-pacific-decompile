#include "bsp/native_scene_path_point.hpp"

#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/native_entity_pose.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
// Actual Native read-only D7A24C bytes are 00 00 80 3F. This supplies that
// verified constant in current Source storage, not an invented mutable global.
const float native_scene_path_one = 1.0f;

// Original B62D10 physical call: ECX source, stack destination/matrix, RET8.
// The complete accepted Source kernel takes destination in EDX and RET4.
// Remove only the consumed destination word while retaining the return slot,
// then tail into the actual kernel. No transform work or success is replaced.
__declspec(naked) void* __fastcall scene_path_vector4_original_call(
    const void*, void*, void*, const void*) {
    __asm {
        mov edx, dword ptr [esp + 4]
        pop eax
        mov dword ptr [esp], eax
        jmp transform_native_vector4_00b62d10
    }
}
} // namespace

__declspec(naked) void* __fastcall transform_native_scene_path_point_007af800(
    void*, void*, void*, std::uint32_t) {
    __asm {
        sub esp, 30h
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 14h]
        cmp byte ptr [edi + 0c8h], 0
        jnz pose_ready
        mov ecx, edi
        call refresh_native_entity_pose_00414db0
    pose_ready:
        mov ecx, dword ptr [esi + 8]
        test ecx, ecx
        lea ebx, [edi + 0cch]
        mov edi, dword ptr [esp + 44h]
        jz invalid_index
        mov eax, dword ptr [esi + 0ch]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jb index_ready
    invalid_index:
        call invoke_native_invalid_parameter_00bf6713
    index_ready:
        mov eax, dword ptr [esi + 8]
        mov eax, dword ptr [eax + edi * 4]
        movss xmm0, dword ptr [eax]
        movss xmm1, dword ptr [eax + 4]
        movss xmm2, dword ptr [eax + 8]
        push ebx
        lea ecx, [esp + 30h]
        movss dword ptr [esp + 20h], xmm0
        movss xmm0, dword ptr [native_scene_path_one]
        push ecx
        lea ecx, [esp + 24h]
        movss dword ptr [esp + 28h], xmm1
        movss dword ptr [esp + 2ch], xmm2
        movss dword ptr [esp + 30h], xmm0
        call scene_path_vector4_original_call
        fld dword ptr [eax]
        fstp dword ptr [esp + 0ch]
        pop edi
        fld dword ptr [eax + 4]
        pop esi
        fstp dword ptr [esp + 8]
        pop ebx
        fld dword ptr [eax + 8]
        fstp dword ptr [esp + 8]
        fld dword ptr [eax + 0ch]
        mov eax, dword ptr [esp + 34h]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        fld1
        fdivrp st(1), st(0)
        fstp dword ptr [esp + 38h]
        fld dword ptr [esp]
        fld dword ptr [esp + 38h]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fld dword ptr [esp + 4]
        fmul st(0), st(1)
        fstp dword ptr [eax + 4]
        fmul dword ptr [esp + 8]
        fstp dword ptr [eax + 8]
        add esp, 30h
        ret 8
    }
}
} // namespace bsp
