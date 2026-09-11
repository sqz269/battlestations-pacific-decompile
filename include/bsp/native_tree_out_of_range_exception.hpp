#pragma once

#include "bsp/native_legacy_exception_owner.hpp"

namespace bsp {

// Complete 00441760..00441779 exclusive. Native ECX actual 28h destination,
// stack source-owner address; EAX destination; RET4. The existing logic-error
// copy owns all failure cleanup; publish out_of_range D6926C only on success.
NativeLegacyExceptionStorage& copy_native_tree_out_of_range_00441760(
    NativeLegacyExceptionStorage&, const NativeLegacyExceptionStorage&);

// Complete 004412B0..004412E2 exclusive. Native ECX actual 28h owner; RET via
// base-destructor tail. Publish logic-error D69248, dispose/reset member, then
// dispose the current owned base message. Does not free the owner allocation.
void destroy_native_tree_out_of_range_004412b0(
    NativeLegacyExceptionStorage&) noexcept;

// Borrow actual legacy storage. Numeric native profiles remain address data;
// these operations do not implement native RTTI, exception dispatch or ABI.
} // namespace bsp
