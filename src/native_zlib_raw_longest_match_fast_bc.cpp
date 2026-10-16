#include "bsp/native_zlib_raw_longest_match_fast_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw register/frame transfer requires MSVC Win32.
#endif

namespace {
// Complete 00BCBB40..00BCBC05 longest_match_fast body. ESI=raw backing and
// [entry ESP+4]=actual current_match. The Source stdcall spelling describes
// only its one stack word/RET4 and EAX result, not the custom ESI input.
// Ordinary C++ must not call this private body. All EDX upper bits stay real.
__declspec(naked) std::uint32_t __stdcall
longest_match_fast_body(std::uint32_t) noexcept {
    __asm {
        mov ecx, dword ptr [esi + 030h]
        mov eax, dword ptr [esi + 064h]
        push ebp
        mov ebp, dword ptr [esp + 8]
        mov dl, byte ptr [ecx + ebp * 1]
        add eax, ecx
        add ecx, ebp
        cmp dl, byte ptr [eax]
        push edi
        lea edi, [eax + 0102h]
        jnz fallback_two
        mov dl, byte ptr [ecx + 1]
        cmp dl, byte ptr [eax + 1]
        jnz fallback_two
        add eax, 2
        add ecx, 2
    compare_eight_bytes:
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        mov dl, byte ptr [eax + 1]
        add eax, 1
        add ecx, 1
        cmp dl, byte ptr [ecx]
        jnz compute_length
        cmp eax, edi
        jc compare_eight_bytes
    compute_length:
        sub eax, edi
        add eax, 0102h
        cmp eax, 3
        jl fallback_two
        mov ecx, dword ptr [esi + 06ch]
        cmp eax, ecx
        mov dword ptr [esi + 068h], ebp
        jbe restore_result
        pop edi
        mov eax, ecx
        pop ebp
        ret 4
    fallback_two:
        mov eax, 2
    restore_result:
        pop edi
        pop ebp
        ret 4
    }
}
}

// No duplicate current_match argument, copied frame or ordinary-return shim.
// The chosen frame's current continuation governs the private RET4 transfer.
extern "C" __declspec(naked) __declspec(noreturn) void __cdecl
bsp_native_zlib_longest_match_fast_frame_00bcbb40(void*, void*) noexcept {
    __asm {
        mov esi, dword ptr [esp + 4]
        mov esp, dword ptr [esp + 8]
        jmp longest_match_fast_body
    }
}
