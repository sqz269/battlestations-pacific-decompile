#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoidance clip cleanup gates require MSVC Win32.
#endif

namespace bsp {

// Complete004167C0..004167F7,56B. ECX actual first8B zone/record header:
// pointer-array+0, uint32 count+4. No stack arguments, plainRET, ESI/EDI saved.
// Each iteration reloads CURRENT array, frees its indexed pointer even ifnull,
// reloads CURRENT count AFTER free, then unsigned-increments/compares. Finally
// zero ONLY count. Array allocation/entries, capacity and trailing owner words
// remain. No record inspection/virtual destructor, callback or array free.
void __fastcall clear_native_avoid_corner_records_004167c0(void* actual_record_header);

// Complete00412EF0..00412EFD,14B. ECX addresses the actual four-byte pointer
// cell; no stack inputs, plainRET. Capture CURRENT *cell ONCE at invocation;
// ifnonnull, free that value then POP ECX from the actual pushed argument cell.
// Do not reset/reread *cell or substitute a previously cached cleanup owner.
// Null value skips free; the cell address itself must be backed.
void __fastcall free_native_current_pointer_cell_00412ef0(void* actual_pointer_cell);

// Complete00412DC0..00412DD9,26B. ECX actual owner; no stack inputs, plainRET,
// ESI preserved. Capture owner and *owner once; nonnull free followed by zero
// of the captured owner's pointer word, even if free changed that word. Null
// pointer skips both free AND store. Count/capacity/trailing words are untouched.
void __fastcall free_and_clear_native_avoid_pointer_00412dc0(void* actual_owner);

// All return void; incidental Native EAX/volatile state is not a semantic
// result. Literal raw instructions/flags/call/cleanup schedule are retained,
// with only fixed free-call operands bound to genuine singleton_lifetime_free.
// Native BF6989->BF65AC->BF9DC8 and BF65AC->BF9DC8 are returning free services.
// Actual reached header/array/stack/control cells and matched heap ownership
// must be valid. No original C++ class/virtual/RTTI identity, callback API,
// local unwind frame, Native CRT/SEH/runtime or gameplay claim is introduced.
// Older AvoidZoneAllocationAccess projections remain distinct and unchanged.
// Evidence: docs/GOAL15I_AVOID_CLIP_CLEANUP.md.

} // namespace bsp
