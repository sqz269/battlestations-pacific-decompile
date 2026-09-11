#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// 00425E10: ECX=destination, source pointer on stack, RET4. Snapshot source
// length and old destination length, resize preserving, then reload both data
// pointers. Self-append works through that reload. Valid nonoverlapping memcpy
// regions and nonwrapping total lengths are required by the native operation.
void append_native_string_00425e10(NativeString& destination, const NativeString& source,
    NativeStringStorage& = crt_string_storage());
} // namespace bsp
