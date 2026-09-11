#pragma once

#include "bsp/d3d9_texture2d_pool.hpp"
#include "bsp/native_renderer_texture_name_notification.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/native_surface_owner.hpp"

namespace bsp {

inline constexpr std::size_t native_texture_2d_owner_bytes = 0x50;

// Borrow the application's actual services, publication slots and table words.
// Notification and surface contexts must view the SAME renderer publication
// and string pool. The surface context supplies this owner's actual support
// singleton and lifetime domain. No replacement renderer,
// pool, COM reference count or retained-source owner is created by this context.
struct NativeTexture2DOwnerContext {
    NativeRendererTextureNameNotificationContext& renderer_notification;
    NativeRetainedMemoryOwnerContext& retained_memory;
    NativeSurfaceOwnerContext& surfaces;
    D3D9Texture2DPool& actual_texture_pool_0108db38;
    std::uint32_t& actual_shared_serial_0108d6e8;
    volatile std::uint32_t& actual_tracking_counter_0108daf8;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_surface_profile_00d619a0;
};

// Full B3F930: native ECX actual raw owner; stack name/COM/saved width/saved
// height/flags; EAX owner; RET14h. Construct the actual named base and D61948
// owner, query current COM tables, preserve the borrowed-COM diagnostic and
// actual support access. HRESULT is ignored; output fields retain the native
// read timing. Caller owns the original COM reference and raw slot on failure.
void* construct_native_texture_2d_00b3f930(void* actual_raw_owner,
    const void* actual_name_header, IDirect3DTexture9* input_texture,
    std::uint32_t saved_width, std::uint32_t saved_height,
    std::uint32_t flags, NativeTexture2DOwnerContext&);

// Full B3F2E0, including post-free tail: native ECX owner, RET. Release current
// retained source, notify through captured current renderer table+6C, unregister
// using fresh global renderer, release COM, release current cached surfaces,
// destroy cache storage and named base. Native current/captured read order and
// cleanup-only exception states survive; there is no source/COM cleanup retry.
void destroy_native_texture_2d_00b3f2e0(
    void* actual_owner, NativeTexture2DOwnerContext&);

// Full B3F590: native ECX owner, stack flags, EAX original address, RET4.
// Flags bit0 returns the canonical pool slot only after complete destruction.
// The returned address may be reusable. All entries preserve pool ID at +50h.
void* delete_native_texture_2d_00b3f590(void* actual_owner,
    std::uint32_t flags, NativeTexture2DOwnerContext&);

// Supported current native tables: renderer D5F0A8/+6C=B32250; cached surface
// D619A0/zero=BD30E0/deleting=B3F5B0; retained-memory tables and deleting slots
// already admitted by NativeRetainedMemoryOwnerContext. Numeric original code
// words select these established C++ functions; they are not host callbacks.
// COM calls use the actual current interface table. Accessed storage must remain
// valid at native reads/writes, and strings/cache blocks use the actual shared
// allocation domains. No safe-null COM fallback or GPU/game/ABI proof is added.

} // namespace bsp
