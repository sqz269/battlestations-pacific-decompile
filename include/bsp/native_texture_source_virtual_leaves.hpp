#pragma once

#include "bsp/native_procedural_resource_lifetime.hpp"
#include <cstdint>

namespace bsp {

// C30210[18]: ECX fresh actual base storage; RET/EAX captured self. Reuses
// B19980's fresh/aligned/exclusive +04 atomic construction contract, then
// stamps D79AEC at +00. At least the actual8B base is required. No payload,
// callable vtable, allocation, retain, complete-owner or derived admission.
void* construct_native_texture_source_base_00c30210(void* actual_storage) noexcept;

// C30270[30]: ECX actual base, stacked flags, RET4/EAX captured self. Genuine
// B19750 completes first; free captured self iff low-byte bit0. Returns those
// captured bits even after free. Caller owns canonical/companion retirement.
void* delete_native_texture_source_base_00c30270(
    void* actual_owner, std::uint32_t flags) noexcept;

// C30550[30], published at D79B54+04. D64494/D644D0/D79B70 are separate
// C30570 producer entries at profile+1C, not deleting-table starts. Reuses the genuine
// existing C304A0 over the SAME live C30470 payload, canonical child domain
// and fresh retained operation. Operation diagnostics describe that delegated
// C304A0 invocation; no rewrite to wrapper function/site is made. Only normal
// destroy return can free captured owner (flags bit0), then record that free.
// Failure preserves existing cleanup/state semantics and never frees owner.
// No count change, null-child guard, reset, replay, unbind or new child credit.
void* delete_native_texture_source_00c30550(
    void* actual_owner, std::uint32_t flags, NativeRenderActualOwners&,
    NativeProceduralResourceLifetimeOperation&);

} // namespace bsp
