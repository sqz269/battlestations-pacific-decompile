#pragma once

#include "bsp/xlive_pipe_framing.hpp"

namespace bsp {

// MSVC Win32 machine-storage observation, not a recovered game function.
// Source/destination are valid, distinct, owned ranges of at least count bytes.
// All destination bytes are written by the leaf. It uses no APIs, allocations,
// TLS or callbacks and preserves LastError and the x86 nonvolatile registers.
// Normal Win32 ABI (direction flag clear) is required. This explicit compiler
// extension avoids ordinary C++ reads of indeterminate source representations.
__declspec(noinline) void capture_current_process_xlive_pipe_bytes(
    const void* source, void* destination, std::size_t count) noexcept;

// Observe the actual addresses supplied by the owning allocation/frame body.
// These are this process's heap/stack bytes, not an original-game trace. Changes
// in compiler, allocator and execution history can change the captured values.
// All8 bits are captured into defined C++ byte storage; only low2 bits of encoded
// preimages reach the recovered transforms. Raw random64 uses all64 bits.
class CurrentProcessXLivePipePreimageHost final : public XLivePipeProtocolPreimageHost,
    public XLivePipeFramePreimageHost {
public:
    XLivePipeProtocolAllocationPreimage context_allocation_preimage(
        const XLivePipeProtocolNativeState& actual_allocation) override;
    std::array<std::uint8_t, 72> temporary_wide_preimage(
        const XLivePipeEncodedWideValue& actual_temporary) override;
};
} // namespace bsp
