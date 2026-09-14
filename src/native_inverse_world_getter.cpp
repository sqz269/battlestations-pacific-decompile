#include "bsp/native_inverse_world_getter.hpp"

#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_matrix_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native inverse-world getter requires MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void* __fastcall get_native_node_inverse_world_00b6e0d0(void*) {
    __asm {
        sub esp, 40h
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 5Ch]
        test al, 8
        jnz cached
        test al, 2
        jnz invert
        call refresh_native_camera_world_00b6db70
    invert:
        lea edx, [esi + 0F0h]
        lea ecx, [esp + 4]
        call invert_native_camera_scaled_affine_00b63b30
        push eax
        lea ecx, [esi + 60h]
        call copy_native_camera_matrix_004134f0
        or dword ptr [esi + 5Ch], 8
    cached:
        lea eax, [esi + 60h]
        pop esi
        add esp, 40h
        ret
    }
}

} // namespace bsp
