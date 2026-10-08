#pragma once

namespace bsp {

// Ordinary C++ interface to the complete native 00810020 wake fill.
// actual_wake supplies writable native storage: 40 records at +8, stride 0x18,
// head at +0x3C8 and flag at +0x3CC. Position addresses three readable floats
// and may alias writable wake storage; head/flag stores precede position reads.
// Fills XYZ/heading/segment in slots 39..0, preserving every yaw, residual,
// prefix, padding and other byte. The caller owns the preserved preimage,
// lifetime and synchronization. No written count or owner model is introduced.
// Requires four free x87 slots. Preserves ambient FP settings and the exact
// FCOS/FSIN, spill, read/store and arithmetic status/exception sequence.
void fill_native_unit_wake_00810020(void* actual_wake,
    const float* position, float heading);

} // namespace bsp
