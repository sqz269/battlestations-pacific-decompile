#pragma once

#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Full 004D6F70..004D7012: ECX destination, stack source, EAX destination,
// RET4. Actual2Ch record. Compare identity before zeroing the name; initial
// name copy precedes name-cleanup registration. Copy the actual alias list,
// then five interleaved DWORDs and the unretained pointer at+28. Preserve+8.
// C66220 is its eight-byte current-name unwind action, calling0041DD20.
NativeRenderResourceRecord& copy_construct_native_resource_record_004d6f70(
    NativeRenderResourceRecord& actual_destination,
    const NativeRenderResourceRecord& actual_source,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation);

// Full 004D45A0..004D4618: ECX actual record, RET; no specified result.
// Name cleanup is armed across list clear, current-sentinel free/null, and
// capture of current name data. Disarm before its conditional pool return.
// Leave the name header, date words and unretained+28 pointer untouched.
// C65FD0 is its eight-byte current-name unwind action, calling0041DD20.
void destroy_native_resource_record_004d45a0(
    NativeRenderResourceRecord& actual_record,
    ActualNativeStringPoolStorage& actual_string_pool);

// Require valid current native storage and the application's actual pool
// publication/gate/lifetime binding. Existing typed particle/render APIs are
// independent. These entries use host C++ EH; the existing string release
// interface is noexcept, including its lazy-getter boundary. No SEH or binary
// ABI parity, sink ownership operation, cache or vector construction is added.

} // namespace bsp
