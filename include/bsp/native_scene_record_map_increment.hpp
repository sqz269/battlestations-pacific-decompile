#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene-map iterator increment requires MSVC Win32.
#endif

namespace bsp {

// Complete 00467980..004679E2. Native ECX=actual8h {owner,node} iterator,
// RET, no defined semantic result. Nodes have left+0,parent+4,right+8,nil+19h.
// The concrete current CRT invalid-parameter provider may RETURN: continue
// after a zero owner, but return immediately after reporting a nil node.
// Ascent publishes each intermediate node before reading its parent and
// rereads the CURRENT iterator.node before each parent-right comparison.
// Every raw access reached requires valid storage; owner/nil failures are
// retained rather than made preconditions. No callback or overlay is added.
void __fastcall increment_native_scene_record_map_iterator_00467980(
    void* actual_iterator8);

// Complete12-byte wrappers. ECX=iterator, RET, EAX=captured iterator pointer
// after the child returns, including a returning invalid-parameter handler.
void* __fastcall increment_native_scene_record_map_iterator_004681f0(
    void* actual_iterator8);
void* __fastcall increment_native_scene_record_map_iterator_004688d0(
    void* actual_iterator8);

// Observed ECX/RET shape is retained. Original CRT policy, private frames,
// flags/ambient registers and game behavior are separately qualified in the
// packet evidence; this is not a drop-in original-binary replacement.
} // namespace bsp
