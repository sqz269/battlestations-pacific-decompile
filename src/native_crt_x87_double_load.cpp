#include "bsp/native_crt_x87_double_load.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT x87 double load requires MSVC Win32 raw assembly.
#endif

namespace bsp {

// Original library symbol: __fload_withFB. Complete 67-byte instruction body,
// including both RETs and all current input reads; no provider substitutions.
// See the header for the assembly-only ST0/EAX/EFLAGS contract and uncertainty.
__declspec(naked) std::uint32_t __fastcall
load_native_crt_double_x87_00c083d5(void*, const void*) {
    __asm {
        mov eax, dword ptr [edx + 4] // 00c083d5
        and eax, 07ff00000h // 00c083d8
        cmp eax, 07ff00000h // 00c083dd
        je exceptional // 00c083e2
        fld qword ptr [edx] // 00c083e4
        ret // 00c083e6
    exceptional:
        mov eax, dword ptr [edx + 4] // 00c083e7
        sub esp, 0ah // 00c083ea
        or eax, 07fff0000h // 00c083ed
        mov dword ptr [esp + 6], eax // 00c083f2
        mov eax, dword ptr [edx + 4] // 00c083f6
        mov ecx, dword ptr [edx] // 00c083f9
        shld eax, ecx, 0bh // 00c083fb
        shl ecx, 0bh // 00c083ff
        mov dword ptr [esp + 4], eax // 00c08402
        mov dword ptr [esp], ecx // 00c08406
        fld tbyte ptr [esp] // 00c08409
        add esp, 0ah // 00c0840c
        test eax, 0 // 00c0840f
        mov eax, dword ptr [edx + 4] // 00c08414
        ret // 00c08417
    }
}

} // namespace bsp
