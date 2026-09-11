#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pow fallback requires MSVC Win32 raw assembly.
#endif

namespace bsp {

// Binding description only; the body never reads this aggregate. Assembly
// callers load EBP/EBX/ESI/EDI respectively from these four borrowed pointers.
// EBP addresses the actual current DWORD0109DD78; each original CMP rereads
// that cell. The other regions retain actual immutable original bytes/layout:
// EBX: six-qword D6A684 status region (48 bytes).
// ESI: D7A280 half cell (8 bytes).
// EDI: E165A0 anchor with original relative cells: halfE15850 at -D50h (8),
// nameE15858 at -D48h (4, "pow\0"), NaN80E15C70 at -930h (10),
// infinityE165A0 at +0 (8), qNaNE165A8 at +8 (8), zeroE165C0 at +20h (8),
// infinity80E166D0 at +130h (10). Gaps are not read. This is the original
// relative region layout, not a copied/synthesized table. Every reached cell
// must remain valid; no initial value snapshot, validation or policy is added.
struct NativeCrtPowFallbackContext {
    const volatile std::uint32_t* dispatch_0109dd78;
    const void* status_literals_00d6a684;
    const void* special_half_00d7a280;
    const void* literal_anchor_00e165a0;
};
static_assert(sizeof(NativeCrtPowFallbackContext) == 16);

// ASSEMBLY CALLERS ONLY. New source binding, not an ordinary C++ pow call or
// an original-address/drop-in ABI. Enter via CALL with EAX=y's already-loaded
// high DWORD, ST0=current y, and the four register bindings above. At entry:
// ESP+0 return; +4 current x qword; +0Ch current y qword; +14h readable scratch
// DWORD. EAX and ST0 are original caller inputs, never rebuilt from the slots.
// Three additional free x87 slots cover the original negative-base path.
// On normal return ST0 contains the result; caller removes its20-byte x/y/
// scratch frame. No C++ floating return or extra stack-argument adapter exists.
//
// The original PUSH EAX creates packed CW storage. Waiting FSTCW overwrites
// only its low word; the high scratch word remains original unless C083A5
// writes the working CW there. All current raw widths/rereads remain, including
// TEST dword [packedCW+17h],80h spanning y's sign byte and three scratch bytes.
// Every normal return consumes packed CW into EDX before consuming the return.
//
// Source adaptations: six same-length address operands; a private11-byte
// LEA EDX,[EDI-D50h]/JMP binds the complete BFED32 helper at both CALL sites;
// two XCHG EBX,ESI surround only the original C19E24 CALL. They preserve flags,
// stack and FP, restoring nonvolatile bindings on normal return. The helper
// thunk changes intermediate EDX, whose old value neither continuation uses;
// eventual EDX is the current packed CW. Full original helper CL/AX/flags and
// original stack arguments remain. No nonlocal-exit register restore promised.
//
// Waiting FSAVE, its actual108-byte save area, original uninitialized output
// cell, argument FSTPs and normal FRSTOR/current-output FLD stay ordered. The
// special provider runs under the environment reset by FSAVE. No new EH guard
// or fault cleanup is installed: nonlocal exits need not FRSTOR or restore CW.
// Current x87 rounding/precision/status/tags/exceptions apply; no host pow or
// generic arithmetic callback. Persistent actual LegacyCrtMathRuntime must be
// bound before a reached error dispatch. Its reserved-FPIEEE/precision01,
// FP-status, volatile-register and Win32 runtime/SEH limits are inherited.
// This entry does not read/change109EEA0 or implement the public SSE2 dispatch.
void __cdecl evaluate_native_crt_pow_fallback_00bfeb6d();

} // namespace bsp
