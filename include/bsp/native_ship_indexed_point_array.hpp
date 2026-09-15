#pragma once

#include <cstdint>

namespace bsp {

// Complete ordinary body 00829180-00829203 (132 bytes). Original ABI:
// ECX={DWORD data, signed count, signed capacity}, one signed stack argument,
// RET4, no stable result. Elements are 10h bytes: three binary32 coordinates
// followed by a raw DWORD index. Storage is the actual borrowed Win32 header.
// Clamp the request to >=1, grow only, preserve count, allocate wrapped
// capacity*10h bytes, and copy each coordinate through an ordered x87 FLD/FSTP
// pair before copying its index. Reload source/count as in the native loop.
// Free the current old buffer before publishing data and then capacity.
void reserve_native_ship_indexed_point_array_00829180(
    void* actual_header, std::int32_t requested);

// New source interface using singleton_lifetime_allocate/free. This is not an
// original ABI/FH3 bridge or an original CRT heap replacement. Callers provide
// valid storage for every reached access. Native signed comparisons, wrapped
// DWORD arithmetic, null-destination skip, and x87 state are retained; no size
// validation, zero fallback, rollback or hardware-fault recovery is added.
} // namespace bsp
