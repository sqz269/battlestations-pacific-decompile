#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual registry+4 array header. The data word addresses DWORD class-pointer
// slots; this layer neither owns nor retains/releases the pointed-to classes.
struct NativeDeviceRegistryArrayStorage {
    std::uint32_t data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(NativeDeviceRegistryArrayStorage) == 0x0c);
static_assert(offsetof(NativeDeviceRegistryArrayStorage, data_00) == 0);
static_assert(offsetof(NativeDeviceRegistryArrayStorage, count_04) == 4);
static_assert(offsetof(NativeDeviceRegistryArrayStorage, capacity_08) == 8);

// Complete 0043FA60..0043FABE (95 bytes). Native ECX=actual header, stacked
// signed capacity, RET4; no stable result. Clamp request to one, compare
// capacity signed, allocate wrapped DWORD(request*4), copy current DWORD slots,
// free the current data word, then publish replacement data and capacity.
void reserve_native_device_registry_array_0043fa60(
    NativeDeviceRegistryArrayStorage&, std::int32_t capacity);

// Complete 00440180..004401CF (80 bytes). Native ECX=actual header, stacked
// signed count, RET4; no stable result. Growth reserves and zeroes each reached
// slot. Shrink decrements the published count only; it does not clear slots or
// release the class pointers. The requested count is written at the end.
void resize_native_device_registry_array_00440180(
    NativeDeviceRegistryArrayStorage&, std::int32_t count);

// New MSVC Win32 source interfaces, not original ABI shims. Borrow the actual
// registry header and preserve signed comparisons, DWORD wrap, current-header
// reloads, and reached null-destination tests. Every reached nonnull address
// must name backed memory. No overflow/storage validation or recovery is added;
// original CRT allocation, exception, and hardware-fault identity are unproved.

} // namespace bsp
