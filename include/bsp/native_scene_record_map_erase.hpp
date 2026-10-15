#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene-map iterator erase requires MSVC Win32.
#endif

namespace bsp {

// Explicit backing for native output/owner/node stack cells at entry+4/+8/+C,
// and the saved-map DWORD at entry-54h. Bindings and their addresses are stable;
// the frame metadata itself is disjoint from all cells/payload it describes.
// Parameter/saved-map/payload backing may alias when every reached access and
// release is valid. No native private-frame coincidence is supplied implicitly.
struct NativeSceneRecordMapEraseFrame {
    void* actual_parameters12;
    volatile std::uint32_t& saved_map;
};

// Whole0046A470..0046A73B plus Source temporary-unwind scope00C61C70.
// Native ECX map; stacked output,owner,node; EAX CURRENT output; RET0Ch.
// Source adds explicit frame backing/raw-pool context; it is not that ABI.
// Preserve captured erased node, CURRENT parameter node after increment,
// saved-map reloads, ordered rotation repair, raw key return/node free,
// current unsigned count and final owner->output->node reads before writes.
// Nil input throws existing owning D6926C current-C++ transport. A returning
// current CRT invalid-owner handler remains admitted through the child.
void* erase_native_scene_record_map_iterator_0046a470(void* actual_map,
    const NativeSceneRecordMapEraseFrame&, NativeStringRawPoolContext&);

// Current CRT allocation/handler/C++ EH boundaries are concrete. Original
// RTTI/catch identity, FH3/SEH/private frames, flags/ambient registers, binary
// replacement and game behavior remain separately qualified in packet docs.
} // namespace bsp
