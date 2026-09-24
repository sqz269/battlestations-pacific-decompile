// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages.
// See include/bsp/game_hosts_hud.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/hud_ship_screen.hpp"
#include "bsp/hud_warning_screen.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/hud_markers_runtime.hpp"
#include "bsp/camera_transform.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/ocean_wave_field.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"

#include "bsp/hud_screens.hpp"
#include "bsp/in_mission_interface_runtime.hpp"
#include "bsp/ingame_interface.hpp"

#include <cstdio>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// The ocean the camera's probes sample: 0078CF20 over the same flat wave field
// the unit host runs (amplitude +24h 0.0f, no coverage regions;
// src/game_hosts_units.cpp OceanFieldBinding).
class CameraOceanField final : public bsp::OceanHeightHost {
public:
    float wave_height_0078c890(float x, float z) override {
        bsp::OceanWaveFieldState field;
        field.flat_f9 = false;
        field.inv_tile_b4 = static_cast<float>(1.0 / 100.0);
        field.amplitude_24 = 0.0f;
        const bsp::OceanWaveGridView zero_grid{};
        return bsp::ocean_wave_field_sample_0078c890(field, zero_grid, x, z);
    }
    float coverage_mask_00b9cf50(float x, float z) override {
        static const std::vector<bsp::OceanCoverageRegion> kNoRegions;
        return bsp::ocean_coverage_mask_00b9cf50(kNoRegions, x, z);
    }
};

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameHudHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in) : log(log_in), menu(menu_in) {}

    GameHostLog& log;
    GameMenuHost& menu;
    // Packet cc9_mission_camera: the ShipCaptain mover 0064DA40 creates for the
    // 25h arm's ship view, and what it reads off its unit.
    GameMissionLuaHost* lua{nullptr};
    bool camera_bound{false};
    std::size_t camera_unit{0};
    std::uint64_t camera_last_frame{~0ull};
    bsp::ShipCaptainCamera camera{};
    bsp::ShipCameraSettings camera_settings{};
    bsp::ShipClassCameraFields camera_class{};
    float camera_class_length{0.0f};
    // Screen 45h's pipe-sight FOV block (docs/MISSION_CAMERA.md section 9).
    bool camera_fov_read{false};
    double camera_fov_ship_degrees{0.0};
    bool camera_pipe_sight_read{false};
    bool camera_pipe_sight_enabled{false};
    float camera_pipe_sight_zoom_rate{0.0f};
    bsp::MissionCameraProjection camera_projection{};
    // Packet cc9_ship_screen_update: screen 45h's fields and its widgets.
    bsp::ShipScreenState ship_screen{};
    std::size_t ship_screen_unit{0};
    bool ship_screen_widgets_bound{false};
    std::map<int, GuiLayoutWidget*> ship_screen_widgets;
    GuiLayoutWidget* ship_screen_widget(bsp::ShipScreenWidget widget);
    // Packet cc9_screen_50h: screen 50h's fields and its widgets.
    bsp::WarningScreenState warning_screen{};
    bool warning_widgets_bound{false};
    std::map<int, GuiLayoutWidget*> warning_widgets;
    GuiLayoutWidget* warning_widget(bsp::WarningWidget widget);
    // Packet cc9_screen_49h.
    bsp::FollowScreen49State follow_screen{};
    // Packet cc9_screen_46h: screen 46h's +1Ch, stored by 0064DA40.
    bool ship_view_has_unit{false};
    bsp::IntegratedControlsState ship_view_controls{};
    bsp::BinocularsState binoculars{};
    // Screen 44h, the HUD root (00649860). The enter 006488D0 zeroes +F4h;
    // the default state is that store, and the screen enters once per run.
    bsp::HudRootUpdateState hud_root{};
    bool hud_root_widgets_bound{false};
    GuiLayoutWidget* hud_root_closed_group{nullptr};
    GuiLayoutWidget* hud_root_closed_group_widget();
    // Screen 29h, the unit pick (00527260, 00526A40).
    bsp::UnitPickScreenState unit_pick{};
    bool lock_radius_read{false};
    std::vector<float> lock_radius;
    const std::vector<float>& lock_radius_multipliers();
    void bind_mission_camera_0064da40();
    void step_mission_camera(float seconds);
    bool camera_target_view(bsp::ShipCaptainTargetView& out);
    GameHudSummary summary;
    InGameInterfaceManager manager{};
    InMissionInterfaceUpdateState update_state{};
    // 004bca50's answer during the mission. The load resolves a single-player
    // campaign mission to 8, the same value the scene contents pass uses.
    int effective_game_mode{8};
    // Milestone 2j: 004c9ca0's loading element at [00e198c4]+D4h. The 1 arm
    // allocates 34h bytes at 004c9cd7 and constructs with 00636d90, which has no
    // reconstruction, so what this process holds is the object's identity and
    // the two bytes the routine itself reads and writes: +4h at 004c9d1f /
    // 004c9d82 and +5h at 004c9d72 / 004c9d85.
    bool loading_element_present{false};
    bool loading_element_4{false};
    bool loading_element_5{false};
    bool in_game_interface_applied{false};

    // Milestone 2k. The two screens whose update virtual is reconstructed, and
    // the controlled-unit payload the second interface request carries.
    std::unique_ptr<GameHudMinimapHost> minimap;
    std::unique_ptr<GameHudMarkersHost> markers;
    GameUnitsHost* units{nullptr};
    bool unit_request_pending{false};
    bool unit_request_applied{false};
    int unit_interface_id{0};

    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }
};

// ---------------------------------------------------------------------------
// 0068aca0's host
// ---------------------------------------------------------------------------

namespace {

// The 20h arm's unit probe, 0068ae0b. Milestone 2h reached it only with a null
// payload, so none of these was called and all three recorded. Milestone 2k
// makes the second request carry the controlled unit, and the probe is then the
// recovered class test over the recovered chain: a ship answers IsKindOf(6), so
// the arm re-enters its own virtual +10h with 25h INTF_CAPTAIN.
class HudUnitQuery final : public InGameInterfaceUnitQuery {
public:
    explicit HudUnitQuery(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_is_kind_of(int type_code) override {
        if (owner_.units != nullptr && owner_.units->controlled_bound()) {
            owner_.done("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
            return owner_.units->unit_is_kind_of(owner_.units->controlled_index(),
                type_code);
        }
        owner_.record("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
        return false;
    }
    bool plane_is_in_flight() override {
        owner_.record("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
        return false;
    }
    bool has_delegate_unit() override {
        owner_.record("InGameInterface::has_delegate_unit", 0x0068af18u);
        return false;
    }

private:
    GameHudHost::Impl& owner_;
};

class HudInterfaceBinding final : public InGameInterfaceHost {
public:
    HudInterfaceBinding(GameHudHost::Impl& owner, HudUnitQuery& query)
        : owner_(owner), query_(query) {}

    bool apply_base_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 00684600. The lock at 00e19894 is the front-end owner's and is clear
        // in this process, so the base accepts the request; the record keeps the
        // base body itself out of the concrete count.
        owner_.record("InGameInterface::apply_base_request", 0x00684600u);
        owner_.summary.applied_interface_id = interface_id;
        return true;
    }
    bool payload_unit_is_dead() override { return false; }
    void limbo_screen_take_unit() override {
        owner_.record("InGameInterface::limbo_screen_take_unit", 0x00565fb0u);
    }
    void hud_root_screen_set_interface(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        // 00646040 BSP_InGameHudRootScreen_SetInterface, the per-id hook the
        // manager calls on registry slot 44h before the switch.
        owner_.record("InGameInterface::hud_root_set_interface", 0x00646040u);
    }
    bool mission_overlay_present() override { return false; }
    void tick_mission_overlay() override {
        owner_.record("InGameInterface::tick_mission_overlay", 0x0042a930u);
    }
    void set_level1_screen_set(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_screen_set_004f8530(ids, count);
        owner_.done("InGameInterface::set_level1_screen_set", 0x004f8530u);
        owner_.summary.level1_screens = 0;
        owner_.summary.level1_screen_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;  // the varargs terminator
            owner_.summary.level1_screen_ids.push_back(ids[i]);
            ++owner_.summary.level1_screens;
        }
    }
    void set_level1_input_contexts(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_input_contexts_004d8a50(ids, count);
        owner_.done("InGameInterface::set_level1_input_contexts", 0x004d8a50u);
        owner_.summary.level1_contexts = 0;
        owner_.summary.level1_context_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;
            owner_.summary.level1_context_ids.push_back(ids[i]);
            ++owner_.summary.level1_contexts;
        }
    }
    void screen_receive_unit(std::uint16_t manager_offset) override {
        owner_.record("InGameInterface::screen_receive_unit", 0x0068ad07u);
        // The 25h arm's hand-off on interface+7Ch is 0064DA40, the ship view
        // that creates the ShipCaptain camera mover (docs/MISSION_CAMERA.md).
        if (kMissionCameraBound && manager_offset == 0x7C) owner_.bind_mission_camera_0064da40();
        // 0064DA40 stores the unit at screen 46h's +1Ch.
        if (manager_offset == 0x7C && owner_.units != nullptr && owner_.units->controlled_bound()) {
            owner_.ship_view_has_unit = true;
        }
        // The 25h arm's hand-off on interface+78h is 0064D590, which stores
        // the unit in screen 45h's +184h (0064D598..0064D5EC).
        if (manager_offset == 0x78 && owner_.units != nullptr && owner_.units->controlled_bound()) {
            const std::size_t unit = owner_.units->controlled_index();
            // 0064D598..0064D5B7: a different unit sets +188h (and stops the
            // timed entries of four widgets through 00AA8B80(2), which this
            // host never starts).
            if (!owner_.ship_screen.has_unit || owner_.ship_screen_unit != unit) {
                owner_.ship_screen.unit_changed_188 = true;
            }
            owner_.ship_screen.has_unit = true;
            owner_.ship_screen_unit = unit;
            // 0064D5E3: with the screen applied, 0064AE20 sets the gauge
            // flags +104h..+107h from the unit's class (Lua VehicleClass
            // NoRepairGUI, class+C8h, 00818300, 0059CB90, 00852350) and the
            // recon and torpedo icons. SUBSTITUTION: not bound, so the flags
            // keep the enter's clear values and only the speed gauge runs.
            if (kHudShipScreenGaugesBound) {
                owner_.record("HudShipScreen::apply_unit_0064ae20", 0x0064ae20u);
            }
        }
    }
    int hud_root_screen_query() override {
        owner_.record("InGameInterface::hud_root_screen_query", 0x00644230u);
        return 0;
    }
    void bomb_view_screen_bind(int value) override { static_cast<void>(value); }
    void bomb_view_screen_store(int value) override { static_cast<void>(value); }
    std::uint32_t unit_ambient_sound_source() override { return 0; }
    void stop_ambient_sound() override {
        owner_.record("InGameInterface::stop_ambient_sound", 0x0054d510u);
    }
    std::uint32_t start_ambient_sound(std::uint32_t source) override {
        static_cast<void>(source);
        owner_.record("InGameInterface::start_ambient_sound", 0x00a7acf0u);
        return 0;
    }
    void collapse_overlays(bool clear_level2, bool run_extra_hook) override {
        static_cast<void>(clear_level2);
        static_cast<void>(run_extra_hook);
        // 0068ab80 collapses the level-2 and level-3 sets. Both are empty here.
        owner_.record("InGameInterface::collapse_overlays", 0x0068ab80u);
    }
    InGameInterfaceUnitQuery& unit_query() override { return query_; }
    bool is_multiplayer() override { return false; }
    void redispatch(int interface_id, bool has_payload) override {
        // 0068AFB4, `this->vtable[10h](newId, payload)`, which is 0068ACA0
        // itself. Milestone 2h recorded it because the null-payload arm chose no
        // id; with the controlled unit as the payload the 20h arm is a unit-kind
        // classifier and this re-entry is what publishes the ship HUD.
        owner_.done("InGameInterface::redispatch", 0x0068afb4u);
        if (depth_ >= 4) return;  // the native has no guard; a cycle would hang
        ++depth_;
        apply_in_game_interface_0068aca0(owner_.manager, *this, interface_id, has_payload);
        --depth_;
    }

private:
    GameHudHost::Impl& owner_;
    HudUnitQuery& query_;
    int depth_{0};
};

// ---------------------------------------------------------------------------
// 0068c1f0's host, the per-frame update
// ---------------------------------------------------------------------------
//
// Step 17 of the in-mission branch of 004e4a40 calls this at 004e5252. Almost
// everything it reads belongs to one of the 42 screen classes, to the camera at
// game+19FCh or to the controlled unit at 00e188d8, and this process owns none
// of those: every one of them is a record with its own address and a neutral
// answer, and the recovered control flow runs over them. The two exceptions are
// the level-3 vector and the input contexts, which are the same stacks the
// level-1 publish uses.
class HudUpdateBinding final : public InMissionInterfaceUpdateHost {
public:
    explicit HudUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    int effective_game_mode() override {
        owner_.done("InGameInterfaceUpdate::effective_game_mode", 0x004bca50u);
        return owner_.effective_game_mode;
    }
    int local_player_slot() override { return 0; }
    bool local_slot_flag_19() override { return false; }
    int local_slot_unit_id() override { return -1; }
    bool has_controlled_unit() override {
        // 00e188d8, written only by BSP_Game_SetControlledUnit 004c0880. Nothing
        // in this process sets a controlled unit, so the HUD has no unit.
        owner_.record("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
        return false;
    }
    bool input_action_pressed(int action) override {
        if (!kHudPresentationTopBound) {
            static_cast<void>(action);
            owner_.record("InGameInterfaceUpdate::input_action_pressed", 0x004c43c0u);
            return false;
        }
        // 004C43C0 through the menu host's action records, the same route the
        // application frame's 00737AE7 test takes (GameFrameHost). The only
        // record the executable drives is the press-start action 4Eh; every
        // other index is a record 00A92370 zeroes each frame and nothing starts,
        // which is the state of an action no device reports, so the rising-edge
        // test answers false for it.
        owner_.done("InGameInterfaceUpdate::input_action_pressed", 0x004c43c0u);
        return owner_.menu.input_action_pressed(action);
    }
    bool modifier_key_allows_back_out() override { return false; }
    bool session_flag_19c4() override { return false; }
    bool overlay_c_extra_gate() override { return false; }

    bool camera_screen_spectating() override { return record_false("006529e0", 0x006529e0u); }
    bool camera_screen_flag_09() override { return record_false("00652a20", 0x00652a20u); }
    bool camera_screen_flag_08() override { return record_false("00652a30", 0x00652a30u); }
    void camera_screen_set_flag_08(bool value) override {
        static_cast<void>(value);
        owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_08", 0x00652a50u);
    }
    bool camera_screen_flag_0a() override { return false; }
    void camera_screen_set_flag_0a(bool value) override { static_cast<void>(value); }
    bool camera_screen_flag_30() override { return false; }
    bool camera_screen_flag_66() override { return record_false("006529c0", 0x006529c0u); }
    void camera_screen_set_flag_66(bool value) override {
        static_cast<void>(value);
        owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_66", 0x006529b0u);
    }
    void camera_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::camera_screen_frame_step", 0x00673130u);
    }

    std::size_t level3_screen_count() override { return 0; }
    int level3_screen_id(std::size_t index) override { static_cast<void>(index); return 0; }
    void set_level3_screen_set(const int* ids, std::size_t count) override {
        static_cast<void>(ids);
        static_cast<void>(count);
        owner_.record("InGameInterfaceUpdate::set_level3_screen_set", 0x004f8670u);
    }
    void set_level3_input_contexts(const int* ids, std::size_t count) override {
        static_cast<void>(ids);
        static_cast<void>(count);
        owner_.record("InGameInterfaceUpdate::set_level3_input_contexts", 0x004d8b70u);
    }

    // The spectate walk over [[game+19cch]+58h]. construct_world 004de610 is a
    // load record, so the world object that list hangs off does not exist.
    std::size_t spectate_unit_count() override {
        owner_.record("InGameInterfaceUpdate::spectate_unit_list", 0x004de610u);
        return 0;
    }
    bool unit_is_kind_of(std::size_t, int) override { return false; }
    bool unit_has_pilot(std::size_t) override { return false; }
    int unit_owner_id(std::size_t) override { return -1; }
    int unit_pilot_state(std::size_t) override { return 0; }
    bool unit_dead(std::size_t) override { return false; }
    int unit_player_slot(std::size_t) override { return -1; }
    int unit_team(std::size_t) override { return 0; }
    void pilot_detach(std::size_t) override {
        owner_.record("InGameInterfaceUpdate::pilot_detach", 0x007ee4e0u);
    }
    void unit_bind_player(std::size_t, int) override {
        owner_.record("InGameInterfaceUpdate::unit_bind_player", 0x00927cc0u);
    }
    void hud_root_set_spectated_unit(std::size_t, bool) override {
        owner_.record("InGameInterfaceUpdate::hud_root_set_spectated_unit", 0x00647300u);
    }

    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        owner_.record("InGameInterfaceUpdate::push_interface_request", 0x004cc460u);
    }
    void push_scene_request_for_unit() override {
        owner_.record("InGameInterfaceUpdate::push_scene_request_for_unit", 0x004b4b00u);
    }
    void toggle_tactical_overlay(std::uint32_t) override {
        owner_.record("InGameInterfaceUpdate::toggle_tactical_overlay", 0x0068c0b0u);
    }
    void exit_free_camera_overlay() override {
        owner_.record("InGameInterfaceUpdate::exit_free_camera_overlay", 0x0068b3f0u);
    }
    void back_out_one_overlay_level() override {
        owner_.record("InGameInterfaceUpdate::back_out_one_overlay_level", 0x0068b470u);
    }
    void collapse_overlays(bool, bool) override {
        owner_.record("InGameInterfaceUpdate::collapse_overlays", 0x0068ab80u);
    }
    void toggle_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_movie_camera", 0x0068a160u);
    }
    void toggle_new_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_new_movie_camera", 0x0068a1f0u);
    }
    bool query_scene_unit(SceneRequestUnitFlags& flags, bool& is_pending_payload) override {
        flags = SceneRequestUnitFlags{};
        is_pending_payload = false;
        owner_.record("InGameInterfaceUpdate::query_scene_unit", 0x004b4b00u);
        return false;
    }

    void hud_root_set_field_1c(std::uint32_t) override {
        owner_.record("InGameInterfaceUpdate::hud_root_set_field_1c", 0x00644220u);
    }
    bool screen_2b_wanted() override { return false; }
    void screen_2b_set_mode(bool) override {
        owner_.record("InGameInterfaceUpdate::screen_2b_set_mode", 0x0053d290u);
    }
    bool screen_34_query() override { return record_false("0054d3c0", 0x0054d3c0u); }
    bool screen_2a_wanted() override { return false; }
    void screen_2a_clear_wanted() override {}
    bool screen_3b_block_a() override { return false; }
    bool screen_3b_block_b() override { return false; }
    void screen_3b_idle() override {
        owner_.record("InGameInterfaceUpdate::screen_3b_idle", 0x005fb080u);
    }
    bool screen_45_block_a() override { return false; }
    bool screen_45_block_b() override { return false; }
    bool map_screen_wanted() override { return false; }
    void map_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::map_screen_frame_step", 0x00612da0u);
    }
    bool map_screen_busy() override { return record_false("00611750", 0x00611750u); }
    std::uint32_t map_screen_overlay_argument() override { return 0; }
    bool pause_screen_flag_05() override { return false; }
    bool pause_screen_flag_08() override { return false; }
    bool pause_screen_child_query() override { return false; }
    bool support_request_pending() override { return record_false("008ed9c0", 0x008ed9c0u); }

    void input_manager_update(float seconds) override {
        static_cast<void>(seconds);
        // 004bec00 then 00a92c40. The mission frame already ticks the input
        // manager through its own step; this call is the manager's own.
        owner_.record("InGameInterfaceUpdate::input_manager_update", 0x004bec00u);
    }
    bool device_a_flag_0b() override { return false; }
    bool device_a_flag_10() override { return false; }
    bool device_b_flag_0b() override { return false; }
    bool device_b_flag_10() override { return false; }
    std::uint32_t controlled_unit_overlay_argument() override { return 0; }

    bool has_ambient_sound_instance() override { return false; }
    bool camera_present() override {
        // game+19fch, the in-mission camera. Nothing in this process builds one.
        owner_.record("InGameInterfaceUpdate::camera", 0x004e4a40u);
        return false;
    }
    void refresh_camera_transform() override {
        owner_.record("InGameInterfaceUpdate::refresh_camera_transform", 0x00b6db70u);
    }
    float camera_height() override { return 0.0f; }
    float camera_ground_x() override { return 0.0f; }
    void set_ambient_volume(float volume) override {
        static_cast<void>(volume);
        owner_.record("InGameInterfaceUpdate::set_ambient_volume", 0x00a79880u);
    }
    float water_height(float x, float y) override {
        static_cast<void>(x);
        static_cast<void>(y);
        owner_.record("InGameInterfaceUpdate::water_height", 0x0078cf20u);
        return 0.0f;
    }
    bool cockpit_screen_wanted() override { return false; }
    int cockpit_screen_mode() override { return record_false("00604f30", 0x00604f30u) ? 1 : 0; }
    bool second_cockpit_screen_wanted() override { return false; }
    int second_cockpit_screen_mode() override { return 0; }
    bool controlled_unit_is_submarine() override { return false; }
    void refresh_controlled_unit_pose() override {}
    float controlled_unit_height() override { return 0.0f; }
    void set_audio_environment(std::string_view name) override {
        owner_.summary.audio_environment.assign(name.data(), name.size());
        owner_.record("InGameInterfaceUpdate::set_audio_environment", 0x00a7b710u);
    }
    void tick_profile_hints(int slot) override {
        static_cast<void>(slot);
        owner_.record("InGameInterfaceUpdate::tick_profile_hints", 0x004c1e90u);
    }

private:
    bool record_false(const char* suffix, std::uint32_t address) {
        char method[80];
        std::snprintf(method, sizeof(method), "InGameInterfaceUpdate::query_%s", suffix);
        owner_.record(method, address);
        return false;
    }

    GameHudHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0068cc70's host, the manager's Init
// ---------------------------------------------------------------------------
//
// bsp::init_in_game_interface_0068cc70 drives this, so the order the 42 screens
// are built and registered in is the recovered one. The 42 leaf classes are not
// reconstructed: the allocation and the constructor are records and the
// registry record is the executable's own, the substitution milestone 2c makes
// for the seven main-menu screen classes. What register_screen then does is the
// recovered part of each +10h override, 004f71d0 plus the page the screen loads
// through 00aa5840 and the widget names it binds through 00aa7e00.
class HudInitBinding final : public InMissionInterfaceInitHost {
public:
    explicit HudInitBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    void load_texture_atlas(std::string_view stem) override {
        static_cast<void>(stem);
        // 00af0060 on the atlas manager at 00f8c26c. The sprite bridge already
        // merges every atlas the installation ships, so the load is the
        // bridge's and the native call stays a record.
        owner_.record("InGameInterface::load_atlas", 0x00af0060u);
    }
    void publish_manager_global() override {
        // 0068ccfe, the second write of 00e198c4. The object is this process's
        // own InGameInterfaceManager, so the publish is real.
        owner_.summary.manager_global_published = true;
        owner_.done("InGameInterface::publish_manager_global", 0x0068ccfeu);
    }
    void base_init() override {
        // 0068cd04, the base Init 00683a90, a bare RET in this build.
        owner_.done("InGameInterface::base_init", 0x00683a90u);
    }
    bool allocate_screen(std::size_t index, std::size_t size_bytes) override {
        static_cast<void>(index);
        static_cast<void>(size_bytes);
        owner_.record("InGameInterface::allocate_screen", 0x00bf681bu);
        return true;
    }
    void construct_screen(std::size_t index, std::uint32_t constructor) override {
        static_cast<void>(index);
        // The 42 leaf constructors have 42 distinct addresses, all of them call
        // sites inside 0068cc70; kInGameHudScreens carries them. One record
        // stands for the set, because recording them under one method name
        // would put 42 call sites on one row.
        if (constructor != 0) ++owner_.summary.screen_constructors;
        owner_.record("InGameInterface::screen_constructor", 0x0068cc70u);
    }
    void register_screen(std::size_t index) override {
        if (index >= kInGameHudScreenCount) return;
        const InGameHudScreenSlot& slot = kInGameHudScreens[index];
        const HudScreenLayout* layout = hud_screen_layout_for_slot(slot.registry_slot);
        GameHudScreenRecord record;
        record.registry_slot = slot.registry_slot;
        record.manager_offset = slot.offset;
        record.register_virtual = layout != nullptr ? layout->register_virtual : 0;
        record.layout_virtual = layout != nullptr ? layout->layout_virtual : 0;

        char name[32];
        std::snprintf(name, sizeof(name), "HUD%02X",
            static_cast<unsigned>(slot.registry_slot));
        if (!owner_.menu.register_in_game_screen(slot.registry_slot, name,
                record.register_virtual, record.layout_virtual)) {
            owner_.log.notef("HUD screen slot %02X could not be registered",
                static_cast<unsigned>(slot.registry_slot));
            return;
        }
        owner_.done("InGameInterface::screen_register", 0x004f71d0u);
        ++owner_.summary.screens_built;

        if (layout != nullptr) {
            for (std::size_t p = 0; p < layout->page_count; ++p) {
                const std::string page = layout->pages[p];
                record.pages.push_back(page);
                ++owner_.summary.pages_requested;
                if (owner_.menu.attach_in_game_page(slot.registry_slot, page)) {
                    ++record.pages_loaded;
                    ++owner_.summary.pages_loaded;
                }
            }
            for (std::size_t w = 0; w < layout->widget_count; ++w) {
                const std::string widget = layout->widgets[w].name;
                ++record.widgets_requested;
                ++owner_.summary.widgets_requested;
                if (owner_.menu.in_game_page_has_child(slot.registry_slot, widget)) {
                    ++record.widgets_bound;
                    ++owner_.summary.widgets_bound;
                }
            }
        }
        owner_.summary.screens.push_back(std::move(record));
    }
    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 0068d73a, 004cc460(20h, 0). The pending record it writes is the
        // front-end owner's; the id is what the executable applies.
        owner_.summary.pushed_interface_id = interface_id;
        owner_.record("InGameInterface::push_interface_request", 0x004cc460u);
    }
    void hud_root_set_field_1c(std::uint32_t value) override {
        static_cast<void>(value);
        owner_.record("InGameInterface::hud_root_set_field_1c", 0x00644220u);
    }

private:
    GameHudHost::Impl& owner_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameHudHost
// ---------------------------------------------------------------------------

GameHudHost::GameHudHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudHost::~GameHudHost() = default;

const GameHudSummary& GameHudHost::summary() const noexcept { return impl_->summary; }

void GameHudHost::build_manager_0068a990() {
    Impl& impl = *impl_;
    if (impl.summary.manager_built) return;
    // 004e0452 allocates 108h bytes and 004e046c runs the constructor; 0068cc70
    // is the Init the caller runs next. Packet cc_hud_updates reconstructed
    // both, so the executable runs them rather than recording them: the object
    // is bsp::InGameInterfaceManager and the order the 42 screens are built and
    // registered in is 0068cc70's own.
    construct_in_game_interface_0068a990(impl.manager);
    impl.done("InGameInterface::construct_manager", 0x0068a990u);

    HudInitBinding init(impl);
    init_in_game_interface_0068cc70(impl.manager, init);
    impl.done("InGameInterface::manager_init", 0x0068cc70u);

    impl.summary.manager_built = true;
    // manager+3Ch, the byte 00684700 sets and 006840FE and the frame's step 17
    // test. The manager's vtable slot +08h is 00684700 itself, so the native
    // raises it through the vtable; nothing in the reconstructed load reaches
    // that call, and Ghidra reports only three direct callers of 00684700, none
    // of them this manager. The executable sets the byte so the request Init
    // pushed can be serviced and step 17 can run, and says so. The other half
    // of 00684700, deactivating every other registered manager, has already
    // happened: the load destroyed the main-menu manager through 00686c90.
    impl.manager.base.active = true;
    impl.record("InGameInterface::activate_manager", 0x00684700u);
    impl.log.notef("in-mission HUD manager: %zu of %zu screens registered through "
        "bsp::init_in_game_interface_0068cc70 (%zu leaf constructors are records), %zu of "
        "%zu pages loaded, %zu of %zu named widgets bound; INTF_SCENE3D (%02Xh) pushed by "
        "Init at 0068d73a", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.screen_constructors,
        impl.summary.pages_loaded, impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.pushed_interface_id));
}

void GameHudHost::apply_pending_interface_0068aca0() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    bool has_payload = false;
    if (!impl.summary.interface_applied) {
        has_payload = false;
    } else if (impl.unit_request_pending && !impl.unit_request_applied) {
        // Milestone 2k: the second request, the one that carries the controlled
        // unit. Serviced through the same 006840f0 / 00684600 path.
        has_payload = true;
        impl.unit_request_applied = true;
    } else {
        return;
    }
    // 006840f0 services the pending record and 00684600 hands the id to the
    // manager's own virtual +10h.
    impl.record("InGameInterface::service_pending_request", 0x006840f0u);
    HudUnitQuery query(impl);
    HudInterfaceBinding binding(impl, query);
    const bool accepted = apply_in_game_interface_0068aca0(impl.manager, binding,
        kInterfaceScene3d, has_payload);
    impl.done("InGameInterface::apply_pending_interface", 0x0068aca0u);
    if (!has_payload) impl.summary.interface_applied = accepted;
    if (has_payload) impl.unit_interface_id = impl.summary.applied_interface_id;

    // Which of the level-1 screens hold a page, for the run's own report.
    impl.summary.level1_pages.clear();
    for (GameHudScreenRecord& record : impl.summary.screens) record.in_level1_set = false;
    for (const int id : impl.summary.level1_screen_ids) {
        for (GameHudScreenRecord& record : impl.summary.screens) {
            if (record.registry_slot != id) continue;
            record.in_level1_set = true;
            for (const std::string& page : record.pages) {
                impl.summary.level1_pages.push_back(page);
            }
        }
    }

    std::string screens;
    for (const int id : impl.summary.level1_screen_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        screens += text;
    }
    std::string contexts;
    for (const int id : impl.summary.level1_context_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        contexts += text;
    }
    std::string pages;
    for (const std::string& page : impl.summary.level1_pages) {
        if (!pages.empty()) pages += ' ';
        pages += page;
    }
    impl.log.notef("in-mission level-1 set for INTF_SCENE3D (%s payload, single player) "
        "applied as %02Xh: screens %s| contexts %s| pages %s",
        has_payload ? "controlled unit" : "null",
        static_cast<unsigned>(impl.summary.applied_interface_id), screens.c_str(),
        contexts.c_str(), pages.c_str());
    if (has_payload) {
        impl.log.note("the 20h arm classified the controlled unit through the recovered "
            "IsKindOf chain and re-entered its own virtual +10h, which is what raises the "
            "world markers screen (4Dh) and keeps the minimap (35h): both have a "
            "reconstructed update virtual and the pump 004f8830 now calls them");
    }
}

bool GameHudHost::Impl::camera_target_view(bsp::ShipCaptainTargetView& out) {
    if (units == nullptr) return false;
    float right[3], up[3], forward[3], translation[3];
    if (!units->unit_pose(camera_unit, right, up, forward, translation)) return false;
    out.world = {right[0], right[1], right[2], 0.0f, up[0], up[1], up[2], 0.0f,
        forward[0], forward[1], forward[2], 0.0f,
        translation[0], translation[1], translation[2], 1.0f};
    const GameUnitRow* row = units->unit_row(camera_unit);
    out.throttle = row != nullptr ? row->throttle : 0.0f;        // unit+980h
    out.rudder = row != nullptr ? row->ordered_rudder : 0.0f;    // unit+984h
    out.gate_5d = units->unit_flag_005d(camera_unit);
    out.min_height = camera_class.min_height;
    out.distance_front = camera_class.distance_front;
    out.distance_side = camera_class.distance_side;
    out.distance_vertical = camera_class.distance_vertical;
    out.length = camera_class_length;
    return true;
}

void GameHudHost::Impl::bind_mission_camera_0064da40() {
    if (units == nullptr || !units->controlled_bound()) return;
    const std::size_t index = units->controlled_index();
    const GameUnitRow* row = units->unit_row(index);
    // The settings block (00424C40, ShipGlobals["ShipCamera"]) and the class
    // camera keys (00831E0D) come from the live Lua state.
    bool settings_read = false;
    bool class_read = false;
    bsp::ShipClassCameraInputs inputs{};
    inputs.captain_camera_global = 10.0f;   // 00CE38B8
    if (lua != nullptr) {
        settings_read = lua->read_ship_camera_settings_0083b5e0(camera_settings);
        if (row != nullptr) class_read = lua->read_ship_class_camera_00831e0d(row->type_id, inputs);
    }
    if (!settings_read) record("MissionCamera::ship_camera_settings", 0x0083b5e0u);
    if (!class_read) record("MissionCamera::class_camera_keys", 0x00831e0du);
    if (lua != nullptr) {
        camera_fov_read = lua->read_global_fov_ship_0087d7b0(camera_fov_ship_degrees);
        camera_pipe_sight_read = lua->read_pipe_sight_params_0083b5e0(
            camera_pipe_sight_enabled, camera_pipe_sight_zoom_rate);
    }
    if (!camera_fov_read) record("MissionCamera::global_config_fovs", 0x0087d7b0u);
    if (!camera_pipe_sight_read) record("MissionCamera::pipe_sight_params", 0x0083b5e0u);
    camera_class = bsp::ship_class_camera_00831e0d(inputs);
    camera_class_length = inputs.base_length;
    if (!camera_bound) {
        bsp::construct_ship_captain_0064b650(camera);
        // The three 00BD2F10 phase draws of 00432750, not taken (see
        // bsp::construct_ship_captain_0064b650).
        for (int i = 0; i < 3; ++i) record("MissionCamera::phase_draw", 0x00bd2f10u);
    }
    camera_unit = index;
    bsp::ShipCaptainTargetView view{};
    camera_target_view(view);
    bsp::bind_ship_captain_0064da40(camera, view, camera_settings);
    camera_bound = true;
    done("MissionCamera::bind_ship_view", 0x0064da40u);
    log.notef("mission camera: ShipCaptain mover bound to \"%s\" (0064da40): ShipCamera "
        "ZoomOffset %.3f LengthMult %.3f angles [%.1f, %.1f] deg; class CameraDistance front "
        "%.1f side %.1f vertical %.1f, CameraMinHeight %.1f, Length %.1f; yaw %.4f pitch %.4f",
        row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(camera_settings.zoom_offset),
        static_cast<double>(camera_settings.length_mult),
        static_cast<double>(camera_settings.min_angle_deg),
        static_cast<double>(camera_settings.max_angle_deg),
        static_cast<double>(camera_class.distance_front),
        static_cast<double>(camera_class.distance_side),
        static_cast<double>(camera_class.distance_vertical),
        static_cast<double>(camera_class.min_height),
        static_cast<double>(camera_class_length),
        static_cast<double>(camera.yaw_384), static_cast<double>(camera.pitch_388));
}

namespace {
class CameraOcean final : public bsp::MissionCameraOcean {
public:
    explicit CameraOcean(GameHostLog& log) : log_(log) {}
    bool present() override { return true; }   // [game+19F0h], the ocean owner
    float water_height(float x, float z) override;
private:
    GameHostLog& log_;
};
}  // namespace

void GameHudHost::Impl::step_mission_camera(float seconds) {
    if (!kMissionCameraBound || !camera_bound) return;
    // The mover is a world entity ticked by the world update before the
    // interface runs. SUBSTITUTION: this host ticks it once per in-game
    // interface frame, at the first screen update of that frame, with the
    // screen's own delta.
    if (camera_last_frame == summary.update_frames) return;
    camera_last_frame = summary.update_frames;
    bsp::ShipCaptainTargetView view{};
    if (!camera_target_view(view)) return;
    CameraOcean ocean(log);
    bsp::CameraMatrix16 world{};
    const bool published =
        bsp::update_ship_captain_00432ed0(camera, view, camera_settings, ocean, seconds, world);
    done("MissionCamera::update", 0x00432ed0u);
    // 0042F4DC, once per probe: the ray against the target's collision.
    for (int i = 0; i < 5; ++i) record("MissionCamera::collision_ray", 0x0098b370u);
    if (!published) return;
    done("MissionCamera::publish_pose", 0x004329d0u);
    // Screen 45h's update (0064DD30) sets the fov every frame while its unit is
    // bound and pipe sight is enabled: 004DC940(1 - zoom_rate * [00E197F4], 1).
    // SUBSTITUTION: [00E197F4], the pipe-sight zoom, grows only through the
    // player's gun-fire adds at 0064DFDE (gunnery, not hooked here) and its
    // spring and drag never move it off zero without them, so it is 0.0f.
    if (kMissionFovBound && camera_fov_read && camera_pipe_sight_read &&
        camera_pipe_sight_enabled) {
        const float stored = bsp::global_config_fov_0087ec0f(camera_fov_ship_degrees,
            bsp::kFovDivisor00f889b4);
        const float scale = bsp::pipe_sight_fov_scale_0064e2d6(camera_pipe_sight_zoom_rate, 0.0f);
        camera_projection.fov = bsp::mission_fov_004dc940(stored, bsp::kFovDivisor00f889b4, scale);
        done("MissionCamera::pipe_sight_fov", 0x004dc940u);
    }
    bsp::publish_mission_camera(world, camera_projection);
}

namespace {
float CameraOcean::water_height(float x, float z) {
    CameraOceanField field;
    log_.implemented("MissionCamera::ocean_height", "0078cf20");
    return bsp::ocean_water_height_0078cf20(x, z, field);
}
}  // namespace

void GameHudHost::attach_world_2k(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& impl = *impl_;
    impl.units = &units;
    impl.lua = &lua;
    if (!impl.minimap) impl.minimap = std::make_unique<GameHudMinimapHost>(impl.log, impl.menu);
    if (!impl.markers) impl.markers = std::make_unique<GameHudMarkersHost>(impl.log, impl.menu);
    impl.minimap->attach_world(units, lua);
    impl.markers->attach_world(units);
}

void GameHudHost::detach_world_2k() noexcept {
    Impl& impl = *impl_;
    impl.units = nullptr;
    impl.lua = nullptr;
    impl.camera_bound = false;
    impl.camera_last_frame = ~0ull;
    impl.ship_screen = bsp::ShipScreenState{};
    impl.ship_screen_widgets.clear();
    impl.ship_screen_widgets_bound = false;
    impl.warning_screen = bsp::WarningScreenState{};
    impl.warning_widgets.clear();
    impl.warning_widgets_bound = false;
    impl.follow_screen = bsp::FollowScreen49State{};
    impl.ship_view_has_unit = false;
    impl.ship_view_controls = bsp::IntegratedControlsState{};
    impl.binoculars = bsp::BinocularsState{};
    bsp::clear_mission_camera();
    impl.unit_request_pending = false;
    impl.unit_request_applied = false;
    impl.unit_interface_id = 0;
    // The child hosts borrow units and Lua. Their default destructors release
    // source caches without invoking their borrowed owners.
    impl.markers.reset();
    impl.minimap.reset();
}

void GameHudHost::request_scene_interface_for_unit_004cc460() {
    Impl& impl = *impl_;
    if (impl.units == nullptr || !impl.units->controlled_bound()) return;
    if (impl.unit_request_pending) return;
    // The load creates the units on its `load_scene_contents` row and builds the
    // HUD manager on a later row, so the request outlives the push: it is
    // serviced by the first 004c40f0 pass that finds the manager built, right
    // after the null-payload request Init pushed. In the game the pusher is the
    // HUD root, which by definition already has a manager; that ordering is the
    // executable's and is the only part of this step that is.
    // 004cc460(20h, unit). The native pushers are the HUD root's own 00649860,
    // 006485a0 and 00647300, all of which read a HUD root object this process
    // does not own, so the push is recorded and the executable makes it.
    impl.record("InGameInterface::push_interface_request", 0x004cc460u);
    impl.unit_request_pending = true;
    impl.log.note("interface request 20h pushed with the controlled unit as its payload: "
        "milestone 2h applied the null-payload request Init pushes at 0068d73a, whose arm "
        "publishes only 29h 49h 44h 35h; the same arm with a unit is a classifier");
}

void GameHudHost::update_minimap_screen_005c0f20(float seconds) {
    Impl& impl = *impl_;
    impl.step_mission_camera(seconds);
    if (!impl.minimap) {
        impl.record("HudMinimap::update", 0x005c0f20u);
        return;
    }
    impl.minimap->update_005c0f20(seconds);
}

GuiLayoutWidget* GameHudHost::Impl::ship_screen_widget(bsp::ShipScreenWidget widget) {
    if (!ship_screen_widgets_bound) {
        ship_screen_widgets_bound = true;
        // 0064C0F0's lookups: the offset each named widget is stored at.
        static const std::pair<int, const char*> kNames[] = {
            {0x48, "ship_stick_Icon"}, {0xF8, "ship_relation_Icon"},
            {0x18C, "VillanasFelso_Icon"}, {0x190, "VillanasAlso_Icon"},
            {0x194, "VillanasBal_Icon"}, {0x198, "VillanasJobb_Icon"},
            // Parts 3 and 4 (packet cc9_ship_screen_parts34).
            {0x50, "ship_dir_Icon"}, {0xC4, "Icon_3_Icon"}, {0xD4, "Icon_5_Icon"},
            {0xD8, "Hl_1_Icon"}, {0xDC, "Hl_2_Icon"}, {0xE0, "Hl_3_Icon"},
            {0xE4, "Hl_4_Icon"}, {0xE8, "circle_1_Section"}, {0xEC, "circle_2_Section"},
            {0xF0, "circle_3_Section"}, {0xF4, "circle_4_Section"}};
        static const char* const kPages[] = {
            "GUI_ship", "GUI_repair", "GUI_ship_effects", "GUI_ship_damage"};
        for (const auto& entry : kNames) {
            GuiLayoutWidget* found = nullptr;
            for (const char* page_name : kPages) {
                GuiLayoutPage* page = menu.in_game_page(0x45, page_name);
                if (page == nullptr || !page->root) continue;
                found = bsp::find_descendant_by_name(*page->root, entry.second);
                if (found != nullptr) break;
            }
            ship_screen_widgets[entry.first] = found;
        }
    }
    auto it = ship_screen_widgets.find(static_cast<int>(widget));
    return it != ship_screen_widgets.end() ? it->second : nullptr;
}

namespace {
// bsp::ShipScreenHost over this process's HUD.
class ShipScreenBinding final : public bsp::ShipScreenHost {
public:
    explicit ShipScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    void set_visible(bsp::ShipScreenWidget widget, bool visible) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    void set_rotation(bsp::ShipScreenWidget widget, float radians) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*w, radians);
    }
    float rotation(bsp::ShipScreenWidget widget) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        return w != nullptr ? w->transform.rotate : 0.0f;
    }
    void set_alpha(bsp::ShipScreenWidget widget, float alpha) override {
        // Virtual +4Ch (00AA6980) writes the alpha lane of the Color property.
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, w->color[0], w->color[1], w->color[2], alpha);
    }
    void select_state(bsp::ShipScreenWidget widget, int state, int zero, float one) override {
        static_cast<void>(widget);
        static_cast<void>(state);
        static_cast<void>(zero);
        static_cast<void>(one);
        // The icon state select, 00AB1710 (gui_icon.hpp); the bridge draws
        // the first authored state only.
        owner_.record("HudShipScreen::relation_select_state", 0x00ab1710u);
    }
    bool unit_is_kind_of(int class_id) override {
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.ship_screen_unit, class_id);
    }
    bool unit_in_formation() override {
        return owner_.units != nullptr
            && owner_.units->unit_formation_group_0284(owner_.ship_screen_unit) >= 0;
    }
    bool unit_leads_formation() override {
        if (owner_.units == nullptr) return false;
        const std::int32_t group = owner_.units->unit_formation_group_0284(owner_.ship_screen_unit);
        return owner_.units->formation_leader_0014(group) == owner_.ship_screen_unit;
    }
    float unit_throttle() override {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        return row != nullptr ? row->throttle : 0.0f;              // unit+980h
    }
    bool flash_view_mode_is_1() override {
        // SUBSTITUTION: [[00E198C4]+4Ch]+30h is on an interface object this
        // process does not build; the enable reads clear.
        owner_.record("HudShipScreen::flash_view_mode", 0x0064e31au);
        return false;
    }
    void relation_slide_block() override {
        owner_.record("HudShipScreen::relation_slide", 0x0064dd6du);
    }
    void pipe_sight_block(float dt) override {
        static_cast<void>(dt);
        // With kMissionFovBound the fov this block sets through 004DC940 is
        // applied by the mission camera's tick (docs/MISSION_CAMERA.md 9).
        owner_.record("HudShipScreen::pipe_sight_block", 0x0064de92u);
    }
    bool controlled_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    bool controlled_is_kind_of(int class_id) override {
        return controlled_present()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool controlled_is_local_player() override {
        // 00927F30(unit, 0): the controlled unit is the local player's ship.
        return controlled_present();
    }
    bool controlled_class_repair() override {
        // [unit+538h]+D0h, the VehicleClass `Repair` byte (00962E16), not
        // loaded by the host. Both answers reach the same path with no input
        // (0064E5FB and 0064E626 both end at 0064F496), so it is recorded.
        owner_.record("HudShipScreen::class_repair_flag", 0x00962e16u);
        return false;
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool input_held(int action) override { return owner_.menu.input_action_held(action); }
    bool input_released(int action) override { return owner_.menu.input_action_released(action); }
    void turn_to_camera_order() override {
        owner_.record("HudShipScreen::turn_to_camera_order", 0x0077c2a0u);
    }
    void turn_to_camera_release() override {
        owner_.record("HudShipScreen::turn_to_camera_release", 0x0064e5c3u);
    }
    bool turn_timer_expired_009539e0() override {
        owner_.record("HudShipScreen::turn_timer_expired", 0x009539e0u);
        return false;
    }
    void repair_menu_open(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::repair_menu", 0x0064e62cu);
    }
    void repair_order_route() override {
        owner_.record("HudShipScreen::repair_order_route", 0x0077c2a0u);
    }
    void warning_pulse(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::warning_pulse", 0x0064f3c2u);
    }
    void repair_mode_panel(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::repair_mode_panel", 0x0064f4a3u);
    }
    bool other_screen_gate() override {
        // SUBSTITUTION: +156h is the host's (clear); [[00E198C4]+64h]+81h and
        // +82h sit on an interface object this process does not build and
        // read clear, so the gate passes.
        return true;
    }
    void other_screen_00545360() override {
        // 00545360 on [00E198C4]+50h: that screen's +D4h = 1, +1Ch = 0.
        owner_.record("HudShipScreen::other_screen_00545360", 0x00545360u);
    }
    bool controls_bound() override { return kHudShipScreenControlsBound; }
    void remainder_from_0064e415(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064e415u);
    }
    void remainder_from_0064f665(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064f665u);
    }
    // Part 3, packet cc9_ship_screen_parts34.
    bool damage_bound() override { return kHudShipScreenDamageBound; }
    void color(bsp::ShipScreenWidget widget, float out[4]) override {
        // Virtual +54h copies the Color property.
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        for (int i = 0; i < 4; ++i) out[i] = w != nullptr ? w->color[i] : 0.0f;
    }
    void set_color(bsp::ShipScreenWidget widget, const float rgba[4]) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, rgba[0], rgba[1], rgba[2], rgba[3]);
    }
    bsp::ShipRepairTaskTerms controlled_repair_task() override {
        // SUBSTITUTION: the repair task at unit+A20h (priority +24h, the two
        // timers +34h/+38h, their divisors unit+A5Ch/+A60h and 0093A3F0's
        // EngineJam record) has no producer in this host
        // (docs/UNIT_FIRE_AND_REPAIR.md; its constructor is unread). Every
        // term reads zero: +150h = -1 and the three task circles hide.
        owner_.record("HudShipScreen::repair_task", 0x0064f6bdu);
        return {};
    }
    float settings_engine_jam_seconds() override {
        // SUBSTITUTION: GameSettings+3E4h, the failure descriptor vector, is
        // not loaded by this host; the loop finds no EngineJam and answers 0.
        owner_.record("HudShipScreen::settings_failure_descriptors", 0x0064f8e7u);
        return 0.0f;
    }
    float settings_4c4() override {
        // Recorded with controlled_float_125c, the submarine arm's other term.
        return 0.0f;
    }
    float controlled_float_125c() override {
        // SUBSTITUTION: unit+125Ch and GameSettings+4C4h, the submarine
        // circle's terms, are not modelled.
        owner_.record("HudShipScreen::submarine_circle_terms", 0x0064f7ebu);
        return 0.0f;
    }
    int controlled_device_count() override {
        // SUBSTITUTION: the device list unit+48h with the damage fields
        // +36Ch/+370h/+378h is not modelled; the list reads empty.
        owner_.record("HudShipScreen::device_list", 0x0064fbecu);
        return 0;
    }
    bsp::ShipDeviceTerms controlled_device(int index) override {
        static_cast<void>(index);
        return {};
    }
    void circle_progress(bsp::ShipScreenWidget widget, float ratio, bool snap) override {
        static_cast<void>(widget);
        static_cast<void>(ratio);
        // 00ABE6E0 on a Section, or 00AA8B00(2)'s timed entry: the bridge
        // draws neither.
        if (snap) {
            owner_.record("HudShipScreen::circle_quad", 0x00abe6e0u);
        } else {
            owner_.record("HudShipScreen::circle_progress", 0x00aa8b00u);
        }
    }
    // Part 4.
    bool gauges_bound() override { return kHudShipScreenGaugesBound; }
    void remainder_from_0064fd24(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064fd24u);
    }
    float unit_ordered_rudder() override {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        return row != nullptr ? row->ordered_rudder : 0.0f;       // unit+984h
    }
    void dir_clock_step(bsp::ShipScreenState& screen) override {
        static_cast<void>(screen);
        // SUBSTITUTION: the step runs only when the platform clock (01090AB0
        // +14h, milliseconds) passes the static 00E19808 + 21h; that clock is
        // not reachable from the HUD host. +124h and +12Ch have no reader in
        // the screen's code (a disp32 scan of 124h finds only the enter and
        // this block), so the step is recorded and not applied.
        owner_.record("HudShipScreen::dir_clock_step", 0x0064fdd4u);
    }
    float unit_forward_speed() override {
        // 0092D730 on [unit+1018h], reconstructed.
        return owner_.units != nullptr
            ? owner_.units->unit_forward_speed_0092d730(owner_.ship_screen_unit) : 0.0f;
    }
    int unit_int_638() override {
        owner_.record("HudShipScreen::gauge_source_638", 0x0064fff9u);
        return 0;
    }
    float unit_float_1124() override {
        owner_.record("HudShipScreen::gauge_source_class_790", 0x00650019u);
        return 0.0f;
    }
    int unit_class_int_790() override { return 0; }
    int unit_device_count_00852300() override {
        owner_.record("HudShipScreen::gauge_source_00852300", 0x00852300u);
        return 0;
    }
    int unit_torpedo_stock_00815850() override {
        owner_.record("HudShipScreen::gauge_source_00815850", 0x00815850u);
        return 0;
    }
    void gauge_digit(int gauge, int digit, float magnitude) override {
        static_cast<void>(gauge);
        static_cast<void>(digit);
        static_cast<void>(magnitude);
        // 0043ABA0 rewrites the digit icon's vertex UVs through its vertex
        // stream (+A0h, lock +10h, unlock +14h); the sprite bridge draws a
        // widget's first authored state only.
        owner_.record("HudShipScreen::gauge_digit_uv", 0x0043aba0u);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::update_ship_screen_0064dd30(float seconds, bool active) {
    Impl& impl = *impl_;
    impl.ship_screen.active_05 = active;
    ShipScreenBinding binding(impl);
    bsp::ship_screen_update_0064dd30(impl.ship_screen, binding, seconds);
    impl.done("HudShipScreen::update", 0x0064dd30u);
}

GuiLayoutWidget* GameHudHost::Impl::warning_widget(bsp::WarningWidget widget) {
    if (!warning_widgets_bound) {
        warning_widgets_bound = true;
        // 006823C0's lookups on GUI_Warning.
        static const std::pair<int, const char*> kNames[] = {
            {0x50, "warning_text"}, {0x54, "warning_1_Icon"}, {0x58, "warning_2_Icon"},
            {0x68, "first_Group"}};
        GuiLayoutPage* page = menu.in_game_page(0x50, "GUI_Warning");
        for (const auto& entry : kNames) {
            warning_widgets[entry.first] = page != nullptr && page->root
                ? bsp::find_descendant_by_name(*page->root, entry.second) : nullptr;
        }
    }
    auto it = warning_widgets.find(static_cast<int>(widget));
    return it != warning_widgets.end() ? it->second : nullptr;
}

namespace {
// bsp::WarningScreenHost over this process's HUD.
class WarningScreenBinding final : public bsp::WarningScreenHost {
public:
    explicit WarningScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    bool controlled_flag_5d() override {
        return owner_.units->unit_flag_005d(owner_.units->controlled_index());
    }
    bool controlled_is_kind_of(int class_id) override {
        return owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool plane_stall_007c6e10() override {
        // SUBSTITUTION: 007C6E10 on the plane's [unit+3D0h] is unread here;
        // reached only when the controlled unit is a plane.
        owner_.record("HudWarningScreen::plane_stall", 0x007c6e10u);
        return false;
    }
    bool ship_shallow_1011() override {
        // SUBSTITUTION: unit+1011h is the latched kind-8 physics contact
        // (the grounding edge 008255B0 rotates from +1010h,
        // docs/UNIT_INSTANCE_UPDATE.md); this host has no terrain contacts,
        // so the latch reads clear.
        owner_.record("HudWarningScreen::contact_latch_1011", 0x006830a5u);
        return false;
    }
    float submarine_127c() override {
        // SUBSTITUTION: unit+127Ch (the submarine's oxygen) is not modelled;
        // reached only for a submarine.
        owner_.record("HudWarningScreen::submarine_oxygen", 0x006830e6u);
        return 1.0f;
    }
    bool submarine_below_00852860() override {
        owner_.record("HudWarningScreen::submarine_below", 0x00852860u);
        return false;
    }
    bool pose_current_c8() override {
        // The host's pose is always current, so 00414DB0 has nothing to refresh.
        return true;
    }
    void refresh_pose_00414db0() override {}
    bool near_world_edge_00681f40() override {
        // SUBSTITUTION: GGame+711Ch..+7130h, the world bounds 00681F40 tests,
        // are unmodelled (as in the units host's fly-to solver).
        owner_.record("HudWarningScreen::world_edge", 0x00681f40u);
        return false;
    }
    void set_visible(bsp::WarningWidget widget, bool visible) override {
        GuiLayoutWidget* w = owner_.warning_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    void set_alpha(bsp::WarningWidget widget, float alpha) override {
        GuiLayoutWidget* w = owner_.warning_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, w->color[0], w->color[1], w->color[2], alpha);
    }
    void sound_stop(int alert, bool flag) override {
        static_cast<void>(alert);
        static_cast<void>(flag);
        owner_.record("HudWarningScreen::sound_stop", 0x006831b5u);
    }
    bool sound_finished(int alert) override {
        static_cast<void>(alert);
        owner_.record("HudWarningScreen::sound_finished", 0x0068321du);
        return false;
    }
    void sound_release(int alert) override {
        static_cast<void>(alert);
        owner_.record("HudWarningScreen::sound_release", 0x0068322fu);
    }
    void show_alert(bsp::WarningScreenState& screen, int alert) override {
        static_cast<void>(screen);
        static const std::uint32_t kShow[4] = {0x00682ed0u, 0x00682cb0u, 0x00682d60u,
                                               0x00682e10u};
        owner_.record("HudWarningScreen::show_alert", kShow[alert & 3]);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

namespace {
// bsp::FollowScreen49Host over this process's HUD.
class FollowScreen49Binding final : public bsp::FollowScreen49Host {
public:
    explicit FollowScreen49Binding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool screen_29h_applied() override { return owner_.menu.in_game_screen_applied(0x29); }
    std::size_t screen_29h_unit() override {
        // Screen 29h's +4Ch, stored by its update (005272E5, from 00526A40).
        if (kHudUnitPickScreenBound) return owner_.unit_pick.pick_4c;
        // SUBSTITUTION with 29h unbound: it reads null.
        owner_.record("HudFollowScreen::screen_29h_unit", 0x0067bf44u);
        return 0;
    }
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    std::size_t controlled_target_00927880() override {
        // SUBSTITUTION: 00927880 goes through the unit's vtable +114h and
        // that object's +18h, neither read; it answers none.
        owner_.record("HudFollowScreen::controlled_target", 0x00927880u);
        return 0;
    }
    bool is_kind_of(std::size_t unit, int class_id) override {
        return owner_.units != nullptr && unit != 0
            && owner_.units->unit_is_kind_of(unit - 1, class_id);
    }
    bool alive_and_visible(std::size_t unit) override {
        return owner_.units != nullptr && unit != 0
            && owner_.units->unit_alive_and_visible(unit - 1);
    }
    bool leader_3d0_alive(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudFollowScreen::leader_3d0", 0x0067bfafu);
        return false;
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

namespace {
// bsp::ShipViewScreen46Host over this process's HUD.
class ShipViewScreen46Binding final : public bsp::ShipViewScreen46Host {
public:
    ShipViewScreen46Binding(GameHudHost::Impl& owner, bool wanted)
        : owner_(owner), wanted_(wanted) {}
    bool wanted_04() override { return wanted_; }
    bool has_unit_1c() override { return owner_.ship_view_has_unit; }
    void view_input_0064a400(float dt) override;
    void integrated_controls_0064b870(float dt) override;
    bool screen_2eh_present() override {
        // [00E198C4]+50h is slot 2Eh's screen, which the registry holds.
        return owner_.menu.in_game_screen_registered(0x2e);
    }
    void screen_2eh_005484f0() override {
        owner_.record("HudShipView::screen_2eh_005484f0", 0x005484f0u);
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool unit_virtual_234() override {
        owner_.record("HudShipView::unit_virtual_234", 0x0064d697u);
        return false;
    }
    // 00927F30(unit, 0): the controlled unit is the local player's ship.
    bool unit_local_player() override { return true; }
    bool unit_is_kind_of(int class_id) override {
        return owner_.units != nullptr && owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool unit_00812960() override {
        owner_.record("HudShipView::unit_00812960", 0x00812960u);
        return false;
    }
    void order_route_0077d600() override {
        owner_.record("HudShipView::order_route_0077d600", 0x0077d600u);
    }
    void order_route_0077c2a0() override {
        owner_.record("HudShipView::order_route_0077c2a0", 0x0077c2a0u);
    }

private:
    GameHudHost::Impl& owner_;
    bool wanted_;
};
}  // namespace

namespace {
// bsp::IntegratedControlsHost over this process's HUD.
class IntegratedControlsBinding final : public bsp::IntegratedControlsHost {
public:
    explicit IntegratedControlsBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_byte_6c8() override {
        // The unit constructor stores 1 at 0095CE29 and a disp32 scan finds no
        // other byte writer of +6C8h, so the gate reads set.
        return true;
    }
    bsp::IntegratedControlsInputs inputs() override {
        // SUBSTITUTION: the input manager (004BEC00) axes +1BE4h/+1BB4h, the
        // two binding queries on +1B90h and the byte +1B91h are not modelled;
        // no in-mission input is driven, so they read zero and clear.
        owner_.record("HudShipView::control_inputs", 0x004bec00u);
        return {};
    }
    bool unit_1130_clear() override {
        owner_.record("HudShipView::unit_1130", 0x0064b97du);
        return true;
    }
    bool local_player_role(int role) override {
        if (kHudShipViewRoleTableBound) {
            // 00927F30(unit, role): the role's holder at unit+1ACh is the
            // local player's slot, game+18ECh, 0 in this single-player host.
            // The units host owns that table (the 27h take 0067BB50 and the
            // BSP_PLAYER_HELM transfer write it). With role 1 held the branch
            // below reaches issue_order, which is a record: the helm option's
            // own 00816A40 issue stays the only one.
            std::int32_t holder = -1;
            return owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_current_role_slot(owner_.units->controlled_index(),
                                                        role, holder)
                && holder == 0;
        }
        // SUBSTITUTION: role 0 held, role 1 not, until an input-started
        // transfer (docs/CONTROLLED_UNIT_HELM.md section 6).
        owner_.record("HudShipView::local_player_role", 0x00927f30u);
        return role == 0;
    }
    void role_transfer_0077c470(int mask, int take) override {
        static_cast<void>(mask);
        static_cast<void>(take);
        owner_.record("HudShipView::role_transfer", 0x0077c470u);
    }
    float unit_ordered_rudder() override {
        const GameUnitRow* row = controlled_row();
        return row != nullptr ? row->ordered_rudder : 0.0f;
    }
    float unit_throttle() override {
        const GameUnitRow* row = controlled_row();
        return row != nullptr ? row->throttle : 0.0f;
    }
    void issue_order(float thrust, float turn) override {
        static_cast<void>(thrust);
        static_cast<void>(turn);
        owner_.record("HudShipView::issue_order", 0x00816a40u);
    }
    bool game_19c4() override {
        owner_.record("HudShipView::game_19c4", 0x0064bb2cu);
        return false;
    }

private:
    const GameUnitRow* controlled_row() const {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return nullptr;
        return owner_.units->unit_row(owner_.units->controlled_index());
    }
    GameHudHost::Impl& owner_;
};

// bsp::ShipViewInputHost over this process's HUD and the mission camera's
// mover.
class ShipViewInputBinding final : public bsp::ShipViewInputHost {
public:
    explicit ShipViewInputBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bsp::BinocularsViewTerms view_terms() override {
        // SUBSTITUTION: the input manager's axes +1584h/+15B4h/+15E4h are not
        // modelled and read zero (no in-mission input is driven);
        // GlobalConfig+4 is unread and reads 1.0. It only scales a zero axis.
        owner_.record("HudShipView::view_input_terms", 0x0051f061u);
        bsp::BinocularsViewTerms terms{};
        terms.config_04 = 1.0f;
        return terms;
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    void raise_toggle_0051e7e0(bsp::BinocularsState& screen) override {
        static_cast<void>(screen);
        owner_.record("HudShipView::binoculars_raise_toggle", 0x0051e7e0u);
    }
    void raised_view(bsp::BinocularsState& screen, float dt) override {
        static_cast<void>(screen);
        static_cast<void>(dt);
        owner_.record("HudShipView::binoculars_raised_view", 0x0051efb7u);
    }
    void set_model_visible(bool visible) override {
        // Screen 26h's +20h, the Tavcso_Model widget of GUI_binoculars.
        GuiLayoutPage* page = owner_.menu.in_game_page(0x26, "GUI_binoculars");
        GuiLayoutWidget* w = page != nullptr && page->root
            ? bsp::find_descendant_by_name(*page->root, "Tavcso_Model") : nullptr;
        if (w != nullptr) owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    void lens_effect_off_00452b80() override {
        // 00B0D020 on the renderer object [00F8D39C] is not modelled.
        owner_.record("HudShipView::binoculars_lens_off", 0x00452b80u);
    }
    bool mover_present() override { return owner_.camera_bound; }
    float mover_yaw() override { return owner_.camera.yaw_384; }
    void set_mover_yaw(float yaw) override { owner_.camera.yaw_384 = yaw; }
    float mover_pitch() override { return owner_.camera.pitch_388; }
    void set_mover_pitch(float pitch) override { owner_.camera.pitch_388 = pitch; }
    float mover_min_pitch() override { return owner_.camera.min_pitch_3ec; }
    float mover_max_pitch() override { return owner_.camera.max_pitch_3f0; }
    void view_arm(bsp::BinocularsState& screen, float dt) override {
        static_cast<void>(screen);
        static_cast<void>(dt);
        owner_.record("HudShipView::binoculars_view_arm", 0x0051f1c0u);
    }
    void screen_2eh_005454b0(float a, float b, float c) override {
        static_cast<void>(a);
        static_cast<void>(b);
        static_cast<void>(c);
        // Screen 2Eh's +48h..+50h and screen 4Dh's +44h/+48h have no reader
        // in this host.
        owner_.record("HudShipView::screen_2eh_005454b0", 0x005454b0u);
    }
    bool ship_view_mover_20() override { return owner_.camera_bound; }

private:
    GameHudHost::Impl& owner_;
};

void ShipViewScreen46Binding::view_input_0064a400(float dt) {
    if (!kHudShipViewInputBound) {
        // Screen 26h's 0051F330 and screen 2Eh's 005454B0, not bound.
        owner_.record("HudShipView::view_input", 0x0064a400u);
        return;
    }
    ShipViewInputBinding binding(owner_);
    bsp::ship_view_input_0064a400(owner_.binoculars, binding, dt);
    owner_.done("HudShipView::view_input_0064a400", 0x0064a400u);
}

void ShipViewScreen46Binding::integrated_controls_0064b870(float dt) {
    if (!kHudShipViewControlsBound) {
        // BSP_HudUnitOrder_UpdateIntegratedControls, not bound.
        owner_.record("HudShipView::integrated_controls", 0x0064b870u);
        return;
    }
    IntegratedControlsBinding binding(owner_);
    bsp::integrated_controls_0064b870(owner_.ship_view_controls, binding, dt);
    owner_.done("HudShipView::integrated_controls_0064b870", 0x0064b870u);
}
}  // namespace

void GameHudHost::update_ship_view_screen_0064d610(float seconds, bool wanted) {
    Impl& impl = *impl_;
    ShipViewScreen46Binding binding(impl, wanted);
    bsp::ship_view_update_0064d610(binding, seconds);
    impl.done("HudShipView::update", 0x0064d610u);
}

void GameHudHost::update_follow_screen_0067bf00() {
    Impl& impl = *impl_;
    FollowScreen49Binding binding(impl);
    bsp::follow_screen_update_0067bf00(impl.follow_screen, binding);
    impl.done("HudFollowScreen::update", 0x0067bf00u);
}

void GameHudHost::update_warning_screen_00683020(float seconds) {
    Impl& impl = *impl_;
    WarningScreenBinding binding(impl);
    bsp::warning_screen_update_00683020(impl.warning_screen, binding, seconds);
    impl.done("HudWarningScreen::update", 0x00683020u);
}

namespace {
// bsp::HudRootUpdateHost over this process's HUD, for 00649860. One record per
// input the host cannot source. Every body that writes the controlled unit,
// pushes an interface request or toggles the closed HUD is a record that
// performs nothing, and none of them is reached in USN04.
class HudRootUpdateBinding final : public bsp::HudRootUpdateHost {
public:
    explicit HudRootUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    bool game_hud_suppressed() override {
        // SUBSTITUTION: game+61Fh (004D95F0, the pause menu's "set paused") and
        // game+620h (004D94F0, the tutorial hints' pause) are not modelled.
        // The host never pauses, so both read clear.
        owner_.record("HudRootScreen::game_pause_bytes", 0x00649863u);
        return false;
    }
    bool platform_row_inset() override {
        // SUBSTITUTION: platform 0109CF04+0Dh (the widescreen byte) does not
        // reach the HUD host (docs/HUD_PRESENTATION_TOP.md row 2). It only
        // places the first power-up row, and no row is placed here.
        owner_.record("HudRootScreen::platform_row_inset", 0x006499d9u);
        return false;
    }
    float mission_clock() override {
        owner_.record("HudRootScreen::mission_clock", 0x00649c6bu);
        return 0.0f;
    }
    bool controlled_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    // The clone vectors +104h and +114h: nothing is ever cloned here (the
    // power-up collection is a record answering none), so both are empty and
    // the destroy-and-erase loops at 006498AC..006499CC do nothing.
    void release_icon_clones() override {}
    void release_circle_clones() override {}
    void collect_powerups() override {
        // SUBSTITUTION: the power-up manager [00F88C30] is not built; 008E9AF0
        // answers five empty vectors.
        owner_.record("HudRootScreen::collect_powerups", 0x008e9af0u);
    }
    std::size_t untimed_entry_count() override { return 0; }
    std::size_t timed_entry_count() override { return 0; }
    float timed_entry_deadline(std::size_t) override { return 0.0f; }
    bool untimed_entry_applies(std::size_t) override {
        owner_.record("HudRootScreen::powerup_applies", 0x008e62a0u);
        return false;
    }
    bool timed_entry_applies(std::size_t) override {
        owner_.record("HudRootScreen::powerup_applies", 0x008e62a0u);
        return false;
    }
    std::uint32_t clone_icon_template() override {
        owner_.record("HudRootScreen::clone_template", 0x00aab4c0u);
        return 0;
    }
    std::uint32_t clone_circle_template() override {
        owner_.record("HudRootScreen::clone_template", 0x00aab4c0u);
        return 0;
    }
    float icon_template_height() override { return 0.0f; }
    void widget_set_resolved_position(std::uint32_t, const bsp::HudGuiPoint&) override {}
    int local_team_index() override {
        if (!controlled_unit_present()) return 0;
        const GameUnitRow* row = owner_.units->unit_row(owner_.units->controlled_index());
        return row != nullptr && row->party >= 0 && row->party <= 7 ? row->party : 0;
    }
    void icon_add_state_from_entry(std::uint32_t, std::size_t, bool) override {}
    void widget_select_state(std::uint32_t) override {}
    void circle_set_fill(std::uint32_t, float) override {}
    float timed_entry_duration(std::size_t) override { return 1.0f; }

    void advance_ticker() override {
        owner_.record("HudRootScreen::advance_ticker", 0x00648060u);
    }
    // +78h's only writer is 0076CFD0, the receiver of session message 4Dh,
    // which Lua MW_MultiSelectUnit (008AB3D0) sends. The host keeps that
    // native unimplemented and USN04 never calls it, so the handle stays 0 and
    // none of the following is reached.
    bool pending_unit_selectable(std::uint16_t) override {
        owner_.record("HudRootScreen::pending_unit_handle", 0x0064a018u);
        return false;
    }
    bool pending_unit_allowed() override { return false; }
    void commit_pending_unit() override {
        // 00645600 sets the controlled unit: gameplay state, not performed.
        owner_.record("HudRootScreen::commit_pending_unit", 0x00645600u);
    }
    bool interface_manager_idle() override { return false; }
    void notify_camera_and_input() override {}
    int controlled_unit_scene_payload() override { return 0; }
    void push_interface_request(int, int) override {
        owner_.record("HudRootScreen::push_interface_request", 0x004cc460u);
    }

    void widget_set_shown(std::uint32_t widget, bool shown) override {
        if (widget != 0x40u) {
            owner_.record("HudRootScreen::clone_set_shown", 0x00649aafu);
            return;
        }
        // 0064A108: +40h is ClosedUnitHUD_Group (006463E0 stores it at
        // 00646AAE, found under Units_Group on GUI_selector), shown through
        // virtual +34h(1).
        GuiLayoutWidget* w = owner_.hud_root_closed_group_widget();
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, shown);
    }
    std::uint32_t resolve_weapon_info(bool& needs_fallback) override {
        // 00644CC0 (and its fallback 00644C20) look the controlled unit up in
        // the primary and secondary lists 00648290 builds; neither the lists
        // nor the lookups are bound here. +C2h has no reader in this host.
        owner_.record("HudRootScreen::selection_tuple", 0x0064a114u);
        needs_fallback = false;
        return 0xFFFF0000u;
    }
    std::uint32_t resolve_weapon_info_fallback() override { return 0xFFFF0000u; }
    void update_unit_rows(float) override {
        // 00644DB0, the selection poll over actions 8Dh, 8Ch, 8Eh and 8Fh.
        owner_.record("HudRootScreen::selection_poll", 0x00644db0u);
    }
    void update_medals() override {
        // 00648C20, the unit rows (name, flag, payload, health, command).
        owner_.record("HudRootScreen::unit_rows", 0x00648c20u);
    }

    bool controlled_unit_is_kind(int kind) override {
        return owner_.units->unit_is_kind_of(owner_.units->controlled_index(), kind);
    }
    bool game_blocks_toggle() override {
        // SUBSTITUTION: game+19C4h is not modelled (as for screen 46h); read clear.
        owner_.record("HudRootScreen::game_19c4", 0x0064a179u);
        return false;
    }
    bool input_action_pressed(int action) override {
        return owner_.menu.input_action_pressed(action);
    }
    bool controlled_unit_flag_379() override {
        // Reached only for a kind-18h (plane) controlled unit.
        owner_.record("HudRootScreen::plane_flag_379", 0x0064a1e0u);
        return false;
    }
    void toggle_closed_hud() override {
        // 00647080 pushes interface requests 22h/23h: not performed.
        owner_.record("HudRootScreen::toggle_closed_hud", 0x00647080u);
    }
    // +20h is the screen's own byte; nothing in this host sets it, and the
    // tail then stops at game+1FE4h, which is zero in single player.
    bool game_has_group_manager() override { return false; }
    int controlled_unit_team() override { return local_team_index(); }
    void rebuild_group_rows() override {
        owner_.record("HudRootScreen::rebuild_group_rows", 0x006485a0u);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

GuiLayoutWidget* GameHudHost::Impl::hud_root_closed_group_widget() {
    if (!hud_root_widgets_bound) {
        hud_root_widgets_bound = true;
        GuiLayoutPage* page = menu.in_game_page(0x44, "GUI_selector");
        hud_root_closed_group = page != nullptr && page->root
            ? bsp::find_descendant_by_name(*page->root, "ClosedUnitHUD_Group") : nullptr;
    }
    return hud_root_closed_group;
}

void GameHudHost::update_hud_root_screen_00649860(float seconds) {
    Impl& impl = *impl_;
    // 00649FD7..00649FF5: with the ticker timer expired the award queue at
    // +B0h..+B4h is read. SUBSTITUTION: 00648AB0, which
    // BSP_MissionScoring_GrantAward (0090EDE0) calls to queue an award
    // message, is not reached from this host's award grants, so the queue
    // reads empty and the timer is never reloaded.
    if (impl.hud_root.ticker_timer <= 0.0f) {
        impl.record("HudRootScreen::award_ticker_queue", 0x00649fe0u);
    }
    impl.hud_root.ticker_queue_count = 0;
    impl.hud_root.selector_widget = 0x40u;  // ClosedUnitHUD_Group's field
    HudRootUpdateBinding binding(impl);
    bsp::hud_root_screen_update(impl.hud_root, binding, seconds);
    impl.done("HudRootScreen::update", 0x00649860u);
}

namespace {
// bsp::UnitPickHost over this process's HUD, for screen 29h (00527260 and
// 00526A40). Units are index + 1. The lock branches, which send orders, move
// roles or take control, are records that perform nothing; each is reached
// only after an input flag.
class UnitPickBinding final : public bsp::UnitPickHost {
public:
    explicit UnitPickBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    float lock_zoom_modifier_5c() override {
        // Reached only below full zoom; the binoculars zoom stays 1.0 here.
        owner_.record("UnitPickScreen::lock_zoom_modifier", 0x00526a8eu);
        return 0.8f;  // 0087DDF7's default, 3F4CCCCDh
    }
    float interpolate_clamped_00419010(float, float) override {
        owner_.record("UnitPickScreen::interpolate_clamped", 0x00419010u);
        return 1.0f;
    }
    bool game_19c4() override {
        // SUBSTITUTION: game+19C4h is not modelled (as for screens 44h and 46h).
        // The first query of an update is 00526A40's (00526B05); the later
        // ones are 00527260's own (005273C5 onwards).
        const bool pick = game_19c4_queries_++ == 0;
        owner_.record(pick ? "UnitPickScreen::game_19c4_pick" : "UnitPickScreen::game_19c4",
                      pick ? 0x00526b05u : 0x005273c5u);
        return false;
    }
    std::size_t spectated_unit_005a1310() override {
        owner_.record("UnitPickScreen::spectated_unit", 0x005a1310u);
        return 0;
    }
    std::size_t firing_unit_004b4b00() override {
        // 004B4B00: the controlled unit when it is kind 5, its [+3D0h] when it
        // is kind 18h, else none.
        const std::size_t c = controlled_unit();
        if (c == 0) return 0;
        if (is_kind_of(c, 5)) return c;
        if (is_kind_of(c, 0x18)) {
            owner_.record("UnitPickScreen::squadron_first_member", 0x004b4b35u);
            return 0;
        }
        return 0;
    }
    bool is_kind_of(std::size_t unit, int class_id) override {
        return unit != 0 && owner_.units->unit_is_kind_of(unit - 1, class_id);
    }
    int unit_slot_1b4(std::size_t) override {
        owner_.record("UnitPickScreen::unit_slot_1b4", 0x00526b36u);
        return -1;
    }
    bool slot_auto_engage_00927f10(int) override {
        owner_.record("UnitPickScreen::slot_auto_engage", 0x00927f10u);
        return false;
    }
    void camera_basis(float position[3], float forward[3]) override {
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!(kMissionCameraBound && node.ready)) {
            owner_.record("UnitPickScreen::camera_basis", 0x00526b71u);
            for (int i = 0; i < 3; ++i) position[i] = forward[i] = 0.0f;
            return;
        }
        // 00526B77/00526BBA: 00B6DB70 when the world-valid bit 2 is clear,
        // then +110h..+118h (row 2) and +120h..+128h (row 3) of the world.
        if ((node.state.transform.valid_flags & 2u) == 0) {
            bsp::refresh_camera_world_00b6db70(node.state.transform);
        }
        const bsp::CameraMatrix& world = node.state.transform.world;
        for (int i = 0; i < 3; ++i) {
            forward[i] = world[8 + i];
            position[i] = world[12 + i];
        }
    }
    bool ray_pick_009043a0(const float*, const float*, std::size_t, std::size_t& hit) override {
        // SUBSTITUTION: the spatial index ([00E188A8]+19CCh, 0098ADD0) is not
        // built, so the segment query reports no hit.
        owner_.record("UnitPickScreen::segment_query", 0x009043a0u);
        hit = 0;
        return false;
    }
    std::size_t ray_hit_branch(bsp::UnitPickScreenState&, std::size_t) override {
        owner_.record("UnitPickScreen::ray_hit_branch", 0x00526c74u);
        return 0;
    }
    bool game_1fe4() override { return false; }  // single player: no session
    int game_difficulty_6ac() override { return game_effective_difficulty_6ac(); }
    float lock_radius_multiplier(int index) override {
        const std::vector<float>& m = owner_.lock_radius_multipliers();
        if (index < 0 || static_cast<std::size_t>(index) >= m.size()) {
            // No config vector: the image would fault in 00BF6713. Radius 0
            // picks nothing.
            owner_.record("UnitPickScreen::lock_radius_multiplier", 0x00526dd7u);
            return 0.0f;
        }
        return m[static_cast<std::size_t>(index)];
    }
    std::size_t list_size(bsp::UnitPickList list) override {
        if (list == bsp::UnitPickList::Kind35) {
            // SUBSTITUTION: game+19BCh, 004C3CB0's second-walk list of kind-35h
            // and grey-arrow units (team record +DE8h), is not built: empty.
            owner_.record("UnitPickScreen::kind35_list", 0x004c3e99u);
            return 0;
        }
        // SUBSTITUTION: game+1974h is 004C3CB0's first walk over the local team
        // record's +DDCh list, which this process does not fill. The stand-in
        // walks the created units with that walk's own filter: the local
        // party, +5Ch set with +5Dh/+60h/+5Eh clear, and not kind 2Ah.
        owner_.record("UnitPickScreen::team_unit_list", 0x004c3cb0u);
        team_.clear();
        const std::size_t c = controlled_unit();
        if (c == 0) return 0;
        const GameUnitRow* self = owner_.units->unit_row(c - 1);
        if (self == nullptr) return 0;
        const std::size_t count = owner_.units->units().size();
        for (std::size_t i = 0; i < count; ++i) {
            const GameUnitRow* row = owner_.units->unit_row(i);
            if (row == nullptr || row->party != self->party) continue;
            if (!owner_.units->unit_alive_and_visible(i)) continue;
            if (owner_.units->unit_is_kind_of(i, 0x2a)) continue;
            team_.push_back(i + 1);
        }
        return team_.size();
    }
    std::size_t list_unit(bsp::UnitPickList, std::size_t i) override {
        return i < team_.size() ? team_[i] : 0;
    }
    int member_count_3cc(std::size_t) override { return 0; }
    std::size_t member_3d0(std::size_t, int) override {
        // SUBSTITUTION: squadron members (+3D0h, count +3CCh) are not exposed.
        owner_.record("UnitPickScreen::squadron_members", 0x00526e58u);
        return 0;
    }
    bool grey_arrow_contains_008ddf90(std::size_t) override {
        // SUBSTITUTION: no entity set is built at game+21A4h
        // (docs/AI_SQUADRON_SERVED.md), so nothing is a grey-arrow member.
        owner_.record("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
        return false;
    }
    bool flag_5d(std::size_t unit) override {
        return owner_.units->unit_flag_005d(unit - 1);
    }
    void unit_position(std::size_t unit, float out[3]) override {
        const GameUnitRow* row = owner_.units->unit_row(unit - 1);
        for (int i = 0; i < 3; ++i) out[i] = row != nullptr ? row->position[i] : 0.0f;
    }
    void intercept_point_00901c20(std::size_t, std::size_t, float out[3]) override {
        owner_.record("UnitPickScreen::gunbot_intercept", 0x00901c20u);
        for (int i = 0; i < 3; ++i) out[i] = 0.0f;
    }
    unsigned project_0043a660(const float point[3], float& x, float& y) override {
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!(kMissionCameraBound && node.ready)) {
            owner_.record("UnitPickScreen::project", 0x0043a660u);
            return 0;
        }
        // 0043A697..0043A6AF: 00B70490 then 00B62D10, then the mode-1 map.
        const bsp::CameraMatrix& vp = bsp::get_camera_view_projection_00b70490(node.state);
        const float source[4] = {point[0], point[1], point[2], 1.0f};
        float clip[4];
        bsp::transform_native_vector4_00b62d10(source, clip, vp.data());
        // SUBSTITUTION: mode 1 multiplies y by the GUI extent height
        // (00AA1FE0()+4h), which reaches the markers host only; 1.0 is the
        // value its 4/3 law gives (docs/HUD_PRESENTATION_TOP.md row 2).
        owner_.record("UnitPickScreen::gui_extent", 0x00aa1fe0u);
        const bsp::HudMarkerScreenProjection p = bsp::camera_project_world_to_screen_0043a660(
            clip[0], clip[1], clip[2], clip[3], bsp::HudMarkerProjectMode::ViewportAspect, true,
            1.0f, 1.0f);
        x = p.screen_x;
        y = p.screen_y;
        return p.clip_mask;
    }
    bool ray_hit_filter_005220c0(std::size_t) override {
        owner_.record("UnitPickScreen::ray_hit_filter", 0x005220c0u);
        return true;
    }
    bool alive_and_visible(std::size_t unit) override {
        return owner_.units->unit_alive_and_visible(unit - 1);
    }

    float binoculars_zoom() override { return owner_.binoculars.zoom_38; }  // 26h+38h
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    std::size_t owner_140(std::size_t) override {
        // Unit vtable +140h (the scene payload 0064A0EB also asks for), unread.
        owner_.record("UnitPickScreen::owner_140", 0x0052731cu);
        return 0;
    }
    bool team_record_19() override {
        // [game+18CCh+slot*4]+19h, the local player record's byte: unread.
        owner_.record("UnitPickScreen::team_record_19", 0x0052733cu);
        return false;
    }
    bool byte_e0e350() override {
        owner_.record("UnitPickScreen::byte_e0e350", 0x005273cfu);
        return false;
    }
    int interface_id() override { return owner_.summary.applied_interface_id; }
    bool action_byte(int, int) override {
        // The action records' device state ([record+2Ch]+0Bh/+10h): no device
        // drives an in-mission action here. Recorded once per update.
        return false;
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    void reset_c0_00525170() override {
        owner_.record("UnitPickScreen::reset_c0", 0x00525170u);
    }
    bool lock_branch(bsp::UnitPickScreenState&, std::uint32_t address) override {
        owner_.record("UnitPickScreen::lock_branch", address);
        return true;
    }
    void tail_sound_00a7e490() override {
        owner_.record("UnitPickScreen::tail_sound", 0x00a7e490u);
    }

private:
    GameHudHost::Impl& owner_;
    std::vector<std::size_t> team_;
    int game_19c4_queries_{0};
};
}  // namespace

const std::vector<float>& GameHudHost::Impl::lock_radius_multipliers() {
    if (!lock_radius_read) {
        lock_radius_read = true;
        if (lua == nullptr || !lua->read_lock_radius_multipliers_0087dc85(lock_radius)) {
            lock_radius.clear();
        }
    }
    return lock_radius;
}

void GameHudHost::update_unit_pick_screen_00527260(float seconds) {
    Impl& impl = *impl_;
    // 00527419..005275D5 read the action records' device bytes directly
    // (CEh, CFh, D0h's neighbours D1h/D3h/D4h/D5h); one record per update.
    impl.record("UnitPickScreen::input_record_bytes", 0x00527419u);
    UnitPickBinding binding(impl);
    bsp::unit_pick_screen_update_00527260(impl.unit_pick, binding, seconds);
    impl.done("UnitPickScreen::update", 0x00527260u);
}

void GameHudHost::update_markers_screen_006435d0(float seconds) {
    Impl& impl = *impl_;
    impl.step_mission_camera(seconds);
    if (!impl.markers) {
        impl.record("HudMarkers::update", 0x006435d0u);
        return;
    }
    impl.markers->update_006435d0(seconds);
}

void GameHudHost::apply_in_game_interface_004c9ca0(bool loading) {
    Impl& host = *impl_;
    // 004c9cc0 tests the one stack argument and branches on it, so this is two
    // routines sharing a frame. The load calls it with 1 at 004e1873 and the
    // mission-state entry with 0 at 004da746.
    if (loading) {
        // 004c9ccd..004c9d68, the arm that puts the loading element up. The
        // element is allocated at 004c9cd7 and constructed by 00636d90, neither
        // of which is reconstructed, so both are records; the two stores this
        // process can make are the slot at manager+D4h and the byte at +4h.
        if (!host.loading_element_present) {
            host.record("MissionLoad::allocate_loading_element", 0x00bf681bu);
            host.record("MissionLoad::construct_loading_element", 0x00636d90u);
            host.loading_element_present = true;
            // 004c9d11, the element's own vtable +10h right after the store.
            host.record("MissionLoad::loading_element_init", 0x004c9d11u);
        }
        host.loading_element_4 = true;   // 004c9d1f
        host.done("MissionLoad::apply_in_game_interface", 0x004c9ca0u);
        host.log.notef("in-game interface applied with 1: the loading element at "
            "[00e198c4]+D4h is up and its +4h byte is set; the single-player branch at "
            "004c9d23 ends the routine, because game+1FE4h is zero here");
        return;
    }

    // 004c9d6b..004c9ea2, the arm the mission-state entry takes.
    if (host.loading_element_present) {
        if (host.loading_element_5) {
            // 004c9d7e, the element's exit virtual +1Ch, only when +5h is set.
            host.record("MissionEntry::loading_element_exit", 0x004c9d7eu);
        }
        host.loading_element_4 = false;  // 004c9d82
        host.loading_element_5 = false;  // 004c9d85
        // 004c9d88, 004f83b0 on the element: the cleared bytes are committed
        // into the page tree. No page is attached to this element, so the walk
        // reaches nothing and the call is recorded rather than claimed.
        host.record("MissionEntry::commit_loading_element", 0x004f83b0u);
        // 004c9daa, the element's deleting destructor through vtable +0Ch with
        // 1, then the slot is nulled at 004c9dac.
        host.record("MissionEntry::destroy_loading_element", 0x004c9daau);
        host.loading_element_present = false;
    }
    // 004c9dae..004c9dcf: the name at 00ce765c is assigned into a pooled string
    // and handed to the atlas manager at 00f8c26c. The sprite bridge owns every
    // atlas in this process, so the release is a record.
    host.record("MissionEntry::release_front_end_atlas", 0x00aefa30u);
    // 004c9dfb: PUSH 1; PUSH 3; CALL 004c1ac0; MOV ECX,EAX; CALL 00518250. The
    // two pushes are 00518250's arguments (set 3, commit 1) and 004c1ac0 takes
    // none: it is the lazy getter for the 164h front-end frame object at
    // 00e18d80. This is the committing call, and it is what releases the front
    // end's own layouts.
    host.done("MissionEntry::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, true);
    // 004c9e1d: 007f8d60(game+650h, game+2198h) and the latch at game+1EE0h,
    // which is the engine-movie flag the frame reads. The comparison is over the
    // player profile's own record set, which this process does not own.
    host.record("MissionEntry::engine_movie_check", 0x007f8d60u);
    // 004c9e32: the single-player tail. game+1FE4h is zero in this process, so
    // the branch at 004c9e38 jumps over the input-context push 004bec00 /
    // 00a933f0, the controlled-unit registry walk 008053c0 / 008073c0, the unit
    // list rebuild 004c3cb0 and the interface refresh 006485a0. Recording them
    // would claim call sites this run does not reach, so it does not.
    host.in_game_interface_applied = true;
    host.done("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
    host.log.notef("in-game interface applied with 0: the loading element is torn down and "
        "00518250(3, commit=1) at 004c9e06 releases every other set's layouts, so FE_frame "
        "and FE_frame_title come off the screen and set 3's GUI_pause pair is acquired "
        "hidden");
}

void GameHudHost::select_front_end_layout_00518250() {
    // The load's own row at 004e04e4: PUSH EBX (0); PUSH 3; CALL 004c1ac0; MOV
    // ECX,EAX; CALL 00518250. Milestone 2i read the two pushes as 004c1ac0's
    // arguments; they are 00518250's (set 3, commit 0). 004c1ac0 is
    // BSP_FrontEndFrame_GetOrCreate, the lazy getter for the 164h front-end
    // frame object at 00e18d80, and it takes none: it only supplies ECX. Set 3
    // is the pause pair GUI_pause / GUI_pause_title
    // (bsp/title_init.hpp, FrontEndFrameSet::Pause).
    impl_->done("MissionLoad::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, false);
}

void GameHudHost::release_main_menu_manager_00686c90() {
    impl_->menu.destroy_main_menu_manager_00686c90();
    impl_->log.note("the main-menu manager was destroyed: 00687300 -> 00686c90 -> 00683aa0, "
        "whose tail publishes the empty level-4 screen set (004f8710) and the empty level-4 "
        "input contexts (004d8c00), so the front-end pages come down");
}

void GameHudHost::update_interface_only_004c40f0(float raw_delta) {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.menu.pump_interface_only_004c40f0(raw_delta);
    ++impl.summary.pump_frames;
}

void GameHudHost::report() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.log.notef("summary mission hud screens=%zu/%zu pages=%zu/%zu widgets=%zu/%zu "
        "interface=%02Xh level1_screens=%zu level1_contexts=%zu pump_frames=%llu "
        "update_frames=%llu audio_environment=%s", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.pages_loaded,
        impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.applied_interface_id),
        impl.summary.level1_screens, impl.summary.level1_contexts,
        impl.summary.pump_frames, impl.summary.update_frames,
        impl.summary.audio_environment.empty() ? "(none)"
                                               : impl.summary.audio_environment.c_str());
    // Milestone 2j. `minimap_islandmap_Icon` of GUI_minimap names the texture
    // `error.tga` with the material `minimap_terrain.mshd`, and milestone 2h
    // read that as the page's own authored texture. It is, and the material is
    // not an ordinary single-texture one: the installed
    // shaderfx/gui/minimap_terrain.shfx declares two samplers, `RadarMap` at
    // texture register 0 with clamped addressing and `FadeBorder` at index 1,
    // and names no texture file at all. Sampler 0 is the widget's own texture
    // slot, which is why `error.tga` reaches it and why the icon already takes
    // exactly the texture path every other HUD texture takes (the bridge's VFS
    // loader standing in for the renderer's vtable +64h, 00aa5e60). What the
    // running game puts there instead is the mission's own radar map, which the
    // minimap screen 005bec50 binds after taking the widget by name at 005bed21
    // through 00aa7e00, and which the renderer owner produces. There is no
    // second texture for this process to load.
    impl.record("HudMinimap::radar_map_texture", 0x00aa5e60u);
    if (impl.minimap) impl.minimap->report();
    if (impl.markers) impl.markers->report();
    impl.log.note("minimap island-map icon: minimap_terrain.mshd is the two-sampler GUI "
        "shader effect shaderfx/gui/minimap_terrain.shfx (RadarMap at register 0, "
        "FadeBorder at index 1) and names no texture, so error.tga is sampler 0's authored "
        "value and the icon is already on the same texture path as every other HUD texture; "
        "what replaces it is the renderer owner's radar map");
}

bool GameHudHost::manager_active() const noexcept {
    return impl_->summary.manager_built && impl_->manager.base.active;
}

void GameHudHost::update_in_game_interface_0068c1f0() {
    Impl& impl = *impl_;
    if (!manager_active()) return;
    HudUpdateBinding binding(impl);
    bsp::update_in_game_interface_0068c1f0(impl.manager, impl.update_state, binding);
    impl.done("InGameInterface::update", 0x0068c1f0u);
    ++impl.summary.update_frames;
}

}  // namespace bsp::game
