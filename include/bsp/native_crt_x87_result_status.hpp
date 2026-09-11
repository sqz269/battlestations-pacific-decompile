#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT x87 result status requires MSVC Win32.
#endif

namespace bsp {

// Binding description only: the assembly caller loads EBX with this pointer;
// the body never reads this aggregate. Borrow the actual immutable 48-byte
// region D6A684..D6A6B3, retaining its original relative layout (six qwords):
// +00h max finite, +08h min normal, +10h -1536, +18h +1536,
// +20h positive infinity, +28h positive zero. Each reached read is eight bytes.
// No copy/synthesized table, entry-time validation, global or snapshot is added.
// Storage must remain valid for each reached access. EBX remains preserved by
// the raw wrapper and the concrete providers' established Win32 cdecl contract.
struct NativeCrtX87ResultStatusContext {
    const void* literals_00d6a684;
};
static_assert(sizeof(NativeCrtX87ResultStatusContext) == 4);

// ASSEMBLY JMP/TAIL ENTRY ONLY. This declaration is an assembly naming
// interface, not a safe ordinary C++ CALL, original address or drop-in ABI.
// Enter with ESP pointing to the actual four-byte packed saved-control word:
// +00h packed CW; +04h actual return; +08h current first argument qword;
// +10h current second argument qword when operation1Dh chooses binary dispatch.
// ECX=actual name, EDX=operation, ST0=current result; EBX is bound as above.
// One additional free x87 slot is required. Normal return leaves one result
// in ST0, POPs the current entire packed word into EDX, then RET consumes the
// actual return. Outer argument cleanup belongs to that caller. No wrapper
// may push a new return above the CW before entering this tail.
//
// Complete C08479[163]. Temporary FST to double does NOT pop ST0. The rounded
// exponent drives original normal/zero/all-ones paths; it is not a host FP
// predicate. All FSCALE/FCOMP/FMUL operations, three waiting FSTSW AX sequences,
// current CW rereads/default027Fh skip and type8 status/mask path remain exact.
// Operation1Dh calls complete binary dispatch; other operations use unary.
// Current precision/rounding/status/tags/exceptions apply. EAX/ECX and integer
// flags follow the actual path/providers; no incidental original volatile
// register equivalence is promised through the concrete C++ error provider.
//
// Before any reached dispatch, bind the persistent actual LegacyCrtMathRuntime
// through the existing API: actual E16BD0 cell and owning CRT errno accessor.
// Its reserved-FPIEEE/precision01/FP-status and Win32 runtime/SEH boundaries
// remain. No generic callback, host arithmetic or forced dispatch policy.
// There is no EH guard/new cleanup; faults/nonlocal exits need not restore CW,
// pop the packed word or return. These are normal-return obligations only.
void __cdecl finish_native_crt_x87_result_status_tail_00c08479();

} // namespace bsp
