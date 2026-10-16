#pragma once

#include <cstdint>

namespace bsp {

// Required callee services for the whole __stricmp parent at00BF7FBF.
// No current locale, TLS/errno owner, invalid handler or default adapter is
// created here. Keep this provider and all reached storage/call arguments live.
// Numeric suffixes identify Original callees, never callable host addresses.
class NativeCrtStricmpCallees {
public:
    virtual ~NativeCrtStricmpCallees() = default;
    // Called only for a null input on the zero-locale-changed arm. Return the
    // actual writable errno DWORD; no null check or substitute cell is added.
    virtual volatile std::int32_t* errno_cell_00bffb8b() = 0;
    // The parent supplies five DWORD zeros. This interface does not infer the
    // unmatched callee's complete private argument types or handler ownership.
    // It may return, throw or terminate; INT_MAX follows only a normal return.
    virtual void invalid_parameter_00bf66ef(std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t, std::uint32_t) = 0;
    // The nonzero changed-locale arm delegates even null inputs unchanged and
    // passes a null third locale pointer; this callee supplies its real current
    // locale/TLS policy. Never replace it with the ASCII leaf by default.
    virtual std::int32_t compare_locale_00bf7eec(
        const char* left, const char* right, void* actual_locale) = 0;
};

// Whole Original __stricmp [00BF7FBF,00BF800F),80B/32 instructions:
// cdecl(two stacked pointers), EAX signed int, plain RET; restores ESI/EBP
// before the zero-mode valid-input tail JMP to ___ascii_stricmp/00BF7EB7.
// Borrow the live0109DE1C DWORD and sample it once. Zero mode validates left
// then right, otherwise writes errno0x16 (22), invokes five-zero BF66EF and
// returns INT_MAX. Nonzero mode calls BF7EEC(left,right,null locale).
// The new Source API has extra explicit services/cell arguments. It preserves
// reached external-cell/callee effects, not Original entry/register/stack-home
// aliases, fault/SEH ABI, a CRT owner, whole locale callee or GUI activation.
std::int32_t compare_native_crt_strings_00bf7fbf(const char* actual_left,
    const char* actual_right, const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees&);

} // namespace bsp
