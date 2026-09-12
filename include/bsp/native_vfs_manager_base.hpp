#pragma once

#include "bsp/sound_lifetime_access.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS manager base requires MSVC Win32.
#endif

namespace bsp {

// Complete logical raw-storage bodies: BDA6F0[145], BDA790[153], BDA8E0[30].
// Native ctor/dtor consume ECX=owner, no stack arguments, RET; ctor EAX=owner.
// Native deleting wrapper consumes ECX=owner, one DWORD flags, EAX=owner, RET4.
// Source bindings explicitly borrow the actual0109CEEC publication cell and
// existing lifetime bridge (raw01090AA0 or the canonical semantic domain).
// Only owner+0 is written: D683E4 during base lifetime, CE3818 on destruction
// or construction failure. Remaining bytes of the containing object survive.
// Profile DWORDs identify native tables; they are not host-callable vtables.
void* construct_native_vfs_manager_base_00bda6f0(
    void* owner, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime);

void destroy_native_vfs_manager_base_00bda790(
    void* owner, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime);

// Always destroy; free original owner only for flags bit0, ignore other bits.
// A propagated destruction exception does not reach free. Return the original
// address even when freed; no null-owner special case exists in native code.
void* delete_native_vfs_manager_base_00bda8e0(
    void* owner, std::uint32_t flags, void* volatile& actual_published_0109ceec,
    SoundLifetimeAccess lifetime);

// Source C++ cleanup preserves getter/registration failures and current-global
// scheduling. Original FH3/SEH maps, hardware-fault cleanup during Enter/Leave,
// mutable EH spills and provider register/throw identities remain boundaries.
// These additional source bindings are not the original callable ABI, and
// this module alone does not establish complete-manager or game validation.

} // namespace bsp
