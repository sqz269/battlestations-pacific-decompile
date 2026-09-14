#include "bsp/native_crt_vector_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT vector copying requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Complete native entry 00C0C7A4; exact stack/register contract in header.
__declspec(naked) void __cdecl copy_native_crt_aligned_128_chunks_00c0c7a4(
    void*, const void*, std::uint32_t) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 8
        mov dword ptr [ebp - 4], edi
        mov dword ptr [ebp - 8], esi
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp + 0x10]
        shr ecx, 7
        jmp chunk_00c0c7c4
        // Original six-byte self-LEA padding, skipped by the preceding JMP.
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    chunk_00c0c7c4:
        movdqa xmm0, xmmword ptr [esi]
        movdqa xmm1, xmmword ptr [esi + 0x10]
        movdqa xmm2, xmmword ptr [esi + 0x20]
        movdqa xmm3, xmmword ptr [esi + 0x30]
        movdqa xmmword ptr [edi], xmm0
        movdqa xmmword ptr [edi + 0x10], xmm1
        movdqa xmmword ptr [edi + 0x20], xmm2
        movdqa xmmword ptr [edi + 0x30], xmm3
        movdqa xmm4, xmmword ptr [esi + 0x40]
        movdqa xmm5, xmmword ptr [esi + 0x50]
        movdqa xmm6, xmmword ptr [esi + 0x60]
        movdqa xmm7, xmmword ptr [esi + 0x70]
        movdqa xmmword ptr [edi + 0x40], xmm4
        movdqa xmmword ptr [edi + 0x50], xmm5
        movdqa xmmword ptr [edi + 0x60], xmm6
        movdqa xmmword ptr [edi + 0x70], xmm7
        lea esi, [esi + 0x80]
        lea edi, [edi + 0x80]
        dec ecx
        jne chunk_00c0c7c4
        mov esi, dword ptr [ebp - 8]
        mov edi, dword ptr [ebp - 4]
        mov esp, ebp
        pop ebp
        ret
    }
}

// Complete native entry 00C0C82B; exact stack/register contract in header.
__declspec(naked) void* __cdecl copy_native_crt_vector_00c0c82b(
    void*, const void*, std::uint32_t) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x1c
        mov dword ptr [ebp - 0xc], edi
        mov dword ptr [ebp - 8], esi
        mov dword ptr [ebp - 4], ebx
        mov ebx, dword ptr [ebp + 0xc]
        mov eax, ebx
        cdq
        mov ecx, eax
        mov eax, dword ptr [ebp + 8]
        xor ecx, edx
        sub ecx, edx
        and ecx, 0xf
        xor ecx, edx
        sub ecx, edx
        cdq
        mov edi, eax
        xor edi, edx
        sub edi, edx
        and edi, 0xf
        xor edi, edx
        sub edi, edx
        mov edx, ecx
        or edx, edi
        jne dispatch_00c0c8ae
        mov esi, dword ptr [ebp + 0x10]
        mov ecx, esi
        and ecx, 0x7f
        mov dword ptr [ebp - 0x18], ecx
        cmp esi, ecx
        je dispatch_00c0c886
        sub esi, ecx
        push esi
        push ebx
        push eax
        call copy_native_crt_aligned_128_chunks_00c0c7a4
        add esp, 0xc
        mov eax, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 0x18]
    dispatch_00c0c886:
        test ecx, ecx
        je dispatch_00c0c901
        mov ebx, dword ptr [ebp + 0x10]
        mov edx, dword ptr [ebp + 0xc]
        add edx, ebx
        sub edx, ecx
        mov dword ptr [ebp - 0x14], edx
        add ebx, eax
        sub ebx, ecx
        mov dword ptr [ebp - 0x10], ebx
        mov esi, dword ptr [ebp - 0x14]
        mov edi, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 0x18]
        rep movsb
        mov eax, dword ptr [ebp + 8]
        jmp dispatch_00c0c901
    dispatch_00c0c8ae:
        cmp ecx, edi
        jne dispatch_00c0c8e7
        neg ecx
        add ecx, 0x10
        mov dword ptr [ebp - 0x1c], ecx
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 0x1c]
        rep movsb
        mov ecx, dword ptr [ebp + 8]
        add ecx, dword ptr [ebp - 0x1c]
        mov edx, dword ptr [ebp + 0xc]
        add edx, dword ptr [ebp - 0x1c]
        mov eax, dword ptr [ebp + 0x10]
        sub eax, dword ptr [ebp - 0x1c]
        push eax
        push edx
        push ecx
        call copy_native_crt_vector_00c0c82b
        add esp, 0xc
        mov eax, dword ptr [ebp + 8]
        jmp dispatch_00c0c901
    dispatch_00c0c8e7:
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp + 0x10]
        mov edx, ecx
        shr ecx, 2
        rep movsd
        mov ecx, edx
        and ecx, 3
        rep movsb
        mov eax, dword ptr [ebp + 8]
    dispatch_00c0c901:
        mov ebx, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 8]
        mov edi, dword ptr [ebp - 0xc]
        mov esp, ebp
        pop ebp
        ret
    }
}

} // namespace bsp
