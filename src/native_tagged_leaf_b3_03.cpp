#include "bsp/native_tagged_leaf_b3_03.hpp"

#include <cstring>

namespace bsp {

// Each complete executable COMDAT must equal the corresponding whole Native
// body, including its RET. The proof rejects prefixes, tails and helper code.
std::uint32_t __fastcall native_tagged_b3_read_dword_18_00b3cd00(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x18, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_30_00b3ce90(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x30, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_24_00b3d090(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x24, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_28_00b3d0a0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x28, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_2c_00b3d0b0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x2C, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d100(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d380(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d390(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, base, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d3c0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d3d0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, base, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d400(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d410(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, base, sizeof(value));
    return value;
}

} // namespace bsp
