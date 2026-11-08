// One lifecycle regression: E0 raises the retained screen, EAA0 reaches the
// quarter-zoom limit, and E0 lowers it/restores the same camera's near plane.
// This is source/PC24 fixture evidence, not an original ABI or gameplay test.
// Build after vcvars32: cl /std:c++17 /EHsc /MD /O2 /Gy /I include THIS.cpp
// src/hud_warning_screen.cpp src/mission_camera.cpp src/hud_weapon_group_screen.cpp
// bsp_core.lib bsp_lua511.lib bsp_zlib121.lib
// user32.lib powrprof.lib /link /MANIFEST:EMBED /OPT:REF (libraries in build/win32/Release).
#include "bsp/cc13_sweepA3.hpp"
#include "bsp/hud_warning_screen.hpp"
#include "bsp/segs_13_15_22_44_cc13.hpp"
#include "bsp/view_seg14_cc13.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/seg10_cc13.hpp"
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
    bsp::cc13_seg14::BinocularsResetHooks,
    bsp::cc13_sweepA3::BinocularsAxesHost,
    bsp::segs_13_15_22_44_cc13::BinocularsFovHost,
    bsp::cc13_seg10::ViewFovHost {
    bsp::BinocularsState screen;
    alignas(4) std::array<unsigned char, 0x404> mover{};
    alignas(4) std::array<unsigned char, 0x1dc> camera{};
    alignas(4) std::array<unsigned char, 0x2a30> bank{};
    alignas(4) std::array<unsigned char, 0x1fe8> game{};
    alignas(4) std::array<unsigned char, 0xfc> config{};
    bsp::MissionCameraProjection projection;
    float saved_near = 0.0f;
    bool shown = false;
    bool applied = true;
    bool relative = false;
    unsigned raises = 0, part_calls = 0, overlays = 0;
    Lifecycle() {
        bsp::clear_mission_camera();
        at<void*>(mover.data(), 0x1bc) = camera.data();
        at<float>(camera.data(), 0x1d4) = 2.0f;
        projection.near_plane = 2.0f;
        at<void*>(game.data(), 0x19fc) = camera.data();
        at<float>(config.data(), 0xf8) = 1.0f; // normalized Ship FOV fixture setting
        at<float>(mover.data(), 0x384) = 0.4f;
        at<float>(mover.data(), 0x388) = 0.2f;
        at<float>(mover.data(), 0x3ec) = -0.5235988f;
        at<float>(mover.data(), 0x3f0) = 1.5533431f;
        binoculars_set_view_fov(1.0f, 1);
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
    bool binoculars_screen_applied(void* p) override {
        assert(p == &screen); return applied;
    }
    void* binoculars_model_widget(void* p) override {
        assert(p == &screen); return &shown;
    }
    void widget_show_virtual_34(void* p, std::uint32_t flag) override {
        assert(p == &shown); shown = flag != 0;
    }
    void lens_flare_set_enabled_00452b80(void* p, std::uint32_t flag) override {
        assert(p == reinterpret_cast<void*>(0x00e081a0u) && flag == 0);
        lens_effect_off_00452b80();
    }
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
        projection.near_plane = v;
        bsp::update_mission_camera_projection(projection);
    }
    void unit_broadcast_controlled_part_flag_00817380(void* p, std::uint32_t) override {
        assert(p == this); ++part_calls;
    }
    void binoculars_apply_fov_0051e6e0(void* s) override {
        bsp::segs_13_15_22_44_cc13::binoculars_apply_fov_0051e6e0(
            {static_cast<unsigned char*>(s)}, *this);
    }
    void binoculars_set_view_fov(float scale, std::int32_t index) override {
        assert(index == 1);
        bsp::cc13_seg10::game_set_view_fov_004dc940(*this, game.data(), scale, index);
    }
    bsp::cc13_seg10::Object game_00e188a8() override { return game.data(); }
    bsp::cc13_seg10::Object mission_00e198c4() override { assert(false); return nullptr; }
    bool vcall_38_bool(bsp::cc13_seg10::Object) override { assert(false); return false; }
    bsp::cc13_seg10::Object global_config_00432650() override { return config.data(); }
    float fov_scale_00f889b4() override { return bsp::kFovDivisor00f889b4; }
    void camera_set_fov_00b6fbb0(bsp::cc13_seg10::Object p, float fov) override {
        assert(p == camera.data());
        at<float>(p, 0x1c4) = fov;
        bsp::set_mission_camera_fov(projection, fov);
    }
    void publish_view(const bsp::CameraMatrix16& world) {
        // Same frame tail as GameHudHost::Impl::step_mission_camera, with
        // pipe sight enabled and the live retained binocular mode.
        const bsp::MissionCameraFovFrame frame{true, screen.mode_30 == 1, 0.5f, 0.0f};
        bsp::publish_ship_view_mission_camera(world, projection, frame, *this);
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
    assert(fixture.projection.fov == bsp::kFovDivisor00f889b4 * 0.25f);
    fixture.press(true);
    bsp::ship_view_input_0064a400(fixture.screen, fixture, 0.1f);
    assert(fixture.screen.mode_30 == 0 && fixture.screen.zoom_38 == 1.0f);
    assert(at<float>(fixture.camera.data(), 0x1d4) == 2.0f &&
        fixture.projection.fov == bsp::kFovDivisor00f889b4);
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
    // Review 1854: overlay/map cleanup and the screen's +1Ch exit both
    // reset the retained screen that was just raised, including live FOV
    // and the very same mover's zoom flag (not a fresh default screen copy).
    Lifecycle overlay;
    const bsp::CameraMatrix16 world{1, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1};
    overlay.publish_view(world);
    auto& published = bsp::mission_camera_publication();
    const bsp::CameraMatrix default_view = bsp::get_camera_view_projection_00b70490(published.state);
    assert(published.ready && (published.state.projection.valid_flags & 0x10) != 0);
    const auto before_zoom_publishes = published.publishes;
    const auto assert_default_view = [&overlay]() {
        assert(overlay.screen.mode_30 == 0 && overlay.screen.zoom_38 == 1.0f);
        assert(overlay.screen.zoom_limit_28 == 1.0f &&
            overlay.projection.fov == bsp::kFovDivisor00f889b4);
        assert(at<unsigned char>(overlay.mover.data(), 0x3a0) == 0 && !overlay.shown);
    };
    overlay.press(true);
    bsp::ship_view_input_0064a400(overlay.screen, overlay, 0.1f);
    assert(overlay.screen.mode_30 == 1 &&
        overlay.projection.fov == bsp::kFovDivisor00f889b4 * 0.5f);
    assert(published.state.projection.fov == overlay.projection.fov);
    assert(published.state.projection.near_plane == 1.5f);
    assert((published.state.projection.valid_flags & 0x18) == 0);
    assert(published.publishes == before_zoom_publishes && published.state.transform.world == world);
    const bsp::CameraMatrix immediate_zoom = bsp::get_camera_view_projection_00b70490(published.state);
    assert(immediate_zoom[0] > default_view[0]);
    overlay.publish_view(world); // pipe-sight scale 1 must not overwrite active zoom
    assert(published.state.projection.fov == bsp::kFovDivisor00f889b4 * 0.5f);
    assert(bsp::get_camera_view_projection_00b70490(published.state)[0] == immediate_zoom[0]);
    bsp::cc13_seg14::binoculars_reset_0051e8e0(&overlay.screen, overlay);
    assert_default_view();
    assert(published.state.projection.fov == bsp::kFovDivisor00f889b4);
    assert((published.state.projection.valid_flags & 0x18) == 0);
    overlay.publish_view(world);
    assert(bsp::get_camera_view_projection_00b70490(published.state)[0] == default_view[0]);
    overlay.press(false);
    bsp::ship_view_input_0064a400(overlay.screen, overlay, 0.1f);
    overlay.press(true);
    bsp::ship_view_input_0064a400(overlay.screen, overlay, 0.1f);
    assert(overlay.screen.mode_30 == 1 &&
        overlay.projection.fov == bsp::kFovDivisor00f889b4 * 0.5f);
    overlay.publish_view(world);
    assert(published.state.projection.fov == overlay.projection.fov);
    bsp::get_camera_view_projection_00b70490(published.state);
    bsp::cc13_seg14::binoculars_screen_exit_0051e9b0(&overlay.screen, overlay);
    assert_default_view();
    assert(published.state.projection.fov == bsp::kFovDivisor00f889b4);
    assert((published.state.projection.valid_flags & 0x18) == 0);
    overlay.publish_view(world);
    assert(bsp::get_camera_view_projection_00b70490(published.state)[0] == default_view[0]);
    _controlfp_s(&ignored, old_control, _MCW_PC | _MCW_RC);
    std::puts("c845 binocular lifecycle: PASS (raise/zoom/reset/exit; published pipe-sight projection/cache; PC24)");
}
