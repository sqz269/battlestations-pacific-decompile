#pragma once

#include <cstdint>

namespace bsp {

// Complete BF4FF0..BF5017 raw body; original ECX is ignored, one stack token,
// AL Boolean, RET4. Read the application's three current contiguous ID words
// at 0109DC30/34/38, stopping at the first match. The caller supplies that actual
// volatile storage; zero is an ordinary token. No owner, copy or fixed IDs.
bool query_native_physical_stream_type_00bf4ff0(std::uint32_t token,
    const volatile std::uint32_t* ids_0109dc30) noexcept;

// Complete BF4F20..BF4F3F; original ECX actual20h stream, stacked distance
// low/high DWORDs and origin, full SetFilePointerEx BOOL in EAX, RET0C.
// Reuses the raw20h owner produced by construct_native_physical_stream_00bf50d0
// in native_physical_stream_open.hpp: HANDLE+8, output position+10/+14.
// The OS writes directly to that existing field, including its failure behavior.
// No cached-size update, origin normalization, handle check or error translation.
std::int32_t seek_native_physical_stream_00bf4f20(void* actual_stream,
    std::uint32_t distance_low, std::uint32_t distance_high,
    std::uint32_t origin) noexcept;

// New C++ service interfaces, not drop-in binary ABI replacements. Descriptive
// names remain hypotheses. The caller owns the stream and ID publication.
} // namespace bsp
