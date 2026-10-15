#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input-device subtree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A7540..006A75E0 (161B/51 instructions). Native ECX is an opaque
// tree word forwarded only to right recursion, never dereferenced; EDX unused;
// one stacked node DWORD, RET4, no semantic EAX. Source adds stacked RawCtx.
// Actual9Ch nodes: links0/4/8, pair+C, record+14, color98, nil99. Test nil99;
// capture CURRENT right and recurse; then capture CURRENT left; destroy the
// complete pair at+C through accepted55B8B0; free original current node;
// test captured-left nil99 before selecting it as next current node.
// No tree/count/reset work, null guard, scalar flags or extra key guard.
void __fastcall destroy_native_input_device_subtree_006a7540(
    void* opaque_tree, std::uint32_t unused_edx, void* node,
    NativeStringRawPoolContext& strings);

// Whole native8B action00C7F5A0..00C7F5A7, parent-bound logical Source API.
// Native[EBP+4] ALREADY holds current pair=node+C: parent7580 overwrites its
// incoming node stack word with that pointer before state0/record cleanup.
// Native tail-JMP41DD20; Source takes the saved pair directly, never adds+C
// again and never invents an EBP reader. Direct Source use may propagate.
void unwind_native_input_device_subtree_key_00c7f5a0(void* saved_current_pair,
    NativeStringRawPoolContext& strings);

// Require finite uniquely owned valid actual nodes/sentinel, actual borrowed
// raw-pool cells and matching current-CRT allocations. Accepted pair cleanup
// already owns CURRENT-key unwind: do not wrap it in another key guard.
// Record/right/key-getter failures stop later work; normal key failure after
// disarm skips node free/left iteration. Secondary C++ unwind failure follows
// the accepted CurrentCpp termination policy. No NativeStringStorage bridge,
// callbacks or host model is added; older real AV domains remain separate.
// Added context/by-value inputs/private frames and native argument overwrite
// are not original FH3/SEH/RTTI/heap/CRT/fault/stack-exhaustion/register/flags/
// exception identity, binary ABI, runtime or game validation.
} // namespace bsp
