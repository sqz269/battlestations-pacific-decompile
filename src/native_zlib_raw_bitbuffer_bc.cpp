#include "bsp/native_zlib_raw_bitbuffer_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These raw returning register bodies require MSVC Win32.
#endif

// The same-function prefix maps the genuine Source ECX input to observed EAX.
// Every following instruction belongs to complete 00BCEF40..00BCEFB4.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_bi_flush_00bcef40(void*) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 016b4h]
        cmp ecx, 010h
        push ebx
        jnz flush_not_sixteen
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], 1
        movzx ebx, byte ptr [eax + 016b1h]
        mov ecx, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], 1
        xor ecx, ecx
        mov dword ptr [eax + 016b4h], ecx
        mov word ptr [eax + 016b0h], cx
        pop ebx
        ret
    flush_not_sixteen:
        cmp ecx, 8
        jl flush_restore
        mov ecx, dword ptr [eax + 8]
        mov edx, dword ptr [eax + 014h]
        mov bl, byte ptr [eax + 016b0h]
        mov byte ptr [ecx + edx * 1], bl
        movzx cx, byte ptr [eax + 016b1h]
        add dword ptr [eax + 014h], 1
        add dword ptr [eax + 016b4h], -8
        mov word ptr [eax + 016b0h], cx
    flush_restore:
        pop ebx
        ret
    }
}

// Complete 00BCEFC0..00BCF032 follows its own same-function MOV prefix.
// Actual POP ESI precedes WORD-buffer then DWORD-valid resets; POP EBX follows.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_bi_windup_00bcefc0(void*) noexcept {
    __asm {
        mov eax, ecx
        mov edx, dword ptr [eax + 016b4h]
        cmp edx, 8
        push ebx
        push esi
        jle windup_at_most_eight
        movzx ebx, byte ptr [eax + 016b0h]
        mov edx, dword ptr [eax + 014h]
        mov ecx, dword ptr [eax + 8]
        mov byte ptr [ecx + edx * 1], bl
        movzx ebx, byte ptr [eax + 016b1h]
        mov edx, dword ptr [eax + 8]
        mov esi, 1
        add dword ptr [eax + 014h], esi
        mov ecx, dword ptr [eax + 014h]
        mov byte ptr [ecx + edx * 1], bl
        add dword ptr [eax + 014h], esi
        xor ecx, ecx
        pop esi
        mov word ptr [eax + 016b0h], cx
        mov dword ptr [eax + 016b4h], ecx
        pop ebx
        ret
    windup_at_most_eight:
        xor ecx, ecx
        cmp edx, ecx
        jle windup_reset
        mov esi, dword ptr [eax + 014h]
        mov edx, dword ptr [eax + 8]
        mov bl, byte ptr [eax + 016b0h]
        mov byte ptr [edx + esi * 1], bl
        add dword ptr [eax + 014h], 1
    windup_reset:
        pop esi
        mov word ptr [eax + 016b0h], cx
        mov dword ptr [eax + 016b4h], ecx
        pop ebx
        ret
    }
}
