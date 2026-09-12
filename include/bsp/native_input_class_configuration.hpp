#pragma once
#include <cstdint>

namespace bsp {
// Complete A917E0 game body over the SAME actual F8h backend allocation.
// Native ECX=backend; stack DWORDs class, requested_count, optional actual10h
// identifier-vector pointer; RET0Ch. No EDX input or consumed return value.
// Source ABI differs. Each actual10h vector is {untouched,begin,end,capacity}.
//
// Stores requested_count at +68h+class*24h, erases the active range, then the
// accepted-ID range. A nonnull source assigns its DWORDs; null appends count
// copies of FFFFFFFF. No device retain/release, dirty write, callback or SDK
// call occurs. Class/count are raw DWORDs without invented signed checks.
//
// Recognized checked-STL calls use stateless source storage contracts, not
// original library implementations or native CRT/EH replacements. Supply
// complete readable DWORD ranges and allocations in the existing
// singleton_lifetime_allocate/free domain. Returning CRT handlers may mutate
// the headers: the game body's captured ranges and subsequent reloads remain
// ordered. Arbitrary malformed storage/heap and original FH3/SEH are outside
// this interface. Existing A90DC0 destroys the resulting class buffers.
void configure_native_input_class_00a917e0(void* actual_backend,
    std::uint32_t class_index, std::uint32_t requested_count,
    const void* optional_actual_identifier_vector);
} // namespace bsp
