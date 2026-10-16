#include "bsp/native_zlib_raw_tr_align_bc.hpp"
#include "bsp/native_zlib_raw_bitbuffer_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw returning register body requires MSVC Win32.
#endif

// Complete 00BCF530..00BCF71A schedule with three genuine AD public bindings.
// All actual save/control/data aliases remain real. MOV ECX,EAX before each
// CALL transports backing, and the current verified flush body leaves EAX
// physically unchanged on normal return. There is no logical return coercion.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_tr_align_00bcf530(void*) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 016b4h]
        mov edx, 2
        shl edx, cl
        push ebx
        push esi
        push edi
        or word ptr [eax + 016b0h], dx
        cmp ecx, 0dh
        mov esi, 1
        jle first_header_short
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], esi
        movzx ebx, byte ptr [eax + 016b1h]
        mov ecx, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        mov edx, dword ptr [eax + 016b4h]
        add dword ptr [eax + 014h], esi
        mov cl, 010h
        sub cl, dl
        mov edi, 2
        shr di, cl
        add edx, -0dh
        mov dword ptr [eax + 016b4h], edx
        mov word ptr [eax + 016b0h], di
        jmp first_send_eob
    first_header_short:
        add ecx, 3
        mov dword ptr [eax + 016b4h], ecx
    first_send_eob:
        mov ecx, dword ptr [eax + 016b4h]
        xor edx, edx
        shl edx, cl
        or word ptr [eax + 016b0h], dx
        cmp ecx, 9
        jle first_eob_short
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], esi
        movzx ebx, byte ptr [eax + 016b1h]
        mov ecx, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        mov edx, dword ptr [eax + 016b4h]
        add dword ptr [eax + 014h], esi
        mov cl, 010h
        sub cl, dl
        xor edi, edi
        shr di, cl
        add edx, -9
        mov dword ptr [eax + 016b4h], edx
        mov word ptr [eax + 016b0h], di
        jmp flush_first
    first_eob_short:
        add ecx, 7
        mov dword ptr [eax + 016b4h], ecx
    flush_first:
        mov ecx, eax
        call bsp_native_zlib_bi_flush_00bcef40
        mov ecx, dword ptr [eax + 016b4h]
        mov edx, dword ptr [eax + 016ach]
        sub edx, ecx
        add edx, 0bh
        cmp edx, 9
        jge restore_after_flush
        mov edx, 2
        shl edx, cl
        or word ptr [eax + 016b0h], dx
        cmp ecx, 0dh
        jle second_header_short
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], esi
        movzx ebx, byte ptr [eax + 016b1h]
        mov ecx, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        mov edx, dword ptr [eax + 016b4h]
        add dword ptr [eax + 014h], esi
        mov cl, 010h
        sub cl, dl
        mov edi, 2
        shr di, cl
        add edx, -0dh
        mov dword ptr [eax + 016b4h], edx
        mov word ptr [eax + 016b0h], di
        jmp second_send_eob
    second_header_short:
        add ecx, 3
        mov dword ptr [eax + 016b4h], ecx
    second_send_eob:
        mov ecx, dword ptr [eax + 016b4h]
        xor edx, edx
        shl edx, cl
        or word ptr [eax + 016b0h], dx
        cmp ecx, 9
        jle second_eob_short
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], esi
        movzx ebx, byte ptr [eax + 016b1h]
        mov ecx, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        mov edx, dword ptr [eax + 016b4h]
        add dword ptr [eax + 014h], esi
        mov cl, 010h
        sub cl, dl
        xor esi, esi
        shr si, cl
        add edx, -9
        mov dword ptr [eax + 016b4h], edx
        mov word ptr [eax + 016b0h], si
        mov ecx, eax
        call bsp_native_zlib_bi_flush_00bcef40
        pop edi
        pop esi
        mov dword ptr [eax + 016ach], 7
        pop ebx
        ret
    second_eob_short:
        add ecx, 7
        mov dword ptr [eax + 016b4h], ecx
        mov ecx, eax
        call bsp_native_zlib_bi_flush_00bcef40
    restore_after_flush:
        pop edi
        pop esi
        mov dword ptr [eax + 016ach], 7
        pop ebx
        ret
    }
}
