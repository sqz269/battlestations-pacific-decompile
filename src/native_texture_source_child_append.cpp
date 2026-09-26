#include "bsp/native_texture_source_child_append.hpp"
#include "bsp/native_cube_texture_owner_array_reserve.hpp"

#include <cstdint>
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture-source child append requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
} // namespace

void append_native_texture_source_child_00c307ad_fragment(
    NativeTextureSourcePayload& payload, void* captured_child) {
    using Pointer = void*;
    volatile auto& actual = payload.children_08;
    const auto capacity = actual.capacity_08;
    if (actual.count_04 == capacity) {
        const auto doubled = signed_bits(static_cast<std::uint32_t>(capacity) * 2u);
        reserve_native_cube_texture_owner_array_00735ff0(
            payload.children_08, doubled > 1 ? doubled : 1);
    }
    const auto count = static_cast<std::uint32_t>(actual.count_04);
    const auto data = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual.data_00));
    const auto destination = data + count * 4u;
    if (destination != 0) {
        auto* const live = ::new (reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(destination))) Pointer;
        *static_cast<volatile Pointer*>(live) = captured_child;
    }
    actual.count_04 = signed_bits(static_cast<std::uint32_t>(actual.count_04) + 1u);
}

} // namespace bsp
