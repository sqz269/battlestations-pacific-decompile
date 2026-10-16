#include "bsp/native_zlib_raw_scan_tree_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw register/frame transfer requires MSVC Win32.
#endif

namespace {
// Complete 00BCE180..00BCE263 scan_tree body. EAX=raw writable tree,
// ECX=signed max_code bits, actual [entry ESP+4]=backing pointer. The stdcall
// spelling describes only one real stack word/RET4, not the custom registers.
// Ordinary C++ must not call this private body. No locals/frame copies exist.
__declspec(naked) void __stdcall scan_tree_body(void*) noexcept {
    __asm {
        sub esp, 8
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 014h]
        push esi
        push edi
        movzx edi, word ptr [eax + 2]
        xor edx, edx
        test edi, edi
        mov ebx, ecx
        mov dword ptr [esp + 010h], 0ffffffffh
        lea ecx, [edx + 7]
        lea esi, [edx + 4]
        jnz initial_thresholds_ready
        mov ecx, 08ah
        lea esi, [edx + 3]
    initial_thresholds_ready:
        test ebx, ebx
        mov word ptr [eax + ebx * 4 + 6], 0ffffh
        jl restore_frame
        add eax, 6
        add ebx, 1
        mov dword ptr [esp + 014h], ebx
        mov dword ptr [esp + 01ch], eax
        mov ebx, 1
        nop
    scan_next_length:
        mov eax, edi
        mov edi, dword ptr [esp + 01ch]
        movzx edi, word ptr [edi]
        add edx, ebx
        cmp edx, ecx
        jge flush_run
        cmp eax, edi
        jz advance_cursor
    flush_run:
        cmp edx, esi
        jge encode_repeated_length
        add word ptr [ebp + eax * 4 + 0a74h], dx
        jmp thresholds_after_flush
    encode_repeated_length:
        test eax, eax
        jz encode_zero_run
        cmp eax, dword ptr [esp + 010h]
        jz add_repeat_frequency
        add word ptr [ebp + eax * 4 + 0a74h], bx
    add_repeat_frequency:
        add word ptr [ebp + 0ab4h], bx
        jmp thresholds_after_flush
    encode_zero_run:
        cmp edx, 0ah
        jg add_long_zero_frequency
        add word ptr [ebp + 0ab8h], bx
        jmp thresholds_after_flush
    add_long_zero_frequency:
        add word ptr [ebp + 0abch], bx
    thresholds_after_flush:
        xor edx, edx
        test edi, edi
        mov dword ptr [esp + 010h], eax
        jnz nonzero_next_length
        mov ecx, 08ah
        lea esi, [edx + 3]
        jmp advance_cursor
    nonzero_next_length:
        cmp eax, edi
        jnz differing_next_length
        mov ecx, 6
        lea esi, [ecx - 3]
        jmp advance_cursor
    differing_next_length:
        mov ecx, 7
        lea esi, [ecx - 3]
    advance_cursor:
        add dword ptr [esp + 01ch], 4
        sub dword ptr [esp + 014h], ebx
        jnz scan_next_length
    restore_frame:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 4
    }
}
}

// The Source caller supplies its actual chosen frame. No fourth backing input,
// continuation snapshot, saved ESP global or ordinary returning shim is added.
extern "C" __declspec(naked) __declspec(noreturn) void __cdecl
bsp_native_zlib_scan_tree_frame_00bce180(void*, std::int32_t, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        mov esp, dword ptr [esp + 00ch]
        jmp scan_tree_body
    }
}
