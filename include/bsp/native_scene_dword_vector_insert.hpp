#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DWORD-vector insertion requires MSVC Win32.
#endif

namespace bsp {
// Native N-insert entry+4/+8/+C/+10h: unread iterator owner, position,
// count, value-pointer. Bind actual live writable cells, not snapshots. The
// value-pointer cell is replaced with its pointed DWORD BEFORE any vector
// access, including count0. On an in-place path the count cell becomes count*4.
// Binding metadata is stable and disjoint from cells/payload. All reached
// accesses/releases must remain valid; physical private-frame identity is open.
struct NativeSceneDwordVectorInsertNFrame {
    void* actual_parameters16;
};

// Complete 0046AFB0. Native ECX raw vector16, RET10h, no semantic result
// established by its sole caller. Storage+4/+8/+C supplies begin/end/capacity;
// +0 is untouched directly. Preserve captured begin/count, repeated current
// end reads, full unsigned growth arithmetic, allocation/copy/fill/free order,
// both in-place paths, and actual argument-cell mutations. No rollback added.
void insert_native_scene_dword_vector_n_0046afb0(void* actual_vector16,
    const NativeSceneDwordVectorInsertNFrame&);

// Native single-insert entry+4/+8/+C/+10h: output, owner, position,
// value-pointer. Child metadata/backing is needed only when execution reaches
// the real N-insert preparation after initial returning diagnostics. Native
// unused private word at entry-4 needs no Source backing.
struct NativeSceneDwordVectorInsertOneFrame {
    void* actual_parameters16;
    const NativeSceneDwordVectorInsertNFrame* child_frame;
};

// Complete 0046B4A0. Native ECX vector16, RET10h/EAX current output pointer.
// Retain captured owner/begin and index across returning CRT diagnostics. The
// empty/zero-distance entry skips owner validation. Child count is1. After the
// call, publish captured new begin to the actual position cell, validate the
// computed result, then publish output NODE followed by OWNER.
void* insert_native_scene_dword_vector_one_0046b4a0(void* actual_vector16,
    const NativeSceneDwordVectorInsertOneFrame&);

// New explicit raw interfaces over actual providers, without nominal element
// ownership or an adapter collection. Reached cell/payload aliases are visible;
// Source/native argument-home coincidence, original register/stack/CRT/heap/
// FH3/fault ABI, application adoption and game behavior remain unproved.
} // namespace bsp
