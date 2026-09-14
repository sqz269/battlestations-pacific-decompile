#pragma once

#include <cstdint>

namespace bsp {
// Stable borrowed bindings to the two distinct actual words of the same CRT
// cookie domain. The caller supplies their existing values and lifetime; this
// interface creates no canonical state and does not bind a host CRT cookie.
struct NativeCrtCookieInitializationContext {
    volatile std::uint32_t& cookie_00e15590;
    volatile std::uint32_t& complement_00e15594;
};

// Complete C1815E[148] schedule. Native entry has no arguments and plain RET;
// this qualified naked Win32 interface adds one cdecl context argument.
// Current nondefault cookie with nonzero high16 only refreshes its complement.
// Otherwise invoke the five actual Win32 time/identity/counter services, apply
// the original adjustments, and publish cookie before complement. The QPC
// result is ignored and its output slot remains uninitialized before the call.
// Bindings must remain valid/stable, including across those real OS calls.
void __cdecl initialize_native_crt_security_cookie_00c1815e(
    NativeCrtCookieInitializationContext&);

// Full C04EF3[8]: the native stacked reason is ignored; AND the actual word
// with zero and RET. This interface adds a second cdecl reference argument.
// Preserves the original EAX and final AND flags, with an additional EAX save
// and binding load. It does not invoke a debugger/error callback.
void __cdecl clear_native_crt_debugger_hook_00c04ef3(
    std::uint32_t ignored_reason, volatile std::uint32_t& actual_word_0109eea8);

// Added bindings, loads and code addresses are a new source ABI. Original
// incidental volatile registers, exact fault sites and native continuation
// identity are not established. No exception translation, extra randomness,
// pointer encoding, Watson/report/check body or canonical process owner is
// supplied here. Cookie/check/report consumers must share the actual domain.
} // namespace bsp
