#pragma once

#include "bsp/native_scene_record_map_erase.hpp"

namespace bsp {

// Stable metadata is disjoint from all actual cells/payload. Outer20B backing
// is output/first-owner/first-node/last-owner/last-node. Child metadata and its
// actual12B argument/saved-map backing are required only after a reached partial
// iteration's increment returns. Full/empty paths admit null child metadata and
// unbacked private output. No original physical private-stack aliases are implied.
struct NativeSceneRecordMapEraseRangeFrame {
    void* actual_parameters20;
    void* actual_private_output8;
    const NativeSceneRecordMapEraseFrame* child_frame;
};

// Whole0046B570:200 instruction bytes in201B envelope, excluding unreachable
// NOP0046B5EF. Native ECX map; stack output,first-owner,node,last-owner,node;
// RET14h/EAX CURRENT output. Initial owner precedes head/min capture and report;
// initial node is read afterward. Full match clears CURRENT root/head/count,
// captures current minimum BEFORE current output and publishes map then node.
// Partial iteration increments actual outer first iterator, prepares child args
// OLD node/OLD owner/private output in push order, then erases. Reload outer
// node then owner after child returns. Empty exit publishes captured registers.
// All current-CRT report-return paths remain. No snapshots, guards or callbacks.
void* erase_native_scene_record_map_range_0046b570(void* actual_map,
    const NativeSceneRecordMapEraseRangeFrame&, NativeStringRawPoolContext&);

// New explicit frame/context Source interface. Reached native-layout backing
// and matching current-provider allocations must remain valid. Original CRT,
// private frames/SEH/registers/binary ABI, application and game proof are open.
} // namespace bsp
