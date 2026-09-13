#include "bsp/native_fog_access.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native fog access requires MSVC Win32 x87 and 32-bit address arithmetic.
#endif

namespace bsp {
__declspec(naked) const void* __fastcall get_native_fog_underwater_color_00b84c90(const void*) {
    __asm { lea eax, [ecx + 18h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_68_00b84ca0(const void*) {
    __asm { fld dword ptr [ecx + 68h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_6c_00b84cb0(const void*) {
    __asm { fld dword ptr [ecx + 6ch] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_70_00b84cc0(const void*) {
    __asm { fld dword ptr [ecx + 70h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_74_00b84cd0(const void*) {
    __asm { fld dword ptr [ecx + 74h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_78_00b84ce0(const void*) {
    __asm { fld dword ptr [ecx + 78h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_7c_00b84cf0(const void*) {
    __asm { fld dword ptr [ecx + 7ch] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_80_00b84da0(const void*) {
    __asm { fld dword ptr [ecx + 80h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_84_00b84db0(const void*) {
    __asm { fld dword ptr [ecx + 84h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_88_00b84e20(const void*) {
    __asm { fld dword ptr [ecx + 88h] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_8c_00b84e30(const void*) {
    __asm { fld dword ptr [ecx + 8ch] }
    __asm { ret }
}

__declspec(naked) float __fastcall load_native_fog_scalar_90_00b84e40(const void*) {
    __asm { fld dword ptr [ecx + 90h] }
    __asm { ret }
}

__declspec(naked) const void* __fastcall get_native_fog_directional_color_00b84fd0(const void*, std::uint32_t) {
    __asm { mov eax, edx }
    __asm { shl eax, 4 }
    __asm { lea eax, [eax + ecx + 28h] }
    __asm { ret }
}

} // namespace bsp
