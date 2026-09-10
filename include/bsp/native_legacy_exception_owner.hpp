#pragma once

#include "bsp/native_legacy_sbo_string.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// Actual Win32 28h exception storage. The leading 0Ch is std::exception's
// vtable, message pointer and ownership DWORD; the existing 1Ch SBO string
// starts at +0Ch. Destruction is explicit and does not clear base fields.
struct NativeLegacyExceptionStorage {
    std::uint32_t native_vtable_00;
    char* base_message_04;
    std::uint32_t owns_base_message_08;
    NativeLegacySboStringStorage message_0c;
};

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeLegacyExceptionStorage) == 0x28);
static_assert(offsetof(NativeLegacyExceptionStorage, base_message_04) == 4);
static_assert(offsetof(NativeLegacyExceptionStorage, owns_base_message_08) == 8);
static_assert(offsetof(NativeLegacyExceptionStorage, message_0c) == 0x0c);
static_assert(std::is_trivially_default_constructible_v<NativeLegacyExceptionStorage>);
static_assert(std::is_trivially_copyable_v<NativeLegacyExceptionStorage>);

// 00411700..0041175F, ECX destination, stack source SBO address, RET4/EAX this.
// Initialize the base, initialize only the observed member-string fields, then
// copy its full substring. A failed member copy cleans up the completed base.
NativeLegacyExceptionStorage& construct_native_legacy_logic_error_00411700(
    NativeLegacyExceptionStorage&, const NativeLegacySboStringStorage& source);

// 00411780..004117B2, ECX actual owner, RET0, no semantic return. Dispose/reset
// the member string, then dispose the base. Do not free owner storage itself.
void destroy_native_legacy_logic_error_00411780(NativeLegacyExceptionStorage&) noexcept;

// 004117C0..00411807, ECX actual owner, stack flags, RET4/EAX original owner.
// Destroy first; flags & 1 additionally frees the actual owner. Other bits do
// not request disposal. The returned address may already have been freed.
NativeLegacyExceptionStorage* scalar_delete_native_legacy_logic_error_004117c0(
    NativeLegacyExceptionStorage&, std::uint32_t flags) noexcept;

// 004118D0..00411935, ECX destination, stack source owner, RET4/EAX this.
// Base copying is outside the owner's state-0 unwind scope. A base-owned
// message uses nullable CRT malloc, then reloads source.message for strcpy_s.
NativeLegacyExceptionStorage& copy_native_legacy_logic_error_004118d0(
    NativeLegacyExceptionStorage&, const NativeLegacyExceptionStorage& source);

// 00411940..00411959: call logic-error copy, then publish length-error vtable.
// ECX destination, stack source owner, RET4/EAX this.
NativeLegacyExceptionStorage& copy_native_legacy_length_error_00411940(
    NativeLegacyExceptionStorage&, const NativeLegacyExceptionStorage& source);

// Native vtable DWORDs are retained as verified address data, not callable host
// C++ vtables. These functions do not establish native throw/exception ABI.
// The caller begins the raw storage lifetime by default-initialization without
// parentheses, which performs no field writes. These functions preserve the member
// string's leading DWORD and unused buffer bytes unless the string helper writes
// them. No implicit struct construction or additional member cleanup is applied.

}
