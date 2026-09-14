#include "bsp/native_material_geometry_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material geometry queries require MSVC Win32.
#endif

namespace bsp {

// Complete original 00B48D50[4]: ECX+70 DWORD -> EAX; RET.
__declspec(naked) std::uint32_t __fastcall get_native_logical_vertex_base_00b48d50(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 070h]
        ret
    }
}

// Complete original 00B48DE0[4]: ECX+20 DWORD -> EAX; RET.
__declspec(naked) std::uint32_t __fastcall get_native_logical_index_base_00b48de0(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 020h]
        ret
    }
}

// Complete original 00B855A0[4]: ECX+1C DWORD -> EAX; RET.
__declspec(naked) std::uint32_t __fastcall get_native_draw_section_instance_count_00b855a0(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 01ch]
        ret
    }
}

// Complete original 00B855F0[4]: ECX+58 byte -> AL only; RET.
__declspec(naked) std::uint8_t __fastcall get_native_draw_section_indexed_byte_00b855f0(const void*) {
    __asm {
        mov al, byte ptr [ecx + 058h]
        ret
    }
}

} // namespace bsp
