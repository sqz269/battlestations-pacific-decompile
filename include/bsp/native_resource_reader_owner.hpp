#pragma once

#include <cstddef>

namespace bsp {

class NativeAdoptedSubstreamDispatch;
struct NativeStringRawPoolContext;

inline constexpr std::size_t native_structured_resource_reader_bytes = 0x70;

// Actual-storage owner bodies, new C++ ABI. Native ECX=owner, no stack args,
// RET; only BEA150 supplies a stable EAX result (the original reader).
// Buffer header0Ch is data/count/capacity. Base reader10h has stream+0 and
// buffer header+4. Structured reader70h adds ten8h strings at+10..+5F,
// index+60, control+64 and a separate8h string at+68/+6C. No projected owner.

// BF0980: resize actual buffer to0, then free CURRENT data; leave the stale
// data/capacity words. No local cleanup retries a failed resize or free.
void destroy_native_resource_reader_buffer_00bf0980(
    void* actual_buffer_header, NativeStringRawPoolContext&);

// BF09B0: capture stream before arming buffer cleanup; decrement captured+4,
// dispatch CURRENT slot0 at zero, then clear reader+0 after normal return.
// Normal path disarms cleanup before destroying reader+4. Stream failure
// invokes buffer destruction once through the original state0 unwind action.
void destroy_native_resource_reader_00bf09b0(void* actual_reader,
    NativeStringRawPoolContext&, NativeAdoptedSubstreamDispatch&);

// BEA150: initialize base10h, construct ten empty strings forward, then index0,
// controlFFFFFFFF and empty name. Iterator failure destroys only completed
// strings in reverse; outer state0 then destroys the base. No array cleanup
// state is added after the iterator returns. Return original reader.
void* construct_native_structured_resource_reader_00bea150(void* actual_reader,
    NativeStringRawPoolContext&, NativeAdoptedSubstreamDispatch&);

// BE9F10: name first in state1, ten strings in reverse in state0, base last in
// state-1. The vector iterator unwinds preceding strings after an element
// failure, never retries that element; the outer map then unwinds the base.
// Name failure unwinds the entire array and then base. Headers stay unchanged
// except for effects of the original called bodies/current callbacks.
void destroy_native_structured_resource_reader_00be9f10(void* actual_reader,
    NativeStringRawPoolContext&, NativeAdoptedSubstreamDispatch&);

// Ordinary raw-pool/dispatch exceptions propagate. A C++ exception raised by
// an unwind action terminates, matching ArrayUnwind/FrameUnwindToState filters.
// Storage must stay valid for all required accesses. This does not replace
// native CRT/FH3 identity, ProcessingThrow/CLR state or concrete stream dispatch.

} // namespace bsp
