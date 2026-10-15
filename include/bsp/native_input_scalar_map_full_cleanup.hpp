#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Only the admitted full-selection prefix 00444B10..00444B8E (127B/49
// instructions). The existing ordinary range API and its partial branch are
// separate. Native ECX tree, EDX unused, stacked output/first-owner/first-node/
// last-owner/last-node, RET14h and EAX output. Added context is a Source service
// input, not an original register or stack parameter.
// Require valid exclusively owned actual storage, nonnull tree,
// first_owner==last_owner==tree, first_node==initial head.left, and
// last_node==fresh head; selection storage remains stable through selection.
// Output is the writable private 8B iterator used by the actual consumer.
// Raw TEST/CMP/load order is retained without inventing a diagnostic path for
// inputs outside this fragment. Child444760 uses the actual pooled-string
// context; getter failure propagates before remaining resets/output stores.
void* __fastcall clear_native_input_scalar_tree_full_prefix_00444b10(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings);

// Whole00445900..00445933, 52B/23 instructions. Native ECX actual12B header,
// unusedEDX, no stack inputs, plainRET and defined EAX0. Capture H thenH.left;
// private8B full-range result; free CURRENT head, clear+4 then+8, return0.
// Opaque+0 is untouched. No null/empty/repeated-destroy guards or rollback.
std::uint32_t __fastcall destroy_native_input_scalar_map_00445900(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Finite uniquely owned actual nodes/head and matching current-CRT allocation
// ownership are required. Existing qualified soldier-loop APIs are preserved.
// Source/context/private-frame changes do not prove native ABI, flags, original
// heap/RTTI/FH3/SEH/fault/stack-exhaustion identity or game behavior. No local EH
// registration or catch exists in either owned native scope.
} // namespace bsp
