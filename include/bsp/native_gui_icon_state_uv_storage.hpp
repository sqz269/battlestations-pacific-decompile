#pragma once
#include <cstdint>

namespace bsp {
// Whole normal data/FP schedule of AB1680..AB1701. Native ECX is an actual
// already-constructed 40h Icon state; four DWORD stack homes are consumed and
// RET10h follows. This ordinary MSVC Win32 Source interface exposes those homes
// directly and changes the binary ABI. Incidental EAX/EDX/FLAGS are not outputs.
//
// Keep the genuine state/backing/subobjects and four addressable DWORD homes
// alive and writable through the call. The caller stages the real by-value
// inputs at its reached PUSH epochs; this function neither seeds nor copies
// their storage. Volatile views of ordinary DWORD objects are permitted. Homes
// may alias state DWORD subobjects where their C++ lifetimes are valid: the
// reached native read/store order is retained, without snapshots or a domain
// filter. The Source implementation's own parameter/control/spill storage is
// separate, not a substitute for the original private stack or fault ABI.
//
// actual_one_00d7a24c borrows the SAME stable authenticated PE float cell used
// by the actual GUI suppliers, through its existing retained read-only owner.
// No literal copy, alternate bank, owner, reference/count or storage birth is
// created here. There is no nullable fallback or new record/profile guard.
//
// SSE UCOMISS/LAHF/TEST/JP and all four FLD/FSTP forms stay explicit, including
// current authored reads and signed-zero/NaN swap behavior. No C++ FP operation,
// FP reset/control policy, allocation, callback, EH action or cleanup is added.
// Ambient x87/MXCSR state remains the caller's actual state. No noexcept,
// Original FLAGS/register/stack, SEH/FH3, hardware-fault or unmasked-FP exception
// equivalence is certified by this Source API; emitted code remains unverified.
void set_native_gui_icon_state_uv_storage_00ab1680(void* actual_40b_state,
    volatile std::uint32_t (&actual_argument_homes)[4],
    const volatile float& actual_one_00d7a24c);
} // namespace bsp
