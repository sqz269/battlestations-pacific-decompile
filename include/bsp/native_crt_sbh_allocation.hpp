#pragma once

#include "bsp/native_crt_sbh_state.hpp"

#include <cstdint>

namespace bsp {
// Complete original bodies: C12968 739B, C1207C 176B, C1212C 262B.
// Evidence: NATIVE_CRT_SBH_ALLOC_BW.md, discovery d61bab1db06225b86fc80d7b087e29ab4d90b2b0.
// Require actual bootstrapped native SBH state, the original valid size-class
// domain, mapped writable native extents, and the external lock-4 contract.
// Preserve real Win32 allocation failure and partial publication; no ownership
// or heap provider is created. Added C++ state references and scratch/register
// differences are not a native ABI, SEH/FH3 or asynchronous-fault claim.
void* allocate_native_sbh_block_00c12968(
    std::uint32_t requested_bytes, const NativeCrtSbhState& state);
void* allocate_native_sbh_region_00c1207c(const NativeCrtSbhState& state);
std::int32_t allocate_native_sbh_group_00c1212c(void* actual_descriptor);
} // namespace bsp
