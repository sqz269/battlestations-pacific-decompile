#include "bsp/native_plane_set.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native plane-set storage requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
// B250B0..B25209: the original eight-record unroll executes twice. Retaining
// its address calculations also retains exact overlap and fault boundaries.
// The eight unreachable alignment bytes are preserved for whole-body proof.
__declspec(naked) void __fastcall copy_native_plane_set_records_00b250b0(
    void*, void*, const void*) {
    __asm {
        push ebx
        push esi
        push edi
        mov edi, dword ptr [esp + 10h]
        mov eax, ecx
        lea esi, [edi + 20h]
        lea edx, [eax + 18h]
        sub edi, eax
        mov ebx, 2
        jmp short copy_block
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 090h
    copy_block:
        fld dword ptr [esi - 20h]
        add esi, 0a0h
        fstp dword ptr [edx - 18h]
        fld dword ptr [esi - 0bch]
        fstp dword ptr [edx - 14h]
        fld dword ptr [esi - 0b8h]
        fstp dword ptr [edx - 10h]
        fld dword ptr [esi - 0b4h]
        fstp dword ptr [edx - 0ch]
        mov ecx, dword ptr [esi - 0b0h]
        mov dword ptr [edx - 8], ecx
        fld dword ptr [esi - 0ach]
        fstp dword ptr [edx - 4]
        fld dword ptr [edi + edx]
        fstp dword ptr [edx]
        fld dword ptr [esi - 0a4h]
        fstp dword ptr [edx + 4]
        fld dword ptr [esi - 0a0h]
        fstp dword ptr [edx + 8]
        mov ecx, dword ptr [esi - 09ch]
        mov dword ptr [edx + 0ch], ecx
        fld dword ptr [esi - 098h]
        fstp dword ptr [edx + 10h]
        fld dword ptr [esi - 094h]
        fstp dword ptr [edx + 14h]
        fld dword ptr [esi - 090h]
        fstp dword ptr [edx + 18h]
        fld dword ptr [esi - 08ch]
        fstp dword ptr [edx + 1ch]
        mov ecx, dword ptr [esi - 088h]
        mov dword ptr [edx + 20h], ecx
        fld dword ptr [esi - 084h]
        fstp dword ptr [edx + 24h]
        fld dword ptr [esi - 080h]
        fstp dword ptr [edx + 28h]
        fld dword ptr [esi - 07ch]
        fstp dword ptr [edx + 2ch]
        fld dword ptr [esi - 078h]
        fstp dword ptr [edx + 30h]
        mov ecx, dword ptr [esi - 074h]
        mov dword ptr [edx + 34h], ecx
        fld dword ptr [esi - 070h]
        fstp dword ptr [edx + 38h]
        fld dword ptr [esi - 06ch]
        fstp dword ptr [edx + 3ch]
        fld dword ptr [esi - 068h]
        fstp dword ptr [edx + 40h]
        fld dword ptr [esi - 064h]
        fstp dword ptr [edx + 44h]
        mov ecx, dword ptr [esi - 060h]
        mov dword ptr [edx + 48h], ecx
        fld dword ptr [esi - 05ch]
        fstp dword ptr [edx + 4ch]
        fld dword ptr [esi - 058h]
        fstp dword ptr [edx + 50h]
        fld dword ptr [esi - 054h]
        fstp dword ptr [edx + 54h]
        fld dword ptr [esi - 050h]
        fstp dword ptr [edx + 58h]
        mov ecx, dword ptr [esi - 04ch]
        mov dword ptr [edx + 5ch], ecx
        fld dword ptr [esi - 048h]
        fstp dword ptr [edx + 60h]
        fld dword ptr [esi - 044h]
        fstp dword ptr [edx + 64h]
        fld dword ptr [esi - 040h]
        fstp dword ptr [edx + 68h]
        fld dword ptr [esi - 03ch]
        fstp dword ptr [edx + 6ch]
        mov ecx, dword ptr [esi - 038h]
        mov dword ptr [edx + 70h], ecx
        fld dword ptr [esi - 034h]
        fstp dword ptr [edx + 74h]
        fld dword ptr [esi - 030h]
        fstp dword ptr [edx + 78h]
        fld dword ptr [esi - 02ch]
        fstp dword ptr [edx + 7ch]
        fld dword ptr [esi - 028h]
        fstp dword ptr [edx + 80h]
        mov ecx, dword ptr [esi - 024h]
        mov dword ptr [edx + 84h], ecx
        add edx, 0a0h
        sub ebx, 1
        jnz copy_block
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall
native_plane_set_count_00b65080(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 140h]
        ret
    }
}

__declspec(naked) const NativePlaneRecord* __fastcall
native_plane_set_plane_00b656f0(const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        lea eax, [eax + eax * 4]
        lea eax, [ecx + eax * 4]
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall
native_plane_set_flags_00b65700(const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        lea eax, [eax + eax * 4]
        mov eax, dword ptr [ecx + eax * 4 + 10h]
        ret 4
    }
}
} // namespace bsp
