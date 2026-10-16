#include "bsp/native_zlib_raw_copy_block_bc.hpp"
#include "bsp/native_zlib_raw_bitbuffer_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw copy_block register/frame entry requires MSVC Win32.
#endif

namespace {
// Complete 00BCF040..00BCF0C3, with a genuine full-public-entry AD call.
// EAX=backing, ECX=length, EDX=bytes; actual DWORD[entryESP+4]=header/count.
// No ordinary C++ call establishes this private custom register entry.
__declspec(naked) void __cdecl copy_block_body() noexcept {
    __asm {
        push ebx // 00bcf040
        push esi // 00bcf041
        push edi // 00bcf042
        mov ebx, ecx // 00bcf043
        mov esi, edx // 00bcf045
        mov ecx, eax // Source adaptation: genuine AD backing parameter.
        call bsp_native_zlib_bi_windup_00bcefc0 // 00bcf047, whole public entry.
        cmp dword ptr [esp + 010h], 0 // 00bcf04c
        mov dword ptr [eax + 016ach], 8 // 00bcf051
        mov edi, 1 // 00bcf05b
        jz copy_payload_check // 00bcf060
        mov ecx, dword ptr [eax + 014h] // 00bcf062
        mov edx, dword ptr [eax + 8] // 00bcf065
        mov byte ptr [ecx + edx], bl // 00bcf068
        add dword ptr [eax + 014h], edi // 00bcf06b
        mov ecx, dword ptr [eax + 014h] // 00bcf06e
        mov edx, dword ptr [eax + 8] // 00bcf071
        mov byte ptr [ecx + edx], bh // 00bcf074
        add dword ptr [eax + 014h], edi // 00bcf077
        mov edx, dword ptr [eax + 8] // 00bcf07a
        push ebp // 00bcf07d
        mov ebp, dword ptr [eax + 014h] // 00bcf07e
        mov cl, bl // 00bcf081
        not cl // 00bcf083
        mov byte ptr [edx + ebp], cl // 00bcf085
        add dword ptr [eax + 014h], edi // 00bcf088
        mov ebp, dword ptr [eax + 014h] // 00bcf08b
        mov edx, dword ptr [eax + 8] // 00bcf08e
        mov ecx, ebx // 00bcf091
        not ecx // 00bcf093
        mov byte ptr [edx + ebp], ch // 00bcf095
        add dword ptr [eax + 014h], edi // 00bcf098
        pop ebp // 00bcf09b
    copy_payload_check:
        test ebx, ebx // 00bcf09c
        jz copy_done // 00bcf09e
    copy_payload_loop:
        mov ecx, dword ptr [eax + 014h] // 00bcf0a0
        mov edx, dword ptr [eax + 8] // 00bcf0a3
        sub ebx, edi // 00bcf0a6
        mov dword ptr [esp + 010h], ebx // 00bcf0a8
        mov bl, byte ptr [esi] // 00bcf0ac
        mov byte ptr [ecx + edx], bl // 00bcf0ae
        mov ebx, dword ptr [esp + 010h] // 00bcf0b1
        add dword ptr [eax + 014h], edi // 00bcf0b5
        add esi, edi // 00bcf0b8
        test ebx, ebx // 00bcf0ba
        jnz copy_payload_loop // 00bcf0bc
    copy_done:
        pop edi // 00bcf0be
        pop esi // 00bcf0bf
        pop ebx // 00bcf0c0
        ret 4 // 00bcf0c1
    }
}
} // namespace

// Four actual adapter reads; entry ESP is selected only after loading EAX/ECX/EDX.
// The real borrowed frame supplies header/count/continuation; nothing is copied.
extern "C" __declspec(naked) __declspec(noreturn) void __cdecl
bsp_native_zlib_copy_block_frame_00bcf040(
    void*, std::uint32_t, const void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [esp + 00ch]
        mov esp, dword ptr [esp + 010h]
        jmp copy_block_body
    }
}

// Genuine returning Source protocol. T is this entry's actual ESP; consume
// length at T+4 as C's mutable continuation, leaving actual raw_header at C+4.
// The body overwrites the temporary C-8 and old return C-4 saves before its
// first backing read. No return address remains protected through the body.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_copy_block_00bcf040(
    void*, const void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        push ebx
        mov ebx, dword ptr [esp + 4]
        mov dword ptr [esp + 8], ebx
        pop ebx
        add esp, 4
        jmp copy_block_body
    }
}
