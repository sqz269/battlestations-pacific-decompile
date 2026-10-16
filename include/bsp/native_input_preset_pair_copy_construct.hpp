#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input preset pair copy construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 0069E7A0, 99 bytes / 37 instructions. The actual 1Ch pair has an
// eight-byte key header (length +0, data +4) and five opaque DWORDs +8..+18.
// Native: ECX destination, unconsumed EDX, stack source, RET4, EAX destination.
// Source borrows the actual raw pool and 0109EEA4 feature cell: RET0Ch/new ABI.
// Compare addresses before zeroing both destination key words, even on self
// construction. Old backing is abandoned. On unequal addresses use the actual
// resize (preserve=1) and memcpy services with their current-field reloads.
// Copy each mapped four-byte representation before reading the next one; no
// floating evaluation, alias snapshot, key reset/cleanup, guard or local catch.
// Reached storage/extents and borrowed providers must be valid and exclude
// active private frames. Callee exceptions propagate after preceding effects.
// Current C++/CRT/RawCtx semantics do not establish Original ABI, private-frame,
// FH3/SEH/fault/heap/runtime/game identity. Name is a descriptive hypothesis;
// the existing STL_StringPresetPair_CopyConstruct_0069E7A0 identity is retained.
void* __fastcall copy_construct_native_input_preset_pair_0069e7a0(
    void* actual_destination, std::uint32_t unused_edx,
    const void* actual_source, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
