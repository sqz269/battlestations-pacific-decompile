#pragma once

#include "bsp/d3d9_surface_pool.hpp"
#include "bsp/native_renderer_texture_name_notification.hpp"
#include "bsp/native_retained_memory_owners.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_volume_texture_owner_bytes = 0x34;

// Borrow the actual application publication, strings, retained-memory domain,
// renderer profile and initialized volume pool at native 0108DBA8. Native
// CD7BA0 calls B3EC60 for this pool, sharing the existing 38h-slot algorithm.
// This companion must wrap that actual volume storage, not the surface pool
// at 0108DB00. It creates no pool, allocator list, renderer or reference count.
struct NativeVolumeTextureOwnerContext {
    NativeRendererTextureNameNotificationContext& renderer_notification;
    NativeRetainedMemoryOwnerContext& retained_memory;
    D3D9SurfacePool& actual_volume_pool_0108dba8;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
};

// Full B340F0: native ECX owner, five-byte JMP B33F50; no extra state.
// Distinct volume base cleanup entry, with the actual current name domain.
void unwind_native_volume_texture_base_00b340f0(
    void* actual_owner, NativeStringStorage& actual_strings);

// Full B3EB70: native ECX owner, RET; no semantic result. Notify through the
// captured renderer/current table +6C, then release CURRENT retained source
// +30. Capture COM +10 for AddRef and Release; independently reload +10 for
// its final Release. Each field clears only after its returning release path.
// State-0 unwind runs B340F0 -> the actual named base B33F50; no retry or
// slot return occurs. Current name/base destruction also runs on normal exit.
void destroy_native_volume_texture_00b3eb70(
    void* actual_owner, NativeVolumeTextureOwnerContext&);

// Full B3F430: native ECX owner, stack flags, EAX original address, RET4.
// After successful destruction, flags bit0 returns the actual volume slot
// with B3D860. Other bits add no action. Preserve owner metadata and +34
// slab index; the returned address may already be reusable.
void* delete_native_volume_texture_00b3f430(void* actual_owner,
    std::uint32_t flags, NativeVolumeTextureOwnerContext&);

// Supported renderer profile D5F0A8 has +6C=B32250; retained profiles are
// those admitted by NativeRetainedMemoryOwnerContext. All reached storage
// and actual COM tables must remain valid at the native accesses. The name
// storage and notification sized pool share their actual allocation domain.
// New MSVC Win32 source interfaces, not native ABI entries or game proof.

} // namespace bsp
