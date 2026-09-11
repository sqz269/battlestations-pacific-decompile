#pragma once

namespace bsp {

// Complete 00B600B0..00B600E4. Native ECX actual44h hardware layout owner,
// RET; no semantic result. Borrow its actual COM declaration pointer at+40h.
// Capture the initial pointer: if nonnull, call its current AddRef slot, then
// reload its table and call Release on that same captured pointer. Reread the
// owner's current+40h; if nonnull, call its current Release and clear+40h only
// after that call returns. COM callbacks may change the table or owner field.
// No owner allocation, profile/descriptor/tree action, or exception cleanup.
void release_native_hardware_layout_device_resource_00b600b0(void* actual_owner);

// New MSVC Win32 source interface; no original ABI or game validation.
} // namespace bsp
