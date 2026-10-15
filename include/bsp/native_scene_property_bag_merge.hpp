#pragma once

#include "bsp/native_scene_property_clone.hpp"

#include <cstdint>

namespace bsp {

// Complete ordinary successful-domain reconstruction of [008F54F0,008F566F),
// 383 bytes/131 instructions. The descriptive name is a hypothesis. Original
// ECX=destination, stack=source/keep DWORD, RET8; this new C++ context-bearing
// interface supplies no register/stack/FS/SEH/EH/OOM or game/class ABI.
//
// Borrow distinct actual raw61-produced114h bags with consistent maps at+4,
// count/64heads, real live14h nodes and canonical owning8B pooled keys. Records
// and their owned payloads have actual raw-constructor provenance; phase words
// alone do not establish it. Source/destination ownership graphs are finite,
// unshared, acyclic and disjoint, with compatible matching tags on collisions.
// Source roots, nodes, keys, payloads and frames survive all destination frees.
// No self/ancestor/shared-storage alias, invalidation, mutation or reentry.
//
// Context is the SAME live initialized canonical string/physical-pool domain
// already used by clone, insertion and retirement. Closed stable NUL-free ASCII
// keys include empty; lengths/length+1 do not overflow and CRT locale is C.
// Keep canonical fallback cells, key-pool generations/membership and return
// gates live. Require DF0, sufficient recursion stack, successful providers,
// and no invalid memory, overflow, concurrent mutation/trim or provider throw.
// All current whole clone and assignment payload contracts apply: positive
// nonwrapping array spans/clone strides and sole-owned assignment arrays whose
// +2C flag is already1. At EVERY Type1 conversion require empty x87 stack,
// six masks1, valid PC24/53/64 and any RC, no pending unmasked exception. Actual
// FLD/FSTP behavior is delegated unchanged, with no FP state reset.
//
// Allocate/free a real12B iterator even for an empty source. Each query is a
// real8B raw pooled header. Capture a hit's mapped record BEFORE returning the
// query, then branch. Destination Type6 recurses BEFORE the keep test, using
// the same full keep DWORD/context; no source-tag guard is introduced. Other
// hits assign only when the low keep byte is zero. On miss call whole clone,
// physically reread iterator+4/node+8/source+34 and store clone+34 AGAIN, then
// fetch the current key and call whole insertion. No alias, ordinal, zero-size
// or allocation-failure repair and no exception cleanup are added. Untouched
// metadata retains each underlying provider's literal behavior.
void merge_native_scene_property_bag_008f54f0(void* actual_destination_bag,
    const void* actual_source_bag, std::uint32_t keep_existing,
    NativeScenePropertyCloneContext& context);

} // namespace bsp
