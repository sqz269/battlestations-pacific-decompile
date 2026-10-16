#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene string-list ownership requires MSVC Win32.
#endif

#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

// Whole normal body 0094F270 [0094F270,0094F2ED), 125B/38 instructions.
// Actual raw12 layout: slots+0, count+4, capacity+8. Grow only on equality.
// Low flag byte zero copies the borrowed actual raw8 length/data header into
// a fresh current-heap eight-byte header; every other low byte adopts the
// supplied pointer, including null. Upper 24 bits are ignored. After reached
// providers, reload count/base, publish the record, increment/reload count.
// A separate current-C++ guard frees only the fresh header on copy exception;
// it disarms after normal slot/count publication and the return-count read.
std::uint32_t append_native_scene_string_list_0094f270(void* actual_list12,
    void* actual_record8, std::uint32_t native_flags,
    NativeStringRawPoolContext& raw);

// Whole normal body 0094F3E0 [0094F3E0,0094F441), 97B/42 instructions.
// Reload slots each iteration. Release captured record data through the raw
// context, free that captured header, then clear the captured slot address.
// Null slots skip all three operations. Reread signed count each iteration;
// write count0 at exit. Capacity and the slot allocation remain live.
void clear_native_scene_string_list_0094f3e0(void* actual_list12,
    NativeStringRawPoolContext& raw);

// Whole normal body 0094F540 [0094F540,0094F55F), 31B/12 instructions.
// Clear, reload slots, free nonnull slots and only then zero slots+0.
// Capacity and caller-owned raw12 lifetime are untouched. Append/growth after
// retirement requires genuine constructor reinitialization first.
void retire_native_scene_string_list_0094f540(void* actual_list12,
    NativeStringRawPoolContext& raw);

// Caller prepares the exact retained game_native_string_process().raw_context()
// before entry. No implicit process getter, replacement publications or pool
// cache is introduced. Raw destruction resolves 00419CC0 for EVERY nonnull
// data release, including large blocks and disabled small returns.
//
// Admitted domain: actual constructed aligned raw12 with coherent nonnegative
// count<=capacity, valid reached slots, nonwrapping positive allocations and
// indexes, matching current heap/pool generation, uniquely owned disjoint raw8
// headers/data, normal provider returns, serialized operation without faults,
// throws, aliasing, reentry, trim/drain/teardown or lifetime replacement.
// Adoption excludes stack/inline/synthetic/duplicate-owned headers. These are
// caller preconditions, not runtime repairs. Full limits are documented in
// docs/NATIVE_SCENE_STRING_LIST_OWNERSHIP.md.
//
// Ordinary explicit-context C++ APIs; potentially throwing. Original ECX/RET8
// or RET entry contracts, private FH3/FS0 cleanup and fault/ABI equivalence are
// not claimed. The separate current-C++ guard earns no Original EH credit.
// Application 008F3270 ownership and comparator/sort roles remain separate.

} // namespace bsp
