#include "bsp/native_material_pass_execution.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pass execution leaves require MSVC Win32.
#endif

namespace bsp {
// Kept independently linkable: the genuine base leaf has no derived-pass,
// renderer, constant-builder, diagnostic or retained-owner dependency.
__declspec(naked) void __fastcall execute_native_material_pass_base_00b5e5e0(
    void*, void*, void*) noexcept {
    __asm { ret 4 }
}

__declspec(naked) void* __fastcall get_native_material_plane_context_00b1bfa0(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 30h]
        ret
    }
}
} // namespace bsp
