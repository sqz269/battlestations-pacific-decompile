#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene DWORD vector ranges require MSVC Win32.
#endif

namespace bsp {

// Complete00468460..0046848E [47B/19ops], native cdecl(first,last,end), RET.
// Read last/first bits, subtract modulo32, exact SAR2, capture destination_end,
// scale shifted bits by4 modulo32 and capture end-bytes BEFORE any CRT call.
// Only signed-positive shifted count calls actual selectedSDK memmove_s with
// (captured_start,bytes,captured_first,bytes); status ignored. Return captured
// start even after a returning report. Zero/negative shifted counts still
// return rounded/wrapped start, with no move or alignment/order/null guard.
void* __cdecl copy_backward_native_scene_dword_range_00468460(
    const void* first, const void* last, void* destination_end);

// Complete00469950..0046997D [46B/21ops], native stdcall(first,count,value),
// RET0Ch. Raw unsigned count0 skips source access; high-bit counts still loop.
// Each reached iteration reads CURRENT value DWORD, stores, decrements count,
// advances4. Return bits independently wrap originalfirst+(originalcount<<2).
// Whole native body is byte-identical to accepted00441180. Source zero path
// returns first without forwarding value; nonzero reuses that actual complete
// provider, retaining progressive payload aliases rather than a value snapshot.
void* __stdcall fill_native_scene_dword_count_00469950(
    void* first, std::uint32_t count, const void* value_address);

// Source interfaces use raw32 address/count arithmetic, no nominal container,
// ownership, callback or fabricated child. Correct native STL names remain.
// Valid reached backing/SDK calls are required. CurrentSDK errno/handler policy
// is concrete, not original staticCRT/private-frame/argument-cell/register/
// spill/flags/fault/FH3/SEH/binary identity. Asynchronous mutation and original
// private-frame/payload coincidences are not exposed by these value interfaces.
// No native differential, application or game proof follows from compilation.

} // namespace bsp
