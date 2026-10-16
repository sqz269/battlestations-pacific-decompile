#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual controller-name pair destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A2BB0[143B/46starts]. Native ECX pair, EDX unused, no stacked
// arguments/plainRET, incidental EAX. Source semanticvoid adds borrowed actual
// RawCtx/RET4. Key length0/data4, embedded tree8 with headC/count10.
// Capture head/left and real8B iterator arguments, then arm key cleanup before
// the whole actual range. Keep armed through current head free/head0/count0
// and current data4 capture; disarm/end scope BEFORE normal length/getter.
// Normal release uses captured data, never rereads through 41DD20. No header
// clear, pair free, generic model/callback/layout bridge, rollback or retry.
void __fastcall destroy_native_input_controller_name_pair_006a2bb0(
    void* actual_pair, std::uint32_t unused_edx,
    NativeStringRawPoolContext& strings);

// Whole8B/2starts logical C7F200 effect: Native MOV ECX,[runtimeEBP-18h],
// tail41DD20. New explicit pair/RawCtx logical Source interface, no EBP reader
// or added header offset. Source fastcall uses ECX pair/EDX RawCtx, plainRET;
// not Original implicit-frame ABI. RawCtx getter failure propagates.
void __fastcall cleanup_native_input_controller_name_pair_key_00c7f200(
    void* actual_pair, NativeStringRawPoolContext& strings);

// Source guard destruction is noexcept(false). A second cleanup exception
// during C++ unwinding retains the current nested-failure/termination limit.
// No Original FH3/SEH/RTTI/fault/private-frame/heap/register/flags/binary ABI or
// game equivalence is established. Existing AV/typed/RawCtx domains remain.
} // namespace bsp
