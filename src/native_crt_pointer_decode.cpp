#include "bsp/native_crt_pointer_decode.hpp"

#include "bsp/native_crt_pointer_decode_support.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pointer decode requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeCrtPointerDecodeContext) == 16);
const char kernel32_name[13] = "KERNEL32.DLL";
const char decode_pointer_name[14] = "DecodePointer";
} // namespace

__declspec(naked) void* __cdecl native_crt_decode_pointer_00c04fde(
    void*, const NativeCrtPointerDecodeContext&) {
    __asm {
        push esi
        // Entry argument remains at [ESP+8]; added context is [ESP+0Ch].
        mov eax, dword ptr [esp + 0ch]
        mov ecx, dword ptr [eax]
        push dword ptr [ecx]
        // Capture actual IAT only after the first current-index PUSH.
        mov edx, dword ptr [eax + 8]
        mov esi, dword ptr [edx]
        call esi
        test eax, eax
        je module_fallback
        mov eax, dword ptr [esp + 0ch]
        mov eax, dword ptr [eax + 4]
        mov eax, dword ptr [eax]
        cmp eax, -1
        je module_fallback
        push eax
        // Captured PTD index is below the second current TLS-index push.
        mov ecx, dword ptr [esp + 10h]
        mov ecx, dword ptr [ecx]
        push dword ptr [ecx]
        call esi
        call eax
        test eax, eax
        je module_fallback
        mov eax, dword ptr [eax + 1fch]
        jmp selected_decoder
    module_fallback:
        push offset kernel32_name
        call GetModuleHandleA
        mov esi, eax
        test esi, esi
        je return_current_argument
        mov eax, dword ptr [esp + 0ch]
        push dword ptr [eax + 0ch]
        call native_crt_pointer_decode_module_gate_00c04efb
        add esp, 4
        test eax, eax
        je return_current_argument
        push offset decode_pointer_name
        push esi
        call GetProcAddress
    selected_decoder:
        test eax, eax
        je return_current_argument
        push dword ptr [esp + 8]
        call eax
        mov dword ptr [esp + 8], eax
    return_current_argument:
        mov eax, dword ptr [esp + 8]
        pop esi
        ret
    }
}
} // namespace bsp
