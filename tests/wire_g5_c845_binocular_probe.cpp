// One lifecycle regression: E0 raises the retained screen, EAA0 reaches the
// quarter-zoom limit, and E0 lowers it/restores the same camera's near plane.
// This is source/PC24 fixture evidence, not an original ABI or gameplay test.
// Build after vcvars32: cl /std:c++17 /EHsc /MD /O2 /I include THIS.cpp
// src/hud_warning_screen.cpp bsp_core.lib bsp_lua511.lib bsp_zlib121.lib
// user32.lib powrprof.lib /link /MANIFEST:EMBED /OPT:REF (libraries in build/win32/Release).
#include "bsp/cc13_sweepA3.hpp"
#include "bsp/hud_warning_screen.hpp"
#include "bsp/segs_13_15_22_44_cc13.hpp"
#include "bsp/view_seg14_cc13.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_rudder.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <float.h>

namespace {
template<class T> T& at(void* p, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
struct Lifecycle final : bsp::ShipViewInputHost,
    bsp::cc13_seg14::BinocularsToggleHooks,
    bsp::cc13_sweepA3::BinocularsAxesHost,
    bsp::segs_13_15_22_44_cc13::BinocularsFovHost {
    bsp::BinocularsState screen;
    alignas(4) std::array<unsigned char, 0x404> mover{};
    alignas(4) std::array<unsigned char, 0x1dc> camera{};
    alignas(4) std::array<unsigned char, 0x2a30> bank{};
    float saved_near = 0.0f;
    float fov = 1.0f;
    bool shown = false;
    bool relative = false;
    unsigned raises = 0, part_calls = 0, overlays = 0;
    Lifecycle() {
        at<void*>(mover.data(), 0x1bc) = camera.data();
        at<float>(camera.data(), 0x1d4) = 2.0f;
        at<float>(mover.data(), 0x384) = 0.4f;
        at<float>(mover.data(), 0x388) = 0.2f;
        at<float>(mover.data(), 0x3ec) = -0.5235988f;
        at<float>(mover.data(), 0x3f0) = 1.5533431f;
    }
    bsp::BinocularsViewTerms view_terms() override {
        bsp::BinocularsViewTerms terms;
        terms.config_04 = 1.0f;
        terms.raise_axis_15e4 = at<float>(bank.data(), 0x15e4);
        return terms;
    }
    bool input_pressed(int action) override {
        if (action != 0xe0) return false;
        bsp::InputActionRecord state;
        auto* p = bank.data() + 0x2a00;
        state.current_down = at<unsigned char>(p, 0x28) != 0;
        state.current_hold = at<float>(p, 0x24);
        state.previous_down = at<unsigned char>(p, 0x20) != 0;
        state.previous_hold = at<float>(p, 0x1c);
        return bsp::action_pressed_this_frame_004c43c0(state);
    }
    void press(bool pressed) {
        at<unsigned char>(bank.data(), 0x2a28) = pressed;
        at<float>(bank.data(), 0x2a24) = pressed ? 1.0f : 0.0f;
    }
    void raise_toggle_0051e7e0(bsp::BinocularsState& s) override {
        ++raises;
        bsp::cc13_seg14::binoculars_toggle_zoom_0051e7e0(&s, this, saved_near, *this);
    }
    void raised_view(bsp::BinocularsState& s, float dt) override {
        bsp::cc13_sweepA3::hud_binoculars_step_input_axes_0051eaa0(
            &s.zoom_38, &s.zoom_limit_28, s.zoom_divisor_3c, dt,
            &s.zoom_active_2c, &s.zoom_rising_2d, &s.zoom_falling_2e, 0, *this);
        binoculars_apply_fov_0051e6e0(&s);
        set_model_visible(true);
    }
    void set_model_visible(bool show) override { shown = show; }
    void lens_effect_off_00452b80() override {}
    void* mover_object() override { return mover.data(); }
    float mover_yaw() override { return at<float>(mover.data(), 0x384); }
    void set_mover_yaw(float v) override { at<float>(mover.data(), 0x384) = v; }
    float mover_pitch() override { return at<float>(mover.data(), 0x388); }
    void set_mover_pitch(float v) override { at<float>(mover.data(), 0x388) = v; }
    float mover_min_pitch() override { return at<float>(mover.data(), 0x3ec); }
    float mover_max_pitch() override { return at<float>(mover.data(), 0x3f0); }
    void view_arm(bsp::BinocularsState&, float) override { assert(false); }
    void screen_2eh_005454b0(float, float, float) override {}
    bool ship_view_mover_20() override { return true; }
    bool object_is_kind_virtual_5c(void* p, std::uint32_t kind) override {
        assert(p == this); return kind == 6;
    }
    void camera_set_near_plane_00b6fbf0(void* p, float v) override {
        assert(p == camera.data()); at<float>(p, 0x1d4) = v;
    }
    void unit_broadcast_controlled_part_flag_00817380(void* p, std::uint32_t) override {
        assert(p == this); ++part_calls;
    }
    void binoculars_apply_fov_0051e6e0(void* s) override {
        bsp::segs_13_15_22_44_cc13::binoculars_apply_fov_0051e6e0(
            {static_cast<unsigned char*>(s)}, *this);
    }
    void binoculars_set_view_fov(float scale, std::int32_t index) override {
        assert(index == 1); fov = scale;
    }
    void unit_virtual_dc(void* p, float v) override {
        assert(p == mover.data()); set_mover_yaw(bsp::wrapped_angle_add_00438aa0(mover_yaw(), v));
    }
    bsp::cc13_sweepA::Raw binoculars_input_bank() override { return {bank.data()}; }
    bool input_action_has_device_query_00a92090(void* p) override {
        assert(p == bank.data() + 0x15c0 || p == bank.data() + 0x2a00); return relative;
    }
    bool binoculars_zoom_pressed() override { return input_pressed(0xe0); }
    void math_step_towards_0042ac60(float* value, float target, float step) override {
        *value = bsp::unit_step_towards_0042ac60(*value, target, step);
    }
    void binoculars_reset_overlay() override { ++overlays; }
};
}
int main() {
    unsigned old_control;
    _controlfp_s(&old_control, 0, 0);
    unsigned ignored;
    _controlfp_s(&ignored, _PC_24 | _RC_NEAR, _MCW_PC | _MCW_RC);
    Lifecycle fixture;
    fixture.press(true);
    bsp::ship_view_input_0064a400(fixture.screen, fixture, 0.1f);
    assert(fixture.screen.mode_30 == 1 && fixture.screen.zoom_38 == 0.5f);
    assert(fixture.screen.zoom_limit_28 == 0.25f && fixture.screen.zoom_active_2c == 1);
    assert(at<float>(fixture.camera.data(), 0x1d4) == 1.5f);
    assert(at<unsigned char>(fixture.mover.data(), 0x3a0) == 1 && fixture.shown);
    fixture.press(false);
    bsp::ship_view_input_0064a400(fixture.screen, fixture, 0.1f);
    assert(fixture.screen.zoom_38 == 0.25f && fixture.screen.zoom_active_2c == 0);
    assert(fixture.fov == 0.25f);
    fixture.press(true);
    bsp::ship_view_input_0064a400(fixture.screen, fixture, 0.1f);
    assert(fixture.screen.mode_30 == 0 && fixture.screen.zoom_38 == 1.0f);
    assert(at<float>(fixture.camera.data(), 0x1d4) == 2.0f && fixture.fov == 1.0f);
    assert(at<unsigned char>(fixture.mover.data(), 0x3a0) == 0 && !fixture.shown);
    assert(fixture.raises == 2 && fixture.part_calls == 2 && fixture.overlays == 0);
    assert(fixture.mover_yaw() == 0.4f && fixture.mover_pitch() == 0.2f);
    // A precision-sensitive analog sample from the 0051EB23/EBCE..EBF6
    // instruction sequence. The former double chain produced 3F32504B.
    fixture.relative = true;
    fixture.press(false);
    at<float>(fixture.bank.data(), 0x15e4) = -0.26697996258735657f;
    at<float>(fixture.bank.data(), 0x2a24) = -0.45774707198143005f;
    fixture.raised_view(fixture.screen, 0.1f);
    std::uint32_t zoom_bits;
    std::memcpy(&zoom_bits, &fixture.screen.zoom_38, sizeof zoom_bits);
    assert(zoom_bits == 0x3f32504au);
    _controlfp_s(&ignored, old_control, _MCW_PC | _MCW_RC);
    std::puts("c845 binocular lifecycle: PASS (raise -> quarter zoom -> lower/restore; analog PC24 bits)");
}
