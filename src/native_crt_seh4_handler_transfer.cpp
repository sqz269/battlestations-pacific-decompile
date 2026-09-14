#include "bsp/native_crt_seh4_handler_transfer.hpp"
#include "bsp/native_crt_nlg_notify.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SEH4 handler transfer requires MSVC Win32.
#endif

namespace bsp {
// Naked original register entry; the real caller arrives by JMP. Do not add
// a normal prolog/epilog, frame owner, callback call or no-return annotation.
__declspec(naked) void __fastcall transfer_native_crt_seh4_handler_00c0dccd(
    void*, void*) {
    __asm {
        mov ebp, edx                            // 00c0dccd
        mov esi, ecx                            // 00c0dccf
        mov eax, ecx                            // 00c0dcd1
        push 1                                  // 00c0dcd3
        call notify_native_crt_nlg_00c16879       // 00c0dcd5
        xor eax, eax                            // 00c0dcda
        xor ebx, ebx                            // 00c0dcdc
        xor ecx, ecx                            // 00c0dcde
        xor edx, edx                            // 00c0dce0
        xor edi, edi                            // 00c0dce2
        jmp esi                                 // 00c0dce4
    }
}
} // namespace bsp
