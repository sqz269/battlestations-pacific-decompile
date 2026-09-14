#include "bsp/native_model_stream_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native model stream queries require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) const void* __fastcall get_native_stream_decode_record_00b61e10(
    const void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        shl eax, 5
        add eax, dword ptr [ecx + 50h]
        ret 4
    }
}

__declspec(naked) std::int32_t __fastcall get_native_model_bone_count_00b8ff00(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 188h]
        ret
    }
}

__declspec(naked) void* __fastcall get_native_model_bone_node_00b90620(
    const void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 184h]
        mov ecx, dword ptr [esp + 4]
        mov eax, dword ptr [eax + ecx*4]
        ret 4
    }
}
} // namespace bsp
