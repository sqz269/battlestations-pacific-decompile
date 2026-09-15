#include "bsp/native_crt_eh3_filter.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native EH3 validation filter requires MSVC Win32.
#endif

namespace bsp {
// 00C16B4C..00C16B5F: retain inherited EBP and the complete native RET.
__declspec(naked) std::int32_t __cdecl
filter_native_crt_eh3_access_violation_00c16b4c() {
    __asm {
        mov ecx, dword ptr [ebp - 014h]
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [edx]
        xor ecx, ecx
        cmp eax, 0c0000005h
        sete cl
        mov eax, ecx
        ret
    }
}
} // namespace bsp
