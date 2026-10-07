#include "bsp/native_unit_group_storage.hpp"

namespace bsp {
namespace {
// Native storage has aligned 32-bit fields. Volatile accesses preserve the
// observed single constant capture and store ordering on the Win32 target.
void store_word(unsigned char* destination, std::uint32_t bits) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(destination) = bits;
}
} // namespace

void* construct_native_unit_group_0070dab0(
    void* receiver, const volatile std::uint32_t& native_cf4888_bits) noexcept {
    // 70DAB0 MOVSS reads CF4888 before the first receiver write at 70DAC1.
    const std::uint32_t speed_bits = native_cf4888_bits;
    auto* const group = static_cast<unsigned char*>(receiver);
    store_word(group + 0x04, 0);
    store_word(group + 0x08, 0);
    store_word(group + 0x0c, 0);
    *reinterpret_cast<volatile unsigned char*>(group + 0x10) = 0;
    store_word(group, 0x00cfd6f8); // Actual instruction 70DACE, raw profile.

    for (unsigned member = 0; member != 24; ++member) {
        auto* const record = group + 0x18 + member * 0x34;
        store_word(record, 0);
        store_word(record + 0x30, speed_bits);
        for (unsigned column = 0; column != 4; ++column) {
            // 70DAF0 axial precedes 70DAF5 lateral for each column.
            store_word(record + 0x20 + column * 4, 0);
            store_word(record + 0x10 + column * 4, 0);
        }
    }
    store_word(group + 0x14, 0);
    store_word(group + 0x500, 0);
    store_word(group + 0x4f8, 0);
    return receiver;
}
} // namespace bsp
