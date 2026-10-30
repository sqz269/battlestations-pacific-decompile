#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// 0073C240: original ECX points to the actual 4000h bucket array followed by
// its DWORD count. This ordinary Source API adds the SAME raw string context
// and genuine initialized E1768C pool; it creates or validates neither owner.
// Current heads are traversed via saved node+14 next, payload then slot return.
// Only whole normal completion clears the heads and separately zeroes count.
// Retain full partial graph/provider custody after any escape; no retry or
// cleanup is inferred. Stable Enter/Leave imports/no hooks are required when
// factoring the Original cached-Enter inline sequence through the slot helper.
// No Original ABI/FH3/fault, loaded-manager, producer or startup claim.
void clear_native_locale_map_0073c240(void* actual_map_4004,
    NativeStringRawPoolContext& actual_strings,
    void* actual_initialized_locale_pool_00e1768c);
} // namespace bsp
