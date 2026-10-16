#include "bsp/native_tagged_leaf_b3_01.hpp"

#include <cstring>

namespace bsp {

// memcpy expresses the observed unaligned x86 reads without assigning a C++
// class, field type, or lifetime to unknown Original backing. /O2 emits each
// entire entry as its Original MOV/LEA and RET; see the packet's COFF receipt.
std::uint32_t __fastcall native_tagged_b3_read_dword_00b34300(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, base, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34310(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint8_t __fastcall native_tagged_b3_read_byte_0c_00b34320(const void* base) noexcept {
    return static_cast<const std::uint8_t*>(base)[0x0C];
}

std::uint32_t __fastcall native_tagged_b3_read_dword_10_00b34330(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x10, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_address_04_00b34340(std::uint32_t base) noexcept {
    return base + 0x04U;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_20_00b34350(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x20, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_14_00b34360(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x14, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_address_18_00b34370(std::uint32_t base) noexcept {
    return base + 0x18U;
}

} // namespace bsp
