#include "bsp/native_entity_pose.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_matrix_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw entity pose refreshing requires MSVC Win32 assembly.
#endif

namespace bsp {
// All 89 original bytes except natural direct-call relocation operands.
// Names are descriptive hypotheses; the original object/class is external.
__declspec(naked) void __fastcall refresh_native_entity_pose_00414db0(void*, void*) {
    __asm {
        sub esp,40h
        push esi
        mov esi,ecx
        cmp byte ptr [esi+0c8h],0
        jnz done
        mov ecx,dword ptr [esi+3ch]
        test ecx,ecx
        jz parent_ready
        call refresh_native_entity_pose_00414db0
    parent_ready:
        mov eax,dword ptr [esi+3ch]
        test eax,eax
        jz root_local
        add eax,0cch
        push eax
        lea eax,[esp+8]
        push eax
        lea ecx,[esi+74h]
        call multiply_native_camera_matrices_00413920
        jmp publish
    root_local:
        lea eax,[esi+74h]
    publish:
        push eax
        lea ecx,[esi+0cch]
        call copy_native_camera_matrix_004134f0
        mov byte ptr [esi+0c8h],1
        mov byte ptr [esi+10ch],0
    done:
        pop esi
        add esp,40h
        ret
    }
}
} // namespace bsp
