#include "bsp/native_tagged_leaf_b3_02.hpp"

#include <cstring>

namespace bsp {

// memcpy retains unaligned raw backing without inventing a class layout.
// The strict Win32 actual-object proof checks the entire MOV/RET bodies.
std::uint32_t __fastcall native_tagged_b3_read_dword_84_00b343d0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x84, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34450(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34460(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34470(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34480(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x04, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_00b344a0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, base, sizeof(value));
    return value;
}

std::uint32_t __cdecl native_tagged_b3_return_0a_00b34500() noexcept {
    return 0x0AU;
}

std::uint32_t __cdecl native_tagged_b3_return_06_00b34510() noexcept {
    return 0x06U;
}

std::uint32_t __cdecl native_tagged_b3_return_10_00b34520() noexcept {
    return 0x10U;
}

} // namespace bsp
