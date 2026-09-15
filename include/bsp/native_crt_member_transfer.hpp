#pragma once

namespace bsp {
// Complete _CallMemberFunction0 at 00BF6AA6[7]. MSVC Win32 two-word
// stdcall entry: [ESP] continuation, [ESP+4] actual object, [ESP+8] actual
// native zero-argument member entry. This takes an executable address, not
// a C++ pointer-to-member descriptor or an injected host callback.
//
// POP EAX; POP ECX; XCHG [ESP],EAX; JMP EAX leaves the real continuation at
// the top of the stack and ECX=object, EAX=member_entry. The memory XCHG
// retains x86 implicit atomicity. The adapter changes no flags, nonvolatile
// registers, EDX or FP state. Target effects propagate without normalization.
// A balanced target RET resumes the original caller with both argument words
// consumed (8 bytes); nonlocal exit or a fault need not return. The adapter
// supplies no validation, extra frame, data state, target or exception owner.
// The caller must supply the actual native object, executable member entry
// and valid stack domain. A null object is forwarded unchanged.
//
// Evidence: docs/NATIVE_CRT_MEMBER_TRANSFER_EX.md and its companion report.
// Exact artifact bytes establish this leaf only; enclosing exception/runtime
// closure and gameplay validity remain separate and were not exercised.
void __stdcall call_native_crt_member_function0_00bf6aa6(
    void* object, void* member_entry);
} // namespace bsp
