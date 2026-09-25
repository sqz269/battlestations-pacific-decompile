#pragma once
// 00AA1FE0 BSP_Gui_GetAspectExtent, __thiscall(float out[2]) with ECX = out,
// plain RET, body 00AA1FE0..00AA201A. Packet cc9_gui_extent_inputs,
// docs/GUI_EXTENT_INPUTS.md. Recomputed on every call from the platform
// singleton [0109CF04]: +0Dh the widescreen byte, +10h the active aspect.
// Names are descriptive hypotheses, not recovered symbols. A new C++
// interface, not ABI-compatible.

namespace bsp {

struct GuiAspectExtent {
    float width{1.0f};    // out[0]
    float height{1.0f};   // out[1]
};

inline constexpr float kGuiAspectNarrow = 1.3333334f;    // 00D5BD98, 0x3FAAAAAB
inline constexpr float kGuiAspectWide = 1.7777778f;      // 00D5BD9C, 0x3FE38E39
inline constexpr double kGuiAspectBase = 1.3333333730697632;  // 00CF5750 (double)

// a = 16/9 when widescreen, else 4/3 (MOVSS, then FLD float); out[0] =
// a / 4/3 by FDIVR against the double; out[1] = a / active aspect by FDIV of
// the float field; each FSTP float. The x87 quotient at the default 53-bit
// precision control rounds as the double division here does.
inline GuiAspectExtent gui_aspect_extent_00aa1fe0(bool widescreen,
                                                  float active_aspect) noexcept {
    const double a = widescreen ? kGuiAspectWide : kGuiAspectNarrow;
    GuiAspectExtent out;
    out.width = static_cast<float>(a / kGuiAspectBase);
    out.height = static_cast<float>(a / static_cast<double>(active_aspect));
    return out;
}

}  // namespace bsp
