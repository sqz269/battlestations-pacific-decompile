#pragma once

#include <cstdint>

namespace bsp {

// Complete native 00C17643, stdcall(pointer, ignored spin), RET 8.
// Borrows an actual Win32 CRITICAL_SECTION. Calls the real Windows initializer;
// returns 1 only after normal API return. No exception/fault translation.
std::int32_t __stdcall initialize_native_crt_critical_section_without_spin_00c17643(
    void* actual_critical_section, std::uint32_t ignored_spin);

// Complete native 00C17639 store, with an additional borrowed-state argument.
// Bind the stable, actual writable DWORD at native 0109E454. The supplied DWORD
// is already encoded: this primitive neither encodes it nor owns a cache.
// The actual word must not alias this entry's stack frame or argument storage.
// EAX retains the input; the native caller does not consume a semantic result.
// The extra argument and stack save make this a qualified source interface,
// not a drop-in replacement for the original one-argument cdecl entry.
std::uint32_t __cdecl store_native_crt_encoded_critical_section_initializer_00c17639(
    std::uint32_t encoded_word, volatile std::uint32_t& actual_word_0109e454);

} // namespace bsp
