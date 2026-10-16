#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene string-list indexed insertion requires MSVC Win32.
#endif

#include <cstdint>

namespace bsp {

struct NativeStringRawPoolContext;

// Whole normal body 0094F2F0 [0094F2F0,0094F39F), 175B/61 instructions.
// Actual raw12: slots+0, signed count+4, capacity+8. Equality-only growth
// precedes the flag test. Low flag byte zero copies a borrowed actual raw8
// through an exact fresh eight-byte heap header and the raw-context copy;
// all 255 nonzero low bytes adopt the supplied pointer, including null.
// Upper flag bits are ignored. Only after providers, clamp the signed index:
// negative becomes zero without an upper-count read; otherwise clamp to the
// then-current count. Reload count for descending pointer shifts, reloading
// slots each iteration. Finally reload slots, publish, increment current count.
// Return the captured clamped index without rereading count for the return.
std::int32_t insert_native_scene_string_list_indexed_0094f2f0(
    void* actual_list12, void* actual_record8, std::int32_t native_index,
    std::uint32_t native_flags, NativeStringRawPoolContext& raw);

// Caller prepares the exact retained game_native_string_process().raw_context()
// before entry, with genuine 01090AA8/01090AA4/01090AA0/generation bindings.
// Genuine constructed/grown, aligned writable raw12 and exact slot allocation:
// 0<=count<=capacity<=0x3fffffff, capacity>=64 and a multiple of 64; full growth
// requires old capacity<=0x3fffffbf. Initialized reached slots and writable
// spare slot[count], nonwrapping addressing/count+1, unique separately allocated
// owned headers/data, matching heap/pool generation, and normal provider returns
// are mandatory. Copy may borrow a live existing record across growth. Adoption
// excludes duplicate ownership, stack/inline/fake headers and storage aliases.
// No faults, throws, OOM, reentry, concurrency, teardown or lifetime replacement
// are admitted in normal composition. Full domain and positive resource/DF-clear
// conditions are in docs/NATIVE_SCENE_STRING_LIST_INDEXED_INSERT.md.
//
// Separate current-C++ policy: copy-only guard frees just its captured fresh
// header if copying throws; disarm after final slot/count publication and
// captured result preservation. No data destruction or growth/shift rollback.
// This API is potentially throwing. Its int32 result is a NEW Source exposure
// of measured final EAX, not recovered Original return/caller semantics. Native
// ECX/RET0C, private CRT/EH/SEH, full physical ABI, application and game/runtime
// equivalence remain unproved. Later clear/retire are lifecycle dependencies.

} // namespace bsp
