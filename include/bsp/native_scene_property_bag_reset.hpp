#pragma once

#include "bsp/native_scene_property_clone.hpp"

namespace bsp {

// Whole ordinary successful-domain 008F5470..008F54E5 (117B/45 instructions).
// The name is a hypothesis. Borrow an actual raw61-produced destination bag;
// clear its map at +4 through the SAME canonical context, preserving bag phase,
// owner+110 and the existing ordinal counter+10C (not resetting it). Full
// insertion can increment that counter for incoming ordinal-zero records.
// A null source means clear-only.
// For distinct nonnull source, both finite unshared owning graphs and payloads
// must be disjoint. Source remains borrowed and unchanged. Exact self-reset
// follows the original clear-then-empty-iterator path, not a shortcut; other
// ancestor/descendant overlap is excluded because clear may destroy the source.
// Inherit the full clone/insertion/lifetime contracts: admitted actual raw
// producers, current heap, ASCII/C locale, live canonical pool generation,
// valid DF/ES and Type1 FP environment, sufficient stack and successful normal
// providers. No overflow, invalid memory, concurrency, trim or reentry.
// A nonnull source always uses a real 12B heap iterator, including empty/self
// source. Clone each record BEFORE obtaining the iterator key (E186ED role),
// then full insert and next; retire the iterator once. No cloneF89450 fallback,
// invented heads, host callbacks, counter reset or failure cleanup is added.
// New ordinary C++ ABI; no Original ECX/RET4/residual-register/FS/SEH/OOM,
// private CRT, class/vtable dispatch, startup/World or game qualification.
void reset_native_scene_property_bag_008f5470(void* actual_destination_bag,
    const void* optional_source_bag, NativeScenePropertyCloneContext& context);

} // namespace bsp
