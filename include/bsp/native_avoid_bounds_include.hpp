#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw bounds inclusion requires MSVC Win32 x87 and MOVSS.
#endif

namespace bsp {
// Complete00415010..0041508E,127B. Original ECX raw4float bounds and one
// stacked float2 point pointer; RET4, no semantic EAX result. Source EDX is
// an unused binding word, not an Original input. Capture point in EDX then
// reuse the ACTUAL incoming point word for each x87 float spill. Reload point
// before every ordered comparison; prior bounds stores can alter later reads.
// Equal/unordered comparisons skip each corresponding bound store. Preserve
// ambient FP control/status/traps, raw aliases and current stack-cell identity.
// All reached bound/point/control addresses require backing. No typed view,
// point snapshot, numeric repair or FP reset is added. Existing typed Source
// interfaces remain separate; Native object/frame/address ABI is not asserted.
void __fastcall include_native_avoid_bounds_point_00415010(
    void* actual_bounds_float4, void* unused_edx, const float* actual_point_float2);
} // namespace bsp
