#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 insertion requires MSVC Win32.
#endif

namespace bsp {

// Complete normal operations and both recovered catch operations of
// 00420F80..004211C0. Original: ECX actual header, four stacked DWORDs
// (unread iterator owner, position, uint32 count, raw8 value pointer), RET10h.
// No semantic EAX result is established. Source EDX is an unused adapter
// argument, not an Original input. The entry passes the ACTUAL incoming four
// stack cells to its private current C++ EH implementation; their native
// reads and value/count/position-slot reuse are retained, not copied away.
// Header +0 is unread; +4/+8/+C are first/last/end. No host vector is created.
//
// Value word1 then word0 and initial capacity are consumed even for count0.
// The captured raw pair, uint32 wrap/SAR counts, current-header reloads,
// growth/in-place publication order, and genuine concrete range/allocation/
// free/failure providers are retained. Growth catch frees the CURRENT reused
// allocation slot then rethrows; short-tail catch only rethrows. There is no
// added rollback, validation, element destruction or capacity normalization.
//
// Every actually reached memory/stack access must be backed and range loops
// must terminate under their literal +/-8 equality schedules. Native malformed
// storage is not made safe. The Source tag uses its own frame preimage with
// only its low byte zeroed. Extra private call frames/current C++ EH and catch
// types differ from Native FH3/CRT/frame identity; this is not a drop-in Native
// exception/SEH or game-runtime claim. Evidence: GOAL15G_POD8_VECTOR_INSERT.md.
void __fastcall insert_native_pod8_vector_00420f80(
    void* owner, void* unused_edx, void* ignored_iterator_owner,
    void* position, std::uint32_t count, const void* value);

} // namespace bsp
