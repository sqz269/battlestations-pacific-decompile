#include "bsp/native_logical_buffer_device_restore.hpp"
#include "bsp/native_physical_buffer_access.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_resource_creation_flags.hpp"
#include "bsp/native_shader_device_reset.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical buffer device restore requires MSVC Win32 DWORD accesses.
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
    const NativeLogicalBufferDeviceRestoreProfiles& profiles) noexcept {
    const auto token = load_word(physical);
    if (token == 0x00d61e10u) return profiles.private_index_00d61e10;
    if (token == 0x00d61e34u) return profiles.private_vertex_00d61e34;
    if (token == 0x00d61e58u) return profiles.pooled_index_00d61e58;
    if (token == 0x00d61e7cu) return profiles.pooled_vertex_00d61e7c;
    __assume(0); // No recovered binding for any other physical profile.
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
void attach(std::uint32_t target, void* physical, void* com,
    std::uint32_t flags, std::uint32_t bytes,
    NativePhysicalBufferOwnerContext& context) {
    if (target == 0x00b4c250u) {
        attach_native_physical_index_buffer_00b4c250(physical,
            static_cast<IDirect3DIndexBuffer9*>(com), flags, bytes, context);
        return;
    }
    if (target == 0x00b4c370u) {
        attach_native_physical_vertex_buffer_00b4c370(physical,
            static_cast<IDirect3DVertexBuffer9*>(com), flags, bytes, context);
        return;
    }
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
using CreateBuffer = HRESULT (STDMETHODCALLTYPE*)(void*, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, void**, void*);
using ComReference = ULONG (STDMETHODCALLTYPE*)(void*);
void release_current_temporary(void** temporary) {
    // Attach can change the escaped Create output cell. Read it again and
    // unconditionally dereference its current COM table, even when null.
    void* const com = pointer(temporary);
    const auto release = reinterpret_cast<ComReference>(load_word(at(pointer(com), 8)));
    (void)release(com);
}
bool needs_restore(void* logical, std::uint32_t physical_offset,
    std::uint32_t flags_offset, NativeLogicalBufferDeviceRestoreContext& context) {
    if ((load_word(at(logical, flags_offset)) & 0xf000u) == 0x1000u) return false;
    void* const physical = pointer(logical, physical_offset);
    if (!physical) return false;
    return get_com(physical, profile(physical, context.actual_physical_profiles)) == nullptr;
}
void copy_and_finish(void* logical, std::uint32_t physical_offset,
    std::uint32_t shadow_offset, std::uint32_t& base_offset,
    NativeLogicalBufferDeviceRestoreContext& context) {
    void* physical = pointer(logical, physical_offset);
    base_offset = 0;
    const auto captured_table = profile(physical, context.actual_physical_profiles);
    const auto lock_bytes = capacity(physical, captured_table);
    physical = pointer(logical, physical_offset);
    void* const destination = lock(physical, captured_table,
        context.actual_physical_lock, lock_bytes, &base_offset);
    physical = pointer(logical, physical_offset);
    const auto copy_bytes = capacity(physical,
        profile(physical, context.actual_physical_profiles));
    std::memcpy(destination, pointer(logical, shadow_offset), copy_bytes);
    physical = pointer(logical, physical_offset);
    unlock(physical, profile(physical, context.actual_physical_profiles));
    singleton_lifetime_free(pointer(logical, shadow_offset));
    // The returning-free continuation is part of both complete native bodies.
    store_word(at(logical, shadow_offset), 0);
}
std::uint32_t index_stride(std::uint32_t format) noexcept {
    if (format == 0x65u) return 2;
    if (format == 0x66u) return 4;
    return 0;
}
} // namespace

void __fastcall restore_native_logical_vertex_buffer_after_device_reset_00b49dc0(
    void* logical, NativeLogicalBufferDeviceRestoreContext& context) {
    if (!needs_restore(logical, 0x58, 0x60, context)) return;
    auto flags = load_word(at(logical, 0x60));
    std::uint32_t pool_and_base;
    switch (flags & 0x0fu) {
    case 0: pool_and_base = 0; flags |= 0x10000u; break;
    case 1: pool_and_base = 1; break;
    case 2: pool_and_base = 2; break;
    case 3: pool_and_base = 3; break;
    default: pool_and_base = context.native_pool_stack_bits; break;
    }
    std::uint32_t usage = (flags & 0x10u) != 0 ? 1u : 0u;
    switch (flags & 0x0f00u) {
    case 0x0100: usage |= 0x0002; break;
    case 0x0200: usage |= 0x4000; break;
    case 0x0300: usage |= 0x0040; break;
    case 0x0400: usage |= 0x0100; break;
    case 0x0500: usage |= 0x0080; break;
    }
    if ((flags & 0x0f000u) == 0x01000u) usage |= 0x200u;
    if ((flags & 0x0f0000u) == 0x010000u) usage |= 8u;

    const void* const renderer = context.actual_renderer_00f8d394;
    void* temporary = nullptr;
    void* const device = get_native_renderer_device_00b1fef0(renderer);
    const void* const device_table = pointer(device);
    const auto stride = load_word(at(pointer(logical, 0x68), 0xcc));
    const auto count = load_word(at(logical, 0x64));
    const auto bytes = stride * count;
    const auto create = reinterpret_cast<CreateBuffer>(load_word(at(device_table, 0x68)));
    (void)create(device, bytes, usage, 0, pool_and_base, &temporary, nullptr);

    const auto current_stride = load_word(at(pointer(logical, 0x68), 0xcc));
    const auto current_count = load_word(at(logical, 0x64));
    const auto current_bytes = current_stride * current_count;
    void* const physical = pointer(logical, 0x58);
    const auto table = profile(physical, context.actual_physical_profiles);
    const auto target = table[5];
    const auto current_flags = load_word(at(logical, 0x60));
    void* const current_temporary = pointer(&temporary);
    attach(target, physical, current_temporary, current_flags, current_bytes,
        context.actual_physical_attachment);
    release_current_temporary(&temporary);
    // The native vertex entry reuses its pool-local cell as the Lock base.
    copy_and_finish(logical, 0x58, 0x6c, pool_and_base, context);
}

void __fastcall restore_native_logical_index_buffer_after_device_reset_00b4a040(
    void* logical, NativeLogicalBufferDeviceRestoreContext& context) {
    if (!needs_restore(logical, 8, 0x10, context)) return;
    const auto flags = load_word(at(logical, 0x10));
    std::uint32_t usage;
    std::uint32_t pool;
    if ((flags & 0x0fu) > 3) pool = context.native_pool_stack_bits;
    translate_native_resource_creation_flags_00b20a80(&usage, &pool, flags, 7);
    const void* const renderer = context.actual_renderer_00f8d394;
    void* temporary = nullptr;
    void* const device = get_native_renderer_device_00b1fef0(renderer);
    const auto format = load_word(at(logical, 0x18));
    const auto stride = index_stride(format);
    const void* const device_table = pointer(device);
    const auto current_pool = load_word(&pool);
    const auto current_usage = load_word(&usage);
    const auto count = load_word(at(logical, 0x14));
    const auto bytes = count * stride;
    const auto create = reinterpret_cast<CreateBuffer>(load_word(at(device_table, 0x6c)));
    (void)create(device, bytes, current_usage, format, current_pool, &temporary, nullptr);

    const auto current_stride = index_stride(load_word(at(logical, 0x18)));
    const auto current_count = load_word(at(logical, 0x14));
    void* const physical = pointer(logical, 8);
    const auto current_bytes = current_count * current_stride;
    const auto current_flags = load_word(at(logical, 0x10));
    const auto table = profile(physical, context.actual_physical_profiles);
    const auto target = table[5];
    void* const current_temporary = pointer(&temporary);
    attach(target, physical, current_temporary, current_flags, current_bytes,
        context.actual_physical_attachment);
    release_current_temporary(&temporary);
    std::uint32_t base_offset;
    copy_and_finish(logical, 8, 0x1c, base_offset, context);
}
} // namespace bsp
