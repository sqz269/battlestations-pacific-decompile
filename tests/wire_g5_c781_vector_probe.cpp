// Focused standalone native-reference probe; C781 report gives the Win32
// compile command. No game execution or relocated child-node calls.
#include <windows.h>
#include <float.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "bsp/gameplay_seg33_cc13.hpp"
#include "bsp/band_006b_cc13.hpp"

int main() {
    // Live bsp.py ghidra bytes 006E2220..006E22C4, /battlestationspacific.exe.
    // +48h stays null: the original's child-pose loop (and its relative call)
    // is never entered. Only the extrapolated +A4h..+ACh position is compared.
    const char hex[] =
        "83ec18d9811803000056d9442420d9c0"
        "decad9c9d95c2404d9811c030000d8c9"
        "d95c2408d88920030000d95c240cd981"
        "48040000d8442404d95c2410d9814c04"
        "0000d8442408d95c2414d98150040000"
        "d844240cd95c2418d9442410d999a400"
        "0000d9442414d999a8000000d9442418"
        "d999ac0000008b714885f6c681c80000"
        "0000c6810c010000007413eb038d4900"
        "8bcee899cad4ff8b764485f675f25e83"
        "c418c20400";
    const char helper_hex[] =
        "83ec0c8b442410d900d9442414d9c0de"
        "cad9c9d91c24d94004d8c9d95c2404d8"
        "4808d95c2408d901d80424d919d94424"
        "04d84104d95904d94108d8442408d959"
        "0883c40cc20800";
    constexpr std::size_t count = (sizeof(hex) - 1) / 2;
    constexpr std::size_t helper_count = (sizeof(helper_hex) - 1) / 2;
    constexpr std::size_t allocation_count = 0x100 + helper_count;
    auto* code = static_cast<unsigned char*>(VirtualAlloc(nullptr, allocation_count,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!code) return 2;
    const auto digit = [](char c) { return c <= '9' ? c - '0' : c - 'a' + 10; };
    for (std::size_t i = 0; i < count; ++i)
        code[i] = static_cast<unsigned char>((digit(hex[2 * i]) << 4) | digit(hex[2 * i + 1]));
    for (std::size_t i = 0; i < helper_count; ++i)
        code[0x100 + i] = static_cast<unsigned char>((digit(helper_hex[2 * i]) << 4) | digit(helper_hex[2 * i + 1]));
    DWORD previous;
    if (!VirtualProtect(code, allocation_count, PAGE_EXECUTE_READ, &previous)) return 3;
    FlushInstructionCache(GetCurrentProcess(), code, allocation_count);
    const auto native = reinterpret_cast<void (__fastcall*)(void*, void*, float)>(code);
    const auto helper_native = reinterpret_cast<void (__fastcall*)(float*, int, const float*, float)>(code + 0x100);
    unsigned int saved;
    _controlfp_s(&saved, 0, 0);
    unsigned int current;
    _controlfp_s(&current, _PC_24 | _MCW_EM, _MCW_PC | _MCW_EM);
    constexpr std::uint32_t edges[] = {0, 0x80000000, 1, 0x80000001,
        0x007fffff, 0x807fffff, 0x00800000, 0x80800000, 0x3f800000,
        0xbf800000, 0x7f7fffff, 0xff7fffff, 0x7f800000, 0xff800000,
        0x7fc12345, 0xffc12345, 0x7fa12345, 0xffa12345};
    std::uint32_t state = 0x781e0540;
    const auto next = [&]() {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return state;
    };
    unsigned failures = 0;
    unsigned helper_failures = 0;
    constexpr unsigned cases = 48000;
    for (unsigned i = 0; i < cases; ++i) {
        float values[7];
        for (unsigned j = 0; j < 7; ++j) {
            const auto random = next();
            const std::uint32_t bits = i < 8000 ? edges[random % _countof(edges)] : random;
            std::memcpy(values + j, &bits, 4);
        }
        if (i >= 16000) {
            const std::uint32_t base_nan = 0x7f800001u | (next() & 0x807fffffu);
            const std::uint32_t velocity_nan = 0x7f800001u | (next() & 0x807fffffu);
            std::memcpy(values + 1, &base_nan, 4);
            std::memcpy(values + 4, &velocity_nan, 4);
            values[6] = 1.0f;
        }
        float helper_expected[3], helper_actual[3];
        std::memcpy(helper_expected, values, 12);
        std::memcpy(helper_actual, values, 12);
        helper_native(helper_expected, 0, values + 3, values[6]);
        bsp::band_006b_cc13::vec3_add_scaled_in_place_006e0540(helper_actual, values + 3, values[6]);
        if (std::memcmp(helper_expected, helper_actual, 12) != 0) ++helper_failures;
        alignas(4) unsigned char record[0x454]{};
        std::memcpy(record + 0x448, values, 12);
        std::memcpy(record + 0x318, values + 3, 12);
        native(record, nullptr, values[6]);
        const bsp::gameplay_seg33_cc13::Seg33Vec3 base{values[0], values[1], values[2]};
        const bsp::gameplay_seg33_cc13::Seg33Vec3 velocity{values[3], values[4], values[5]};
        const auto actual = bsp::gameplay_seg33_cc13::projectile_extrapolate_position_006e2220(
            base, velocity, values[6]);
        if (std::memcmp(record + 0xa4, &actual, 12) != 0) {
            if (failures < 5) {
                std::uint32_t input_bits[7], expected_bits[3], actual_bits[3];
                std::memcpy(input_bits, values, sizeof(input_bits));
                std::memcpy(expected_bits, record + 0xa4, sizeof(expected_bits));
                std::memcpy(actual_bits, &actual, sizeof(actual_bits));
                std::printf("input %08x %08x %08x / %08x %08x %08x t=%08x expected=%08x,%08x,%08x actual=%08x,%08x,%08x\n",
                    input_bits[0], input_bits[1], input_bits[2], input_bits[3], input_bits[4], input_bits[5], input_bits[6],
                    expected_bits[0], expected_bits[1], expected_bits[2], actual_bits[0], actual_bits[1], actual_bits[2]);
            }
            ++failures;
        }
    }
    _controlfp_s(&current, saved, _MCW_PC | _MCW_EM);
    VirtualFree(code, 0, MEM_RELEASE);
    std::printf("C781 original 006E2220 vs wired extrapolator at PC24: %u cases, %u bitwise mismatches\n",
        cases, failures);
    std::printf("C781 original 006E0540 vs reconstructed helper at PC24: %u cases, %u bitwise mismatches\n",
        cases, helper_failures);
    return failures || helper_failures ? 1 : 0;
}
