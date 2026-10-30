#include "bsp/native_wide_string_raw_assign.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide assignment requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4 && sizeof(std::uint16_t) == 2);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
template<class T> T read(Word location) noexcept {
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(location)), sizeof value);
    _ReadWriteBarrier();
    return value;
}
template<class T> void write(Word location, T value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(location)), &value, sizeof value);
    _ReadWriteBarrier();
}
} // namespace

void* assign_native_wide_string_header_004c5e20(void* actual_header8,
    const void* actual_source8, NativeStringRawPoolContext& strings) {
    if (actual_header8 == actual_source8)
        return actual_header8; // BEFORE any header or provider read/effect

    const Word source = address(actual_source8);
    const Word destination = address(actual_header8);
    const Word requested = read<Word>(source);
    resize_native_wide_string_header_004c53e0(actual_header8, strings, requested, true);
    if (read<Word>(source) != 0u) {
        Word input = read<Word>(source + 4u); // CURRENT source DATA FIRST
        Word output = read<Word>(destination + 4u);
        std::uint16_t unit;
        do {
            unit = read<std::uint16_t>(input);
            write<std::uint16_t>(output, unit);
            input += 2u;
            output += 2u;
        } while (unit != 0u);
    }
    return actual_header8;
}
} // namespace bsp
