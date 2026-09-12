#pragma once

#include "bsp/native_retained_memory_owners.hpp"

#include <cstdint>

namespace bsp {

// Complete original leaf ABIs, MSVC Win32. The unused EDX argument lets C++
// callers express the native ECX-only register input without adding a stack
// argument. Raw storage is the existing 14h owner produced by BEF6D0, with
// original numeric profile D642C0, count+04, backing+08, end+0C, cursor+10.
// The 10h backing layout is established by 8D43C0: profile/count/data/length.
bool __fastcall native_memory_stream_open_00bef4c0(
    const void* actual_stream, void* unused_edx) noexcept; // AL=1, RET
std::int64_t __fastcall native_memory_stream_length_00bef600(
    const void* actual_stream, void* unused_edx) noexcept; // EDX:EAX, RET
// BEF610: reload backing+08, then its data+08; cursor/end are unconsumed.
std::uint8_t* __fastcall native_memory_stream_data_00bef610(
    const void* actual_stream, void* unused_edx) noexcept; // EAX data, RET
void __fastcall native_memory_stream_read_00bef590(void* actual_stream,
    void* unused_edx, void* destination, std::uint32_t requested,
    std::uint32_t* optional_actual) noexcept; // three stack DWORDs, RET0C

// Explicit binding of the original immutable D642C0 profile to these leaves.
// No vtable replacement, callbacks, owner registry or shadow reference count.
// Domain: live D642C0 owners and the verified original profile words borrowed
// by NativeRetainedMemoryOwnerContext; arbitrary/mutated profiles are excluded.
// Current profile and slot reads are retained. This is a new C++ interface.
bool is_native_memory_stream_00d642c0(const void* actual_stream) noexcept;
bool dispatch_native_memory_stream_open(void* actual_stream,
    NativeRetainedMemoryOwnerContext&) noexcept;
std::int64_t dispatch_native_memory_stream_length(void* actual_stream,
    NativeRetainedMemoryOwnerContext&) noexcept;
void dispatch_native_memory_stream_read(void* actual_stream, void* destination,
    std::uint32_t requested, std::uint32_t* optional_actual,
    NativeRetainedMemoryOwnerContext&) noexcept;

// Implements the current slot0 -> BD30E0 -> current slot4 flag1 dispatch for
// original D642C0/BB8F90 and D15AD8/8D4470 owners by reusing their established
// complete destructors. It does NOT decrement references; the caller does so
// and invokes it only when the actual InterlockedDecrement result is zero.
// Null is accepted by BD30E0. Other profiles/slot identities are out of domain.
void dispatch_native_memory_owner_zero_reference(void* actual_owner,
    NativeRetainedMemoryOwnerContext&);

// Complete BEFA40 body/control flow over actual owners and shared CRT services.
// Native ABI: ECX source, stack low/high byte count, EAX stream, RET8. This
// interface adds the explicit existing ownership context, so is not that ABI.
// Null source -> null. Only low count is used; signed backing allocation rules
// are not normalized. A construction throw frees raw backing storage; after
// construction the original has no rollback guard around copy/wrapper creation.
void* create_native_memory_stream_from_copy_00befa40(const void* source,
    std::uint32_t count_low, std::uint32_t count_high,
    NativeRetainedMemoryOwnerContext&);

} // namespace bsp
