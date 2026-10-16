#include "bsp/native_tagged_leaf_b3_05.hpp"

#include <cstring>

namespace bsp {

static_assert(sizeof(void*) == sizeof(std::uint32_t), "Original address arithmetic requires Win32");

std::uint32_t __stdcall native_tagged_b3_zero_pop04_00b32910(std::uint32_t) noexcept {
    return 0;
}

std::uint32_t __fastcall native_tagged_b3_write_dword_08_00b33a90(void* base, std::uint32_t value) noexcept {
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &value, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_18_00b33e60(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x18, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_1c_00b33e70(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x1C, sizeof(value));
    return value;
}

std::uint8_t __cdecl native_tagged_b3_zero_byte_00b33e80() noexcept {
    return 0;
}

void __cdecl native_tagged_b3_noop_00b33f30() noexcept {
}

void __cdecl native_tagged_b3_noop_00b33f40() noexcept {
}

void __stdcall native_tagged_b3_noop_pop08_00b34070(std::uint32_t, std::uint32_t) noexcept {
}

std::uint32_t __stdcall native_tagged_b3_zero_pop08_00b34080(std::uint32_t, std::uint32_t) noexcept {
    return 0;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34550(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 4, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_child_dword_04_00b34560(const void* owner) noexcept {
    std::uint32_t child_bits;
    std::uint32_t value;
    std::memcpy(&child_bits, static_cast<const std::uint8_t*>(owner) + 0x24, sizeof(child_bits));
    const auto child_value_address = static_cast<std::uintptr_t>(child_bits + 4U);
    std::memcpy(&value, reinterpret_cast<const void*>(child_value_address), sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_child_scaled_address_00b347c0(const void* owner, std::uint32_t index) noexcept {
    std::uint32_t child_bits;
    std::uint32_t base_word;
    std::memcpy(&child_bits, static_cast<const std::uint8_t*>(owner) + 0x24, sizeof(child_bits));
    std::memcpy(&base_word, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(child_bits)), sizeof(base_word));
    return base_word + index * 8U;
}

} // namespace bsp
