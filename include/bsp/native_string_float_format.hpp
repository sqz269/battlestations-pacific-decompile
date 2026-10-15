#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Whole 0043BDF0, through a NEW context-bearing C++ interface. Original:
// ECX fresh8B header; entry stack+4 float32; EAX same header; RET4.
// Uses current-CRT std::sprintf("%f", widened_float32), NOT the game's
// private CRT. Active current locale/options/FP state determine the text.
//
// The fresh, writable aligned8B output owns its resulting raw pool buffer.
// Its caller releases that buffer once through this SAME canonical context.
// Two raw string allocations, captured normal temporary release, and only
// the completed-temporary exception cleanup are retained. No output rollback.
//
// Unbounded formatting must produce a NUL-terminated result fitting the
// accessible52B Source scratch. This models the observed safe Original stack
// prefix, not a recovered declared Original array extent. No finite/NaN/Inf,
// zero, null, locale or rounding filter/default is introduced. All accepted
// raw getter/pool/memory failure and accessibility qualifications still apply.
// No Original binary ABI/FS/EH/private-CRT text/runtime/game parity is claimed.
void* construct_native_string_float_header_0043bdf0(void* actual_fresh_header,
    NativeStringRawPoolContext& strings, float value);
} // namespace bsp
