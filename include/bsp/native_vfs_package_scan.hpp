#pragma once

#include "bsp/native_render_alias_checked_ops.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
struct NativeVfsEnumerationContext;
struct NativeVfsMountRegistrationContext;

struct NativeVfsPackageScanContext {
    void* volatile& manager_0109ceec;
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsEnumerationContext& enumeration;
    NativeVfsMountRegistrationContext& mounting;
};

// Complete normal 0073CB10 scan of actual manager storage. No stack arguments,
// incoming ECX ignored, plain RET. The new C++ API borrows all services.
void scan_native_vfs_packages_0073cb10(NativeVfsPackageScanContext&);

// Complete normal path of 00557A90. ECX is the actual 0Ch list owner, stack
// argument is an actual 8h string header, EAX returns that header, RET4.
// The caller owns the resulting string and must release it through the same
// native pool. The list retains its 10h sentinel after the front node is
// removed. The native FH3 unwind handler is outside this source contract.
void* pop_native_vfs_package_name_00557a90(void* actual_list_owner,
    void* actual_output_string, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Complete 00BDB120. ECX actual A0h manager, stack actual 8h system-name
// header, EAX first matching provider or null, RET4. The current mount tree
// and provider strings are borrowed; no projection or cached registration.
void* find_native_vfs_mounted_system_name_00bdb120(void* actual_manager,
    const void* actual_system_name, const SingletonLifetimeCallbacks& invalid_parameters);

} // namespace bsp
