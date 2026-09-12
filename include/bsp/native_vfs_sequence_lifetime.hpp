#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS sequence lifetime requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;

// Complete BDAED0[72],7F8310[72] and their five-byte native tail thunks.
// Original/source ABI: ECX raw owner, unused EDX, RET0. Owner+4 is the head,
// +8 the count; node+0/+4 are links. Reset links/count before walking, capture
// next before each free, reread current head, free head, then clear owner+4.
// Payload pointers and owner+0 stay untouched. No null-owner/head guards.
void __fastcall destroy_native_vfs_plain_list_00bdaed0(void*, void* unused_edx) noexcept;
void __fastcall destroy_native_vfs_plain_list_007f8310(void*, void* unused_edx) noexcept;
void __fastcall destroy_native_vfs_plain_list_thunk_00bdb3c0(void*, void* unused_edx) noexcept;
void __fastcall destroy_native_vfs_plain_list_thunk_007f8770(void*, void* unused_edx) noexcept;

// Complete BDB850[118] normal storage behavior: native ECX=10h pair, RET0.
// Release the eight-byte string at +8, then +0, retaining both headers.
// Source storage is explicit. Its noexcept release/actual pool-getter boundary
// does not reproduce native FH3 cleanup after a throwing getter; see the doc.
void destroy_native_vfs_string_pair_00bdb850(void* actual_pair,
    NativeStringStorage&) noexcept;

// Complete BDCC60[175]: native ECX=destination, stack source, EAX=destination,
// RET4. Zero each string's header before its identity guard, then resize/copy
// with current header reloads. Second-string failure releases only the current
// first string; failure during first construction has no owner cleanup.
// BF7680 copies backward for overlapping destination>source: use memmove,
// retaining the existing resize provider's own buffer/CRT boundary.
void* copy_construct_native_vfs_string_pair_00bdcc60(void* actual_destination,
    const void* actual_source, NativeStringStorage&);

// Actual vector header: backing DWORD+0, signed count+4, signed capacity+8.
// Elements are 10h pairs. Explicit string storage changes these source C++
// interfaces from the original ECX owner + one stack argument / RET4 ABI.
// BDCD10[221]: signed capacity clamp >=1, DWORD capacity<<4, copied elements,
// old-value destruction/free, then new backing/capacity publication. Native
// reserve unwind calls the actual RET-only401130: do not add rollback.
void reserve_native_vfs_pair_vector_00bdcd10(void* actual_owner,
    std::int32_t capacity, NativeStringStorage&);

// BDEC70[101]: all signed grow/shrink branches; decrement live count before
// destroying each removed value, then store requested count. No shadow count.
void resize_native_vfs_pair_vector_00bdec70(void* actual_owner,
    std::int32_t count, NativeStringStorage&);

// BE0350[23]: native ECX owner, RET0. Resize0, free current backing; retain its
// pointer/capacity fields. Uses the existing malloc-paired lifetime service.
void destroy_native_vfs_pair_vector_00be0350(void* actual_owner,
    NativeStringStorage&);

// Descriptive names are hypotheses. All raw missing body intervals, original
// ABI/EH and current CRT/pool/FH3 limits: docs/NATIVE_VFS_SEQUENCE_LIFETIME.md.
// These helpers do not establish complete VFS manager lifetime readiness.
} // namespace bsp
