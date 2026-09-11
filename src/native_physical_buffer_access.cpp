#include "bsp/native_physical_buffer_access.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical buffer access requires MSVC Win32 assembly.
#endif

namespace bsp {
__declspec(naked) void __fastcall
release_native_physical_vertex_buffer_for_reset_00b23270(void*) {
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 28h]
        test edi, edi
        jz short vertex_final
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [eax + 4]
        push edi
        call ecx
        mov edx, dword ptr [edi]
        mov eax, dword ptr [edx + 8]
        push edi
        call eax
    vertex_final:
        mov eax, dword ptr [esi + 28h]
        test eax, eax
        jz short vertex_done
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 8]
        push eax
        call edx
        mov dword ptr [esi + 28h], 0
    vertex_done:
        pop edi
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall
release_native_physical_index_buffer_for_reset_00b23180(void*) {
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 28h]
        test edi, edi
        jz short index_final
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [eax + 4]
        push edi
        call ecx
        mov edx, dword ptr [edi]
        mov eax, dword ptr [edx + 8]
        push edi
        call eax
    index_final:
        mov eax, dword ptr [esi + 28h]
        test eax, eax
        jz short index_done
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 8]
        push eax
        call edx
        mov dword ptr [esi + 28h], 0
    index_done:
        pop edi
        pop esi
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
native_physical_index_buffer_capacity_00b4b800(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 18h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
native_physical_vertex_buffer_capacity_00b4b9b0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 18h]
        ret
    }
}

__declspec(naked) void __fastcall
unlock_native_physical_index_buffer_00b4b820(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 28h]
        test eax, eax
        jz short index_unlock_done
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 30h]
        push eax
        call edx
        add dword ptr [esi + 20h], -1
    index_unlock_done:
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall
unlock_native_physical_vertex_buffer_00b4b9d0(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 28h]
        test eax, eax
        jz short vertex_unlock_done
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 30h]
        push eax
        call edx
        add dword ptr [esi + 20h], -1
    vertex_unlock_done:
        pop esi
        ret
    }
}
} // namespace bsp
