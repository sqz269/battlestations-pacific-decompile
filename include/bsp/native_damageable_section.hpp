#pragma once
#include <cstdint>

namespace bsp {

// Complete 00878B40, native ECX destination / stack source / EAX destination /
// RET4. Both records are actual borrowed 30h storage. Write the stable actual
// D0DF04 table identity, two integers, six ordered x87 float transfers; zero
// destination+24 BEFORE reading/retaining source+24; transfer two more floats.
// Self-copy consequently abandons its handle without a release or retain.
// Preserve x87 control/status behavior: this is not a byte-copy operation.
void* copy_construct_native_damageable_section_00878b40(void* actual_destination,
    const void* actual_source, std::uint32_t actual_vtable_00d0df04);

// Complete 0041DE40, native ECX one-word handle slot / RET. Capture its current
// owner, atomically decrement owner+4, dispatch the captured owner's actual
// vslot0 on zero, then clear the original slot. A null slot skips all writes.
// Do not clear before callbacks, retain replacement values, or release a new
// value published by a callback. No invented destructor/noexcept/EH wrapper.
void release_native_ref_counted_handle_0041de40(void* actual_handle_slot);

// Complete 00878EF0, native ECX actual30h record / RET. Write its stable actual
// D0DF04 table identity, then perform the same handle release on record+24.
// All other record bytes remain untouched; no record free or base cleanup.
void destroy_native_damageable_section_00878ef0(void* actual_section,
    std::uint32_t actual_vtable_00d0df04);

} // namespace bsp
