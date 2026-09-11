#pragma once

#include "bsp/native_physical_buffer_lock.hpp"

#include <cstdint>

namespace bsp {
// Borrow all nine original DWORDs of each observed immutable physical profile.
// These cells contain native code-address tokens, never callable host pointers.
struct NativeLogicalBufferDeviceSaveProfiles {
    const volatile std::uint32_t* private_index_00d61e10;
    const volatile std::uint32_t* private_vertex_00d61e34;
    const volatile std::uint32_t* pooled_index_00d61e58;
    const volatile std::uint32_t* pooled_vertex_00d61e7c;
};
struct NativeLogicalBufferDeviceSaveContext {
    NativePhysicalBufferLockContext& actual_physical_lock;
    const NativeLogicalBufferDeviceSaveProfiles& actual_physical_profiles;
};

// Complete native ECX logical-owner, no-stack-argument, plain-RET operations.
// The new host ABI additionally borrows the actual service context in EDX.
// Actual vertex fields: physical+58, flags+60, shadow+6C. Index: +08/+10/+1C.
// Only the four observed immutable profiles and their exact recovered targets
// are in-domain. Each selected slot calls the actual compiled implementation;
// no numeric native address is invoked and no arbitrary callback fallback exists.
// Allocation, Lock and copy use three separate current capacity observations.
// Old shadow is not freed; exceptions preserve already completed mutations.
// The owner, table, capacity and shadow are reloaded at their original points.
void __fastcall save_native_logical_vertex_buffer_for_device_reset_00b49d00(
    void* actual_logical, NativeLogicalBufferDeviceSaveContext& actual_context);
void __fastcall save_native_logical_index_buffer_for_device_reset_00b49f80(
    void* actual_logical, NativeLogicalBufferDeviceSaveContext& actual_context);
} // namespace bsp
