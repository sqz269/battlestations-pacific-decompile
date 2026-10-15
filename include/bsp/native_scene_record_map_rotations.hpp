#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene record map rotations require MSVC Win32.
#endif

namespace bsp {

// Complete00467620..0046766D[78B], native ECX map, stack pivot, RET4; no
// semantic result. Source retains physical slots with explicit unused EDX.
// Left rotation promotes pivot+8. Capture promoted+0, store pivot+8, then
// REREAD promoted+0 for sentinel+19 and optional child+4 parent repair.
// Read current pivot+4 and publish promoted+4 BEFORE reading current map+4
// head/root. Root case writes captured head+4. Nonroot rereads pivot+4, then
// chooses parent+0 iff its current left==pivot, otherwise parent+8. Finally
// store promoted+0=pivot then pivot+4=promoted, in that order on every path.
void __fastcall rotate_left_native_scene_record_map_00467620(
    void* actual_map, std::uint32_t unused_edx, void* actual_pivot);

// Complete00467670..004676C1[82B], same native/Source input slots and RET4.
// Right mirror: promote pivot+0; transfer/reload promoted+8; optional child
// parent repair; publish promoted parent before current head/root lookup.
// Nonroot chooses parent+8 iff its current right==pivot, otherwise parent+0.
// Final ordered stores: promoted+8=pivot, then pivot+4=promoted.
void __fastcall rotate_right_native_scene_record_map_00467670(
    void* actual_map, std::uint32_t unused_edx, void* actual_pivot);

// Ordinary Source over actual Win32 DWORD/byte storage, with every external
// reload and ordered write retained even for aliasing storage. No copied node,
// STL replacement, callback/provider, allocation, added validation, color/key
// access or implicit owner is introduced. No noexcept or EH metadata claim.
// Valid taken backing remains required. Private argument/frame aliases,
// original mangling/binary identity, native fault addresses and game behavior
// are outside this Source interface; original ABI compatibility is unproved.
// Left/right names are body/layout-derived hypotheses; established native
// library names STL_Tree_Rotate_00467620/00467670 are preserved.

} // namespace bsp
