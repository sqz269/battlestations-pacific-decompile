#pragma once

#include "bsp/native_crt_sbh_state.hpp"

#include <cstdint>

namespace bsp {
// Complete C11D68 native body: 788 bytes, cdecl(descriptor,payload), no
// meaningful EAX return. Evidence: NATIVE_CRT_SBH_FREE_BW.md, accepted
// discovery b7fc7f55138b4f39c2ee6b7c94d56602f8dbd1f1.
// Require actual canonical SBH cells, native descriptor/tags/rings and mapped
// writable extents, actual bootstrap and external lock-4 ownership. The
// separate feature reference binds actual0109EEA4 for the complete memmove
// provider. DF=0 is a caller precondition, including scalar forward REP paths.
// Any selected vector path additionally requires actual CPU/OS SSE2 support.
// Preserve current-state reloads, real API failure publications and overlapping
// descriptor compaction. No heap/state, lock/SEH, validation or rollback owner.
// Added source references and changed compiler/argument-slot scratch are not
// native ABI, frame, SEH/FH3 or asynchronous-fault identity. Copy extents must
// not alias active source/provider frames or their live argument/return slots.
void free_native_sbh_block_00c11d68(
    void* actual_descriptor, void* actual_payload,
    const NativeCrtSbhState& state,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
