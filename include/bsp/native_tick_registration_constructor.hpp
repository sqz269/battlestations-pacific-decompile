#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick registration construction requires MSVC Win32.
#endif

namespace bsp {

// Ordinary 00875890[139] through a new explicit Source C++ interface.
// Borrow the actual stable publication cells and a fixed raw tail with valid
// writable backing: its +04 field is the actual pending-last cell. Read the
// current timer word only after linking and the receiver+2C zero-word store.
// The receiver needs at least 34h bytes; the payload is an opaque pointer.
// Preserve raw holes and the second current tail+04 read after node+04/+08.
// No owned cleanup scope, validation, sentinel creation or startup is added.
// Numeric profiles do not provide callable Source tables; this interface does
// not reproduce Native ECX/RET8, residual XMM0/flags or hardware-fault behavior.
void* construct_native_tick_registration_00875890(
    void* actual_receiver,
    void* actual_payload,
    std::uint32_t raw_group_word,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0,
    void* actual_fixed_pending_tail_00e0b704,
    const volatile std::uint32_t& actual_timer_word_00d7a260);

} // namespace bsp
