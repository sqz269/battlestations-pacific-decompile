#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer set rotations require MSVC Win32.
#endif

namespace bsp {

// Complete 00467740..0046778D [78B] and 00467790..004677E1 [82B].
// Native ECX=actual tree, incoming EDX unused, stack pivot, RET4; no semantic
// result. Explicit unused EDX preserves physical input slots in Source.
// Actual 14h nodes: left+0, parent+4, right+8, color+10h, nil+11h.
// Each body transfers a child, RELOADS it for nil/parent repair, publishes the
// promoted parent, THEN reads current tree+4 head/root. Nonroot rereads the
// pivot parent. Final stores publish promoted child before pivot parent.
void __fastcall rotate_left_native_scene_integer_set_00467740(
    void* actual_tree, std::uint32_t unused_edx, void* actual_pivot);
void __fastcall rotate_right_native_scene_integer_set_00467790(
    void* actual_tree, std::uint32_t unused_edx, void* actual_pivot);

// Separate ordinary Source over actual DWORD/byte storage, retaining ordered
// external accesses for aliases. No record-map nil19h body, copied owner/node,
// callback, provider, allocation, validation, key/color access or STL container.
// Valid backing for each taken access is required. Left/right are descriptive
// body-derived hypotheses; native STL_Tree_Rotate library names are preserved.
// Native private-frame aliases, mangling, fault/SEH identity, binary ABI and
// game behavior are not proved by this Source interface or its build.

} // namespace bsp
