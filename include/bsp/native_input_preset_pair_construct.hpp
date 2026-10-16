#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-pair construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0069E6E0..0069E742:99B/37instructions. Established descriptive
// analysis name STL_StringPresetPair_Construct_0069E6E0 is preserved.
// Native ECX actualdestination, EDX unconsumed, stack(keyheader,mapped5DWORDs),
// RET8/EAX captured destination. Source additionally borrows actualRawCtx and
// actualcanonical0109EEA4 reference:RET10h/new interface, no binaryABI claim.
// Keyheader is length0/data4; mapped five opaque DWORDs occupy destination8..18.
void* __fastcall construct_native_input_preset_pair_0069e6e0(
    void* actual_destination, std::uint32_t unused_edx,
    const void* actual_key_header, const void* actual_mapped5,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);

// Private volatile discriminator captures self-key BEFORE zeroing length/data.
// Its private predicate spill/frame/flags are Source qualifications. Self-key remains
// empty; no repair, source snapshot or prior-buffer release. Nonself uses
// CURRENTkeylength after zeros/actualRawresize(preserve1), then CURRENTkeylength
// again; capture CURRENTdestlength/keydata/destdata in that order before actual
// BF7680, even for zero count. Five mapped read/store pairs stay interleaved,
// preserving representation bits and alias effects, with no 20-byte snapshot.
// Keyheader DWORD accesses are volatile; mapped copies use ordinary byte
// representations through separate4B temporaries, never typed payload values.
// No guard, cleanup or rollback; child failure preserves completed writes.
// Existing Rawresize retains its accepted currentCRT
// internal-copy policy. Actual BF7680 requires realfeaturecell truth/lifetime,
// DF0/SSE2selected/validnonwrapextents/liveframe constraints. Privateargument/
// frame/registerflags/OriginalABI/CRT/fault/FH3SEH/heap/runtime/game unproved.
} // namespace bsp
