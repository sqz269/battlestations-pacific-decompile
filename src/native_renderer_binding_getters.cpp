#include "bsp/native_renderer_binding_getters.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer binding getters require MSVC Win32 assembly.
#endif

namespace bsp {
__declspec(naked) void* __fastcall
native_texture_get_com_00b3cea0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 10h]
        ret
    }
}

__declspec(naked) void* __fastcall
native_logical_vertex_stream_get_declaration_00b48ce0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 68h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
native_logical_vertex_stream_get_offset_00b48d10(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 5ch]
        ret
    }
}

__declspec(naked) void* __fastcall
native_logical_vertex_stream_get_buffer_00b48cf0(const void*) {
    __asm {
        mov ecx, dword ptr [ecx + 58h]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 1ch]
        jmp edx
    }
}

__declspec(naked) void* __fastcall
native_logical_index_stream_get_buffer_00b48dc0(const void*) {
    __asm {
        mov ecx, dword ptr [ecx + 08h]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 1ch]
        jmp edx
    }
}

__declspec(naked) void* __fastcall
native_physical_vertex_buffer_get_com_00b4b9f0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 28h]
        ret
    }
}

__declspec(naked) void* __fastcall
native_physical_index_buffer_get_com_00b4b840(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 28h]
        ret
    }
}

__declspec(naked) void* __fastcall
native_vertex_layout_get_com_00b5ff00(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 40h]
        ret
    }
}
} // namespace bsp
