#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

// Borrow the actual CE3A34 "%d\0" span and the same three raw pool cells.
// Span contents alone do not establish its Original-data provenance. All
// borrows, the raw output8 and their disjoint backing must remain live; there
// is no semantic NativeString owner or formatter callback in this interface.
struct NativeStringFromIntRawContext {
    NativeStringRawPoolContext& strings;
    const char* const actual_format_00ce3a34;
    const std::size_t actual_format_bytes;
};

// Normalized 004260B0 (Original ECX=output8, stack int32, EAX=output8, RET4).
// Raw output length/data are cleared before formatting. The selected unwind
// owns only the CURRENT private temporary after its buffer copy; normal return
// instead releases the earlier captured data/length pair, already disarmed.
// The output is never locally cleaned up. Current snprintf/Cpp-unwind and
// lower-provider qualifications are detailed in NATIVE_STRING_FROM_INT_RAW.md;
// this is not an Original ABI, hardware/SEH or loaded-locale readiness claim.
void* construct_native_string_from_int_004260b0(void* actual_output8,
    std::int32_t value, const NativeStringFromIntRawContext& context);

} // namespace bsp
