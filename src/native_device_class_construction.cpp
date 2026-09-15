#include "bsp/native_device_class_construction.hpp"
#include "bsp/native_damageable_class_construction.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native device class construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void write(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
} // namespace

void* construct_native_gun_class_00442b90(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_damageable_class_0087c640(actual_class, access.damageable);
    write(actual_class, 0x70, 0);
    write(actual_class, 0, access.actual_gun_vtable_00ce4534);
    write(actual_class, 0x74, 0);
    write(actual_class, 0x78, 0);
    write(actual_class, 0x7c, 0);
    write(actual_class, 0x9c, 0);
    write(actual_class, 0xa0, 0);
    write(actual_class, 0xa4, 0);
    write(actual_class, 0xbc, 0);
    write(actual_class, 0xc0, 0);
    write(actual_class, 0xc4, 0);
    return actual_class;
}

void* construct_native_bomb_platform_00442c50(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_bomb_vtable_00ce4560);
    write(actual_class, 0x80, 10);
    return actual_class;
}

void* construct_native_multibomb_platform_00442ce0(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_multibomb_vtable_00ce459c);
    write(actual_class, 0x80, 10);
    return actual_class;
}

void* construct_native_rapid_fixed_slave_gun_00442d80(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_rapid_fixed_slave_vtable_00ce45e0);
    return actual_class;
}

void* construct_native_rapid_turning_gun_00442e20(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_rapid_turning_vtable_00ce4614);
    return actual_class;
}

void* construct_native_single_turning_gun_00442ec0(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_single_turning_vtable_00ce4648);
    return actual_class;
}

void* construct_native_depth_charge_launcher_00442f50(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_depth_charge_vtable_00ce467c);
    return actual_class;
}

void* construct_native_catapult_00442ff0(
    void* actual_class, const NativeDeviceClassConstructionAccess& access) {
    construct_native_gun_class_00442b90(actual_class, access);
    write(actual_class, 0, access.actual_catapult_vtable_00ce46c0);
    write(actual_class, 0x80, 11);
    return actual_class;
}
} // namespace bsp
