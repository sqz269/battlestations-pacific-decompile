#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer record destruction requires MSVC Win32.
#endif

namespace bsp {

// Actual 0Ch header: data+00, signed count+04, signed capacity+08. Each full
// 61-byte native body consumes ECX=header and returns with plain RET.
// If capacity is negative, call the existing matching reserve with request0.
// Then decrement current positive count until nonpositive, CAPTURE current
// data, publish count0, and free captured data in the shared lifetime domain.
// Preserve the stale data pointer and current capacity; do not free the header.
// Reserve failure propagates before this body's count-clearing/free schedule;
// the existing allocator/handler effects are retained without added rollback.
// Raw reachable extents/lifetimes and native wrapping arithmetic are caller
// preconditions. New source CRT/private-frame exception boundary; no game or
// complete original binary caller/register/SEH compatibility claim.

// B29BA0..B29BDC: matching stride10h reserve B228B0.
void __fastcall destroy_native_renderer_records16_00b29ba0(void* actual_header);
// B29BE0..B29C1C: matching stride14h reserve B22940.
void __fastcall destroy_native_renderer_records20_00b29be0(void* actual_header);
// B29C20..B29C5C: matching stride18h reserve B229D0.
void __fastcall destroy_native_renderer_records24_00b29c20(void* actual_header);
// B29C60..B29C9C: matching stride28h reserve B22A70.
void __fastcall destroy_native_renderer_records40_00b29c60(void* actual_header);

} // namespace bsp
