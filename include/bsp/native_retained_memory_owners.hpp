#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_memory_backing_bytes = 0x10;
inline constexpr std::size_t native_memory_stream_bytes = 0x14;

// Borrow the actual non-atomic DWORD counters and immutable native profiles.
// Both supported profiles contain BD30E0 at slot0; slot4 contains 8D4470 for
// backing D15AD8 or BB8F90 for stream D642C0. Current profile/table reads remain
// observable. Other current profiles/terminals are outside the input domain;
// these original-address words are not callable host vtables or callbacks.
struct NativeRetainedMemoryOwnerContext {
    volatile std::uint32_t& actual_object_count_0109db98;
    volatile std::uint32_t& actual_requested_bytes_0109db9c;
    const volatile std::uint32_t* actual_backing_profile_00d15ad8;
    const volatile std::uint32_t* actual_stream_profile_00d642c0;
};

// 8D43C0: native ECX raw10h backing, stack signed requested length, EAX owner,
// RET4. Count1/profile, stored length=max(request,0), array allocation at least
// one byte. Then increment actual object count and add ORIGINAL request bits
// to the byte counter. Allocation failure restores only base profile CEB130.
void* construct_native_memory_backing_008d43c0(void* actual_raw_owner,
    std::int32_t requested_length, NativeRetainedMemoryOwnerContext&);

// 8D4440: native ECX backing, RET via base tail. Capture data+08 before profile
// write, free it, decrement count, subtract CURRENT stored length+0C and write
// CEB130. Keep count+04 and stale data/length fields; do not free owner storage.
void destroy_native_memory_backing_008d4440(
    void* actual_owner, NativeRetainedMemoryOwnerContext&) noexcept;

// 8D4470: native ECX backing, stack flags, EAX original owner, RET4. Same full
// destruction, then scalar-free owner only for flags bit0. No double-free guard.
void* delete_native_memory_backing_008d4470(void* actual_owner,
    std::uint32_t flags, NativeRetainedMemoryOwnerContext&) noexcept;

// BEF6D0: native ECX backing, RET, EAX new14h stream. Allocate actual raw owner,
// initialize count1/D642C0 and fields, reload/release old backing+08, publish
// input backing, real InterlockedIncrement, load its data then CURRENT length,
// publish cursor+10 then wrapped end+0C. No EH rollback or safe null-allocation
// return is added; a null input backing is not repaired.
void* create_native_memory_stream_from_backing_00bef6d0(
    void* actual_backing, NativeRetainedMemoryOwnerContext&);

// BEF9C0: native ECX stream, RET. Capture backing+08 before arming state0;
// decrement its real intrusive count and invoke current deleting dispatch only
// at zero, then clear CURRENT stream+08. Both normal and unwind cleanup write
// stream-base D5C104 then reference-base CEB130. Leave cursor/end untouched.
void destroy_native_memory_stream_00bef9c0(
    void* actual_owner, NativeRetainedMemoryOwnerContext&);

// BB8F90: native ECX stream, stack flags, EAX original owner, RET4. Destroy
// first; scalar-free only after successful return and flags bit0.
void* delete_native_memory_stream_00bb8f90(void* actual_owner,
    std::uint32_t flags, NativeRetainedMemoryOwnerContext&);

// B23640: native ECX actual destination DWORD slot, EDX source DWORD slot,
// EAX destination, RET. Capture source first, old destination second. Identity
// skips all writes/atomics. Otherwise publish/retain new before releasing old;
// a throwing old terminal keeps the published and already-retained new value.
void* assign_native_retained_memory_slot_00b23640(void* actual_destination_slot,
    const void* actual_source_slot, NativeRetainedMemoryOwnerContext&);

// New MSVC Win32 interfaces over actual storage/shared CRT allocation. No
// shared_ptr lifetime, shadow reference count, binary ABI or game proof.
} // namespace bsp
