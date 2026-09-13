#pragma once

#include <cstdint>

namespace bsp {
class NativeAdoptedSubstreamDispatch;

// Complete BBC1D0..BBC31D[334]. Original ECX raw34h owner; stack source,
// descriptor12h, two UNUSED capacities; EAX owner; RET10h. Descriptor words
// are offset/compressed/decoded, read sequentially from actual storage.
// Publish D64400/count1/source; set only byte+9 among bytes8..B; allocate
// actual10h input/output headers and fixed4000h/10000h arrays, then raw38h
// z_stream. Initialize only its five native input/allocator words before
// calling stock zlib1.2.1 inflateInit2_(-15,"1.2.1",38h); ignore status.
// Retain CURRENT source+0C, seek CURRENT source/current slot1C to CURRENT
// offset+10, then reload remaining sizes and zero position.
//
// On exception restore stream/reference profiles. Only an array-construction
// failure frees its captured raw10h header. Earlier published buffers, decoder
// storage and source references have no rollback. No owner free occurs here.
void* construct_native_raw_inflate_owner_00bbc1d0(void* actual_raw_owner,
    void* actual_source, const void* actual_descriptor,
    std::uint32_t unused_input_capacity, std::uint32_t unused_output_capacity,
    NativeAdoptedSubstreamDispatch&);

// New source interface over original storage and shared allocation service.
// Stock zlib is a library dependency, not reconstructed game source. This
// constructor does not provide the remaining D64400 numeric stream methods.
} // namespace bsp
