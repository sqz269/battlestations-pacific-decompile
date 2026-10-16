#include "bsp/native_tagged_sizes_bb_16j.hpp"

#include <cstring>

static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

// 00BB40E0: first is read once; a zero first skips the end load. Integer
// addresses and unsigned subtraction preserve both x86 wrap operations.
extern "C" std::int32_t __fastcall bsp_tagged_sizes_bb_16j_00bb40e0(const void* storage) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(storage);
    std::uint32_t first;
    std::memcpy(&first, reinterpret_cast<const void*>(address + 4u), sizeof first);
    if (first == 0u) return 0;
    std::uint32_t last;
    std::memcpy(&last, reinterpret_cast<const void*>(address + 8u), sizeof last);
    const std::uint32_t difference = last - first;
    std::int32_t signed_difference;
    std::memcpy(&signed_difference, &difference, sizeof signed_difference);
    return signed_difference / 36;
}

// 00BB41E0: the same ordered raw-word operation, with signed division by 20.
extern "C" std::int32_t __fastcall bsp_tagged_sizes_bb_16j_00bb41e0(const void* storage) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(storage);
    std::uint32_t first;
    std::memcpy(&first, reinterpret_cast<const void*>(address + 4u), sizeof first);
    if (first == 0u) return 0;
    std::uint32_t last;
    std::memcpy(&last, reinterpret_cast<const void*>(address + 8u), sizeof last);
    const std::uint32_t difference = last - first;
    std::int32_t signed_difference;
    std::memcpy(&signed_difference, &difference, sizeof signed_difference);
    return signed_difference / 20;
}

// 00BBA220: signed division by 24. No pointer subtraction, element object,
// range normalization, receiver guard, capacity read or external provider.
extern "C" std::int32_t __fastcall bsp_tagged_sizes_bb_16j_00bba220(const void* storage) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(storage);
    std::uint32_t first;
    std::memcpy(&first, reinterpret_cast<const void*>(address + 4u), sizeof first);
    if (first == 0u) return 0;
    std::uint32_t last;
    std::memcpy(&last, reinterpret_cast<const void*>(address + 8u), sizeof last);
    const std::uint32_t difference = last - first;
    std::int32_t signed_difference;
    std::memcpy(&signed_difference, &difference, sizeof signed_difference);
    return signed_difference / 24;
}
