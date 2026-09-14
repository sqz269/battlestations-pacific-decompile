#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS startup search registration requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Complete ordered 00738360 registrations against the already constructed
// actual A0h VFS manager. Borrowed manager, string pool and callbacks.
// This source interface is not the original binary or EH ABI.
void register_native_vfs_search_defaults_00738360(
    void* actual_manager, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks);
}
