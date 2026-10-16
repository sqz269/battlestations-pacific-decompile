#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw FrameBox record allocation requires MSVC Win32.
#endif

namespace bsp {

// Complete 00ACEF10..00ACEF60 at the established Source CRT service boundary.
// Native ECX is an unsigned count; EDX is not consumed; EAX is the raw result,
// with no stack inputs and plain RET. Reserve unused EDX in this fastcall API.
// Count >05D1745Dh uses the already admitted Source std::bad_alloc policy.
// Otherwise request exactly count*2Ch native AND host bytes from the genuine
// singleton_lifetime_allocate object-byte domain. Zero STILL requests zero
// bytes; it does not bypass allocation or force a null result.
//
// Returned storage belongs to that Source malloc family and its matching
// singleton_lifetime_free, including packet 119 only under its separate
// constructed-span/retention contract. Original/private/foreign heaps,
// arbitrary new/new[] blocks, pool slots, raw names and resource pointers
// are not admitted by this allocation/free contract.
// The caller owns raw backing, any partial graph and allocator/new-handler
// lifetime across entry/failure; this function initializes/frees no records,
// owns no graph, and supplies no replay, retirement or production binding.
//
// Host CRT exception/handler/heap and C++ EH are intentional Source boundaries:
// no Original D6923C/E03CC0 object/RTTI/FH3 identity, original flags/register
// scratch schedule or drop-in binary replacement ABI is claimed. No fixed
// Native code address, synthesized provider, manager or global is invoked.
void* __fastcall allocate_native_gui_frame_box_records_00acef10(
    std::uint32_t count, void* unused_edx);

} // namespace bsp
