#pragma once

#include "bsp/native_damageable_class_construction.hpp"

#include <cstddef>

namespace bsp {

// Existing vehicle-base allocation/derived-layout contract. The complete
// 00749050 body itself touches through +133h and preserves +134h..+137h.
inline constexpr std::size_t kNativeVehicleClassBaseStorageBytes = 0x138;

// Complete 226-byte 00749050. Original ECX=actual class, EAX=same class,
// bare RET. This ordinary MSVC Win32 C++ interface is not its register ABI.
//
// Borrow a fresh, writable, four-byte-aligned actual vehicle-class allocation
// of at least 138h bytes. Do not value-initialize it here or cast a semantic
// VehicleClassDescriptor to this layout. Its address and the real damageable
// access/string-publication domains must remain valid and stable throughout
// construction and child cleanup; access/publication storage must not overlap
// this allocation or the child's tree node. No class allocation, factory,
// registry, ownership transfer, model load, retain or release is added.
//
// Invoke complete 0087C640 first. If it throws, propagate its existing C++
// cleanup and partial state without any vehicle stores or outer allocation
// free. On return, perform all three profile writes and each observed field
// store in order; every other byte is preserved except for the child's own
// writes. Profile values are opaque identities, never callable addresses.
// No local cleanup state is armed in the Native caller. Hardware faults,
// private FS/FH3 behavior and Original ABI compatibility remain unproved.
void* construct_native_vehicle_class_base_00749050(void* actual_class,
    const NativeDamageableClassConstructionAccess& damageable);

} // namespace bsp
