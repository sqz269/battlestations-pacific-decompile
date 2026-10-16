#pragma once

#include <cstdint>

namespace bsp {

// Whole00552340..00552394 (85B/30 instructions), right rotation.
// Whole00553F00..00553F50 (81B/30 instructions), left rotation.
// Native ECX actual tree, incoming EDX unused, one stacked node, RET4.
// EAX remains the pivot on native normal exits; callers do not consume it as
// a semantic result. These Source interfaces deliberately return void.
void __fastcall rotate_native_input_device_tree_right_00552340(
    void* tree, std::uint32_t unused_edx, void* node);
void __fastcall rotate_native_input_device_tree_left_00553f00(
    void* tree, std::uint32_t unused_edx, void* node);

// Actual tree head is DWORD+4; node links are DWORD0/4/8 and nil is byte99h.
// Require valid writable reached storage, including pivot/child/sentinel and
// parent/head links. No null/nil-start/ownership guard is inserted.
// Each body retains the pivot-child reread AFTER the node-child store, current
// parent publication, once-captured head, reloaded parent and native side-test
// order. Right rotation compares parent.right first; left compares parent.left.
// No callback, context, allocator, local EH or generic direction helper.
// Separate inline assembly preserves raw DWORD/byte access order. Compiler
// private frames/register saves, flags/residue, faults/SEH and native binary
// ABI or game behavior are not established by these new C++ interfaces.
} // namespace bsp
