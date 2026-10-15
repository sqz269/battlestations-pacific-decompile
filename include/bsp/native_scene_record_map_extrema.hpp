#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene record map extrema require MSVC Win32.
#endif

namespace bsp {

// Complete functional00466A00: executable ranges00466A00..0C and00466A10..1B,
// 25 bytes. Envelope28B includes3 unreachable alignment bytes at00466A0D..0F.
// Native ECX actual node, EDX scratch, no stack arguments, plain RET/EAX node.
// First read node+8 and test THAT child's sentinel+19, without testing the
// initial node's sentinel. While child.nil==0, advance node=child and repeat.
// Return current node, never the terminal sentinel child. No state writes.
void* __fastcall maximum_native_scene_record_map_node_00466a00(void* actual_node);

// Complete00466A20..3A[27 executable bytes], including6B reachable neutral
// LEA EBX,[EBX] at00466A2A..2F. Same ABI/traversal, selected child=node+0.
// Source omits the neutral instruction; all external read points are retained.
void* __fastcall minimum_native_scene_record_map_node_00466a20(void* actual_node);

// Ordinary Source over actual links0/8 and sentinel byte19, no node overlay,
// allocation/provider/callback, owner conversion, validation or cycle guard.
// A sentinel input still reads its selected child and can descend through it;
// no positive/non-sentinel-only domain is introduced. Valid taken backing is
// required. Current void pseudocode discards the actual EAX result consumed by
// both known callers; Source returns that result without changing live Ghidra
// prototypes. Correct native library names are preserved; interpretations are
// body/layout-derived hypotheses. No original mangling/binary/private-frame,
// fault-address, direct native execution or game equivalence is established.

} // namespace bsp
