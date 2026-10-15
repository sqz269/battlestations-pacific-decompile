#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 in-place range kernels require MSVC Win32.
#endif

namespace bsp {

// Complete 0041C680..0041C6A3, 36B: ordinary cdecl three stack arguments,
// plain RET. Capture endpoints; equality skips even the value-pointer load.
// Otherwise capture value pointer once, then EACH iteration read word0/store0
// BEFORE reading word1/store1. Source contents may alias either destination
// word. ADD8/equality retains literal DWORD wrap; no null destination skip,
// pair snapshot, numeric range guard or C++ pointer subtraction is added.
// ESI preserved. Incidental EAX==captured last on normal return is unclaimed.
void __cdecl fill_native_pod8_range_0041c680(
    void* first, const void* last, const void* value_source);

// Complete 0041DAC0..0041DAFD, 62B: ordinary cdecl three stack arguments,
// plain RET, ESI/EDI preserved. EAX result is captured before the empty test:
// destination_end - (SAR32(uint32(last-first),3)*8), all modulo DWORD.
// Each backward iteration reads word0 at old last-8 BEFORE decrement/CMP;
// stores word0, freshly reads word1, stores word1, then branches on that CMP.
// Aliased word0 stores can change the subsequent word1 read. No null skip,
// memmove, pair snapshot or signed C++ pointer difference is substituted.
void* __cdecl copy_backward_native_pod8_range_0041dac0(
    const void* first, const void* last, void* destination_end);

// Actual byte endpoints, not private pair objects. Both loops terminate by
// equality after +/-8 modulo DWORD. Every reached source/destination/stack/
// control cell must be backed; equal endpoints access no pointed storage.
// These literal Win32 bodies do not establish original C++ template/return
// types, installed Native-runtime/SEH compatibility or game validation.
// Evidence: docs/GOAL15G_POD8_INPLACE_RANGE.md.

} // namespace bsp
