#pragma once

#include "bsp/native_scene_record_map_erase_range.hpp"

namespace bsp {

// The range's actual_parameters20 binds this destructor's five outgoing words.
// actual_range_output8 separately binds this destructor's private result8B;
// it is NOT implicitly the range's own partial-erase private output8B.
// Stable metadata is disjoint from all actual cells/payload it describes.
// Reached backing and releases must remain valid; explicit valid payload aliases
// are admitted, not implicit original physical stack/frame coincidences.
struct NativeSceneRecordMapDestroyFrame {
    const NativeSceneRecordMapEraseRangeFrame& range;
    void* actual_range_output8;
};

// Three complete52B bodies, identical except same-target relative CALL operands.
// Native ECX actual tree, RET, no semantic result (EAX is cleared to0).
// Capture head then minimum before outgoing stores in last-node,last-owner,
// first-node,first-owner,private-output push order. Call the COMPLETE actual
// range provider, retaining all returning-handler/partial paths and reached
// child backing. Then free CURRENT head, write head0 then count0; tree+0 and
// mapped objects are untouched. No local EH, rollback or ownership is added.
void destroy_native_scene_record_map_0046c500(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame&, NativeStringRawPoolContext&);
void destroy_native_scene_record_map_0046ced0(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame&, NativeStringRawPoolContext&);
void destroy_native_scene_record_map_0046ef70(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame&, NativeStringRawPoolContext&);

// New explicit Source frame/context interfaces; no original register/private
// frame/CRT/FH3/SEH binary ABI, startup execution or gameplay parity is claimed.
} // namespace bsp
