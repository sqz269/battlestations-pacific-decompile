#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoidance container tidy kernels require MSVC Win32.
#endif

namespace bsp {

// Complete 0041DDF0..0041DE19 (42B). ECX addresses the actual 10h vector
// header: untouched proxy+0, first+4, last+8, end+Ch. Capture first once;
// conditionally call the genuine current free service, then zero4/8/C.
// No element destruction, proxy cleanup or outer-header free is added.
void __fastcall tidy_native_avoid_body_vector_0041ddf0(void* actual_vector_header);

// Complete independent 29B library bodies, not aliases/thunks. ECX addresses
// the actual 0Ch list header: untouched proxy+0, current head+4, size+8.
// Call complete real hull-list clear, THEN load CURRENT head, free it even if
// null, then zero+4 after normal return. Clear owns size/selflink/hull/node
// work. No saved pre-clear head or invented success callback is used.
void __fastcall tidy_native_avoid_hull_list_0041f230(void* actual_list_header);
void __fastcall tidy_native_avoid_hull_list_0041e3a0(void* actual_list_header);

// All retain ECX/no stacked inputs/plain RET/ESI preservation; incidental EAX
// is deliberately unclaimed. Literal raw access order and call schedule are
// preserved by naked bodies, with only direct provider operands substituted.
// Actual reached backing/stack/control cells and matched provider lifetimes
// must be valid. Current CRT heap, provider C++ EH frames and Native runtime/
// SEH identities are qualified. The old typed/semantic interfaces are distinct.
// E3A0 has no current live/physical direct reference; no caller is invented.
// Evidence: docs/GOAL15F_AVOID_CONTAINER_TIDY.md.

} // namespace bsp
