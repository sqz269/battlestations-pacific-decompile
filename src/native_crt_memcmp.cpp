#include "bsp/native_crt_memcmp.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = std::uint32_t;
using Int32 = std::int32_t;
using ReadableBytes = const volatile Byte*;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Uint32) == 4);
static_assert(sizeof(Int32) == 4);

// Addressed MSVC Win32 loads admit genuine, possibly unaligned byte storage.
// No typed uint16_t/uint32_t object or aliasing/alignment promise is invented.
__forceinline Uint32 read_dword(ReadableBytes address) noexcept {
    Uint32 value = 0;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}

__forceinline Uint32 read_word_zero_extended(ReadableBytes address) noexcept {
    Uint32 value = 0;
    __asm {
        mov eax, address
        movzx eax, word ptr [eax]
        mov value, eax
    }
    return value;
}

__forceinline Int32 normalized_difference(Byte first, Byte second) noexcept {
    if (first == second) {
        return 0;
    }
    return first < second ? -1 : 1;
}

// Four actual Original fallback schedules. CacheFirst applies only to lane0;
// other lanes always reread bytes. Each volatile payload read is sequenced.
template <bool CacheFirst, bool ReverseFirst, bool ReverseAll>
__forceinline Int32 compare_dword_fallback(
    ReadableBytes first, ReadableBytes second, Uint32 first_word) noexcept {
    for (Uint32 lane = 0; lane != 4U; ++lane) {
        Byte first_byte = static_cast<Byte>(first_word);
        Byte second_byte = 0;
        if constexpr (CacheFirst) {
            if (lane == 0U) {
                second_byte = second[lane];
            } else {
                first_byte = first[lane];
                second_byte = second[lane];
            }
        } else if constexpr (ReverseAll) {
            second_byte = second[lane];
            first_byte = first[lane];
        } else if constexpr (ReverseFirst) {
            if (lane == 0U) {
                second_byte = second[lane];
                first_byte = first[lane];
            } else {
                first_byte = first[lane];
                second_byte = second[lane];
            }
        } else {
            first_byte = first[lane];
            second_byte = second[lane];
        }
        const Int32 difference = normalized_difference(first_byte, second_byte);
        if (difference != 0) {
            return difference;
        }
    }
    return 0;
}
} // namespace

Int32 __cdecl compare_native_regions_00c1ef55(
    const void* first, const void* second, Uint32 count) noexcept {
    if (count == 0U) {
        return 0;
    }
    ReadableBytes first_cursor = static_cast<ReadableBytes>(first);
    ReadableBytes second_cursor = static_cast<ReadableBytes>(second);

    // Initial small counts use BYTE-only LR pairs, unlike a later small tail.
    if (count <= 4U) {
        for (Uint32 offset = 0; offset != count; ++offset) {
            const Byte first_byte = first_cursor[offset];
            const Byte second_byte = second_cursor[offset];
            const Int32 difference = normalized_difference(first_byte, second_byte);
            if (difference != 0) {
                return difference;
            }
        }
        return 0;
    }

    Uint32 remaining = count;
    while (remaining >= 32U) {
        for (Uint32 offset = 0; offset != 32U; offset += 4U) {
            const Uint32 first_word = read_dword(first_cursor + offset);
            const Uint32 second_word = read_dword(second_cursor + offset);
            if (first_word != second_word) {
                if (offset == 16U) {
                    return compare_dword_fallback<false, true, false>(
                        first_cursor + offset, second_cursor + offset, first_word);
                }
                return compare_dword_fallback<false, false, false>(
                    first_cursor + offset, second_cursor + offset, first_word);
            }
        }
        first_cursor += 32U;
        second_cursor += 32U;
        remaining -= 32U;
    }

    // Arithmetic routing of all 32 Original table cases. distance is the
    // positive magnitude of each Original negative endpoint displacement.
    ReadableBytes const first_end = first_cursor + remaining;
    ReadableBytes const second_end = second_cursor + remaining;
    Uint32 distance = remaining;
    while (distance >= 4U) {
        ReadableBytes const first_block = first_end - distance;
        ReadableBytes const second_block = second_end - distance;
        const Uint32 first_word = read_dword(first_block);
        const Uint32 second_word = read_dword(second_block);
        if (first_word != second_word) {
            if (distance == 10U) {
                return compare_dword_fallback<false, true, true>(
                    first_block, second_block, first_word);
            }
            if (distance == 12U || distance == 9U ||
                distance == 31U || distance == 15U) {
                return compare_dword_fallback<false, true, false>(
                    first_block, second_block, first_word);
            }
            return compare_dword_fallback<true, false, false>(
                first_block, second_block, first_word);
        }
        distance -= 4U;
    }

    if (distance == 0U) {
        return 0;
    }
    if (distance == 1U) {
        const Byte second_byte = second_end[-1];
        const Byte first_byte = first_end[-1];
        return normalized_difference(first_byte, second_byte);
    }
    if (distance == 2U) {
        const Uint32 first_word = read_word_zero_extended(first_end - 2U);
        const Uint32 second_word = read_word_zero_extended(second_end - 2U);
        if (first_word == second_word) {
            return 0;
        }
    } else {
        // Mod3 bypasses the WORD16 test; its first suffix pair is LR.
        const Byte first_byte = first_end[-3];
        const Byte second_byte = second_end[-3];
        const Int32 difference = normalized_difference(first_byte, second_byte);
        if (difference != 0) {
            return difference;
        }
    }
    // Reached mod2/mod3 final pairs are RL, at end-2 and then end-1.
    for (Uint32 tail_distance = 2U; tail_distance != 0U; --tail_distance) {
        const Byte second_byte = *(second_end - tail_distance);
        const Byte first_byte = *(first_end - tail_distance);
        const Int32 difference = normalized_difference(first_byte, second_byte);
        if (difference != 0) {
            return difference;
        }
    }
    return 0;
}

} // namespace bsp
