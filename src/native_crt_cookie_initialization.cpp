#include "bsp/native_crt_cookie_initialization.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT cookie initialization requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeCrtCookieInitializationContext) == 8);
static_assert(sizeof(FILETIME) == 8 && sizeof(LARGE_INTEGER) == 8);

__declspec(naked) void __cdecl initialize_native_crt_security_cookie_00c1815e(
    NativeCrtCookieInitializationContext&) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 10h
        // Added context -> stable cookie binding -> current actual word.
        mov eax, dword ptr [ebp + 8]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax]
        and dword ptr [ebp - 8], 0
        and dword ptr [ebp - 4], 0
        push ebx
        push edi
        mov edi, 0bb40e64eh
        cmp eax, edi
        mov ebx, 0ffff0000h
        je reseed
        test ebx, eax
        je reseed
        not eax
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 4]
        mov dword ptr [ecx], eax
        jmp done
    reseed:
        push esi
        lea eax, [ebp - 8]
        push eax
        call GetSystemTimeAsFileTime
        mov esi, dword ptr [ebp - 4]
        xor esi, dword ptr [ebp - 8]
        call GetCurrentProcessId
        xor esi, eax
        call GetCurrentThreadId
        xor esi, eax
        call GetTickCount
        xor esi, eax
        // The lower eight local bytes receive no initialization store. Keep
        // the native ignored BOOL and raw output loads even if QPC fails.
        lea eax, [ebp - 10h]
        push eax
        call QueryPerformanceCounter
        mov eax, dword ptr [ebp - 0ch]
        xor eax, dword ptr [ebp - 10h]
        xor esi, eax
        cmp esi, edi
        jne high_word_check
        mov esi, 0bb40e64fh
        jmp publish
    high_word_check:
        test ebx, esi
        jne publish
        mov eax, esi
        shl eax, 10h
        or esi, eax
    publish:
        mov ecx, dword ptr [ebp + 8]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx], esi
        not esi
        mov ecx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ecx + 4]
        mov dword ptr [ecx], esi
        pop esi
    done:
        pop edi
        pop ebx
        leave
        ret
    }
}

__declspec(naked) void __cdecl clear_native_crt_debugger_hook_00c04ef3(
    std::uint32_t, volatile std::uint32_t&) {
    __asm {
        push eax
        mov eax, dword ptr [esp + 0ch]
        and dword ptr [eax], 0
        pop eax
        ret
    }
}
} // namespace bsp
