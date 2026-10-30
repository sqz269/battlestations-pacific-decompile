#include "bsp/native_wide_string_raw_copy.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide copy construction requires MSVC Win32.
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

void* copy_construct_native_wide_string_header_004c8dd0(void* actual_header,
    const void* actual_source, NativeStringRawPoolContext& context) {
    const bool same_header = actual_header == actual_source; //4C8DD8 BEFORE stores
    const Word destination = address(actual_header);
    write(destination, Word{0}); //4C8DDA, even exact self identity
    write(destination + 4u, Word{0}); //4C8DE0; abandon preimage, never release
    if (!same_header) {
        const Word source = address(actual_source);
        const Word requested = read<Word>(source); //4C8DE9, captured before resize
        resize_native_wide_string_header_004c53e0(actual_header, context, requested, true);
        if (read<Word>(source) != 0u) { //4C8DF3, CURRENT length after callbacks
            Word input = read<Word>(source + 4u); //4C8DF8, BEFORE destination data
            Word output = read<Word>(destination + 4u); //4C8DFB
            std::uint16_t unit;
            do {
                unit = read<std::uint16_t>(input); //4C8E00
                write(output, unit); //4C8E03, including NUL, forward overlap order
                input += 2u;
                output += 2u;
            } while (unit != 0u); //no captured-length/count bound
        }
    }
    return actual_header;
}
} // namespace bsp
