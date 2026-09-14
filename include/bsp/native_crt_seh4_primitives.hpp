#pragma once

#include <cstdint>

namespace bsp {

// Complete native __SEH_epilog4, 00C07C45[20]. This is a compiler-frame
// instruction entry, not a cleanup function for an ordinary C++ caller.
// On entry: EBP=F, ESP=P-4 holds this helper's continuation, [P] is the saved
// cookie, then saved EDI/ESI/EBX; [F-10h] is the previous FS:[0] link and [F]
// is the incoming EBP. FS:[0] must identify this frame at F-10h. The caller
// owns the actual valid frame and FS chain.
// Unlink first, discard cookie, restore, and return with ESP=F+4; preserve
// EAX and arithmetic flags. No cookie check, fault repair or exception policy.
void __cdecl leave_native_crt_seh4_frame_00c07c45();

// Complete native _EH4_CallFilterFunc, 00C0DCB6[23]. Native fastcall register
// inputs: ECX is the actual frame-based filter entry, EDX the actual frame.
// Borrows both identities; supplies no filter body, scope table or frame owner.
// The native funclet must return with a balanced stack. Preserve the original
// four-register save/restore, set EBP to the frame and zero EAX/EBX/EDX/ESI/EDI
// before CALL ECX. Return the actual filter EAX and arithmetic flags unchanged.
// No null validation, result normalization, catch or FS-chain repair is added.
std::int32_t __fastcall call_native_crt_eh4_filter_00c0dcb6(
    std::uintptr_t actual_native_filter_entry, void* actual_establisher_frame);

// These source symbols do not establish native-frame/exception/runtime or
// drop-in binary equivalence. The prolog, dispatcher, cookies and their owning
// state are separate; neither primitive makes C17653 source-complete.
} // namespace bsp
