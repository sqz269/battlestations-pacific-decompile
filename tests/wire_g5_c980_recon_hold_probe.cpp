// Focused regression for 00779D37's FADD before the hold-expiry comparison.
// Build as a Win32 probe against bsp_core.lib; see reports/wire-g5-c980.json.
#include "bsp/mp_seg43_cc13.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <float.h>

namespace {
template<class T>
void store(std::array<unsigned char, 0x304>& unit, std::size_t offset, T value) {
    std::memcpy(unit.data() + offset, &value, sizeof(value));
}
float from_bits(std::uint32_t bits) {
    float result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
struct PrecisionScope {
    unsigned int previous;
    explicit PrecisionScope(unsigned int precision) {
        unsigned int ignored;
        _controlfp_s(&previous, 0, 0);
        _controlfp_s(&ignored, precision, _MCW_PC);
    }
    ~PrecisionScope() {
        unsigned int ignored;
        _controlfp_s(&ignored, previous & _MCW_PC, _MCW_PC);
    }
};
}

int main() {
    alignas(4) std::array<unsigned char, 0x304> unit{};
    store<std::int32_t>(unit, 0x54, 0); // party zero selects recon party one.
    store<std::int32_t>(unit, 0x21c + 4, 1);
    store<std::int32_t>(unit, 0x2fc, 2);
    store(unit, 0x300, from_bits(0x42c83335u)); // 100.10001373291015625
    const float now = from_bits(0x4302199au);  // 130.100006103515625
    std::uint8_t* absent = nullptr;
    const std::int32_t zero = 0;
    const std::uint8_t off = 0;
    const bsp::cc13_seg43::MpGlobals globals{
        absent, absent, absent, zero, now, off, absent, absent, absent, nullptr,
        {zero, zero, absent, zero, absent}};
    int failures = 0;
    // The exact sum is 130.10001373291015625. PC24 rounds it DOWN to now:
    // FCOMIP/JA must expire the hold. PC53 retains the exact sum and the hold.
    for (const auto precision : {static_cast<unsigned int>(_PC_24),
                                 static_cast<unsigned int>(_PC_53)}) {
        PrecisionScope scope(precision);
        const int expected = precision == _PC_24 ? 1 : 2;
        const int actual = bsp::cc13_seg43::unit_recon_record_value_00779d00(
            unit.data(), globals);
        std::printf("PC%u expiry boundary: expected=%d actual=%d\n",
            precision == _PC_24 ? 24u : 53u, expected, actual);
        if (actual != expected) ++failures;
    }
    return failures;
}
