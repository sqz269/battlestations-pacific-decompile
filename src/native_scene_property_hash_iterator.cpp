#include "bsp/native_scene_property_hash_iterator.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "This raw iterator pair requires MSVC Win32.");

// Whole [008EEB10,008EEB51): 65 bytes, 22 instructions; no CALL provider.
__declspec(naked) void __fastcall begin_native_scene_property_hash_008eeb10(void*) {
    __asm {
        mov edx, dword ptr [ecx]
        cmp dword ptr [edx + 4], 0
        jz L_begin_empty
        mov dword ptr [ecx + 8], 0
        cmp dword ptr [edx + 8], 0
        jnz L_begin_publish
        push edi
    L_begin_scan:
        mov eax, dword ptr [ecx + 8]
        cmp eax, 40h
        jnc L_begin_scan_done
        add eax, 1
        mov edi, edx
        mov dword ptr [ecx + 8], eax
        cmp dword ptr [edi + eax * 4 + 8], 0
        jz L_begin_scan
    L_begin_scan_done:
        pop edi
    L_begin_publish:
        mov eax, dword ptr [ecx + 8]
        mov edx, dword ptr [edx + eax * 4 + 8]
        mov dword ptr [ecx + 4], edx
        ret
    L_begin_empty:
        mov dword ptr [ecx + 4], 0
        ret
    }
}

// Whole [008EEB60,008EEBBC): 92 bytes, 33 instructions; no CALL provider.
__declspec(naked) std::uint32_t __fastcall next_native_scene_property_hash_008eeb60(void*) {
    __asm {
        mov eax, dword ptr [ecx + 4]
        test eax, eax
        jz L_next_result
        mov eax, dword ptr [eax + 0ch]
        test eax, eax
        jnz L_next_publish
        add dword ptr [ecx + 8], 1
        mov eax, dword ptr [ecx + 8]
        cmp eax, 40h
        jnc L_next_after_scan
        mov edx, dword ptr [ecx]
        // Preserve Original 8D 64 24 00, rather than the shorter LEA encoding.
        _emit 08dh
        _emit 064h
        _emit 024h
        _emit 000h
    L_next_scan:
        cmp dword ptr [edx + eax * 4 + 8], 0
        jnz L_next_after_scan
        add eax, 1
        cmp eax, 40h
        mov dword ptr [ecx + 8], eax
        jc L_next_scan
    L_next_after_scan:
        mov eax, dword ptr [ecx + 8]
        cmp eax, 40h
        jnc L_next_clear
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [edx + eax * 4 + 8]
    L_next_publish:
        mov dword ptr [ecx + 4], eax
        xor eax, eax
        cmp dword ptr [ecx + 4], eax
        setnz al
        ret
    L_next_clear:
        mov dword ptr [ecx + 4], 0
    L_next_result:
        xor eax, eax
        cmp dword ptr [ecx + 4], eax
        setnz al
        ret
    }
}

} // namespace bsp
