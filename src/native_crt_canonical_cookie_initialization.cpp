#include "bsp/native_crt_canonical_cookie_initialization.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT canonical cookie initialization requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(FILETIME) == 8 && sizeof(LARGE_INTEGER) == 8);

// Complete C1815E[148]. _emit preserves the native absolute-address encoding
// without MSVC's additional DS prefix; these operands access the actual words.
// All five calls are genuine dllimport symbols, with normal COFF relocations.
__declspec(naked) void __cdecl initialize_native_crt_canonical_security_cookie_00c1815e() {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 10h
        // 00C18164: MOV EAX,[00E15590].
        _emit 0xa1
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        and dword ptr [ebp - 8], 0
        and dword ptr [ebp - 4], 0
        push ebx
        push edi
        mov edi, 0bb40e64eh
        cmp eax, edi
        mov ebx, 0ffff0000h
        je reseed
        // 00C18181: TEST EBX,EAX; retain native operand encoding.
        _emit 0x85
        _emit 0xc3
        je reseed
        not eax
        // 00C18187: MOV [00E15594],EAX.
        _emit 0xa3
        _emit 0x94
        _emit 0x55
        _emit 0xe1
        _emit 0x00
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
        // Native QPC output receives no initialization stores; BOOL is ignored.
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
        // 00C181D4: TEST EBX,ESI; retain native operand encoding.
        _emit 0x85
        _emit 0xf3
        jne publish
        mov eax, esi
        shl eax, 10h
        or esi, eax
    publish:
        // 00C181DF: MOV [00E15590],ESI.
        _emit 0x89
        _emit 0x35
        _emit 0x90
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        not esi
        // 00C181E7: MOV [00E15594],ESI.
        _emit 0x89
        _emit 0x35
        _emit 0x94
        _emit 0x55
        _emit 0xe1
        _emit 0x00
        pop esi
    done:
        pop edi
        pop ebx
        leave
        ret
    }
}
} // namespace bsp
