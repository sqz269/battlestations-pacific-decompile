#pragma once

namespace bsp {

// Actual scalar/int-key tree18 leaves. Address names are evidence-backed
// hypotheses; these entries consume caller-owned raw Win32 storage directly.
// Iterator: owner DWORD+0, node DWORD+4. Tree: head DWORD+4. Node: left+0,
// parent+4, right+8, color BYTE+14h, nil BYTE+15h, extent18h.
// No typed node/owner model, context, allocation, payload, or repair is added.
//
// 0069B190, whole137B/48 instructions: ECX actual8B iterator, entryEDX
// unconsumed, no stack input/plainRET, semanticvoid/incidentalEAX. The real
// current-SDK BF6713 Source provider may return, throw, or terminate. Each
// diagnostic tail restoresESI and has no later owned action if it returns.
// Original encoded-handler CRT/FH3/SEH/fault/runtime/game behavior is unproved.
void __fastcall decrement_native_input_scalar_tree_iterator_0069b190(
    void* actual_iterator, void* unused_edx);

// 005530D0, whole78B/30 instructions, and00553120, whole82B/30 instructions:
// ECX tree, entryEDX unconsumed, stackedpivot DWORD, three physicalRET4 exits.
// Semanticvoid; incidentalEAX is the retained promoted node. Exact raw load/
// store order includes rereading the promoted near link AFTER the pivot-link
// store, current pivot.parent AFTER the child-parent store, current tree.head
// AFTER the promoted-parent store, and pivot.parent again on nonroot paths.
// ESI/stack restoration and emitted instruction/flag schedules are retained;
// caller stack/DF and storage lifetime/accessibility are inherited. These
// compiled Source symbols are not original-address binary replacements and
// do not establish Original fault PCs, SDK exception transport, or runtime ABI.
void __fastcall rotate_left_native_input_scalar_tree_005530d0(
    void* tree, void* unused_edx, void* pivot);
void __fastcall rotate_right_native_input_scalar_tree_00553120(
    void* tree, void* unused_edx, void* pivot);

} // namespace bsp
