#pragma once
#include <cstdint>

namespace bsp {
struct NativeIntPointerTree18Iterator;
struct NativeIntPointerTree18Pair;

// Whole AD4600[492B/167 starts] aliases established86EBE0; its real18h node
// allocation/rotations and full one-state EH actions are proven in170. This
// ordinary Source __cdecl entry forwards SAME arguments to that genuine
// provider, with no second algorithm, key comparison or new86EBE0 credit.
// AD4AA0 unsigned-WORD unique search and whole Font load remain separate.
//
// Required actual initialized Font/tree BASE has opaque00/head04/count08.
// Use genuine existing Source iterator8B(owner0/node4) and pair8B objects,
// valid whenever reached. All FOUR bytes of pair.key_00 must be defined for
// its DWORD copy (including the Font key's upper padding WORD). The signed
// field is not compared/narrowed/normalized here. No unrelated WORD-struct,
// descriptor, semantic Font or owner/companion cast supplies these objects.
// Nodes use links0/4/8,colorBYTE14,nilBYTE15; trailing16/17 stay untouched.
// The mapped pointer is copied without retain/release/dereference; the glyph
// payload is a distinct20h allocation. No owner, default sentinel/service,
// early operand validation, rollback, cleanup or retry is added.
//
// Caller independently retains the real same-SDK graph, pair/output and
// payload/resource backing through allocation/new-handler calls and failure;
// externally serialize and keep all later reached operands valid on reentry.
// Keep the existing owningSDK nonnull-or-throw allocator and actual host
// length-error transport. No general callback-replaced-tree or failed-graph
// replay guarantee follows. Only valid public Source overlap is admitted;
// BOTH Native and Source callees' private stack/argument/scratch aliases are
// excluded. This does not reproduce Native ECX/RET10h, private argument-read
// timing/registers, heap/encoded handlers/catch/RTTI/FH3/SEH/fault ABI, game
// execution, populated Font ownership or normal-host activation readiness.
NativeIntPointerTree18Iterator* __cdecl link_native_font_tree_node_00ad4600(
    void* actual_font_base, NativeIntPointerTree18Iterator* output8,
    std::uint8_t insert_left, void* parent,
    const NativeIntPointerTree18Pair* pair8);
} // namespace bsp
