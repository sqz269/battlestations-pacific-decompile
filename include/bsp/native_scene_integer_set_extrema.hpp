#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer set extrema require MSVC Win32.
#endif

namespace bsp {

// Complete functional 00466A40 body: 25 executed bytes in a 28-byte envelope;
// unreachable three-byte alignment at A4D..A4F is excluded. Native ECX initial
// node, EDX scratch, no stack arguments, plain RET, EAX current-node result.
// Read right+8 FIRST, then child.nil11; follow while nil is zero and return
// the last current node. Do not add an initial-node nil shortcut.
void* __fastcall maximum_native_scene_integer_set_node_00466a40(void* actual_node);

// Complete 00466A60..00466A7A [27 executed bytes], including reachable six-byte
// LEA EBX,[EBX] identity alignment. Same ABI/result; follow left+0 and nil11.
void* __fastcall minimum_native_scene_integer_set_node_00466a60(void* actual_node);

// Distinct raw14h-node Source; no record-map nil19h call, copied node, owner,
// STL container, callback, provider or validation. Valid backing is required
// for every taken access. Native library names are preserved; EAX consumed by
// actual callers corrects the void pseudocode interpretation in Source only.
// No live prototype repair, private-frame/mangling/fault/SEH/binary ABI or game
// proof follows from this interface, compilation or existing checks.

} // namespace bsp
