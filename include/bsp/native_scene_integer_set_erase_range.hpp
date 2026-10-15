#pragma once

namespace bsp {
struct NativeSceneIntegerSetEraseFrame;

// Stable metadata is disjoint from actual cells/payload. Outer20B binds
// output/first-owner/first-node/last-owner/last-node. Child metadata and its
// actual12B arguments are required only after a reached partial increment
// returns. The child owns its separate reached-only savedNODE4 requirement.
// Full/empty paths do not inspect child metadata or the private output8B.
struct NativeSceneIntegerSetEraseRangeFrame {
    void* actual_parameters20;
    void* actual_private_output8;
    const NativeSceneIntegerSetEraseFrame* child_frame;
};

// Complete0046B170:200 instruction bytes in201B envelope, unreachable NOP at
// 0046B1EF. Native ECX tree;stack output,first-owner,node,last-owner,node;
// RET14/EAX current output. Preserve head/min captures across returning CRT
// diagnostics, current argument reads, full clear and partial real increment/
// erase. Child push stores are oldNODE,oldOWNER,privateOUTPUT after increment.
// Full clear publishes tree then current minimum; empty exit uses captured
// owner/node. No nil19h/string provider, callback, stub or guard is introduced.
void* erase_native_scene_integer_set_range_0046b170(void* actual_tree,
    const NativeSceneIntegerSetEraseRangeFrame&);

// New explicit raw frame interface; all reached storage/releases remain valid.
// Actual cells/payload may alias, while metadata and bindings remain stable.
// Original physical private-stack/CRT/FH3/SEH/register/binary ABI, application
// admission and game behavior are separately qualified rather than implied.
} // namespace bsp
