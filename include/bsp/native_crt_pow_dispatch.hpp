#pragma once

#include <cstdint>

namespace bsp {
struct NativeCrtPowFallbackContext;
struct NativeCrtLibmErrorSupportContext;

// Stable borrowed pointers to the actual dispatch cell and complete provider
// contexts. The referenced dispatch DWORD is read at entry; it is neither an
// initial-value snapshot nor a request to force either implementation.
struct NativeCrtPowDispatchContext {
    const volatile std::uint32_t* dispatch_0109eea0;
    const NativeCrtPowFallbackContext* fallback;
    const NativeCrtLibmErrorSupportContext* sse2;
};

// ASSEMBLY CALLERS ONLY. Full BFEB10[84], with added context at entryESP+4.
// Enter with ST0=y and ST1=x, return one ST0 result replacing those two values.
// The void declaration does not expose an ordinary C++ numeric-return ABI.
// Plain RET; caller removes the added context word. EBX/EBP/ESI/EDI are
// preserved on normal return. Original branch decisions are retained; incidental
// final EFLAGS equality is excluded because ADD ESP uses a shifted source frame.
// The selected full provider governs floating-point effects and volatile state.
void __cdecl dispatch_native_crt_pow_00bfeb10(
    const NativeCrtPowDispatchContext&);

// Nonzero current dispatch first checks MXCSR mask bits1F80, then only on
// equality FNSTCW bits7F. The native LEA cleanup preserves the selected test's
// flags. Full SSE2 wrapper is reached only when both comparisons match.
// Otherwise the fallback retains FXCH/FSTP x/FST y/current y-high-DWORD order.
// No FP controls or status are normalized. The actual platform must support
// every reached instruction, and x87 must have the providers' required space.
//
// Context/member bindings and the added argument stay valid/stable. Only the
// reached provider's context is dereferenced. The fallback's four raw bindings
// and persistent owning LegacyCrtMathRuntime must satisfy its existing header;
// the SSE2 context satisfies native_crt_libm_error_support.hpp. No second errno,
// callback registry, TLS domain, host pow or default dispatch value is created.
//
// New source stack reservations, saved binding registers, SSE2 forwarding
// frame and code/data addresses are explicit ABI differences. Caller-private
// frame aliases, nonlocal-exit register restoration, native SEH/fault-site
// identity and original caller/drop-in compatibility are not established.
// Adjacent alternate entry BFEB64 is outside this complete84-byte function.
} // namespace bsp
