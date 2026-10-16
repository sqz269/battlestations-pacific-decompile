#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Whole defined006A7AA0..006A7B1E +006A7B20..006A7B68:200B/76starts.
// Undefined90 padding006A7B1F is not promoted. Native ECXtree/EDXunused;
// stacked output/firstowner/firstnode/lastowner/lastnode, EAXoutput/RET14h.
// Added borrowed RawCtx changes Source ABI to six stacked words/RET18h.
// Capture head.left BEFORE firstowner diagnostic, firstnode AFTER it; only
// matching firstnode captures lastowner/currenthead before last diagnostic,
// then reads currentlastnode. Full path repeats actual head/root/reset loads.
// Partial loop short-circuits currentlastowner read on cachedowner0, compares
// cachednode with currentlastnode AFTER diagnostics, advances mutable8B current
// BEFORE erasing cachednode, ignores child output, reloads node THEN owner.
// Output owner precedes node. No repeated admitted-prefix call, new guard,
// rollback, normal cleanup/EH, callback/provider, count or ownership bridge.
void* __fastcall erase_native_raw_input_device_tree_range_006a7aa0(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings);

// Requires valid reached actual tree/node/output and matching rawpool/currentCRT
// ownership. Actual subtree, iterator, eraser and diagnostic Source are composed;
// normal C++ failures propagate with their established progression/termination.
// CurrentSDK diagnostic/CurrentCpp transport, new helper ABIs, Source private
// iterator/argument frames, ambient registers/flags, original CRT/FH3/SEH/RTTI,
// faults and game identity remain qualified; no binary/runtime proof claimed.
// Existing127B admitted prefix and actual AV/full/partial domains stay separate.
} // namespace bsp
