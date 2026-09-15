#include "bsp/native_shadow_draw_leaves.hpp"
#include "bsp/native_plane_set.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow draw leaves require MSVC Win32 assembly.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall native_logical_vertex_physical_00b48d00(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 58h] // B48D00
        _emit 0c3h // B48D03: exact RET, not RET0.
    }
}

__declspec(naked) const NativePlaneRecord* __fastcall native_camera_plane_00b6fdc0(
    const void*, void*, std::uint32_t) {
    __asm {
        add ecx, 2f4h // B6FDC0
        jmp native_plane_set_plane_00b656f0 // B6FDC6; existing RET4.
    }
}
} // namespace bsp
