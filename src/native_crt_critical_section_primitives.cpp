#include "bsp/native_crt_critical_section_primitives.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT critical-section primitives require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 24);

// Native 00C17643..00C17652: preserve the API call and XOR/INC flag schedule.
// No wrapper, SEH policy, or private critical-section storage is introduced.
__declspec(naked) std::int32_t __stdcall
initialize_native_crt_critical_section_without_spin_00c17643(
    void*, std::uint32_t) {
    __asm {
        push dword ptr [esp + 4]
        call InitializeCriticalSection
        xor eax, eax
        inc eax
        ret 8
    }
}

// Native 00C17639..00C17642: load the input before the single DWORD store.
// Borrowing the real word replaces the absolute destination; preserve ECX and
// arithmetic flags while loading that binding. Its address must remain stable
// and must not alias this routine's stack frame or argument storage.
__declspec(naked) std::uint32_t __cdecl
store_native_crt_encoded_critical_section_initializer_00c17639(
    std::uint32_t, volatile std::uint32_t&) {
    __asm {
        push ecx
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [esp + 0ch]
        mov dword ptr [ecx], eax
        pop ecx
        ret
    }
}

} // namespace bsp
