#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_radix_update {

// Complete 00C54DA0..00C55A55, descriptive name hypothesis. Original ECX
// manager, no stack arguments, plain RET. This explicit new C++ interface
// borrows actual raw Win32 manager storage and genuine new/free/malloc CRT
// custody, plus the canonical memset feature cell and native half constant.
// Live 00D7A280 bytes are 000000000000E03F (double 0.5); no substitute cell,
// provider, object, class, vtable or global is created by this implementation.
//
// Retains the native SSE/x87 instruction schedule, fixed 1040h probed frame,
// three uint32-wrapping 16-aligned dynamic stack allocations, stable four-byte
// radix passes, pair sweep, pending appends and endpoint-index publication.
// Returns observed final RDTSC EAX bits, without claiming a semantic clock API.
//
// Requires MSVC Win32 and its real _alloca_probe/_alloca_probe_16 runtime,
// valid native object/ring/count/tag invariants, native ambient x87/MXCSR/DF
// state, and actual CPU support. No heap scratch, default binding, guard,
// exception cleanup, original ABI adapter or activation is supplied. Context
// and qualified vendor bridges change call/fault locations and frame identity.
std::uint32_t rebuild_radix_pairs_00c54da0(
    void* manager, const cc15_dyn_live::DynLiveCrt& crt,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280);

}  // namespace bsp::cc16_dyn_radix_update
