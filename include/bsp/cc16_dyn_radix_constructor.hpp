#pragma once

#include <cstdint>

namespace bsp::cc16_dyn_radix_constructor {

// Required genuine table binding corresponding to original image identity
// 00D7A118. This packet supplies no callable host table or default binding.
// The caller must establish provider custody before using it at runtime.
struct VtableBinding {
    const void* table_for_00d7a118;
};

// Recovered original bytes 00C37210..00C37236, 39 bytes, still undefined in
// Ghidra at recovery time. Native ABI: EAX = manager and unchanged result,
// no stack arguments, plain RET; ECX becomes zero, other GPRs unchanged.
//
// Store the required binding for original immediate 00D7A118 at manager+00h,
// then zero ten dwords at +04h..+28h in order. Nothing beyond +2Bh is touched.
// Valid writable Win32 storage required; no null guard or allocation added.
// Explicit C++ transport does not reproduce the native register/flags ABI.
// Body credit and activation remain provisional pending integrator boundary
// acceptance and genuine vtable-provider custody. Names are hypotheses.
// See docs/CC16_DYN_RADIX_CONSTRUCTOR.md.
void* construct_manager_00c37210(void* manager,
    const VtableBinding& vtable) noexcept;

} // namespace bsp::cc16_dyn_radix_constructor
