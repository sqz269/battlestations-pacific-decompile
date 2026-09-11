#pragma once

#include <cstdint>

namespace bsp {

// Full00425850 against the canonical actual8-byte NativeString header:
// uint32 length+0, char* data+4. No null-header guard or storage ownership.
// Native ECX=header, stack=candidate, AL=boolean, RET4; the upper native EAX
// bits are unspecified on the null branches. This C++ result models AL only.
// Null data ignores recorded length and scans the complete nonnull candidate.
// Nonnull data/null candidate tests recorded length, not the first data byte.
// A nonnull pair uses the actual CRT _stricmp and its current locale.
bool equal_native_string_header_00425850(const void* actual_header,
    const char* candidate) noexcept;

// Full004259F0; native ECX=header, stack=(old byte, new byte, signed start),
// RET12, no defined result. Null data returns before start/length inspection.
// Negative start clamps to0. Scan by unsigned recorded length, including NUL
// bytes. Reload data every iteration and length after every conditional store,
// so aliases into the actual header affect subsequent iterations. No resize.
void replace_native_string_character_004259f0(void* actual_header,
    std::uint8_t old_byte, std::uint8_t new_byte, std::int32_t start) noexcept;

} // namespace bsp
