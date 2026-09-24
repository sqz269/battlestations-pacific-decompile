#pragma once
#include "bsp/native_node_construction.hpp"

namespace bsp {

// Complete B7C4C0 (182 bytes) and B7C6B0 (25 bytes). New source interfaces for
// native ECX=actual slot, stack raw 8-byte name header, EAX=same slot, RET4.
// Requires aligned caller-owned storage covering the actual 1F0h directional
// pool slot. Reuses the genuine B6F5A0 raw-name constructor on that SAME prefix;
// no allocation, copied count, companion admission or implicit slot return.
//
// B7C4C0 reads CURRENT node_constants.one only AFTER B6F5A0 returns. It captures
// that word once, then reads CURRENT CE7820 after zeroing +1C4/+1C8/+1CC.
// The pointed constant cells and raw name may alias accessible native storage;
// their addresses, pool context and source interface arguments must stay valid.
// Private source frames/argument cells may not alias storage written by a call.
// Preserve +194..1A3,+1B4..1C3,+1DC..1EB and pool ID +1EC..1EF unchanged.
// The derived wrapper changes only the profile after successful base return.
//
// B6F5A0 performs its existing one-shot prefix cleanup on C++ failure; neither
// native wrapper has another cleanup handler. The caller owns physical slot
// recovery, including AC59A0's construction-only B7B610 state. On success the
// raw tail has no added C++ host members/lifetime or lighting view. Actual
// runtime admission, terminal destruction, native FH3/outer ABI, application
// graph closure and gameplay are separate boundaries.
void* construct_native_light_raw_00b7c4c0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants& node_constants,
    const volatile std::uint32_t& sixty_four_00ce7820);
void* construct_native_directional_light_raw_00b7c6b0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants& node_constants,
    const volatile std::uint32_t& sixty_four_00ce7820);

} // namespace bsp
