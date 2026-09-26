#pragma once

#include <cstdint>

namespace bsp {

// Full B340A0..B340E6: native ECX owner, stack borrowed COM/flags,
// EAX same owner, RET8. This source interface is not a native ABI entry.
// Writable 24h prefix, actual scalar fields, and four-byte aligned +4 must
// be fresh or fully retired and exclusive. Start ONE count at its native
// write; never pass a live count/companion to reconstruct or reset it.
// SAME live DWORD serial 0108D6E8 must not overlap owner storage and must
// be exclusively accessed; do not supply a private/default counter.
// Does not retain COM, touch +18 or construct the remaining owner/header.
// No pool-reuse, complete D618B0 owner, canonical admission or cleanup proof.
void* construct_native_unnamed_volume_texture_base_00b340a0(
    void* actual_owner, void* borrowed_com, std::uint32_t flags,
    std::uint32_t& actual_shared_serial_0108d6e8);

} // namespace bsp
