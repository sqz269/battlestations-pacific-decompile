#pragma once

#include "bsp/native_scene_dword_vector_push_back.hpp"

namespace bsp {

// Stable Source metadata is disjoint from the actual mutable homes/payload.
// actual_parameter4 binds the readable LIVE original destination-pointer home.
// Each reached append rewrites the SAME actual_push_parameter4 with a Page-slot
// ADDRESS; Push owns the late pointer/value reads. Fallback requires the real
// output8 and One/N metadata with their actual homes/private backing. Valid
// home/payload aliases retain those effects; Source/native private stack
// addresses do not implicitly coincide.
struct NativeTacticalLibraryCollectFrame {
    const void* actual_parameter4;
    void* actual_push_parameter4;
    void* actual_push_private_output8;
    const NativeSceneDwordVectorInsertOneFrame* child_frame;
};

// Whole00509DA0 [55B/19ops]: native ECX actual Tactical5D0, one stacked vector
// pointer, RET4. Capture receiver/destination ONCE before the first child-home
// write. Append +4DC/+4E0/+4E4 addresses unconditionally through real raw Push;
// no Page snapshot, null skip, reference/destructor, rollback or new allocator.
// Caller must establish genuine retained Tactical/Page/GUI/creator and raw
// vector custody, same Source heap, all child preconditions and lifetimes.
// This binding does not authenticate them or accept a semantic screen cast.
// New C++ interface, unwired: ordinary C++ Push/One/N and the reached naked
// BF6713 diagnostic retain their distinct Source failure qualifications.
// Native register/stack/FH3/SEH/fault ABI and caller/game admission are separate.
void collect_native_tactical_library_children_00509da0(void* actual_tactical,
    const NativeTacticalLibraryCollectFrame&);

} // namespace bsp
