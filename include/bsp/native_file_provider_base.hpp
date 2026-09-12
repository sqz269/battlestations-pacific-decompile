#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Complete BB5590..BB5622. Native ECX actual provider base (at least14h),
// stack actual8h source name header, EAX owner, RET4. This source interface
// adds explicit existing NativeStringStorage. Use ActualNativeStringPoolStorage
// bound to the application's real publication/gate and shared lifetime domain.
// Writes CEB130,count+4=1,D641A0; zeros name+8/+C even for self-alias; copies
// current header contents, then writes device+10=FFFFFFFF. No string wrapper,
// reference acquisition, registration, or allocation of the provider itself.
// C++ unwinding restores only CEB130; native FH3/SEH ABI is not reproduced.
void* construct_native_file_provider_base_00bb5590(void* actual_owner,
    const void* actual_source_name, NativeStringStorage&);

// Complete BB5380..BB53E6. Native ECX owner, no stack arguments, RET.
// Install D641A0, capture current nonnull data+0C and length+8, release exactly
// length+1 via the supplied storage, then restore CEB130. All other fields,
// including stale name words, reference count and device ID, remain unchanged.
// NativeStringStorage::release is noexcept: lazy-pool-getter C++ failure there
// terminates under that existing interface; native throwing-release/FH3/SEH
// parity is outside this source domain. Provider allocation is not freed.
void destroy_native_file_provider_base_00bb5380(void* actual_owner,
    NativeStringStorage&) noexcept;

} // namespace bsp
