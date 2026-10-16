#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw mapped GUI rectangle writing requires MSVC Win32.
#endif

namespace bsp {
// Whole ACF260[ACF260,ACF486),550B/170 starts. Four stack arguments, ECX
// unconsumed, RET10h; explicit Win32 stdcall Source entry. Writes expanded
// TL/TR/BR/TL/BR/BL positions and UVs directly into the actual mapped stream.
// Current stream08/0C/10/28, rectangle and index reads retain Native order;
// each index increment precedes that vertex's UV stores. No color/index-array,
// allocation, callback, capacity clamp, mapping or ownership is added.
//
// Caller retains genuine live actual stream/header/mapping and all input/index
// backing through completion. Every reached wrapped32-bit effective address
// must permit its access, including any admitted overlap feedback. No input,
// mapping or header may alias the callee's private stack/argument/scratch area.
// Source adds no admission check, fault recovery, FPU-control change or cleanup.
// The naked body preserves the observed x87 spill/reload and SSE bit stores;
// Original placement/caller adoption, arbitrary faults and game drawing remain
// separate evidence. It does not supply whole FrameBox geometry/state rebuild.
void __stdcall write_native_gui_rectangle_00acf260(void* actual_stream,
    volatile std::uint32_t* actual_index,
    const void* actual_position_rectangle, const void* actual_uv_rectangle) noexcept;
} // namespace bsp
