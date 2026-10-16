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

namespace bsp {
struct NativeIntPointerTree18InsertResult;

// Whole AD4AA0[189B/73starts]: original ECX actual Font/tree BASE, stack
// output/pair, EAX SAME output, RET8; no local EH. New ordinary Source ABI.
// Reuses the genuine shared unique driver, actual168 predecessor and176 link.
// BASE is opaque00/head04/unsigned element-count08, with genuine18h nodes;
// it is not adjusted by8 and its element count supplies no lifetime barrier.
//
// Required objects are the existing actual Pair8 and Result12 types. All FOUR
// pair-key bytes must be defined for the later allocator DWORD copy; preserve
// the upperWORD preimage when producing a Font key. AD5055 alone writes only
// the lowWORD. No WORD-struct/semantic Font cast or normalized pair is used.
// Search captures unsigned incomingWORD once, only for a nonempty root, then
// reads current nodeWORD each step. After real predecessor, read candidateWORD
// BEFORE CURRENT incomingWORD. Pair remains live/current for the real allocator.
// Publish volatile ownerDWORD/nodeDWORD/insertedBYTE; preserve output09..0B.
//
// Caller independently retains the actual initialized same-SDK Font/head/node
// graph, pair/output and unowned glyph/resource backing through allocation,
// new/invalid handlers and failure. Existing valid public-overlap/object-lifetime
// contracts apply; exclude BOTH callees' private stack/argument/scratch aliases.
// Externally serialize; valid current mutations keep all reached operands live.
// The private cachedWORD/iterator and Source argument identities do not claim
// Native private frame/register/argument-load order. SDK nonnull-or-throw and
// length-error transport remain genuine Source boundaries, not Native CRT/heap/
// encoded-handler/RTTI/FH3/SEH/fault ABI. No catch, mapped-value destruction,
// cleanup, rollback, retry, owner, protective retain or extra count is added.
// Whole Font loading, production admission and Native execution remain separate.
NativeIntPointerTree18InsertResult* __cdecl
insert_native_font_tree_unique_00ad4aa0(void* actual_font_base,
    NativeIntPointerTree18InsertResult* actual_output12,
    const NativeIntPointerTree18Pair* actual_pair8);
} // namespace bsp
