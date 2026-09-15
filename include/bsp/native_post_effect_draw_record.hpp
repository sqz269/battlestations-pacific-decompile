#pragma once

#include <cstddef>

namespace bsp {

struct NativeTracelineRenderAccess;

inline constexpr std::size_t native_post_effect_draw_record_bytes = 0x28;

// Complete B51BD0..B51C10. Native ECX is the caller's raw40-byte allocation;
// stack: leading, section, geometry, model, camera, visibility; EAX is the same
// allocation; RET18h. Source adds only the existing borrowed EDX access.
// Preserve the native x87 float copies, positive-zero depth and flags555h before
// entering the concrete raw B51A20 initializer. No allocation, retain, free,
// companion, or initialization of the existing sort-key bytes+20h/+24h occurs.
// B51A20 requires live original section/model/camera storage and the established
// NativeTracelineRenderAccess service domain. No null fallback or new callback
// interface is supplied. No binary ABI, hardware-fault unwind or game claim.
void* __fastcall construct_native_post_effect_draw_record_00b51bd0(
    void* actual_allocation, const NativeTracelineRenderAccess*, float leading,
    void* actual_section, void* actual_geometry, void* actual_model,
    void* actual_camera, float visibility);

} // namespace bsp
