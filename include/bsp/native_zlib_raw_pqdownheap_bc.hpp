#pragma once

// New MSVC Win32 raw stack-transfer interface. This is not an ordinary
// returning cdecl helper or a recovered Original C++ declaration/layout.
// The real frame at kernel_entry_stack supplies current continuation at +0
// and the actual mutable index word at +4. The body uses real scratch/save
// cells below that address, which may alias backing, tree or control storage.
// No frame/index/scratch is copied, initialized, checked or repaired.
//
// MOV loads backing into EAX, tree into EDI and this frame address into ESP,
// then JMP enters the complete private register body. Its actual RET4 exits
// use the current continuation and leave ESP=entry+8 and ECX=current scratch.
// Other CPU/memory effects, including malformed bits and actual saved cells,
// follow the full instruction schedule in docs/GOAL16R_ZLIB_RAW_PQDOWNHEAP.md.
// Source-only noreturn describes this transfer, not Original no-return state.
// Ordinary C++ return, caller adoption, SEH/unwind and gameplay are unproved.
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_pqdownheap_frame_00bcde90(
    void* backing, const void* tree, void* kernel_entry_stack) noexcept;
