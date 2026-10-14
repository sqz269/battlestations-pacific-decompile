#pragma once

// Leaf helpers from the 00600000-0066FFFF band (cc13-sweep60, docs/CC13_BAND_0060_0066.md):
// HUD screen presentation code. New C++ interfaces, not the native register ABI. Names are
// hypotheses, not recovered symbols; each declaration states the original address and ABI.
//
// x87 model (as in bsp/band_0045_cc13.hpp): the image runs with 53-bit precision, so an FSTP to a
// float slot is a float cast and unspilled intermediates are doubles.

#include <array>
#include <cstdint>

namespace bsp::band_0060_cc13 {

struct Vec2 {
    float x{};
    float y{};
};

// The blink state shared by the submarine screen (+1Ch byte, +20h float) and the plane HUD
// screen (+140h byte, +13Ch float).
struct PulseAlphaState {
    float value{};
    bool falling{};
};

// 006501A0 BSP_HudSubmarineScreen_PulseAlpha and 00604CF0 BSP_HudPlaneScreen_PulseAlpha,
// __fastcall(ECX screen), RET. value = float(value -/+ step) with the double step 00D7A2F8
// (0.019999999552965164); value <= 0 clears `falling`, value >= top sets it; the new value is
// returned for the widget's alpha setter (virtual +4Ch on +18h or +E0h), which stays with the
// caller. The submarine compares with the float 00CE3D30 (0.6f, COMISS), the plane screen with
// the double 00CE65D0 (0.4000000059604645, x87 compare); a NaN never sets or clears the flag.
inline constexpr double kPulseStep = 0.019999999552965164;      // 00D7A2F8
inline constexpr double kSubmarinePulseTop = 0.6000000238418579; // 00CE3D30 (float 0.6f)
inline constexpr double kPlanePulseTop = 0.4000000059604645;     // 00CE65D0 (double)
float pulse_alpha_step_006501a0(PulseAlphaState& state, double top) noexcept;

// 00658580 BSP_HudSupportManagerScreen_WorldToMap, __thiscall(ECX owner, Vec2* out, x, y, z),
// RET 10h. `map_size` is BSP_GuiWidget_GetSize of the support-manager screen's widget +1C0h
// ([00E198C4]+BCh), `lo`/`hi` are the owner's bounds (+460h/+468h and +46Ch/+474h). Each axis:
// t = float((v - lo) / (hi - lo)), out = float((size - 0) * t + 0). y is unused.
struct MapBounds {
    float lo_x{};  // +460h
    float lo_z{};  // +468h
    float hi_x{};  // +46Ch
    float hi_z{};  // +474h
};
Vec2 world_to_map_00658580(const Vec2& map_size, const MapBounds& bounds, float x, float z) noexcept;

// 00638B90 BSP_HudMarker_ClampToScreenMargins, __fastcall(ECX float* x, EDX float* y), RET.
// Margins are runtime globals: x in [00E197D8, 00E197D4], or [00E197D0, 00E197CC] when byte +0Dh
// of *0109CF04 (the wide-screen layout) is set; y in [00E197C8, 00E197C4]. Each axis: lo when
// lo > v, else hi when v > hi, else v (FCOMIP / JBE, so a NaN is kept).
struct ScreenMargins {
    float x_lo{};       // 00E197D8
    float x_hi{};       // 00E197D4
    float x_lo_wide{};  // 00E197D0
    float x_hi_wide{};  // 00E197CC
    float y_lo{};       // 00E197C8
    float y_hi{};       // 00E197C4
};
void clamp_to_screen_margins_00638b90(float& x, float& y, const ScreenMargins& margins, bool wide) noexcept;

// 00622E30 BSP_RewardsScreen_StepPage, __thiscall(ECX screen, int direction), RET 4.
// page is +58h, enabled is the byte table 00E08C88[6]. direction > 0 scans i = page+1 ..
// page+5 for the first enabled i % 6; direction < 0 scans i = page+5 down to 1 (so for page > 0
// the scan reaches page itself); direction == 0 keeps the page. A hit stores i % 6 to +58h
// (00622E9F / 00622F09); every path then tail-jumps to 00621C60(screen, +58h)
// (BSP_RewardsScreen_RefreshPage; JMPs at 00622E85 / 00622EA9 / 00622EEF / 00622F14). The forward
// bound page+6 re-reads +58h every pass (00622E70); the table is read live at each test.
struct RewardsStepPageHost {
    virtual ~RewardsStepPageHost() = default;
    virtual std::int32_t screen_page_58(void* screen) = 0;                // [ECX+58h]
    virtual void set_screen_page_58(void* screen, std::int32_t page) = 0;  // [ECX+58h] = page
    virtual bool page_enabled_00e08c88(std::int32_t index) = 0;           // byte 00E08C88[index]
    // 00621C60, __thiscall(screen, page), RET 4, reached by tail jump.
    virtual void rewards_refresh_page_00621c60(void* screen, std::int32_t page) = 0;
};
void rewards_step_page_00622e30(void* screen, int direction, RewardsStepPageHost& host);

// 006440C0 BSP_Hud_UnitClassIconIndex, __fastcall(ECX class id), RET.
int unit_class_icon_index_006440c0(int class_id) noexcept;

// 00653200 BSP_Hud_PayloadIconFrame, __thiscall(ECX object), RET, AL = frame. The native probes
// the object's virtual +18h(kind) in a fixed order and returns at the first match; with a pure
// predicate the answer depends only on the set of kinds it accepts, given here as a bit mask
// (bit k = virtual +18h(k) is true, k < 32). `byte_82c` / `byte_808` are the object's bytes +82Ch
// and +808h.
std::uint8_t payload_icon_frame_00653200(std::uint32_t kind_mask, bool byte_82c, bool byte_808) noexcept;

// 00604C80 BSP_HudPlaneScreen_GetPayloadFrame, __fastcall(ECX screen), RET. With a plane at
// +168h maps its +970h; returns -1 without a plane or for an unmapped value.
int plane_payload_frame_00604c80(bool has_plane, int plane_970) noexcept;

// 0064A740 BSP_Math_SignFloat_0064A740, stdcall(float), RET 4: COMISS against 0, so a NaN
// answers 0.
int sign_float_0064a740(float value) noexcept;

}  // namespace bsp::band_0060_cc13
