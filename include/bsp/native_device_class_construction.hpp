#pragma once

#include <cstdint>

namespace bsp {
struct NativeDamageableClassConstructionAccess;

// Actual stable vtable word identities. This descriptor supplies no fabricated
// virtual methods and owns no class storage. Keep it outside constructed bytes.
struct NativeDeviceClassConstructionAccess {
    const NativeDamageableClassConstructionAccess& damageable;
    const std::uint32_t actual_gun_vtable_00ce4534;
    const std::uint32_t actual_bomb_vtable_00ce4560;
    const std::uint32_t actual_multibomb_vtable_00ce459c;
    const std::uint32_t actual_rapid_fixed_slave_vtable_00ce45e0;
    const std::uint32_t actual_rapid_turning_vtable_00ce4614;
    const std::uint32_t actual_single_turning_vtable_00ce4648;
    const std::uint32_t actual_depth_charge_vtable_00ce467c;
    const std::uint32_t actual_catapult_vtable_00ce46c0;
};

// Complete ordinary constructor bodies. Original ABI: ECX actual class,
// no stacked arguments, plain RET, EAX returns that same class. No allocation
// of the class itself is performed. Compose the genuine damageable base first,
// retain its exception behavior, then perform only the native ordered writes.
void* construct_native_gun_class_00442b90(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_bomb_platform_00442c50(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_multibomb_platform_00442ce0(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_rapid_fixed_slave_gun_00442d80(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_rapid_turning_gun_00442e20(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_single_turning_gun_00442ec0(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_depth_charge_launcher_00442f50(
    void* actual_class, const NativeDeviceClassConstructionAccess&);
void* construct_native_catapult_00442ff0(
    void* actual_class, const NativeDeviceClassConstructionAccess&);

// New MSVC Win32 C++ interfaces, not original ABI/FH3 bridges. The base owns
// its native sentinel allocation/cleanup schedule. Derived bodies initialize
// neither their full allocation nor the class id at+6Ch. Supplied vtable words
// must identify genuine dispatch tables when subsequent consumers use them;
// storing these identities does not reconstruct their absent readers/destructors.
// Hardware-fault recovery and native exception identity are not supplied.
} // namespace bsp
