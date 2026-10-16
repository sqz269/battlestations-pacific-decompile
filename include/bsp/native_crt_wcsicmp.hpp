#pragma once

#include <cstdint>

namespace bsp {

// The SAME borrowed locale view declared by native_crt_wcsicmp_locale.hpp.
// This parent only forwards null; it does not construct or inspect a view.
struct NativeCrtWcsicmpLocaleView;

// Required reached callee services, not another CRT/TLS/errno/locale owner.
// The error cell must be the owning CRT's real current errno address. Keep
// the context and all reached providers alive throughout the call. No callback
// defaults, missing-service fallback or SDK CRT substitution is supplied.
struct NativeCrtWcsicmpServices {
    void* context;
    volatile std::int32_t* (*errno_address_00bffb8b)(void*);
    void (*invalid_parameter_00bf66ef)(void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t, std::uint32_t);
    std::int32_t (*compare_locale_00c0392a)(void*, const char16_t*, const char16_t*,
        const NativeCrtWcsicmpLocaleView*);
};

// Whole Original __wcsicmp [00C03A39,00C03ACA),145B/56 instructions.
// Read the borrowed real DWORD0109DE1C first. Nonzero forwards exact pointers
// and NULL third locale argument to __wcsicmp_l. Zero checks left then right;
// invalid input calls errno-address, stores0x16(22), then delivers FIVE zero
// DWORD slots to invalid-parameter. If it returns, return INT_MAX.
// Valid strings read/fold unsigned16-bit left then right, A..Z only, advance
// both addresses2, test left NUL before equality, return zero-extended difference.
// Borrow accessible reached UTF-16 units; no Unicode/SDK locale conversion.
// New Source context interface, not Original cdecl(two wchar pointers)/EAX/RET
// ABI, private argument-home/register identity, fault/SEH or complete CRT proof.
// Provider exceptions/nonreturn propagate; the parent owns no cleanup frame.
std::int32_t compare_native_crt_wide_strings_00c03a39(
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtWcsicmpServices&, const char16_t* actual_left,
    const char16_t* actual_right);

} // namespace bsp
