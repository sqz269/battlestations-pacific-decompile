#pragma once

#include "bsp/native_singleton_vector_allocation.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic vector failure children require MSVC Win32.
#endif

namespace bsp {

// Whole logical 0047E970..0047E9C4,85B/25ops. Original: count in ECX,
// no consumed incoming EDX or stack argument, EAX raw storage, plain RET.
// Zero still requests zero bytes; count3FFFFFFF still requests FFFFFFFC.
// Counts above3FFFFFFF use the already qualified current-CRT std::bad_alloc
// transport; admitted counts use the actual malloc/_callnewh/retry service.
// The complete Original matches 00BCFEB0 except its three true CALL rel32
// operands, whose decoded Native targets also match. Source composes that
// complete real implementation; it is not a Native edge to 00BCFEB0.
void* __fastcall allocate_native_traffic_vector_0047e970(std::uint32_t count);

// Whole logical 00490240..004902A8,105B/23ops; no consumed input or normal
// return. Native completed-message state0/C62E40 cleanup matches CC5470's
// real Source guard: initialize actual1Ch SBO fields, assign18 bytes, arm
// cleanup only after assignment, construct actual28h storage, publish D69260,
// then throw owning NativeSingletonVectorLengthError. Its construction/copy/
// destruction are real legacy-storage providers, not std::length_error.
[[noreturn]] void __cdecl throw_native_traffic_vector_length_00490240();

// Existing declared service boundaries remain: current allocation/exception
// transport, new host catch type/RTTI/EH, no original private CRT/ThrowInfo/
// FH3/SEH/heap/new-handler identity, arbitrary unwind-spill alias mutation or
// hardware-fault/register/flag identity. No complete binary ABI or game proof.
// No new class, provider stub, callback/global, guard or allocator domain.
// See docs/GOAL15T_TRAFFIC_VECTOR_FAILURE_CHILDREN.md.

} // namespace bsp
