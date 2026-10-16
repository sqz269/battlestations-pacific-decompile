#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-key subtree requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A0770..006A07C1,82B/34 current starts. Native ECX tree, EDX
// unconsumed, stack node/RET4; no semantic result. Added borrowed RawCtx
// makes Source RET8. Actual2Ch node: left/right0/8, key length/data+C/+10,
// nilBYTE29. Tree is forwarded through right recursion, never dereferenced.
// Capture key data BEFORE left after recursion; only nonnull data reads
// CURRENT length+1 (DWORD wrap), resolves actual419CC0 and returns BD1510.
// Captured left survives getter/pool effects. Free current node, then read
// captured left's nil byte. Do not clear key/mapped payload or head/count.
void __fastcall destroy_native_input_preset_key_subtree_006a0770(
    void* tree, std::uint32_t unused_edx, void* node,
    NativeStringRawPoolContext& strings);

// Valid reached nodes, raw publication/shutdown cells and matching currentCRT
// ownership are required. Getter failures propagate before current-node free;
// no cleanup/rollback/EH guard or noexcept string-storage bridge is added.
// Actual return/free providers retain their existing noexcept boundaries.
// New Source ABI/private frames, unused EDX canonicalization, original CRT,
// FH3/SEH/RTTI/fault/register identity and gameplay remain unproved. Existing
// typed/AV key-tree and InputSettings projections are preserved separately.
} // namespace bsp
