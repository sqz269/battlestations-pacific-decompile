#include "bsp/native_logical_buffer_device_save.hpp"
#include "bsp/native_physical_buffer_access.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical buffer device save requires MSVC Win32 DWORD accesses.
#endif
#pragma function(memcpy)

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::size_t) == 4);

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t load_word(const void* address) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
void store_word(void* address, std::uint32_t value) noexcept {
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(load_word(at(base, offset)));
}
using Profile = const volatile std::uint32_t*;
Profile profile(const void* physical,
    const NativeLogicalBufferDeviceSaveProfiles& profiles) noexcept {
    const auto token = load_word(physical);
    if (token == 0x00d61e10u) return profiles.private_index_00d61e10;
    if (token == 0x00d61e34u) return profiles.private_vertex_00d61e34;
    if (token == 0x00d61e58u) return profiles.pooled_index_00d61e58;
    if (token == 0x00d61e7cu) return profiles.pooled_vertex_00d61e7c;
    __assume(0); // Other physical profiles have no recovered binding here.
}
void* get_com(void* physical, Profile table) noexcept {
    const auto target = table[7];
    if (target == 0x00b4b840u)
        return native_physical_index_buffer_get_com_00b4b840(physical);
    if (target == 0x00b4b9f0u)
        return native_physical_vertex_buffer_get_com_00b4b9f0(physical);
    __assume(0);
}
std::uint32_t capacity(void* physical, Profile table) noexcept {
    const auto target = table[6];
    if (target == 0x00b4b800u)
        return native_physical_index_buffer_capacity_00b4b800(physical);
    if (target == 0x00b4b9b0u)
        return native_physical_vertex_buffer_capacity_00b4b9b0(physical);
    __assume(0);
}
void* lock(void* physical, Profile captured_table,
    NativePhysicalBufferLockContext& context, std::uint32_t bytes,
    std::uint32_t* base_offset) {
    const auto target = captured_table[2];
    if (target == 0x00b4b850u)
        return lock_native_physical_index_buffer_00b4b850(
            physical, context, bytes, 0, 1, base_offset, 1);
    if (target == 0x00b4ba00u)
        return lock_native_physical_vertex_buffer_00b4ba00(
            physical, context, bytes, 0, 1, base_offset, 1);
    __assume(0);
}
void unlock(void* physical, Profile table) {
    const auto target = table[3];
    if (target == 0x00b4b820u) {
        unlock_native_physical_index_buffer_00b4b820(physical);
        return;
    }
    if (target == 0x00b4b9d0u) {
        unlock_native_physical_vertex_buffer_00b4b9d0(physical);
        return;
    }
    __assume(0);
}
using ComReference = unsigned long (__stdcall*)(void*);

void save(void* logical, NativeLogicalBufferDeviceSaveContext& context,
    std::uint32_t physical_offset, std::uint32_t flags_offset,
    std::uint32_t shadow_offset, bool index) {
    if ((load_word(at(logical, flags_offset)) & 0xf000u) == 0x1000u) return;
    void* physical = pointer(logical, physical_offset);
    if (!physical) return;
    if (!get_com(physical, profile(physical, context.actual_physical_profiles))) return;

    physical = pointer(logical, physical_offset);
    void* const com = get_com(physical, profile(physical, context.actual_physical_profiles));
    if (com) {
        const auto add_ref = reinterpret_cast<ComReference>(
            load_word(at(pointer(com), 4)));
        (void)add_ref(com);
        const auto release = reinterpret_cast<ComReference>(
            load_word(at(pointer(com), 8)));
        (void)release(com);
    }

    physical = pointer(logical, physical_offset);
    const auto allocation_bytes = capacity(physical,
        profile(physical, context.actual_physical_profiles));
    void* const shadow = singleton_lifetime_allocate({
        SingletonAllocationKind::object, allocation_bytes, allocation_bytes});
    // Native reloads the physical owner before publishing the new shadow,
    // then captures that owner's table after publication and base=0.
    physical = pointer(logical, physical_offset);
    store_word(at(logical, shadow_offset), reinterpret_cast<std::uint32_t>(shadow));
    std::uint32_t base_offset = 0;
    const auto captured_table = profile(physical, context.actual_physical_profiles);
    const auto lock_bytes = capacity(physical, captured_table);
    physical = pointer(logical, physical_offset);
    void* const data = lock(physical, captured_table,
        context.actual_physical_lock, lock_bytes, &base_offset);

    physical = pointer(logical, physical_offset);
    const auto copy_bytes = capacity(physical,
        profile(physical, context.actual_physical_profiles));
    std::memcpy(pointer(logical, shadow_offset), data, copy_bytes);
    physical = pointer(logical, physical_offset);
    unlock(physical, profile(physical, context.actual_physical_profiles));
    physical = pointer(logical, physical_offset);
    if (index) release_native_physical_index_buffer_for_reset_00b23180(physical);
    else release_native_physical_vertex_buffer_for_reset_00b23270(physical);
}
} // namespace

void __fastcall save_native_logical_vertex_buffer_for_device_reset_00b49d00(
    void* logical, NativeLogicalBufferDeviceSaveContext& context) {
    save(logical, context, 0x58, 0x60, 0x6c, false);
}
void __fastcall save_native_logical_index_buffer_for_device_reset_00b49f80(
    void* logical, NativeLogicalBufferDeviceSaveContext& context) {
    save(logical, context, 8, 0x10, 0x1c, true);
}
} // namespace bsp
