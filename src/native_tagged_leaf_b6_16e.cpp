#include "bsp/native_tagged_leaf_b6_16e.hpp"

#include <cstring>

static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

// Integer address arithmetic retains x86 address wrap without adding a typed
// receiver, field alignment or a whole-object pointer-arithmetic requirement.
extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b663f0(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0xcu;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" std::int32_t __fastcall bsp_tagged_b6_16e_00b6bdd0(const void* storage) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(storage);
    std::uint32_t begin;
    std::memcpy(&begin, reinterpret_cast<const void*>(address + 4u), sizeof begin);
    if (begin == 0u) {
        return 0;
    }
    std::uint32_t end;
    std::memcpy(&end, reinterpret_cast<const void*>(address + 8u), sizeof end);
    // Native SUB wraps first, then the signed multiply/shift sequence divides
    // the resulting int32 value by twelve, truncating toward zero.
    const std::uint32_t difference = end - begin;
    std::int32_t signed_difference;
    std::memcpy(&signed_difference, &difference, sizeof signed_difference);
    return signed_difference / 12;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b6d980(const void* storage) noexcept {
    std::uint32_t value;
    const auto address = reinterpret_cast<std::uintptr_t>(storage) + 0x38u;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}

extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b6fc60(std::uint32_t address_bits) noexcept {
    return address_bits + 0x1acu;
}
