#include "bsp/native_texture_surface_cache_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture surface cache storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
// Explicit scalar accesses preserve the native load/store order, including
// aliases of the actual header. DWORD addresses wrap before each access.
std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
} // namespace

void reserve_native_texture_surface_cache_00b3d9b0(
    NativeTextureSurfaceCacheStorage& header, std::int32_t requested_capacity) {
    if (requested_capacity < 1) requested_capacity = 1;
    if (signed_bits(word(&header, 8)) >= requested_capacity) return;
    const auto bytes = static_cast<std::uint32_t>(requested_capacity) * 8u;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto destination = reinterpret_cast<std::uintptr_t>(replacement);
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(word(&header, 4))) {
        if (destination != 0) {
            const auto captured_data = word(&header);
            const auto source = captured_data + index * 8u;
            const auto first = word(pointer(source));
            put(pointer(destination), 0, first);
            const auto second = word(pointer(source), 4);
            put(pointer(destination), 4, second);
        }
        ++index;
        destination += 8u;
    }
    singleton_lifetime_free(pointer(word(&header)));
    // B3DA0A..B3DA12 continues after free despite the saved no-return analysis.
    put(&header, 0, reinterpret_cast<std::uintptr_t>(replacement));
    put(&header, 8, static_cast<std::uint32_t>(requested_capacity));
}

void resize_native_texture_surface_cache_00b3da20(
    NativeTextureSurfaceCacheStorage& header, std::int32_t requested_count) {
    if (requested_count > signed_bits(word(&header, 8)))
        reserve_native_texture_surface_cache_00b3d9b0(header, requested_count);
    auto index = word(&header, 4);
    while (signed_bits(index) < requested_count) {
        const auto destination = word(&header) + index * 8u;
        if (destination != 0) {
            put(pointer(destination), 0, 0);
            put(pointer(destination), 4, 0);
        }
        ++index;
    }
    while (requested_count < signed_bits(word(&header, 4)))
        put(&header, 4, word(&header, 4) - 1u);
    put(&header, 4, static_cast<std::uint32_t>(requested_count));
}

void destroy_native_texture_surface_cache_00b3ec40(
    NativeTextureSurfaceCacheStorage& header) {
    resize_native_texture_surface_cache_00b3da20(header, 0);
    singleton_lifetime_free(pointer(word(&header)));
    // B3EC52..B3EC56 returns without clearing data or capacity.
}
} // namespace bsp
