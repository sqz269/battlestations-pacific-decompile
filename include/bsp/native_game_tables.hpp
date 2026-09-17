#pragma once
#include <array>
#include <cstdint>
namespace bsp {
struct NativeUnitConversionDefinition {
    const char* source;
    const char* target;
    std::uint32_t factor_bits;
    std::uint32_t reciprocal;
};
using NativeUnitConversionDefinitions=std::array<NativeUnitConversionDefinition,23>;
// Immutable native literals/factor bits; pointer identities belong to this C++
// image. Diagnostic callers may bind the equivalent original-image literals.
const NativeUnitConversionDefinitions& native_unit_conversion_definitions() noexcept;
const std::array<std::uint32_t,12*97>& native_gunnery_preference_words() noexcept;
struct NativeGameTablesContext {
    void* unit_rows_00f88a50;
    volatile std::uint32_t& unit_count_00f88bc0;
    // Native local DWORD has only its low byte assigned. Make its three opaque
    // preexisting bytes an explicit input; do not silently zero or drop them.
    std::uint32_t unit_frame_word_preimage;
    const NativeUnitConversionDefinitions& unit_definitions;
    const void* preferences_00e092c8;
    void* ranks_00e19bf8;
};
// Complete normal1528B builder. Appends23 rows without resetting the count;
// native storage holds23 rows and therefore requires fresh count0 at startup.
// Each row: two pointers, factor bits, and low-byte reciprocal flag plus the
// supplied opaque upper24 bits. Count is reloaded/incremented after each row.
// The native consumer8D97A0 reads only the flag byte at+0C.
void append_native_unit_conversions_008d9150(NativeGameTablesContext&) noexcept;
// Raw adapter of the already recovered rank rule. Clear each97-word row before
// reading its97 authored DWORDs. Nonzero IDs consume rank1,2,... and are written
// without clamping. ID97 in row0 writes into row1 before row1 is cleared.
// Valid actual storage and native authored data required. No null fallback.
void build_native_gunnery_ranks_00727bd0(const void* preferences,void* ranks) noexcept;
} // namespace bsp
