#include "bsp/native_tagged_leaf_b3_04.hpp"

#include <cstring>

namespace bsp {

// These three complete semantic entries preserve uint32 arithmetic and actual
// +4-then-+0 read order on valid stable backing. Their emitted volatile-register
// and EFLAGS effects differ from Original; see the per-entry audit qualification.
std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e370(const void* record) noexcept {
    std::uint32_t scale_word;
    std::uint32_t base_word;
    std::memcpy(&scale_word, static_cast<const std::uint8_t*>(record) + 4, sizeof(scale_word));
    std::memcpy(&base_word, record, sizeof(base_word));
    return base_word + scale_word * 4U - 4U;
}

std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e4f0(const void* record) noexcept {
    std::uint32_t scale_word;
    std::uint32_t base_word;
    std::memcpy(&scale_word, static_cast<const std::uint8_t*>(record) + 4, sizeof(scale_word));
    std::memcpy(&base_word, record, sizeof(base_word));
    return base_word + scale_word * 4U - 4U;
}

std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e670(const void* record) noexcept {
    std::uint32_t scale_word;
    std::uint32_t base_word;
    std::memcpy(&scale_word, static_cast<const std::uint8_t*>(record) + 4, sizeof(scale_word));
    std::memcpy(&base_word, record, sizeof(base_word));
    return base_word + scale_word * 4U - 4U;
}

std::uint32_t __fastcall native_tagged_b3_identity_00b3ff40(std::uint32_t bits) noexcept {
    return bits;
}

std::uint32_t __fastcall native_tagged_b3_identity_00b3ff50(std::uint32_t bits) noexcept {
    return bits;
}

std::uint32_t __fastcall native_tagged_b3_identity_00b3ff60(std::uint32_t bits) noexcept {
    return bits;
}

std::uint32_t __fastcall native_tagged_b3_address_78_00b3ff90(std::uint32_t bits) noexcept {
    return bits + 0x78U;
}

std::uint8_t __fastcall native_tagged_b3_read_byte_0e_00b3ffb0(const void* base) noexcept {
    return static_cast<const std::uint8_t*>(base)[0x0E];
}

std::uint32_t __fastcall native_tagged_b3_read_dword_34_00b3ffc0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x34, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_170_00b3ffd0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x170, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_190_00b3ffe0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x190, sizeof(value));
    return value;
}

std::uint32_t __fastcall native_tagged_b3_read_dword_08_00b3fff0(const void* base) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + 0x08, sizeof(value));
    return value;
}

} // namespace bsp
