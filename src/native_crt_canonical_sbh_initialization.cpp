#include "bsp/native_crt_canonical_sbh_initialization.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native canonical CRT small-block heap initialization requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Complete original ___sbh_heap_init schedule, including memory read/modify/write
// effects and the delayed incoming-threshold read after HeapAlloc succeeds.
__declspec(naked) std::uint32_t __cdecl
initialize_native_crt_canonical_sbh_00c11cf5(std::uint32_t) {
    __asm {
        push 0140h
        push 0
        // C11CFC: PUSH DWORD PTR [0109E1BC]; actual current heap word.
        _emit 0xff
        _emit 0x35
        _emit 0xbc
        _emit 0xe1
        _emit 0x09
        _emit 0x01
        call HeapAlloc
        test eax, eax
        // C11D0A: mov dword ptr [0109ed68h], eax; exact absolute encoding, no DS prefix.
        _emit 0xa3
        _emit 0x68
        _emit 0xed
        _emit 0x09
        _emit 0x01
        jnz allocation_succeeded
        ret
    allocation_succeeded:
        mov ecx, dword ptr [esp + 4]
        // C11D16: and dword ptr [0109e310h], 0; exact absolute encoding, no DS prefix.
        _emit 0x83
        _emit 0x25
        _emit 0x10
        _emit 0xe3
        _emit 0x09
        _emit 0x01
        _emit 0x00
        // C11D1D: and dword ptr [0109ed64h], 0; exact absolute encoding, no DS prefix.
        _emit 0x83
        _emit 0x25
        _emit 0x64
        _emit 0xed
        _emit 0x09
        _emit 0x01
        _emit 0x00
        // C11D24: mov dword ptr [0109ed70h], eax; exact absolute encoding, no DS prefix.
        _emit 0xa3
        _emit 0x70
        _emit 0xed
        _emit 0x09
        _emit 0x01
        xor eax, eax
        // C11D2B: mov dword ptr [0109ed6ch], ecx; exact absolute encoding, no DS prefix.
        _emit 0x89
        _emit 0x0d
        _emit 0x6c
        _emit 0xed
        _emit 0x09
        _emit 0x01
        // C11D31: mov dword ptr [0109ed74h], 010h; exact absolute encoding, no DS prefix.
        _emit 0xc7
        _emit 0x05
        _emit 0x74
        _emit 0xed
        _emit 0x09
        _emit 0x01
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        inc eax
        ret
    }
}
} // namespace bsp
