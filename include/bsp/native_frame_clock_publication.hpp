#pragma once
#include "bsp/native_frame_clock_actual.hpp"

namespace bsp {
struct ClockTimestamp;

// Borrow the SAME publication and method context through every call and drain.
// The published non-null pointer must designate genuine live 80h storage.
// This context owns no allocation, registration, table or advancing clock.
struct NativeFrameClockPublicationContext final {
    void* volatile& actual_clock_01090ab0;
    const NativeFrameClockActualContext& methods;
};

// Each operation captures current AB0 once and admits only D68D50 plus its
// exact borrowed slot word, then calls the existing concrete source provider.
// Null/unsupported profiles or targets throw std::logic_error: this is a source
// contract failure, not reproduction of the original fault/exception ABI.
void advance_published_native_frame_clock(const NativeFrameClockPublicationContext&);
// Returned pointers borrow the actual 16-byte member; no FrameClock cast or
// snapshot is made. A caller that needs a value must copy it before retirement.
const void* current_published_native_frame_clock(const NativeFrameClockPublicationContext&);
const void* interval_published_native_frame_clock(const NativeFrameClockPublicationContext&);
// Output must point to writable ClockTimestamp storage. Its preimage is not
// changed before the existing raw BEE080 sampler; native QPC BOOL is ignored.
ClockTimestamp* sample_published_native_frame_clock(
    const NativeFrameClockPublicationContext&, ClockTimestamp* output);
void enable_fixed_published_native_frame_clock(
    const NativeFrameClockPublicationContext&, std::int32_t milliseconds);

// These are source adapters for current-publication callsites, not native ABI
// replacements. Consumers with an earlier captured owner (e.g. B46C89) must
// retain that capture instead of substituting a new publication reload here.
} // namespace bsp
