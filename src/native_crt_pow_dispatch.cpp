#include "bsp/native_crt_pow_dispatch.hpp"

#include "bsp/native_crt_pow_fallback.hpp"
#include "bsp/native_crt_sse2_pow.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pow dispatch requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeCrtPowDispatchContext) == 12);
static_assert(offsetof(NativeCrtPowDispatchContext, dispatch_0109eea0) == 0);
static_assert(offsetof(NativeCrtPowDispatchContext, fallback) == 4);
static_assert(offsetof(NativeCrtPowDispatchContext, sse2) == 8);
static_assert(offsetof(NativeCrtPowFallbackContext, dispatch_0109dd78) == 0);
static_assert(offsetof(NativeCrtPowFallbackContext, status_literals_00d6a684) == 4);
static_assert(offsetof(NativeCrtPowFallbackContext, special_half_00d7a280) == 8);
static_assert(offsetof(NativeCrtPowFallbackContext, literal_anchor_00e165a0) == 12);

// The original tail transfer now forwards the actual selected source context.
// MOV/PUSH/LEA introduce no extra floating-point work or flag normalization.
__declspec(naked) void __cdecl enter_complete_sse2_pow() {
    __asm {
        mov edx, dword ptr [esp + 4]
        push dword ptr [edx + 8]
        call native_crt_sse2_pow_x87_00c19260
        lea esp, [esp + 4]
        ret
    }
}
} // namespace

__declspec(naked) void __cdecl dispatch_native_crt_pow_00bfeb10(
    const NativeCrtPowDispatchContext&) {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov edx, dword ptr [edx]
        cmp dword ptr [edx], 0           // BFEB10: actual current109EEA0
        jz fallback_path                // BFEB17
        sub esp, 8                      // BFEB19
        stmxcsr dword ptr [esp + 4]      // BFEB1C
        mov eax, dword ptr [esp + 4]     // BFEB21
        and eax, 1f80h                  // BFEB25
        cmp eax, 1f80h                  // BFEB2A
        jnz restore_probe_frame         // BFEB2F
        fnstcw word ptr [esp]            // BFEB31
        mov ax, word ptr [esp]           // BFEB34
        and ax, 7fh                     // BFEB38
        cmp ax, 7fh                     // BFEB3C
    restore_probe_frame:
        lea esp, [esp + 8]              // BFEB40: preserve lastCMP flags
        jnz fallback_path               // BFEB44
        jmp enter_complete_sse2_pow      // BFEB46
    fallback_path:
        // Only this route reads fallback bindings. Keep original argument
        // generation below these saves so the complete provider retains its
        // exact x/y/scratch frame and EAX/ST0 input contract.
        push ebp
        push ebx
        push esi
        push edi
        mov edx, dword ptr [esp + 14h]   // original added context at entryESP+4
        mov edx, dword ptr [edx + 4]
        mov ebp, dword ptr [edx]
        mov ebx, dword ptr [edx + 4]
        mov esi, dword ptr [edx + 8]
        mov edi, dword ptr [edx + 0ch]
        sub esp, 14h                    // BFEB4B
        fxch st(1)                      // BFEB4E
        fstp qword ptr [esp]             // BFEB50: x
        fst qword ptr [esp + 8]          // BFEB53: y, retained in ST0
        mov eax, dword ptr [esp + 0ch]   // BFEB57: current stored y high word
        call evaluate_native_crt_pow_fallback_00bfeb6d // BFEB5B
        add esp, 14h                    // BFEB60
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret                             // BFEB63
    }
}
} // namespace bsp
