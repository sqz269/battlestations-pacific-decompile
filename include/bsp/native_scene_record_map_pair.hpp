#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene-record map pairs require MSVC Win32 actual storage.
#endif

#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {

// Complete 00469980 and 00469A20: native ECX destination, stacked source,
// EAX destination, RET4. Both pointers address actual 12-byte storage: native
// string at +0/+4 and an opaque mapped DWORD at +8. Clear the destination
// header even on identity, then use the concrete raw-pool header-copy body.
// Read source+8 only AFTER header work and copy its bits with integer accesses.
// There is no old-output release, scalar conversion, snapshot or local cleanup.
void* copy_construct_native_scene_record_map_pair_00469980(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context);
void* copy_construct_native_scene_record_map_pair_00469a20(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context);

// Complete 00469FC0 normal body plus its two Source unwind scopes. Native
// __fastcall: ECX destination12B, EDX mapped word, stacked BY-VALUE8B string
// key (length,data); EAX destination, RET8. This new C++ interface receives
// the ACTUAL callee-owned parameter cell by address; it does not make a copy.
// Its buffer is consumed on success. Discard that cell without another owner
// destructor afterward; the native cleanup leaves its header bytes untouched.
//
// Capture key.data then key.length BEFORE clearing the destination. Normal
// resize/copy/return uses those captures; unwind destroys the CURRENT parameter
// header. Once +8 is stored, a normal parameter-return failure instead destroys
// the CURRENT output header, without retrying the parameter. Output cleanup is
// not armed for an incomplete key construction. A second C++ cleanup failure
// during true unwinding terminates. No native FH3/SEH frame/resumption ABI is
// supplied by this ordinary C++ interface. Evidence and alias/ownership domain:
// docs/GOAL_OCT15_SCENE_MAP_PAIR.md.
void* construct_native_scene_record_map_pair_00469fc0(void* actual_destination,
    std::uint32_t mapped_word, void* actual_consumed_key_header,
    NativeStringRawPoolContext& context);

} // namespace bsp
