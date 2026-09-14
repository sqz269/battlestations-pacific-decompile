#include "bsp/native_crt_local_unwind_adapters.hpp"
#include "bsp/native_crt_seh4_nested_handler.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT local unwind adapters require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// C0DC9A: descriptive source name; the original library symbol is unknown.
__declspec(naked) void __stdcall unwind_native_crt_context_00c0dc9a(
    const void*) {
    __asm {
        push ebp                                      // 00c0dc9a
        mov ecx, dword ptr [esp + 8]                  // 00c0dc9b
        mov ebp, dword ptr [ecx]                      // 00c0dc9f
        push dword ptr [ecx + 1ch]                    // 00c0dca1
        push dword ptr [ecx + 18h]                    // 00c0dca4
        push dword ptr [ecx + 28h]                    // 00c0dca7
        call unwind_native_crt_local_scopes_00c0dbc4   // 00c0dcaa
        add esp, 0ch                                 // 00c0dcaf
        pop ebp                                      // 00c0dcb2
        ret 4                                        // 00c0dcb3
    }
}

// C0DD00: complete _EH4_LocalUnwind; the following function starts C0DD17.
__declspec(naked) void __fastcall unwind_native_crt_eh4_local_00c0dd00(
    void*, std::uint32_t, void*, const volatile std::uint32_t*) {
    __asm {
        push ebp                                      // 00c0dd00
        mov ebp, dword ptr [esp + 8]                  // 00c0dd01
        push edx                                      // 00c0dd05
        push ecx                                      // 00c0dd06
        push dword ptr [esp + 14h]                    // 00c0dd07
        call unwind_native_crt_local_scopes_00c0dbc4   // 00c0dd0b
        add esp, 0ch                                 // 00c0dd10
        pop ebp                                      // 00c0dd13
        ret 8                                        // 00c0dd14
    }
}
} // namespace bsp
