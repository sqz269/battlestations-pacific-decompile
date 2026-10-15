#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer string vector fill requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6FA0 ordinary fill and006A6FFB completed-prefix catch effects.
// Native ECX destination, EDX unsigned count; stack(value ADDRESS, owner,
// dead tag3, dead tag4), RET10h. Source retains those words and adds actual
// context (RET14h). No semantic native return: zero count leaves entry FS:[0]
// in EAX, a nonnull last construction leaves its EAX, and null skips preserve
// prior residue. Source deliberately exposes void, not this private residue.
// Each nonnull slot directly constructs an actual16B outer owning header
// through0055B150. The fixed value address is reused; its header is not copied
// ahead of time. Then decrement count, advance cursor by16 with DWORD wrap,
// and publish completed end. Null slots skip construction but join the prefix.
// The catch tidies only that prefix, without a null check, and rethrows. The
// failing current child has its own cleanup; no outer range allocation is freed.
void __fastcall fill_native_outer_string_vector_uninitialized_n_006a6fa0(
    void* destination, std::uint32_t count, const void* value,
    std::uint32_t unused_owner, std::uint32_t unused_tag3,
    std::uint32_t unused_tag4, NativeStringRawPoolContext& strings);

// Complete006A7500 dispatcher. Native ECX owner metadata, incoming EDX unused,
// stack(destination,count,value ADDRESS), RET0Ch. Source adds stacked context
// (RET10h). Forward tags are value address bits and (owner & FFFFFF00h).
// Return is native DWORD destination + (original count <<4), after the helper,
// including wrap; it is not the helper's ambient EAX. One native caller uses it.
void* __fastcall fill_native_outer_string_vector_dispatch_006a7500(
    std::uint32_t unused_owner, std::uint32_t unused_edx,
    void* destination, std::uint32_t count, const void* value,
    NativeStringRawPoolContext& strings);

// Admitted actual storage/child ownership/pool/CRT contracts apply. Every
// accessed header and every completed-prefix cleanup access must be valid;
// a skipped null slot is not removed from a later cleanup range. Preserve +0
// opaque words through the actual children; no new range/null/self/alias guard.
// Aliases between value and slots retain child rereads and ownership effects.
// Exclude aliases to native/Source private frames. Source C++ exceptions,
// explicit context, current CRT and proven no-op placement-unwind elision do
// not reproduce native FH3/SEH/RTTI, fault, register or binary ABI identity.
// No native execution or gameplay claim.
} // namespace bsp
