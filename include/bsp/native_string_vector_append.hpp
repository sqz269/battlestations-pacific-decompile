#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Source reconstruction of STL_CheckedNativeStringVector_Append_00450540.
// Native ECX is the actual 10h vector header; incoming EDX is unconsumed;
// the single stacked argument is a borrowed actual 8-byte string header.
// The explicit pool context and private scratch are new Source interfaces,
// not a binary-compatible replacement for the original RET 4 entry point.
void __fastcall append_native_string_header_vector_00450540(
    void* header, std::uint32_t unused_edx, const void* value,
    NativeStringRawPoolContext& strings);
} // namespace bsp
