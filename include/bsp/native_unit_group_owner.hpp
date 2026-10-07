#pragma once

namespace bsp {
// Complete 00778890..007788A7 (24 bytes). Native ECX is the actual unit,
// no stack arguments, plain RET. Only AL is the boolean result; the native
// nonnull path leaves the group-pointer bits in upper EAX (MOV AL,DL).
// Borrow valid actual unit/group storage. Capture unit+284h once; null is
// false, otherwise compare captured group+14h with that SAME actual unit.
// No ownership, initialization, callback, default or extra validation.
// Source bool is a new C++ interface, not an original class-ABI replacement.
bool native_unit_is_group_owner_00778890(const void* actual_unit) noexcept;
} // namespace bsp
