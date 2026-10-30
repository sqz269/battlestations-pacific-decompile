#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Whole0041E350 ordinary raw8 assignment. Nullable source requests0 without
// reads; otherwise scan before real RAW41DD40(preserve0). CURRENT destination
// DATA first: null skips LENGTH/copy; nonnull CURRENT LENGTH is copied through
// genuine BF7680, even count0, with original source and actual feature word.
// No clear, terminator copy, host-copy adapter, Frame, cleanup or error repair.
// Caller retains actual extents/cells/provider lifetime and partial debt;
// Original registers/private-stack/fault/ABI/runtime identity is unproved.
void* assign_native_string_cstring_0041e350(void* actual_header8,
    const char* nullable_source, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4);
} // namespace bsp
