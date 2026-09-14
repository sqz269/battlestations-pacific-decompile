#pragma once

#include "bsp/native_singleton_vector_allocation.hpp"

#include <cstdint>

namespace bsp {

// Complete source allocation contract for 00B0D3B0..00B0D405. Original ECX
// is the unsigned record count; EDX is not consumed; EAX is returned raw
// storage and RET does not pop arguments. This __fastcall source declaration
// reserves EDX explicitly. Zero still requests zero bytes through the actual
// malloc/new-handler service. Positive counts up to UINT32_MAX/300 request
// precisely count*300 native and host bytes; larger counts throw source
// std::bad_alloc. Callers release returned storage with singleton_lifetime_free.
void* __fastcall allocate_native_material_records_00b0d3b0(
    std::uint32_t count, void* unused_edx);

// 00B135C0..00B13628 has no input or normal return. The source path builds
// the actual 1Ch SBO temporary, assigns 18 bytes, arms its cleanup only after
// assignment succeeds, constructs a 28h native length-error payload, and
// throws the SAME host NativeSingletonVectorLengthError catch type used by the
// analogous 00BD0590 source. Native D83F98 RTTI/FH3/SEH is not host C++ EH.
[[noreturn]] void __cdecl STL_xlen_throw_00b135c0();

} // namespace bsp
