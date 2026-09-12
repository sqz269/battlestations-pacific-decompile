#pragma once

namespace bsp {
struct NativeCrtLibmErrorSupportContext;

// ASSEMBLY CALLERS ONLY: these void declarations do not expose a C++ numeric
// return. Complete C19260[25] wrapper, qualified by an added explicit context
// word at entry ESP+4 and extra stack reservation. Enter with ST0=y exponent,
// ST1=x base. FXCH and two ordered FSTPs round the inputs under current x87
// controls before the core call. Normal return replaces those two entries with
// one ST0 result, retaining deeper entries. EBP and entry ESP are restored.
void __cdecl native_crt_sse2_pow_x87_00c19260(
    const NativeCrtLibmErrorSupportContext&);

// Complete C19279[2872], all 645 instructions and 15 returns. E=entry ESP:
// original x qword at E+4, y qword at E+Ch; added context address at E+14h.
// Plain RET; caller removes 20 argument bytes. Normal return pushes one result
// on x87 ST0. XMM0 is not a uniform numeric return. Core integer/XMM effects
// remain the original sequence; caller upper XMM lanes are not assumed zero.
// EAX/ECX/EDX, XMM0..7, integer flags and FP status may change. EBX/EBP are
// untouched; ESI/EDI are saved on the scaling route. The actual full error
// service additionally has its documented provider/callback ABI effects.
void __cdecl native_crt_sse2_pow_00c19279(
    double x, double y, const NativeCrtLibmErrorSupportContext&);

// Both entries retain current MXCSR and x87 controls, including rounding,
// exception masks, DAZ/FTZ and accrued status. Packed operations, deliberate
// FP exceptions and memory/register scalar-move lane effects are preserved.
// Call with sufficient available x87 depth and actual SSE2 support. The raw
// wrapper aligns outgoing x/y to 16 bytes; core entry ESP is then 12 modulo16.
// No host pow, environment normalization, exception guard or forced dispatch.
//
// The borrowed context, its references and actual owning-CRT providers must
// satisfy native_crt_libm_error_support.hpp for every reached error route.
// Added context storage/bindings stay valid and stable for the call. The fixed
// bridge passes original x/y addresses and the core's private result to the
// complete C0F0E4 source provider. Caller argument or provider-side mutations
// retain the service's actual ordering. No ambient binding/TLS or substitute
// generic math-error callback is introduced here.
//
// Exact original constant bytes reside in rebuilt aligned read-only arrays.
// New code/data addresses, extra wrapper/bridge stack, context loads and the
// provider's qualified source ABI do not reproduce original fault addresses,
// SEH continuation or incidental provider volatile-register behavior. This is
// a qualified raw source interface, not original-address/drop-in compatibility.
} // namespace bsp
