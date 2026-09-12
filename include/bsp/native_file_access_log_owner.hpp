#pragma once

#include "bsp/native_string.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include <cstdint>

namespace bsp {

// Borrow the application's existing singleton lifetime and actual109CEE8 cell.
// This bridge neither allocates a lifetime manager nor supplies a private log.
struct NativeFileAccessLogLifetimeBindings {
    SoundLifetimeAccess lifetime;
    void* volatile& publication_0109cee8;
};

// Complete native bodies against the actual four-byte owner, whose sole word
// is a native vtable address token. No stream/string is retained in the owner.
// Native7374A0/737540: ECX owner, RET; constructor EAX owner. The second lifetime
// getter precedes reading the current publication argument. Unwind restores
// CE3818 after releasing the captured section; publication is not rolled back.
void* construct_native_file_access_log_base_007374a0(void* actual_owner,
    NativeFileAccessLogLifetimeBindings&);
void destroy_native_file_access_log_base_00737540(void* actual_owner,
    NativeFileAccessLogLifetimeBindings&);

// Complete737C40. Native ECX owner; stacked native8h subject value; RET8, EAX
// owner. Supply that actual argument header here: it is consumed, not retained,
// and its bytes remain untouched. Construction failure releases its CURRENT
// header. Normal release captures data before writing the derived CFEAE0 token,
// then reads length+1. ActualNativeStringPoolStorage supplies the shared pool.
void* construct_native_file_access_log_00737c40(void* actual_owner,
    void* actual_subject_argument, NativeFileAccessLogLifetimeBindings&,
    NativeStringStorage&);

// Complete deleting wrappers, including installed ADD ESP,4 listing gaps.
// CFEA6C[0]=7376A0; CFEAE0[0]=737CC0. Native ECX owner, stacked flags DWORD,
// RET4; bit0 alone controls CRT free AFTER737540. Both return the captured owner,
// including the freed case. Source API is not the original ABI/FH3 replacement.
void* scalar_delete_native_file_access_log_base_007376a0(void* actual_owner,
    std::uint32_t flags, NativeFileAccessLogLifetimeBindings&);
void* scalar_delete_native_file_access_log_00737cc0(void* actual_owner,
    std::uint32_t flags, NativeFileAccessLogLifetimeBindings&);

} // namespace bsp
