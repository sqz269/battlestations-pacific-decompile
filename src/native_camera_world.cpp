#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_matrix_math.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera world refresh requires MSVC Win32 raw assembly.
#endif

namespace bsp {
// Complete original B6DB70..B6DBBF. Only the three direct CALL operands relocate
// to complete raw providers. Actual raw-node parent contract and uncertainty
// are documented in the header; this does not promote companion-backed owners.
__declspec(naked) void __fastcall refresh_native_camera_world_00b6db70(void*) {
    __asm {
        push esi // 00b6db70
        mov esi, ecx // 00b6db71
        push edi // 00b6db73
        mov edi, dword ptr [esi + 30h] // 00b6db74
        test edi, edi // 00b6db77
        jz root_node // 00b6db79
        test byte ptr [edi + 5ch], 2 // 00b6db7b
        jnz parent_ready // 00b6db7f
        mov ecx, edi // 00b6db81
        call refresh_native_camera_world_00b6db70 // 00b6db83
    parent_ready:
        add edi, 0f0h // 00b6db88
        push edi // 00b6db8e
        lea edx, [esi + 0b0h] // 00b6db8f
        lea ecx, [esi + 0f0h] // 00b6db95
        call compose_native_camera_affine_00b6d4d0 // 00b6db9b
        or dword ptr [esi + 5ch], 2 // 00b6dba0
        pop edi // 00b6dba4
        pop esi // 00b6dba5
        ret // 00b6dba6
    root_node:
        lea eax, [esi + 0b0h] // 00b6dba7
        push eax // 00b6dbad
        lea ecx, [esi + 0f0h] // 00b6dbae
        call copy_native_camera_matrix_004134f0 // 00b6dbb4
        or dword ptr [esi + 5ch], 2 // 00b6dbb9
        pop edi // 00b6dbbd
        pop esi // 00b6dbbe
        ret // 00b6dbbf
    }
}
} // namespace bsp
