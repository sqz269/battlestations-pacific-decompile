#pragma once

#include "bsp/native_device_registry_array.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual E17BF4 owner: deleting profile at +00 and the existing raw device
// class pointer-array header at +04/+08/+0C.
struct NativeDeviceRegistryStorage {
    std::uint32_t native_vtable_00;
    NativeDeviceRegistryArrayStorage classes_04;
};
static_assert(sizeof(NativeDeviceRegistryStorage) == 0x10);
static_assert(offsetof(NativeDeviceRegistryStorage, native_vtable_00) == 0);
static_assert(offsetof(NativeDeviceRegistryStorage, classes_04) == 4);

inline constexpr std::uint32_t kNativeDeviceRegistryDeletingProfile = 0x00ce44dcu;
inline constexpr std::uint32_t kNativeSingletonBaseProfile = 0x00ce3818u;

// Complete source behavior of 00441780[178]. Native entry consumes no input,
// returns EAX and uses RET. The source interface borrows the application's
// actual raw manager and registry publication cells. It captures manager+10,
// enters and increments its physical +18 counter, then rechecks the registry.
// A fresh raw10h owner is initialized and published before a second manager
// lookup and a current-publication registration through BD0C30. Both normal
// and exceptional exits release the first captured section; registration
// failure deliberately retains the published owner. The slow return reloads
// publication after release, while the fast return preserves its first read.
NativeDeviceRegistryStorage* get_native_device_registry_00441780(
    void* volatile& actual_manager_publication_01090aa0,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4);

// Complete 0043EBF0[17]. Clear the supplied actual publication before writing
// the CE3818 base profile. This is both the normal tail and the state0 unwind
// action of 00441360; it does not free storage or unregister the owner.
void unwind_native_device_registry_0043ebf0(
    NativeDeviceRegistryStorage& owner,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4) noexcept;

// Complete 00441360[94] and 00441840[30]. The nondeleting destructor resizes
// the +4 header to zero, frees its current data word, then clears publication
// and writes CE3818. Its state0 unwind performs the same final two stores.
// The scalar deleter frees only for flags bit0 and returns the captured owner.
// Neither body releases the borrowed class pointers or unregisters itself.
void destroy_native_device_registry_00441360(
    NativeDeviceRegistryStorage& owner,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4);
NativeDeviceRegistryStorage* delete_native_device_registry_00441840(
    NativeDeviceRegistryStorage* owner, std::uint32_t flags,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4);

// One process-static E17BF4 cell with explicit lifetime. The getter wrapper
// still receives the application's SAME actual manager publication. Canonical
// manager drain dispatches CE44DC owners through this cell.
NativeDeviceRegistryStorage* volatile& process_native_device_registry_00e17bf4() noexcept;
NativeDeviceRegistryStorage* get_process_native_device_registry_00441780(
    void* volatile& actual_manager_publication_01090aa0);

// New MSVC Win32 source interfaces, not original ABI shims. Numeric profile
// DWORDs are identity only and are never invoked as C++ vtables. The source
// exception scopes reproduce native C++ unwind effects; original FH3/SEH stack
// layout, hardware-fault cleanup and provider register identity remain open.

} // namespace bsp
