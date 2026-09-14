#include "bsp/native_crt_memset.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT memset requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(int) == 4);

// Complete native entry 00C0C90E; exact contracts are documented in header.
__declspec(naked) void __cdecl zero_native_crt_aligned_128_chunks_00c0c90e(
    void*, std::uint32_t) {
    __asm {
        // Native 00c0c90e
        push ebp
        // Native 00c0c90f
        mov ebp, esp
        // Native 00c0c911
        sub esp, 4
        // Native 00c0c914
        mov dword ptr [ebp - 4], edi
        // Native 00c0c917
        mov edi, dword ptr [ebp + 8]
        // Native 00c0c91a
        mov ecx, dword ptr [ebp + 0xc]
        // Native 00c0c91d
        shr ecx, 7
        // Native 00c0c920
        pxor xmm0, xmm0
        // Native 00c0c924
        jmp native_00c0c92e
        // Native 00c0c926
        // Original skipped padding; preceding JMP reaches C0C92E.
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Native 00c0c92d
        // Original skipped padding; preceding JMP reaches C0C92E.
        _emit 0x90
    native_00c0c92e:
        movdqa xmmword ptr [edi], xmm0
        // Native 00c0c932
        movdqa xmmword ptr [edi + 0x10], xmm0
        // Native 00c0c937
        movdqa xmmword ptr [edi + 0x20], xmm0
        // Native 00c0c93c
        movdqa xmmword ptr [edi + 0x30], xmm0
        // Native 00c0c941
        movdqa xmmword ptr [edi + 0x40], xmm0
        // Native 00c0c946
        movdqa xmmword ptr [edi + 0x50], xmm0
        // Native 00c0c94b
        movdqa xmmword ptr [edi + 0x60], xmm0
        // Native 00c0c950
        movdqa xmmword ptr [edi + 0x70], xmm0
        // Native 00c0c955
        lea edi, [edi + 0x80]
        // Native 00c0c95b
        dec ecx
        // Native 00c0c95c
        jne native_00c0c92e
        // Native 00c0c95e
        mov edi, dword ptr [ebp - 4]
        // Native 00c0c961
        mov esp, ebp
        // Native 00c0c963
        pop ebp
        // Native 00c0c964
        ret
    }
}

// Complete native entry 00C0C965; exact contracts are documented in header.
__declspec(naked) void* __cdecl zero_native_crt_vector_00c0c965(
    void*, int, std::uint32_t) {
    __asm {
        // Native 00c0c965
        push ebp
        // Native 00c0c966
        mov ebp, esp
        // Native 00c0c968
        sub esp, 0x10
        // Native 00c0c96b
        mov dword ptr [ebp - 4], edi
        // Native 00c0c96e
        mov eax, dword ptr [ebp + 8]
        // Native 00c0c971
        cdq
        // Native 00c0c972
        mov edi, eax
        // Native 00c0c974
        xor edi, edx
        // Native 00c0c976
        sub edi, edx
        // Native 00c0c978
        and edi, 0xf
        // Native 00c0c97b
        xor edi, edx
        // Native 00c0c97d
        sub edi, edx
        // Native 00c0c97f
        test edi, edi
        // Native 00c0c981
        jne native_00c0c9bf
        // Native 00c0c983
        mov ecx, dword ptr [ebp + 0x10]
        // Native 00c0c986
        mov edx, ecx
        // Native 00c0c988
        and edx, 0x7f
        // Native 00c0c98b
        mov dword ptr [ebp - 0xc], edx
        // Native 00c0c98e
        cmp ecx, edx
        // Native 00c0c990
        je native_00c0c9a4
        // Native 00c0c992
        sub ecx, edx
        // Native 00c0c994
        push ecx
        // Native 00c0c995
        push eax
        // Native 00c0c996
        call zero_native_crt_aligned_128_chunks_00c0c90e
        // Native 00c0c99b
        add esp, 8
        // Native 00c0c99e
        mov eax, dword ptr [ebp + 8]
        // Native 00c0c9a1
        mov edx, dword ptr [ebp - 0xc]
    native_00c0c9a4:
        test edx, edx
        // Native 00c0c9a6
        je native_00c0c9ed
        // Native 00c0c9a8
        add eax, dword ptr [ebp + 0x10]
        // Native 00c0c9ab
        sub eax, edx
        // Native 00c0c9ad
        mov dword ptr [ebp - 8], eax
        // Native 00c0c9b0
        xor eax, eax
        // Native 00c0c9b2
        mov edi, dword ptr [ebp - 8]
        // Native 00c0c9b5
        mov ecx, dword ptr [ebp - 0xc]
        // Native 00c0c9b8
        rep stosb
        // Native 00c0c9ba
        mov eax, dword ptr [ebp + 8]
        // Native 00c0c9bd
        jmp native_00c0c9ed
    native_00c0c9bf:
        neg edi
        // Native 00c0c9c1
        add edi, 0x10
        // Native 00c0c9c4
        mov dword ptr [ebp - 0x10], edi
        // Native 00c0c9c7
        xor eax, eax
        // Native 00c0c9c9
        mov edi, dword ptr [ebp + 8]
        // Native 00c0c9cc
        mov ecx, dword ptr [ebp - 0x10]
        // Native 00c0c9cf
        rep stosb
        // Native 00c0c9d1
        mov eax, dword ptr [ebp - 0x10]
        // Native 00c0c9d4
        mov ecx, dword ptr [ebp + 8]
        // Native 00c0c9d7
        mov edx, dword ptr [ebp + 0x10]
        // Native 00c0c9da
        add ecx, eax
        // Native 00c0c9dc
        sub edx, eax
        // Native 00c0c9de
        push edx
        // Native 00c0c9df
        push 0
        // Native 00c0c9e1
        push ecx
        // Native 00c0c9e2
        call zero_native_crt_vector_00c0c965
        // Native 00c0c9e7
        add esp, 0xc
        // Native 00c0c9ea
        mov eax, dword ptr [ebp + 8]
    native_00c0c9ed:
        mov edi, dword ptr [ebp - 4]
        // Native 00c0c9f0
        mov esp, ebp
        // Native 00c0c9f2
        pop ebp
        // Native 00c0c9f3
        ret
    }
}

// Complete native entry 00BF79F0; exact contracts are documented in header.
__declspec(naked) void* __cdecl fill_native_crt_bytes_00bf79f0(
    void*, int, std::uint32_t, const volatile std::uint32_t&) {
    __asm {
        // Native 00bf79f0
        mov edx, dword ptr [esp + 0xc]
        // Native 00bf79f4
        mov ecx, dword ptr [esp + 4]
        // Native 00bf79f8
        test edx, edx
        // Native 00bf79fa
        je native_00bf7a65
        // Native 00bf79fc
        xor eax, eax
        // Native 00bf79fe
        mov al, byte ptr [esp + 8]
        // Native 00bf7a02
        test al, al
        // Native 00bf7a04
        jne native_00bf7a1c
        // Native 00bf7a06
        cmp edx, 0x100
        // Native 00bf7a0c
        jb native_00bf7a1c
        // Native 00bf7a0e
        // Restore live zero fill with MOV so CMP flags reach the original JE.
        mov eax, dword ptr [esp + 0x10]
        cmp dword ptr [eax], 0
        mov eax, 0
        // Native 00bf7a15
        je native_00bf7a1c
        // Native 00bf7a17
        jmp zero_native_crt_vector_00c0c965
    native_00bf7a1c:
        push edi
        // Native 00bf7a1d
        mov edi, ecx
        // Native 00bf7a1f
        cmp edx, 4
        // Native 00bf7a22
        jb native_00bf7a55
        // Native 00bf7a24
        neg ecx
        // Native 00bf7a26
        and ecx, 3
        // Native 00bf7a29
        je native_00bf7a37
        // Native 00bf7a2b
        sub edx, ecx
    native_00bf7a2d:
        mov byte ptr [edi], al
        // Native 00bf7a2f
        add edi, 1
        // Native 00bf7a32
        sub ecx, 1
        // Native 00bf7a35
        jne native_00bf7a2d
    native_00bf7a37:
        mov ecx, eax
        // Native 00bf7a39
        shl eax, 8
        // Native 00bf7a3c
        add eax, ecx
        // Native 00bf7a3e
        mov ecx, eax
        // Native 00bf7a40
        shl eax, 0x10
        // Native 00bf7a43
        add eax, ecx
        // Native 00bf7a45
        mov ecx, edx
        // Native 00bf7a47
        and edx, 3
        // Native 00bf7a4a
        shr ecx, 2
        // Native 00bf7a4d
        je native_00bf7a55
        // Native 00bf7a4f
        rep stosd
        // Native 00bf7a51
        test edx, edx
        // Native 00bf7a53
        je native_00bf7a5f
    native_00bf7a55:
        mov byte ptr [edi], al
        // Native 00bf7a57
        add edi, 1
        // Native 00bf7a5a
        sub edx, 1
        // Native 00bf7a5d
        jne native_00bf7a55
    native_00bf7a5f:
        mov eax, dword ptr [esp + 8]
        // Native 00bf7a63
        pop edi
        // Native 00bf7a64
        ret
    native_00bf7a65:
        mov eax, dword ptr [esp + 4]
        // Native 00bf7a69
        ret
    }
}

} // namespace bsp
