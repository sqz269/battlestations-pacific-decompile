#pragma once

#include "bsp/native_physical_buffer_lock.hpp"
#include "bsp/native_physical_buffer_owner.hpp"

#include <cstdint>

namespace bsp {
// Borrow all nine original DWORDs of each observed immutable physical profile.
// Native code-address cells are tokens, never callable host function pointers.
struct NativeLogicalBufferDeviceRestoreProfiles {
    const volatile std::uint32_t* private_index_00d61e10;
    const volatile std::uint32_t* private_vertex_00d61e34;
    const volatile std::uint32_t* pooled_index_00d61e58;
    const volatile std::uint32_t* pooled_vertex_00d61e7c;
};
struct NativeLogicalBufferDeviceRestoreContext {
    NativeLogicalBufferDeviceRestoreContext(const void* volatile& renderer,
        NativePhysicalBufferOwnerContext& attachment,
        NativePhysicalBufferLockContext& lock,
        const NativeLogicalBufferDeviceRestoreProfiles& profiles,
        std::uint32_t pool_stack_bits) noexcept
        : actual_renderer_00f8d394(renderer), actual_physical_attachment(attachment),
          actual_physical_lock(lock), actual_physical_profiles(profiles),
          native_pool_stack_bits(pool_stack_bits) {}

    const void* volatile& actual_renderer_00f8d394;
    NativePhysicalBufferOwnerContext& actual_physical_attachment;
    NativePhysicalBufferLockContext& actual_physical_lock;
    const NativeLogicalBufferDeviceRestoreProfiles& actual_physical_profiles;
    // Required INPUT bits for the original uninitialized pool local: vertex
    // entry ESP-4, index entry ESP-12. Read only when low flag nibble exceeds3.
    // This is not an externally writable stack output or a default D3D pool.
    const std::uint32_t native_pool_stack_bits;
};

// Complete native ECX logical owner/no stack args/plain RET. The new host ABI
// additionally requires context in EDX. Original callers need that binding.
// Actual vertex fields: physical+58, flags+60, count+64, declaration+68,
// shadow+6C; declaration stride+CC. Index fields: +08/+10/+14/+18/+1C.
// Actual Attach and Lock contexts must share the actual lifetime domain.
// Only the four observed immutable profiles and recovered slots are admitted.
// Temporary COM Release is unconditional, including the native null fault.
// Current shadow is freed and then cleared; no rollback or extra guards exist.
void __fastcall restore_native_logical_vertex_buffer_after_device_reset_00b49dc0(
    void* actual_logical, NativeLogicalBufferDeviceRestoreContext& actual_context);
void __fastcall restore_native_logical_index_buffer_after_device_reset_00b4a040(
    void* actual_logical, NativeLogicalBufferDeviceRestoreContext& actual_context);
} // namespace bsp
