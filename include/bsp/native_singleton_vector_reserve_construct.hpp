#pragma once

#include <cstdint>

namespace bsp {

// Full BD0600[165]: ECX raw owner, EDX unused, stack requested capacity,
// RET4. Owner slots are DWORD begin+4/end+8/capacity-end+0C. Preserve unsigned
// bounds/capacity comparisons, raw32 SUB/SAR distances, retained captures,
// returning validation, current post-copy size, and begin/capacity/end stores.
// Growth has no replacement-allocation rollback if a later service throws.
void __fastcall reserve_native_singleton_slots_00bd0600(
    void* owner, void* unused_edx, std::uint32_t requested_capacity);

// Full BD0960[80]: ECX actual raw 14h manager, EDX unused, EAX same owner,
// plain RET. Zero +4/+8/+0C, reserve256, create actual raw tracked section,
// then publish +10. Preserve +0 and old +10 until the final publication.
// Native state0 -> CC5490 -> BD0220 is expressed by C++ catch/cleanup/rethrow.
// Synchronous source C++ failures from reserve/section creation clear storage;
// native FH3/SEH metadata, arbitrary unwind-spill aliases and hardware faults
// are not this ABI. The original section allocator gains no rollback.
void* __fastcall construct_native_singleton_manager_00bd0960(
    void* owner, void* unused_edx);

// MSVC Win32 only. Fixed actual source CRT allocation/free/memmove_s and
// _invalid_parameter_noinfo services are used, with their current ownership.
// A returning installed invalid handler is allowed to mutate raw owner state;
// it must obey the actual source CRT calling/exception contract. UCRT chooses
// thread-local then global handlers, unlike original global-only 109DD64.
// Original BF6713/BF66EF/encoded-handler/Watson internals and RTTI/throw identity
// are not reconstructed. Raw storage must belong to these concrete providers;
// no C++ typed-manager projection or original binary replacement is implied.
} // namespace bsp
