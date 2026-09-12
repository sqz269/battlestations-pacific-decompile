#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS mount records require MSVC Win32.
#endif

namespace bsp {

// Complete584110[86]: ECX actual8h string header, stack C-string character set,
// RET4. Null/empty set, null data or zero length return. Search with actual CRT
// strchr and signed-byte promotion; trim matching trailing bytes but retain the
// first byte. Call actual resize(length,1), including the equal-length path.
void trim_native_string_right_00584110(void* actual_header,
    const char* trim_set, NativeStringStorage&);

// CompleteBDCE40[92]: ECX destination, stacked source pointer, EAX destination,
// RET4. Raw14h-byte record: priority+0, string length/data+4/+8, provider+C,
// byte+10, untouched padding+11..13. This is construction, not assignment:
// zero the destination string before the self-header skip. Reload source length,
// provider and byte after resize/copy; copy does not acquire provider ownership.
void* copy_native_vfs_mount_record_00bdce40(void* actual_destination,
    const void* actual_source, NativeStringStorage&);

// CompleteBDCFC0[93]: ECX raw14h destination, stacked priority-DWORD pointer and
// raw10h payload pointer, EAX destination, RET8. Payload: string+0/+4,
// provider+8, byte+C, untouched padding+D..F. Capture *priority before stores.
// Same string construction/reload and untouched-padding rules as BDCE40.
void* construct_native_vfs_mount_record_00bdcfc0(void* actual_destination,
    const void* actual_priority_dword, const void* actual_payload,
    NativeStringStorage&);

// CompleteBDEEC0[132]: ECX raw14h destination, EDX priority bits, actual10h
// by-value payload on stack, EAX captured destination, RET10. The explicit source
// pointer below denotes that consumed argument storage, not a borrowed payload:
// call BDCFC0 with a captured local priority, then release its current string.
// Argument bytes remain untouched by cleanup. Inner construction failure releases
// the argument without destroying the incomplete destination. Native state0 can
// additionally release a completed destination if argument cleanup throws.
void* construct_native_vfs_priority_record_00bdeec0(void* actual_destination,
    std::uint32_t priority, void* consumed_actual_argument,
    NativeStringStorage&);

// Source interfaces preserve logical raw storage/order, not original stack ABI,
// FH3/SEH or mutable EH-spill identity. NativeStringStorage::release is noexcept;
// throwing native pool getters/cleanup and simultaneous exceptions are outside
// that existing source contract. Zero-byte memmove is omitted after all original
// field reads, matching the existing actual-header source policy. No game proof.
} // namespace bsp
