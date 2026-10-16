#include "bsp/native_zlib_raw_reset_type_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw zlib reset/type bodies require MSVC Win32.
#endif

// 00BC9650 inflateReset: cache the raw state address once. Ordered stores can
// alias either backing, pointer cells or the current return slot. LEA computes
// address bits without dereferencing state+528h. No zlib private class is used.
extern "C" __declspec(naked) std::int32_t __fastcall bsp_native_zlib_raw_reset_00bc9650(void*) noexcept {
    __asm {
        xor edx, edx
        cmp ecx, edx
        je invalid_backing
        mov eax, dword ptr [ecx + 01ch]
        cmp eax, edx
        je invalid_backing
        mov dword ptr [eax + 018h], edx
        mov dword ptr [ecx + 014h], edx
        mov dword ptr [ecx + 008h], edx
        mov dword ptr [ecx + 018h], edx
        lea ecx, [eax + 0528h]
        mov dword ptr [eax], edx
        mov dword ptr [eax + 004h], edx
        mov dword ptr [eax + 00ch], edx
        mov dword ptr [eax + 020h], edx
        mov dword ptr [eax + 024h], edx
        mov dword ptr [eax + 030h], edx
        mov dword ptr [eax + 034h], edx
        mov dword ptr [eax + 064h], ecx
        mov dword ptr [eax + 048h], ecx
        mov dword ptr [eax + 044h], ecx
        xor eax, eax
        ret
    invalid_backing:
        mov eax, 0fffffffeh
        ret
    }
}

// 00BCEE90 set_data_type: seven ordered WORD reads precede stack saves.
// Each loop rereads WORD cells; final byte output precedes the last POP.
// Preserve incidental EAX/flags and actual saved-cell aliases, not just type.
extern "C" __declspec(naked) void __fastcall bsp_native_zlib_raw_type_00bcee90(void*) noexcept {
    __asm {
        movzx edx, word ptr [ecx + 0a0h]
        movzx eax, word ptr [ecx + 0a4h]
        add eax, edx
        movzx edx, word ptr [ecx + 09ch]
        add eax, edx
        movzx edx, word ptr [ecx + 098h]
        add eax, edx
        movzx edx, word ptr [ecx + 094h]
        add eax, edx
        movzx edx, word ptr [ecx + 090h]
        add eax, edx
        movzx edx, word ptr [ecx + 08ch]
        push ebx
        push esi
        xor esi, esi
        push edi
        add eax, edx
        lea edx, [ecx + 0a8h]
        mov edi, 079h
        // Native three-byte LEA ECX,[ECX+0], preserving all flags.
        _emit 08dh
        _emit 049h
        _emit 000h
    sum_text_words:
        movzx ebx, word ptr [edx]
        add esi, ebx
        add edx, 004h
        sub edi, 001h
        jne sum_text_words
        lea edx, [ecx + 028ch]
        mov edi, 080h
    sum_other_words:
        movzx ebx, word ptr [edx]
        add eax, ebx
        add edx, 004h
        sub edi, 001h
        jne sum_other_words
        shr esi, 002h
        cmp eax, esi
        pop edi
        setbe al
        pop esi
        mov byte ptr [ecx + 01ch], al
        pop ebx
        ret
    }
}
