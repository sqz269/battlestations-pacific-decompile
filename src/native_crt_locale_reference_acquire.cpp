#include "bsp/native_crt_locale_reference_acquire.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32 inline assembly.
#endif

// Declare the actual KERNEL32 export directly, avoiding SDK intrinsic macros.
// This is the Win32 LONG/volatile LONG*/stdcall ABI, not a substitute callback.
extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement(
    volatile long* addend);

static_assert(sizeof(void*) == 4);
static_assert(sizeof(long) == 4);

namespace bsp {

__declspec(naked) void __cdecl acquire_native_crt_locale_references_00c0190b(
    void*, const void*) {
    __asm {
        push ebx
        push ebp
        push esi
        mov esi, dword ptr [esp + 10h]
        push edi
        mov edi, dword ptr [InterlockedIncrement]
        push esi
        call edi
        mov eax, dword ptr [esi + 0b0h]
        test eax, eax
        jz next_b8
        push eax
        call edi
    next_b8:
        mov eax, dword ptr [esi + 0b8h]
        test eax, eax
        jz next_b4
        push eax
        call edi
    next_b4:
        mov eax, dword ptr [esi + 0b4h]
        test eax, eax
        jz next_c0
        push eax
        call edi
    next_c0:
        mov eax, dword ptr [esi + 0c0h]
        test eax, eax
        jz categories
        push eax
        call edi
    categories:
        push 6
        lea ebx, [esi + 50h]
        pop ebp
    category_loop:
        // Four saved registers leave the borrowed second argument at ESP+18h.
        mov ecx, dword ptr [esp + 18h]
        cmp dword ptr [ebx - 8], ecx
        je wide_category
        mov eax, dword ptr [ebx]
        test eax, eax
        jz wide_category
        push eax
        call edi
    wide_category:
        cmp dword ptr [ebx - 4], 0
        jz next_category
        mov eax, dword ptr [ebx + 4]
        test eax, eax
        jz next_category
        push eax
        call edi
    next_category:
        add ebx, 10h
        dec ebp
        jnz category_loop
        mov eax, dword ptr [esi + 0d4h]
        add eax, 0b4h
        push eax
        call edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

} // namespace bsp
