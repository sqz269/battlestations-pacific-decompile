#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene-map iterator operations require MSVC Win32.
#endif

namespace bsp {

// Complete 004678F0..00467978. Native ECX=actual8h {owner,node} iterator,
// RET, no defined result. Nodes have left+0,parent+4,right+8,Isnil+19h.
// The current CRT invalid-parameter provider may RETURN. Continue after the
// initial owner check; tail-check failures return after the handler without
// any extra node publication. Preserve intermediate iterator.node writes.
// No bounds/sentinel/owner overlay, rollback or new host callback is added.
void __fastcall decrement_native_scene_record_map_iterator_004678f0(
    void* actual_iterator);

// Complete12-byte wrappers. Native ECX iterator, RET, EAX same iterator after
// the child returns. The captured pointer is returned even after handler work.
void* __fastcall decrement_native_scene_record_map_iterator_00468200(
    void* actual_iterator);
void* __fastcall decrement_native_scene_record_map_iterator_004688e0(
    void* actual_iterator);

// These signatures retain the observed ECX/RET shape. Original CRT selection,
// private frames, returned flags/register residue and game behavior are not
// established by this Source implementation. Evidence: companion packet doc.
} // namespace bsp
