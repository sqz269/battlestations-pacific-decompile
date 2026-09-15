#pragma once

#include <cstdint>

namespace bsp {

// Complete native filter entry 00C16B4C[20] from __ValidateEH3RN's EH4 scope.
// No stacked arguments. Requires the ORIGINAL inherited EBP: [EBP-14h]
// points to native EXCEPTION_POINTERS, whose first word is EXCEPTION_RECORD*.
// The caller owns that native frame and both pointed-to objects. This is not
// an ordinary C++ call interface: use only the actual native filter context.
// Read the three words in original order. Return EAX=ECX=1 exactly when the
// original exception code is C0000005h; otherwise EAX=ECX=0. EDX retains the
// EXCEPTION_RECORD pointer. Arithmetic flags remain those of CMP(code,
// C0000005h); preserve EBP/EBX/ESI/EDI and consume only the RET address.
// No null check, frame binder, memory owner, catch, or handler is supplied.
std::int32_t __cdecl filter_native_crt_eh3_access_violation_00c16b4c();

// Byte/build/static-link evidence does not prove native exception execution
// or gameplay. C16960, its fixed scope and C16B60 landing remain incomplete.
} // namespace bsp
