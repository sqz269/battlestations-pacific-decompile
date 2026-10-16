#pragma once

#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
// 00BCE180 scan_tree: new Source control-transfer interface, not ordinary return.
// Three real inputs select EAX=raw writable tree, ECX=max_code bits and actual
// kernel-entry ESP. [entry ESP+4] supplies the backing pointer, then becomes the
// actual mutable cursor. The frame must contain a genuine execution continuation
// and stack protocol; aliases may change its local, saved and control cells.
// The adapter adds three ordered stack reads; MOV/MOV/MOV/JMP preserves flags
// and incoming non-parameter registers before the complete private RET4 body.
// Source-only noreturn does not change Original scan_tree no_return=false.
// No Original ABI/type/layout, ordinary C++ return, caller adoption, SEH/unwind
// or game validation is claimed. Full physical schedule: GOAL16U documentation.
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_scan_tree_frame_00bce180(
    void* tree, std::int32_t max_code, void* kernel_entry_stack) noexcept;
#endif
