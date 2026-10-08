#pragma once

#include <cstdint>

namespace bsp {

// COMPLETE007EFB00..007EFB2C raw Win32 entry. ECX is THIS actual squadron,
// stack DWORD is the queried class word, full EAX is 0/1, RET4. EDX is unused.
// Native words24/2/1/0 return before any receiver-field access; otherwise ONE
// fresh DWORD at actual+C4 is compared. No class census, copied value, default,
// profile dispatch, allocation, callback or initializer is supplied.
// Ordinary caller domain: stable nonnull actual receiver with a genuinely live
// aligned DWORD at+C4 and readable backing through C8. The fixed-word paths
// perform no receiver read, but do not establish a constructor/lifetime domain.
// Original class/table binding, whole constructor/InitAll/world, invalid/fault
// paths, concurrency, private EH and gameplay remain outside this raw entry.
std::uint32_t __fastcall native_plane_squadron_is_kind_007efb00(
    const void* actual_squadron, void* unused_edx, std::uint32_t class_word);

} // namespace bsp
