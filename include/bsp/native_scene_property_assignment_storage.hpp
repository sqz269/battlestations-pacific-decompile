#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Scene property assignment storage requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary successful-domain reconstructions. Names are hypotheses;
// these new C++ interfaces do not supply Original register/stack/flags ABI,
// historical private CRT, failure/EH/OOM, class/vtable or game behavior.
// All nonnull old pointers are sole-owned actual current canonical allocations.
// Require live stable storage, DF0, successful providers and no reentry,
// concurrent mutation, invalid aliasing, overflow or access to freed storage.

// Whole [004357F0,0043580A),26 bytes/11 instructions. The actual writable
// four-byte pointer slot lies outside its old allocation and active frames.
// Null performs no free and no store. Nonnull calls singleton_lifetime_free
// FIRST, then physically stores zero in the still-live actual slot.
void free_and_clear_native_owned_pointer_004357f0(void* actual_pointer_slot);

// Whole [008F03B0,008F03EF),63 bytes/27 instructions. Supply actual eight-byte
// destination/source blocks: +0 pointer, +4 BYTE size. Capture source pointer
// first and size second BEFORE reading/freeing the destination old pointer.
// Null old pointer skips free/clear; nonnull old pointer is freed then cleared.
// Publish captured size BEFORE canonical allocation, publish returned pointer
// BEFORE real memcpy. Do not replace the ordered physical writes with a model.
//
// The captured count is POSITIVE; source remains readable for that nonwrapping
// byte span. Source bytes, both headers and active frames are disjoint from the
// old destination allocation and the new output. Owning self/shared old-buffer
// alias would be UAF and is excluded. New storage may reuse the old address.
// No stride, tag or ownership flag is read/changed. At record+20, destination
// ownership byte+2C must already be1 for later owning canonical retirement.
// Zero-count allocation/memcpy and non-owning destination lifetime are excluded;
// no zero shortcut, count conversion, ownership repair or failure cleanup.
// Caller owns the actual returned/stored allocation and retires it once through
// singleton_lifetime_free or its established same-domain record lifetime.
void copy_native_scene_property_array_block_008f03b0(
    void* actual_destination_block, const void* actual_source_block);

} // namespace bsp
