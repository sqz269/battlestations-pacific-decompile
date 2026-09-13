#include "bsp/native_camera_configuration_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera configuration leaves require MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void __fastcall set_native_camera_near_00b6fbf0(
    void*, const void*) noexcept {
    __asm {
        movss xmm0, dword ptr [edx] // B6FBF0: original [ESP+4]
        and dword ptr [ecx + 2f0h], 0ffffff41h // B6FBF6
        movss dword ptr [ecx + 1d4h], xmm0 // B6FC00
        ret // original B6FC08 RET4; argument address now EDX
    }
}

__declspec(naked) void __fastcall set_native_camera_far_00b6fc10(
    void*, const void*) noexcept {
    __asm {
        movss xmm0, dword ptr [edx] // B6FC10: original [ESP+4]
        and dword ptr [ecx + 2f0h], 0ffffff41h // B6FC16
        movss dword ptr [ecx + 1d8h], xmm0 // B6FC20
        ret // original B6FC28 RET4
    }
}

__declspec(naked) std::uint32_t __fastcall set_native_camera_clear_flags_00b6fe10(
    void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [edx] // B6FE10: original [ESP+4]
        mov dword ptr [ecx + 188h], eax // B6FE14
        ret // original B6FE1A RET4; EAX remains complete input DWORD
    }
}

__declspec(naked) void __fastcall set_native_camera_clear_depth_00b6fe20(
    void*, const void*) noexcept {
    __asm {
        movss xmm0, dword ptr [edx] // B6FE20: original [ESP+4]
        movss dword ptr [ecx + 18ch], xmm0 // B6FE26
        ret // original B6FE2E RET4
    }
}

} // namespace bsp
