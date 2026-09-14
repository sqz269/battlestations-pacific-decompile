#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;

// Complete 00531030..005310A9: ECX actual8h output header, stack byte word;
// EAX output, RET4. Clear the header before allocating two bytes, then reload
// it for preserve/return. Store length one, NUL, and the word's low byte even
// when that byte is NUL. This is construction, so previous ownership is lost.
// Supply the application's ActualNativeStringPoolStorage to resolve00419CC0
// separately at every allocation/nonnull return. New explicit-storage ABI.
void* construct_native_string_byte_00531030(void* actual_output,
    std::uint32_t byte_word, NativeStringStorage&);

// Complete 0054AA70..0054AB02: ECX actual8h destination, stack byte word,
// RET4, no semantic result. Construct the one-byte temporary, capture its
// length/data, resize destination with preserve, copy captured bytes and
// return the captured temporary buffer. Only destination data is reloaded
// after resize. Uses the established0041DD40 storage-domain/overlap limits;
// native FH3, arbitrary compiler-stack aliases and hardware faults excluded.
void append_native_string_byte_0054aa70(void* actual_destination,
    std::uint32_t byte_word, NativeStringStorage&);
} // namespace bsp
