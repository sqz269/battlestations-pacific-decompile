#pragma once
#include "bsp/native_input_binding_storage.hpp"
#include <array>
#include <cstdint>

namespace bsp {
// Actual owner24h, action30h and binding34h storage, shared with the parser.
// Native ECX owner; stack action; EAX signed count; RET4. No action bounds check.
std::int32_t count_native_input_binding_slots_00a92820(
    const void* actual_owner, std::uint32_t action) noexcept;

// Native ECX owner; stack action, signed slot, descriptor14h output, float*
// output; RET10h. Signed count > slot selects a record, including negative slots
// when addressable. Copies five words in order, then x87 FLD/FSTP scale.
// Missing slot: {-1,0,0,-1,preimage & FFFFFF00}, +0 scale. The last word's high
// bytes originate in an uninitialized NATIVE stack local, not the output buffer.
// The explicit preimage represents those otherwise unspecified padding bytes.
void read_native_input_binding_slot_00a92790(const void* actual_owner,
    std::uint32_t action, std::int32_t slot, void* descriptor14,
    float* scale, std::uint32_t missing_flag_stack_preimage) noexcept;

// Native ECX owner; stack action, signed slot, FIVE descriptor words BY VALUE,
// float scale; RET20h. Capture by value before possible growth. Preserve modifier
// ownership and all unwritten bytes; replace the full flag word and raw scale
// bits, set curve=(class==2), then concretely rebind every binding in the action.
void install_native_input_binding_slot_00a93750(void* actual_owner,
    std::uint32_t action, std::int32_t slot,
    std::array<std::uint32_t, 5> descriptor, float scale,
    const NativeInputBindingStorageContext&,
    void* volatile& backend_00f8bbf4);

// Source service ABI, not original ABI/SEH. Caller supplies addressable storage,
// real CRT allocation and actual backend pointer lists. No hardware polling.
} // namespace bsp
