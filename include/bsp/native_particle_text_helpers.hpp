#pragma once
#include <cstdint>

namespace bsp {
// AEDF60: ECX points to an actual token pointer cell, not an8h pooled-string
// header. Dereference text, call the current CRT atof boundary, round ST0
// through an explicit binary32 x87 store/reload, RET with ST0 result.
// Text is nonnull. Current CRT parsing is a provider boundary; this does not
// substitute a recovered original CRT body or establish its locale parity.
float parse_native_particle_token_float_00aedf60(const void* actual_token);

// AF3E90: ECX points to an actual token pointer cell; EAX signed count, RET.
// Null text returns0. Otherwise return1 plus the number of runs of ASCII spaces
// encountered before a signed byte below32 or byte127. Leading/trailing spaces
// count; empty or immediately nonprintable text returns1. No Unicode policy,
// token-vector bounds checking, or generic whitespace normalization is added.
std::int32_t count_native_particle_token_fields_00af3e90(const void* actual_token) noexcept;
} // namespace bsp
