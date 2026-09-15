#include "bsp/native_crt_canonical_unlock.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native canonical CRT unlock requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::int32_t) == 4);

// Native __unlock hypothesis, complete 21-byte entry. The descriptor base is
// original fixed storage, not an extra context argument or private global.
__declspec(naked) void __cdecl unlock_native_crt_canonical_00c11b31(std::int32_t) {
    __asm {
        push ebp
        mov ebp, esp
        mov eax, dword ptr [ebp + 8]
        push dword ptr [eax * 8 + 00e16478h]
        call LeaveCriticalSection
        pop ebp
        ret
    }
}

} // namespace bsp
