#pragma once

#include "bsp/native_raw_scalar_reader.hpp"

namespace bsp {

// BF0430..BF0468 (57 bytes): native ECX reader, one stack stream, RET4;
// EAX has no stable semantic return. This context-bearing cdecl interface is
// a source reconstruction, not a native ABI/frame replacement.
//
// Valid writable reader+0 and real intrusive stream counts at +4 are required.
// Retain the incoming reference independently of the caller's reference:
// capture new then old; identity does nothing; publish new, increment new,
// decrement captured old, and invoke its CURRENT slot0 only at zero.
// No later reader write or rollback occurs if the selected terminal throws.
// Aliased count locations and actual provider effects remain observable.
//
// Zero terminals are qualified to actual memory D642C0/BD30E0/BB8F90 and
// physical D691B0/BF55A0 providers. Only the selected terminal needs its
// nonnull existing context; physical profile words must be readable at D691B0.
// The memory invoker reloads current profile/slot4 and deletes with flags1;
// physical recycling owns current slot4/flags0 and the real pool append.
// Unsupported profiles/slots throw a SOURCE boundary exception, not a native
// error. Provider hooks/concurrent profile changes and native SEH/FH3/async
// faults are outside this domain. No allocation, seek or reader cleanup here.
// Evidence: NATIVE_RAW_READER_ATTACHMENT_LIFETIME_BO.md and
// NATIVE_RAW_READER_ATTACHMENT_BP.md; descriptive name is a hypothesis.
void assign_native_raw_reader_stream_00bf0430(void* actual_reader,
    void* actual_stream, NativeRawScalarReaderContext&);

} // namespace bsp
