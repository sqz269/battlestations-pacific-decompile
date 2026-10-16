#include "bsp/native_tagged_leaf_b3_06.hpp"

#include <cstring>

namespace bsp {

static_assert(sizeof(void*) == sizeof(std::uint32_t), "Original address arithmetic requires Win32");

std::uint32_t __fastcall native_tagged_b3_read_dword_14_00b33e50(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x14, sizeof(value));
    return value;
}

std::uint8_t __fastcall native_tagged_b3_write_byte_74_00b343b0(void* base, std::uint8_t value) noexcept {
    std::memcpy(static_cast<std::uint8_t*>(base) + 0x74, &value, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_write_dword_84_00b343e0(void* base, std::uint32_t value) noexcept {
    std::memcpy(static_cast<std::uint8_t*>(base) + 0x84, &value, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_identity_00b3ff30(std::uint32_t bits) noexcept {
    return bits;
}

std::uint32_t __fastcall native_tagged_b3_address_10_00b3ccf0(std::uint32_t bits) noexcept {
    return bits + 0x10U;
}

std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344c0(void* base) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(base, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 4, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &zero, sizeof(zero));
    return zero;
}

std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344d0(void* base) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(base, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 4, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &zero, sizeof(zero));
    return zero;
}

std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344e0(void* base) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(base, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 4, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &zero, sizeof(zero));
    return zero;
}

std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b344f0(void* base) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(base, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 4, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &zero, sizeof(zero));
    return zero;
}

std::uint32_t __fastcall native_tagged_b3_zero_three_dwords_00b3d110(void* base) noexcept {
    const std::uint32_t zero = 0;
    std::memcpy(base, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 4, &zero, sizeof(zero));
    std::memcpy(static_cast<std::uint8_t*>(base) + 8, &zero, sizeof(zero));
    return zero;
}

void __cdecl native_tagged_b3_noop_00b3c4f0() noexcept {
}

void __cdecl native_tagged_b3_noop_00b3c500() noexcept {
}

void __cdecl native_tagged_b3_noop_00b3c510() noexcept {
}

void __cdecl native_tagged_b3_noop_00b3dc80() noexcept {
}

void __stdcall native_tagged_b3_noop_pop04_00b3d4c0(std::uint32_t) noexcept {
}

void __stdcall native_tagged_b3_noop_pop04_00b3d4d0(std::uint32_t) noexcept {
}

void __stdcall native_tagged_b3_noop_pop04_00b3dc90(std::uint32_t) noexcept {
}

std::uint8_t __cdecl native_tagged_b3_zero_byte_00b3d4e0() noexcept {
    return 0;
}

std::uint8_t __cdecl native_tagged_b3_zero_byte_00b3d500() noexcept {
    return 0;
}

__declspec(naked) void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e380(void*) noexcept {
    __asm {
        add dword ptr [ecx + 4], -1
        ret
    }
}

__declspec(naked) void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e500(void*) noexcept {
    __asm {
        add dword ptr [ecx + 4], -1
        ret
    }
}

__declspec(naked) void __fastcall native_tagged_b3_add_minus_one_dword_04_00b3e680(void*) noexcept {
    __asm {
        add dword ptr [ecx + 4], -1
        ret
    }
}

} // namespace bsp
