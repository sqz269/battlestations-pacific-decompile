#include "bsp/native_crt_x87_control.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT x87 control entries require MSVC Win32 raw assembly.
#endif

namespace bsp {

// Original LIBCRT_unmatched_00c083a5; descriptive source name only.
// Complete CALL entry, including original packed-slot read/write order.
__declspec(naked) void __cdecl prepare_native_crt_x87_control_00c083a5() {
    __asm {
        mov edx, dword ptr [esp + 4] // 00c083a5
        and edx, 300h // 00c083a9
        or edx, 7fh // 00c083af
        mov word ptr [esp + 6], dx // 00c083b2
        fldcw word ptr [esp + 6] // 00c083b7
        ret // 00c083bb
    }
}

// Original LIBCRT_unmatched_00c0842e; not an ordinary callable function ABI.
// Complete CW-top JMP tail. The caller's real return is below the packed word.
__declspec(naked) void __cdecl restore_native_crt_x87_control_tail_00c0842e() {
    __asm {
        cmp word ptr [esp], 27fh // 00c0842e
        jz finished // 00c08434
        fldcw word ptr [esp] // 00c08436
    finished:
        pop edx // 00c08439
        ret // 00c0843a
    }
}

} // namespace bsp
