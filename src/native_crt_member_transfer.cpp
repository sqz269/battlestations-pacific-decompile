#include "bsp/native_crt_member_transfer.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native member transfer requires MSVC Win32.
#endif

namespace bsp {
// Preserve all seven bytes, including the implicitly locked memory XCHG.
__declspec(naked) void __stdcall call_native_crt_member_function0_00bf6aa6(
    void*, void*) {
    __asm {
        pop eax
        pop ecx
        xchg dword ptr [esp], eax
        jmp eax
    }
}
} // namespace bsp
