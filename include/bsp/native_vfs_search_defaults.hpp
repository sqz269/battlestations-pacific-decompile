#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS startup search registration requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Complete ordered 00738360 registrations against the actual A0h VFS manager
// publication. Borrow its 0109CEEC slot and reread it after constructing both
// string headers for each call; callbacks may replace the published manager.
// The publication, string pool, and callbacks must remain alive through use.
// This source interface is not the original binary or EH ABI.
void register_native_vfs_search_defaults_00738360(
    void* volatile& actual_manager_publication_0109ceec,
    ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks);
}
