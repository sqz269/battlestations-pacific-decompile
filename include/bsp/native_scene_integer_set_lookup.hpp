#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer-set lookup requires MSVC Win32.
#endif

namespace bsp {

// Complete00468DB0..00468E18 (105B/39ops), retained library name
// STL_inst_00468db0. Native ECX actual tree, unused EDX, stacked output8/key4;
// RET8; EAX actual output. Find is a behavior-derived hypothesis. The tree has
// head+4;14h nodes have left0,parent4,right8,signed keyC,color10,nil11.
// Read head,root,nil before any key read. A nonnil root loads the search key
// once for the descent. Reload CURRENT head afterward; only a candidate that
// differs from that head rereads CURRENT key before candidate+C. Capture both
// selected iterator words before ordered output+0 owner,output+4 node stores.
void* __fastcall find_native_scene_integer_set_00468db0(void* actual_tree,
    std::uint32_t unused_edx, void* actual_output8, const void* actual_key4);

// Ordinary Source over actual reached storage; no STL container, comparator,
// callout, allocation, validation, native diagnostic or initial-node shortcut.
// Payload/tree/key/output may alias when every reached access remains valid.
// Source does not expose implicit aliases with native argument homes/private
// iterator temporaries, asynchronous mutation, original register/SEH/fault
// identity, or game behavior. Physical slots alone do not prove binary ABI.

} // namespace bsp
