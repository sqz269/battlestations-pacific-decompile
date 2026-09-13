#pragma once

namespace bsp {

// Complete 00B0CC10..00B0CCA1 (146 bytes). Native ECX bank, no stack
// arguments, plain RET; EAX is incidentally zero. Clear exactly 40 DWORDs
// in native store order. Borrow writable raw storage through bank+9Fh.
void clear_native_renderer_frame_statistics_bank_00b0cc10(void* bank) noexcept;

// Complete 00B0CCB0..00B0CCCB (28 bytes). Native ECX bank pair, no stack
// arguments, tail JMP to 00B0CC10. Copy all A0h bytes forward to bank_pair+A0h
// before clearing the first bank. Borrow readable/writable storage through
// +13Fh. The constructor's trailing 15 words at +140h..+17Bh are untouched.
// DF must be clear, as required by the original REP MOVSD and Win32 ABI.
void publish_native_renderer_frame_statistics_00b0ccb0(void* bank_pair) noexcept;

// Complete 00B0CCE0..00B0CD50 (113 bytes). Native ECX raw 17Ch storage,
// no stack arguments, EAX original receiver, plain RET. Clear the two banks
// through the complete leaf, then 15 trailing DWORDs in native store order.
void* construct_native_renderer_frame_statistics_00b0cce0(void* storage) noexcept;

// Actual storage starts at renderer+1B78h; the current and published banks are
// renderer+1B78h..1C17h and +1C18h..1CB7h. No private renderer, ownership,
// counter-field interpretation, null handling or synchronization is added.
// These are new C++ interfaces, not original-caller ABI or game validation.
// The larger renderer constructor and EndFrame remain independently incomplete.

} // namespace bsp
