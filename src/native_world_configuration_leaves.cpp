#include "bsp/native_world_configuration_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native world configuration leaves require MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void __fastcall set_native_camera_clear_color_00b6fe50(
    void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov dword ptr [ecx + 190h], eax
        ret 4
    }
}

__declspec(naked) void __fastcall set_native_renderer_texture_detail_bias_00b1ffb0(
    void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov dword ptr [ecx + 1d84h], eax
        ret 4
    }
}

__declspec(naked) void __fastcall set_native_foliage_byte_00ad5750(
    void*, void*, std::uint32_t) {
    __asm {
        mov al, byte ptr [esp + 4]
        mov byte ptr [ecx + 10h], al
        ret 4
    }
}

} // namespace bsp
