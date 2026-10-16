#include "bsp/native_zlib_raw_init_block_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw returning register body requires MSVC Win32.
#endif

// One genuine Source input and one register-only transport prefix, followed by
// every operation of 00BCDE20..00BCDE8D. Actual frame/data/control aliases stay
// borrowed memory; no detached state, helper, guard or forced restoration.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_init_block_00bcde20(void*) noexcept {
    __asm {
        mov edx, ecx
        push esi
        lea eax, [edx + 08ch]
        mov ecx, 011eh
        xor esi, esi
        mov edi, edi
    literal_frequency_loop:
        mov word ptr [eax], si
        add eax, 4
        sub ecx, 1
        jnz literal_frequency_loop
        lea eax, [edx + 0980h]
        mov ecx, 01eh
    distance_frequency_loop:
        mov word ptr [eax], si
        add eax, 4
        sub ecx, 1
        jnz distance_frequency_loop
        lea eax, [edx + 0a74h]
        mov ecx, 013h
        lea esp, [esp]
    bitlength_frequency_loop:
        mov word ptr [eax], si
        add eax, 4
        sub ecx, 1
        jnz bitlength_frequency_loop
        mov dword ptr [edx + 016a4h], esi
        mov dword ptr [edx + 016a0h], esi
        mov dword ptr [edx + 016a8h], esi
        mov dword ptr [edx + 01698h], esi
        mov word ptr [edx + 048ch], 1
        pop esi
        ret
    }
}
