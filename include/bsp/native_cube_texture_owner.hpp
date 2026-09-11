#pragma once

#include "bsp/native_renderer_texture_name_notification.hpp"
#include "bsp/native_retained_memory_owners.hpp"

#include <cstddef>
#include <cstdint>
#include <d3d9.h>

namespace bsp {

inline constexpr std::size_t native_cube_texture_owner_bytes = 0x30;

// Borrow the application's actual publication, strings, retained-memory
// counters/tables, shared serial and initialized canonical raw cube pool.
// The notification's string storage and sized pool must share the same actual
// allocation domain. No renderer, COM reference count, owner or pool is made
// by this context. The table contains original code words, not host callbacks.
struct NativeCubeTextureOwnerContext {
    NativeRendererTextureNameNotificationContext& renderer_notification;
    NativeRetainedMemoryOwnerContext& retained_memory;
    void* actual_cube_pool_0108db70;
    std::uint32_t& actual_shared_serial_0108d6e8;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
};

// Full B3D650: native ECX actual raw owner; stack cube COM/flags; EAX owner;
// RET8. Complete unnamed base, then current input COM GetLevelDesc(0) and
// GetLevelCount. Store Width before the second call, Format and returned mip
// count afterward. Descriptor storage is not zeroed; HRESULT is ignored.
// No AddRef. Caller retains responsibility for the raw slot and COM on failure.
void* construct_native_cube_texture_00b3d650(void* actual_raw_owner,
    IDirect3DCubeTexture9* input_texture, std::uint32_t flags,
    NativeCubeTextureOwnerContext&);

// Full B3EAD0: native ECX owner, RET. Release current retained source, notify
// through captured renderer/current table+6C, release current COM, then destroy
// the actual name/base. Native cleanup-only state invokes B34090/B33F50;
// source/COM release is not retried and clears occur only after calls return.
void destroy_native_cube_texture_00b3ead0(
    void* actual_owner, NativeCubeTextureOwnerContext&);

// Full B3F410: native ECX owner, stack flags, EAX original address, RET4.
// Only flags bit0 returns the actual canonical pool slot, after destruction.
// All three entries preserve owner+28 and pool slab index+30. The returned
// address may be reusable. No generic-registry removal is added.
void* delete_native_cube_texture_00b3f410(void* actual_owner,
    std::uint32_t flags, NativeCubeTextureOwnerContext&);

// Supported native tables are renderer D5F0A8/+6C=B32250 and the existing
// backing/stream profiles admitted by NativeRetainedMemoryOwnerContext. COM
// dispatch uses its actual current interface table. Reached storage must be
// valid at the native accesses. These are new MSVC Win32 source interfaces,
// not drop-in native ABI entries or GPU/game validation.

} // namespace bsp
