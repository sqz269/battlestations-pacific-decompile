#pragma once

#include <cstdint>

namespace bsp {
class NativeMpakContainerLibrary;
class NativeAdoptedSubstreamDispatch;
struct NativeStoredStreamConversionContext;

// Explicit CURRENT stream slot20 dispatch. Numeric stream profiles must be
// bound to their actual reconstructed position method by the shared runtime.
class NativeMpakEntryPositionDispatch {
public:
    virtual ~NativeMpakEntryPositionDispatch() = default;
    virtual std::uint64_t source_position(std::uintptr_t entry, void* actual_source) = 0;
};
struct NativeMpakEntryContext {
    NativeMpakContainerLibrary& containers;
    NativeAdoptedSubstreamDispatch& streams;
    NativeStoredStreamConversionContext& conversion;
    NativeMpakEntryPositionDispatch& position;
};

// Complete BB5080..BB521E[415]. Original ECX actual44h provider, stack signed
// file index, EAX independent memory wrapper, RET4. Negative returns null
// before owner reads. Validate file vector; capture actual24h record; publish
// provider+40; use source slot20 EAX (ignore high word) to choose first unsigned
// offset >= position, else first offset. Keep returning CRT checks and reloads.
// Flag0 copies the selected range via actual BEF840. Nonzero flag allocates34h,
// constructs actual BBC1D0 with descriptor{offset,record+C,record+8}, converts
// via actual BEF750 and decrements the temporary inflater's real count/current
// slot0. Only constructor failure frees the captured34h allocation. No cleanup
// is armed during conversion or zero-reference dispatch.
void* materialize_native_mpak_entry_00bb5080(void* actual_provider,
    std::int32_t file_index, NativeMpakEntryContext&);

// New C++ interface, not original ABI/FH3. Numeric raw-inflate dispatch in
// conversion/streams is an integration dependency, never a successful stub.
} // namespace bsp
