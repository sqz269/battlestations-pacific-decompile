#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending registry scalar retirement requires MSVC Win32.
#endif

namespace bsp {

// Ordinary Source interface for complete 00875850[55]. Borrow the actual raw8
// receiver, caller flags byte and mutable F878CC publication cell. Read bit 0
// only after actual owner+04 section release, before clearing the publication
// and resetting the base profile; then optionally free the captured receiver.
// Returns its address value even after free, without dereferencing it again.
// Numeric native profile stamps do not deliver callable Source tables. This
// cdecl/reference interface does not preserve native ECX/RET4, EH or fault ABI.
void* retire_native_pending_registry_scalar_00875850(
    void* actual_receiver,
    const volatile std::uint8_t& actual_deleting_flags_byte,
    void* volatile& actual_publication_00f878cc);

} // namespace bsp
