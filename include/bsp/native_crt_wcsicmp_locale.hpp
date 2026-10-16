#pragma once

#include <cstdint>

namespace bsp {

// Borrowed first two DWORDs of the Original localeinfo_struct/_locale_t.
// These are actual CRT identities, not locale names, copied tables or a host
// CRT locale. Both records and all transitively reached storage must stay live.
// This is a standard-layout Source projection. Callback adapters must honor
// its C++ object/alias domain; no cast-safety to unrelated CRT C++ types follows.
struct NativeCrtWcsicmpLocaleView {
    const void* locinfo_00;
    const void* mbcinfo_04;
};

// Original _LocaleUpdate stack object: BF7E24 writes acquired_0c first. An
// explicit locale writes only locale_00; ptd_08 and padding remain untouched.
// Null-locale construction obtains the real PTD and may acquire its +70 bit1.
struct NativeCrtWcsicmpLocaleUpdate {
    NativeCrtWcsicmpLocaleView locale_00;
    void* ptd_08;
    std::uint8_t acquired_0c;
    std::uint8_t untouched_0d[3];
};

// Required borrowed callee services. There are no defaults or owned CRT cells.
// A producer must supply the actual BF7E24, BFFB8B, BF66EF and BFD39F contracts
// over one live locale/PTD/errno/handler domain. The added context arguments
// are ordinary Source plumbing, not the Original call ABIs.
struct NativeCrtWcsicmpLocaleServices {
    void* context;
    void (*locale_update_00bf7e24)(void*, NativeCrtWcsicmpLocaleUpdate&,
        const NativeCrtWcsicmpLocaleView* requested_locale);
    volatile std::int32_t* (*errno_address_00bffb8b)(void*);
    // Five Original DWORD stack slots; every argument is zero on this path.
    void (*invalid_parameter_00bf66ef)(void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t, std::uint32_t);
    // Input is the zero-extended WORD. Only the returned EAX low WORD is used.
    // The exact same local locale view is passed for left and right calls.
    std::uint32_t (*towlower_l_00bfd39f)(void*, std::uint32_t,
        NativeCrtWcsicmpLocaleView* same_local_locale);
};

// Whole Original __wcsicmp_l [00C0392A,00C03A39), 271 bytes/105 instructions.
// Original cdecl ABI is THREE stack arguments (left, right, _locale_t), RET/EAX
// int. This Source entry adds services and is not an entry-point replacement.
// The GUI's two-argument callback projects __wcsicmp C03A39, whose changed-
// locale arm explicitly calls this library function with locale == nullptr.
//
// Construct the locale update before checking either pointer. A null input
// writes errno 0x16 (22), calls the handler with five zeros, then returns
// INT_MAX if the handler returns. Otherwise select ASCII A..Z folding from
// DWORD[locinfo+14] == 0, or make left/right towlower calls over the same local
// view. Read unsigned WORDs, advance both Win32 cursors by two, stop on folded
// left zero or inequality, and return the full signed WORD difference.
// On each normal return, acquired_0c != 0 clears only bit1 of DWORD[ptd+70].
//
// Callbacks and reached raw storage are required and borrowed. Valid aligned
// reads/writes and normal-return callee behavior delimit the proven domain.
// No synchronization, input guard, TLS/locale owner, exception translation or
// unwind cleanup is added. Throwing callbacks, native faults/SEH/nonlocal
// transfer, private stack aliases and exact register/code identity are outside
// this ordinary C++ interface's equivalence claim; a nonreturning handler never
// reaches the normal cleanup. No game or complete CRT-subtree claim is made.
std::int32_t compare_native_crt_wide_strings_l_00c0392a(
    NativeCrtWcsicmpLocaleServices& services, const char16_t* actual_left,
    const char16_t* actual_right,
    const NativeCrtWcsicmpLocaleView* requested_locale);

} // namespace bsp
