#include "bsp/native_zlib_raw_gen_bitlen_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw gen_bitlen register/frame entry requires MSVC Win32.
#endif

namespace {
// Complete 00BCDF60..00BCE0A9 and 00BCE0B0..00BCE176 owned body.
// EAX=raw backing, ECX=raw descriptor; no actual stack arguments, plain RET.
// Keep all 144 named instructions. The unowned six-byte alignment hole is
// omitted and every internal branch resolves to its corresponding Source label.
// Actual locals, saves, narrow RMWs and control cells remain real and aliasable.
// No ordinary C++ call establishes this private custom register entry.
__declspec(naked) void __cdecl gen_bitlen_body() noexcept {
    __asm {
        sub esp, 020h // 00bcdf60
        mov edx, dword ptr [ecx + 4] // 00bcdf63
        push ebx // 00bcdf66
        mov ebx, dword ptr [ecx] // 00bcdf67
        mov ecx, dword ptr [ecx + 8] // 00bcdf69
        mov dword ptr [esp + 018h], edx // 00bcdf6c
        mov edx, dword ptr [ecx + 4] // 00bcdf70
        push ebp // 00bcdf73
        mov ebp, dword ptr [ecx] // 00bcdf74
        mov dword ptr [esp + 024h], edx // 00bcdf76
        mov edx, dword ptr [ecx + 8] // 00bcdf7a
        push esi // 00bcdf7d
        mov esi, dword ptr [ecx + 010h] // 00bcdf7e
        xor ecx, ecx // 00bcdf81
        mov dword ptr [eax + 0b34h], ecx // 00bcdf83
        mov dword ptr [eax + 0b38h], ecx // 00bcdf89
        mov dword ptr [eax + 0b3ch], ecx // 00bcdf8f
        mov dword ptr [eax + 0b40h], ecx // 00bcdf95
        mov dword ptr [eax + 0b44h], ecx // 00bcdf9b
        mov dword ptr [eax + 0b48h], ecx // 00bcdfa1
        mov dword ptr [eax + 0b4ch], ecx // 00bcdfa7
        mov dword ptr [eax + 0b50h], ecx // 00bcdfad
        mov dword ptr [esp + 024h], edx // 00bcdfb3
        mov edx, dword ptr [eax + 0144ch] // 00bcdfb7
        mov edx, dword ptr [eax + edx*4 + 0b54h] // 00bcdfbd
        push edi // 00bcdfc4
        mov word ptr [ebx + edx*4 + 2], cx // 00bcdfc5
        mov edi, dword ptr [eax + 0144ch] // 00bcdfca
        add edi, 1 // 00bcdfd0
        cmp edi, 023dh // 00bcdfd3
        mov dword ptr [esp + 01ch], esi // 00bcdfd9
        mov dword ptr [esp + 018h], ecx // 00bcdfdd
        jge block_00bce16f // 00bcdfe1
        lea ecx, [eax + edi*4 + 0b54h] // 00bcdfe7
        mov dword ptr [esp + 014h], ecx // 00bcdfee
        mov ecx, 023dh // 00bcdff2
        sub ecx, edi // 00bcdff7
        add edi, ecx // 00bcdff9
        mov dword ptr [esp + 020h], ecx // 00bcdffb
        mov dword ptr [esp + 010h], edi // 00bcdfff
        jmp block_00bce009 // 00bce003
    block_00bce005:
        mov esi, dword ptr [esp + 01ch] // 00bce005
    block_00bce009:
        mov edx, dword ptr [esp + 014h] // 00bce009
        mov edx, dword ptr [edx] // 00bce00d
        movzx ecx, word ptr [ebx + edx*4 + 2] // 00bce00f
        movzx ecx, word ptr [ebx + ecx*4 + 2] // 00bce014
        add ecx, 1 // 00bce019
        cmp ecx, esi // 00bce01c
        jle block_00bce027 // 00bce01e
        add dword ptr [esp + 018h], 1 // 00bce020
        mov ecx, esi // 00bce025
    block_00bce027:
        cmp edx, dword ptr [esp + 024h] // 00bce027
        mov word ptr [ebx + edx*4 + 2], cx // 00bce02b
        jg block_00bce07a // 00bce030
        mov edi, dword ptr [esp + 028h] // 00bce032
        add word ptr [eax + ecx*2 + 0b34h], 1 // 00bce036
        xor esi, esi // 00bce03f
        cmp edx, edi // 00bce041
        jl block_00bce050 // 00bce043
        mov esi, edx // 00bce045
        sub esi, edi // 00bce047
        mov edi, dword ptr [esp + 02ch] // 00bce049
        mov esi, dword ptr [edi + esi*4] // 00bce04d
    block_00bce050:
        movzx edi, word ptr [ebx + edx*4] // 00bce050
        movzx edi, di // 00bce054
        add ecx, esi // 00bce057
        imul ecx, edi // 00bce059
        add dword ptr [eax + 016a0h], ecx // 00bce05c
        test ebp, ebp // 00bce062
        je block_00bce076 // 00bce064
        movzx edx, word ptr [ebp + edx*4 + 2] // 00bce066
        add edx, esi // 00bce06b
        imul edx, edi // 00bce06d
        add dword ptr [eax + 016a4h], edx // 00bce070
    block_00bce076:
        mov edi, dword ptr [esp + 010h] // 00bce076
    block_00bce07a:
        add dword ptr [esp + 014h], 4 // 00bce07a
        sub dword ptr [esp + 020h], 1 // 00bce07f
        jne block_00bce005 // 00bce084
        mov ebp, dword ptr [esp + 018h] // 00bce08a
        test ebp, ebp // 00bce08e
        je block_00bce16f // 00bce090
        mov ecx, dword ptr [esp + 01ch] // 00bce096
        lea edx, [ecx - 1] // 00bce09a
        mov dword ptr [esp + 02ch], edx // 00bce09d
        lea esi, [eax + ecx*2 + 0b34h] // 00bce0a1
        jmp block_00bce0b0 // 00bce0a8
    block_00bce0b0:
        mov ecx, dword ptr [esp + 02ch] // 00bce0b0
        cmp word ptr [eax + ecx*2 + 0b34h], 0 // 00bce0b4
        lea edx, [eax + ecx*2 + 0b34h] // 00bce0bd
        jne block_00bce0d2 // 00bce0c4
    block_00bce0c6:
        sub edx, 2 // 00bce0c6
        sub ecx, 1 // 00bce0c9
        cmp word ptr [edx], 0 // 00bce0cc
        je block_00bce0c6 // 00bce0d0
    block_00bce0d2:
        add word ptr [eax + ecx*2 + 0b34h], 0ffffh // 00bce0d2
        add word ptr [eax + ecx*2 + 0b36h], 2 // 00bce0dc
        add word ptr [esi], 0ffffh // 00bce0e5
        sub ebp, 2 // 00bce0ea
        test ebp, ebp // 00bce0ed
        jg block_00bce0b0 // 00bce0ef
        mov edx, dword ptr [esp + 01ch] // 00bce0f1
        test edx, edx // 00bce0f5
        je block_00bce16f // 00bce0f7
        mov dword ptr [esp + 020h], esi // 00bce0f9
        lea ecx, [ecx] // 00bce0fd
    block_00bce100:
        movzx esi, word ptr [esi] // 00bce100
        test esi, esi // 00bce103
        mov dword ptr [esp + 01ch], esi // 00bce105
        je block_00bce15d // 00bce109
        lea ebp, [eax + edi*4 + 0b54h] // 00bce10b
    block_00bce112:
        mov ecx, dword ptr [ebp - 4] // 00bce112
        sub dword ptr [esp + 010h], 1 // 00bce115
        sub ebp, 4 // 00bce11a
        cmp ecx, dword ptr [esp + 024h] // 00bce11d
        mov dword ptr [esp + 02ch], ebp // 00bce121
        jg block_00bce155 // 00bce125
        movzx edi, word ptr [ebx + ecx*4 + 2] // 00bce127
        cmp edi, edx // 00bce12c
        lea esi, [ebx + ecx*4 + 2] // 00bce12e
        je block_00bce14c // 00bce132
        movzx ecx, word ptr [ebx + ecx*4] // 00bce134
        mov ebp, edx // 00bce138
        sub ebp, edi // 00bce13a
        imul ebp, ecx // 00bce13c
        add dword ptr [eax + 016a0h], ebp // 00bce13f
        mov ebp, dword ptr [esp + 02ch] // 00bce145
        mov word ptr [esi], dx // 00bce149
    block_00bce14c:
        sub dword ptr [esp + 01ch], 1 // 00bce14c
        mov esi, dword ptr [esp + 01ch] // 00bce151
    block_00bce155:
        test esi, esi // 00bce155
        jne block_00bce112 // 00bce157
        mov edi, dword ptr [esp + 010h] // 00bce159
    block_00bce15d:
        mov esi, dword ptr [esp + 020h] // 00bce15d
        sub edx, 1 // 00bce161
        sub esi, 2 // 00bce164
        test edx, edx // 00bce167
        mov dword ptr [esp + 020h], esi // 00bce169
        jne block_00bce100 // 00bce16d
    block_00bce16f:
        pop edi // 00bce16f
        pop esi // 00bce170
        pop ebp // 00bce171
        pop ebx // 00bce172
        add esp, 020h // 00bce173
        ret // 00bce176
    }
}
} // namespace

// Three genuine Source inputs select physical kernel registers and a real
// Source execution frame. No descriptor/frame/local/control data is copied.
// Source-only noreturn is not an Original annotation or a normal-return ABI.
extern "C" __declspec(naked) __declspec(noreturn) void __cdecl
bsp_native_zlib_gen_bitlen_frame_00bcdf60(void*, const void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        mov esp, dword ptr [esp + 00ch]
        jmp gen_bitlen_body
    }
}
