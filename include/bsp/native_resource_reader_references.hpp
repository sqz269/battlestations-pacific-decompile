#pragma once

#include <cstddef>

namespace bsp {

class NativeAdoptedSubstreamDispatch;

inline constexpr std::size_t native_resource_reader_base_bytes = 0x10;
inline constexpr std::size_t native_resource_reference_slot_bytes = 0x04;

// Actual Win32 storage, with a new C++ ABI. No projected reader, smart pointer,
// allocation or ownership adapter. The caller supplies live storage throughout
// the accesses below; nonnull referents have an aligned LONG at +4 and a DWORD
// table at +0. A zero result reads the CURRENT table and its CURRENT slot0.
// Dispatch source_zero_reference receives entry, captured referent and table;
// its callable adapter preserves native ECX=referent, EDX=table, no stack args.
// Numeric game profiles require reconstructed dispatch; this does not implement
// any concrete resource destructor or establish terminal-failure ownership.

// BF09A0: ECX reader10h; EAX original reader; RET. Store zero at 0/4/8/C
// in that order. Does not release an existing stream or optional buffer.
void* construct_native_resource_reader_00bf09a0(void* actual_reader) noexcept;

// BF0430: ECX reader, stack new stream, RET4; no stable return contract.
// Same pointer is a no-op. Otherwise publish new, retain new+4 if nonnull,
// then decrement captured old+4 and dispatch old slot0 only at zero.
// Callback failure leaves the new publication and both reference changes.
void assign_native_resource_reader_stream_00bf0430(void* actual_reader,
    void* new_stream, NativeAdoptedSubstreamDispatch&);

// BE9ED0: ECX actual4h handle, RET; no stable return contract.
// 483850: ECX actual4h resource slot, EAX captured slot, RET.
// Both capture slot/pointee, decrement a nonnull pointee, dispatch at zero,
// then clear the captured slot even if the callback replaced its contents.
// Null contents cause no write. A throwing terminal skips the clear; there is
// no native cleanup handler, rollback, second release or implicit teardown.
void release_native_structured_node_handle_00be9ed0(
    void* actual_handle, NativeAdoptedSubstreamDispatch&);
void* release_native_resource_slot_00483850(
    void* actual_slot, NativeAdoptedSubstreamDispatch&);

} // namespace bsp
