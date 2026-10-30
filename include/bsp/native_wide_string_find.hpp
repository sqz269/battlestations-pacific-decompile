#pragma once

#include <cstdint>

namespace bsp {
// Original A9EF60: ECX haystack raw8, stack(set raw8,start), EAX index bits/-1,
// RET8. This is first-character-of-set through the existing actual raw CRT
// C0382C boundary, not substring search or a semantic wchar/string view.
// Set DATA precedes hay DATA and gated hay length. Negative start clamps0;
// unsigned start==captured length still calls CRT. Compare result+start ONLY
// for equality with CURRENT hay length AFTER that call; preserve return bits.
// Caller supplies valid reached raw header/WORD/control/extent/alias lifetimes
// under the real CRT contract and excludes invalidating writers/fault protocols.
// No input owner/Frame/pool/cleanup or Original register/ABI/runtime grant.
std::int32_t find_first_of_native_wide_string_header_00a9ef60(
    const void* actual_haystack8, const void* actual_set8, std::int32_t start);
} // namespace bsp
