#pragma once

#include "bsp/native_crt_sbh_state.hpp"

#include <cstdint>

namespace bsp {

// Whole physical 00C11D3D..00C11D67, 43B/14ops; 00C11D61 is its interior
// loop test, not a second standard-ABI provider. Native entry: cdecl one live
// pointer DWORD at entryESP+4, EAX descriptor/null result, plain RET.
// Added state references bind the real canonical count109ED64/base109ED68.
// actual_parameter4 points to the actual live four-byte argument cell, read
// anew only on each reached iteration; an empty captured interval permits
// absent backing. Raw word reads retain x86 unaligned access without guards.
// Added Source binding/bookkeeping must be stable and disjoint from reached
// raw storage. Only reached reads must be valid; no guard/bootstrap/lock owner
// or Original ABI/frame/fault/game equivalence is supplied by this interface.
void* find_native_crt_sbh_region_00c11d3d(
    const NativeCrtSbhState& state,
    const void* actual_parameter4);

} // namespace bsp
