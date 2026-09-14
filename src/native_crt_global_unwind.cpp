#include "bsp/native_crt_global_unwind.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT global unwind requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Native C2F25C: preserve a separate actual Windows import tail thunk.
__declspec(naked) void __stdcall rtl_unwind_import_00c2f25c(
    void*, void*, _EXCEPTION_RECORD*, void*) {
    __asm {
        jmp RtlUnwind
    }
}

// Native C0DCE6: retain the direct call through the real import thunk and the
// actual code-address argument. TargetIp is our own restoration label; it is
// never the original PE address and never a fabricated host callback.
__declspec(naked) void __fastcall global_unwind_native_crt_frame_00c0dce6(
    void*) {
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push 0
        push 0
        push offset restore_unwind_registers
        push ecx
        call rtl_unwind_import_00c2f25c
    restore_unwind_registers:
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}

} // namespace bsp
