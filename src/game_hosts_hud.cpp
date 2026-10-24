// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages.
// See include/bsp/game_hosts_hud.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/gui_aspect_extent.hpp"
#include "bsp/platform_window.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hint_passes.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/interface_runtime_tail.hpp"
#include "bsp/hud_movie_camera.hpp"
#include "bsp/unit_instance_layout.hpp"
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
#include "bsp/ocean_spectrum.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/gunnery_tables.hpp"      // kInstalledLosTargetHeightAdd (cc13-aimunit3)
#include "bsp/unit_gunnery_pass.hpp"   // kUnitGunneryKindShipBase (cc13-aimunit3)
#include "bsp/game_hosts_world.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/frontier_wiring_cc13.hpp"
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/cc13_front6.hpp"
#include "bsp/cc13_big1_null_host.hpp"
#include "bsp/hud_root_rows.hpp"
#include "bsp/local_player_unit_lists.hpp"

#include "bsp/hud_screens.hpp"
#include "bsp/in_mission_interface_runtime.hpp"
#include "bsp/ingame_interface.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/unit_kind_query.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// Packet cc9_hud_ray_pick (docs/HUD_PICK_SEGMENT_QUERY.md). True: 00526A40's
// camera segment query 009043A0 runs over the gunnery host's units
// (GameGunneryHost::query_segment_units) and the hit branch 00526C74..00526DB2
// picks a hit of kind 6, 0Fh, 45h, 46h, 1Bh or 35h by ray. False: the query
// reports no hit and the branch is a record.
constexpr bool kHudPickSegmentQueryBound = true;

// Packet cc13-wire1, granted by cc9 2026-10-18. 0064AE20 (src/cc13_big1.cpp,
// __fastcall(ECX = screen 45h), RET) is what 0064D590 calls at 0064D5F9 when
// screen+5 is set, after it stores the unit at +184h. The image: Lua
// VehicleClass[class+70h].NoRepairGUI hides +40h (the GUI_ship_damage page)
// when it is boolean 1; +44h = clamp((unit+980h + 0.5) / 1.5, 0, 1); the three gauges'
// +18h bytes cleared; +128h = 0064AAD0(), +12Ch = 0, 0064A9F0(0); flags
// +105h (class+C8h > 0), +107h (00818300 != 0), +106h (!+105h and 0059CB90
// == 16h), +104h (submarine kind 8 with 00852350 != 0); widget vt34 / vt88
// on +54h..+74h; then 0064A960. The current host (OFF): the gauges switch
// records 0064AE20 and the flags keep the enter's clear values. ON runs the
// body through ShipScreenApplyUnitAdapter below, whose stand-ins are: the Lua
// lookups (no NoRepairGUI key is read, so +40h shows), class+C8h = 0,
// 00818300 / 00852350 / 0059CB90 = 0, and the widgets at +40h, +54h, +58h and
// +68h..+74h, which 0064C0F0's host name map does not bind (no-ops).
// ON with both sub-switches (cc9-lua69, 2026-10-18): USN01 all-OFF vs all-ON pair,
// gameplay identical (presentation only; SQUADRON_LAND_TASK 5ir).
inline constexpr bool kHudShipScreenApplyUnitBound = true;

// Packet cc13-wire2, granted by cc9 2026-10-18: two OFF sub-switches that close
// stand-ins of the adapter above. Neither changes anything while
// kHudShipScreenApplyUnitBound is OFF, and each is OFF on its own.
// - kHudShipScreenApplyUnitLuaBound: the class reads 0064AE20 makes through
//   GameMissionLuaHost. VehicleClass[class+70h].NoRepairGUI (0064AED4, key
//   00CF5C88) through read_vehicle_class_boolean(type_id, .., false), and
//   class+C8h, which 00960503 stores from Catapult.LaunchStock (IntegerOr 0,
//   vehicle_class_fields.hpp kLaunchStock), through read_vehicle_class_integer.
//   OFF: NoRepairGUI reads as not a boolean (+40h shown) and class+C8h = 0.
// - kHudShipScreenApplyUnitWidgetsBound: 0064C0F0's stores for the offsets
//   0064AE20 drives, read from the bytes: +40h is the GUI_ship_damage PAGE
//   (00AA5840 at 0064CD06, name 00CF5F4C, stored at 0064CD0B), bound to the
//   page root as game_hosts_mission.cpp ~1763 does; +54h ship_recon_Icon
//   (00CF6044, 0064C5D8), +58h ship_torpedo_Icon (00CF6030, 0064C648), +68h
//   ship_recon_num1_Icon (00CF60B0, 0064C3A5), +6Ch ship_recon_num2_Icon
//   (00CF6098, 0064C415), +70h ship_torpedo_num1_Icon (00CF6080, 0064C485),
//   +74h ship_torpedo_num2_Icon (00CF6068, 0064C4F5). OFF: they stay unbound.
// LABELLED (both ON): the Lua reads key VehicleClass by class+70h taken as the row's
// type_id, which is inferred, not traced.
inline constexpr bool kHudShipScreenApplyUnitLuaBound = true;
inline constexpr bool kHudShipScreenApplyUnitWidgetsBound = true;

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// The ocean the camera's probes sample: 0078CF20 over the same flat wave field
// the unit host runs (amplitude +24h 0.0f, no coverage regions;
// src/game_hosts_units.cpp OceanFieldBinding).
class CameraOceanField final : public bsp::OceanHeightHost {
public:
    float wave_height_0078c890(float x, float z) override {
        if (bsp::ocean_live_field_ready()) {  // packet cc9_physics1_sea
            const bsp::OceanLiveField& live = bsp::ocean_live_field_process();
            return bsp::ocean_wave_field_sample_0078c890(live.field,
                bsp::ocean_spectrum_grid(live.spectrum), x, z);
        }
        bsp::OceanWaveFieldState field;
        field.flat_f9 = false;
        field.inv_tile_b4 = static_cast<float>(1.0 / 100.0);
        field.amplitude_24 = 0.0f;
        const bsp::OceanWaveGridView zero_grid{};
        return bsp::ocean_wave_field_sample_0078c890(field, zero_grid, x, z);
    }
    float coverage_mask_00b9cf50(float x, float z) override {
        return bsp::ocean_live_field_coverage_mask(x, z);  // packet cc9_physics1_sea
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
    // 0064AE20 through the Big1Host adapter (kHudShipScreenApplyUnitBound).
    void apply_ship_screen_unit_0064ae20();
    // Packet cc9_screen_50h: screen 50h's fields and its widgets.
    bsp::WarningScreenState warning_screen{};
    bool warning_widgets_bound{false};
    std::map<int, GuiLayoutWidget*> warning_widgets;
    GuiLayoutWidget* warning_widget(bsp::WarningWidget widget);
    // Packet cc9_screen_49h.
    bsp::FollowScreen49State follow_screen{};
    // Packet cc13-fix24: the hints screen at HUD manager +A8h (vtable 00CEE290,
    // 7Ch bytes), read only by 0054D3C0 at +54h (seconds left). Register
    // 0054D900 stores +54h = 0.0f; ShowHint, Update and Dismiss have no host
    // path, so the host's copy keeps that value. No other field is modelled.
    alignas(4) std::uint8_t hints_screen[0x7C]{};
    // Packet cc9_screen_46h: screen 46h's +1Ch, stored by 0064DA40.
    bool ship_view_has_unit{false};
    std::size_t ship_view_unit{SIZE_MAX};
    // Packet cc13-releasegun (kHarnessReleaseGunMapBound): armed by the plan's
    // first `releasegun` line; map_up is screen 4Ch's +4h ([this+54h]+4h)
    // once 0068AC30 has published the level-2 set.
    bool release_gun_armed{false};
    bool release_gun_map_up{false};
    void back_out_one_overlay_level_0068b470();
    void log_map_state(const char* state);
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
    // Screen 2Eh, the weapon groups (00549260, 005484F0).
    bsp::HudWeaponGroupScreenState weapon_group{};
    // Packet cc9_player_gun_seat: the message 00954A10 last built.
    GunAimMessage79 pending_gun_message{};
    bool pending_gun_message_valid{false};
    // cc15-smA (kGunAimMessageCc15Bound): the 79h session copy, and what the
    // copy changed against the angles the OFF delivery derives from forward.
    bsp::HostSessionMessages session_messages;
    unsigned long long gun_aim_round_trips{0};
    unsigned long long gun_aim_changed[6]{};  // yaw, pitch, camera, bools, group, target
    double gun_aim_max_yaw_delta{0.0};
    double gun_aim_max_pitch_delta{0.0};
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
    // The helm `aim unit` track's diagnostics (packet cc13-aimunit).
    unsigned long long aim_track_frames{0};
    bool aim_track_warned{false};
    unsigned aim_track_serial{0};
    bool unit_request_pending{false};
    bool unit_request_applied{false};
    int unit_interface_id{0};
    // Packet cc9_set_selected_unit: 00647300's calls, acceptances, and the
    // 20h pushes it made (serviced through the same pending/applied pair).
    unsigned long long select_calls{0};
    // Packet cc9_controlled_unit_observer: the unit the root observer is on.
    std::size_t observed_unit{SIZE_MAX};
    unsigned long long observer_notices{0};
    unsigned long long observer_releases{0};
    // Packet cc9_squadron_observer_liveness: slot deaths the squadron outlived.
    unsigned long long observer_squadron_holds{0};
    bool squadron_hold_logged{false};
    void run_root_observer_notice();
    // Packet cc9_force_select_unit: the HUD root's unit vectors and cursor.
    bsp::HudRootUnitLists root_lists{};
    unsigned long long root_rebuilds{0};
    unsigned long long force_calls{0};
    unsigned long long force_selects{0};
    void run_root_lists(bool force);
    unsigned long long select_accepted{0};
    unsigned long long select_pushes{0};
    bool pick_first_basis_logged{false};
    // Packet cc9_movie_interface_and_reseed: the movie screen 37h's +1Ch/+20h,
    // the 00E19894 lock as 005CD1A0 sees it (never raised in single player),
    // and a pending non-20h request (004CC460 writes the one pending record,
    // so a later push replaces an earlier one).
    bsp::HudMovieScreenState movie_screen{};
    bsp::FrontEndInterfaceLock movie_lock{};
    int pending_interface_id{0};
    unsigned long long movie_calls{0};
    unsigned long long movie_engages{0};
    unsigned long long movie_pushes{0};
    unsigned long long movie_reseeds{0};
    // Packet cc9_movie_camera_mover_bind: the movie camera (screen 37h's +1Ch)
    // while it is the installed mover.
    bsp::HudMovieCamera movie{};
    bool movie_active{false};
    unsigned long long movie_builds{0};
    unsigned long long movie_destroys{0};
    unsigned long long movie_keyframes{0};
    unsigned long long movie_publishes{0};
    unsigned long long movie_unsupported{0};
    unsigned long long movie_events_fired{0};   // cc9_lua49_movie_modifier
    unsigned long long movie_step_draws{0};   // switch 4, 00798D07
    unsigned long long movie_last_pose_log{0};
    int pick_basis_source{0};   // 0 none, 1 ShipCaptain, 2 movie camera
    bool build_movie_camera_005cc170();
    void destroy_movie_camera(const char* why);
    void step_movie_camera(float seconds);
    void reseed_stream(int stream, std::uint32_t seed, std::uint32_t site);
    bsp::UnitSelectableInputs selectable_inputs_00645060(std::size_t unit);
    // Packet cc9_vtable124_liveness: unit->vtable[124h]() by class.
    bool unit_vtable_124(std::size_t unit, std::uint32_t site);
    // Packet cc9_squadron_slot_class: the class the selection tests see, and
    // their IsKindOf over it. A squadron's fused slot answers 18h when bound.
    int selection_class_id(std::size_t unit) const;
    bool selection_is_kind_of(std::size_t unit, int class_id) const;
    bool is_squadron_slot(std::size_t unit) const;
    std::vector<std::size_t> squadron_slots_seen;   // census, each once
    // The units the class map answers true for and the ship-only rule does not,
    // each counted once, with the site that first asked.
    std::vector<std::pair<std::size_t, std::uint32_t>> vtable124_map_only;
    unsigned long long vtable124_asks{0};
    unsigned long long vtable124_map_only_asks{0};

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
        // 0068AE84 007BB9A0(unit) (packet cc9_plane_in_flight_test).
        bsp::PilotCmdLocalInputGate gate{};
        const bool plane = owner_.units != nullptr && owner_.units->controlled_bound()
            && owner_.units->plane_local_input_gate_007bb9a0(
                owner_.units->controlled_index(), gate);
        const bool answer = plane && bsp::pilot_cmd_local_input_enabled_007bb9a0(gate);
        owner_.log.notef("InGameInterface 007BB9A0 for the 20h payload: plane=%d c0c=%d aa0=%.1f "
            "chA=%d/%.2f 00604a20=%d 5d=%d -> %d (bound=%d)", plane ? 1 : 0,
            gate.enable_byte_c0c ? 1 : 0, static_cast<double>(gate.gate_float_aa0),
            gate.gate_block_present ? 1 : 0, static_cast<double>(gate.gate_block_value),
            gate.global_block_00604a20 ? 1 : 0, gate.unit_suppressed ? 1 : 0, answer ? 1 : 0,
            kPlaneInFlightTestBound ? 1 : 0);
        if constexpr (kPlaneInFlightTestBound) {
            owner_.done("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
            return answer;
        } else {
            owner_.record("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
            return false;
        }
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
            owner_.ship_view_unit = owner_.units->controlled_index();
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
                if constexpr (kHudShipScreenApplyUnitBound) {
                    // 0064D5F1: only with screen+5 set.
                    if (owner_.ship_screen.active_05) owner_.apply_ship_screen_unit_0064ae20();
                } else {
                    owner_.record("HudShipScreen::apply_unit_0064ae20", 0x0064ae20u);
                }
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
        // 00e188d8, written by BSP_Game_SetControlledUnit 004c0880; the units
        // host binds its controlled unit through 004c0890.
        if constexpr (kHudInterfaceControlledUnitBound) {
            owner_.done("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
            return owner_.units != nullptr && owner_.units->controlled_bound();
        } else {
            owner_.record("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
            return false;
        }
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
    bool modifier_key_allows_back_out() override {
        // 0068C65A..0068C66E: [00F88A30] or GetKeyState(A4h, VK_LMENU) >= 0.
        // Packet cc13-releasegun: an armed plan answers "left Alt not held";
        // the harness has no keyboard (LABELLED).
        return kHarnessReleaseGunMapBound && owner_.release_gun_armed;
    }
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
        if (kHarnessReleaseGunMapBound && owner_.release_gun_armed) {
            owner_.back_out_one_overlay_level_0068b470();
            return;
        }
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
    bool screen_34_query() override {
        if constexpr (kHudHintsSecondsLeftBound) {
            // 0068C526..0068C533: ECX = [this+A8h]; CALL 0054D3C0; TEST EAX,EAX;
            // JNE. Non-zero for |+54h| >= 1.0 and for NaN or |+54h| >= 2^31
            // (80000000h), zero for +54h in (-1, 1).
            owner_.done("InGameInterfaceUpdate::screen_34_query", 0x0054d3c0u);
            const bsp::cc13_sweepA::Raw screen{owner_.hints_screen};
            return bsp::cc13_front6::hints_screen_seconds_left_0054d3c0(screen) != 0;
        } else {
            return record_false("0054d3c0", 0x0054d3c0u);
        }
    }
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
    bool controlled_unit_is_submarine() override {
        // 0068CC04: IsKindOf(8) on 00E188D8.
        if constexpr (kHudInterfaceControlledUnitBound) {
            return owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 8);
        } else {
            return false;
        }
    }
    void refresh_controlled_unit_pose() override {}
    float controlled_unit_height() override {
        // The unit's +100h, the world translation's y.
        if constexpr (kHudInterfaceControlledUnitBound) {
            float r[3], u[3], f[3], o[3];
            if (owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_pose(owner_.units->controlled_index(), r, u, f, o)) {
                return o[1];
            }
        }
        return 0.0f;
    }
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
    int requested = kInterfaceScene3d;
    if (!impl.summary.interface_applied) {
        has_payload = false;
    } else if (impl.pending_interface_id != 0) {
        // Packet cc9_movie_interface_and_reseed: a non-20h push (005CD1CE's 2Ch
        // with [00E188D8]), serviced through the same 006840f0 / 00684600 path.
        requested = impl.pending_interface_id;
        impl.pending_interface_id = 0;
        has_payload = impl.units != nullptr && impl.units->controlled_bound();
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
        requested, has_payload);
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
    impl.log.notef("in-mission level-1 set for %s (%s payload, single player) "
        "applied as %02Xh: screens %s| contexts %s| pages %s",
        requested == kInterfaceScene3d ? "INTF_SCENE3D" : "request 2Ch (movie camera new)",
        has_payload ? "controlled unit" : "null",
        static_cast<unsigned>(impl.summary.applied_interface_id), screens.c_str(),
        contexts.c_str(), pages.c_str());
    if (has_payload && requested == kInterfaceScene3d) {
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

namespace {
// bsp::HudWeaponGroupLayoutHost over the two pages screen 2Eh registers.
class WeaponGroupLayoutBinding final : public bsp::HudWeaponGroupLayoutHost {
public:
    explicit WeaponGroupLayoutBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::uintptr_t page_58() override { return page("GUI_cross_ship"); }
    std::uintptr_t page_dc() override { return page("GUI_cross_gunstate"); }
    std::uintptr_t find_child(std::uintptr_t parent, const char* name) override {
        if (parent == 0) return 0;
        return reinterpret_cast<std::uintptr_t>(bsp::find_descendant_by_name(
            *reinterpret_cast<GuiLayoutWidget*>(parent), name));
    }
    void place_f2(std::uintptr_t, std::uintptr_t, int) override {
        owner_.record("HudWeaponGroupScreen::place_f2", 0x00546da2u);
    }
    void set_visible(std::uintptr_t widget, bool visible) override {
        if (widget == 0) return;
        owner_.menu.frontend().set_widget_visible(
            *reinterpret_cast<GuiLayoutWidget*>(widget), visible);
    }

private:
    std::uintptr_t page(const char* name) {
        GuiLayoutPage* p = owner_.menu.in_game_page(0x2e, name);
        return p != nullptr && p->root ? reinterpret_cast<std::uintptr_t>(p->root.get()) : 0;
    }
    GameHudHost::Impl& owner_;
};

// bsp::HudWeaponGroupScreenHost over this process's HUD (packet
// cc9_screen_2eh_group). Screen 2Eh's layout 00546A20 is not run, so its
// widgets are records; units are index + 1.
class WeaponGroupScreenBinding final : public bsp::HudWeaponGroupScreenHost {
public:
    explicit WeaponGroupScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    bool unit_is_kind_of(std::size_t unit, int kind) override {
        return unit != 0 && owner_.units != nullptr && owner_.units->unit_is_kind_of(unit - 1, kind);
    }
    bool local_player_role(std::size_t unit, int role) override {
        // 00927F30(unit, role): the holder at unit+1ACh is the local player's
        // slot 0, as for screen 46h (section 21).
        std::int32_t holder = -1;
        return unit != 0 && owner_.units != nullptr
            && owner_.units->unit_current_role_slot(unit - 1, role, holder) && holder == 0;
    }
    bool group_available_009542b0(std::size_t unit, int group) override {
        if constexpr (kHudGunnerRoleTakeBound) {
            return group_available_bound(unit, group);
        } else {
            // SUBSTITUTION: "not available". The image answers from the
            // unit's permission word (+194h/+198h/+19Ch/+1A4h against
            // PLAYER_ANY or the local slot) and its gun list at unit+48h, and
            // a yes makes the bind's cycle take that group's gunner roles
            // (00545410 -> 0077C470). Section 27.
            static_cast<void>(unit);
            static_cast<void>(group);
            owner_.record("HudWeaponGroupScreen::group_available", 0x009542b0u);
            return false;
        }
    }
    void role_request_0077c470(std::size_t unit, std::uint32_t mask, bool take) override {
        if constexpr (kHudGunnerRoleTakeBound) {
            if (unit != 0 && owner_.units != nullptr) {
                owner_.units->role_request_0077c470(unit - 1, mask, take);
                owner_.done("HudWeaponGroupScreen::role_request", 0x0077c470u);
                return;
            }
        }
        static_cast<void>(unit);
        static_cast<void>(mask);
        static_cast<void>(take);
        owner_.record("HudWeaponGroupScreen::role_request", 0x0077c470u);
    }
    // 009542B0(unit, group) for groups 2..5 (section 31).
    bool group_available_bound(std::size_t unit, int group) {
        if (unit == 0 || owner_.units == nullptr) return false;
        const std::size_t index = unit - 1;
        // 0059BBD0(unit, role, slot): the permission word is 9 or the local
        // slot game+18ECh (0 here).
        auto open = [&](int role) {
            std::int32_t word = 8;
            return owner_.units->unit_role_permission(index, role, word)
                && (word == 9 || word == 0);
        };
        bool permitted = false;
        switch (group) {
        case 2: permitted = open(3) || open(2); break;   // +194h, else 0059BBD0(2)
        case 3: permitted = open(4); break;              // +198h
        case 4: permitted = open(5); break;              // +19Ch
        case 5: permitted = open(7); break;              // +1A4h
        default: return false;
        }
        owner_.done("HudWeaponGroupScreen::group_available", 0x009542b0u);
        if (!permitted) return false;
        if (group != 5 && owner_.units->unit_is_kind_of(index, 8)) {
            // A kind-8 unit's depth tests (the pose y against 00CE3D50, and
            // BSP_Submarine_IsShallowEnoughToEngage for group 4): not held.
            owner_.record("HudWeaponGroupScreen::submarine_depth", 0x009542b0u);
            return false;
        }
        const GameGunneryHost* gunnery = owner_.units->gunnery();
        if (gunnery == nullptr) {
            owner_.record("HudWeaponGroupScreen::gun_list", 0x009542b0u);
            return false;
        }
        // The kind-20h children on unit+48h; the weapon Function is
        // [gun+3F4h]+80h, which the gunnery host keeps as the category.
        for (const GameGunRow& gun : gunnery->guns()) {
            if (gun.unit_index != index) continue;
            const int f = gun.category;
            switch (group) {
            case 2: if (f == 1 || f == 5 || f == 6) return true; break;   // 00728A90
            case 3: if (f == 2 || f == 3 || f == 4 || f == 6) return true; break;  // 005459B0
            case 4: if (f == 7) return true; break;
            case 5: if (f == 8 || f == 9) return true; break;
            default: break;
            }
        }
        return false;
    }
    std::vector<std::size_t> unit_records_778(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::unit_records_778", 0x00549337u);
        return {};
    }
    void clear_child_flags_005470c0(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::clear_child_flags", 0x005470e7u);
    }
    void reset_widgets_005464e0() override {
        owner_.record("HudWeaponGroupScreen::reset_widgets", 0x005464e0u);
    }
    bool player_unit_is_lst_rocket() override {
        owner_.record("HudWeaponGroupScreen::player_unit_name", 0x005493f9u);
        return false;
    }
    void lvlaa_hint_00548360() override {
        // 00548378: [00E188D8] must be kind 18h (a plane squadron); a ship
        // ends the routine here whatever the applied byte +5h holds.
        if (owner_.units != nullptr && owner_.units->controlled_bound()
            && !owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 0x18)) {
            return;
        }
        owner_.record("HudWeaponGroupScreen::lvlaa_hint", 0x00548360u);
    }
    bool player_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    void one_shot_108_0054856b() override {
        owner_.record("HudWeaponGroupScreen::one_shot_108", 0x0054856bu);
    }
    void one_shot_109_0054861b(float pitch_10c, float yaw_110) override {
        static_cast<void>(pitch_10c);
        static_cast<void>(yaw_110);
        owner_.record("HudWeaponGroupScreen::one_shot_109", 0x0054861bu);
    }
    void hide_widgets_005452f0() override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            bsp::HudWeaponGroupScreenState& s = layout();
            WeaponGroupLayoutBinding gui(owner_);
            for (auto& row : s.cells) {
                for (std::uintptr_t cell : row) gui.set_visible(cell, false);   // 005452FA
            }
            gui.set_visible(s.disable_c0, false);                     // 0054531A..
            gui.set_visible(s.aim_c4, false);
            gui.set_visible(s.aim_c8, false);
            gui.set_visible(s.aim_cc, false);
            owner_.done("HudWeaponGroupScreen::hide_widgets", 0x005452f0u);
        } else {
            owner_.record("HudWeaponGroupScreen::hide_widgets", 0x005452f0u);
        }
    }
    // 00546A20 has run at registration in the image; here it runs once,
    // before the first widget use (labelled).
    bsp::HudWeaponGroupScreenState& layout() {
        bsp::HudWeaponGroupScreenState& s = owner_.weapon_group;
        if (!s.layout_done) {
            WeaponGroupLayoutBinding gui(owner_);
            bsp::weapon_group_layout_00546a20(s, gui);
            owner_.done("HudWeaponGroupScreen::layout", 0x00546a20u);
        }
        return s;
    }
    void group_sight_00548300(int group) override {
        static_cast<void>(group);
        owner_.record("HudWeaponGroupScreen::group_sight", 0x00548300u);
    }
    void rocket_widgets_00548716() override {
        owner_.record("HudWeaponGroupScreen::rocket_widgets", 0x00548716u);
    }
    void widget_98_005486c7(bool flag_d5) override {
        static_cast<void>(flag_d5);
        owner_.record("HudWeaponGroupScreen::widget_98", 0x005486c7u);
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool input_held(int action) override { return owner_.menu.input_action_held(action); }
    bool input_axis_active_004d9480(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_axis_active", 0x004d9480u);
        return false;
    }
    float input_axis_value_004c5070(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_axis_value", 0x004c5070u);
        return 0.0f;
    }
    bool input_in_set_00547250(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_in_set", 0x00547250u);
        return false;
    }
    float clock_f876a4() override {
        owner_.record("HudWeaponGroupScreen::clock", 0x00548808u);
        return 0.0f;
    }
    bool lock_branch_00548b06(bsp::HudWeaponGroupScreenState&) override {
        owner_.record("HudWeaponGroupScreen::lock_branch", 0x00548b06u);
        return false;
    }
    bool torpedo_branch_00548cb6(bsp::HudWeaponGroupScreenState&) override {
        owner_.record("HudWeaponGroupScreen::torpedo_branch", 0x00548cb6u);
        return false;
    }
    std::size_t hud_target_00548856() override {
        if constexpr (kHudWeaponGroupTargetBound && kHudUnitPickScreenBound) {
            // The manager's +CCh is screen 29h; its +4Ch is the unit pick.
            owner_.done("HudWeaponGroupScreen::hud_target", 0x00548856u);
            return owner_.unit_pick.pick_4c;
        } else {
            owner_.record("HudWeaponGroupScreen::hud_target", 0x00548856u);
            return 0;
        }
    }
    float gunner_distance_00548ea9(std::size_t unit, std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548EA9..00548ECC: 00427EB0 on both (pose +FCh), 00427C90,
            // then 00427E30, the squared length.
            float r[3], u[3], f[3], up[3], tp[3];
            if (unit == 0 || target == 0 || owner_.units == nullptr
                || !owner_.units->unit_pose(unit - 1, r, u, f, up)
                || !owner_.units->unit_pose(target - 1, r, u, f, tp)) {
                return 0.0f;
            }
            const float d[3] = {up[0] - tp[0], up[1] - tp[1], up[2] - tp[2]};
            owner_.done("HudWeaponGroupScreen::gunner_distance", 0x00548ea9u);
            return d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
        } else {
            owner_.record("HudWeaponGroupScreen::gunner_distance", 0x00548ea9u);
            return 0.0f;
        }
    }
    float gunner_range_49c(std::size_t unit) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548ED0: [unit+49Ch], which 00953B80 writes.
            GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (unit == 0 || gunnery == nullptr) return 0.0f;
            owner_.done("HudWeaponGroupScreen::gunner_range", 0x00548ed0u);
            return gunnery->unit_gun_range_sq_049c(unit - 1);
        } else {
            owner_.record("HudWeaponGroupScreen::gunner_range", 0x00548ed0u);
            return 0.0f;
        }
    }
    bool target_virtual_10c(std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // vtable[10Ch]: 0074E1A0 on the plane classes, the
            // cached speed [unit+B1Ch] (007B8E60) above 3.0 (00CE3854,
            // FCOMIP/JBE); 0042BB40 (false) on every ship, submarine and the
            // dummy kamikaze. The cache is the magnitude 007D8020 stores of the
            // flight controller's world velocity.
            if (target == 0 || owner_.units == nullptr
                || !owner_.units->unit_is_kind_of(target - 1, bsp::kUnitGunneryKindPlaneBase)) {
                return false;
            }
            float v[3];
            if (!owner_.units->unit_plane_world_velocity(target - 1, v)) return false;
            const float speed = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            owner_.done("HudWeaponGroupScreen::target_virtual_10c", 0x0074e1a0u);
            return speed > 3.0f;
        } else {
            owner_.record("HudWeaponGroupScreen::target_virtual_10c", 0x00548923u);
            return false;
        }
    }
    int relation_00803ce0(std::size_t, std::size_t) override {
        owner_.record("HudWeaponGroupScreen::relation", 0x00803ce0u);
        return 0;
    }
    bool same_side_54(std::size_t target, std::size_t first) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548913..0054891F: [target+54h] against the screen unit's +54h.
            if (target == 0 || first == 0 || owner_.units == nullptr) return true;
            owner_.done("HudWeaponGroupScreen::same_side", 0x00548919u);
            return owner_.units->unit_side_0054(target - 1)
                == owner_.units->unit_side_0054(first - 1);
        } else {
            owner_.record("HudWeaponGroupScreen::same_side", 0x00548919u);
            return true;
        }
    }
    std::uint16_t target_id_174(std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548934 MOVZX [target+174h]. SUBSTITUTION: the host has no
            // entity handle table, so the id is the 1-based unit index that
            // apply_gun_aim_message resolves in place of 00521E30.
            owner_.done("HudWeaponGroupScreen::target_id", 0x00548934u);
            return static_cast<std::uint16_t>(target);
        } else {
            owner_.record("HudWeaponGroupScreen::target_id", 0x00548934u);
            return 0;
        }
    }
    void target_debug_00548942(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::target_debug", 0x00548942u);
    }
    bool row_widget_present(int row, int column) override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            owner_.done("HudWeaponGroupScreen::row_widget", 0x005488d5u);
            return row >= 0 && row < 5 && column >= 0 && column < 5
                && layout().cells[row][column] != 0;
        } else {
            owner_.record("HudWeaponGroupScreen::row_widget", 0x005488d5u);
            return false;
        }
    }
    void show_row_widget(int row, int column, bool shown_d4) override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            if (row >= 0 && row < 5 && column >= 0 && column < 5) {
                WeaponGroupLayoutBinding gui(owner_);
                gui.set_visible(layout().cells[row][column], shown_d4);
            }
            owner_.done("HudWeaponGroupScreen::show_row_widget", 0x005488f9u);
        } else {
            owner_.record("HudWeaponGroupScreen::show_row_widget", 0x005488f9u);
        }
    }
    void build_fire_message_00954a10(int group, bool pressed, bool held, bool has_target,
                                     std::uint16_t target_id) override {
        if constexpr (kPlayerGunSeatBound) {
            // 00548839..005489DA: the mover's world matrix (0042D7E0) forward
            // row and position (00427EB0); the Operator node carries the
            // mover's published pose.
            bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
            if (node.ready) {
                if ((node.state.transform.valid_flags & 2u) == 0) {
                    bsp::refresh_camera_world_00b6db70(node.state.transform);
                }
                const bsp::CameraMatrix& world = node.state.transform.world;
                GunAimMessage79& m = owner_.pending_gun_message;
                m.group = group;
                for (int i = 0; i < 3; ++i) {
                    m.forward[i] = world[8 + i];
                    m.camera[i] = world[12 + i];
                }
                m.held_34 = held;
                m.pressed_35 = pressed;
                m.has_target_36 = has_target;
                m.target_38 = target_id;
                owner_.pending_gun_message_valid = true;
                owner_.done("HudWeaponGroupScreen::fire_message", 0x00954a10u);
                return;
            }
            owner_.pending_gun_message_valid = false;
        }
        static_cast<void>(group);
        static_cast<void>(pressed);
        static_cast<void>(held);
        static_cast<void>(has_target);
        static_cast<void>(target_id);
        owner_.record("HudWeaponGroupScreen::fire_message", 0x00954a10u);
    }
    // cc15-smA: count what the 79h copy changed, field by field, against the
    // posted value and the angles the OFF delivery derives from `forward`.
    void note_gun_aim_round_trip(const GunAimMessage79& m, const GunAimMessage79& q) {
        ++owner_.gun_aim_round_trips;
        const float fy = std::max(-1.0f, std::min(1.0f, m.forward[1]));
        const float pitch = std::max(-1.57f, std::min(1.57f, std::asin(fy)));
        const float yaw = std::atan2(m.forward[0], m.forward[2]);
        const double dy = std::fabs(static_cast<double>(q.yaw_2c) - yaw);
        const double dp = std::fabs(static_cast<double>(q.pitch_30) - pitch);
        if (q.yaw_2c != yaw) ++owner_.gun_aim_changed[0];
        if (q.pitch_30 != pitch) ++owner_.gun_aim_changed[1];
        if (std::memcmp(q.camera, m.camera, sizeof q.camera) != 0) ++owner_.gun_aim_changed[2];
        if (q.held_34 != m.held_34 || q.pressed_35 != m.pressed_35
            || q.has_target_36 != m.has_target_36) {
            ++owner_.gun_aim_changed[3];
        }
        if (q.group != m.group) ++owner_.gun_aim_changed[4];
        if (q.target_38 != m.target_38) ++owner_.gun_aim_changed[5];
        if (dy > owner_.gun_aim_max_yaw_delta) owner_.gun_aim_max_yaw_delta = dy;
        if (dp > owner_.gun_aim_max_pitch_delta) owner_.gun_aim_max_pitch_delta = dp;
    }
    void route_fire_message_0077c2a0(std::size_t unit) override {
        if constexpr (kPlayerGunSeatBound) {
            GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (unit != 0 && gunnery != nullptr && owner_.pending_gun_message_valid) {
                if constexpr (bsp::kGunAimMessageCc15Bound) {
                    // 00548A1F: 0077C2A0(flags 3) takes the local bit, and
                    // 0076E520's copy is what 00959C20 reads.
                    const GunAimMessage79& m = owner_.pending_gun_message;
                    const GunAimMessage79 queued = owner_.session_messages.queue_gun_aim_79(m);
                    note_gun_aim_round_trip(m, queued);
                    gunnery->apply_gun_aim_message_00959c20(unit - 1, queued);
                } else {
                    gunnery->apply_gun_aim_message_00959c20(unit - 1, owner_.pending_gun_message);
                }
                owner_.done("HudWeaponGroupScreen::route_fire_message", 0x0077c2a0u);
                return;
            }
        }
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::route_fire_message", 0x0077c2a0u);
    }
    bool camera_busy_00518f30() override {
        owner_.record("HudWeaponGroupScreen::camera_busy", 0x00518f30u);
        return false;
    }
    bool depth_charge_child_00549061(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::depth_charge_child", 0x00549061u);
        return false;
    }
    void hold_camera_005490a6(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::hold_camera", 0x005490a6u);
    }
    int input_mode_f8a0c4() override {
        owner_.record("HudWeaponGroupScreen::input_mode", 0x0054917du);
        return 0;
    }
    void show_widget_c0_005491c2() override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            WeaponGroupLayoutBinding gui(owner_);
            gui.set_visible(layout().disable_c0, true);
            owner_.done("HudWeaponGroupScreen::widget_c0", 0x005491c2u);
        } else {
            owner_.record("HudWeaponGroupScreen::widget_c0", 0x005491c2u);
        }
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::Impl::bind_mission_camera_0064da40() {
    if (units == nullptr || !units->controlled_bound()) return;
    const std::size_t index = units->controlled_index();
    const GameUnitRow* row = units->unit_row(index);
    // 0064DA64: screen 2Eh's 005470C0 before anything else.
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(*this);
        bsp::weapon_group_screen_release_005470c0(weapon_group, group_host);
    }
    // 0064DA69..0064DA77: with +1Ch set, 0077C470(+1Ch, 1FEh, 0) releases the
    // previous unit's roles 1..8 before 0064DAA3 stores the new one (the caller
    // stores ship_view_unit after this returns). Packet cc13-relgaps gap 3. The
    // image's +1Ch is cleared by its observer when that entity is deleted; host
    // slots are never deleted in a mission and ship_view_has_unit lives until the
    // mission reset, so the guard is the slot bound, as at the 0064A490 exit.
    if constexpr (kShipViewPreviousUnitReleaseCc13Bound) {
        if (ship_view_has_unit && ship_view_unit < units->count()) {
            units->role_request_0077c470(ship_view_unit, 0x1FEu, false);
            done("HudShipView::release_previous_roles", 0x0077c470u);
        }
    }
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
    // Packet cc9_movie_camera_mover_bind: 004BC410 at 0064DC9C installs the
    // ShipCaptain and destroys the outgoing mover through 00926D90; when that
    // is the movie camera, its observer clears screen 37h's +1Ch.
    if (movie_active) destroy_movie_camera("0064DA40 installed the ShipCaptain (004BC410 at 0064DC9C)");
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
    // 0064DCC4: 00549260(screen 2Eh, 46h's +1Ch the unit, 46h's +20h the mover).
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(*this);
        bsp::weapon_group_screen_bind_00549260(weapon_group, group_host, index + 1, camera_bound);
        done("HudWeaponGroupScreen::bind", 0x00549260u);
    }
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
    if (!kMissionCameraBound) return;
    if (!movie_active && !camera_bound) return;
    // The mover is a world entity ticked by the world update before the
    // interface runs. SUBSTITUTION: this host ticks it once per in-game
    // interface frame, at the first screen update of that frame, with the
    // screen's own delta.
    if (camera_last_frame == summary.update_frames) return;
    camera_last_frame = summary.update_frames;
    if (movie_active) {
        // Packet cc9_movie_camera_mover_bind: the installed mover is the movie
        // camera (004BC410 at 005CC2A9 replaced the ShipCaptain).
        step_movie_camera(seconds);
        return;
    }
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

namespace {
// The HUD whose world is attached, for hud_set_selected_unit_00647300. The
// image's route is the global manager [00E198C4] and its +40h (slot 44h).
GameHudHost*& attached_hud_for_selection() noexcept {
    static GameHudHost* hud = nullptr;
    return hud;
}
}  // namespace

bool hud_set_selected_unit_00647300(std::size_t unit, bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr) return false;
    return hud->set_selected_unit_00647300(unit);
}

bool hud_force_select_unit_006485a0() {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return false;
    hud->force_select_unit_006485a0();
    return true;
}

bool hud_arm_release_gun_map_toggle(bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr || !kHarnessReleaseGunMapBound) return false;
    hud->arm_release_gun_map_toggle();
    return true;
}

void GameHudHost::arm_release_gun_map_toggle() noexcept {
    if (kHarnessReleaseGunMapBound) impl_->release_gun_armed = true;
}

bool GameHudHost::release_gun_armed() const noexcept {
    return kHarnessReleaseGunMapBound && impl_->release_gun_armed;
}

// Packet cc13-releasegun, for cc9's plans: one line per map state change (log
// only, no switch), naming the ship-view unit whose roles 0064A490 releases.
void GameHudHost::Impl::log_map_state(const char* state) {
    const std::size_t unit = ship_view_unit;
    const GameUnitRow* row = units != nullptr && unit < units->count()
        ? units->unit_row(unit) : nullptr;
    log.notef("map %s t=%llu (hud update frame) unit=%s", state, summary.update_frames,
        row != nullptr ? row->name.c_str() : "-");
}

// Packet cc13-releasegun: 0068B470, reached from 0068C6B1 on the C1h edge
// (IC_MAP_TOGGLE in this installation's Inputs.lua).
void GameHudHost::Impl::back_out_one_overlay_level_0068b470() {
    // 0068B4A8..0068B4C6: the level-3 collapse runs only with 00E18D1C non-empty; this
    // host never fills level 3 (level3_screen_count answers 0).
    record("InGameInterface::binoculars_reset", 0x0051e8e0u);   // 0068B4CE
    if (release_gun_map_up) {
        // 0068B4E9..0068B507: 004F85D0(0), 004D8AE0(game, 0), then 0068AA40
        // for the controlled vehicle. Not hosted: the line only opens the map.
        record("InGameInterface::back_out_close_level2", 0x0068b4e9u);
        log.note("releasegun: C1h with the map up would close it (0068B4E9); not hosted");
        log_map_state("close-requested (not hosted, map stays open)");
        return;
    }
    // 0068AC30. 00E08880 is EnableMap, 1 in single player
    // (docs/MISSION_LOBBY_SETTINGS.md), so the sound-queue arm is not taken.
    // 0068AC50: 0068AB30 pushes 20h only on interface 31h/32h; the captain's
    // is 25h.
    record("InGameInterface::leave_unit_request_level", 0x0068ab30u);
    // 0068AC63 / 0068AC73: HUD root +1Ch to +ECh, [00E188D8] to +F0h. The HUD
    // root object is not this host's.
    record("InGameInterface::hud_root_copy_selection", 0x0068ac63u);
    // 0068AC79 / 0068AC8E, both pushed right to left and zero-terminated.
    static const int kLevel2Screens[] = {0x4C, 0x39, 0x44, 0x29, 0x49};
    static const int kLevel2Contexts[] = {0x0E, 0x14, 0x0C, 0x11};
    menu.publish_level2_screen_set_004f85d0(kLevel2Screens, 5);
    done("InGameInterface::set_level2_screen_set", 0x004f85d0u);
    menu.publish_level2_input_contexts_004d8ae0(kLevel2Contexts, 4);
    done("InGameInterface::set_level2_input_contexts", 0x004d8ae0u);
    release_gun_map_up = true;
    // Packet cc13-helmmapgate: 46h, whose update 0064D610 runs 0064B870, is now
    // occluded. The units host counts the helm calls this skips, and skips them
    // only under kHelmPrepareMapUpGateCc13Bound.
    if (units != nullptr) units->set_helm_screen_occluded_by_map(true);
    done("InGameInterface::restore_hud_screen_set", 0x0068ac30u);
    log_map_state("open");
    log.note("releasegun: C1h opened the map: level 2 {4Ch 39h 44h 29h 49h}, contexts "
        "{0Eh 14h 0Ch 11h}; 4Ch's +8h (005A8EF0) occludes level 1");
}

// 0064A490, screen 46h's +1Ch (vtable 00CF7978): 005470C0 with ECX =
// [[00E198C4]+50h], then, with +1Ch (the unit 0064DA40 stored) set,
// 0077C470(+1Ch, 1FEh, 0), session message 4Bh with take 0.
void GameHudHost::exit_ship_view_screen_0064a490() {
    Impl& impl = *impl_;
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(impl);
        bsp::weapon_group_screen_release_005470c0(impl.weapon_group, group_host);
        impl.done("HudWeaponGroupScreen::release", 0x005470c0u);
    } else {
        impl.record("HudWeaponGroupScreen::release", 0x005470c0u);
    }
    const bool roles = impl.ship_view_has_unit && impl.units != nullptr
        && impl.ship_view_unit < impl.units->count();
    if (roles) {
        impl.units->role_request_0077c470(impl.ship_view_unit, 0x1FEu, false);
        impl.done("HudShipView::release_roles", 0x0077c470u);
    }
    impl.done("HudShipView::exit", 0x0064a490u);
    impl.log.notef("releasegun: screen 46h exit 0064A490 released the weapon group%s",
        roles ? " and roles 1..8 (1FEh)" : "; no ship-view unit");
}

void GameHudHost::update_map_screen_005b45c0() {
    Impl& impl = *impl_;
    // 005B4673..005B46B3, every update while the map is up: [00E188D8]'s
    // vtable[13Ch]() unit, its role-1 holder +1B0h (+1ACh[1]); not 8 (005B468D)
    // and not AI-held by 00927F10 (005B4695) sends 0077C470(unit, 2, 0).
    // Stand-ins: "not AI-held" is holder == local slot 0, the units host's
    // labelled 00927F10 substitution (game_hosts_units.cpp:13466); vtable[13Ch]()
    // returning the unit itself is checked for a ship only, and the controlled
    // index (read live on each call) stands for it.
    std::int32_t holder = 8;
    const bool release = impl.units != nullptr && impl.units->controlled_bound()
        && impl.units->unit_current_role_slot(impl.units->controlled_index(), 1, holder)
        && holder != 8 && holder == 0;
    if (release) {
        impl.units->role_request_0077c470(impl.units->controlled_index(), 0x2u, false);
        impl.done("HudMapScreen::release_helm", 0x0077c470u);
    }
    // The rest of 005B45C0 (005B45E6..005B466E, 005B46B8.. the map view) is not hosted.
    impl.record("HudMapScreen::update", 0x005b45c0u);
}

void GameHudHost::attach_world_2k(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& impl = *impl_;
    impl.units = &units;
    impl.lua = &lua;
    if (!impl.minimap) impl.minimap = std::make_unique<GameHudMinimapHost>(impl.log, impl.menu);
    if (!impl.markers) impl.markers = std::make_unique<GameHudMarkersHost>(impl.log, impl.menu);
    impl.minimap->attach_world(units, lua);
    impl.markers->attach_world(units);
    attached_hud_for_selection() = this;
}

void GameHudHost::detach_world_2k() noexcept {
    Impl& impl = *impl_;
    if (attached_hud_for_selection() == this) attached_hud_for_selection() = nullptr;
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
    impl.weapon_group = bsp::HudWeaponGroupScreenState{};
    impl.ship_view_has_unit = false;
    impl.ship_view_controls = bsp::IntegratedControlsState{};
    impl.binoculars = bsp::BinocularsState{};
    bsp::clear_mission_camera();
    impl.unit_request_pending = false;
    impl.unit_request_applied = false;
    impl.unit_interface_id = 0;
    impl.movie_screen = bsp::HudMovieScreenState{};
    impl.pending_interface_id = 0;
    impl.movie = bsp::HudMovieCamera{};
    impl.movie_active = false;
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

// 00645060's inputs for one unit, teamIndex [game+18ECh] = 0 (single player)
// and allowSpectate 1 (00647313 PUSH 1).
bsp::UnitSelectableInputs GameHudHost::Impl::selectable_inputs_00645060(std::size_t unit) {
    bsp::UnitSelectableInputs in{};
    // 00645091..006450B3: +5Ch set, +5Dh/+60h/+5Eh clear. The units host
    // answers the four together (its 0043F080 test, the same four bytes).
    const bool alive = units->unit_alive_and_visible(unit)
        || bsp::plane_squadron_live_fused_slot(unit) != nullptr;   // CC13 item A, OFF
    in.alive_5c = alive;
    // 006450B9..006450BF: [unit+54h] == [[game+18CCh]+28h]. SUBSTITUTION
    // (labelled, as game_hosts_ai.cpp does for the same read): the local
    // player's side record is party 0, and unit+54h is the authored party.
    in.team_matches_owner =
        units->unit_side_0054(unit) == bsp::game::local_player_party();  // section 125
    // Packet cc9_squadron_slot_class: the unit's own class test, which for a
    // squadron's fused slot is PlaneSquadronGen's (18h) when bound.
    in.is_kind_2 = selection_is_kind_of(unit, bsp::kUnitTraitSelectableBase);
    in.is_kind_0f = selection_is_kind_of(unit, 0x0f);
    in.is_kind_2a = selection_is_kind_of(unit, bsp::kUnitTraitRejectedA);
    in.is_kind_46 = selection_is_kind_of(unit, bsp::kUnitTraitRejectedB);
    in.is_kind_45 = selection_is_kind_of(unit, bsp::kUnitTraitRejectedC);
    if (is_squadron_slot(unit)) {
        bool seen = false;
        for (const std::size_t u : squadron_slots_seen) seen = seen || u == unit;
        if (!seen) squadron_slots_seen.push_back(unit);
    }
    // 00645110 vtable[124h]() (packet cc9_vtable124_liveness).
    in.vtable_124_allows = unit_vtable_124(unit, 0x00645110u);
    // 00645121 00927C50(unit, 0): any role word 9 or 0.
    for (int role = 0; role < bsp::kUnitRoleTableEntries; ++role) {
        std::int32_t word = 0;
        if (!units->unit_role_permission(unit, role, word)) continue;
        if (role == 0) in.spectate_kind = word;          // [unit+188h]
        if (word == 9 || word == 0) in.team_query_00927c50 = true;
    }
    // 0064507A: in single player game+2194h is clear, so BL = 0 and the
    // spectator door is shut. SUBSTITUTION (labelled): +2194h is not modelled;
    // [team+19h] is unread and left false.
    in.spectate_allowed = false;
    in.team_is_local = false;
    return in;
}

// Packet cc9_squadron_slot_class. A slot is a squadron's when a registry
// record names it as its squadron_unit (the fused wing 0).
bool GameHudHost::Impl::is_squadron_slot(std::size_t unit) const {
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit) return true;
    }
    return false;
}

int GameHudHost::Impl::selection_class_id(std::size_t unit) const {
    if constexpr (kSquadronSlotClassBound) {
        if (is_squadron_slot(unit)) return 0x18;         // PlaneSquadronGen
    }
    return units->unit_class_id(unit);
}

bool GameHudHost::Impl::selection_is_kind_of(std::size_t unit, int class_id) const {
    if (unit >= units->count()) return false;
    return bsp::unit_is_kind_of(selection_class_id(unit), class_id);
}

// unit->vtable[124h]() by class (include/bsp/game_hosts_hud.hpp,
// kUnitVtable124MapBound). The four bytes are 0043F080's, which the units
// host answers as unit_alive_and_visible.
bool GameHudHost::Impl::unit_vtable_124(std::size_t unit, std::uint32_t site) {
    ++vtable124_asks;
    const bool alive = units->unit_alive_and_visible(unit);
    // The rule before this packet: the ship vtable only.
    const bool ship_rule = units->unit_is_kind_of(unit, 6) && alive;
    bool map = false;
    if (selection_is_kind_of(unit, 0x1c)) {
        // 006F5920 MCommandBuilding: +790h is clear in single player (its one
        // setter 006F292D needs effective game mode 2 or 3, and 004BCA50
        // reports 8), so the +70h == 58h arm is not reached: the four bytes.
        map = alive;
    } else if (selection_is_kind_of(unit, 0x1b)) {
        map = false;                                   // 00745A50 MLandFort
    } else if (selection_is_kind_of(unit, 5)) {
        map = alive;                                   // 006D1EF0
    } else if (selection_is_kind_of(unit, 0x18)) {
        // 007EE670 PlaneSquadronGen. +3B0h is only ever cleared (007F2DD3,
        // 007EFB69), so it reads 0. SUBSTITUTION (labelled): +361h, set by
        // 007F31A0 when the leader leaves the map with survivors, is not
        // modelled by this host and reads clear. Then the four bytes of the
        // slot-0 plane +3D0h (007EE685), false when there is none.
        const bsp::PlaneSquadronHostRecord* squadron = nullptr;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == unit) { squadron = &r; break; }
        }
        const std::size_t leader = squadron != nullptr ? squadron->flight_leader()
                                                       : bsp::kPlaneSquadronNoUnit;
        map = leader != bsp::kPlaneSquadronNoUnit && leader < units->count()
            && units->unit_alive_and_visible(leader);
    } else {
        map = false;                                   // 00927800, the class-02 base
    }
    if (map && !ship_rule) {
        ++vtable124_map_only_asks;
        bool seen = false;
        for (const auto& entry : vtable124_map_only) {
            if (entry.first == unit) { seen = true; break; }
        }
        if (!seen) vtable124_map_only.emplace_back(unit, site);
    }
    if constexpr (kUnitVtable124MapBound) {
        done("Unit::vtable_124", site);
        return map;
    } else {
        if (!units->unit_is_kind_of(unit, 6)) {
            record("SetSelectedUnit::unit_vtable_124", 0x00645110u);
        }
        return ship_rule;
    }
}

namespace {
// 00645600's host over the units host. The unit-side broadcasts and the
// observer pair are records: no host state carries them.
class SelectControlledUnitBinding final : public bsp::SelectControlledUnitHost {
public:
    SelectControlledUnitBinding(GameHudHost::Impl& owner, std::size_t candidate, bool selectable)
        : owner_(owner), candidate_(candidate), selectable_(selectable) {}
    bool current_is_kind_6() override {
        return owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 6);
    }
    void release_unit_parts() override {
        owner_.record("SetControlledUnit::release_unit_parts", 0x00817380u);
    }
    void release_unit_nodes() override {
        // 0064562D 0080E290(current). The caller (00645603..0064562D) reaches here only
        // with a bound kind-6 controlled unit, which is still the outgoing one: the
        // setter (004C0890) runs after. kHelmReleaseGunsToRestCc13Bound.
        GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
        if (bsp::kHelmReleaseGunsToRestCc13Bound && gunnery != nullptr) {
            gunnery->aim_unit_turning_guns_to_rest_0080e290(owner_.units->controlled_index());
            owner_.done("SetControlledUnit::release_unit_nodes", 0x0080e290u);
        } else {
            owner_.record("SetControlledUnit::release_unit_nodes", 0x0080e290u);
        }
    }
    void clear_hud_slot() override {
        owner_.record("SetControlledUnit::clear_hud_root_1c", 0x00645637u);
    }
    bool candidate_is_selectable() override { return selectable_; }
    bool candidate_is_kind_1() override {
        return owner_.selection_is_kind_of(candidate_, 1);   // 0064565A
    }
    bool candidate_vtable_124() override {
        // 0064565F (packet cc9_vtable124_liveness).
        return owner_.unit_vtable_124(candidate_, 0x0064565fu);
    }
    void unregister_observer() override {
        if constexpr (kControlledUnitObserverBound) {
            // 00645699 006952A0(controlled, root+8h).
            owner_.observed_unit = SIZE_MAX;
            owner_.done("SetControlledUnit::unregister_observer", 0x006952a0u);
        } else {
            owner_.record("SetControlledUnit::unregister_observer", 0x006952a0u);
        }
    }
    void register_observer() override {
        if constexpr (kControlledUnitObserverBound) {
            // 006456D3 00694A60([00E188D8], root+8h), after 004C0890 bound it.
            owner_.observed_unit = owner_.units->controlled_bound()
                ? owner_.units->controlled_index() : SIZE_MAX;
            owner_.done("SetControlledUnit::register_observer", 0x00694a60u);
        } else {
            owner_.record("SetControlledUnit::register_observer", 0x00694a60u);
        }
    }
    void set_unit_controlled_audio(bool) override {
        owner_.record("SetControlledUnit::controlled_audio", 0x00954990u);
    }
    void set_controlled_unit(bool candidate_present) override {
        if (!candidate_present) {
            // 004C0890(null): not reached from 00647300, whose 00645060 already
            // accepted the same unit with the same arguments.
            owner_.record("SetControlledUnit::clear_controlled_unit", 0x004c0890u);
            return;
        }
        owner_.units->set_controlled_unit_004c0890(candidate_);
    }
    bool controlled_unit_present() override { return owner_.units->controlled_bound(); }
    bool controlled_is_candidate() override {
        return owner_.units->controlled_bound()
            && owner_.units->controlled_index() == candidate_;
    }
    bool controlled_is_kind_5() override {
        return owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 5);
    }

private:
    GameHudHost::Impl& owner_;
    std::size_t candidate_;
    bool selectable_;
};
}  // namespace

namespace {
// 005CD1A0's callees over this host. Switch 1 binds the 2Ch push; the reseed
// (switch 2) and the camera (switch 3) stay records here.
class MovieInterfaceBinding final : public bsp::MovieInterfaceHost {
public:
    explicit MovieInterfaceBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    void seed_random_stream(int stream, std::uint32_t seed) override {
        // 005CD1B0 / 005CD1BC.
        owner_.reseed_stream(stream, seed, 0x005cd1b0u);
    }
    void push_interface_request(int interface_id, const void*) override {
        // 005CD1CE 004CC460(2Ch, [00E188D8]); the pending record is replaced.
        owner_.pending_interface_id = interface_id;
        ++owner_.movie_pushes;
        owner_.done("MovieInterface::push_interface_2ch", 0x005cd1ceu);
    }
    void set_input_context_level(int, int) override {
        // 005CD1F6, only behind the multiplayer lock gate: not reached here.
        owner_.record("MovieInterface::input_context_level", 0x00a933f0u);
    }
    std::uint32_t ensure_movie_camera(std::uint32_t existing) override {
        if constexpr (kMovieMoverBound) {
            // 005CC170: only while +1Ch is null (005CC192).
            if (existing != 0) return existing;
            return owner_.build_movie_camera_005cc170() ? 1u : 0u;
        } else {
            owner_.record("MovieInterface::ensure_movie_camera", 0x005cc170u);
            return existing;
        }
    }
    void send_unit_session_message(int) override {
        // 005CD217..005CD224 0077C470([00E188D8], 1FFh, 0): release roles 0..8 of
        // the unit just tested at 005CD20E. Packet cc13-relgaps gap 1. The unit is
        // the controlled index read live here, the same read the caller's 00927F30
        // test made this call; the engage runs once per movie-screen instance.
        if constexpr (kMovieEngageRoleReleaseCc13Bound) {
            if (owner_.units != nullptr && owner_.units->controlled_bound()) {
                owner_.units->role_request_0077c470(owner_.units->controlled_index(), 0x1FFu, false);
                owner_.done("MovieInterface::session_message_1ff", 0x0077c470u);
            } else {
                owner_.record("MovieInterface::session_message_1ff", 0x0077c470u);
            }
        } else {
            owner_.record("MovieInterface::session_message_1ff", 0x0077c470u);
        }
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::movie_screen_camera_005cd240() {
    Impl& impl = *impl_;
    ++impl.movie_calls;
    const bool first = !impl.movie_screen.engaged;
    bsp::MovieInterfaceEngageInputs inputs{};
    inputs.game_present = true;              // 005CD1D3
    inputs.local_view_mode_active = false;   // game+1FE4h, single player
    inputs.player_unit_present = impl.units != nullptr && impl.units->controlled_bound();
    if (inputs.player_unit_present) {
        // 005CD20E 00927F30(unit, 0): the unit's role 0 is held by slot 0.
        std::int32_t holder = -1;
        inputs.unit_is_local_player_role =
            impl.units->unit_current_role_slot(impl.units->controlled_index(), 0, holder)
            && holder == 0;
    }
    MovieInterfaceBinding binding(impl);
    bsp::movie_screen_camera_005cd240(impl.movie_screen, impl.movie_lock, inputs, nullptr,
        binding);
    impl.done("MovieScreen::get_camera_005cd240", 0x005cd240u);
    if (first) {
        ++impl.movie_engages;
        impl.log.notef("movie interface 005CD1A0 engaged on the first MovCamNew call (hud update "
            "frame %llu): 004CC460(2Ch, controlled unit) pushed; the next 006840f0 applies "
            "{37h}", impl.summary.update_frames);
    }
}

namespace {
// The keyframes' parent queries over the units host.
class MovieParentBinding final : public bsp::MovieCameraParentHost {
public:
    explicit MovieParentBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool destroyed_5e(std::size_t unit) override {
        if constexpr (kMovieParentKilledByteBound) {
            // Switch 5 (packet cc9_movie_camera_keys): 00799D70's CMP byte
            // [entity+5Eh], 0 alone, from the units host's scene-node flags.
            // A unit the host cannot resolve keeps the OFF answer (no parent).
            bsp::SceneNodeFlags flags;
            if (owner_.units != nullptr && owner_.units->unit_scene_node_flags(unit, flags)) {
                return flags.destroyed;
            }
            return true;
        } else {
            // SUBSTITUTION: 00799D70 reads +5Eh alone; the units host answers the
            // four bytes of 0043F080 together.
            return owner_.units == nullptr || !owner_.units->unit_alive_and_visible(unit);
        }
    }
    bool is_kind(std::size_t unit, int kind) override {
        return owner_.units != nullptr && owner_.units->unit_is_kind_of(unit, kind);
    }
    std::size_t squadron_leader_3d0(std::size_t unit) override {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != unit) continue;
            for (const std::size_t u : r.member_units) {
                if (u != bsp::kPlaneSquadronNoUnit) return u;
            }
        }
        unsupported("MovieKeyframe::squadron_leader", 0x00799d9bu);
        return bsp::kMovieNoParent;
    }
    bool world(std::size_t unit, std::array<float, 16>& m) override {
        float right[3], up[3], forward[3], translation[3];
        if (owner_.units == nullptr
            || !owner_.units->unit_pose(unit, right, up, forward, translation)) {
            m = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            return false;
        }
        m = {right[0], right[1], right[2], 0.0f, up[0], up[1], up[2], 0.0f,
            forward[0], forward[1], forward[2], 0.0f,
            translation[0], translation[1], translation[2], 1.0f};
        return true;
    }
    float class_height_a8(std::size_t unit) override {
        // [[unit+538h]+A8h] is the class `Height` (docs/GUN_MOUNT_POSITIONS.md),
        // read from the installed VehicleClass row.
        const GameUnitRow* row = owner_.units != nullptr ? owner_.units->unit_row(unit) : nullptr;
        if (row == nullptr || owner_.lua == nullptr) return 0.0f;
        return owner_.lua->read_vehicle_class_number(row->type_id, "Height", 0.0f);
    }
    bool ground_height_00903860(const std::array<float, 3>& point, float& out) override {
        // Switch 6 (packet cc9_movie_camera_keys): the scene-contents host's
        // Landscape query, the same 00903860 the image calls at 00795B62.
        return world_ground_height_00903860(point.data(), out);
    }
    std::array<float, 3> plane_velocity_810(std::size_t) override {
        // SUBSTITUTION: plane+810h..818h has no traced writer
        // (docs/PLANE_ADVANCE_POSE.md); zero.
        return {0.0f, 0.0f, 0.0f};
    }
    void unsupported(const char* what, std::uint32_t address) override {
        ++owner_.movie_unsupported;
        owner_.record(what, address);
    }
    void run_script_00887e50(const std::string& function) override {
        // Packet cc9_lua49_movie_modifier: a fired keyframe event's `function`
        // through the script-orders host's 00887E50 (`_G[name]()`, no arguments).
        // LABELLED: that entry point also counts the call among its blackout
        // callbacks.
        GameScriptOrdersHost* orders = owner_.lua != nullptr ? owner_.lua->script_orders() : nullptr;
        owner_.log.notef("movie keyframe event fires \"%s\" at hud update frame %llu (00791540 -> "
            "00887E50)", function.c_str(), owner_.summary.update_frames);
        if (orders == nullptr) {
            unsupported("MovieKeyframeEvent::run_script", 0x00887e50u);
            return;
        }
        static_cast<bsp::MissionBlackoutHost&>(*orders).mission_lua_call_named_00887e50(function);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::Impl::reseed_stream(int stream, std::uint32_t seed, std::uint32_t site) {
    // Switch 2: stream 1 reaches the gunnery host's shared stand-in (a
    // labelled substitution there); stream 0 has no host generator.
    GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
    if (kMovieReseedBound && gunnery != nullptr
        && gunnery->reseed_shared_stream_00bd2fd0(stream, seed)) {
        ++movie_reseeds;
        done(site == 0x005cd1b0u ? "MovieInterface::seed_random_stream_1"
                                 : "MovieCamera::seed_random_stream_1", site);
        return;
    }
    record(site == 0x005cd1b0u ? "MovieInterface::seed_random_stream"
                               : "MovieCamera::seed_random_stream", 0x00bd2fd0u);
}

bool GameHudHost::Impl::build_movie_camera_005cc170() {
    // 005CC19F operator new(570h), 0079D020, then its 00BD2FD0(1, 123).
    bsp::movie_camera_construct_0079d020(movie);
    reseed_stream(1, 123u, 0x0079d1abu);
    record("MovieCamera::place_in_world", 0x00923870u);
    // 005CC27C..005CC29A: 00B6DB70 on the node, then 007A0860 with its world.
    std::array<float, 16> seed{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
    if (node.ready) {
        for (std::size_t i = 0; i < 16; ++i) seed[i] = node.state.transform.world[i];
    }
    bsp::movie_camera_seed_007a0860(movie, seed);
    // 005CC2A9 004BC410: the outgoing mover is destroyed (00926D90).
    const bool replaced = camera_bound;
    camera_bound = false;
    movie_active = true;
    ++movie_builds;
    record("MovieCamera::observer_pair", 0x00694a60u);
    done("MovieCamera::build_005cc170", 0x005cc170u);
    log.notef("movie camera 005CC170 built and installed as the mover (004BC410 at 005CC2A9, "
        "hud update frame %llu); it replaced %s; seed position (%.1f,%.1f,%.1f)",
        summary.update_frames, replaced ? "the ShipCaptain, destroyed" : "no mover",
        static_cast<double>(seed[12]), static_cast<double>(seed[13]),
        static_cast<double>(seed[14]));
    return true;
}

void GameHudHost::Impl::destroy_movie_camera(const char* why) {
    movie_active = false;
    movie_screen.camera = 0;                            // the 00694A60 observer
    ++movie_destroys;
    // 0079A2FB, the destructor: 00BD2FD0(1, wall-clock milliseconds). Not
    // reproducible (docs/RANDOM_STREAMS.md), so a record.
    record("MovieCamera::destructor_reseed_wall_clock", 0x0079a2fbu);
    log.notef("movie camera destroyed at hud update frame %llu: %s (clock %.2f s, %zu camera "
        "and %zu look-at keyframes)", summary.update_frames, why,
        static_cast<double>(movie.clock_3b0), movie.camera_498.keys.size(),
        movie.target_414.keys.size());
    movie = bsp::HudMovieCamera{};
}

void GameHudHost::Impl::step_movie_camera(float seconds) {
    // SUBSTITUTION: the tick element's wave-3 slot 00798C80 runs once per
    // fixed step; this host runs one 0.05 s fixed step per frame, so it is
    // called once per frame here, before the frame's 0079A3B0.
    const bool latched_before = movie.running_391;   // 00798C8A CMP byte [ESI+221h]
    if (bsp::movie_camera_fixed_step_00798c80(movie, 0.05f)) {
        reseed_stream(1, 12345u, 0x00798cadu);          // 00798CAD
        reseed_stream(0, 54321u, 0x00798cb9u);          // 00798CB9
    }
    if constexpr (kMovieStepDrawBound) {
        // Switch 4 (packet cc9_movie_camera_keys): the else arm runs on every
        // step after the latch step. 00798D07 is 00BD2F10(ECX = 1, 0.0,
        // [00D046A8] = 65535.0). SUBSTITUTION, labelled: the draw goes through
        // the gunnery host's death-mode entry point with stream 1, which is the
        // shared stand-in generator on the default path. Under
        // BSP_GUNNERY_RNG_STREAMS=1 its key is (death_mode, FFFFFFh), an index
        // no unit has, so no other draw reads it. A dedicated entry point waits
        // on src/game_hosts_gunnery.cpp's lease.
        GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
        if (latched_before && gunnery != nullptr) {
            const float draw = gunnery->death_mode_draw_00bd2f10(1, 0xFFFFFFu, 0.0f, 65535.0f);
            bsp::movie_camera_store_step_draw_00798cd9(movie, 0.05f, draw);
            ++movie_step_draws;
            done("MovieCamera::fixed_step_draw", 0x00798d07u);
        } else if (latched_before) {
            record("MovieCamera::fixed_step_draw", 0x00798d07u);
        }
    } else {
        // 00798D07: one stream-1 draw per step into a 16-entry ring. SUBSTITUTION:
        // not taken (the ring feeds nothing this host reads).
        record("MovieCamera::fixed_step_draw", 0x00798d07u);
    }
    MovieParentBinding parents(*this);
    if constexpr (bsp::kMovieKeyframeModifierBound) {
        // 00798D38..00798D59, the step's tail: 00797ED0 over both tracks. An
        // event's script may destroy this camera; the update below then finds
        // empty tracks.
        movie_events_fired += static_cast<unsigned long long>(
            bsp::movie_camera_key_ticks_00797ed0(movie, 0.05f, parents));
        done("MovieCamera::key_ticks_00797ed0", 0x00797ed0u);
        if (!movie_active) return;
    }
    std::array<float, 16> world{};
    float fov = 0.0f;
    if (!bsp::movie_camera_update_0079a3b0(movie, parents, seconds, world, fov)) {
        record("MovieCamera::update_empty_track", 0x0079bbd5u);
        return;
    }
    done("MovieCamera::update", 0x0079a3b0u);
    bsp::CameraMatrix16 matrix{};
    for (std::size_t i = 0; i < 16; ++i) matrix[i] = world[i];
    bsp::MissionCameraProjection projection = camera_projection;
    projection.fov = fov;                                // 0079B833 00B6FBB0
    bsp::publish_mission_camera(matrix, projection);
    ++movie_publishes;
    if (movie_publishes == 1 || summary.update_frames >= movie_last_pose_log + 40) {
        movie_last_pose_log = summary.update_frames;
        log.notef("movie camera pose at hud update frame %llu: clock %.2f s, position "
            "(%.1f,%.1f,%.1f), look-at (%.1f,%.1f,%.1f), forward (%.4f,%.4f,%.4f), fov %.4f",
            summary.update_frames, static_cast<double>(movie.clock_3b0),
            static_cast<double>(world[12]), static_cast<double>(world[13]),
            static_cast<double>(world[14]), static_cast<double>(movie.look_384[0]),
            static_cast<double>(movie.look_384[1]), static_cast<double>(movie.look_384[2]),
            static_cast<double>(world[8]), static_cast<double>(world[9]),
            static_cast<double>(world[10]), static_cast<double>(fov));
    }
}

int GameHudHost::movie_add_position_007a44d0(const bsp::MovieKeyframeInput& in) {
    Impl& impl = *impl_;
    // 008B7AE6: the camera 005CD240 returned; null skips the store.
    if (!impl.movie_active || impl.movie_screen.camera == 0) {
        impl.record("MovieCamera::add_position_no_camera", 0x008b7ae8u);
        return 0;
    }
    MovieParentBinding parents(impl);
    const int added = bsp::movie_camera_add_position_007a42c0(impl.movie, in, parents);
    impl.movie_keyframes += static_cast<unsigned long long>(added);
    impl.done("MovieCamera::add_position_007a44d0", 0x007a44d0u);
    return added;
}

int hud_movie_add_position_007a44d0(const bsp::MovieKeyframeInput& in) {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return -1;
    return hud->movie_add_position_007a44d0(in);
}

void GameHudHost::update_movie_screen_005cbaf0(float seconds) {
    Impl& impl = *impl_;
    impl.record("MovieScreen::black_bars_update", 0x005cbaf0u);
    impl.step_mission_camera(seconds);
}

bool hud_movie_screen_camera_005cd240() {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return false;
    hud->movie_screen_camera_005cd240();
    return true;
}

bool hud_unit_selectable_00645060(std::size_t unit, bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr) return false;
    return hud->unit_selectable_00645060(unit);
}

bool GameHudHost::unit_selectable_00645060(std::size_t unit) {
    Impl& impl = *impl_;
    if (impl.units == nullptr || unit >= impl.units->count()) return false;   // 00645082
    impl.done("HelmOrder::selectable_00645060", 0x00645060u);
    return bsp::unit_is_selectable_00645060(impl.selectable_inputs_00645060(unit));
}

bool GameHudHost::set_selected_unit_00647300(std::size_t unit) {
    Impl& impl = *impl_;
    ++impl.select_calls;
    if (impl.units == nullptr || unit >= impl.units->count()) {
        // 00645082: a null unit is rejected by 00645060.
        impl.log.notef("SetSelectedUnit 00647300: unit index %zu has no created unit; "
            "00645060 rejects a null unit", unit);
        return false;
    }
    const GameUnitRow* row = impl.units->unit_row(unit);
    const std::string name = row != nullptr ? row->name : std::string("?");
    const bsp::UnitSelectableInputs in = impl.selectable_inputs_00645060(unit);
    const bool accepted = bsp::unit_is_selectable_00645060(in);        // 00647317
    impl.done("SetSelectedUnit::selectable_00645060", 0x00645060u);
    if (!accepted) {                                                  // 0064731E
        impl.log.notef("SetSelectedUnit 00647300: \"%s\" rejected by 00645060 (alive=%d "
            "party0=%d kind2=%d kind0F=%d kind2A=%d kind46=%d kind45=%d vt124=%d role=%d)",
            name.c_str(), in.alive_5c ? 1 : 0, in.team_matches_owner ? 1 : 0,
            in.is_kind_2 ? 1 : 0, in.is_kind_0f ? 1 : 0, in.is_kind_2a ? 1 : 0,
            in.is_kind_46 ? 1 : 0, in.is_kind_45 ? 1 : 0, in.vtable_124_allows ? 1 : 0,
            in.team_query_00927c50 ? 1 : 0);
        return false;
    }
    ++impl.select_accepted;
    const std::string before = impl.units->controlled_bound()
        ? impl.units->unit_row(impl.units->controlled_index())->name : std::string("none");
    SelectControlledUnitBinding binding(impl, unit, accepted);
    const bsp::SelectControlledUnitResult result =
        bsp::select_controlled_unit_00645600(true, binding);          // 00647323
    impl.done("SetSelectedUnit::set_controlled_unit_00645600", 0x00645600u);
    if (!impl.units->controlled_bound()) {                            // 00647328
        // 00647331..0064734B: the manager idle, push 34h INTF_LIMBO. Not
        // reachable after an accepted unit (the setter bound it).
        impl.record("SetSelectedUnit::push_limbo_34h", 0x0064734bu);
        return true;
    }
    // 00647040: 0059DA80 on [manager+54h] is a bare RET; 005251C0 on
    // [manager+CCh] (a screen reset: +C0h, its vector, +D4h = -1 and two input
    // context levels) is unread here and recorded.
    impl.record("SetSelectedUnit::screen_reset_005251c0", 0x005251c0u);
    // 0064705E..00647077: 004CC460(20h, [00E188D8]->vtable[140h]()). For a
    // ship the slot is 0047F320, `MOV EAX,ECX`: the unit itself, which is the
    // payload the host's classifier already reads (the controlled unit).
    impl.unit_request_pending = true;
    impl.unit_request_applied = false;
    impl.pending_interface_id = 0;   // 004CC460 overwrites the one pending record
    ++impl.select_pushes;
    impl.done("SetSelectedUnit::push_interface_20h", 0x00647077u);
    impl.log.notef("SetSelectedUnit 00647300: \"%s\" accepted by 00645060; 00645600 moved "
        "00e188d8 from \"%s\" to \"%s\" (released_previous=%d refreshed_in_place=%d); "
        "00647040 pushed interface 20h with the unit, serviced by the next 006840f0",
        name.c_str(), before.c_str(), name.c_str(), result.released_previous ? 1 : 0,
        result.refreshed_in_place ? 1 : 0);
    return true;
}

namespace {
// Packet cc9_force_select_unit: bsp::HudRootUnitListsHost over the units host.
// Handles are unit index + 1; 0 is null.
class HudRootListsBinding final : public bsp::HudRootUnitListsHost {
public:
    explicit HudRootListsBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::uint32_t controlled_unit() override {
        return owner_.units->controlled_bound()
            ? static_cast<std::uint32_t>(owner_.units->controlled_index() + 1) : 0u;
    }
    const std::vector<std::uint32_t>& local_unit_list_1970() override {
        // game+1974h is the head of game+1970h, which the world host's last
        // 004C3CB0 body built (walk 0 minus ordnance, UnitRef = index + 1).
        list_.clear();
        const bsp::LocalPlayerUnitLists* lists = game_local_player_unit_lists();
        if (lists == nullptr) {
            owner_.record("HudRootLists::list_1970", 0x00648369u);
            return list_;
        }
        for (const bsp::UnitRef ref : (*lists)[static_cast<std::size_t>(
                 bsp::LocalPlayerUnitList::kWalk0Rest)]) {
            list_.push_back(ref);
        }
        return list_;
    }
    bool unit_selectable_00645060(std::uint32_t unit) override {
        if (unit == 0 || unit > owner_.units->count()) return false;
        return bsp::unit_is_selectable_00645060(owner_.selectable_inputs_00645060(unit - 1));
    }
    bool unit_is_group_leader_00778890(std::uint32_t unit) override {
        const std::int32_t group = group_of(unit);
        return group >= 0 && owner_.units->formation_leader_0014(group) == unit - 1;
    }
    bool unit_is_group_member_007788b0(std::uint32_t unit) override {
        return unit != 0 && owner_.units->unit_is_formation_follower_007788b0(unit - 1);
    }
    std::uint32_t group_leader_007788d0(std::uint32_t unit) override {
        const std::int32_t group = group_of(unit);
        if (group < 0) return 0;
        const std::size_t leader = owner_.units->formation_leader_0014(group);
        return leader == SIZE_MAX ? 0u : static_cast<std::uint32_t>(leader + 1);
    }
    bool role_word_188_open(std::uint32_t unit) override {
        // [unit+188h] == 9 or == [game+18ECh]. SUBSTITUTION (labelled, as
        // selectable_inputs_00645060 takes it): the local player index is 0.
        std::int32_t word = 0;
        if (unit == 0 || !owner_.units->unit_role_permission(unit - 1, 0, word)) return false;
        return word == 9 || word == 0;
    }
    int group_member_count_4f8(std::uint32_t leader) override {
        const std::int32_t group = group_of(leader);
        return group < 0 ? 0 : owner_.units->formation_member_count(group);
    }
    std::uint32_t group_member_at_0070d060(std::uint32_t leader, int slot) override {
        const std::int32_t group = group_of(leader);
        if (group < 0) return 0;
        const std::size_t member = owner_.units->formation_member_unit(group, slot);
        return member == SIZE_MAX ? 0u : static_cast<std::uint32_t>(member + 1);
    }
    void set_controlled_unit_00645600(std::uint32_t unit) override {
        const std::size_t index = unit != 0 ? unit - 1 : 0;
        const bool selectable = unit != 0 && unit_selectable_00645060(unit);
        SelectControlledUnitBinding binding(owner_, index, selectable);
        bsp::select_controlled_unit_00645600(unit != 0, binding);
        owner_.done("ForceSelectUnit::set_controlled_unit_00645600", 0x00645600u);
    }
    bool multiplayer_1fe4() override { return false; }
    // SUBSTITUTION (labelled): the manager's +4h/+20h/+1Ch/+38h are not read
    // here. They matter only on an empty +8Ch vector, where the answers
    // (applied 0, pending -1) take neither push; that arm is a record.
    int manager_applied_04() override {
        owner_.record("ForceSelectUnit::empty_list_manager_state", 0x006485d7u);
        return 0;
    }
    int manager_pending_20() override { return -1; }
    bool manager_1c_equals_38() override { return false; }
    void push_interface_request_004cc460(int) override {
        owner_.record("ForceSelectUnit::push_limbo", 0x004cc460u);
    }
    void push_controlled_unit_interface_00647040() override {
        // The same 00647040 as SetSelectedUnit's tail: 005251C0 is recorded,
        // then 004CC460(20h, the unit) re-arms the pending/applied pair.
        owner_.record("ForceSelectUnit::screen_reset_005251c0", 0x005251c0u);
        owner_.unit_request_pending = true;
        owner_.unit_request_applied = false;
        owner_.pending_interface_id = 0;
        ++owner_.force_selects;
        owner_.done("ForceSelectUnit::push_interface_20h", 0x00647077u);
    }

private:
    std::int32_t group_of(std::uint32_t unit) const {
        return unit == 0 ? -1 : owner_.units->unit_formation_group_0284(unit - 1);
    }
    GameHudHost::Impl& owner_;
    std::vector<std::uint32_t> list_;
};
}  // namespace

void GameHudHost::Impl::run_root_lists(bool force) {
    HudRootListsBinding binding(*this);
    const std::string before = units->controlled_bound()
        ? units->unit_row(units->controlled_index())->name : std::string("none");
    if (force) {
        bsp::hud_root_force_select_unit_006485a0(root_lists, binding);
    } else {
        bsp::hud_root_rebuild_unit_lists_00648290(root_lists, binding);
    }
    ++root_rebuilds;
    const std::uint32_t at = bsp::hud_root_cursor_unit_00644a60(root_lists);
    const GameUnitRow* at_row = at != 0 ? units->unit_row(at - 1) : nullptr;
    const std::string after = units->controlled_bound()
        ? units->unit_row(units->controlled_index())->name : std::string("none");
    log.notef("hud root unit lists %s (hud update frame %llu): +8Ch %zu units, +9Ch %zu units, "
        "cursor (%d,%d) at \"%s\"; controlled \"%s\" -> \"%s\"",
        force ? "006485A0 ForceSelectUnit" : "00648290 rebuild at the 44h enter",
        summary.update_frames, root_lists.primary_8c.size(), root_lists.secondary_9c.size(),
        root_lists.cursor.secondary ? 1 : 0,
        root_lists.cursor.index == 0xffffu ? -1 : static_cast<int>(root_lists.cursor.index),
        at_row != nullptr ? at_row->name.c_str() : "none", before.c_str(), after.c_str());
    // Packet cc9_vtable124_liveness: the +8Ch members in order, with class ids.
    std::string members;
    for (const std::uint32_t id : root_lists.primary_8c) {
        const GameUnitRow* row = id != 0 && id <= units->count() ? units->unit_row(id - 1) : nullptr;
        char cls[16];
        std::snprintf(cls, sizeof(cls), "/%02X", id != 0 ? units->unit_class_id(id - 1) & 0xff : 0);
        if (!members.empty()) members += ' ';
        members += row != nullptr ? row->name : std::string("?");
        members += cls;
    }
    log.notef("  +8Ch members: %s", members.empty() ? "(none)" : members.c_str());
}

void GameHudHost::enter_hud_root_screen_006488d0() {
    Impl& impl = *impl_;
    if (impl.units == nullptr) {
        impl.record("HudRootScreen::enter", 0x006488d0u);
        return;
    }
    // 006488D0..00648923: the three widget +4Ch(0.0f) calls, game mode into
    // +24h and +F4h = 0 are not performed (records, as before this packet);
    // 00648913 00648290 is.
    impl.record("HudRootScreen::enter_widgets_mode_counter", 0x006488d0u);
    impl.run_root_lists(false);
    impl.done("HudRootScreen::rebuild_unit_lists_00648290", 0x00648913u);
}

void GameHudHost::force_select_unit_006485a0() {
    Impl& impl = *impl_;
    ++impl.force_calls;
    if (impl.units == nullptr) {
        impl.record("ForceSelectUnit::no_units", 0x006485a0u);
        return;
    }
    impl.run_root_lists(true);
    impl.done("ForceSelectUnit::force_select_006485a0", 0x006485a0u);
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
        if constexpr (kHudShipScreenApplyUnitWidgetsBound) {
            // Packet cc13-wire2: the widgets 0064AE20 drives (addresses at the switch).
            static const std::pair<int, const char*> kApplyUnitNames[] = {
                {0x54, "ship_recon_Icon"}, {0x58, "ship_torpedo_Icon"},
                {0x68, "ship_recon_num1_Icon"}, {0x6C, "ship_recon_num2_Icon"},
                {0x70, "ship_torpedo_num1_Icon"}, {0x74, "ship_torpedo_num2_Icon"}};
            for (const auto& entry : kApplyUnitNames) {
                GuiLayoutWidget* found = nullptr;
                for (const char* page_name : kPages) {
                    GuiLayoutPage* page = menu.in_game_page(0x45, page_name);
                    if (page == nullptr || !page->root) continue;
                    found = bsp::find_descendant_by_name(*page->root, entry.second);
                    if (found != nullptr) break;
                }
                ship_screen_widgets[entry.first] = found;
            }
            // +40h holds the page itself (00AA5840's result), not a descendant.
            GuiLayoutPage* damage = menu.in_game_page(0x45, "GUI_ship_damage");
            ship_screen_widgets[0x40] =
                damage != nullptr && damage->root ? damage->root.get() : nullptr;
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
        if constexpr (kHudWeaponGroupLayoutBound) {
            // +1Ch is the screen's own byte, not modelled (labelled).
            owner_.weapon_group.flag_d4 = true;
            owner_.done("HudShipScreen::other_screen_00545360", 0x00545360u);
        } else {
            owner_.record("HudShipScreen::other_screen_00545360", 0x00545360u);
        }
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

namespace {
// The Big1Host adapter for 0064AE20 (kHudShipScreenApplyUnitBound). The body
// reads and writes the native layout, so the adapter lays out a scratch
// screen, unit and class record, runs the body over them and copies the
// written fields back into GameHudHost::Impl::ship_screen. Host state in:
// unit+980h = the row's throttle, class+70h = the row's type_id, screen+104h
// = gauge_sub_104 (the body leaves it unwritten off the submarine arm), and
// vtable[5Ch](8) = unit_is_kind_of. Every other input is a stand-in, labelled
// at its hook.
class ShipScreenApplyUnitAdapter final : public bsp::cc13_big1::Big1NullHost {
public:
    ShipScreenApplyUnitAdapter(GameHudHost::Impl& owner, bsp::ShipScreenHost& binding)
        : owner_(owner), binding_(binding) {}

    void run() {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        put<float>(unit_, 0x980, row != nullptr ? row->throttle : 0.0f);
        put<std::int32_t>(class_, 0x70, row != nullptr ? row->type_id : -1);
        if constexpr (kHudShipScreenApplyUnitLuaBound) {
            // class+C8h (the SETG at 0064AFA3) is Catapult.LaunchStock as 00960503
            // stored it at class load; NoRepairGUI is the 0064AED4 lookup. Read once
            // here, served by the Lua hooks below. Uncertain: whether anything
            // rewrites class+C8h after the load is not established.
            if (row != nullptr && owner_.lua != nullptr) {
                GameMissionLuaHost& lua = *owner_.lua;
                put<std::int32_t>(class_, 0xC8, lua.read_vehicle_class_integer(
                                                     row->type_id, "Catapult", "LaunchStock", 0));
                no_repair_gui_ =
                    lua.read_vehicle_class_boolean(row->type_id, "NoRepairGUI", false) ? 1 : 0;
            }
        }
        // OFF (or no row / Lua): STAND-IN class+C8h = 0.
        put<void*>(unit_, 0x538, class_);
        put<void*>(screen_, 0x184, unit_);
        for (int offset : {0x40, 0x54, 0x58, 0x68, 0x6C, 0x70, 0x74}) {
            put<void*>(screen_, offset, widget_tags_ + offset);
        }
        for (int i = 0; i < 3; ++i) put<void*>(screen_, 0x78 + 4 * i, gauges_[i]);
        bsp::ShipScreenState& state = owner_.ship_screen;
        screen_[0x104] = state.gauge_sub_104 ? 1 : 0;

        bsp::cc13_big1::hud_ship_screen45_apply_unit_0064ae20(screen_, *this);

        state.stick_44 = get<float>(screen_, 0x44);
        state.dir_target_128 = get<float>(screen_, 0x128);
        state.dir_rate_12c = get<float>(screen_, 0x12C);
        state.gauge_sub_104 = screen_[0x104] != 0;
        state.gauge_recon_105 = screen_[0x105] != 0;
        state.gauge_class_106 = screen_[0x106] != 0;
        state.gauge_torpedo_107 = screen_[0x107] != 0;
        for (int i = 0; i < 3; ++i) state.gauges[i].primed_18 = gauges_[i][0x18] != 0;
        owner_.done("HudShipScreen::apply_unit_0064ae20", 0x0064ae20u);
    }

    void unreached(const char* hook, std::uint32_t address) override {
        static_cast<void>(hook);
        owner_.record("HudShipScreen::apply_unit_unexpected_hook", address);
    }
    // STAND-IN: the game singleton is only the base of 0064AE4D's +1A0Ch owner,
    // which the stand-in Lua hooks below never read.
    void* game_00e188a8() override { return game_; }
    // The Lua object hooks. The table walk itself (globals, VehicleClass, the
    // row, NoRepairGUI) is served by the one read_vehicle_class_boolean in run()
    // under kHudShipScreenApplyUnitLuaBound; these only carry its answer. The
    // body tests is_boolean && get_boolean == 1, and the reader returns false
    // unless the key is boolean true, so answering is_boolean = get_boolean =
    // the read value takes the same arm. OFF: STAND-IN, the key reads as not a
    // boolean and +40h is shown (0064AEE2 false arm).
    void* lua_state_owner_get_globals_00b67980(void*, void* out) override { return out; }
    void* lua_object_get_by_name_00b67800(void*, void* out, const char*) override { return out; }
    void* lua_object_get_by_index_00b67720(void*, void* out, std::int32_t) override { return out; }
    void lua_object_construct_00b65f50(void*) override {}
    void lua_object_assign_00b67690(void*, void*) override {}
    void lua_object_destruct_00b67700(void*) override {}
    bool lua_object_is_boolean_00b66000(void*) override {
        if constexpr (kHudShipScreenApplyUnitLuaBound) {
            return no_repair_gui_ != 0;
        } else {
            owner_.record("HudShipScreen::apply_unit_no_repair_gui", 0x0064aee2u);
            return false;
        }
    }
    std::uint8_t lua_object_get_boolean_00b66250(void*) override {
        return no_repair_gui_ != 0 ? 1 : 0;
    }
    // vtable +34h (show) through the ship screen binding. With
    // kHudShipScreenApplyUnitWidgetsBound the name map holds +40h (the damage
    // page root), +54h, +58h and +68h..+74h; OFF they reach no widget.
    void widget_vtable34(void* widget, std::uint8_t arg) override {
        binding_.set_visible(widget_at(widget), arg != 0);
    }
    // vtable +88h, only ever on +54h (0064B023 frame +106h, 0064B0C5 frame 2).
    // Recorded under this body's own name, not the relation icon's 00AB1710
    // record that ShipScreenBinding::select_state writes: the bridge draws a
    // widget's first authored state only.
    void widget_vtable88(void* widget, std::uint32_t frame, std::int32_t arg,
                         float value) override {
        static_cast<void>(widget);
        static_cast<void>(frame);
        static_cast<void>(arg);
        static_cast<void>(value);
        owner_.record("HudShipScreen::apply_unit_recon_frame", 0x0064b023u);
    }
    float hud_ship_screen45_update_dir_icon_0064aad0(void*) override {
        return bsp::ship_screen_dir_0064aad0(binding_);
    }
    void hud_ship_screen45_update_throttle_indicator_0064a9f0(void*, float value) override {
        bsp::ship_screen_stick_0064a9f0(binding_, value);
    }
    void hud_ship_screen45_update_formation_icon_0064a960(void*) override {
        bsp::ship_screen_relation_icon_0064a960(owner_.ship_screen, binding_);
    }
    bool unit_has_flag_vtable5c(void*, std::int32_t flag) override {
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.ship_screen_unit, flag);
    }
    // STAND-IN: 00818300 sums +448h over the unit's kind-7 gun children,
    // 00852350 counts its bomb-platform children and 0059CB90 reads the HUD
    // icon from the class; the host builds none of them, so each answers 0.
    std::int32_t unit_torpedo_barrel_count_00818300(void*) override {
        owner_.record("HudShipScreen::apply_unit_torpedo_barrels", 0x00818300u);
        return 0;
    }
    std::int32_t unit_count_bomb_platform_children_00852350(void*) override {
        owner_.record("HudShipScreen::apply_unit_bomb_platforms", 0x00852350u);
        return 0;
    }
    std::uint32_t unit_hud_icon_index_0059cb90(void*, void*) override {
        owner_.record("HudShipScreen::apply_unit_hud_icon", 0x0059cb90u);
        return 0;
    }

private:
    template <typename T> static void put(std::uint8_t* base, int offset, T value) {
        std::memcpy(base + offset, &value, sizeof(T));
    }
    template <typename T> static T get(const std::uint8_t* base, int offset) {
        T value;
        std::memcpy(&value, base + offset, sizeof(T));
        return value;
    }
    bsp::ShipScreenWidget widget_at(void* widget) const {
        const auto offset = static_cast<std::uint8_t*>(widget) - widget_tags_;
        return static_cast<bsp::ShipScreenWidget>(offset);
    }

    GameHudHost::Impl& owner_;
    bsp::ShipScreenHost& binding_;
    std::uint32_t no_repair_gui_{0};  // a dword so the alignas(4) arrays need no padding
    alignas(4) std::uint8_t screen_[0x1B0]{};
    alignas(4) std::uint8_t unit_[0x988]{};
    alignas(4) std::uint8_t class_[0xD0]{};
    alignas(4) std::uint8_t gauges_[3][0x1C]{};
    std::uint8_t widget_tags_[0x80]{};
    alignas(4) std::uint8_t game_[0x1A10]{};
};
}  // namespace

void GameHudHost::Impl::apply_ship_screen_unit_0064ae20() {
    ShipScreenBinding binding(*this);
    ShipScreenApplyUnitAdapter adapter(*this, binding);
    adapter.run();
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
        owner_.menu.note_screen_2eh_ran();   // the helm `input` line's lost-edge report
        if constexpr (kHudWeaponGroupScreenBound) {
            WeaponGroupScreenBinding group_host(owner_);
            bsp::weapon_group_screen_update_005484f0(owner_.weapon_group, group_host);
            owner_.done("HudWeaponGroupScreen::update", 0x005484f0u);
        } else {
            owner_.record("HudShipView::screen_2eh_005484f0", 0x005484f0u);
        }
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
    ShipViewInputBinding(GameHudHost::Impl& owner, float dt) : owner_(owner), dt_(dt) {}
    bsp::BinocularsViewTerms view_terms() override {
        // SUBSTITUTION: the input manager's axes +1584h/+15B4h/+15E4h are not
        // modelled and read zero (no in-mission input is driven);
        // GlobalConfig+4 is unread and reads 1.0. It only scales a zero axis.
        owner_.record("HudShipView::view_input_terms", 0x0051f061u);
        bsp::BinocularsViewTerms terms{};
        terms.config_04 = 1.0f;
        helm_aim_axes(terms);
        return terms;
    }
    // Packet cc13-helmaim: the helm `aim` line drives the look axes the way a
    // mouse move does. Each axis is the one that makes 0051F050's step land the
    // mover on the target: yaw += step * -1.0 * zoom * axis * config (0051F159)
    // and pitch += step * 0.5 * zoom * axis * config, wrapped, then clamped into
    // [+3ECh, +3F0h] (0051E650). 0051F050 clamps each axis into [-50, 50], so a
    // long turn takes more than one frame. The target clears once both angles are
    // there, and the mover keeps them: no reader decays +384h / +388h.
    void helm_aim_axes(bsp::BinocularsViewTerms& terms) {
        float yaw = 0.0f;
        float pitch = 0.0f;
        const bool tracking = helm_aim_track_target(yaw, pitch);
        if (!tracking && !injected_view_aim(yaw, pitch)) return;
        if (!owner_.camera_bound) return;
        const float low = owner_.camera.min_pitch_3ec;
        const float high = owner_.camera.max_pitch_3f0;
        if (low > pitch) pitch = low;
        else if (pitch > high) pitch = high;
        const auto wrap = [](double a) {
            constexpr double kPi = 3.141592653589793;
            while (a > kPi) a -= 2.0 * kPi;
            while (a <= -kPi) a += 2.0 * kPi;
            return a;
        };
        const double dyaw = wrap(static_cast<double>(yaw) - owner_.camera.yaw_384);
        const double dpitch = wrap(static_cast<double>(pitch) - owner_.camera.pitch_388);
        constexpr double kReached = 1.0e-4;
        if (tracking) {
            // `aim unit` never completes; it steers again next frame.
            if (std::fabs(dyaw) < kReached && std::fabs(dpitch) < kReached) return;
        } else if (std::fabs(dyaw) < kReached && std::fabs(dpitch) < kReached) {
            clear_injected_view_aim();
            owner_.log.notef("helm aim reached: mover yaw %.4f pitch %.4f rad (+384h / +388h)",
                static_cast<double>(owner_.camera.yaw_384),
                static_cast<double>(owner_.camera.pitch_388));
            return;
        }
        const double step = static_cast<double>(dt_) > 0.5 ? 0.5 : static_cast<double>(dt_);
        const double zoom = owner_.binoculars.zoom_38;
        if (!(step > 0.0) || !(zoom > 0.0)) return;
        terms.yaw_axis_1584 = static_cast<float>(dyaw / (step * -1.0 * zoom * terms.config_04));
        terms.pitch_axis_15b4 = static_cast<float>(dpitch / (step * 0.5 * zoom * terms.config_04));
    }
    // Packet cc13-aimunit: the `aim unit` track. The look is steered in closed
    // loop on the camera's published view (the forward row and position 00954A10
    // reads for message 79h, i.e. the view ray after the orbit position, zoom and
    // shake, which 009578C3 tests), so it needs no model of +384h's frame:
    // 00432ED0 builds the forward row as rot_y(-yaw) then rot_x(-pitch) on the
    // mover's basis, so a heading error e (atan2(x, z) convention) is removed by
    // yaw -= e, and a pitch error by pitch += e. The auto pitch aims at the unit's
    // origin (unit+0FCh). Returns false (no steering) when there is no track or
    // nothing to steer from. A row that becomes unusable is reported once per
    // loss (the look keeps its last yaw) and its return is reported too.
    bool helm_aim_track_target(float& yaw, float& pitch) {
        std::size_t unit = 0;
        bool has_pitch = false;
        float fixed_pitch = 0.0f;
        if (!injected_view_aim_track(unit, has_pitch, fixed_pitch)) return false;
        const unsigned serial = injected_view_aim_track_serial();
        if (serial != owner_.aim_track_serial) {
            // A new `aim unit` line: its own loss report and its own first frames.
            owner_.aim_track_serial = serial;
            owner_.aim_track_warned = false;
            owner_.aim_track_frames = 0;
        }
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!owner_.camera_bound || !node.ready) return false;
        if (owner_.units == nullptr || unit >= owner_.units->count()
            || !owner_.units->unit_alive_and_visible(unit)) {
            if (!owner_.aim_track_warned) {
                owner_.aim_track_warned = true;
                owner_.log.notef("helm aim unit: row %zu is gone or hidden; the look keeps its "
                    "last yaw", unit);
            }
            return false;
        }
        if (owner_.aim_track_warned) {
            owner_.aim_track_warned = false;
            owner_.log.notef("helm aim unit: row %zu is back; tracking resumes", unit);
        }
        // As the 79h build reads it (00548839..005489DA).
        if ((node.state.transform.valid_flags & 2u) == 0) {
            bsp::refresh_camera_world_00b6db70(node.state.transform);
        }
        const bsp::CameraMatrix& world = node.state.transform.world;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        owner_.units->unit_position_00fc(unit, tx, ty, tz);
        // The auto pitch's aim point. A ship (kind 6) is aimed at the image's
        // line-of-sight target point, 00864D90: the pose +0FCh raised at
        // 00864E17 by GlobalConfig+90h, LOSTargetHeightAdd (5.0 on this
        // installation), plus the class Height (+A8h) from the IsKindOf(5) arm at
        // 00864E25. The image applies +90h to every target and gates the Height
        // on IsKindOf(5); restricting the point to kind 6 is this harness's
        // choice. UNCERTAIN: when class+50h is non-empty the image's height term
        // is max(+90h, (bbox+38h - +2Ch) * +98h) through 00415550; that mesh arm
        // is not modelled here. The seat ray's hit must lie above y = 0
        // (00957891: COMISS hit.y, [00D7A218] 0.0f; JBE drops it), and aiming at
        // the origin (the waterline) put every hull hit at or below it.
        // Other units keep their origin.
        double aim_y = ty;
        if (owner_.units->unit_is_kind_of(unit, bsp::kUnitGunneryKindShipBase)) {
            float length = 0.0f, width = 0.0f, height = 0.0f;
            owner_.units->unit_class_extents(unit, length, width, height);
            aim_y = static_cast<double>(ty) + height + bsp::kInstalledLosTargetHeightAdd;
            // HARNESS clamp, not in the image: a ship origin below 0 (heave,
            // sinking) never pulls the aim point below y = 5.0.
            if (aim_y < bsp::kInstalledLosTargetHeightAdd) aim_y = bsp::kInstalledLosTargetHeightAdd;
        }
        const double dx = static_cast<double>(tx) - world[12];
        const double dy = aim_y - world[13];
        const double dz = static_cast<double>(tz) - world[14];
        const double flat = std::sqrt(dx * dx + dz * dz);
        if (!(flat > 1.0e-3)) return false;
        constexpr double kPi = 3.141592653589793;
        double e_heading = std::atan2(dx, dz) - std::atan2(world[8], world[10]);
        while (e_heading > kPi) e_heading -= 2.0 * kPi;
        while (e_heading <= -kPi) e_heading += 2.0 * kPi;
        yaw = static_cast<float>(owner_.camera.yaw_384 - e_heading);
        if (has_pitch) {
            pitch = fixed_pitch;
        } else {
            const double fy = std::max(-1.0, std::min(1.0, static_cast<double>(world[9])));
            pitch = static_cast<float>(owner_.camera.pitch_388
                + (std::atan2(dy, flat) - std::asin(fy)));
        }
        // Every one of a track's first 10 frames (a 2-frame oscillation shows),
        // then every 200th.
        const unsigned long long frame = owner_.aim_track_frames++;
        if (frame < 10 || frame % 200 == 0) {
            owner_.log.notef("helm aim unit: row %zu heading error %.4f rad, target pitch %.4f "
                "rad, range %.0f", unit, e_heading, static_cast<double>(pitch), flat);
        }
        return true;
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
    float dt_;
};

void ShipViewScreen46Binding::view_input_0064a400(float dt) {
    if (!kHudShipViewInputBound) {
        // Screen 26h's 0051F330 and screen 2Eh's 005454B0, not bound.
        owner_.record("HudShipView::view_input", 0x0064a400u);
        return;
    }
    ShipViewInputBinding binding(owner_, dt);
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

// 0067BB50, slot 20h of vtable 00CF7A38 (HUD page 27h). The pump only calls
// it while the page's +4h and +5h are set, so the routine's +4h test at
// 0067BBE0 holds on this path. The units host owns the take.
void GameHudHost::update_role_screen_0067bb50() {
    Impl& impl = *impl_;
    if (impl.units == nullptr) {
        impl.log.unimplemented("HudRoleScreen::update_no_units", "0067bb50");
        return;
    }
    impl.units->role_screen_update_0067bb50();
    impl.done("HudRoleScreen::update", 0x0067bb50u);
}

void GameHudHost::enter_weapon_group_screen_005494c0() {
    Impl& impl = *impl_;
    WeaponGroupScreenBinding host(impl);
    WeaponGroupLayoutBinding gui(impl);
    bsp::weapon_group_screen_enter_005494c0(impl.weapon_group, host, gui);
    impl.done("HudWeaponGroupScreen::bind", 0x00549260u);
    impl.done("HudWeaponGroupScreen::enter", 0x005494c0u);
}

void GameHudHost::exit_weapon_group_screen_005470a0() {
    Impl& impl = *impl_;
    WeaponGroupScreenBinding host(impl);
    WeaponGroupLayoutBinding gui(impl);
    bsp::weapon_group_screen_exit_005470a0(impl.weapon_group, host, gui);
    impl.done("HudWeaponGroupScreen::exit", 0x005470a0u);
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
        // cc15-hint2-gate: 0064987D CMP byte [EAX+620h],0 / JNZ 00649FB3 skips
        // the whole update while a critical hint is shown.
        owner_.record("HudRootScreen::game_pause_bytes", 0x00649863u);
        if constexpr (kHintPauseHudGateCc15Bound) return game_hint_pause_620_cc15();
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
// Packet cc9_pick_squadron_members (docs/PICK_SQUADRON_MEMBERS.md). ON answers
// 00526A40's squadron arm from bsp::plane_squadron_registry():
//  * 00526F33 IsKindOf(18h) on a LIST ENTRY is true when the entry is a
//    registry squadron_unit. SUBSTITUTION: this host fuses the 414h container
//    with its wing-0 leader (docs/PLANE_SQUADRON_HOST.md section 1), so the
//    leader stands for the container here; every other kind test is the unit's.
//  * +3CCh (00526FBE) is the record's live_count(), +3D0h[k] (00526E58,
//    00526FCF) its k-th live member, index + 1. The image's array holds only
//    planes that exist; the host keeps a no-unit slot for a wing that never
//    became a unit, so the k-th LIVE member is the image's slot k.
// OFF keeps the entry as the one plane it is and records squadron_members.
// ON by the verdict: the USN04 and E2 pairs are identical (docs section 4). The
// arm is unexercised there, because no squadron is in either list.
constexpr bool kPickSquadronMembersBound = true;

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
        if (unit == 0) return false;
        if (kPickSquadronMembersBound && class_id == 0x18 && squadron_of(unit) != nullptr) {
            owner_.done("UnitPickScreen::squadron_entry", 0x00526f33u);
            return true;
        }
        return owner_.units->unit_is_kind_of(unit - 1, class_id);
    }
    // The registry record whose squadron_unit is `unit` (index + 1), or null.
    const bsp::PlaneSquadronHostRecord* squadron_of(std::size_t unit) const {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit - 1) {
                return &r;
            }
        }
        return nullptr;
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
        const int source = owner_.movie_active ? 2 : 1;
        if (source != owner_.pick_basis_source) {
            owner_.pick_basis_source = source;
            owner_.pick_first_basis_logged = true;
            owner_.log.notef("unit pick: camera basis now from the %s at hud "
                "update frame %llu: from=(%.1f,%.1f,%.1f) forward=(%.4f,%.4f,%.4f)",
                source == 2 ? "movie camera" : "published mover",
                owner_.summary.update_frames, static_cast<double>(position[0]),
                static_cast<double>(position[1]), static_cast<double>(position[2]),
                static_cast<double>(forward[0]), static_cast<double>(forward[1]),
                static_cast<double>(forward[2]));
        }
    }
    bool ray_pick_009043a0(const float* from, const float* to, std::size_t ignore,
                           std::size_t& hit) override {
        hit = 0;
        if constexpr (kHudPickSegmentQueryBound) {
            // Packet cc9_hud_ray_pick (docs/HUD_PICK_SEGMENT_QUERY.md sections
            // 1-2): 009043A0 with kind filter 0 (00526BCD) and the firing unit
            // as `ignore`, whose vtable +B0h is its own collision root here.
            // 0098ADD0 runs over the gunnery host's units (SegmentBinding):
            // SUBSTITUTION for the world grid [game+19CCh]+84h, which is not
            // built in this process.
            const GameGunneryHost* gunnery =
                owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (gunnery == nullptr) {
                owner_.record("UnitPickScreen::segment_query", 0x009043a0u);
                return false;
            }
            float point[3];
            const bool any = gunnery->query_segment_units(from, to, ignore, hit, point);
            owner_.done("UnitPickScreen::segment_query", 0x009043a0u);
            if (any) owner_.done("UnitPickScreen::segment_query_hit", 0x00526c62u);
            return any;
        } else {
            // SUBSTITUTION: the spatial index ([00E188A8]+19CCh, 0098ADD0) is not
            // built, so the segment query reports no hit.
            static_cast<void>(from);
            static_cast<void>(to);
            static_cast<void>(ignore);
            owner_.record("UnitPickScreen::segment_query", 0x009043a0u);
            return false;
        }
    }
    std::size_t ray_hit_branch(bsp::UnitPickScreenState& screen, std::size_t hit) override {
        if constexpr (kHudPickSegmentQueryBound) {
            // 00526C74..00526DB2.
            if (is_kind_of(hit, 6)) {
                // 00526C80..00526D15: the section label at +54h and the two
                // points from record+38h, which no host narrowphase writes;
                // +54h keeps 00CE43EC "" (section_named_54 false).
                owner_.record("UnitPickScreen::hit_section", 0x00526c8cu);
            }
            // 00526D1A..00526D78: these kinds are the pick themselves.
            for (const int kind : {6, 0x0f, 0x45, 0x46, 0x1b, 0x35}) {
                if (!is_kind_of(hit, kind)) continue;
                const bool own = owner_.units != nullptr && controlled_unit() != 0
                    && owner_.units->unit_side_0054(hit - 1)
                        == owner_.units->unit_side_0054(controlled_unit() - 1);
                owner_.done(own ? "UnitPickScreen::ray_pick_own" : "UnitPickScreen::ray_pick_other",
                    0x00526d9cu);
                return hit;
            }
            // 00526D80: kind 1Eh, +50h = the hit and the pick is 00923810(hit, 1).
            if (is_kind_of(hit, 0x1e)) {
                screen.exclude_hit_50 = hit;
                owner_.record("UnitPickScreen::part_owner_00923810", 0x00923810u);
                return 0;
            }
            owner_.done("UnitPickScreen::ray_hit_other_kind", 0x00526dafu);
            return 0;
        } else {
            static_cast<void>(screen);
            static_cast<void>(hit);
            owner_.record("UnitPickScreen::ray_hit_branch", 0x00526c74u);
            return 0;
        }
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
        if constexpr (kReconUnitListSourcesBound) {
            // Packet cc9_recon_call_sites: 00526E01 reads the head at
            // game+1974h, the list game+1970h (walk 0's everything-not-
            // ordnance), and 00526FF1 the head at game+19BCh, the list
            // game+19B8h (walk 1's 35h and objective-set units, then the
            // merge of ships, squadrons, airfields, shipyards and forts).
            // 004C3CB0 built both once after the load; the tests after the
            // list are 00526A40's own (bsp::unit_pick_00526a40).
            team_.clear();
            const bsp::LocalPlayerUnitLists* lists = game_local_player_unit_lists();
            const bool team = list == bsp::UnitPickList::TeamUnits;
            const char* name = team ? "UnitPickScreen::list_1970" : "UnitPickScreen::list_19b8";
            const std::uint32_t site = team ? 0x00526e01u : 0x00526ff1u;
            if (lists == nullptr) {
                owner_.record(name, site);
                return 0;
            }
            const auto& source = (*lists)[static_cast<std::size_t>(team
                ? bsp::LocalPlayerUnitList::kWalk0Rest : bsp::LocalPlayerUnitList::kMerged)];
            for (const bsp::UnitRef ref : source) team_.push_back(static_cast<std::size_t>(ref));
            owner_.done(name, site);
            report_list_census(team, team_);
            return team_.size();
        }
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
    int member_count_3cc(std::size_t squadron) override {
        if constexpr (kPickSquadronMembersBound) {
            // 00526FBE CMP EBP,[EDI+3CCh].
            const bsp::PlaneSquadronHostRecord* r = squadron_of(squadron);
            return r != nullptr ? r->live_count() : 0;
        } else {
            static_cast<void>(squadron);
            return 0;
        }
    }
    std::size_t member_3d0(std::size_t squadron, int k) override {
        if constexpr (kPickSquadronMembersBound) {
            // 00526E58 [EDI+3D0h] for k = 0, 00526FCF [ECX] for the next slots.
            const bsp::PlaneSquadronHostRecord* r = squadron_of(squadron);
            if (r != nullptr && k >= 0) {
                int live = 0;
                for (const std::size_t u : r->member_units) {
                    if (u == bsp::kPlaneSquadronNoUnit) continue;
                    if (live++ == k) {
                        owner_.done("UnitPickScreen::squadron_members", 0x00526e58u);
                        return u + 1;
                    }
                }
            }
            owner_.done("UnitPickScreen::squadron_member_empty", 0x00526e5eu);
            return 0;
        } else {
            // SUBSTITUTION: squadron members (+3D0h, count +3CCh) are not exposed.
            static_cast<void>(squadron);
            static_cast<void>(k);
            owner_.record("UnitPickScreen::squadron_members", 0x00526e58u);
            return 0;
        }
    }
    bool grey_arrow_contains_008ddf90(std::size_t unit) override {
        if constexpr (kHudGreyArrowSetBound) {
            // 008DDF90 on the local slot's set (game+18ECh is 0 here). The
            // set's unit tree (+18h, size +20h) is what the objective table
            // holds per slot; the Lua host fills it (Objectives_AddUnit).
            const std::vector<std::size_t> set =
                bsp::game::game_objective_sets().units_in_slot(0);
            if (set.empty() || unit == 0 || owner_.units == nullptr) {  // 008DDF93
                owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
                return false;
            }
            std::size_t subject = unit - 1;
            if (!owner_.units->unit_is_kind_of(subject, 5)) {           // 008DDFA9
                if (owner_.units->unit_is_kind_of(subject, 0x18)) {     // 008DDFB8
                    // unit+3D0h, a squadron's plane: not exposed here.
                    owner_.record("UnitPickScreen::grey_arrow_squadron", 0x008ddfc2u);
                }
                owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
                return false;
            }
            owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
            for (const std::size_t member : set) {                      // 008DDF00
                if (member == subject) return true;
            }
            return false;
        }
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
        // Mode 1 multiplies y by the GUI extent height (00AA1FE0()+4h).
        // OFF: SUBSTITUTION of 1.0, the value its 4/3 law gives
        // (docs/HUD_PRESENTATION_TOP.md row 2), as a record.
        float extent_height = 1.0f;
        const Win32PlatformState* platform =
            kHudGuiExtentBound ? active_platform_state() : nullptr;
        if (platform != nullptr) {
            extent_height = bsp::gui_aspect_extent_00aa1fe0(platform->widescreen,
                                                            platform->active_aspect).height;
            owner_.done("UnitPickScreen::gui_extent", 0x00aa1fe0u);
        } else {
            owner_.record("UnitPickScreen::gui_extent", 0x00aa1fe0u);
        }
        const bsp::HudMarkerScreenProjection p = bsp::camera_project_world_to_screen_0043a660(
            clip[0], clip[1], clip[2], clip[3], bsp::HudMarkerProjectMode::ViewportAspect, true,
            extent_height, 1.0f);
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
    // Packet cc9_pick_squadron_members (docs/PICK_SQUADRON_MEMBERS.md section 1):
    // the class census of each pick list. 00526F33 expands an entry that answers
    // IsKindOf(18h) into its +3D0h members. The line prints when a list's shape
    // changes, so a mission logs a few lines, not one per update.
    void report_list_census(bool team, const std::vector<std::size_t>& entries) {
        struct Shape {
            std::size_t size{static_cast<std::size_t>(-1)};
            std::size_t kind_18{0};
            std::size_t kind_5{0};
            std::size_t squadron_units{0};
            std::size_t members{0};
        };
        static Shape last[2];
        Shape now;
        now.size = entries.size();
        std::map<int, std::size_t> classes;
        const bsp::PlaneSquadronRegistry& registry = bsp::plane_squadron_registry();
        for (const std::size_t unit : entries) {
            if (unit == 0) {
                ++classes[-1];
                continue;
            }
            ++classes[owner_.units->unit_class_id(unit - 1)];
            if (owner_.units->unit_is_kind_of(unit - 1, 0x18)) ++now.kind_18;
            if (owner_.units->unit_is_kind_of(unit - 1, 5)) ++now.kind_5;
            if (registry.find_by_member_unit(unit - 1) != nullptr) ++now.members;
            for (const bsp::PlaneSquadronHostRecord& r : registry.records()) {
                if (r.squadron_unit == unit - 1) {
                    ++now.squadron_units;
                    break;
                }
            }
        }
        Shape& prev = last[team ? 0 : 1];
        if (now.size == prev.size && now.kind_18 == prev.kind_18 && now.kind_5 == prev.kind_5
            && now.squadron_units == prev.squadron_units && now.members == prev.members) {
            return;
        }
        prev = now;
        std::string histogram;
        for (const auto& [class_id, count] : classes) {
            char item[32];
            std::snprintf(item, sizeof item, " %Xh:%zu", static_cast<unsigned>(class_id), count);
            histogram += item;
        }
        owner_.log.notef("pick list census: list=%s entries=%zu kind18=%zu kind5=%zu "
            "squadron_units=%zu squadron_members=%zu registry_squadrons=%zu classes:%s",
            team ? "1970" : "19b8", now.size, now.kind_18, now.kind_5, now.squadron_units,
            now.members, registry.size(), histogram.c_str());
    }

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
    impl.log.notef("summary mission hud gun aim message 79h (cc15-smA, 00954A10 / 0075FFF0 / "
        "0075FF60 / 007600D0): bound=%d round_trips=%llu changed yaw=%llu pitch=%llu camera=%llu "
        "bools=%llu group=%llu target=%llu max_delta yaw=%.9g pitch=%.9g",
        bsp::kGunAimMessageCc15Bound ? 1 : 0, impl.gun_aim_round_trips, impl.gun_aim_changed[0],
        impl.gun_aim_changed[1], impl.gun_aim_changed[2], impl.gun_aim_changed[3],
        impl.gun_aim_changed[4], impl.gun_aim_changed[5], impl.gun_aim_max_yaw_delta,
        impl.gun_aim_max_pitch_delta);
    impl.log.notef("summary mission hud set selected unit (packet cc9_set_selected_unit, "
        "008AB260 / 00647300): bound=%d calls=%llu accepted=%llu pushes_20h=%llu",
        kSetSelectedUnitBound ? 1 : 0, impl.select_calls, impl.select_accepted,
        impl.select_pushes);
    {
        std::string only;
        for (const auto& entry : impl.vtable124_map_only) {
            const GameUnitRow* row = impl.units != nullptr ? impl.units->unit_row(entry.first) : nullptr;
            char tail[48];
            std::snprintf(tail, sizeof(tail), "/%02X@%08X",
                impl.units != nullptr ? impl.units->unit_class_id(entry.first) & 0xff : 0,
                static_cast<unsigned>(entry.second));
            if (!only.empty()) only += ' ';
            only += row != nullptr ? row->name : std::string("?");
            only += tail;
        }
        std::string slots;
        for (const std::size_t u : impl.squadron_slots_seen) {
            const GameUnitRow* row = impl.units != nullptr ? impl.units->unit_row(u) : nullptr;
            char tail[24];
            std::snprintf(tail, sizeof(tail), "/%02X->%02X",
                impl.units != nullptr ? impl.units->unit_class_id(u) & 0xff : 0,
                impl.selection_class_id(u) & 0xff);
            if (!slots.empty()) slots += ' ';
            slots += row != nullptr ? row->name : std::string("?");
            slots += tail;
        }
        impl.log.notef("summary mission hud squadron slot class (packet cc9_squadron_slot_class, "
            "00645060 / 00645600): bound=%d squadron slots asked=%zu: %s",
            kSquadronSlotClassBound ? 1 : 0, impl.squadron_slots_seen.size(),
            slots.empty() ? "(none)" : slots.c_str());
        impl.log.notef("summary mission hud unit vtable124 map (packet cc9_vtable124_liveness, "
            "00645110 / 0064565F): bound=%d asks=%llu map_only_asks=%llu map_only_units=%zu: %s",
            kUnitVtable124MapBound ? 1 : 0, impl.vtable124_asks, impl.vtable124_map_only_asks,
            impl.vtable124_map_only.size(), only.empty() ? "(none)" : only.c_str());
    }
    impl.log.notef("summary mission hud movie interface (packet cc9_movie_interface_and_reseed, "
        "005CD240 / 005CD1A0): bound=%d calls=%llu engages=%llu pushes_2ch=%llu reseed_bound=%d "
        "stream1_reseeds=%llu",
        kMovieInterfacePushBound ? 1 : 0, impl.movie_calls, impl.movie_engages,
        impl.movie_pushes, kMovieReseedBound ? 1 : 0, impl.movie_reseeds);
    impl.log.notef("summary mission hud movie camera (packet cc9_movie_camera_mover_bind, "
        "005CC170 / 0079A3B0): bound=%d builds=%llu destroys=%llu keyframes=%llu "
        "publishes=%llu unsupported=%llu active_at_end=%d", kMovieMoverBound ? 1 : 0,
        impl.movie_builds, impl.movie_destroys, impl.movie_keyframes, impl.movie_publishes,
        impl.movie_unsupported, impl.movie_active ? 1 : 0);
    impl.log.notef("summary mission hud movie keyframe modifiers (packet cc9_lua49_movie_modifier, "
        "00795CB7 / 00797ED0): bound=%d events_fired=%llu", bsp::kMovieKeyframeModifierBound ? 1 : 0,
        impl.movie_events_fired);
    impl.log.notef("summary mission hud movie camera keys (packet cc9_movie_camera_keys): "
        "step_draw_bound=%d step_draws=%llu parent_killed_byte_bound=%d "
        "terrain_avoid_bound=%d", kMovieStepDrawBound ? 1 : 0, impl.movie_step_draws,
        kMovieParentKilledByteBound ? 1 : 0, kMovieTerrainAvoidBound ? 1 : 0);
    impl.log.notef("summary mission hud force select unit (packet cc9_force_select_unit, "
        "008AAF30 / 006485A0 / 00648290): bound=%d calls=%llu selects=%llu rebuilds=%llu "
        "primary_8c=%zu secondary_9c=%zu", kForceSelectUnitBound ? 1 : 0, impl.force_calls,
        impl.force_selects, impl.root_rebuilds, impl.root_lists.primary_8c.size(),
        impl.root_lists.secondary_9c.size());
    impl.log.notef("summary mission hud root observer (packet cc9_controlled_unit_observer, "
        "00644A20): bound=%d notices=%llu releases=%llu", kControlledUnitObserverBound ? 1 : 0,
        impl.observer_notices, impl.observer_releases);
    impl.log.notef("summary mission hud squadron observer liveness bound=%d holds=%llu (007F3970 "
        "+3CCh -> 00926D90, packet cc9_squadron_observer_liveness)",
        kSquadronObserverLivenessBound ? 1 : 0, impl.observer_squadron_holds);
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

void GameHudHost::Impl::run_root_observer_notice() {
    // The destruction notice. SUBSTITUTION (labelled): the image delivers it
    // in the on-killed dispatch (009274DE..00927510, 00696330 -> 00693550 ->
    // observer->vtable[4h]); here the units host's destroyed-unit record
    // (a sunk gunnery row) stands for it, read once per interface update.
    if (observed_unit == SIZE_MAX || units == nullptr) return;
    bool destroyed = false;
    for (const auto& death : units->destroyed_units()) {
        if (death.first == observed_unit) { destroyed = true; break; }
    }
    if constexpr (kSquadronObserverLivenessBound) {
        // Packet cc9_squadron_observer_liveness. For a squadron's fused slot
        // the observed entity is the squadron, which 007F3970 kills (00926D90)
        // only when +3CCh reaches 0. SUBSTITUTION (labelled): the host reads
        // the registry record's live count at the interface update; the image
        // kills the squadron inside the last plane's kill flush.
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != observed_unit) {
                continue;
            }
            const bool squadron_gone = r.live_count() <= 0;
            if (destroyed && !squadron_gone && !squadron_hold_logged) {
                ++observer_squadron_holds;
                squadron_hold_logged = true;
                log.notef("hud root observer 00644A20: the fused slot died but squadron \"%s\" "
                    "keeps %d live plane(s) (+3CCh), so no notice (packet "
                    "cc9_squadron_observer_liveness)", r.name.c_str(), r.live_count());
            }
            // Only once the fused slot's own plane has died: a record whose
            // members never became units reads 0 while the squadron is alive.
            destroyed = destroyed && squadron_gone;
            break;
        }
    }
    if (!destroyed) return;
    squadron_hold_logged = false;
    ++observer_notices;
    const std::size_t unit = observed_unit;
    observed_unit = SIZE_MAX;   // the entity's observer list goes with it
    // 00644A20..00644A50: only when the entity is [00E188D8].
    if (!units->controlled_bound() || units->controlled_index() != unit) {
        done("HudRootObserver::destruction_notice_other", 0x00644a2du);
        return;
    }
    record("HudRootObserver::clear_root_1c", 0x00644a31u);   // root+1Ch, no host field
    units->clear_controlled_unit_004c0890();                  // 00644A38
    root_lists.cursor = bsp::HudUnitSelection{false, 0xffffu};   // 00644A3D/00644A44
    ++observer_releases;
    done("HudRootObserver::destruction_notice_00644a20", 0x00644a20u);
    const GameUnitRow* row = units->unit_row(unit);
    log.notef("hud root observer 00644A20: controlled unit \"%s\" destroyed; 004C0890(null) "
        "and cursor (0,-1) at hud update frame %llu", row != nullptr ? row->name.c_str() : "?",
        summary.update_frames);
}

void GameHudHost::update_in_game_interface_0068c1f0() {
    Impl& impl = *impl_;
    if constexpr (kControlledUnitObserverBound) impl.run_root_observer_notice();
    if (!manager_active()) return;
    HudUpdateBinding binding(impl);
    bsp::update_in_game_interface_0068c1f0(impl.manager, impl.update_state, binding);
    impl.done("InGameInterface::update", 0x0068c1f0u);
    ++impl.summary.update_frames;
}

}  // namespace bsp::game
