#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Complete 0041CB50..0041CB66: native ECX is an unsigned length, RET, EAX
// preserves the allocator's block. Resolve the actual 00419CC0 pool first,
// then allocate length+1 with DWORD wrap through actual 00BD1120. The native
// second stack word is the unread value 1. No string header, byte contents,
// terminator or release is supplied by this helper. Getter failures escape;
// the existing allocator's nullable result is returned unchanged.
// The explicit raw context is a new C++ interface, not original ABI parity.
char* allocate_native_string_storage_0041cb50(std::uint32_t length,
    NativeStringRawPoolContext& context);

} // namespace bsp
