#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Movie record allocation requires MSVC Win32.
#endif

namespace bsp {

// Complete AACD30/AACD90 at the established Source CRT service boundary.
// Original ECX=count, EDX unused, EAX=raw allocation; no stack inputs, RET.
// Positive count must satisfy FFFFFFFF/count >= stride. Zero still requests
// zero bytes through the real singleton_lifetime_allocate malloc/new-handler
// service, without a null shortcut. Overflow throws Source std::bad_alloc.
//
// Returned backing uses exactly the Source singleton_lifetime_free domain.
// Foreign/private heaps, arbitrary new/new[] storage, string buffers, widget
// pool slots and records obtained from other suppliers are not authenticated.
// These entries neither construct records nor own any graph; caller retains
// raw backing, partial records, providers and new-handler lifetime on failure.
// No tidy/range/consumer activation, rollback or native body credit follows.
// Source CRT/exception/RTTI/EH and allocation policy replace Original native
// BF681B/BF6340/BF6885/D6923C/E03CC0 identities and private stack schedule.
// No original binary ABI or hardware-fault/SEH equivalence is claimed.
void* __fastcall allocate_native_gui_movie_records_14h_00aacd30(
    std::uint32_t count, void* unused_edx);
void* __fastcall allocate_native_gui_movie_records_24h_00aacd90(
    std::uint32_t count, void* unused_edx);

} // namespace bsp
