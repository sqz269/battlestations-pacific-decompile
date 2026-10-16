#include "bsp/native_zlib_raw_adler32_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Adler32 body requires MSVC Win32.
#endif

// 00BCD720 adler32: preserve null-before-length, conditional EBP frame,
// sixteen individual byte reads, actual saved-cell aliases and RET4.
extern "C" __declspec(naked) std::uint32_t __fastcall bsp_native_zlib_raw_adler32_00bcd720(
    std::uint32_t, const void*, std::uint32_t) noexcept {
    __asm {
        push esi
        push edi
        mov edi, edx
        movzx esi, cx
        shr ecx, 010h
        test edi, edi
        jne nonnull_bytes
        pop edi
        lea eax, [edx + 001h]
        pop esi
        ret 004h
    nonnull_bytes:
        push ebx
        mov ebx, dword ptr [esp + 010h]
        test ebx, ebx
        jbe combine_halves
        push ebp
    next_chunk:
        cmp ebx, 015b0h
        mov eax, ebx
        jb chunk_ready
        mov eax, 015b0h
    chunk_ready:
        sub ebx, eax
        cmp eax, 010h
        jl scalar_tail
        mov edx, eax
        shr edx, 004h
        mov ebp, edx
        neg ebp
        shl ebp, 004h
        add eax, ebp
        // Native four-byte LEA ESP,[ESP+0], preserving flags and frame.
        _emit 08dh
        _emit 064h
        _emit 024h
        _emit 000h
    sixteen_bytes:
        movzx ebp, byte ptr [edi]
        add esi, ebp
        movzx ebp, byte ptr [edi + 001h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 002h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 003h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 004h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 005h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 006h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 007h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 008h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 009h]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00ah]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00bh]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00ch]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00dh]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00eh]
        add ecx, esi
        add esi, ebp
        movzx ebp, byte ptr [edi + 00fh]
        add ecx, esi
        add esi, ebp
        add ecx, esi
        add edi, 010h
        sub edx, 001h
        jne sixteen_bytes
    scalar_tail:
        test eax, eax
        je reduce_halves
        nop
    one_byte:
        movzx edx, byte ptr [edi]
        add esi, edx
        add edi, 001h
        add ecx, esi
        sub eax, 001h
        jne one_byte
    reduce_halves:
        mov eax, 080078071h
        mul esi
        shr edx, 00fh
        imul edx, edx, -0fff1h
        add esi, edx
        mov eax, 080078071h
        mul ecx
        shr edx, 00fh
        imul edx, edx, -0fff1h
        add ecx, edx
        test ebx, ebx
        ja next_chunk
        pop ebp
    combine_halves:
        mov eax, ecx
        pop ebx
        shl eax, 010h
        pop edi
        or eax, esi
        pop esi
        ret 004h
    }
}
