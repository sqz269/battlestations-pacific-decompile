#if defined(_MSC_VER) && defined(_M_IX86) && defined(_DEBUG)
#include <crtdbg.h>
#include <cstdlib>
#include <new.h>
#include <new>
#include "bsp/dyn_profile_scopes.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#endif

#include "bsp/ship_ai_approach_curves.hpp"
#include "bsp/native_session_message_participant_owner.hpp"
#include "bsp/sound_cc13.hpp"
#include "bsp/sound_resource_cache.hpp"
#include "bsp/oct18_tail_c.hpp"
#include "bsp/sound_resource_asset.hpp"
#include "bsp/scene_weather_cc13.hpp"
#include "bsp/scoring_bodies.hpp"
#include "bsp/cc13_seg67b.hpp"
#include "bsp/cc13_seg67n.hpp"
#include "bsp/unit_weapons.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/mmod_hull_convex_box.hpp"
#include "bsp/structured_model.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/ai_planner_tails.hpp"
#include "bsp/ai_tuning_globals.hpp"
#include "bsp/ship_ai_path_corridor.hpp"
#include "bsp/land_and_structures.hpp"
#include "bsp/air_operations.hpp"
#include "bsp/ship_buoyancy_elements.hpp"
#include "bsp/ship_hydro_forces.hpp"
#include "bsp/submarine_model.hpp"
#include "bsp/plane_flight.hpp"
#include "bsp/pilot_controls.hpp"
#include "bsp/cruise_speed_setting.hpp"
#include "bsp/director_update_arms.hpp"
#include "bsp/plane_squadron.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/plane_rack_census_admission.hpp"
#include "bsp/rocket_task_cc13.hpp"
#include "bsp/game_ai_candidate_resource.hpp"
#include "bsp/app_bootstrap.hpp"
#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/avoid_zone_query_binding.hpp"
#include "bsp/award_grant.hpp"
#include "bsp/entity_event_queues.hpp"
#include "bsp/entity_lifecycle_tails.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/blocking_screen.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/archive_text_writer.hpp"
#include "bsp/gun_aiming.hpp"
#include "bsp/gunnery_tables.hpp"
#include "bsp/gun_platform_arc.hpp"
#include "bsp/gameplay_seg52_cc13.hpp"
#include "bsp/gameplay_seg46_cc13.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/gun_bot_remainder.hpp"
#include "bsp/gun_bot_ticks.hpp"
#include "bsp/game_tuning_singleton.hpp"
#include "bsp/cc13_gui1.hpp"
#include "bsp/gui_icon.hpp"
#include "bsp/gui_layer.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_render_order.hpp"
#include "bsp/gui_timed_entry_types.hpp"
#include "bsp/mission_state_frame.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/gui_widget_scene.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/frontend_entry.hpp"
#include "bsp/frontend_screen_animation.hpp"
#include "bsp/frontend_scroller_bridge.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/native_input_action_contexts.hpp"
#include "bsp/seg10_cc13.hpp"
#include "bsp/sound_cc13.hpp"
#include "bsp/hud_screens.hpp"
#include "bsp/hud_minimap.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/ingame_interface.hpp"
#include "bsp/hud_root_rows.hpp"
#include "bsp/game_render_frame.hpp"
#include "bsp/lua_binding_entity_lookup.hpp"
#include "bsp/mission_lobby_settings.hpp"
#include "bsp/attack_commands.hpp"
#include "bsp/command_execution.hpp"
#include "bsp/dyn_lcp_impulse_math.hpp"
#include "bsp/math.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/spatial_index.hpp"
#include "bsp/title_init.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/weapon_director.hpp"
#include "bsp/world_deferred_destroy.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_device_registry.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/renderer_startup.hpp"
#include "bsp/scene_contents_hosts.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_numeric_property_merge.hpp"
#include "bsp/native_property_library_insert.hpp"
#include "bsp/native_property_library_readers.hpp"
#include "bsp/native_property_tree_library_storage.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_scalar_delete.hpp"
#include "bsp/native_enum_symbol_insertion.hpp"
#include "bsp/scene_unit_creators.hpp"
#include "bsp/unit_controller.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_timers.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/input_settings.hpp"
#include "bsp/loading_screen_elements.hpp"
#include "bsp/main_menu_screen.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/press_start_screen.hpp"
#include "bsp/native_gui_text_failure_origin.hpp"
#include "bsp/profile_unlock.hpp"
#include "bsp/session_message_dispatch.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/world_construct.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/powerup_seg58_argon.hpp"
#include "bsp/powerup_config.hpp"
#include "bsp/mission_events.hpp"
#include "bsp/mission_result.hpp"
#include "bsp/mission_load_hosts.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_named_call_args.hpp"
#include "bsp/mission_tree_data.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_effects_startup.hpp"
#include <stdexcept>
#include "bsp/vehicle_class.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/plane_class_fields.hpp"
#include "bsp/vehicle_class_fields.hpp"
#include "bsp/vehicle_class_lua_load.hpp"
#include "bsp/scene_property_bag.hpp"
#include "bsp/scene_traffic_groups.hpp"
#include "bsp/entity_identity.hpp"
#include "bsp/scene_property_bag_merge.hpp"
#include "bsp/scene_record_map.hpp"
#include "bsp/entity_think_dispatch.hpp"
#include "bsp/unit_damage.hpp"
#include "bsp/unit_damage_host_unreached.hpp"
#include "bsp/unit_hit_path.hpp"
#include "bsp/unit_parts.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/recon_slot_lists.hpp"
#include "bsp/sensor_tables.hpp"
#include "bsp/blast_damage.hpp"
#include "bsp/bot_fire_target.hpp"
#include "bsp/collision_shapes.hpp"
#include "bsp/ship_ai_ring_scan.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/pose_derived.hpp"
#include "bsp/torpedo_aim_tick.hpp"
#include "bsp/torpedo_issue_timing.hpp"
#include <algorithm>
#include <cstdlib>
#include <new>
#include <cmath>
#include <memory>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstddef>
#include "bsp/render_fx_cc13.hpp"
#include "bsp/render_sprites_cc13.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool equal(const bsp::Vec3d& a, const bsp::Vec3d& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// A release that repeats the wrong size puts the block on the wrong free list
// of the native pool, so the sizes 0041dd40 passes are the risk worth pinning.
class RecordingStorage final : public bsp::NativeStringStorage {
public:
    char* allocate(std::uint32_t size) override {
        allocated.push_back(size);
        return new char[size];
    }
    void release(char* block, std::uint32_t size) noexcept override {
        released.push_back(size);
        delete[] block;
    }
    std::vector<std::uint32_t> allocated;
    std::vector<std::uint32_t> released;
};

// 004e5540 scans seven switches in a fixed order and the later ones undo the
// earlier ones, so the selected state is the part worth pinning: noskipLogos
// has to cancel skipLogos, and .scn has to win over both.
class StartupSystems final : public bsp::GameStartupSystems {
public:
    explicit StartupSystems(const char* line) : line_(line) {}
    void install_startup_callback() override {}
    void create_startup_controller() override {}
    void register_startup_handler() override {}
    bool logo_sequence_forced() override { return false; }
    const char* command_line() override { return line_; }
    void on_init_once(bool first_time) override { once_first_time = first_time; }
    void on_init_title() override {}
    void notify_title_ready() override {}
    void on_init_mission() override { mission_initialized = true; }
    void drain_state_requests() override {}
    void enqueue_state_request(bsp::GameStartupState state) override { queued = state; }
    void create_logo_sequence() override { logo_built = true; }
    bool once_first_time{false};
    bool mission_initialized{false};
    bool logo_built{false};
    bsp::GameStartupState queued{};

private:
    const char* line_;
};

// Records the call order of 0073bae0's GUI half so the deferred visibility call
// and the double store of MousePtrFE_Icon cannot silently drift.
class GuiStartupRecorder final : public bsp::GuiStartupHost {
public:
    std::string language_font_path() override { return {}; }
    bsp::FontRegistry& font_registry() override { return registry_; }
    bool load_font_descriptors(bsp::FontRegistry&, std::string_view, std::string_view,
        std::string_view) override
    {
        return true;
    }
    void preload_fallback_glyph_table() override {}
    void* gui_manager() override { return &registry_; }
    void* create_gui_resource(void*, void* parent,
        const bsp::GuiManagerResource& entry) override
    {
        std::string line = "create ";
        line += entry.name;
        line += " parent=";
        line += parent ? name_of(parent) : "none";
        transcript.push_back(line);
        names_.emplace_back(new std::string(entry.name));
        return names_.back().get();
    }
    void store_gui_resource(void*, std::uint16_t offset, void*) override
    {
        transcript.push_back("store " + std::to_string(offset));
    }
    void set_gui_resource_visibility(void* resource, bool visible) override
    {
        transcript.push_back("visible " + name_of(resource) + (visible ? " 1" : " 0"));
    }
    void clear_gui_manager_ready_flag(void*) override
    {
        transcript.push_back("clear ready flag");
    }
    std::vector<std::string> transcript;

private:
    static std::string name_of(void* handle) { return *static_cast<std::string*>(handle); }
    bsp::FontRegistry registry_;
    std::vector<std::unique_ptr<std::string>> names_;
};
}

int main() {
    {
        // Chain707: the alpha dispatch domain consumes the retained interval
        // and countdown, and reloads entry+08 between current54/current4C.
        bsp::GuiLayoutWidget first, second;
        first.color[3] = 1.0f;
        second.color[3] = 0.5f;
        const volatile float one = 1.0f, zero = -0.0f;
        bsp::GuiTimedEntryStorage entry;
        bsp::construct_gui_timed_entry_alpha_00ac2ed0(entry, first, one);
        bsp::cc13_seg49_64_78::GuiHudTickerRange interval{entry.target_0c, entry.rate_10};
        bsp::cc13_seg49_64_78::hud_ticker_set_range_00ac2f20(interval, 0, 0x3f800000u);
        entry.remaining_04 = 1.0f;
        struct AlphaCalls final : bsp::GuiTimedEntryAlphaCalls {
            bsp::GuiTimedEntryStorage& entry;
            bsp::GuiLayoutWidget& first;
            bsp::GuiLayoutWidget& second;
            int reads{}, writes{};
            AlphaCalls(bsp::GuiTimedEntryStorage& e, bsp::GuiLayoutWidget& a, bsp::GuiLayoutWidget& b)
                : entry(e), first(a), second(b) {}
            const float* read_color54(bsp::GuiLayoutWidget& current, float (&)[4]) override {
                check(&current == &first, "alpha reads the current borrowed widget");
                ++reads;
                entry.widget_08 = &second;
                return &current.color[0];
            }
            void set_alpha4c(bsp::GuiLayoutWidget& current, float alpha) override {
                check(&current == &second, "alpha reloads its widget after current54");
                ++writes;
                current.color[3] = alpha;
            }
        } calls(entry, first, second);
        const bsp::GuiTimedEntryConstants constants{one, zero};
        check(bsp::advance_gui_timed_entry_00ad39a0(entry, calls, 0.5f, &first, constants)
            && entry.remaining_04 == 0.5f && calls.reads == 0,
            "alpha retained delay prevents early dispatch");
        check(bsp::advance_gui_timed_entry_00ad39a0(entry, calls, 0.75f, &first, constants)
            && entry.remaining_04 == 0.0f && entry.target_0c == 0.0f && entry.rate_10 == 1.0f
            && first.color[3] == 1.0f && second.color[3] == 0.75f
            && calls.reads == 1 && calls.writes == 1,
            "alpha consumes interval bits and only the delta beyond its delay");
        bsp::destroy_gui_timed_entry_profile(entry);
        check(entry.profile_00 == bsp::kGuiTimedEntryBaseProfile,
            "alpha retirement restores the base profile");
    }
    {
        // Chain707 review: run the real frame scheduler with two pending-menu
        // drain passes. The final interface pump configures a fresh fade; only
        // the following GUI phase may charge this frame's 1.25 seconds.
        struct GuiFrameHost final : bsp::MissionFrameHost, bsp::GuiTimedEntryAlphaCalls {
            const bsp::MissionFrameState& frame;
            bsp::GuiLayoutWidget widget;
            bsp::GuiTimedEntryStorage entry;
            const volatile float one{1.0f}, zero{-0.0f};
            int interface_pumps{}, menu_polls{}, gui_updates{};
            bool configured{};
            explicit GuiFrameHost(const bsp::MissionFrameState& f) : frame(f) { widget.color[3] = 1.0f; }
            ~GuiFrameHost() override { if (configured) bsp::destroy_gui_timed_entry_profile(entry); }
            void update_interface_only_004c40f0() override {
                ++interface_pumps;
                if (interface_pumps != 3) return;
                bsp::construct_gui_timed_entry_alpha_00ac2ed0(entry, widget, one);
                bsp::cc13_seg49_64_78::GuiHudTickerRange range{entry.target_0c, entry.rate_10};
                bsp::cc13_seg49_64_78::hud_ticker_set_range_00ac2f20(range, 0, 0x3f800000u);
                entry.remaining_04 = 1.0f;
                configured = true;
            }
            bool service_pending_menu_requests_006840f0() override { return menu_polls++ < 2; }
            void update_front_end_screens_004d8620() override {
                check(interface_pumps == 3 && configured && gui_updates == 0,
                    "GUI animation phase follows every interface-request pump");
                ++gui_updates;
                if (!configured) return;
                const bsp::GuiTimedEntryConstants constants{one, zero};
                bsp::advance_gui_timed_entry_00ad39a0(entry, *this, frame.raw_delta, &widget, constants);
            }
            const float* read_color54(bsp::GuiLayoutWidget& current, float (&)[4]) override {
                check(&current == &widget, "GUI frame reads the configured widget");
                return &current.color[0];
            }
            void set_alpha4c(bsp::GuiLayoutWidget& current, float alpha) override {
                check(&current == &widget, "GUI frame publishes to the configured widget");
                current.color[3] = alpha;
            }
            // Unrelated frame services are inert in this suspended-simulation
            // fixture; the actual scheduler still owns drain/GUI call order.
            bool session_counts_mission_start_004b6260() override { return false; }
            void adjust_mission_start_counters_004bcaa0(bool) override {}
            void update_warning_manager_00987590(float) override {}
            int run_input_effect_sets_004e4e6c(float) override { return 0; }
            void record_action_deadlines_004d8cd0() override {}
            void multiplayer_tick_00778560(float) override {}
            void begin_game_profile_block() override {}
            void end_game_profile_block() override {}
            void register_game_block_label() override {}
            void update_in_mission_subsystems_004c40a0() override {}
            void begin_engine_movie_004cce50() override {}
            bsp::SimulationGateBranch pause_gate_branch_004e5153() override {
                return bsp::SimulationGateBranch::kSimulationOnly;
            }
            bool tutorial_hint_step_available() override { return false; }
            void advance_tutorial_hint_0054e440() override {}
            void toggle_pause_menu_004db030() override {}
            bool in_game_interface_active() override { return false; }
            void update_in_game_interface_0068c1f0() override {}
            void hint_tick_cooldowns_0068ec10(float) override {}
            void hint_drain_queued_00692b00() override {}
            void hint_unit_class_00692b60() override {}
            void hint_weapon_006926f0() override {}
            void hint_environment_00692580() override {}
            void hint_zone_first_get_00692fd0() override {}
            void hint_strategic_map_00692960() override {}
            void update_bot_scheduler_00914ef0(float) override {}
            void update_markers_006dc1a0(float) override {}
            void update_entity_manager_00481640(float) override {}
            void update_rain_descriptor_00865ab0() override {}
            void update_ocean_00bbddd0(float) override {}
            void update_decals_00740e10(float) override {}
            void update_00f89b3c_0094c8f0(float) override {}
            void update_power_ups_008eb110() override {}
            void update_effect_manager_00867ee0(float) override {}
            void flush_entity_activations_00903670() override {}
            bool check_mission_completion_004d7ea0() override { return false; }
            void set_particle_clock_time_00b19a10(float) override {}
            void set_foliage_shader_time_00af0450(float) override {}
            void build_foliage_visible_set_00af0c50() override {}
            void apply_gui_visibility_004c6c70() override {}
            bool interface_manager_present() override { return false; }
            void update_interface_music_00685c80(float) override {}
            void update_multiplayer_interface_004d80d0() override {}
            void pump_peer_queues_00776230() override {}
            void apply_sound_requests_00941140() override {}
            void begin_render_profile_block() override {}
            void render_004ca440() override {}
            void end_render_profile_block() override {}
            void finish_render_frame_004ca1f0() override {}
            bool frame_metrics_enabled() override { return false; }
            void submit_frame_metrics_00757ce0(float) override {}
        };
        bsp::MissionFrameState frame;
        frame.state = bsp::kMissionGameState;
        frame.raw_delta = 1.25f;
        frame.mission_end_suspended = true;
        frame.cinematic_hidden = true;
        GuiFrameHost host(frame);
        const auto result = bsp::run_mission_frame(frame, host);
        check(result.menu_drain_iterations == 2 && host.interface_pumps == 3 && host.gui_updates == 1
            && host.entry.remaining_04 == 0.0f && host.widget.color[3] == 0.75f,
            "repeated interface pumps charge one GUI delta and advance the new fade in the same frame");
    }
    {
        // Chain683: raw native readers borrow the retained slot directly.
        // A stale host cache must not replace the live class+7C/+80 fields.
        bsp::PowerUpSlot slot;
        bsp::PowerupClassConfig cls{};
        slot.source_offset = 91.0f;
        struct Clock final : bsp::powerup_seg58_argon::PowerupSlotClockHost {
            float now{20.0f};
            float& global_f32(std::uint32_t address) override {
                check(address == 0x00f876a4u, "chain683 reads the mission clock");
                return now;
            }
        } clock;
        slot.absolute_start = 3.0f;
        slot.absolute_expiry = 30.0f;
        const auto reads = [&] {
            return std::array<float, 2>{
                bsp::powerup_seg58_argon::powerup_slot_record_get_ready_time_008e4b60(clock, &slot),
                bsp::powerup_seg58_argon::powerup_slot_record_get_end_time_008e4b80(clock, &slot)};
        };
        check(reads() == std::array<float, 2>{3.0f, 30.0f},
            "chain683 native readers borrow the retained absolute times");
        slot.source = &cls;
        slot.relative_deadline = true;
        cls.duration_7c = 4.0f;
        cls.cooldown_80 = 8.0f;
        check(reads() == std::array<float, 2>{24.0f, 28.0f} &&
            bsp::powerup_seg58_argon::powerup_slot_record_is_cooldown_expired_008e4b40(clock, &slot) == 0u,
            "chain683 deferred readers use the current descriptor and clock");
        cls.cooldown_80 = 9.0f;
        check(reads()[1] == 29.0f,
            "chain683 borrowed descriptor updates remain visible without a projection refresh");
    }
    {
        // Chain675: both Lua forms write the same profile containers and
        // dedupe across forms. One fixture catches repeated/partial appends.
        std::vector<std::string> keys;
        std::vector<bsp::ScoringBonusRecord> records;
        bsp::ScoringBonusProfile profile{keys, records};
        bsp::ScoringGrantBonusArguments bonus;
        bonus.key = "text";
        bonus.text_a = "heading";
        bonus.text_b = "detail";
        bonus.text_c = "caption";
        bonus.third_is_string = true;
        bsp::scoring_binding_grant_bonus_008bb770(profile, bonus);
        bonus.key = "TEXT";
        bonus.third_is_string = false;
        bonus.value = 17;
        bsp::scoring_binding_grant_bonus_008bb770(profile, bonus);
        bonus.key = "value";
        bsp::scoring_binding_grant_bonus_008bb770(profile, bonus);
        bonus.key = "VALUE";
        bonus.third_is_string = true;
        bsp::scoring_binding_grant_bonus_008bb770(profile, bonus);
        check(keys == std::vector<std::string>{"text", "value"} && records.size() == 2 &&
            records[0].text_00 == "heading" && records[0].text_08 == "detail" &&
            records[0].text_10 == "caption" && records[0].value_18 == 0 &&
            records[1].text_00 == "heading" && records[1].text_08 == "detail" &&
            records[1].text_10.empty() && records[1].value_18 == 17,
            "chain675 profile bonus forms share live containers and cross-form dedupe");
    }
    {
        // Chain670 review1429: the task's projected ref stays at constructor
        // zeros while the host's retained ref is picked. Exercise that exact
        // separation through the real pick -> release getter path, then keep
        // the borrowed input across repick, retarget reconstruction and freeze.
        struct Random final : bsp::RocketRandom {
            float uniform_00bd2f10(const char*, float low, float) override { return low; }
        } random;
        bsp::RocketTask task;
        task.ap.has_rocket_d4 = true;
        task.ap.rocket.launch_speed = 100.0f;
        task.ap.rocket.vmax_d8 = 200.0f;
        task.ap.rocket.accel_dc = 10.0f;
        task.ap.fire_dist_38 = 200.0f;
        task.ap.aim_point = {0.0f, 0.0f, 100.0f};
        bsp::ApproachTargetRefState produced = bsp::approach_target_ref_construct_009fb200(0.0f);
        bsp::RocketAimInputs inputs{produced};
        inputs.local_point = {0.0f, 0.0f, 100.0f};
        inputs.row.fire_angle_11c = 1.0f;
        const bsp::LeadAimHullExtents hull{100.0f, 20.0f, 10.0f};
        bsp::ShipLeadRandomDraws draws;
        draws.box_x = 0.25f;
        draws.box_y = 0.5f;
        draws.box_z = 0.25f;
        const auto reaches_release = [&] {
            task.aim.timer_18 = -1.0f;
            const bsp::RocketAimResult result = bsp::rocket_aim_tick_007b5430(
                task, inputs, 0.0f, random);
            return result.release.fire
                && result.release.homing_point[0] == produced.body_offset_28[0]
                && result.release.homing_point[1] == produced.body_offset_28[1]
                && result.release.homing_point[2] == produced.body_offset_28[2];
        };
        produced.bias_34 = {3.0f, 0.0f, -2.0f};
        const auto picked = bsp::approach_target_ref_pick_009fa260(produced, hull, draws, true);
        const bool first_pick = picked != std::array<float, 3>{} && reaches_release();
        produced.bias_34 = {-4.0f, 2.0f, 7.0f};
        produced.dirty_41 = true;
        draws.box_z = -0.25f;
        const auto repicked = bsp::approach_target_ref_pick_009fa260(produced, hull, draws, true);
        const bool repick = repicked != picked && reaches_release();
        // A retarget reconstructs the retained object in place. Its new pick
        // (origin-only target + bias) must replace the previous hull offset.
        produced = bsp::approach_target_ref_construct_009fb200(0.0f);
        produced.bias_34 = {8.0f, -3.0f, 5.0f};
        bsp::approach_target_ref_pick_009fa260(produced, {}, {}, false);
        const bool retarget = produced.body_offset_28 == produced.bias_34 && reaches_release();
        const auto before_freeze = produced.body_offset_28;
        bsp::CameraMatrix matrix{};
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
        bsp::approach_target_ref_freeze_009fadae(produced, matrix);
        const bool freeze = produced.frozen && !produced.tracking
            && produced.body_offset_28 == before_freeze && reaches_release();
        check(first_pick && repick && retarget && freeze
                && task.ap.ref.body_offset_28 == std::array<float, 3>{},
            "chain670 rocket release reads nonzero produced offset across repick, retarget and freeze");
    }
    {
        // Chain474 review970: the same retained coordinator must switch both
        // its selected mode and authored values as Capture planners appear
        // and disappear. These are fixture states, not a gameplay probe.
        bsp::AiTuningAuthoredReader reader;
        std::array<bsp::AiTuningBlock, bsp::kAiTuningModeCount> records{};
        for (int i = 0; i < bsp::kAiTuningModeCount; ++i)
            bsp::ai_tuning_load_00a335d0(reader, static_cast<bsp::AiTuningMode>(i), records[i]);
        bsp::AiTuningBlock selected;
        std::array<bool, 8> capture_planners{};
        bsp::ai_tuning_refresh_current(selected, records, 0, capture_planners);
        const bool no_brain = selected.mode == bsp::AiTuningMode::IslandCaptureRegular &&
            selected.at(bsp::kAiTuningCautionMoveDist) == 8000.0f;
        capture_planners[4] = true;
        bsp::ai_tuning_refresh_current(selected, records, 0, capture_planners);
        const bool created = selected.mode == bsp::AiTuningMode::IslandCaptureRookie &&
            selected.at(bsp::kAiTuningCautionMoveDist) == 40000.0f;
        capture_planners[4] = false;
        bsp::ai_tuning_refresh_current(selected, records, 0, capture_planners);
        const bool removed = selected.mode == bsp::AiTuningMode::IslandCaptureRegular &&
            selected.at(bsp::kAiTuningCautionMoveDist) == 8000.0f;
        bsp::ai_tuning_refresh_current(selected, records, 4, capture_planners);
        const bool mode_changed = selected.mode == bsp::AiTuningMode::Duel;
        bsp::ai_tuning_refresh_current(selected, records, 0, capture_planners);
        check(no_brain && created && removed && mode_changed &&
            selected.mode == bsp::AiTuningMode::IslandCaptureRegular,
            "chain474 tuning values and mode follow brain creation/removal and live mode changes");
    }
    {
        // Chain468: the constructor must read the live position AFTER the
        // dirty-pose callback, and a null entity must leave its point alone.
        struct TargetPose final : bsp::MoveToEntityPoseHost {
            std::uint8_t valid{0};
            float position[3]{1.0f, 2.0f, 3.0f};
            int refreshes{0};
            bsp::MoveToEntityPoseView move_to_entity_pose(void*) override {
                return {valid, position};
            }
            void entity_pose_refresh_world_00414db0(void*) override {
                ++refreshes;
                position[0] = 4.0f;
                valid = 1;
            }
        } host;
        bsp::MoveToEntityTargetStorage target{};
        bsp::move_to_target_construct_from_entity_009be0f0(host, &target, &host);
        check(target.profile_00 == 0x00d209c4u && target.initial_world_04[0] == 4.0f
            && target.initial_world_04[1] == 2.0f && target.initial_world_04[2] == 3.0f
            && bsp::move_to_target_entity_009be140(&target) == &host && host.refreshes == 1,
            "chain468 entity target borrows identity and copies the refreshed live pose");
        bsp::move_to_target_construct_from_entity_009be0f0(host, &target, &host);
        const std::uint32_t signalling_nan = 0x7fa12345u;
        std::memcpy(&host.position[1], &signalling_nan, sizeof(signalling_nan));
        bsp::move_to_target_construct_from_entity_009be0f0(host, &target, &host);
        std::uint32_t copied_nan;
        std::memcpy(&copied_nan, &target.initial_world_04[1], sizeof(copied_nan));
        check(copied_nan == 0x7fe12345u,
            "chain468 native FLD/FSTP copy quiets signalling NaN without losing payload");
        bsp::move_to_target_construct_from_entity_009be0f0(host, &target, nullptr);
        check(host.refreshes == 1 && target.entity_10 == nullptr
            && target.initial_world_04[0] == 4.0f,
            "chain468 valid/null target paths preserve refresh and point semantics");

        // Review961: reissue between periodic refreshes of an installed task.
        // The new entity is far below the cruise/path altitude. It must not
        // become the steering height while the existing 0.4s timer is pending.
        host.position[1] = -200.0f;
        bsp::move_to_target_construct_from_entity_009be0f0(host, &target, &host);
        float steer_point[3]{100.0f, 1200.0f, 200.0f};
        bool retarget_pending = false;
        bsp::stage_move_to_entity_point(target, steer_point, retarget_pending);
        check(steer_point[1] == 1200.0f && retarget_pending,
            "chain468 retarget preserves the active cruise/path height before refresh");
        int approach_refreshes = 0;
        const auto refresh_approach = [&] {
            ++approach_refreshes;
            steer_point[1] = 1250.0f; // fixture's newly resolved cruise/path height
        };
        const float remaining = 0.4f, elapsed = 0.1f;
        bsp::refresh_move_to_target_before_steering(!(elapsed < remaining),
            retarget_pending, refresh_approach);
        check(approach_refreshes == 1 && !retarget_pending && steer_point[1] == 1250.0f,
            "chain468 reissued target refreshes before steering despite a positive periodic timer");
        bsp::stage_move_to_entity_point(target, steer_point, retarget_pending);
        bsp::refresh_move_to_target_before_steering(true, retarget_pending, refresh_approach);
        check(approach_refreshes == 2 && steer_point[1] == 1250.0f,
            "chain468 simultaneous retarget and periodic requests refresh only once");
    }
    {
        // Chain413 review834: the first plane aliases the fused squadron
        // index. Its saved and pending weak references still die with that
        // plane, while the controlled squadron outlives it. Reopening must
        // clear +1Ch without adding even a null-payload restore request.
        struct ClosedRowsFixture final : bsp::HudClosedRowsHost {
            std::vector<std::size_t> members{0, 1};
            std::vector<std::pair<int, bsp::HudRootPayload>> pushes;
            std::uint32_t controlled_unit() override { return 1; }
            std::uint32_t first_member(std::uint32_t) override {
                return static_cast<std::uint32_t>(members.front() + 1);
            }
            bool unit_is_kind(std::uint32_t unit, int kind) override {
                return unit != 0 && kind == 0x10;
            }
            bool unit_flag_379(std::uint32_t) override { return false; }
            int unit_owner_1a8(std::uint32_t) override { return 9; }
            int local_team() override { return 0; }
            void push_interface_request(int id, std::uint32_t ref) override {
                pushes.push_back({id, {static_cast<std::size_t>(ref - 1),
                    bsp::HudRootPayloadKind::Plane}});
            }
        } host;
        bsp::HudRootPayload controlled{0, bsp::HudRootPayloadKind::Squadron};
        auto saved = controlled.plane_member(host.members.front());
        bsp::HudClosedRowsState state{0, 0x22, static_cast<std::uint32_t>(saved.unit + 1)};
        bsp::hud_root_toggle_closed_00647080(state, host);
        check(state.closed == 1 && host.pushes.size() == 1 && host.pushes.front().first == 0x23
            && state.interface_id == 0x22 && saved.kind == bsp::HudRootPayloadKind::Plane,
            "chain413 close saves a plane and freezes the original interface");

        // Deliver the fixture's plane-0 death and compacted member snapshot.
        const bool removed = host.members.front() == 0;
        host.members.erase(host.members.begin());
        const bool survivors = !host.members.empty();
        const bool keeps_original_plane = saved.plane_member(host.members.front()).unit == 0;
        const bool pending_released = host.pushes.front().second.release_if_destroyed(true, survivors);
        if (saved.release_if_destroyed(true, survivors)) state.restore_payload = 0;
        check(removed && host.members.size() == 1 && pending_released
            && keeps_original_plane && state.restore_payload == 0
            && !controlled.release_if_destroyed(true, survivors),
            "chain413 death releases saved/pending planes while retaining the actual squadron");
        bsp::hud_root_toggle_closed_00647080(state, host);
        check(state.closed == 0 && host.pushes.size() == 1,
            "chain413 reopen after original-member death queues no restore request");
    }
    {
        // Chain357: a scaled, rotated, translated authored path must choose
        // in LOCAL XYZ. Its first point wins the old world-XZ-only search,
        // while the second is closer under the image's local 3-D metric.
        namespace path = bsp::gameplay_seg46_cc13;
        const bsp::CameraMatrix world{0, 0, -8, 0, 0, 2, 0, 0,
            1, 0, 0, 0, 10, 3, 20, 1};
        bsp::CameraMatrix inverse;
        bsp::derive_pose_affine_inverse_00b63d50(inverse, world);
        const float query[4]{10, 3, 20, 1};
        float homogeneous[4];
        bsp::transform_native_vector4_00b62d10(query, homogeneous, inverse.data());
        const auto local = path::path_local_from_homogeneous_007b0900(homogeneous);
        const path::Vec3 points[]{{0, 8, 0}, {2, 0, 0}};
        float distance = -1.0f;
        check(path::nearest_point_index_007aed10(points, 2, local, &distance) == 1
            && distance == 2.0f, "chain357 searches authored local XYZ through the owner inverse");

        // With one final float rounding these distances tie (44A07779).
        // PC24 per operation gives the second point 44A07778 instead.
        const path::Vec3 near_tie[]{{1283.7335205078125f, 0, 0},
            {-166.87620544433594f, 977.4307250976562f, -815.3239135742188f}};
        check(path::nearest_point_index_007aed10(near_tie, 2, {0, 0, 0}, &distance) == 1
            && distance == 1283.7333984375f, "chain357 PC24 rounding decides the near-tie winner");
    }
    {
        // Chain324 review635: installed arcade Rikufu (VehicleClass[312])
        // keeps two racks at keys 10/11 when equipment 2 substitutes rockets.
        // Unchanged inventory length must not admit those stale bomb counts.
        using bsp::game::SingleRackCensusIdentity;
        using bsp::game::admits_single_rack_round_census;
        for (const int key : {10, 11}) {
            const SingleRackCensusIdentity bomb{key, 89, 0x2A};
            const SingleRackCensusIdentity rocket{key, 416, 0x33};
            check(admits_single_rack_round_census(bomb, bomb, 1)
                && !admits_single_rack_round_census(bomb, rocket, 1),
                "chain324 Rikufu equipment 1 admitted, equipment 2 rejected");
            check(!admits_single_rack_round_census(bomb, {key, 416, 0x2A}, 1)
                && !admits_single_rack_round_census(bomb, {key + 1, 89, 0x2A}, 1)
                && !admits_single_rack_round_census(rocket, rocket, 1)
                && !admits_single_rack_round_census({key, 89, 0x2B}, {key, 89, 0x2B}, 1)
                && !admits_single_rack_round_census({key, 89, 0}, {key, 89, 0}, 1),
                "chain324 mismatched identities and unsupported resolved kinds use fallback");
        }
    }
    {
        // Chain308 review593: a group can contain a logical squadron ID beyond
        // units.count(). Its +304h belongs to the creation row, never whichever
        // wing 007ED610 promotes into slot zero. Distinct nonzero values make
        // both a direct logical-ID lookup and a leader-proxy lookup fail here.
        std::vector<bsp::game::GameUnitRow> rows(3);
        rows[0].resource_usage_304 = 7.5f;
        rows[1].resource_usage_304 = 37.5f;
        rows[2].resource_usage_304 = 900.0f;
        bsp::PlaneSquadronHostRecord canonical;
        canonical.squadron_unit = 1;
        canonical.member_units = {1, 2};
        const bsp::game::AiSquadronResourceOwner owner{canonical.squadron_unit};
        const auto group_inputs = [&] {
            std::vector<float> inputs;
            for (const std::size_t candidate : {std::size_t{0}, rows.size()}) {
                inputs.push_back(bsp::game::ai_candidate_resource_usage(candidate,
                    rows.size(), candidate >= rows.size() ? &owner : nullptr,
                    [&](std::size_t index) -> const bsp::game::GameUnitRow* {
                        return index < rows.size() ? &rows[index] : nullptr;
                    }));
            }
            return inputs;
        };
        const auto before = group_inputs();
        check(before[0] == 7.5f && before[1] == 37.5f && before[0] + before[1] == 45.0f,
            "AI group resource inputs resolve a nonzero logical squadron and an ordinary unit");
        // The two-member post-007ED610 state: the more expensive wing is
        // now slot zero. This fixture exercises ownership resolution, not the
        // separate promotion/reindex transport or its game-only providers.
        std::swap(canonical.member_units[0], canonical.member_units[1]);
        check(canonical.member_units[0] == 2,
            "AI resource fixture makes the differently priced wing the flight leader");
        check(group_inputs() == before,
            "AI squadron ResourceUsage stays on its owning creation row after leader promotion");
    }
    {
        // A malformed second BrowserGroup must unwind both raw8 strings, keep
        // the completed first entry, and leave the mismatched token unread.
        struct BrowserGroupsObserver final : bsp::SceneBrowserGroupsHost {
            int observed{};
            void observe_browser_group(const bsp::SceneBrowserGroupEntry&) override { ++observed; }
        } host;
        bsp::SceneLexer lexer("SceneBrowserGroups { "
            "BrowserGroup { GroupName = \"two words\" GroupID = -7 Parent = \"\" } "
            "BrowserGroup { GroupName = second GroupID = 9 Parent = root Wrong } }");
        const auto result = bsp::read_browser_groups_block(lexer, host);
        check(!result.ok && result.entries.size() == 1 && host.observed == 1
                && result.entries[0].group_name == "two words"
                && result.entries[0].group_id == -7 && result.entries[0].parent.empty()
                && lexer.peek().text == "Wrong" && result.error.find("expected }") != std::string::npos,
            "BrowserGroup native entry adapter retains completed entries and stops at a malformed closing token");
    }
    {
        // 008F6AD7 must apply a redeclaration to its existing bag, preserving
        // every parent for 008F5AB3. The enum adapter retokenizes quoted names;
        // 008F31A0 treats an opening brace as a symbol, not a nested block.
        struct LibraryFixture final : bsp::PropLibraryHost {
            std::vector<bsp::PropLibraryGroup> applied;
            std::vector<std::string> symbols;
            std::vector<std::string> published_tables;
            std::vector<std::string> enumerate_library_files(
                const std::string&, const std::string&) override { return {}; }
            bool read_library_file(const std::string&, std::string& text) override {
                text = "enum Words { \"two words\" = 7; } "
                    "enum Keys { \"{\" = 7; } "
                    "properties Pair (BaseA BaseB) { X = I 1; } "
                    "properties Pair { X = I 2; }";
                return true;
            }
            bool has_group(const std::string& name) override {
                for (const auto& group : applied) if (group.name == name) return true;
                return false;
            }
            bool has_enum_table(const std::string&) override { return false; }
            void register_group(const bsp::PropLibraryGroup& group) override {
                applied.push_back(group);
            }
            void register_enum_table(const bsp::PropLibraryEnumTable& table) override {
                published_tables.push_back(table.name);
            }
            void parse_enum_body(const std::string&, const std::string& body) override {
                bsp::SceneLexer lexer(body, bsp::kSceneDelimiters);
                symbols.push_back(lexer.next().text);
            }
        } host;
        bsp::PropLibraryDocument document;
        std::string error;
        const bool loaded = bsp::load_prop_library_file("fixture.props", host, document, error);
        const auto* value = host.applied.size() == 2 ? host.applied[1].body.find("X") : nullptr;
        check(loaded && error.empty() && host.applied.size() == 2
                && host.applied[0].created && !host.applied[1].created
                && host.applied[0].parents == std::vector<std::string>({"BaseA", "BaseB"})
                && value != nullptr && value->has_integer && value->integer_value == 2
                && host.symbols == std::vector<std::string>({"two words", "{"})
                && host.published_tables == std::vector<std::string>({"Words", "Keys"}),
            "property-library load applies repeated groups and all parents, and publishes quoted enum names including an opening brace");
    }
    {
        // 0084C5D4 must tick once, then read the live +4D4h flag. Pin the
        // ungated/untargeted extra send and a synchronous 0084C4E0 clear.
        struct FireSyncRecorder final : bsp::GunFireRequestHost {
            bool request{true}, blocked{true}, clear_on_send{false};
            bool suppressed{false};
            mutable int request_checks{0};
            int ticks{0};
            std::vector<std::uint16_t> targets;
            bool has_owning_unit() const override { ++request_checks; return true; }
            bool unit_suppressed() const override { return suppressed; }
            bool gun_suppressed() const override { return false; }
            void release_fire_target_ref_006952a0() override {}
            bool is_rapid_fixed_slave_006e3d50(int) override { return false; }
            void send_fixed_slave_fire_message_0077c7b0(bool) override {}
            float random_stagger_00bd2f10(float lo, float) override { return lo; }
            void stop_firing_0072b4c0() override {}
            void release_effect_ref() override {}
            void base_tick_0072ad40(float) override { ++ticks; }
            void set_barrel_reload_timer_0072cf00(int, float) override {}
            bool unit_fire_blocked() const override { return blocked; }
            bool gun_disabled() const override { return false; }
            void send_fire_message_0ad(std::uint16_t target) override {
                targets.push_back(target);
                if (clear_on_send) {
                    bsp::gameplay_seg52_cc13::turning_gun_fire_if_ready_0084c4e0(
                        true, request);
                }
            }
        } host;
        bsp::GunFireRequestState state;
        state.fire_requested = true;
        state.has_target_ref = true;
        state.target_network_id = 42;
        state.barrel_delay_time = 1.0f;
        const auto tick = [&] {
            return bsp::gameplay_seg52_cc13::turning_gun_extra_fire_request_0084c5b0(
                state, host, host.request, 0.25f);
        };
        const auto blocked = tick();
        check(!blocked.base_message_sent && blocked.extra_message_sent
            && host.targets == std::vector<std::uint16_t>{0}
            && host.ticks == 1 && state.barrel_delay_time == 0.75f,
            "0084C5B0 ticks once and sends an untargeted extra AD despite the base gate");
        host.blocked = false;
        host.targets.clear();
        const auto both = tick();
        check(both.base_message_sent && both.extra_message_sent
            && host.targets == std::vector<std::uint16_t>{42, 0}
            && host.ticks == 2 && state.barrel_delay_time == 0.5f,
            "0084C5B0 keeps the targeted base and untargeted extra messages distinct");
        host.clear_on_send = true;
        host.targets.clear();
        const auto cleared = tick();
        check(cleared.base_message_sent && !cleared.extra_message_sent && !host.request
            && host.targets == std::vector<std::uint16_t>{42}
            && host.ticks == 3 && state.barrel_delay_time == 0.25f,
            "0084C5DB observes the 0084C4E0 latch clear during base-message delivery");

        // 006FDCDB rejects before even querying the base host. In contrast,
        // an admitted true request can be suppressed by the base and must
        // clear +4D4h from the resulting latch, not from its true input.
        host.request = true;
        const int checks_before = host.request_checks;
        const bool admitted = bsp::single_turning_gun_set_fire_request_006fdcd0(
            state, host, host.request, true, false);
        const bool untouched = !admitted && state.fire_requested && host.request
            && host.request_checks == checks_before;
        host.suppressed = true;
        bsp::single_turning_gun_request_immediate_fire_006fdc50(
            state, host, host.request, true);
        const bool single_cleared = !state.fire_requested && !host.request;
        bsp::rapid_turning_gun_request_immediate_fire_006fdf60(host.request);
        const bool rapid_armed = host.request;
        bsp::rapid_turning_gun_set_fire_request_006fdc90(state, host, host.request, true);
        check(untouched && single_cleared && rapid_armed && !host.request
            && !state.fire_requested && host.request_checks == checks_before + 2,
            "turning-gun overrides preserve salvo rejection and clear immediate fire after suppression");
    }
    {
        // Passive metadata fixture only: this ordinary header is never passed
        // to a Native provider, pool or destructor. The host exception is real.
        using Origin = bsp::NativeGuiTextFailureOrigin;
        using Runtime = bsp::NativeGuiTextFailureOriginRuntime;
        Runtime runtime{Runtime::Contract::qualified_own_msvc_14_51_source_catch};
        Origin origin;
        bsp::NativeGuiTextFailureHeaderArm arm;
        std::uint32_t passive_header[2]{};
        origin.begin(runtime, &origin, 0);
        check(!origin.committed() && origin.verdict() == Origin::Verdict::unset,
            "origin metadata starts unset, without a committed denial");
        arm.arm(origin, passive_header, 0);
        struct HostMetadataFault {};
        bool caught = false;
        try {
            origin.host(runtime, {0, 1}, &origin, [] { throw HostMetadataFault{}; });
        } catch (const HostMetadataFault&) {
            caught = true;
            const auto cause = runtime.read_current_catch();
            check(cause.valid(), "origin fixture reads the real active Source catch key");
            check(origin.committed() && origin.verdict() == Origin::Verdict::host
                && origin.requires_retention(),
                "host escape with an armed header commits host metadata and retention");
            check(!arm.consume_proven_primary(origin, cause, passive_header, 0),
                "host escape cannot authorize Native header cleanup");
            origin.retain_current(runtime);
            check(origin.committed() && origin.verdict() == Origin::Verdict::host
                && origin.requires_retention(),
                "unknown retention fallback preserves the committed host denial");
            check(!arm.consume_proven_primary(origin, cause, passive_header, 0),
                "retention fallback cannot promote host metadata to Native cleanup");
        }
        check(caught, "origin fixture propagates the real host exception");
    }
    {
        // One Source-only regression for AD3A80's missing captured-block
        // constructor cleanup. The callback replaces CURRENT backing before
        // throwing; neither the old nor replacement slot may be published.
        struct TimedConstructorFault {};
        struct TimedConstructorHooks final : bsp::cc13_gui1::GuiTimedEntryHooks {
            void* widget;
            void** replacement;
            void* block = nullptr;
            int resizes = 0, frees = 0, returned = 0;
            bool correct_constructor = false, wrong_free = false;
            TimedConstructorHooks(void* w, void** r) : widget(w), replacement(r) {}
            ~TimedConstructorHooks() override { std::free(block); }
            void timed_entry_array_resize_00aa77b0(void*, std::int32_t) override { ++resizes; }
            void* operator_new_00bf681b(std::uint32_t bytes) override {
                check(bytes == 0x14, "timed constructor allocates actual 14h block");
                block = std::malloc(bytes);
                if (!block) throw std::bad_alloc();
                return block;
            }
            void* timed_entry_construct_00ac2ed0(void* b, void* w) override {
                correct_constructor = b == block && w == widget;
                std::memcpy(static_cast<unsigned char*>(w) + 0x88,
                    &replacement, sizeof(replacement));
                throw TimedConstructorFault{};
            }
            void free_failed_constructor_00bf65ac(void* b) noexcept override {
                ++frees;
                if (b == block) { std::free(block); block = nullptr; }
                else wrong_free = true;
            }
            void timed_entry_construction_returned(void*) noexcept override { ++returned; }
        };
        alignas(void*) unsigned char widget[0xec]{};
        void* original[1]{};
        void* replacement[1]{};
        void** current = original;
        const std::int32_t one = 1;
        std::memcpy(widget + 0x88, &current, sizeof(current));
        std::memcpy(widget + 0x8c, &one, sizeof(one));
        std::memcpy(widget + 0x90, &one, sizeof(one));
        TimedConstructorHooks hooks(widget, replacement);
        bool escaped = false;
        try { bsp::cc13_gui1::gui_timed_entry_get_or_create_00aa8b00(hooks, widget, 0); }
        catch (const TimedConstructorFault&) { escaped = true; }
        std::memcpy(&current, widget + 0x88, sizeof(current));
        check(escaped && hooks.correct_constructor && hooks.frees == 1 && !hooks.wrong_free
            && hooks.block == nullptr && hooks.returned == 0 && hooks.resizes == 0
            && current == replacement && original[0] == nullptr && replacement[0] == nullptr,
            "timed ctor escape frees captured block once, propagates and publishes neither slot");
    }
#if defined(_MSC_VER) && defined(_M_IX86) && defined(_DEBUG)
    {
        // One source-only fixture for the two C50390 exception boundaries.
        // The actual C44000/SBO allocator runs; no original FH frame executes.
        struct ConstructorFault { int sequence; };
        struct ParentVectorFault {};
        struct State {
            DWORD thread = GetCurrentThreadId();
            _CRT_ALLOC_HOOK previous_hook = nullptr;
            _PNH previous_handler = nullptr;
            bsp::DynProfileNodeStorage* raw = nullptr;
            void* nested = nullptr;
            void* nested_original = nullptr;
            void* parent_buffer = nullptr;
            int allocations = 0, rejected = 0, handlers = 0;
            std::size_t rejected_bytes[2]{};
            int frees68 = 0, frees65 = 0, event_count = 0, events[4]{};
            bool constructor_fault = true, armed = false;
            bool vector_zeroed = false, current_data = false, no_field_clear = false;
            bool original_allocation = true, growth_request = false;
        } state;
        static State* active = nullptr; // Fixture observations, not a native global.
        const auto hook = +[](int kind, void* data, std::size_t bytes, int block,
                              long request, const unsigned char* file, int line) -> int {
            State* const s = active;
            if (s && s->previous_hook
                    && !s->previous_hook(kind,data,bytes,block,request,file,line)) return FALSE;
            // The real 17-character name requests 32, then fallback 18 bytes.
            if (s && s->thread == GetCurrentThreadId() && s->armed
                    && kind == _HOOK_ALLOC && block == _NORMAL_BLOCK
                    && (bytes == 32 || bytes == 18)) {
                auto* const words = reinterpret_cast<std::uint32_t*>(s->raw->bytes);
                if (s->rejected == 0) {
                    s->vector_zeroed = words[1] == 0 && words[2] == 0 && words[3] == 0;
                    words[1] = reinterpret_cast<std::uint32_t>(s->nested);
                    words[2] = 7; words[3] = 9; // Real preallocated 9-slot storage.
                }
                s->rejected_bytes[s->rejected] = bytes;
                ++s->rejected;
                if (s->rejected == 2) s->armed = false;
                return FALSE;
            }
            return TRUE;
        };
        const auto no_retry = +[](std::size_t) -> int {
            throw ConstructorFault{++active->handlers};
        };
        const bsp::AvoidZoneDynHullMemory memory{&state,
            [](void* context, std::size_t bytes) -> void* {
                auto& s = *static_cast<State*>(context);
                if (++s.allocations == 2 && !s.constructor_fault) {
                    s.growth_request = bytes == 16;
                    throw ParentVectorFault{};
                }
                void* const p = std::malloc(bytes);
                if (!p) throw std::bad_alloc();
                if (s.allocations == 1) {
                    s.raw = static_cast<bsp::DynProfileNodeStorage*>(p);
                    s.armed = s.constructor_fault;
                } else s.parent_buffer = p;
                return p;
            },
            [](void* context, void* p) {
                auto& s = *static_cast<State*>(context);
                ++s.frees68;
                if (s.event_count < 4) s.events[s.event_count] = 68;
                ++s.event_count;
                if (p == s.nested_original && s.raw) {
                    auto* const words = reinterpret_cast<std::uint32_t*>(s.raw->bytes);
                    s.current_data = words[1] == reinterpret_cast<std::uint32_t>(p)
                        && words[2] == 7 && words[3] == 9;
                }
                if (p == s.nested) s.nested = nullptr;
                if (p == s.raw) s.raw = nullptr;
                if (p == s.parent_buffer) s.parent_buffer = nullptr;
                std::free(p);
            }};
        const bsp::DynProfilePartialNodeDelete delete65 = +[](void* p) {
            auto& s = *active;
            ++s.frees65;
            if (s.event_count < 4) s.events[s.event_count] = 65;
            ++s.event_count;
            s.original_allocation = s.original_allocation && p == s.raw;
            auto* const words = reinterpret_cast<std::uint32_t*>(p);
            s.no_field_clear = words[1] == reinterpret_cast<std::uint32_t>(s.nested_original)
                && words[2] == 7 && words[3] == 9;
            if (p == s.raw) s.raw = nullptr;
            std::free(p);
        };
        struct RestoreHooks {
            _CRT_ALLOC_HOOK hook;
            _PNH handler;
            State* observations;
            ~RestoreHooks() {
                _CrtSetAllocHook(hook);
                _set_new_handler(handler);
                active = observations;
            }
        };

        bsp::DynProfileNodeStorage parent{};
        bsp::dyn_profile_node_construct_00c44000(parent,"Parent",0);
        alignas(4) unsigned char before[sizeof(parent)];
        std::memcpy(before,parent.bytes,sizeof(parent));
        state.nested = std::calloc(9,4);
        state.nested_original = state.nested;
        check(state.nested != nullptr,"profile unwind fixture owns nested vector storage");
        if (!state.nested) throw std::bad_alloc();
        bool constructor_threw = false;
        int caught_sequence = 0;
        {
            State* const previous_observations = active;
            active = &state;
            RestoreHooks restore{
                _CrtSetAllocHook(hook),_set_new_handler(no_retry),previous_observations};
            state.previous_hook = restore.hook;
            state.previous_handler = restore.handler;
            try {
                bsp::dyn_profile_node_append_child_00c50390(
                    parent,"abcdefghijklmnopq",0x71,memory,delete65);
            } catch (const ConstructorFault& failure) {
                constructor_threw = true; caught_sequence = failure.sequence;
            }
        }
        const bool hook_restored = _CrtSetAllocHook(state.previous_hook) == state.previous_hook;
        const bool handler_restored = _set_new_handler(state.previous_handler) == state.previous_handler;
        check(constructor_threw && caught_sequence == 2 && state.rejected == 2
                && state.rejected_bytes[0] == 32 && state.rejected_bytes[1] == 18
                && state.vector_zeroed && state.current_data && state.no_field_clear
                && state.frees68 == 1 && state.frees65 == 1 && state.event_count == 2
                && state.events[0] == 68 && state.events[1] == 65
                && state.original_allocation && state.raw == nullptr && state.nested == nullptr
                && hook_restored && handler_restored && active == nullptr
                && std::memcmp(before,parent.bytes,sizeof(parent)) == 0,
            "real profile ctor failure cleans current child vector68 then raw allocation65 and rethrows");
        // Failure cleanup for a regression, after restoring all CRT hooks.
        if (state.raw) {
            bsp::native_legacy_sbo_string_destroy_004072d0(
                *reinterpret_cast<bsp::NativeLegacySboStringStorage*>(state.raw->bytes+0x10));
            std::free(state.raw);
        }
        std::free(state.nested);
        std::free(state.parent_buffer);

        state = State{};
        state.constructor_fault = false;
        bsp::DynProfileNodeStorage prior{};
        bsp::dyn_profile_node_construct_00c44000(prior,"Prior",0);
        state.parent_buffer = std::malloc(4);
        check(state.parent_buffer != nullptr,"profile vector boundary fixture owns parent storage");
        if (!state.parent_buffer) throw std::bad_alloc();
        *static_cast<bsp::DynProfileNodeStorage**>(state.parent_buffer) = &prior;
        auto* const parent_words = reinterpret_cast<std::uint32_t*>(parent.bytes);
        parent_words[1] = reinterpret_cast<std::uint32_t>(state.parent_buffer);
        parent_words[2] = 1; parent_words[3] = 1;
        std::memcpy(before,parent.bytes,sizeof(parent));
        reinterpret_cast<std::uint32_t*>(before)[3] = 4; // Native committed capacity.
        bool vector_threw = false;
        active = &state;
        try {
            bsp::dyn_profile_node_append_child_00c50390(parent,"Child",0x72,memory,delete65);
        } catch (const ParentVectorFault&) { vector_threw = true; }
        active = nullptr;
        const bool constructed = state.raw
            && *reinterpret_cast<std::uint32_t*>(state.raw->bytes+0x2c) == 0x72;
        check(vector_threw && state.growth_request && constructed
                && state.frees65 == 0 && state.frees68 == 0
                && std::memcmp(before,parent.bytes,sizeof(parent)) == 0,
            "post-ctor parent growth failure retains completed orphan and committed capacity");
        // The fixture owns this intentional native orphan; production does not free it.
        if (state.raw) {
            bsp::native_legacy_sbo_string_destroy_004072d0(
                *reinterpret_cast<bsp::NativeLegacySboStringStorage*>(state.raw->bytes+0x10));
            std::free(state.raw);
        }
        std::free(state.parent_buffer);
    }
#endif
    {
        // 00441780 publishes before registering, but normal manager drain must
        // still recognize CE44DC, pop the owner, and clear the same E17BF4 cell.
        void* volatile manager = nullptr;
        auto& publication = bsp::process_native_device_registry_00e17bf4();
        check(publication == nullptr,
            "the process device registry fixture starts unpublished");
        auto* const first = bsp::get_process_native_device_registry_00441780(manager);
        auto* const second = bsp::get_process_native_device_registry_00441780(manager);
        check(first != nullptr && second == first && publication == first
                && first->native_vtable_00 == bsp::kNativeDeviceRegistryDeletingProfile
                && first->classes_04.data_00 == 0 && first->classes_04.count_04 == 0
                && first->classes_04.capacity_08 == 0
                && bsp::count_native_singleton_slots_00bcf910(manager, nullptr) == 1,
            "00441780 initializes one raw10h CE44DC owner and registers it once");

        bsp::NativeSingletonDeletionBindings bindings{};
        bsp::destroy_native_singleton_manager_00bd0400(manager, bindings);
        check(publication == nullptr,
            "CE44DC manager drain clears the canonical E17BF4 publication");
        bsp::singleton_lifetime_free(manager);

        // The nondeleting scalar path exposes the post-destruction profile and
        // also proves borrowed class DWORDs are not dereferenced or released.
        bsp::NativeDeviceRegistryStorage retained{
            bsp::kNativeDeviceRegistryDeletingProfile, {0, 0, 0}};
        bsp::NativeDeviceRegistryStorage* volatile retained_publication = &retained;
        bsp::resize_native_device_registry_array_00440180(retained.classes_04, 1);
        *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uintptr_t>(retained.classes_04.data_00)) = 1;
        check(bsp::delete_native_device_registry_00441840(
                  &retained, 0, retained_publication) == &retained
                && retained_publication == nullptr
                && retained.native_vtable_00 == bsp::kNativeSingletonBaseProfile
                && retained.classes_04.count_04 == 0,
            "00441840 flags0 retains the owner after unpublish/base-profile teardown");
    }

    using namespace bsp;
    check(abs_00401170(-3.5f) == 3.5f, "absolute value");
    check(!std::signbit(abs_00401170(-0.0f)), "negative zero becomes positive");
    check(std::isinf(abs_00401170(-std::numeric_limits<float>::infinity())), "infinity");
    check(std::isnan(abs_00401170(std::numeric_limits<float>::quiet_NaN())), "quiet NaN");
    Vec3d out{};
    cross_reversed_00401c20(out, {1, 0, 0}, {0, 1, 0});
    check(equal(out, {0, 0, -1}), "cross product handedness is b cross a");
    cross_reversed_00401c20(out, {1, 2, 3}, {4, 5, 6});
    check(equal(out, {3, -6, 3}), "cross product fixture");
    Vec3d alias{1, 2, 3};
    cross_reversed_00401c20(alias, alias, {4, 5, 6});
    check(equal(alias, {3, 6, 9}), "original sequential writes under aliasing");
    subtract_reversed_00401cb0(out, {1, 2, 3}, {4, 8, 12});
    check(equal(out, {3, 6, 9}), "subtraction operand order");
    subtract_reversed_00401cb0(out, out, {4, 8, 12});
    check(equal(out, {1, 2, 3}), "subtraction aliasing");
    scale_00401cd0(out, -2);
    check(equal(out, {-2, -4, -6}), "in-place scaling");
    check(length_squared_00401cf0({3, 4, 12}) == 169, "squared length fixture");
    check(length_squared_00401cf0({0, 0, 0}) == 0, "zero squared length");
    // A double SSE expression returns 16777218 here. Native 00401CF0 must
    // round both additions at the current x87 PC24 precision instead.
    unsigned short saved_length_control, pc24_length_control;
    __asm { fnstcw saved_length_control }
    pc24_length_control = static_cast<unsigned short>(saved_length_control & ~0x0300u);
    __asm { fldcw pc24_length_control }
    const double pc24_squared_length = length_squared_00401cf0({4096, 1, 1});
    __asm { fldcw saved_length_control }
    check(pc24_squared_length == 16777216.0, "squared length honors x87 PC24 additions");

    RecordingStorage storage;
    NativeString text;
    text.assign_0041e870(storage, "abcd");
    check(text.length() == 4 && std::strcmp(text.data(), "abcd") == 0, "assign copies the terminator");
    text.resize_0041dd40(storage, 2, true);
    check(text.length() == 2 && text.data()[0] == 'a' && text.data()[1] == 'b',
        "shrink preserves min(old, new) bytes");
    check(text.data()[2] == '\0', "resize writes the terminator at the new length");
    text.resize_0041dd40(storage, 2, true);
    text.release_to(storage);
    const std::vector<std::uint32_t> expected_allocated{5, 3};
    const std::vector<std::uint32_t> expected_released{5, 3};
    check(storage.allocated == expected_allocated, "each buffer is length + 1 bytes");
    check(storage.released == expected_released, "each release repeats the allocated size");

    // 0073ce20: "auto" advances the token iterator itself, so the two tokens
    // that follow it are consumed and must not be classified on their own.
    const CommandLineOptions parsed =
        parse_command_line_0073ce20("nozip auto mpak classes memlimit");
    check(!parsed.zip_enabled, "nozip clears the zip flag");
    check(parsed.auto_task == AutoTask::package_classes, "auto mpak classes");
    check(parsed.file_access_log, "auto mpak also raises the file access log");
    check(parsed.memory_limit.has_value() && *parsed.memory_limit,
        "the token after the auto group is still parsed");
    check(parsed.unrecognized.empty(), "auto follower tokens are not left over");
    {
        // 00a904e0 routes a device by its own class row and resolves a -1
        // request to the first free column in that row only, so two classes do
        // not share columns and a full row has no free slot.
        InputDeviceTable table;
        struct Slotted : InputDevice {
            int device_class_value;
            int resets = 0;
            explicit Slotted(int value) : device_class_value(value) {}
            int device_class() const override { return device_class_value; }
            void on_slot_reset() override { ++resets; }
        };
        Slotted pad_a{2}, pad_b{2}, keyboard{0};
        std::string error;
        check(table.attach(&pad_a, -1, error) && pad_a.assigned_slot == 0, "first pad takes slot 0");
        check(table.attach(&pad_b, -1, error) && pad_b.assigned_slot == 1, "second pad takes slot 1");
        check(table.attach(&keyboard, -1, error) && keyboard.assigned_slot == 0,
              "keyboard row has its own free column 0");
        check(table.slot(2, 1) == &pad_b, "pad landed in the joystick row");
        table.reset_all();
        check(pad_a.resets == 1 && pad_b.resets == 1 && keyboard.resets == 1,
              "00a900f0 hits every occupied slot once");
        Slotted overflow[8] = {Slotted{1}, Slotted{1}, Slotted{1}, Slotted{1},
                               Slotted{1}, Slotted{1}, Slotted{1}, Slotted{1}};
        for (int i = 0; i < 8; ++i) check(table.attach(&overflow[i], -1, error), "mouse row fills");
        Slotted extra{1};
        check(!table.attach(&extra, -1, error), "a full row reports no free slot");
    }
    {
        // A short delay vector is checked after movie entry and timer storage.
        // A returning CRT handler repairs it; it must not finish the sequence.
        struct RepairLogoDelay final : LogoSequenceHost {
            LogoSequenceState& state;
            const float repaired_delay = 3.0f;
            std::string calls;
            bool failure_saw_progress = false;
            explicit RepairLogoDelay(LogoSequenceState& value) : state(value) {}
            void install_movie_completion_callback() override { calls += 'c'; }
            void movie_play(const char*, int, float, int) override { calls += 'p'; }
            void movie_screen_enter() override { calls += 'e'; }
            ClockTimestamp now() override { calls += 'n'; return {42, 100}; }
            void destroy_self() override { calls += 'd'; }
            void reenter_title() override { calls += 't'; }
            void invalid_parameter_noinfo_00bf6713() override {
                calls += 'f';
                failure_saw_progress = state.next_index == 1
                    && state.entry_started.ticks == 42 && state.delays == nullptr;
                state.delays = &repaired_delay;
                state.delay_count = 1;
            }
        };
        const char* entry = "logo";
        LogoSequenceState logo;
        logo.entries = &entry;
        logo.entry_count = 1;
        RepairLogoDelay repair(logo);
        const auto outcome = logo_advance_or_finish(logo, repair);
        check(outcome == LogoAdvanceOutcome::EntryStarted && repair.calls == "cpenf"
                && repair.failure_saw_progress && logo.entry_delay == 3.0f,
            "logo delay validation preserves prior effects and a returning CRT handler");
    }
    {
        GameStartupFlags flags{};
        StartupSystems plain("game.exe");
        check(game_on_init(plain, flags) == GameStartupState::kLogoSequence
                && plain.logo_built && !plain.once_first_time,
            "the default boot stops at the logo sequence");

        flags = GameStartupFlags{};
        StartupSystems cancelled("game.exe -skipLogos -noskipLogos");
        check(game_on_init(cancelled, flags) == GameStartupState::kLogoSequence
                && !flags.skip_logos,
            "noskipLogos cancels an earlier skipLogos");

        flags = GameStartupFlags{};
        StartupSystems scenario("game.exe -noskipLogos missions/a.scn");
        check(game_on_init(scenario, flags) == GameStartupState::kScenarioLoad
                && scenario.mission_initialized && flags.skip_title && flags.skip_logos
                && scenario.queued == GameStartupState::kScenarioLoad,
            "a .scn path overrides noskipLogos and reaches the mission state");
    }
    {
        // 00aa5e20 stores MousePtrFE_Icon at both +0x54 and +0x50 and issues its
        // visibility call only after MousePtrGUI_Icon's, at 00aa60a3. Nothing in
        // the type system pins that order, so the transcript is the check.
        GuiStartupRecorder host;
        const bsp::GuiStartupResult result = bsp::run_gui_startup(host);
        check(result.stores == 10 && result.resources_created == 10,
            "the GUI resource list makes ten stores for ten resources");
        const std::vector<std::string> expected{
            "create data/interface/textures/whiteGui.tga parent=none",
            "store 40",
            "create interface/textures/common/transparent.tga parent=none",
            "store 44",
            "create _Mouse parent=none",
            "store 76",
            "visible _Mouse 1",
            "create MousePtrFE_Icon parent=_Mouse",
            "store 84",
            "store 80",
            "create MousePtrGUI_Icon parent=_Mouse",
            "store 88",
            "visible MousePtrGUI_Icon 0",
            "visible MousePtrFE_Icon 0",
            "create _Highlight parent=none",
            "visible _Highlight 1",
            "create hl_FrameBox parent=_Highlight",
            "store 116",
            "visible hl_FrameBox 0",
            "create hlCircle_FrameBox parent=_Highlight",
            "store 120",
            "visible hlCircle_FrameBox 0",
            "create safezone_43_FrameBox parent=_Highlight",
            "store 124",
            "visible safezone_43_FrameBox 0",
            "create safezone_169_FrameBox parent=_Highlight",
            "store 128",
            "visible safezone_169_FrameBox 0",
            "clear ready flag",
        };
        check(host.transcript == expected, "the GUI resource list runs in native order");
    }

    {
        // The timed set at game+5BCh decides an edge purely by which of the two
        // injectors runs: 00a92aa0 clears the previous half and 00a919f0 does not.
        // Getting that backwards would make every injected action fire 004c43c0
        // on every frame it is queued, so it is the one rule worth pinning.
        InputActionRecord record{};
        InputActionListener listener{};
        const InputEffectParam none{};
        start_action_00a92aa0(record, 0.25f, none, &listener);
        check(action_pressed_this_frame_004c43c0(record), "a started injection is an edge");
        continue_action_00a919f0(record, 0.25f);
        check(!action_pressed_this_frame_004c43c0(record), "a continued injection is not an edge");
    }

    {
        // 00f889a0 aliases settings.masterVolume, not constant zero. A prior
        // entry callback can change it before the native 004da724 read.
        struct MissionAudioHost final : MissionStateEntryHost {
            AudioSettings& settings;
            float applied{-1.0f};
            std::uint32_t mask{0};
            explicit MissionAudioHost(AudioSettings& audio) : settings(audio) {}
            void enter_scope(const char*) override {}
            void release_deferred_dynamics() override {}
            void mark_local_slot_ready(std::size_t, std::uint16_t) override {
                settings.master_20 = 0.75f;
            }
            void set_audio_environment_level(float level, std::uint32_t bus_mask) override {
                applied = level;
                mask = bus_mask;
            }
            void apply_in_game_interface(bool) override {}
            void check_multiplayer_player_count() override {}
            std::uint32_t game_state() override { return 0x0D; }
            void set_cinematic_mode(bool, bool, bool) override {}
        };
        AudioSettings settings{};
        settings.master_20 = 0.125f;
        MissionStateEntryState state{};
        MissionAudioHost host(settings);
        check(run_mission_state_entry(state, settings, host), "mission entry completes");
        check(host.applied == 0.75f && host.mask == 0xFFFF,
            "mission entry reads current master volume at the native audio call");
    }

    {
        // 004e4430 servicing order: first in first out, a request enqueued by a
        // handler is serviced in the same pass, and request Dh stops the drain
        // with the rest still queued for the next frame.
        struct OrderHost : bsp::GameFrameControlHost {
            std::vector<std::uint32_t> serviced;
            bsp::GameStateRequestQueue* queue = nullptr;
            void copy_local_player_slots(bsp::LocalPlayerSlot (&)[8]) override {}
            float mission_time_filter_007713a0(float delta) override { return delta; }
            bool max_step_clamp_disabled() override { return false; }
            bool input_action_held(int) override { return false; }
            bool input_action_pressed(int) override { return false; }
            void request_02_004d8000() override { serviced.push_back(0x02u); }
            void request_04_004e4000() override { serviced.push_back(0x04u); }
            void request_06_notify_00e198ac() override { serviced.push_back(0x06u); }
            void request_07_004bfc70() override {
                serviced.push_back(0x07u);
                bsp::enqueue_state_request_004d3ed0(*queue, 0x06u);
            }
            void request_09_notify_00e198b4() override { serviced.push_back(0x09u); }
            void request_0a_0b_004dfb70() override { serviced.push_back(0x0Au); }
            void request_0e_004c6b00() override { serviced.push_back(0x0Eu); }
            void request_0f_004d7970() override { serviced.push_back(0x0Fu); }
            bool request_10_teardown_004e458a(bsp::GameFrameControlState&) override {
                serviced.push_back(0x10u);
                return false;
            }
            void request_12_resume_004cd0f0() override { serviced.push_back(0x12u); }
            void request_14_004bac20() override { serviced.push_back(0x14u); }
            void request_16_notify_00e198b8() override { serviced.push_back(0x16u); }
            bool network_session_active() override { return false; }
            void post_drain_00a95960(float) override {}
            void update_cutscene_playback_004c6b20(float) override {}
            void mission_hud_update(float) override {}
            void accumulate_frame_statistics_0053c510() override {}
            void update_presence_context_004c0170() override {}
            void update_device_wait_screen_004db920() override {}
            void set_front_end_pending_flag(bool) override {}
            void pre_tick_console_commands() override {}
            bsp::MissionResult mission_result() override { return bsp::MissionResult{}; }
            void world_final_tick(float) override {}
            void world_post_tick() override {}
            void show_mission_result_gui(float) override {}
            void close_mission_result() override {}
        };

        bsp::GameFrameControlState state{};
        OrderHost host;
        host.queue = &state.requests;
        for (std::uint32_t request : {0x07u, 0x14u, 0x0Du, 0x02u, 0x04u}) {
            bsp::enqueue_state_request_004d3ed0(state.requests, request);
        }
        bsp::drain_state_requests_004e4430(state, host);

        const std::vector<std::uint32_t> expected{0x07u, 0x14u};
        check(host.serviced == expected,
            "the drain services requests in order and stops on the in-mission request");
        check(state.state == 0x0Du, "the stopping request stays in the state field");
        check(state.requests.count == 3,
            "requests behind the stop and the one a handler added survive the pass");
        check(bsp::front_state_request(state.requests) == 0x02u,
            "the next frame resumes at the request that followed the stop");
    }

    {
        // 00689CC0: the countdown boundary and the delta clamp. COMISS at
        // 00689D5F skips on 0.0 <= countdown, so the frame that lands exactly on
        // zero must not raise the screen, and the clamp at 00689D0E caps one
        // frame's cost at DAT_00D7A24C seconds however long the hitch was.
        struct Host final : bsp::AttractScreenHost {
            bool button{false};
            int state{2};
            int activated{0};
            int deactivated{0};
            bool any_dynamic_device_button_down() override { return button; }
            bool any_joystick_slot_button_down() override { return false; }
            bool scene_context_blocks_attract() override { return false; }
            int game_state() override { return state; }
            void activate_attract_screen() override { ++activated; }
            void deactivate_attract_screen() override { ++deactivated; }
            bool front_end_menu_present() override { return false; }
            void front_end_menu_on_attract_dismissed() override {}
        } host;

        bsp::AttractScreenState screen;
        bsp::update_attract_screen_00689cc0(screen, host, 1000.0f);
        check(screen.idle_countdown == bsp::kAttractIdleSeconds - bsp::kAttractDeltaClampSeconds,
            "a long hitch costs at most one second of idle time");

        screen.idle_countdown = bsp::kAttractIdleSeconds;
        for (int i = 0; i < 45; ++i) bsp::update_attract_screen_00689cc0(screen, host, 1.0f);
        check(screen.idle_countdown == 0.0f && host.activated == 0 && !screen.active,
            "the attract screen does not rise on the frame the countdown reaches zero");

        bsp::update_attract_screen_00689cc0(screen, host, 1.0f);
        check(host.activated == 1 && screen.active, "it rises on the first negative frame");

        host.button = true;
        bsp::update_attract_screen_00689cc0(screen, host, 1.0f);
        check(host.deactivated == 1 && !screen.active
                && screen.idle_countdown == bsp::kAttractIdleSeconds,
            "a button press dismisses it and reloads the countdown");

        screen = bsp::AttractScreenState{};
        host.button = false;
        host.state = bsp::kAttractSuppressedGameStates[0];
        screen.idle_countdown = -1.0f;
        bsp::update_attract_screen_00689cc0(screen, host, 0.0f);
        check(host.activated == 1, "a suppressed game state blocks the raise");
    }

    {
        // The drain at 004e5455 re-polls 006840f0 after every pass and stops only
        // when no channel is left with a mismatched index pair. The first channel
        // needs two passes; the second is active but already matched, and the
        // third has a null global.
        struct DrainHost : bsp::SessionPollHost {
            bsp::MenuInterfaceState menus{};
            int peer_pumps{0};
            int interface_updates{0};
            int latch_clears{0};

            void service_menu_channel(std::size_t index, std::int32_t target_a,
                std::int32_t target_b) override {
                // The native virtual +10h advances the channel toward its target.
                bsp::MenuRequestChannel& channel = menus.channels[index];
                if (channel.current_a < target_a) ++channel.current_a;
                channel.current_b = target_b;
            }
            void read_menu_interface_state(bsp::MenuInterfaceState& out) override { out = menus; }
            void pump_peer_messages_00776230() override { ++peer_pumps; }
            void update_interface_only_004c40f0() override { ++interface_updates; }
            void clear_menu_transition_latch_00e18cdc() override { ++latch_clears; }
            void update_multiplayer_interface_004d80d0() override {}

            void pump_platform_manager_00a409f0() override {}
            bool system_ui_flag_00e188ae() override { return false; }
            void on_system_ui_raised_004ceb40() override {}
            bool profile_changed_pending() override { return false; }
            bool take_storage_removed() override { return false; }
            bool take_invite_accepted() override { return false; }
            int game_state() override { return 0x0D; }
            bool game_flag_620() override { return false; }
            void game_620_handler_004d94f0() override {}
            bool game_flag_61f() override { return false; }
            void game_61f_handler_004d95f0() override {}
            void profile_lost_in_state4_004d7f90() override {}
            void teardown_menu_objects_004db190() override {}
            void teardown_session_004cccc0() override {}
            void clear_game_flag_2180() override {}
            void on_init_title_004c9a70() override {}
            bool storage_owner_idle_0109cecc() override { return false; }
            void storage_removed_side_effect_00bd3450() override {}
            void dismiss_dialog_layers_00530650() override {}
            void invite_decision_inputs(bsp::InviteDecisionInputs&) override {}
            bool leaving_ends_session_004bb8a0() override { return false; }
            void accept_invite_004d8000() override {}
            void show_prompt_00531b00(const bsp::PromptRequest&) override {}
            void update_online_stats_write_004caa90() override {}
            void multiplayer_pre_tick_00778450(float) override {}
            void multiplayer_post_tick_0076ffc0(float) override {}
        };

        DrainHost host;
        host.menus.channels[0].present = true;
        host.menus.channels[0].active = true;
        host.menus.channels[0].target_a = 2;
        host.menus.channels[0].target_b = 7;
        host.menus.channels[1].present = true;
        host.menus.channels[1].active = true;
        host.menus.channels[1].current_a = 5;
        host.menus.channels[1].target_a = 5;
        host.menus.channels[2].active = true; // global is null, so it is skipped
        host.menus.channels[2].target_a = 4;

        const int iterations = bsp::run_menu_interface_drain_004e5434(host);
        check(iterations == 2, "the menu drain repeats until a poll reports nothing pending");
        check(host.peer_pumps == 2 && host.interface_updates == 2,
            "each drain pass runs the peer pump and the interface-only update once");
        check(host.latch_clears == iterations + 1,
            "00e18cdc is cleared after the first poll and after every re-poll");
        check(host.menus.channels[2].current_a == 0,
            "a null channel global is skipped even when its indices differ");
    }

    {
        // The 4Bh chain at 004E5180 falls through into the same pause test its
        // failures jump to, so it reads like a second way to reach the pause
        // menu when it is in fact the only way to skip that test. Pin the
        // inversion: the chain holding suppresses pause, and the chain failing
        // leaves an ordinary press of action 1 able to open the menu.
        bsp::PauseDecisionInputs inputs{};
        inputs.pause_pressed = true;
        inputs.alternate_pause_pressed = true;
        inputs.top_populated_level = 2;
        check(bsp::decide_simulation_gate_branch(inputs)
                == bsp::SimulationGateBranch::kInterfaceOnly,
            "the alternate-pause chain suppresses the pause menu");

        inputs.multiplayer_spectator_active = true;
        check(bsp::decide_simulation_gate_branch(inputs)
                == bsp::SimulationGateBranch::kPauseMenuOpened,
            "a spectator breaks the suppression and the pause test decides");

        inputs.alternate_pause_pressed = false;
        inputs.multiplayer_spectator_active = false;
        check(bsp::decide_simulation_gate_branch(inputs)
                == bsp::SimulationGateBranch::kPauseMenuOpened,
            "without the alternate action a press of action 1 opens the menu");
    }

    {
        // 0068ecc8 compares 0.0 against the remaining time and skips on JBE, so
        // a cooldown that lands exactly on zero is kept and only a strictly
        // negative one is erased. Getting the boundary backwards would drop a
        // hint's cooldown one frame early on every whole-number delta.
        bsp::HintCooldown entries[2] = { { "BASICSHIP", 1.0f }, { "BASICPLANE", 1.5f } };
        check(bsp::tick_hint_cooldowns_0068ec10(entries, 2, 1.0f) == 2
                && entries[0].remaining == 0.0f,
            "a cooldown resting on zero survives the tick");
        check(bsp::tick_hint_cooldowns_0068ec10(entries, 2, 0.25f) == 1
                && entries[0].name != nullptr && std::strcmp(entries[0].name, "BASICPLANE") == 0,
            "the next tick erases only the entry that went negative");
    }


    {
        // 00987590 returns as soon as it retires one expired event, so the
        // higher-priority ready event it had already selected is not applied
        // that frame. An implementation that erased and kept walking would
        // apply it, which is the regression this case pins.
        struct EventHost : bsp::WorldTickHost {
            std::vector<float> starts;
            std::vector<float> priorities;
            std::vector<bool> ready;
            float now{100.0f};
            int applied{-1};
            int destroyed{-1};

            float world_clock() override { return now; }
            void mission_events_pre_pass_00982540() override {}
            void mission_events_periodic_00977990() override {}
            void mission_events_poll_0096d540() override {}
            void mission_events_poll_00968550() override {}
            float mission_event_start_time(std::size_t i) override { return starts[i]; }
            float mission_event_priority(std::size_t i) override { return priorities[i]; }
            bool mission_event_ready_005b71d0(std::size_t i) override { return ready[i]; }
            void mission_event_destroy(std::size_t i) override { destroyed = static_cast<int>(i); }
            void mission_event_apply_00974070(std::size_t i) override { applied = static_cast<int>(i); }
            void bot_retarget_begin_0075b430(int) override {}
            void bot_slot_prepare_00914390(std::size_t) override {}
            void bot_slot_dispatch_0076a9f0(std::uint32_t, std::uint32_t, std::size_t) override {}
            void bot_think_pass_a_00911e80(std::size_t) override {}
            void bot_think_pass_b_00912a60(std::size_t) override {}
            void marker_update(void*, float) override {}
            bsp::MarkerColor marker_get_color(void*) override { return bsp::MarkerColor{}; }
            void marker_set_color(void*, const bsp::MarkerColor&) override {}
            void marker_set_scale_006dbac0(void*, int, int, float) override {}
            void marker_set_highlight_level(void*, float) override {}
            void entity_manager_update(float) override {}
            void power_ups_pre_pass_008eac80() override {}
            void power_up_expire_008e8c30(void*) override {}
            void power_up_notify_ready_009789a0(std::size_t, std::size_t) override {}
            void power_ups_post_pass_00613760() override {}
            void activate_entity_subtree_00922fd0(std::size_t) override {}
            bool input_action_pressed(int) override { return false; }
        };

        EventHost host;
        host.starts = {99.0f, 99.0f, 10.0f};
        host.priorities = {1.0f, 5.0f, 0.0f};
        host.ready = {true, true, false};

        bsp::WorldTickState state;
        state.mission_events.events.resize(3);
        for (auto& event : state.mission_events.events) event.duration = 5.0f;

        const bsp::MissionEventTickResult result
            = bsp::update_mission_events_00987590(state, host);
        check(result.retired == 2, "the expired event at index 2 is the one retired");
        check(result.selected == bsp::kNoMissionEvent && host.applied == -1,
            "retiring an event abandons the selection instead of applying it");
        check(state.mission_events.events.size() == 2,
            "exactly one event leaves the queue per frame");
    }


    {
        // 00BBD310 re-orthogonalises the ocean frame against the reference axis
        // every tick. When the previous up axis is parallel to that reference,
        // both cross products collapse, 00419510 returns the zero vector and the
        // length test at 00BBD3D9 falls below 00CE3800, so the frame resets to
        // the canonical axes instead of keeping a zero basis.
        bsp::OceanState ocean{};
        ocean.enabled = true;
        ocean.frame.up = bsp::OceanVec3{0.0f, 0.0f, 1.0f};
        check(bsp::ocean_orthonormalize_00bbd310(ocean, bsp::OceanVec3{0.0f, 0.0f, 1.0f}),
            "an enabled ocean runs the frame path");
        check(ocean.frame.up.x == 0.0f && ocean.frame.up.y == 1.0f && ocean.frame.up.z == 0.0f,
            "a degenerate ocean frame resets its up axis to +Y");
        check(ocean.frame.right.x == 1.0f && ocean.frame.right.y == 0.0f
                && ocean.frame.right.z == 0.0f,
            "a degenerate ocean frame resets its right axis to +X");

        // A perpendicular reference axis keeps the up axis and rebuilds right.
        ocean.frame.up = bsp::OceanVec3{0.0f, 0.0f, 1.0f};
        check(bsp::ocean_orthonormalize_00bbd310(ocean, bsp::OceanVec3{0.0f, 1.0f, 0.0f}),
            "an enabled ocean runs the frame path for a perpendicular axis");
        check(ocean.frame.up.z == 1.0f && ocean.frame.up.x == 0.0f && ocean.frame.up.y == 0.0f,
            "an already orthogonal up axis survives the Gram-Schmidt step");
        check(ocean.frame.scale == 1.0f && ocean.frame.visible_layer_count == 0,
            "the ocean tick rewrites the scale and clears the visible layer count");
    }

    {
        // 00a8fe30: the capability gate comes first, then 'DF16', and only a
        // refused 'DF16' asks for D3DFMT_D16. A device that accepts both must
        // still end up on 'DF16' with the plain-depth flag clear.
        bsp::RendererCapabilityRecord caps;
        caps.pixel_shader_version = 0x0200;
        const bsp::ShadowDepthProbeResult both
            = bsp::select_shadow_depth_format_00a8fe30(caps, true, true);
        check(both.format == bsp::ShadowDepthFormat::df16
                && both.d3d_format == bsp::kFourCcDf16 && !both.plain_depth_fallback,
            "DF16 wins over D3DFMT_D16 when the device accepts both");

        const bsp::ShadowDepthProbeResult fallback
            = bsp::select_shadow_depth_format_00a8fe30(caps, false, true);
        check(fallback.format == bsp::ShadowDepthFormat::d16
                && fallback.d3d_format == bsp::kD3dFmtD16 && fallback.plain_depth_fallback,
            "a refused DF16 falls back to D3DFMT_D16 and sets the plain-depth flag");

        caps.pixel_shader_version = 0x01FF;
        const bsp::ShadowDepthProbeResult gated
            = bsp::select_shadow_depth_format_00a8fe30(caps, true, true);
        check(gated.format == bsp::ShadowDepthFormat::none && gated.d3d_format == 0,
            "below ps_2_0 the probe records no format even when both are supported");
    }

    {
        // 004e4151 compares game+5D4h against the 3 that GGame::OnInit wrote one
        // call earlier, not against the 4 the drain dispatched. When the platform
        // poll moves the state off 3 the shell is abandoned: no front-end manager
        // is created and 004e4279 never writes 5. Reading that gate as "still 4"
        // would build the front end after a sign-out and leave the state at 3.
        struct ShellHost final : bsp::FrontEndShellHost {
            bsp::SingletonLifetimeDomain lifetime{{this,
                [](void* context, void* owner, std::uint32_t flags) noexcept {
                    auto& self = *static_cast<ShellHost*>(context);
                    bsp::scalar_delete_gameplay_effect_manager_008703e0(
                        static_cast<bsp::GameplayEffectManager*>(owner), flags, self.effects);
                }, [](void*) { throw std::runtime_error("invalid singleton fixture state"); }}};
            bsp::GameplayEffectManager* volatile effect_singleton = nullptr;
            bsp::GameplayEffectManagerAllocationWords effect_words{0xa5a5a5a5};
            bsp::GameplayEffectManagerContext effects{lifetime, effect_singleton, effect_words};
            ~ShellHost() override { lifetime.shutdown(); }
            std::int32_t state_after_poll = 3;
            int managers_created = 0;
            int end_loading_calls = 0;
            bsp::LoadingScreenConfig globals{};
            void renderer_set_budget(std::uint32_t) override {}
            void probe_texture_memory(const char*) override {}
            void probe_sound_memory(const char*) override {}
            bsp::GameplayEffectManagerContext& effect_manager_context() override { return effects; }
            bool title_screen_present() override { return false; }
            void destroy_title_screen() override {}
            bool front_end_manager_b8_present() override { return false; }
            bool front_end_manager_ac_present() override { return false; }
            bool front_end_manager_b4_present() override { return false; }
            void open_load_block(const char*) override {}
            void close_load_block() override {}
            bsp::LoadingScreenConfig& loading_globals() override { return globals; }
            void begin_loading(bsp::LoadingScreenMode) override {}
            void report_progress(float) override {}
            void end_loading() override { ++end_loading_calls; }
            void game_on_init() override {}
            void poll_platform_session_events() override {}
            std::int32_t game_state() override { return state_after_poll; }
            void create_manager_b8() override { ++managers_created; }
            void create_manager_ac() override { ++managers_created; }
            void create_manager_b4() override { ++managers_created; }
            void lua_collect_garbage() override {}
            int manager_ac_mode() override { return 4; }
            void reset_manager_ac_mode() override {}
            void manager_ac_enter() override {}
            void manager_ac_start_sub() override {}
            bool post_state_hook_wanted() override { return false; }
            void post_state_hook() override {}
            bool award_gate_open() override { return false; }
            int award_id(const char*) override { return 0; }
            bool award_system_ready() override { return false; }
            bool award_session_ready() override { return false; }
            void grant_award(int) override {}
            void record_award(const char*, int) override {}
            void send_network_quit() override {}
        };

        ShellHost aborted;
        aborted.state_after_poll = 2;
        bsp::FrontEndShellState aborted_state;
        aborted_state.state = bsp::kGameStateFrontEndRequest;
        check(bsp::enter_front_end_shell(aborted_state, aborted)
                == bsp::FrontEndShellOutcome::AbortedByPlatformEvent
                && aborted.managers_created == 0 && aborted_state.state == 2,
            "a platform event off state 3 abandons the shell before the managers");

        ShellHost ready;
        bsp::FrontEndShellState ready_state;
        ready_state.state = bsp::kGameStateFrontEndRequest;
        check(bsp::enter_front_end_shell(ready_state, ready)
                == bsp::FrontEndShellOutcome::ShellReady
                && ready.managers_created == 3
                && ready_state.state == bsp::kGameStateFrontEndShellReady,
            "the shell settles on state 5, never on the 4 the drain dispatched");
        check(aborted.end_loading_calls == 1 && ready.end_loading_calls == 1,
            "0057c250 runs on both exits, so the loading screen never leaks");
        check(aborted.effect_singleton && ready.effect_singleton
                && aborted.lifetime.published_manager()->count_00bcf910() == 1
                && ready.lifetime.published_manager()->count_00bcf910() == 1
                && ready.effect_singleton->allocator_04 == 0xa5a5a5a5
                && ready.lifetime.published_manager()->system_owner().section_10->recursion_18 == 0
                && bsp::get_gameplay_effect_manager_004c1650(ready.effects) == ready.effect_singleton,
            "effect probe creates and registers its concrete singleton before either shell exit");
        // The native map owns nodes, not effect definitions. A non-dereferenceable
        // raw value detects accidental release/dispatch during probe and teardown.
        ready.effect_singleton->definitions->emplace(7, reinterpret_cast<void*>(1));
        bsp::probe_gameplay_effect_registry_0086b0b0(*ready.effect_singleton, "ignored");
        ready.lifetime.shutdown();
        check(!ready.effect_singleton,
            "effect manager teardown clears its global without touching weak definition values");
    }

    {
        // 00AEF3C0 decides which .ats files a single atlas request pulls in.
        // The prefix span is len(request)-3, taken from the request rather than
        // the candidate, so an off-by-one here silently loads the wrong split
        // parts. The installed interface/textures directory has no common.ats
        // and exactly four common_* parts, which is what this rule must accept.
        const std::string request = "interface/textures/common.ats";
        check(bsp::atlas_split_name_matches_00aef3c0(request,
                  "interface/textures/common_dxt5_1.ats"),
            "a split atlas part matches its logical request");
        check(bsp::atlas_split_name_matches_00aef3c0(request,
                  "interface\\textures\\common.ats"),
            "separator normalisation makes the exact path match");
        check(!bsp::atlas_split_name_matches_00aef3c0(request,
                  "interface/textures/commonx_dxt1.ats"),
            "the underscore is required, so a longer stem is rejected");
        check(!bsp::atlas_split_name_matches_00aef3c0(request,
                  "interface/textures/fe/common_dxt1.ats"),
            "a candidate in a deeper directory is rejected");
    }

    {
        struct AtlasRegistryHost final : bsp::WorldEffectsStartupHost {
            const std::string request{"interface/textures/common.ats"};
            std::vector<std::string> registered;
            std::vector<std::string> events;
            void log_line(const std::string&) override {}
            std::vector<std::string> find_files_with_extension(
                const std::string&, const std::string&) override {
                events.push_back("enumerate");
                return {"interface/textures/common_dxt5_1.ats",
                    "interface/textures/common_dxt5_2.ats",
                    "interface/textures/other.ats"};
            }
            std::vector<std::string> registered_atlas_texture_paths() override {
                events.push_back("registry");
                return registered;
            }
            bool vfs_resolve_existing(std::string&) override {
                events.push_back("resolve");
                return true;
            }
            void load_atlas_descriptor(const std::string&) override {
                events.push_back("load");
                registered.push_back(request);
            }
            [[noreturn]] static void unexpected() {
                throw std::logic_error("unexpected non-atlas fixture call");
            }
            void* create_water_texture_source(int, const std::string&, int) override { unexpected(); }
            void publish_water_texture_source_table(const bsp::WaterTextureSourceTable&) override { unexpected(); }
            std::vector<bsp::DecalDefinition> read_decal_table(const std::string&) override { unexpected(); }
            std::vector<bsp::FoliageTypeDefinition> read_foliage_table(const std::string&) override { unexpected(); }
            void* load_vertex_format(const std::string&) override { unexpected(); }
            void* load_shader(const std::string&) override { unexpected(); }
            void* load_texture(const std::string&) override { unexpected(); }
            void* create_vertex_declaration(void*) override { unexpected(); }
            void* create_index_buffer(std::uint32_t) override { unexpected(); }
            void fill_index_buffer(void*, const std::vector<std::uint16_t>&) override { unexpected(); }
            void release_texture(void*) override { unexpected(); }
            void* create_material_for_effect(const std::string&) override { unexpected(); }
            void set_material_texture_slot(void*, int, void*) override { unexpected(); }
            void publish_decal_system(std::vector<bsp::DecalDefinition>) override { unexpected(); }
            void publish_foliage_system(std::vector<bsp::FoliageTypeDefinition>) override { unexpected(); }
            void publish_particle_shader_set(const bsp::ParticleShaderSet&) override { unexpected(); }
            void publish_foliage_group_manager(const bsp::FoliageGroupManagerState&) override { unexpected(); }
            void set_foliage_enabled_flag(bool) override { unexpected(); }
            std::size_t foliage_type_count() override { unexpected(); }
            std::size_t foliage_group_count() override { unexpected(); }
            void set_foliage_type_enabled(std::size_t, bool) override { unexpected(); }
            void set_foliage_group_enabled(std::size_t, bool) override { unexpected(); }
            void* create_critical_section() override { unexpected(); }
        } host;
        // Registration can change the current array between matching candidates.
        // A nonmatching candidate must not query the array or invoke the loader.
        std::size_t missing = 0;
        const auto loaded = bsp::load_texture_atlas_00af0060(host.request, host, &missing);
        check(loaded == 1 && missing == 0 && host.events == std::vector<std::string>{
                  "enumerate", "registry", "resolve", "load", "registry"},
            "atlas candidates observe registration performed by the preceding loader callback");
    }

    {
        // 00be2fa0 indexes the job array with the value InterlockedDecrement
        // returned, so BSP_Game_Render's two enqueues drain in reverse: the
        // camera update runs before the world view. A FIFO reading of the pool
        // would silently reverse the frame's job order.
        const std::vector<bsp::GameRenderJob> enqueued{
            bsp::GameRenderJob::kWorldView004bbd00, bsp::GameRenderJob::kCameraUpdate004b4820};
        const std::vector<bsp::GameRenderJob> drained = bsp::frame_job_drain_order(enqueued);
        check(drained.size() == 2 && drained[0] == bsp::GameRenderJob::kCameraUpdate004b4820
                && drained[1] == bsp::GameRenderJob::kWorldView004bbd00,
            "the frame job pool drains last in first out");
    }

    {
        // 0057CAE9 negates a non-positive delta with -0.0f minus the delta
        // rather than clearing the sign bit, so the loading worker hands
        // wave_Icon a negative zero whenever two ticks land on the same clock
        // sample. Replacing this with fabs would look equivalent and silently
        // change the sign the widget receives.
        const float repeated = bsp::worker_tick_delta_seconds(1234.0f, 1234.0f);
        check(repeated == 0.0f && std::signbit(repeated),
            "an unchanged clock sample yields -0.0f, not +0.0f");
        check(bsp::worker_tick_delta_seconds(1040.0f, 1000.0f) == 0.04f,
            "a 40 ms tick is one 25 Hz period in seconds");
    }

    {
        // 006851aa compares elapsed against the entry delay with FCOMIP/JBE and
        // only then queries input action 4Ah, so both terms are required and the
        // time term is strict. docs/GAME_FRONTEND_STATES.md read this as a timed
        // advance; the sequence actually advances from the movie-end callback
        // 00685060, and a disjunctive or non-strict rule here would hide that.
        check(bsp::logo_skip_allowed(3.0F, 2.0F, true), "a late skip with the button fires");
        check(!bsp::logo_skip_allowed(3.0F, 2.0F, false), "elapsed time alone never skips");
        check(!bsp::logo_skip_allowed(2.0F, 2.0F, true), "the delay comparison is strict");
    }

    {
        // 0067cfc8 accumulates dt * 2.5 into 00E19888, wraps it with fmod
        // against 2*pi at 0067cfec and masks the sign bit off the sine at
        // 0067d026. The wrap and the mask are what keep the press_start_Text
        // alpha inside [0, 1]; a plain accumulate or a signed sine would drive
        // the element colour negative every other half period.
        bsp::PromptPulse pulse{};
        float alpha = 0.0F;
        for (int frame = 0; frame < 200; ++frame) {
            alpha = bsp::advance_prompt_pulse_0067cfc8(pulse, 0.05F);
            check(alpha >= 0.0F && alpha <= 1.0F, "the prompt alpha stays in [0, 1]");
            check(pulse.phase >= 0.0F
                    && pulse.phase < static_cast<float>(bsp::kPromptPulsePeriod),
                "the pulse phase stays wrapped into one period");
        }
    }

    {
        // 004e4310 stops at the first failing gate, so a host must not see a
        // call the native would not have made. The "RANK" row exists in the
        // shipped table with XLastAchievementID 0, which the range check at
        // 004e437a rejects: looking the name up must not reach the manager.
        struct RecordingHost : bsp::AwardGrantHost {
            std::vector<std::string> calls;
            bool midway_save_folder_preexisted() override {
                calls.push_back("folder");
                return true;
            }
            int award_id_for_name(const char* name) override {
                calls.push_back("lookup");
                return bsp::award_id_for_name_006b8da0(name);
            }
            bool live_enabled_account() override {
                calls.push_back("live");
                return true;
            }
            bool signed_into_live() override {
                calls.push_back("signed");
                return true;
            }
            void queue_online_award(int) override { calls.push_back("queue"); }
            void record_local_award(const char*, int) override { calls.push_back("record"); }
        };
        RecordingHost rank;
        check(!bsp::grant_award_if_earned(rank, "RANK", 1)
                && rank.calls.size() == 2 && rank.calls[1] == "lookup",
            "an ungrantable id stops the sequence before the online checks");
        RecordingHost granted;
        check(bsp::grant_midway_save_award(granted) && granted.calls.size() == 6
                && granted.calls[4] == "queue" && granted.calls[5] == "record",
            "GA_HM queues the online award before the local record");
    }

    {
        // 004F8710 with only the terminator: the level is cleared and the
        // recompute drops every screen that is not held by another level.
        struct Bindings : bsp::FrontEndScreenSetHostBindings {
            bool screen_manages_own_visibility(int) override { return false; }
            bool screen_occludes_lower_levels(int) override { return false; }
        };
        Bindings bindings;
        bsp::FrontEndScreenTable table{};
        bsp::FrontEndScreen briefing{};
        bsp::FrontEndScreen overlay{};
        table.slots[0x03] = &briefing;
        table.slots[0x54] = &overlay;
        bsp::FrontEndScreenSetStack stack{};
        const int briefing_id[1] = {0x03};
        const int overlay_id[1] = {0x54};
        bsp::set_front_end_screen_set_004f8710(stack, table, bindings, briefing_id, 1);
        bsp::set_front_end_screen_set_level(stack, table, bindings, 5, overlay_id, 1);
        bsp::set_front_end_screen_set_004f8710(stack, table, bindings, nullptr, 0);
        check(!briefing.wanted && overlay.wanted && stack.dirty,
            "an empty screen set clears its own level and leaves the other levels standing");
    }

    {
        // 00599DB0 picks the mission group from the page on the five list pages
        // and from the two campaign bytes on the mission-detail page. The two
        // selectors must agree on the four groups they share, which is the claim
        // that corrects the campaign-page naming of docs/MAIN_MENU_SCREENS.md.
        using bsp::MainMenuPage;
        using bsp::MissionGroup;
        check(bsp::mission_group_for_page(static_cast<MainMenuPage>(0x04)) == MissionGroup::Ijn
                && bsp::mission_group_for_page(static_cast<MainMenuPage>(0x05)) == MissionGroup::Usn
                && bsp::mission_group_for_page(static_cast<MainMenuPage>(0x08)) == MissionGroup::Training
                && bsp::mission_group_for_page(static_cast<MainMenuPage>(0x06)) == MissionGroup::IjnDlc
                && bsp::mission_group_for_page(static_cast<MainMenuPage>(0x07)) == MissionGroup::UsnDlc,
            "the five mission-list pages map to the groups 0059A4B4 selects");
        check(bsp::mission_group_for_detail(false, false) == MissionGroup::Ijn
                && bsp::mission_group_for_detail(true, false) == MissionGroup::Usn
                && bsp::mission_group_for_detail(false, true) == MissionGroup::IjnDlc
                && bsp::mission_group_for_detail(true, true) == MissionGroup::UsnDlc
                && bsp::mission_group_binding(MissionGroup::Training).handle_offset == 0x320,
            "the detail page reaches the same four groups and never training");
    }

    {
        // 006834A0 flips branches at range == track (FCOMI/JC at 006834E4), and
        // the two arms have to agree there: the thumb keeps its minimum height
        // and the travel becomes the whole track. The zero-range arm at
        // 006834C8 is the other boundary, where the thumb fills the track.
        const bsp::ScrollBarGeometry at = bsp::compute_scroll_bar_geometry(40.0f, 40.0f, 12.0f);
        const bsp::ScrollBarGeometry just_under =
            bsp::compute_scroll_bar_geometry(std::nextafterf(40.0f, 0.0f), 40.0f, 12.0f);
        const bsp::ScrollBarGeometry empty = bsp::compute_scroll_bar_geometry(0.0f, 40.0f, 12.0f);
        check(at.scrollable && at.thumb_height == 12.0f && at.thumb_travel == 40.0f,
            "a range equal to the track leaves the thumb at its minimum height");
        check(just_under.scrollable && std::fabs(just_under.thumb_height - 12.0f) < 1.0e-4f,
            "the branch just below that boundary agrees with it");
        check(!empty.scrollable && empty.thumb_height == 52.0f && empty.thumb_travel == 0.0f,
            "a zero range fills the track with the thumb and reports not scrollable");
        // Each vector differs by one binary32 ULP from the old double
        // chain. Values stay inside the drag track; compare the native
        // PC24 operation schedule, not a tolerance that hides the error.
        check(bsp::dragged_thumb_y(0.08659771829843521f, 0.48606812953948975f,
                  0.03748560696840286f, 0.023717306554317474f, 0.8193197250366211f)
                == 0.5864341855049133f,
            "00683820 drag rounds FSUB and both FADD operations at PC24");
        check(bsp::scroll_offset_from_thumb_y(0.22475291788578033f, 0.0728134736418724f,
                  0.746063232421875f, 0.9753149747848511f) == -0.19862771034240723f,
            "00683820 drag ratio rounds FSUB before FDIV and its DWORD spill");
    }

    {
        // Chain407 review R1: exercise 006834A0 through the same authored
        // render-record refresh and diffuse selection used by build_quads.
        struct Layout final : bsp::FrontEndScrollerHost {
            static bsp::GuiLayoutWidget& node(void* p) { return *static_cast<bsp::GuiLayoutWidget*>(p); }
            bsp::GuiWidgetPoint widget_position(void* p) override { return bsp::resolved_position(node(p).transform); }
            void set_widget_position(void* p, const bsp::GuiWidgetPoint& v) override {
                node(p).transform.position = bsp::local_position_for_resolved(node(p).transform, v);
            }
            bsp::GuiWidgetSize widget_size(void* p) override { return node(p).transform.size; }
            void widget_set_size(void* p, const bsp::GuiWidgetSize& v) override { node(p).transform.size = v; }
            void widget_set_alpha(void* p, float v) override { node(p).color[3] = v; }
            // Same non-reentrant RGBA provider contract as the live menu host.
            bool alpha_writes_preserve_scroller() const noexcept override { return true; }
            float scroll_axis() override { std::abort(); }
            bool pad_axis(int, double&) override { std::abort(); }
            float cursor_x() override { std::abort(); }
            float cursor_y() override { std::abort(); }
        } layout;
        bsp::GuiLayoutWidget thumb, top, bottom, unrelated;
        thumb.transform.size = {0.04f, 0.12f};
        thumb.color[0] = 0.5f;
        thumb.color[1] = 0.25f;
        bsp::FrontEndScreenScroller scroller;
        bsp::attach_scroll_bar_00683380(scroller, &thumb, 0.4f, &top, &bottom, layout);
        bsp::game::FrontendScrollerBridge bridge;
        bridge.bind(scroller);
        for (const float range : {0.0f, 0.2f}) {
            bsp::set_scroll_range(scroller, range, layout);
            bsp::game::GameWidgetRecord drawn_thumb, drawn_arrow, untouched;
            drawn_thumb.height = 0.12f; // load-time authored size, runtime=false
            untouched.height = 0.7f;
            bridge.refresh(drawn_thumb, &thumb);
            bridge.refresh(drawn_arrow, &top);
            const bool positive = range > 0.0f;
            {
                // build_quads uses this height in bottom = top + height*screen_h
                // and this packed diffuse value in ALL six submitted vertices.
                const float expected_pixels = positive ? 230.4f : 374.4f;
                check(std::fabs(drawn_thumb.height * 720.0f - expected_pixels) < 0.0001f
                        && bridge.color(drawn_thumb, &thumb) == (positive ? 0xff8040ffu : 0x338040ffu)
                        && bridge.color(drawn_arrow, &top) == (positive ? 0xffffffffu : 0x40ffffffu)
                        && bridge.color(drawn_arrow, &bottom) == (positive ? 0xffffffffu : 0x40ffffffu)
                        && !drawn_thumb.runtime,
                    "chain407 zero/positive range reaches authored thumb geometry and thumb/arrow vertex tint");
            }
            check(!bridge.refresh(untouched, &unrelated) && untouched.height == 0.7f
                    && bridge.color(untouched, &unrelated) == 0xffffffffu,
                "chain407 does not opt unrelated authored widgets into live bridge rendering");
        }
    }

    {
        // 00AA6750 stores (parent_resolved + own_position) to the frame before
        // it subtracts the parent's pivot offset, so it is not the same float
        // result as folding the pivot in first. The magnitudes below are chosen
        // so the sum loses the low bit that the other order keeps, and
        // 00AA8240 has to invert whatever the getter produced.
        bsp::GuiWidgetTransform root{};
        root.size.width = 1.0f;
        root.pivot_x = 1.0f;
        bsp::GuiWidgetTransform child{};
        child.parent = &root;
        root.position.x = 16777216.0f; // 2^24, where adding one rounds away
        child.position.x = 1.0f;

        const bsp::GuiWidgetPoint resolved = bsp::resolved_position(child);
        const float folded_first = (16777216.0f - 1.0f) + 1.0f;
        check(resolved.x == 16777215.0f && folded_first == 16777216.0f,
            "the parent chain sums before it subtracts the pivot offset");
        check(bsp::local_position_for_resolved(child, resolved).x
                == (resolved.x - 16777216.0f) + 1.0f,
            "00AA8240 inverts the getter in the same operation order");

        // 00AA7220 scales only the Y lanes by the 4:3 factor, and the pivot Y is
        // negated and sized before that factor is applied.
        child.pivot_y = 0.5f;
        child.size.height = 8.0f;
        child.position.y = 4.0f;
        const bsp::GuiWidgetLocalTransform local = bsp::local_transform(child);
        check(local.translation.x == child.position.x
                && local.translation.y == 3.0f
                && local.pivot_translation_x == 0.0f
                && local.pivot_translation_y == -3.0f,
            "the aspect factor reaches the Y translation and the Y pivot only");
    }

    {
        // 008D64A0 guards most rows with the equality test at 00BD5680, so a
        // field that still holds its default is never written and the reader
        // has to reconstruct it. The four control bytes are the risk: the
        // control reset 008D4820 writes invertCameraY = 0 while the serializer
        // omits that key at 1, so a fresh reset must persist both invert bytes.
        bsp::GameSettingsBlock settings;
        bsp::reset_control_defaults_008d4820(settings, false, false);
        struct Recorder : bsp::SettingsWriter {
            std::vector<std::string> keys;
            bool text_written{false};
            bool first_section_after_text{false};
            bool saw_section{false};
            void write_options_text_008d6170() override { text_written = true; }
            void begin_section(const char*) override
            {
                if (!saw_section) first_section_after_text = text_written;
                saw_section = true;
            }
            void begin_section(const bsp::GuiLuaVariant&) override
            {
                begin_section(static_cast<const char*>(nullptr));
            }
            void end_section() override {}
            void write_field(const char* key, const bsp::SettingsValue&) override
            {
                keys.emplace_back(key);
            }
            void write_field(const bsp::GuiLuaVariant& key, const bsp::SettingsValue& value) override
            {
                if (key.tag == 0) write_field(key.value.text, value);
            }
            void write_keyboard_setup() override {}
        } recorder;
        bsp::write_settings_008d64a0(settings, recorder);
        check(recorder.first_section_after_text,
            "options text persistence precedes archive settings sections");
        const auto has = [&recorder](const char* key) {
            return std::find(recorder.keys.begin(), recorder.keys.end(), std::string(key))
                != recorder.keys.end();
        };
        check(has("invertCameraY") && has("invertPlaneY"),
            "the control reset leaves both invert bytes different from the omitted default");
        check(!has("imperial") && !has("cameraShake"),
            "rows still holding the serializer default are dropped");
    }

    {
        // Chain 542: one complete archive exercises the live writer's key,
        // value, indentation and terminator providers with real owned bytes.
        bsp::ArchiveStringSink sink;
        bsp::ArchiveTextOutput output{nullptr, false, &sink};
        bsp::ArchiveTextWriter writer(output);
        writer.write_field("number", bsp::SettingsValue::from_int(-2147483647 - 1));
        writer.begin_section("T");
        writer.write_field(bsp::gui_lua_key_by_index(3),
            bsp::SettingsValue::from_text("a\\\"b\n\t"));
        writer.begin_section(bsp::gui_lua_key_by_float(1.5f));
        writer.write_field("flag", bsp::SettingsValue::from_bool(true));
        writer.write_field("ratio", bsp::SettingsValue::from_float(-0.25f));
        writer.end_section();
        writer.end_section();
        const std::string expected = "number = -2147483648\nT = {\n"
            "\t[3] = \"a\\\\\\\"b\n\t\",\n\t[1.500000] = {\n"
            "\t\t[\"flag\"] = true,\n\t\t[\"ratio\"] = -0.250000,\n\t},\n}\n";
        check(writer.depth() == 0 && sink.str() == expected,
            "archive native emitters preserve exact nested scalar bytes");
        // 00BD6100 uses signed SAR, not a nibble rotation, for >=80h.
        // FILE takes precedence over the retained buffered sink.
        std::FILE* file = nullptr;
        const auto opened = tmpfile_s(&file);
        check(opened == 0 && file != nullptr, "archive encoded fixture FILE opens");
        if (file != nullptr) {
            output.file_0109cee0 = file;
            output.compact_encoded_0109cee4 = true;
            const unsigned char input[]{' ', '\t', '\r', '\n', 'A', 0x80, 0xf1, 0};
            const unsigned char encoded[]{0x14, 0xf8, 0xff, 0};
            output.emit(std::string_view(reinterpret_cast<const char*>(input), sizeof input));
            std::rewind(file);
            unsigned char actual[sizeof encoded]{};
            check(std::fread(actual, 1, sizeof actual, file) == sizeof actual
                    && std::memcmp(actual, encoded, sizeof actual) == 0
                    && std::fgetc(file) == EOF && sink.str() == expected,
                "archive encoded output strips whitespace and preserves signed high-byte transform");
            std::fclose(file);
            // The successful compact write just warmed TWO same-sized
            // pooled blocks (original + by-value encoded copy). Reusing
            // that input makes unchanged bump/free counts a direct check
            // for missing OR double returns when an output provider fails.
            auto& strings = bsp::game::game_native_string_process();
            auto* const pool = strings.pool_01090aa8();
            check(pool != nullptr && strings.returns_disabled_01090aa4() == 0,
                "archive failure fixture has the actual returning pool");
            const auto bump = pool->bump_6acfc8;
            const auto free_blocks = pool->ring_6acfcc.live_2004b0;
            struct RejectingSink final : bsp::ArchiveByteSink {
                unsigned calls{};
                void write(std::string_view) override {
                    ++calls;
                    throw std::runtime_error("archive rejecting sink");
                }
            } rejecting;
            output = {nullptr, false, &rejecting};
            bool plain_failed = false;
            try {
                output.emit(std::string_view(reinterpret_cast<const char*>(input), sizeof input));
            } catch (const std::runtime_error& error) {
                plain_failed = std::string_view(error.what()) == "archive rejecting sink";
            }
            check(plain_failed && rejecting.calls == 1 && pool->bump_6acfc8 == bump
                    && pool->ring_6acfcc.live_2004b0 == free_blocks,
                "throwing archive sink returns its owned pooled block exactly once");

            std::FILE* read_only = nullptr;
            check(fopen_s(&read_only, "NUL", "rb") == 0 && read_only != nullptr,
                "archive compact failure fixture opens a read-only FILE");
            if (read_only != nullptr) {
                output = {read_only, true, &rejecting};
                bool compact_failed = false;
                try {
                    output.emit(std::string_view(reinterpret_cast<const char*>(input), sizeof input));
                } catch (const std::runtime_error& error) {
                    compact_failed = std::string_view(error.what()) == "archive fwrite failed";
                }
                check(compact_failed && rejecting.calls == 1 && pool->bump_6acfc8 == bump
                        && pool->ring_6acfcc.live_2004b0 == free_blocks,
                    "failed compact fwrite returns both pooled blocks exactly once");
                std::fclose(read_only);
            }
        }
    }

    {
        // 004cd7f0 keys every numbered VFS block of the mission load, and the
        // order of its two tail steps is the part that is easy to get wrong:
        // the truncation happens at the SECOND underscore, and only a name that
        // survives it still carries the ".scn" the last step drops. append_mode
        // 0 leaves out the game-mode suffix (kSceneShortNameModeSuffixBound).
        check(bsp::derive_scene_short_name("universe\\scenes\\pearl_harbor_01.scn", false)
                == "pearl_harbor",
            "the short name is cut at the second underscore, before the extension test");
        check(bsp::derive_scene_short_name("universe/scenes/midway.scn", false) == "midway",
            "a name with no second underscore keeps its stem and loses .scn");
        check(bsp::derive_scene_short_name("universe/Scenes/missions/USN/usn_2_java.scn")
                == (bsp::kSceneShortNameModeSuffixBound ? "usn_2SOLO" : "usn_2"),
            "the campaign stand-in appends the 00E08240 suffix SOLO when the switch is ON");
    }

    {
        // 00AA2490 splits on the LAST underscore, so a key that ends in one has
        // an empty suffix and stays a property, while a key with no underscore
        // at all is matched whole. Both boundaries decide whether a shipped key
        // becomes a widget, and the shipped pages exercise the case folding
        // ("keret_Framebox" against the literal "FrameBox").
        using bsp::GuiWidgetType;
        check(bsp::gui_widget_type_for_key_00aa2490("safezone_43_FrameBox")
                  == GuiWidgetType::FrameBox
              && bsp::gui_widget_type_for_key_00aa2490("keret_Framebox")
                  == GuiWidgetType::FrameBox
              && bsp::gui_widget_type_for_key_00aa2490("Icon")
                  == GuiWidgetType::Icon
              && bsp::gui_widget_type_for_key_00aa2490("Icon_")
                  == GuiWidgetType::None
              && bsp::gui_widget_type_for_key_00aa2490("States")
                  == GuiWidgetType::None,
            "00AA2490 takes the suffix after the last underscore, folded");
    }

    {
        // 00AA8450's fifth argument: a descendant takes the requested value only
        // while the widget the change started at recurses (widget+75h). Without
        // it a child that is hidden on its own node stays hidden when an
        // ancestor is shown, which is the rule the walk exists to enforce.
        struct TestHost final : GuiWidgetSceneHost {
            std::vector<GuiWidgetTransform*> widgets{};
            std::vector<GuiWidgetSceneFlags> state{};
            std::vector<float> factor{};
            std::vector<GuiWidgetTransform*> notified{};

            std::size_t index(GuiWidgetTransform& w) {
                for (std::size_t i = 0; i < widgets.size(); ++i) {
                    if (widgets[i] == &w) return i;
                }
                return 0;
            }
            GuiWidgetSceneFlags& flags(GuiWidgetTransform& w) override {
                return state[index(w)];
            }
            bool is_visible(GuiWidgetTransform& w) override {
                return widget_is_visible(flags(w), factor[index(w)]);
            }
            void on_effective_visibility_changed(
                GuiWidgetTransform& w, bool) override { notified.push_back(&w); }
            void set_node_visibility_factor(void*, float, bool) override {}
            void* create_scene_node(const char*) override { return nullptr; }
            void* clone_scene_node(void*, std::int32_t) override { return nullptr; }
            void set_node_parent(void*, void*) override {}
            void unlink_and_release_node(void*) override {}
            bool is_kind_of_glyph_owner(GuiWidgetTransform&) override { return false; }
            void release_secondary_node(GuiWidgetTransform&) override {}
            void set_active(GuiWidgetTransform&, bool) override {}
            void refresh_local_bounds(GuiWidgetTransform&) override {}
            void recompose_local_transform(GuiWidgetTransform&) override {}
            GuiWidgetTransform* create_widget_of_type(
                std::int32_t, GuiWidgetTransform&) override { return nullptr; }
            void link_child(GuiWidgetTransform&, GuiWidgetTransform&) override {}
        };

        GuiWidgetTransform root{};
        GuiWidgetTransform middle{};
        GuiWidgetTransform leaf{};
        root.children.push_back(&middle);
        middle.parent = &root;
        middle.children.push_back(&leaf);
        leaf.parent = &middle;

        int marker = 0;
        TestHost host{};
        host.widgets = {&root, &middle, &leaf};
        host.state.assign(3, GuiWidgetSceneFlags{});
        for (GuiWidgetSceneFlags& f : host.state) f.scene_node = &marker;
        // The root is hidden, the middle is visible, the leaf is hidden on its
        // own node.
        host.factor = {0.0f, 1.0f, 0.0f};

        set_widget_visible(root, true, host);
        check(host.notified.size() == 2 && host.notified[0] == &root
                && host.notified[1] == &middle,
            "showing a non-recursing widget leaves a leaf hidden on its own node");

        host.notified.clear();
        host.state[0].visibility_recurses = true;
        set_widget_visible(root, true, host);
        check(host.notified.size() == 3 && host.notified[2] == &leaf,
            "the recursing form carries the requested value to every descendant");
    }

    {
        // 00AB1680 reacts to exactly two authored UV_LURB patterns: (_,1,_,0)
        // swaps the resolved V pair and (1,_,0,_) swaps the U pair. Any other
        // authored rectangle, including a partial crop, is stored but leaves
        // the atlas rectangle alone. Getting either pattern backwards would
        // silently mirror 33 authored icons in the shipped pages, and the two
        // tests are independent, so the mixed case is the boundary.
        const bsp::GuiUvRect atlas{0.25f, 0.5f, 0.75f, 1.0f};
        bsp::GuiIconState flipped{};
        flipped.authored_uv = bsp::GuiUvRect{1.0f, 1.0f, 0.0f, 0.0f};
        bsp::gui_icon_set_resolved_uv_00ab1680(flipped, atlas);
        bsp::GuiIconState cropped{};
        cropped.authored_uv = bsp::GuiUvRect{0.0f, 0.25f, 1.0f, 0.75f};
        bsp::gui_icon_set_resolved_uv_00ab1680(cropped, atlas);
        check(flipped.resolved_uv.left == 0.75f
              && flipped.resolved_uv.right == 0.25f
              && flipped.resolved_uv.top == 1.0f
              && flipped.resolved_uv.bottom == 0.5f
              && cropped.resolved_uv.left == atlas.left
              && cropped.resolved_uv.top == atlas.top
              && cropped.resolved_uv.right == atlas.right
              && cropped.resolved_uv.bottom == atlas.bottom,
            "00AB1680 swaps only on the reversed UV_LURB patterns");
    }

    {
        // 00ABAED0 caches the narrow source and compares it case-insensitively,
        // so a widget whose key differs only in case is never re-resolved, while
        // 00A9F4B0 strips one leading '^' before the lookup and reports a
        // leading '.' as the marker that suppresses it. Both rules decide
        // whether a shipped DefaultText reaches the localisation table at all.
        bsp::GuiTextWidget widget;
        widget.source = "MENU_START";
        check(!bsp::source_text_changed_00abaed0(widget, "menu_start")
              && bsp::source_text_changed_00abaed0(widget, "MENU_STARTS"),
            "00ABAED0 caches the source key case-insensitively");
        const auto plain = bsp::split_localisation_key_00a9f4b0("MENU_START");
        const auto caret = bsp::split_localisation_key_00a9f4b0("^MENU_START");
        const auto marked = bsp::split_localisation_key_00a9f4b0("^.RAW");
        check(!plain.had_caret && plain.key == "MENU_START"
              && caret.had_caret && caret.key == "MENU_START"
              && !caret.suppresses_lookup
              && marked.had_caret && marked.suppresses_lookup
              && marked.key == ".RAW",
            "00A9F4B0 strips one caret before testing the lookup marker");
    }

    {
        // The three .scn tokenizer and recovery boundaries that decide whether
        // the shipped scene files load at all: the comma is whitespace rather
        // than a delimiter, sscanf("%f") accepts the prefix of "1.-", and a
        // property key followed by neither '=' nor '{' is dropped so that a
        // "--" prefixed line is applied rather than commented out (008f66b9).
        const std::string scene_text =
            "entity \"A\" (Landscape) {\n"
            "  localframe 1,0,0,0, 0,1,0,0, 0,0,1.-,0, 1,2,3,1 ;\n"
            "  properties (Common, Landscape) {\n"
            "    -- Skill = E SkillLevels : Stun ;\n"
            "    FilePath = S \"islands/\" \"a\" ;\n"
            R"(    GuiName = S "head" ";" "{" "}" "\n\q\\tail\x\N\Q" ; )" "\n"
            "    VecEmpty = V3 ;\n"
            "    VecFull = V3 1.25 -2.5 .75 ;\n"
            "    SceneGroupNum = I 7 ;\n"
            "  }\n"
            "}\n";
        const SceneDocument doc = parse_scene_document(scene_text);
        check(doc.errors.empty() && doc.entities.size() == 1,
            "the scene grammar accepts comma separators without a recovered error");
        if (doc.entities.size() == 1) {
            const SceneEntity& ent = doc.entities[0];
            check(ent.frame[10] == 1.0f, "sscanf float semantics accept the prefix of \"1.-\"");
            check(ent.groups.size() == 2 && ent.groups[1] == "Landscape",
                "commas in the property group list are whitespace, not group names");
            check(ent.properties.find("Skill") != nullptr
                    && ent.properties.find("--") == nullptr,
                "a \"--\" prefix drops only the junk key, leaving the property applied");
            const SceneProperty* path = ent.properties.find("FilePath");
            const SceneProperty* name = ent.properties.find("GuiName");
            const SceneProperty* following = ent.properties.find("SceneGroupNum");
            check(path != nullptr && path->values.size() == 1 && path->values[0] == "islands/a"
                    && name != nullptr && name->values.size() == 1
                    && name->values[0] == "head;{}\n\"\\tail"
                    && following != nullptr && following->values.size() == 1
                    && following->values[0] == "7",
                "008EFB00 joins quoted S runs, decodes exact-case escapes, treats quoted "
                "punctuation as data and retains the following key");
            const SceneProperty* empty = ent.properties.find("VecEmpty");
            const SceneProperty* full = ent.properties.find("VecFull");
            check(empty != nullptr && empty->has_vector3 && empty->values.empty()
                    && empty->vector3[0] == 0.0f && !std::signbit(empty->vector3[0])
                    && empty->vector3[1] == 0.0f && !std::signbit(empty->vector3[1])
                    && empty->vector3[2] == 0.0f && !std::signbit(empty->vector3[2])
                    && ent.properties.find("VecMissing") == nullptr
                    && full != nullptr && full->has_vector3 && full->values.size() == 3
                    && full->vector3[0] == 1.25f && full->vector3[1] == -2.5f
                    && full->vector3[2] == .75f
                    && following != nullptr && following->values.size() == 1
                    && following->values[0] == "7",
                "explicit V3 stores three floats, present-empty positive zeros differ "
                "from absence, and the following key survives");
        }
    }

    {
        // Chain 382: keep a captured enum word when its live symbol changes.
        struct EnumSource final : bsp::game::SceneNativeEnumSource {
            bsp::game::GameNativeStringProcess& strings = bsp::game::game_native_string_process();
            void* pool;
            bsp::NativeSceneEnumOwnerStorage* owner;
            EnumSource() {
                auto& pools = bsp::game::game_native_physical_pool_process();
                pools.initialize_enum_symbol_pool_once_00cc8a50();
                pool = pools.enum_symbol_pool_storage_00e17578();
                owner = static_cast<bsp::NativeSceneEnumOwnerStorage*>(bsp::singleton_lifetime_allocate(
                    {bsp::SingletonAllocationKind::object, 0x19c, 0x19c}));
                static constexpr char type = 'E';
                bsp::initialize_native_scene_enum_owner_008f4dd0(*owner, strings.raw_context(), &type);
                set_word(7);
            }
            ~EnumSource() {
                bsp::delete_native_scene_enum_owner_008f59c0(owner, 1, pool, strings.raw_context());
            }
            void set_word(std::uint32_t word) {
                bsp::NativeString key;
                bsp::construct_native_string_header_0041e870(&key, strings.raw_context(), "Alpha");
                bsp::insert_native_enum_symbol_word_008f2850(&owner->map, key, word, pool,
                    strings.raw_context(), strings.enum_empty_00e186ed(), strings.query_empty_00e17654());
                bsp::destroy_native_string_header_0041dd20(&key, strings.raw_context());
            }
            const void* known_enum_owner(const std::string& table, const std::string& symbol) override {
                return table == "Words" && symbol == "Alpha" ? owner : nullptr;
            }
        } enums;
        std::vector<std::string> errors;
        bsp::SceneLexer enum_text("{ Choice = E Words : Alpha ; }");
        const auto source = bsp::parse_scene_property_block_008f5a00(enum_text, errors);
        bsp::ScenePropertyBlock empty, captured;
        {
            bsp::game::SceneNumericPropertyMerge merge(empty, source, false, &enums);
            captured.values.push_back(merge.result(source.values.at(0)));
        }
        enums.set_word(9);
        bool passed = errors.empty() && captured.values[0].has_enum_value
            && captured.values[0].enum_value == 7;
        {
            bsp::game::SceneNumericPropertyMerge merge(empty, captured, false, &enums);
            passed = passed && merge.result(captured.values[0]).enum_value == 7;
        }
        for (const bool overwrite : {false, true}) {
            bsp::game::SceneNumericPropertyMerge merge(captured, source, overwrite, &enums);
            passed = passed && merge.result(overwrite ? source.values[0] : captured.values[0])
                .enum_value == (overwrite ? 9 : 7);
        }
        check(passed, "native enum copy preserves captured words across symbol changes, while fresh overwrite resolves again");
    }

    {
        // Chain616 forwards a type-5 record's payload through a RET8 helper
        // inside a RET0C setter. Exercise that ABI through the actual owning
        // merge, including old-name retirement and preservation of keep mode.
        std::vector<std::string> errors;
        bsp::SceneLexer prior_text("{ Link = R \"OldTarget\" ; }");
        bsp::SceneLexer source_text("{ link = R \"NextTarget\" ; }");
        const auto prior = bsp::parse_scene_property_block_008f5a00(prior_text, errors);
        const auto source = bsp::parse_scene_property_block_008f5a00(source_text, errors);
        bool passed = errors.empty() && prior.values.size() == 1 && source.values.size() == 1;
        if (passed) {
            for (const bool overwrite : {false, true}) {
                bsp::game::SceneNumericPropertyMerge merge(prior, source, overwrite);
                passed = passed && merge.handles(0);
                const auto result = merge.result(source.values[0]);
                passed = passed && result.values ==
                    std::vector<std::string>{overwrite ? "NextTarget" : "OldTarget"};
            }
        }
        check(passed, "reference merge retires the actual old target and preserves keep/overwrite through payload replacement");
    }

    {
        // Chain 330: one fixture covers authored V3A -> native owning bag ->
        // keep/overwrite/miss merge -> independent publication after teardown.
        const SceneDocument doc = parse_scene_document(
            "entity \"Array\" (Landscape) { "
            "localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1; properties { "
            "Route = V3A 2 (1.25 -0 .75) (-2 3 4); "
            "Short = V3A 2 (1 2 3); "
            "Bad = V3A 1 (1 x 3); "
            "Zero = V3A 0; After = I 7; } }");
        const ScenePropertyBlock& source = doc.entities.at(0).properties;
        const SceneProperty* route = source.find("Route");
        check(doc.errors.empty() && route != nullptr && route->has_vector3_array
                && route->vector3_array.size() == 2
                && std::signbit(route->vector3_array[0][1])
                && !source.find("Short")->has_vector3_array
                && !source.find("Bad")->has_vector3_array
                && !source.find("Zero")->has_vector3_array
                && source.find("After")->integer_value == 7,
            "V3A admits complete triples, preserves signed zero and bounds malformed statements");
        if (route != nullptr && route->has_vector3_array) {
            ScenePropertyBlock destination;
            SceneProperty old = *route;
            old.vector3_array = {{{9.0f, 8.0f, 7.0f}}};
            destination.values.push_back(old);
            SceneProperty kept, replaced, cloned;
            {
                bsp::game::SceneNumericPropertyMerge merge(destination, source, false);
                check(merge.handles(0) && !merge.handles(1) && !merge.handles(2),
                    "native V3A projection handles only the complete payload");
                kept = merge.result(old);
            }
            {
                bsp::game::SceneNumericPropertyMerge merge(destination, source, true);
                replaced = merge.result(*route);
            }
            {
                ScenePropertyBlock empty;
                bsp::game::SceneNumericPropertyMerge merge(empty, source, false);
                cloned = merge.result(*route);
            }
            check(kept.has_vector3_array && kept.vector3_array == old.vector3_array
                    && replaced.has_vector3_array && cloned.has_vector3_array
                    && replaced.vector3_array == route->vector3_array
                    && cloned.vector3_array == route->vector3_array
                    && std::signbit(replaced.vector3_array[0][1])
                    && std::signbit(cloned.vector3_array[0][1]),
                "native type-11 keep, assignment and clone publish owned bytes after both bags retire");
        }
    }

    {
        // Chain 425: FA parser -> actual type-9 owner -> keep/replace/clone,
        // with an independent published payload surviving both native bags.
        const SceneDocument doc = parse_scene_document(
            "entity \"Array\" (Landscape) { "
            "localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1; properties { "
            "Weights = FA 3 1.25 -0 .75; Short = FA 2 1; "
            "Bad = FA 1 x; Zero = FA 0; After = I 7; } }");
        const ScenePropertyBlock& source = doc.entities.at(0).properties;
        const SceneProperty* weights = source.find("Weights");
        check(doc.errors.empty() && weights != nullptr && weights->has_float_array
                && weights->float_array == std::vector<float>({1.25f, -0.0f, 0.75f})
                && std::signbit(weights->float_array[1])
                && !source.find("Short")->has_float_array
                && !source.find("Bad")->has_float_array
                && !source.find("Zero")->has_float_array
                && source.find("After")->integer_value == 7,
            "FA retains complete finite payloads and leaves malformed statements raw");
        if (weights != nullptr && weights->has_float_array) {
            ScenePropertyBlock destination;
            SceneProperty old = *weights;
            old.float_array = {9.0f};
            destination.values.push_back(old);
            SceneProperty kept, replaced, cloned;
            {
                bsp::game::SceneNumericPropertyMerge merge(destination, source, false);
                check(merge.handles(0) && !merge.handles(1) && !merge.handles(2),
                    "native FA projection selects complete payloads only");
                kept = merge.result(old);
            }
            {
                bsp::game::SceneNumericPropertyMerge merge(destination, source, true);
                replaced = merge.result(*weights);
            }
            {
                ScenePropertyBlock empty;
                bsp::game::SceneNumericPropertyMerge merge(empty, source, false);
                cloned = merge.result(*weights);
            }
            check(kept.has_float_array && kept.float_array == old.float_array
                    && replaced.has_float_array && cloned.has_float_array
                    && replaced.float_array == weights->float_array
                    && cloned.float_array == weights->float_array
                    && std::signbit(replaced.float_array[1])
                    && std::signbit(cloned.float_array[1]),
                "native type-9 keep, replacement and clone publish bytes after owner retirement");
        }
    }

    {
        // 004DE610 has no ocean failure branch. The "Ocean initialization
        // failed" literal at 004DF829 is copied into a pooled buffer and
        // released unread, and the block that builds it is guarded by the
        // scene record, not by any ocean result. Both branches of 004DF421
        // still produce an ocean owner: from the record's description when a
        // record exists, from the sky_001 literal when it does not.
        struct RecordingHost final : bsp::WorldConstructHost {
            bool has_record{true};
            bool literal_built{false};
            bool ocean_named{false};
            bool debug_render_flag() override { return false; }
            void set_renderer_budget(std::uint32_t) override {}
            std::uint32_t create_world(const bsp::WorldObjectLayout&) override { return 1; }
            void world_post_construct(std::uint32_t, int, int) override {}
            std::uint32_t create_scene_node(std::size_t, const std::string&) override { return 2; }
            std::uint32_t create_operator_node(std::size_t, const std::string&) override { return 3; }
            void publish_operator_node(std::uint32_t) override {}
            void set_camera_near_plane(std::uint32_t, float) override {}
            std::uint32_t create_operator_child(std::size_t) override { return 4; }
            void attach_operator_child(std::uint32_t, std::uint32_t) override {}
            void release_ref(std::uint32_t) override {}
            void construct_scene_services() override {}
            void construct_lighting() override {}
            std::uint32_t scene_record() override { return has_record ? 0x1000u : 0u; }
            float record_float(std::size_t) override { return 0.0f; }
            std::uint8_t record_byte(std::size_t) override { return 0; }
            std::uint32_t record_field(std::size_t offset) override {
                return 0x1000u + static_cast<std::uint32_t>(offset);
            }
            void set_world_parameter(const std::string&, const std::string&) override {}
            void set_world_parameter_from_record(const std::string&, std::uint32_t) override {}
            std::uint32_t create_ocean_owner(std::size_t, std::uint32_t, std::uint32_t) override {
                return 5;
            }
            std::uint32_t create_ocean_owner_named(
                std::size_t, std::uint32_t, const std::string& name) override {
                ocean_named = name == bsp::kDefaultSkyName;
                return 6;
            }
            void ocean_set_light(std::uint32_t, std::uint32_t) override {}
            void ocean_set_vector(std::size_t, int) override {}
            void ocean_set_scalar(std::size_t, int) override {}
            void ocean_set_quality(std::uint8_t) override {}
            std::uint32_t create_atmosphere(std::size_t) override { return 7; }
            void ocean_set_atmosphere(std::uint32_t, std::uint32_t) override {}
            void operator_set_atmosphere(std::uint32_t, std::uint32_t) override {}
            void atmosphere_add_layer(std::uint32_t, std::size_t, std::size_t) override {}
            std::uint32_t create_sky(std::size_t, std::uint32_t, std::uint8_t) override { return 8; }
            void sky_configure(std::uint32_t, std::uint32_t) override {}
            void build_unused_literal(const std::string& text) override {
                literal_built = text == bsp::kOceanInitFailedLiteral;
            }
            std::uint32_t create_channel_object(std::size_t, std::size_t) override { return 9; }
            void register_channel_object(std::uint32_t) override {}
            std::uint32_t create_tail_object(const bsp::TailConstruction& spec) override {
                return static_cast<std::uint32_t>(spec.game_offset) + 0x10000u;
            }
            void set_input_context(int, bool) override {}
        };
        RecordingHost with_record;
        const auto recorded = bsp::run_world_construct(with_record);
        RecordingHost without_record;
        without_record.has_record = false;
        const auto defaulted = bsp::run_world_construct(without_record);
        check(!bsp::ocean_failure_is_reported()
              && with_record.literal_built && recorded.ocean_owner != 0
              && recorded.ocean_from_scene_record
              && !without_record.literal_built && defaulted.ocean_owner != 0
              && without_record.ocean_named && !defaulted.ocean_from_scene_record
              && recorded.channel_objects == bsp::kChannelObjectCount
              && recorded.marker_manager == 0x121D4u,
            "004DE610 builds an ocean on both branches and never reports a failure");
    }

    {
        // 00BD63B0 and 00BD61C0 disagree on field type 0Ah: the value path runs
        // lua_tolstring and parses the text (00BD67F2), the default path takes
        // the fallback word as a float already (00BD62F0). Making the pair
        // symmetric in either direction would silently read every defaulted
        // 0Ah field as a pointer or every authored one as raw bits.
        float parsed_value = -1.0f;
        float defaulted = -1.0f;
        const bsp::GuiLuaVariant parsed_value_field =
            bsp::gui_lua_field(bsp::GuiLuaFieldType::ParsedFloat, &parsed_value);
        const bsp::GuiLuaVariant defaulted_field =
            bsp::gui_lua_field(bsp::GuiLuaFieldType::ParsedFloat, &defaulted);
        bsp::GuiLuaVariant fallback;
        fallback.value.number = 2.5f;
        const bool crt_sse2_conversion = true;
        check(bsp::gui_lua_store_value_00bd63b0(bsp::GuiValue(std::string("1.5")),
                                                parsed_value_field, nullptr, crt_sse2_conversion)
                  && parsed_value == 1.5f,
            "field type 0Ah parses the script's string");
        check(bsp::gui_lua_store_default_00bd61c0(defaulted_field, fallback)
                  && defaulted == 2.5f,
            "field type 0Ah takes its default as a plain float");
    }

    {
        // The scene class table decides whether an entity block dispatches at
        // all. 0046CF40 reads the descriptor out of the map node without a null
        // check, so an unregistered class token faults rather than being
        // skipped, and the lookup inside 00468FB0 is case-insensitive, which is
        // the only reason the three "Landfort" entities in the shipped files
        // resolve. Both properties have to hold together.
        check(bsp::scene_entity_class_id_from_name("Landfort") == 0x1b
                && bsp::scene_entity_class_name_from_id(0x1b) != nullptr,
            "the scene class lookup is case-insensitive in both directions");
        check(!bsp::scene_entity_class_is_registered("LandVehicle")
                && bsp::scene_entity_class_id_from_name("LandVehicle") == bsp::kSceneUnknownClassId
                && bsp::find_scene_entity_class_by_id(0x19) == nullptr,
            "an unregistered class token has no descriptor to dispatch on");
    }

    {
        // 004B6B20/004B6B30 nest, and 008890F0 plus the Loading_* gates at
        // 008CC93A and 008C78D4 all read the same "depth is at least one" rule.
        // The native counter is signed and unclamped, so an unbalanced leave
        // must go negative and stop suppressing.
        bsp::ScriptLoadGuard guard;
        const bool idle = !guard.active() && !bsp::loading_screen_calls_suppressed(guard.depth());
        guard.enter();
        guard.enter();
        const bool nested = guard.depth() == 2 && bsp::loading_screen_calls_suppressed(guard.depth());
        guard.leave();
        const bool still_held = guard.active();
        guard.adjust(false);
        const bool released = guard.depth() == 0 && !bsp::loading_screen_calls_suppressed(0);
        guard.leave();
        const bool underflows = guard.depth() == -1 && !guard.active();
        check(idle && nested && still_held && released && underflows,
            "game+644h suppresses the loading-screen bindings only while the depth is positive");
    }

    {
        // 00AA4960 descends left only on a strictly smaller key, so layers that
        // share a RenderOrder draw in creation order, and an empty widget name
        // sorts first without dereferencing its null character pointer.
        bsp::GuiCameraStore first{};
        bsp::GuiCameraStore second{};
        bsp::GuiCameraStore ahead{};
        first.key.render_order = 4.0f;
        second.key.render_order = 4.0f;
        ahead.key.render_order = -1.0f;
        bsp::GuiCameraStoreMap map;
        map.insert_00aa5070(first);
        map.insert_00aa5070(second);
        map.insert_00aa5070(ahead);
        check(map.entries().size() == 3 && map.entries()[0].second == &ahead
                  && map.entries()[1].second == &first
                  && map.entries()[2].second == &second,
            "equal RenderOrder keeps insertion order behind a smaller key");
        check(bsp::widget_name_less_00aa2c80("", "a")
                  && !bsp::widget_name_less_00aa2c80("", "")
                  && bsp::widget_name_less_00aa2c80("Alpha", "beta"),
            "empty widget names sort first and the rest fold case");
    }

    {
        // 007FC6C2 tests the named-counter result with JG, so a counter that
        // sits at exactly zero does not unlock, while 0090C560 on the
        // mission-completion map only tests non-zero. The two boundaries are
        // asymmetric and both feed the same any-of walk.
        bsp::ProfileUnlockState profile;
        profile.named_counters["RANK"] = 0;
        profile.mission_completion["IJN04"] = 1;
        check(!bsp::is_unlock_expression_satisfied_007fc4c0(profile, "RANK")
                && bsp::is_unlock_expression_satisfied_007fc4c0(profile, "rank, IJN04")
                && bsp::are_unlock_requirements_met_007fc820(profile, {}),
            "a zero named counter is locked, an any-of token and an empty vector are not");
        // 007F8890 must traverse real list links, gate CRT comparison by byte
        // length, and accept an empty ID without dereferencing its text.
        profile.content_ids = {"unrelated", "DL_Content_0000062", ""};
        check(bsp::classify_unlock_token_007fc4c0(profile, "dl_content_0000062")
                    == bsp::UnlockSource::kDownloadableContent
                && bsp::classify_unlock_token_007fc4c0(profile, "DL_Content_000006")
                    == bsp::UnlockSource::kNone
                && bsp::classify_unlock_token_007fc4c0(profile, "")
                    == bsp::UnlockSource::kDownloadableContent,
            "owned content uses the native list scan and exact length before case folding");
    }

    {
        // Chain 332's concrete risk: type-6 hits must recurse even with keep1,
        // while missing children clone and empty syntax retains inherited data.
        std::vector<std::string> errors;
        bsp::SceneLexer prior_text("{ Inner { Count = I 4 ; Retained = S \"inherited\" ; } "
            "Stay { Word = I 7 ; } }");
        bsp::SceneLexer source_text("{ inner { count = I 9 ; Added = I 12 ; } Stay {} NewEmpty {} }");
        const auto prior = bsp::parse_scene_property_block_008f5a00(prior_text, errors);
        const auto source = bsp::parse_scene_property_block_008f5a00(source_text, errors);
        bool passed = errors.empty();
        for (const bool overwrite : {false, true}) {
            bsp::game::SceneNumericPropertyMerge merge(prior, source, overwrite);
            bsp::ScenePropertyBlock result;
            const bool handled = merge.merge_child("Settings", &prior, source, result);
            passed = passed && handled && result.blocks.size() == 3;
            if (handled && result.blocks.size() == 3) {
                const auto& inner = result.blocks[0].second.values;
                passed = passed && result.blocks[0].first == "Inner" && inner.size() == 3;
                if (inner.size() == 3) {
                    passed = passed && inner[0].integer_value == (overwrite ? 9 : 4)
                        && inner[0].key == (overwrite ? "count" : "Count")
                        && inner[1].values == std::vector<std::string>{"inherited"}
                        && inner[2].integer_value == 12;
                }
                passed = passed && result.blocks[1].second.values.size() == 1
                    && result.blocks[1].second.values[0].integer_value == 7
                    && result.blocks[2].second.values.empty()
                    && result.blocks[2].second.blocks.empty();
            }
            bsp::ScenePropertyBlock missing;
            passed = passed && merge.merge_child("New", nullptr, source, missing)
                && missing.blocks.size() == 3
                && missing.blocks[0].second.values[0].integer_value == 9;
            auto unsupported = source;
            unsupported.blocks[0].second.values[0].has_integer = false;
            bsp::ScenePropertyBlock untouched = prior;
            passed = passed && !merge.merge_child("Raw", &prior, unsupported, untouched)
                && untouched.blocks.size() == prior.blocks.size();
        }
        check(passed, "native child bags recurse for keep/overwrite, clone misses, retain empty children and reject raw leaves");
    }

    {
        // Chain 620's ownership risk: replacing a library mapping must retire
        // the old nested tree, retain the new one, and preserve a single node.
        auto& pools = bsp::game::game_native_physical_pool_process();
        pools.initialize_property_node_pool_once_00cc8a30();
        pools.initialize_property_library_node_pool_once_00cc8a70();
        auto& strings = bsp::game::game_native_string_process();
        bsp::NativePropertyLibraryInsertContext insertion(strings, pools);
        bsp::NativePropertyLibraryReaderContext readers(strings);
        alignas(4) std::byte library[0x10c];
        bsp::initialize_native_property_tree_library_storage_008f5670(library);
        alignas(4) std::byte query[8];
        bsp::construct_native_string_header_0041e870(query, strings.raw_context(), "Group");
        bsp::game::SceneNumericPropertyMerge projection({}, {}, true);
        std::vector<std::string> errors;
        bsp::SceneLexer prior_text("{ Inner { Count = I 4 ; Text = S \"old\" ; } }");
        bsp::SceneLexer next_text("{ Inner { Count = I 9 ; Text = S \"new\" ; } }");
        const auto prior = bsp::parse_scene_property_block_008f5a00(prior_text, errors);
        const auto next = bsp::parse_scene_property_block_008f5a00(next_text, errors);
        void* first = projection.make_owned_complete_bag(prior);
        void* second = projection.make_owned_complete_bag(next);
        bool passed = errors.empty() && first != nullptr && second != nullptr && first != second;
        passed = bsp::insert_or_replace_native_property_library_entry_008f29a0(
            library + 4, query, first, insertion) == 1u && passed;
        passed = bsp::insert_or_replace_native_property_library_entry_008f29a0(
            library + 4, query, second, insertion) == 1u && passed;
        void* mapped = reinterpret_cast<void*>(bsp::read_native_property_library_word_00469b60(
            library, "gROUP", readers));
        // Deliberately use old metadata: native mapped bytes must supply values.
        const auto result = projection.read_complete_bag(mapped, prior);
        passed = passed && mapped == second && result.blocks.size() == 1
            && result.blocks[0].second.values.size() == 2
            && result.blocks[0].second.values[0].integer_value == 9
            && result.blocks[0].second.values[1].values == std::vector<std::string>{"new"};
        auto unsupported = next;
        unsupported.blocks[0].second.values[0].has_integer = false;
        passed = projection.make_owned_complete_bag(unsupported) == nullptr && passed;
        bsp::destroy_native_string_header_0041dd20(query, strings.raw_context());
        bsp::destroy_native_property_tree_library_008f56a0(library, insertion.lifetime());
        std::uint32_t count;
        std::memcpy(&count, library + 8, sizeof(count));
        check(passed && count == 0u,
            "owning library replacement reads new native payloads, rejects raw trees and retires its map");
    }

    {
        // 004E6B6D: the `Command` sub-bag queues a command only when it carries an
        // inner `Command` key. Every shipped unit entity has the sub-bag but most
        // carry only `CommandTarget = R "" ;`, so the presence of the block is not
        // what decides. Getting this backwards would queue ~10k empty commands.
        bsp::ScenePropertyBlock shipped;
        bsp::ScenePropertyBlock shipped_sub;
        bsp::SceneProperty target;
        target.key = "CommandTarget";
        target.type_letter = "R";
        target.values.push_back("");
        shipped_sub.values.push_back(target);
        shipped.blocks.emplace_back("Command", shipped_sub);
        const bsp::SceneUnitCommand quiet = bsp::scene_unit_command_004e6b30(shipped);

        bsp::ScenePropertyBlock authored;
        bsp::ScenePropertyBlock authored_sub = shipped_sub;
        authored_sub.values[0].values[0] = "Path1";
        bsp::SceneProperty command;
        command.key = "Command";
        command.type_letter = "E";
        command.values.push_back("CommandType");
        command.values.push_back(":");
        command.values.push_back("Cruise");
        authored_sub.values.push_back(command);
        authored.blocks.emplace_back("Command", authored_sub);
        const bsp::SceneUnitCommand queued = bsp::scene_unit_command_004e6b30(authored);

        check(!quiet.queued && queued.queued && queued.command == "Cruise" &&
                queued.target == "Path1",
            "004E6B30 queues on the inner Command key, not on the sub-block");

        authored_sub.values.back().values.back().clear();
        bsp::ScenePropertyBlock present_empty;
        present_empty.blocks.emplace_back("Command", authored_sub);
        const auto empty = bsp::scene_unit_command_004e6b30(present_empty);
        check(empty.queued && empty.command.empty() && empty.target == "Path1",
            "004E6B30 preserves present-empty Command and its target");
    }

    {
        // 00AA8BD0 resolves the pointer to the widget with the SMALLEST
        // size.x * scale.x among those containing it, not to the deepest, the
        // nearest in Z or the last in the child list. A parent that also
        // contains the pointer therefore loses to its own smaller child, and a
        // twin keeps the first candidate (the compare at 00AA8E0C is strict).
        bsp::GuiHitNode page{};
        page.visible = true;
        page.mouse_hit = true;
        page.size[0] = 0.5f;
        page.box = {0.0f, 1.0f, 0.0f, 1.0f};

        bsp::GuiHitNode inner{};
        inner.visible = true;
        inner.mouse_hit = true;
        inner.size[0] = 0.05f;
        inner.box = {0.4f, 0.6f, 0.4f, 0.6f};

        bsp::GuiHitNode twin{};
        twin.visible = true;
        twin.mouse_hit = true;
        twin.size[0] = 0.05f;
        twin.box = {0.4f, 0.6f, 0.4f, 0.6f};

        page.children.push_back(inner);
        page.children.push_back(twin);

        const float origin[3] = {0.0f, 0.0f, 0.0f};
        bsp::GuiHitResult result{};
        bsp::hit_test_widget_00aa8bd0(page, origin, 0.5f, 0.5f, result);
        const bool child_beats_parent = result.widget == &page.children[0];

        bsp::GuiHitNode hidden_child = page;
        hidden_child.children[0].visible = false;
        bsp::GuiHitResult skipped{};
        bsp::hit_test_widget_00aa8bd0(hidden_child, origin, 0.5f, 0.5f, skipped);
        const bool hidden_skipped = skipped.widget == &hidden_child.children[1];

        check(child_beats_parent && hidden_skipped,
            "the smallest half-width containing the pointer wins, ties keep the first");
    }

    {
        // 005C6DBE's two arms disagree about what "no size for this mode"
        // means, which is the one mission-tree default a caller would get
        // wrong by assuming a single fallback.
        struct MapSizeView : bsp::MissionTreeLuaView {
            bool table_present{true};
            void enter_by_name(std::string_view) override {}
            void enter_by_index(std::int32_t) override {}
            void leave() override {}
            bool has_name(std::string_view key) override {
                return key == "MultiPlayMapSizes" ? table_present : false;
            }
            bool has_index(std::int32_t) override { return false; }
            std::string read_string(std::string_view, std::string_view fallback) override {
                return std::string(fallback);
            }
            std::int32_t read_int(std::string_view, std::int32_t fallback) override {
                return fallback;
            }
            bool read_bool(std::string_view, bool fallback) override { return fallback; }
            std::array<float, 3> read_vec3(std::string_view,
                                           const std::array<float, 3>& fallback) override {
                return fallback;
            }
            std::vector<std::string> read_string_array(std::string_view) override { return {}; }
            std::vector<std::int32_t> read_int_array(std::string_view) override { return {}; }
            std::vector<std::string> string_keys() override { return {}; }
            std::vector<TableKey> table_keys() override { return {}; }
        };

        MapSizeView absent;
        absent.table_present = false;
        bsp::MissionRecordData without{};
        bsp::read_mission_record_metadata(absent, without);

        MapSizeView present;
        bsp::MissionRecordData with{};
        bsp::read_mission_record_metadata(present, with);

        check(without.extra.map_sizes[0].north_west == bsp::kMissionMapDefaultNorthWest
                  && without.extra.map_sizes[0].south_east == bsp::kMissionMapDefaultSouthEast
                  && with.extra.map_sizes[0].north_west == bsp::kMissionMapMissingCorner
                  && with.screen.difficulty == bsp::kMissionDifficultyFromPlayer,
            "a missing MultiPlayMapSizes is the +/-15000 box while a present one "
            "defaults each absent corner to the origin");
    }

    {
        // The vehicle-class index map is two 800h-entry arrays reset to the
        // identity (00592652), with a single pair rewritten (00592667). The
        // identity default is what lets 0095BA60 map an already-resolved class
        // index a second time without changing it.
        bsp::VehicleClassIndexMap map;
        map.reset_identity_00592652();
        map.remap_00592667(261, 121);
        check(map.to_class_index(1) == 1 && map.to_type_id(1) == 1
                  && map.to_class_index(261) == 121 && map.to_type_id(121) == 261
                  && map.to_class_index(bsp::kVehicleClassIndexMapSize) == -1,
            "the class index map is the identity apart from one remapped pair");
    }

    {
        // 0068ACA0. Every screen id any in-session arm publishes to level 1 must
        // be one of the 42 screens the manager itself constructed in 0068CC70;
        // a level-1 id the manager does not own would leave the HUD arm pointing
        // at another manager's registry slot. This guards the transcription of
        // both tables at once.
        bool every_id_owned = true;
        for (int id = bsp::kFirstInGameInterface; id <= bsp::kLastInGameInterface; ++id) {
            const bsp::InGameInterfaceScreenSet set = bsp::in_game_interface_screen_set(id);
            for (std::size_t i = 0; i < set.screen_count; ++i) {
                if (bsp::in_game_hud_screen_index_for_slot(set.screen_ids[i])
                    == bsp::kInGameHudScreenCount) {
                    every_id_owned = false;
                }
            }
        }
        check(every_id_owned
                  && bsp::in_game_interface_screen_set(bsp::kInterfaceMap).sets_screen_set
                  && !bsp::in_game_interface_screen_set(bsp::kInterfaceAirbase).sets_screen_set,
            "every level-1 screen id in the in-session map names a screen the "
            "manager owns, 21h clears level 1 and 32h leaves it alone");
    }

    {
        // 0042AC60's distance test is strict (FCOMI then JBE at 0042AC8A), so a step
        // exactly equal to the remaining distance steps instead of snapping, and it
        // lands on the target only by arithmetic. 00956600's fade is asymmetric around
        // that: rising is floored at zero and capped at the target, falling is clamped
        // into [target, 1] by 00415690. Both are the results every sub-update in
        // docs/UNIT_TIMED_SUBUPDATES.md is built out of.
        check(bsp::unit_step_towards_0042ac60(0.0f, 1.0f, 1.0f) == 1.0f
                  && bsp::unit_step_towards_0042ac60(0.0f, 1.0f, 2.0f) == 1.0f
                  && bsp::unit_step_towards_0042ac60(1.0f, -1.0f, 0.5f) == 0.5f
                  && bsp::unit_step_towards_0042ac60(1.0f, 1.0f, 0.25f) == 1.0f
                  && bsp::unit_step_fade_009569f8(0.9f, 1.0f, 1.0f) == 1.0f
                  && bsp::unit_step_fade_009569f8(0.1f, 0.0f, 1.0f) == 0.0f
                  && bsp::unit_step_fade_009569f8(0.5f, 0.5f, 1.0f) == 0.5f,
            "the unit rate limiter steps on an exactly equal step and the fade clamps "
            "to its target on both directions");
    }

    {
        // 009606F7 uses strcmp for "FlyingControll" but the case-insensitive
        // 00425850 for "BomberPilot", so only the second tolerates a case change.
        check(bsp::vehicle_class_spec_role_009606bc("BomberPilot")
                  == bsp::kVehicleClassSpecRoleBomberPilot
              && bsp::vehicle_class_spec_role_009606bc("bomberpilot")
                  == bsp::kVehicleClassSpecRoleBomberPilot
              && bsp::vehicle_class_spec_role_009606bc("flyingcontroll")
                  == bsp::kVehicleClassSpecRoleDefault
              && bsp::vehicle_class_spec_role_009606bc(nullptr)
                  == bsp::kVehicleClassSpecRoleDefault,
            "SpecRole matches BomberPilot case-insensitively and FlyingControll exactly");
    }

    {
        // The HUD page table and the per-screen layouts are generated from the
        // same recovered map, so the risk is that they drift apart: a layout
        // naming a page the page table does not carry, or a registry slot the
        // manager does not own. docs/HUD_SCREEN_PAGES.md.
        bool closed = true;
        for (const bsp::HudScreenLayout& layout : bsp::kHudScreenLayouts) {
            if (bsp::in_game_hud_screen_index_for_slot(layout.registry_slot)
                == bsp::kInGameHudScreenCount) {
                closed = false;
            }
            for (std::size_t i = 0; i < layout.page_count; ++i) {
                if (bsp::hud_screen_page(layout.pages[i]) == nullptr) {
                    closed = false;
                }
            }
        }
        check(closed
                  && bsp::hud_screen_layout_for_slot(0x4D) != nullptr
                  && bsp::hud_screen_slot_raised_by_interface(0x4D, bsp::kInterfaceBombView)
                  && !bsp::hud_screen_slot_raised_by_interface(0x4D, bsp::kInterfaceLimbo),
            "every HUD screen layout names a slot the manager owns and pages the "
            "page table carries, and slot 4Dh rises with 26h but not 34h");
    }


    {
        // 009329C0 clamps a hull element's submersion before it ever scales a force:
        // cap the height above water at the element's span, floor it at zero, then
        // subtract from the span (00932D90..00932E42). A point far above the surface
        // is therefore fully dry rather than negatively buoyant, and a point far below
        // saturates at the span instead of growing without bound.
        const bsp::UnitHullBuoyancyElement element{1.0f, 0.0f, 3.0f, 0.0f, {}};
        check(bsp::unit_hull_submersion_009329c0(element, 10.0f).submerged == 0.0f
                  && bsp::unit_hull_submersion_009329c0(element, -10.0f).submerged == 3.0f
                  && bsp::unit_hull_submersion_009329c0(element, 1.0f).submerged == 2.0f,
            "the hull submersion clamp saturates dry at zero and wet at the element span");
    }

    {
        // 00831E60..00831FEC chains the ship camera defaults in body order:
        // CameraDistanceFront falls back to the base Length, both Side and
        // Vertical to Front rather than to each other, and CameraMinHeight to
        // CaptainCameraHeight. Only Front is present here, so the chain has to
        // carry it into both of the others.
        bsp::ShipClassCameraInputs in;
        in.base_length = 250.0f;
        in.captain_camera_global = 12.0f;
        in.has_distance_front = true;
        in.distance_front = 40.0f;
        const bsp::ShipClassCameraFields camera = bsp::ship_class_camera_00831e0d(in);
        check(camera.distance_front == 40.0f && camera.distance_side == 40.0f
                  && camera.distance_vertical == 40.0f
                  && camera.captain_camera_height == 12.0f && camera.min_height == 12.0f,
            "the ship camera defaults chain from CameraDistanceFront and "
            "CaptainCameraHeight, not from the neighbouring slot");
    }

    {
        // The HUD root power-up column: the row accumulator adds the template
        // height and 1/72 before the row is placed, the circle clone ignores
        // the accumulator and sits one 1/120 above the shared column anchor,
        // and the two clones sit on different depth layers. Losing any of that
        // stacks every icon on the first row or hides the circle behind the
        // icon, and none of it is visible in the pseudocode, which aliases the
        // accumulator to the loop counter. docs/HUD_CENTRAL_UPDATES.md.
        auto close_to = [](float a, float b) { return std::fabs(a - b) < 1.0e-5f; };
        const float first = bsp::hud_root_advance_row_y(
            bsp::hud_root_first_row_y(false), 0.05f);
        const float second = bsp::hud_root_advance_row_y(first, 0.05f);
        const bsp::HudGuiPoint icon = bsp::hud_root_icon_position(second);
        const bsp::HudGuiPoint circle = bsp::hud_root_circle_position();
        check(close_to(first, 0.05f + 1.0f / 72.0f)
                  && close_to(second, 2.0f * (0.05f + 1.0f / 72.0f))
                  && close_to(bsp::hud_root_first_row_y(true), 17.0f / 120.0f)
                  && close_to(icon.x, circle.x)
                  && close_to(circle.y, bsp::kHudRootColumnAnchor - 1.0f / 120.0f)
                  && icon.z < circle.z
                  && close_to(bsp::hud_root_circle_fill(12.0f, 4.0f, 16.0f), 0.5f)
                  && !bsp::hud_root_timed_entry_active(4.0f, 4.0f),
            "the HUD root power-up column stacks rows by height plus 1/72, pins "
            "the circle clone to the anchor and keeps it in front of the icon");
    }

    {
        // 007D3DA4 re-reads TurboStrength after all four turbo keys are stored
        // and zeroes TurboTime unless the strength is strictly above 1.0f. The
        // shipped default for TurboStrength is itself 1.0f, so a row that gives
        // a TurboTime but no TurboStrength ends up with turbo disabled.
        check(!bsp::plane_class_turbo_enabled_007d3da4(1.0f)
                  && !bsp::plane_class_turbo_enabled_007d3da4(0.5f)
                  && bsp::plane_class_turbo_enabled_007d3da4(1.5f),
            "the plane turbo gate wants a TurboStrength strictly above 1.0f, so "
            "the 1.0f default zeroes a shipped TurboTime");
    }

    {
        // 0092D300 turns a commanded speed into a velocity, and the acceleration limit
        // is what keeps a throttle change from teleporting the hull: the step is
        // accel*dt and only bites while it is no larger than the remaining gap
        // (0092D43A..0092D45B). docs/UNIT_FORCE_COMMANDS.md.
        bsp::UnitAxialSpeedInputs in{};
        in.axis = {0.0f, 0.0f, 1.0f};
        in.velocity = {0.0f, 0.0f, 0.0f};
        in.commanded_speed = 10.0f;
        in.drive_accel = 2.0f;
        in.brake_accel = 2.0f;
        in.dt = 1.0f;
        const bsp::UnitAxialSpeedStep limited = bsp::unit_approach_axial_speed_0092d300(in);
        in.dt = 100.0f;
        const bsp::UnitAxialSpeedStep snapped = bsp::unit_approach_axial_speed_0092d300(in);
        check(limited.target_speed == 2.0f && limited.velocity.z == 2.0f
                  && snapped.target_speed == 10.0f && snapped.velocity.y == 0.0f,
            "the axial speed command steps by accel*dt and snaps only once the step "
            "covers the gap, leaving the vertical velocity alone");
    }

    {
        // 00987590's two boundaries are easy to get backwards. The periodic pass
        // fires on strictly greater than 4.0f and 009875D1 then stores zero rather
        // than subtracting, so a long frame loses its overshoot; and the retire test
        // at 00987677 is FCOMIP then JBE, so a warning whose lifetime has exactly
        // elapsed is already expired. docs/MISSION_EVENTS_UPDATE.md.
        float accumulator = 0.0f;
        const bool at_period = bsp::advance_warning_periodic(accumulator, 4.0f);
        float overshoot = 3.0f;
        const bool past_period = bsp::advance_warning_periodic(overshoot, 7.5f);
        check(!at_period && accumulator == 4.0f && past_period && overshoot == 0.0f
                  && bsp::warning_is_expired(10.0f, 4.0f, 6.0f)
                  && !bsp::warning_is_expired(10.0f, 4.0f, 6.5f),
            "the warning period fires above 4.0f and resets to zero instead of "
            "subtracting, and an exactly elapsed lifetime already retires");
    }

    {
        // 00959468 is COMISS against the 1.0f at 00d7a24c followed by JBE, so a
        // unit destroyed at exactly one second is still inside the mission-start
        // grace period and is not reported; and 0090646a reads the completion
        // flag before the store, so only the 0 -> set edge stamps the clock.
        // docs/MISSION_RESULT_DECISION.md.
        bsp::UnitDeathInputs at_grace{};
        at_grace.mission_clock = 1.0f;
        at_grace.unit_party_70 = 1;
        at_grace.world_gate_4ac = true;
        bsp::UnitDeathInputs past_grace = at_grace;
        past_grace.mission_clock = 1.0000001f;
        bsp::MissionScoreRecord record{};
        const bool first = bsp::set_slot_mission_completed_00906460(record, true, 42.0f);
        const bool again = bsp::set_slot_mission_completed_00906460(record, true, 99.0f);
        check(!bsp::unit_death_00959450(at_grace).reports_kill
                  && bsp::unit_death_00959450(past_grace).reports_kill && first && !again
                  && record.completion_time_10 == 42.0f,
            "a unit death at exactly the 1.0f grace boundary is not reported, and the "
            "completion timestamp is stamped only on the first completion");
    }

    {
        // 0091A080, the merge 009205E0 runs (kMissionRecordMergeBound): two
        // commits sum +1E0h and keep the max +4, where the copy model keeps
        // only the second record's values (OFF would give 50 and 1).
        bsp::MissionScoreRecord a{};
        a.mission_completed_00 = 1;
        a.ranking_04 = 3;
        a.totals_1c8[6] = 100;
        bsp::MissionScoreRecord b = a;
        b.ranking_04 = 1;
        b.totals_1c8[6] = 50;
        bsp::MissionScoreRecord stored{};
        bsp::merge_mission_score_record_0091a080(stored, a);
        bsp::merge_mission_score_record_0091a080(stored, b);
        check(stored.totals_1c8[6] == 150 && stored.ranking_04 == 3,
            "two 0091A080 merges sum the +1E0h total and keep the larger +4 ranking");
    }

    {
        // docs/UNIT_STATE_MESSAGE.md. The packet's conclusion rests on one
        // interaction: a MT_SHIP_SYNC back-fill (00812FA0) writes ring slots only,
        // and unit+980h/+984h move solely through the rate-limited step inside
        // 00813020. A client (session mode 2) must also keep the four-tick lag the
        // constructor seeds, and the confirmed triple must advance only once an
        // authoritative slot sits under the read cursor.
        bsp::UnitOrderRing ring{};
        bsp::construct_unit_order_ring_00812d40(ring);
        bsp::backfill_unit_order_ring_00812fa0(ring, 1.0f, -0.5f, 3, 4);
        const bool untouched_by_backfill
            = ring.current_param_a == 0.0f && ring.current_param_b == 0.0f
            && ring.read_cursor == 0 && ring.write_cursor == 4 && !ring.slot[0].predicted
            && !ring.slot[4].predicted && ring.slot[5].predicted;
        // Slew 8.0f over a 0.05f tick is 0.4f, under the 1.0f target, so the tick
        // steps rather than snapping, and the client lag stays at four slots.
        bsp::tick_unit_order_ring_00813020(ring, 0.05f, bsp::kUnitOrderRingClientSessionMode);
        check(untouched_by_backfill && ring.current_param_a == 0.4f
                  && ring.current_param_b == -0.1f && ring.read_cursor == 1
                  && ring.write_cursor == 5 && ring.slot[5].predicted
                  && ring.confirmed_param_a == 0.4f && ring.confirmed_kind == 3,
            "a ship-sync back-fill leaves unit+980h/+984h alone and the ring tick steps "
            "them toward the clamped slot while holding the client lag");
    }

    {
        // docs/LUA_BINDING_ENTITY_LOOKUP.md. Two rules the lookup gets wrong easily:
        // 0088B1E0 CMP EBX,0x47 / JA is an *unsigned* bound, so the negative buckets every
        // entity registers on are never searched; and 0088B213..0088B235 wants +5Ch set with
        // +5Dh, +5Eh and +60h all clear, so a destroyed entity on a searched bucket is skipped
        // and the next match wins.
        struct Lists : bsp::MissionEntityListView {
            bsp::MissionEntityCandidate dead{};
            bsp::MissionEntityCandidate live{};
            bsp::MissionEntityCandidate hidden{};
            std::size_t bucket_size(std::int32_t kind) const override {
                if (kind == -1) return 1;      // a base bucket, outside the unsigned bound
                if (kind == 0x00) return 2;    // the unit bucket
                return 0;
            }
            bsp::MissionEntityCandidate bucket_entry(std::int32_t kind,
                                                     std::size_t index) const override {
                if (kind == -1) return hidden;
                return index == 0 ? dead : live;
            }
        } lists;
        int first = 0, second = 0, unreachable = 0;
        lists.dead.entity = &first;
        lists.dead.name = "Carrier";
        lists.dead.active = true;
        lists.dead.released = true;              // byte +5Dh
        lists.live.entity = &second;
        lists.live.name = "CARRIER";             // __stricmp, case-insensitive
        lists.live.active = true;
        lists.hidden.entity = &unreachable;
        lists.hidden.name = "Carrier";
        lists.hidden.active = true;
        const void* found = bsp::mission_entity_find_by_name(lists, "carrier", 7);
        check(found == &second && !bsp::entity_kind_is_searched(-1)
                  && bsp::entity_kind_is_searched(0x47) && !bsp::entity_kind_is_searched(0x48)
                  && bsp::world_bucket_head_offset(0x00) == 0x64,
            "the name lookup skips the released candidate and the negative buckets, and "
            "matches case-insensitively on the first live entity of a searched kind");
    }

    {
        // One installed property line, `Vehicles = IA 8 17 19 20 21 23 68 73 109 ;`
        // from the shipped .scn files. The leading 8 is the element count that
        // 008F636C/008F650C reads before the loop, not a value: a reader that
        // scanned to the `;` would report nine elements.
        bsp::ScenePropertyType type{};
        bsp::SceneReferenceKind kind{};
        const bool letter_is_int_array
            = bsp::scene_property_type_for_letter("IA", type, kind)
            && type == bsp::ScenePropertyType::IntArray;
        const std::vector<std::string> tokens{
            "8", "17", "19", "20", "21", "23", "68", "73", "109"};
        bsp::ScenePropertyValue value{};
        const bool decoded = bsp::scene_decode_property_value(
            bsp::ScenePropertyType::IntArray, kind, tokens, value);
        check(letter_is_int_array && decoded && value.int_array.size() == 8
                  && value.int_array.front() == 17 && value.int_array.back() == 109
                  && bsp::scene_property_array_bytes(
                         bsp::ScenePropertyType::IntArray, 8) == 32,
            "an installed `IA` property line decodes its leading token as the element "
            "count and yields eight values");
    }

    {
        // 00887750's nargs accounting, which is easy to "correct" the wrong way.
        // EBX is cleared at 00887780 and only the self-key block sets it to one,
        // 0088793C adds the record count verbatim even for a tag-4 record that
        // pushes nothing, and stack_first == 0 is the sentinel at 008877F2 that
        // also stops stack_last from being normalised.
        const bsp::MissionLuaStackRange none = bsp::resolve_named_call_stack_range(0, -1, 12);
        const bsp::MissionLuaStackRange relative = bsp::resolve_named_call_stack_range(-3, -1, 12);
        std::vector<bsp::MissionLuaArgument> arguments(2);
        arguments[0].type = bsp::MissionLuaArgumentType::Number;
        arguments[1].type = bsp::MissionLuaArgumentType::Skipped;
        const bsp::NamedCallArgumentCounts without_self
            = bsp::named_call_argument_count(false, arguments, none);
        const bsp::NamedCallArgumentCounts with_self
            = bsp::named_call_argument_count(true, arguments, relative);
        check(!none.active && none.count == 0 && relative.active && relative.first == 10
                  && relative.last == 12 && relative.count == 3 && without_self.declared == 2
                  && without_self.pushed == 1 && with_self.declared == 6 && with_self.pushed == 5,
            "the named call counts a skipped record in nargs but not on the stack, starts at zero "
            "without a self key, and treats stack_first zero as the no-forwarding sentinel");
    }

    {
        // 0081F1A2..0081F1C3: the instance keeps the pre-increment value and the
        // global 00F87151 wraps only once it exceeds 0Bh, so twelve consecutive
        // instances take 0..11 and the thirteenth takes 0 again.
        const bsp::UnitSlotCounterStep last = bsp::unit_slot_counter_step(11);
        const bsp::UnitSlotCounterStep before = bsp::unit_slot_counter_step(10);
        check(last.stored == 11 && last.next == 0 && before.stored == 10
                  && before.next == 11,
            "the unit construction slot counter stores the pre-increment value and "
            "wraps after 11, not at it");
    }

    {
        // The installed Submarine row VehicleClass[8], comment "I400", is the one
        // shipped submarine with no PeriscopeDepth key, so it is the row that
        // exercises the alias branch 00854332 guards: with +810h left at the
        // -1.0f default, the reader goes on to ask for SwimDepth1, which no
        // shipped row provides either, and the slot keeps -1.0f. The row does
        // provide UpSpeed and DownSpeed (1.1) and provides none of UpDownAccel,
        // UpDownRotation or UpDownStopTime, which therefore take their literals.
        bsp::VehicleClassLuaValue absent{};
        bsp::VehicleClassLuaValue up_speed{};
        up_speed.kind = bsp::VehicleClassValueKind::Number;
        up_speed.number = 1.1;

        const float periscope_depth = bsp::vehicle_class_number_or_00b66330(
            absent, bsp::ShipLeafDefaults::kAbsentDepth);
        const bool alias_taken = bsp::submarine_reads_swim_depth1(periscope_depth);
        check(!bsp::submarine_reads_swim_depth1(std::numeric_limits<float>::quiet_NaN()),
            "0085433C JBE skips SwimDepth1 for an unordered periscope depth");
        const float after_alias = alias_taken
            ? bsp::vehicle_class_number_or_00b66330(absent,
                                                    bsp::ShipLeafDefaults::kAbsentDepth)
            : periscope_depth;

        const bsp::ShipLeafFieldSpec* swim1 = bsp::ship_leaf_find_field(
            bsp::ShipLeafClass::Submarine, "SwimDepth1");
        const bsp::ShipLeafFieldSpec* depth = bsp::ship_leaf_find_field(
            bsp::ShipLeafClass::Submarine, "PeriscopeDepth");
        check(alias_taken && after_alias == bsp::ShipLeafDefaults::kAbsentDepth
                  && swim1 != nullptr && depth != nullptr
                  && swim1->offset == depth->offset
                  && bsp::vehicle_class_number_or_00b66330(up_speed, 0.0F) > 1.0F
                  && bsp::vehicle_class_number_or_00b66330(
                         absent, bsp::ShipLeafDefaults::kUpDownRotation)
                         == bsp::ShipLeafDefaults::kUpDownRotation,
            "the installed I400 submarine row leaves +810h at -1.0f through the "
            "SwimDepth1 alias branch and takes the literal UpDownRotation default");
    }

    {
        // 00929460 never re-arms +1E0h, so an entity whose delay has run out stops
        // being a timed entry and falls onto the shared countdown at 00F89A04. The
        // risk is reading 009294BC as "expired means fire again"; it means the
        // opposite, the entry waits for the three-second pass.
        struct CountingHost : bsp::EntityThinkHost {
            int thinks = 0;
            void run_entity_think_00929150(std::uint32_t) override { ++thinks; }
            void free_think_node_0092952c(const bsp::EntityThinkNode&) override {}
            bool gc_gate_predicate_0109cefc_vtable0c() override { return false; }
            void lua_run_string_006b8ad0(const char*, int) override {}
            void splice_pending_into_live_00928380(bsp::EntityThinkList& live,
                const bsp::EntityThinkList& pending) override {
                live.nodes.insert(live.nodes.end(), pending.nodes.begin(), pending.nodes.end());
            }
            void clear_pending_00928330(bsp::EntityThinkList& pending) override {
                pending.nodes.clear();
            }
        } host;
        bsp::EntityThinkList live;
        bsp::EntityThinkList pending;
        bsp::register_pending_think_entity_0088a240(pending, 0x1000u);
        std::vector<bsp::EntityThinkFields> fields(1);
        fields[0].entity = 0x1000u;
        fields[0].initialised = true;
        fields[0].has_think_name = true;
        fields[0].delay_armed = true;
        fields[0].delay_seconds = bsp::clamp_think_delay(0.1f); // clamped to 0.5f at 008982B0
        float countdown = 2.0f;
        const float step = 0.05f;
        int first_fire = -1;
        for (int call = 0; call < 20; ++call) {
            // The native decrements +1E0h only while walking the node, and a
            // registration is spliced in at the end of the call that made it.
            const bool walked = !live.nodes.empty();
            const bsp::EntityThinkRunSummary summary = bsp::run_entity_think_list_00929460(
                step, countdown, live, pending, fields, host);
            if (summary.thinks_run != 0 && first_fire < 0) first_fire = call;
            if (walked && fields[0].delay_armed && fields[0].delay_seconds > 0.0f) {
                fields[0].delay_seconds
                    = bsp::entity_think_delay_after_step(fields[0].delay_seconds, step);
            }
        }
        const int after_timeout = host.thinks;
        countdown = -0.01f; // the pass 00929542 tests, before the 3.0 refill
        const bsp::EntityThinkRunSummary expired = bsp::run_entity_think_list_00929460(
            step, countdown, live, pending, fields, host);
        check(first_fire == 10 && after_timeout == 1 && expired.thinks_run == 1
                  && expired.countdown_expired
                  && std::fabs(countdown - (3.0f - 0.06f)) < 1e-5f,
            "an armed think fires once when its clamped delay runs out, then waits for the "
            "three-second script countdown instead of firing every step");
    }

    {
        // 007D20F3..007D2144. The installed Scripts\datatables\PlaneGlobals.lua sets
        // Dynamics/AccelCheatMul = 1.5 and AccelCheatMulMul = 1.15, so the plane class
        // reader takes the scaling branch and 007D213C never runs. Below 1.0f the clamp
        // runs, writes exactly 1.0f and is idempotent, which is why it is a guard and
        // not a progressive per-class mutation of shared tuning.
        bsp::GameTuningBlock block{};
        block.dynamics_accel_cheat_mul = 1.5F;
        block.dynamics_accel_cheat_mul_mul = 1.15F;
        const float scaled = bsp::game_tuning_apply_accel_cheat_007d20f3(block, 2.0F);

        bsp::GameTuningBlock low{};
        low.dynamics_accel_cheat_mul = 0.8F;
        low.dynamics_accel_cheat_mul_mul = 1.15F;
        const float first = bsp::game_tuning_apply_accel_cheat_007d20f3(low, 2.0F);
        const float clamped_once = low.dynamics_accel_cheat_mul;
        const float second = bsp::game_tuning_apply_accel_cheat_007d20f3(low, 2.0F);

        check(scaled == 1.5F * 1.15F * 2.0F
                  && block.dynamics_accel_cheat_mul == 1.5F
                  && first == 2.0F && clamped_once == 1.0F
                  && second == 2.0F && low.dynamics_accel_cheat_mul == 1.0F
                  && offsetof(bsp::GameTuningBlock, dynamics_accel_cheat_mul)
                         == bsp::kGameTuningAccelCheatMul,
            "the installed AccelCheatMul 1.5 scales Accel and leaves the tuning block "
            "untouched, and a value below 1.0f clamps to exactly 1.0f idempotently");
    }

    {
        // 00951560 leaves slot 0 out of the free list, so the table's first id
        // is reserved; 009517C0 pops the free list's tail while 009516D0 pushes
        // a released id at its head, so a recycled id is only reissued after
        // every never-used one. Both halves are easy to invert.
        bsp::EntityIdTable table = bsp::entity_id_table_construct_00951660(0, 4);
        const char entity_a = 0, entity_b = 0, entity_c = 0, entity_d = 0;
        const std::uint16_t first = bsp::entity_id_table_allocate_009517c0(table, 0, &entity_a);
        const std::uint16_t second = bsp::entity_id_table_allocate_009517c0(table, 0, &entity_b);
        bsp::entity_id_table_release_009516d0(table, first);
        const std::uint16_t third = bsp::entity_id_table_allocate_009517c0(table, 0, &entity_c);
        const std::uint16_t recycled = bsp::entity_id_table_allocate_009517c0(table, 0, &entity_d);
        check(first == 1 && second == 2 && third == 3 && recycled == first
                  && bsp::entity_id_table_lookup(table, recycled) == &entity_d
                  && bsp::entity_id_table_lookup(table, 0) == nullptr,
            "the entity id table reserves its first id, hands out never-used ids in ascending "
            "order and only then reissues a released one");
    }

    {
        // 009041B5..009041C2. The drain's membership test decides whether the
        // header is rewritten at all: a node with no predecessor and no
        // successor is still on the chain when the count is 1, and is not on it
        // when the count is higher. Reading that test as a plain "unlink the
        // head" loses the second case and corrupts the count.
        struct DrainHost final : bsp::WorldDeferredDestroyHost {
            std::uint32_t prev[4]{};
            std::uint32_t next[4]{};
            std::vector<std::uint32_t> destroyed;
            bsp::DeferredDestroyChain* chain{nullptr};
            std::uint32_t detach_on_destroy{0};
            std::uint32_t prev_sibling(std::uint32_t n) override { return prev[n]; }
            std::uint32_t next_sibling(std::uint32_t n) override { return next[n]; }
            void set_prev_sibling(std::uint32_t n, std::uint32_t v) override { prev[n] = v; }
            void set_next_sibling(std::uint32_t n, std::uint32_t v) override { next[n] = v; }
            void destroy_node(std::uint32_t n) override
            {
                destroyed.push_back(n);
                // The native destructor removes the node itself when the loop's
                // membership test declined to.
                if (n == detach_on_destroy && chain != nullptr) {
                    chain->head = next[n] != 0 ? next[n] : 2u;
                    chain->count -= 1;
                }
            }
        };

        bsp::DeferredDestroyChain linked_chain{1, 2, 2};
        DrainHost linked_host;
        linked_host.next[1] = 2;
        linked_host.prev[2] = 1;
        const bsp::WorldDeferredDestroyResult linked
            = bsp::drain_entity_chain_009041a0(linked_host, linked_chain);

        bsp::DeferredDestroyChain detached_chain{1, 2, 2};
        DrainHost detached_host; // node 1 carries neither link while count is 2
        detached_host.chain = &detached_chain;
        detached_host.detach_on_destroy = 1;
        const bsp::WorldDeferredDestroyResult detached
            = bsp::drain_entity_chain_009041a0(detached_host, detached_chain);

        check(linked.destroyed == 2 && linked.unlinked == 2 && linked.skipped_unlink == 0
                  && linked_chain.count == 0 && linked_chain.head == 0 && linked_chain.tail == 0
                  && !linked.stalled && detached.destroyed == 2 && detached.unlinked == 1
                  && detached.skipped_unlink == 1 && detached_chain.count == 0
                  && !detached.stalled,
            "the deferred-destroy drain unlinks a head that is on the chain and destroys an "
            "unlinked head without touching the header");
    }

    {
        // 008F23E0 is not 008F54F0 with keep_existing = 0: it has no
        // clone-and-insert branch, so a key the destination lacks is dropped.
        bsp::ScenePropertyBagModel dest;
        bsp::ScenePropertyValue skill;
        skill.type = bsp::ScenePropertyType::Enum;
        skill.int_value = 1; // SPNormal from universe/library/ship.props
        bsp::scene_property_bag_insert(dest, 0, "Skill", skill);

        bsp::ScenePropertyBagModel source;
        bsp::ScenePropertyValue authored_skill;
        authored_skill.type = bsp::ScenePropertyType::Enum;
        authored_skill.int_value = 5; // Elite, scene175.scn line 26845
        bsp::scene_property_bag_insert(source, 0, "skill", authored_skill);
        bsp::ScenePropertyValue numbering;
        numbering.type = bsp::ScenePropertyType::Int;
        numbering.int_value = 7;
        bsp::scene_property_bag_insert(source, 0, "Numbering", numbering);

        bsp::scene_property_bag_assign_existing(dest, 0, source, 0);
        const auto* merged = bsp::scene_property_bag_find(dest, 0, "Skill");
        check(dest.bags[0].entries.size() == 1
                  && merged != nullptr && merged->value.int_value == 5
                  && bsp::scene_property_bag_find(dest, 0, "Numbering") == nullptr,
            "008F23E0 overwrites a key the destination already has, case-insensitively, "
            "and drops a key it does not");
    }

    {
        // 00443D00 reaches __stricmp, so the hidden-entity map at
        // SceneDatabase+18h matches names case-insensitively. A consumer that
        // used an ordinary case-sensitive map would miss a record a script asks
        // for by a differently cased name.
        bsp::SceneRecordMapView map;
        bsp::SceneHiddenEntityRecord carrier;
        carrier.name = "Carrier_A";
        map.entries.push_back({"Carrier_A", &carrier});
        check(bsp::scene_record_map_find(map, "carrier_a") == &carrier
                  && bsp::scene_record_map_contains(map, "CARRIER_A")
                  && bsp::scene_record_map_find(map, "Carrier_B") == nullptr,
            "00468CD0 matches a hidden-entity record's name case-insensitively");
    }


    {
        // 005D5500 is the only reason the LobbySettings table carries both
        // TimeLimit and TimeLimit_IC: Island Capture (modes 0..3) offers slot 7
        // and nils slot 6, mode 4 does the opposite, and slots 0 and 2 ignore
        // the row entirely (005E30DC, 005E30E3).
        check(bsp::lobby_settings_slot_is_published(0, 7) && !bsp::lobby_settings_slot_is_published(0, 6)
                  && bsp::lobby_settings_slot_is_published(4, 6) && !bsp::lobby_settings_slot_is_published(4, 7)
                  && bsp::lobby_settings_slot_is_published(9, 0) && bsp::lobby_settings_slot_is_published(9, 2)
                  && !bsp::lobby_settings_slot_is_published(9, 5),
            "005D5500 offers TimeLimit_IC in Island Capture and TimeLimit in mode 4, "
            "and PlayerCount and GameMode publish in every mode");
    }

    {
        // 0042ED8A captures the child head before the store. A child's virtual
        // setter can change the parent; 0042EDA2 must reload it for the next one.
        struct InvincibilityTrace final : bsp::UnitDamageHostUnreached {
            bsp::UnitHealth parent{};
            bool head_before_write{};
            std::vector<std::pair<std::uint32_t, float>> delivered;
            std::uint32_t first_child(std::uint32_t) override {
                head_before_write = parent.invincibility == 0.0f;
                return 2;
            }
            void write_invincibility(std::uint32_t, float value) override {
                parent.invincibility = value;
            }
            bsp::UnitHealth read_health(std::uint32_t) override { return parent; }
            std::uint32_t next_sibling(std::uint32_t child) override {
                return child == 2 ? 3u : 0u;
            }
            void dispatch_set_invincible(std::uint32_t child, float value) override {
                delivered.emplace_back(child, value);
                if (child == 2) parent.invincibility = 0.75f;
            }
        } host;
        bsp::entity_set_invincible_0042ed80(host, 1, 0.25f);
        check(host.head_before_write && host.delivered.size() == 2
                  && host.delivered[0] == std::make_pair(2u, 0.25f)
                  && host.delivered[1] == std::make_pair(3u, 0.75f),
            "0042ED80 captures the child head before writing and reloads parent invincibility per child");
    }

    {
        // 00879070 at 008790D8..0087911F: a partial invincibility is a floor on
        // health, not a flag. A quarter-invincible unit at full health takes a
        // damage clamped to the headroom and stops exactly at the floor, and the
        // next call is refused. docs/UNIT_DAMAGE_AND_DEATH.md.
        bsp::UnitHealth health;
        health.max_health = 1000.0f;
        health.current_health = 1000.0f;
        health.invincibility = 0.25f;
        const bsp::UnitDamageGates gates;

        const bsp::UnitDamageOutcome first = bsp::apply_damage_00879070(health, gates, 900.0f);
        health.current_health = first.new_health;
        const bsp::UnitDamageOutcome second = bsp::apply_damage_00879070(health, gates, 1.0f);
        check(!first.refused && first.clamped_amount == 750.0f && first.new_health == 250.0f
                  && second.refused,
            "00879070 clamps damage to current - invincibility * max and then refuses");

        bsp::UnitHealth mortal;
        mortal.max_health = 1000.0f;
        mortal.current_health = 40.0f;
        mortal.invincibility = 0.0f;
        const bsp::UnitDamageOutcome lethal = bsp::apply_damage_00879070(mortal, gates, 40.0f);
        mortal.current_health = lethal.new_health;
        check(!lethal.refused && bsp::unit_is_dead(mortal)
                  && bsp::apply_damage_00879070(mortal, gates, 10.0f).refused,
            "00958DAA treats exactly zero health as dead and 008790C1 refuses further damage");
    }

    {
        // 00879810 negates before calling 00879070, so the repair tick's positive
        // amount must arrive as a negative damage. Inverting this sign would turn
        // every damage-control repair into damage and still compile.
        bsp::UnitHealth health;
        health.max_health = 1000.0f;
        health.current_health = 400.0f;
        health.invincibility = 0.0f;
        const bsp::UnitDamageGates gates;

        const float as_damage = bsp::health_delta_to_damage_00879810(120.0f);
        const bsp::UnitDamageOutcome repaired =
            bsp::apply_damage_00879070(health, gates, as_damage);
        check(as_damage == -120.0f && !repaired.refused && repaired.new_health == 520.0f,
            "00879810 turns a positive delta into a repair through 00879070");
    }

    {
        // 006E7670 and 006E65C0. The position slot takes a half-step gravity
        // term and the velocity slot a full one; swapping them, or dropping
        // either, still compiles and still looks like a ballistic arc.
        bsp::ProjectileFlightState state;
        state.velocity.y = 100.0f;
        state.snapshot_current.y = 50.0f;

        const float step = 0.05f;
        const float g = static_cast<float>(bsp::kProjectileGravity);
        const bsp::ProjectileFlightState next = bsp::projectile_flight_step(state, step);

        const float expected_y = 50.0f + 100.0f * step - (step * g) * step * 0.5f;
        check(next.local_position.y == expected_y && next.velocity.y == 100.0f - step * g &&
                  next.flight_time == step,
            "006E7670 takes 0.5*g*t^2 on the position while 006E65C0 takes g*t on the velocity");
    }


    {
        // The two director stance predicates, decoded from the raw bytes at
        // 0071D560 and 0071D580: fire is free for stances 1 and 2, move for 2
        // and 3. This is the one behavioural claim the packet recovered from
        // bodies rather than from the shipped script table.
        using bsp::FireStance;
        const bool fire_ok =
            !bsp::director_stance_allows_fire_0071d560(FireStance::HoldFire) &&
            bsp::director_stance_allows_fire_0071d560(FireStance::FreeFire) &&
            bsp::director_stance_allows_fire_0071d560(FireStance::FreeAttack) &&
            !bsp::director_stance_allows_fire_0071d560(FireStance::MoveOnly);
        const bool move_ok =
            !bsp::director_stance_allows_move_0071d580(FireStance::HoldFire) &&
            !bsp::director_stance_allows_move_0071d580(FireStance::FreeFire) &&
            bsp::director_stance_allows_move_0071d580(FireStance::FreeAttack) &&
            bsp::director_stance_allows_move_0071d580(FireStance::MoveOnly);
        check(fire_ok && move_ok,
            "0071D560 frees fire for stances 1 and 2, 0071D580 frees move for stances 2 and 3");
    }

    {
        // 00958B07..00958B20: vtable[204h] is reached only through the composite
        // test, so a sub-object that answers the secondary test but is not
        // composite keeps the default break timer. Flattening the short circuit
        // into a plain OR compiles and silently shortens every such timer.
        bsp::SubObjectClassFields fields;
        fields.break_timer_default = 4.0f;  // owner class +190h
        fields.break_timer_kinded = 9.0f;   // owner class +18Ch

        bsp::SubObjectKindTests secondary_only;
        secondary_only.secondary_46 = true;
        bsp::SubObjectKindTests composite = secondary_only;
        composite.is_composite = true;
        bsp::SubObjectKindTests direct;
        direct.is_kind_46 = true;

        check(bsp::unit_subobject_break_timer(secondary_only, fields) == 4.0f
                  && bsp::unit_subobject_break_timer(composite, fields) == 9.0f
                  && bsp::unit_subobject_break_timer(direct, fields) == 9.0f,
            "00958B07 consults vtable[204h] only for a composite sub-object");
    }
    {
        // Packet cc_hud_minimap, docs/HUD_MINIMAP.md. The world-to-minimap
        // transform at 005C1A3C..005C1C99 has three places where a sign or a
        // divisor is easy to get wrong: the rim clamp radius, the rotation
        // sense, and the two different divisors (1024 on x, 768 on y). One
        // input with a hand-computed output pins all three.
        bsp::HudMinimapWorldPoint camera;
        camera.x = 100.0f;
        camera.y = 0.0f;
        camera.z = 200.0f;

        // 2000 units due +x of the camera, inside MinimapRange, heading zero:
        // mx = 2000 * (80/4000) = 40, my = 0. x = 40/1024, y = -0/768.
        bsp::HudMinimapWorldPoint east;
        east.x = camera.x + 2000.0f;
        east.y = 0.0f;
        east.z = camera.z;
        const bsp::HudGuiPoint near_icon = bsp::hud_minimap_icon_position_005c1b62(
            east, camera, bsp::kHudMinimapInstalledRange, 0.0f, bsp::kHudMinimapDepthSelf);
        check(std::fabs(near_icon.x - 40.0f / 1024.0f) < 1e-6f
                  && std::fabs(near_icon.y) < 1e-6f && near_icon.z == -1.0f,
            "005C1B62 scales by 80/MinimapRange then divides x by 1024");

        // 8000 units due +z, beyond MinimapRange, so the rim clamp pulls it to
        // 4000: my = 4000 * 0.02 = 80 and y = -80/768. Nothing on x.
        bsp::HudMinimapWorldPoint far_south;
        far_south.x = camera.x;
        far_south.y = 0.0f;
        far_south.z = camera.z + 8000.0f;
        const bsp::HudGuiPoint rim_icon = bsp::hud_minimap_icon_position_005c1b62(
            far_south, camera, bsp::kHudMinimapInstalledRange, 0.0f, bsp::kHudMinimapDepthSelf);
        check(std::fabs(rim_icon.x) < 1e-4f
                  && std::fabs(rim_icon.y + 80.0f / 768.0f) < 1e-4f,
            "005C1A3C clamps to MinimapRange and 005C1C86 negates y over 768");

        // A quarter turn moves the +x icon onto -y: mx' = 0, my' = -40, so the
        // widget y becomes +40/768 after the negation at 005C1C86.
        const bsp::HudGuiPoint turned = bsp::hud_minimap_icon_position_005c1b62(
            east, camera, bsp::kHudMinimapInstalledRange, 1.5707963267948966f,
            bsp::kHudMinimapDepthSelf);
        check(std::fabs(turned.x) < 1e-4f && std::fabs(turned.y - 40.0f / 768.0f) < 1e-4f,
            "005C1B93 rotates by +heading in the mx*cos + my*sin sense");
    }

    {
        // 0077C2A0's local-session rule. A single-player session ignores the
        // flag override entirely (0077C32F forces 1), except for the word 4,
        // which returns at 0077C31D before the mode is ever consulted. Getting
        // these two apart matters: 7 is what the weapon director passes.
        bsp::SessionRouteInputs local;
        local.mode = bsp::SessionMode::kLocal;
        local.default_flags = bsp::default_route_flags_for_mode(bsp::SessionMode::kLocal);
        local.flags_override = 7;
        const bsp::SessionRouteDecision seven = bsp::route_session_message_0077c2a0(local);
        check(seven.routed && seven.deliver_local && !seven.send_to_host && !seven.send_to_peers,
            "0077C333 replaces a mode-0 flag word with 1, so an override of 7 stays local");

        local.flags_override = 4;
        const bsp::SessionRouteDecision four = bsp::route_session_message_0077c2a0(local);
        check(!four.routed && !four.deliver_local,
            "0077C31D drops a flag word of exactly 4 outside a host session");

        // A client sends an ordinary message to the host and does not apply it.
        bsp::SessionRouteInputs client;
        client.mode = bsp::SessionMode::kClient;
        client.default_flags = bsp::default_route_flags_for_mode(bsp::SessionMode::kClient);
        const bsp::SessionRouteDecision remote = bsp::route_session_message_0077c2a0(client);
        check(remote.send_to_host && !remote.deliver_local,
            "0076FD2C leaves a client on flags 2, so 0077C43D skips the local queue");

        client.privileged_kind = true;
        const bsp::SessionRouteDecision privileged = bsp::route_session_message_0077c2a0(client);
        check(privileged.send_to_host && privileged.deliver_local,
            "0077C35A adds the local bit when 00779FF0 answers true");
    }

    {
        // 0085AD80's per-axis rule. Constants: rate clamp at HorzRotSpeed * dt,
        // then the 00419010 remap of the remaining angle from [0, 10deg] onto
        // [0.5, 1.0], which every class except MRFSGun applies (0085AF76).
        const float dt = 1.0f / 60.0f;
        const float rate = 0.575f; // a shipped HorzRotSpeed from deviceclasses.lua
        const float far_delta = 1.0f; // well beyond both the clamp and ten degrees
        check(std::fabs(bsp::gun_step_axis_0085ad80(far_delta, rate, dt, true) - rate * dt) < 1e-6f,
            "0085AD80 clamps a far target to rate*dt and MRFSGun takes it unscaled");
        check(std::fabs(bsp::gun_step_axis_0085ad80(-far_delta, rate, dt, true) + rate * dt) < 1e-6f,
            "0085AD80 keeps the sign of the delta in the clamp");
        // Half of the ten-degree span leaves the scale at 0.75.
        const float half_span = bsp::kGunAimSoftApproachSpan * 0.5f;
        check(std::fabs(bsp::gun_soft_approach_scale_00419010(half_span) - 0.75f) < 1e-6f,
            "00419010 remaps half of 00CE3990 onto the midpoint of 00CE3800..1.0");
        // Inside the span and below the clamp the step is the delta times that scale.
        const float near_delta = half_span;
        check(std::fabs(bsp::gun_step_axis_0085ad80(near_delta, rate, 1.0f, false)
                  - near_delta * 0.75f) < 1e-6f,
            "0085AF76 scales a non-MRFSGun step by the soft approach");
        check(std::fabs(bsp::gun_step_axis_0085ad80(near_delta, rate, 1.0f, true) - near_delta)
                  < 1e-6f,
            "MRFSGun skips the soft approach and turns at the full rate");
    }

    {
        // 00903610's grace period. An entity marked by 00922FD0 carries a 1 and
        // must survive two more passes: releasing at 2 instead of 3 would free
        // it a whole fixed step early, and ageing a zero counter would release
        // entities nothing ever marked.
        using bsp::EntityExpiryAction;
        check(bsp::entity_expiry_step_00903620(0).action == EntityExpiryAction::kSkip &&
              bsp::entity_expiry_step_00903620(0).counter == 0,
            "00903625 JLE leaves an unmarked entity alone");
        check(bsp::entity_expiry_step_00903620(-1).action == EntityExpiryAction::kSkip,
            "00903625 is a signed test, so a negative counter is skipped too");
        const bsp::EntityExpiryStep marked = bsp::entity_expiry_step_00903620(1);
        check(marked.action == EntityExpiryAction::kAge && marked.counter == 2,
            "00903627 ages the 1 that 00922FE8 stored to 2 and keeps the entity");
        const bsp::EntityExpiryStep aged = bsp::entity_expiry_step_00903620(2);
        check(aged.action == EntityExpiryAction::kRelease && aged.counter == 3,
            "0090362A releases on the pass that reaches 3, the second after marking");
    }

    {
        // 0098BD01's cell key. The re-bucket path packs the AABB's two corner
        // cells without clamping them the way 0098ADD0's walk does, so the edge
        // cases are the risk worth pinning: the last in-grid cell must survive
        // the round trip, and a cell that left the grid must not, because the
        // ADDs borrow across the byte lanes.
        bsp::SpatialCellRect corner{};
        corner.min = {0, 0};
        corner.max = {bsp::kSpatialGridMaxIndex, bsp::kSpatialGridMaxIndex};
        const std::uint32_t corner_key = bsp::spatial_cell_key_0098bd01(corner);
        const bsp::SpatialCellRect corner_back =
            bsp::spatial_cell_key_unpack_0098a3d3(corner_key);
        check(corner_key == 0x95950000u,
            "0098BD01 packs minX, minZ, maxX, maxZ into the four bytes, low first");
        check(corner_back.min.x == 0 && corner_back.min.z == 0
                  && corner_back.max.x == bsp::kSpatialGridMaxIndex
                  && corner_back.max.z == bsp::kSpatialGridMaxIndex,
            "0098A3D0 recovers the cells 0098A310 stored while every index is in range");

        bsp::SpatialCellRect outside{};
        outside.min = {-1, 4};
        outside.max = {2, 5};
        const bsp::SpatialCellRect outside_back =
            bsp::spatial_cell_key_unpack_0098a3d3(bsp::spatial_cell_key_0098bd01(outside));
        check(outside_back.min.x != outside.min.x || outside_back.min.z != outside.min.z,
            "a cell index below the grid borrows into the next lane and 0098A3D0 unlinks the wrong cells");

        // 0098BAD2 sends anything wider than 2x2 to the loose array, which is
        // what keeps the register loop inside the node's four link slots.
        check(bsp::spatial_placement_0098bad2(corner) == bsp::SpatialPlacement::kLooseArray,
            "a grid-wide box cannot fit four cell links");
        bsp::SpatialCellRect quad{};
        quad.min = {10, 10};
        quad.max = {11, 11};
        check(bsp::spatial_placement_0098bad2(quad) == bsp::SpatialPlacement::kGridCells
                  && bsp::spatial_cell_slot_count_0098a310(quad) == bsp::kSpatialNodeCellLinkSlots,
            "a 2x2 span is exactly the four links at node+0Ch");
    }

    {
        // 004705C0's blast falloff. The three constants are complete: the 1.0f
        // the fraction is subtracted from, the strict "> 0" the ignore flag is
        // gated on, and the unguarded divide by the record's +24h range.
        check(std::fabs(bsp::blast_falloff_fraction_004705c0(25.0f, 100.0f, false) - 0.75f)
                  < 1e-6f,
            "004705E7 scales the part distance by the explosion radius");
        check(bsp::blast_falloff_fraction_004705c0(150.0f, 100.0f, true) < 0.0f,
            "00470602 leaves a hit outside the radius negative even when the flag is set");
        check(std::fabs(bsp::blast_falloff_fraction_004705c0(25.0f, 100.0f, true) - 1.0f)
                  < 1e-6f,
            "00470602 lifts a positive fraction to 1.0f");
    }

    {
        // 004239E0's field order over a synthetic two-cell layer. The risk is a
        // silent offset slip: the four scalars and the count are all four bytes
        // wide, so a swapped pair still parses and still consumes every byte.
        const unsigned char nav[] = {
            0x0B, 0x00, 0x00, 0x00, 'T', 'e', 'r', 'r', 'a', 'i', 'n', 'G', 'r', 'i', 'd',
            0x01, 0x00, 0x00, 0x00, // one layer
            0x10, 0x00, 0x00, 0x00, 'T', 'e', 'r', 'r', 'a', 'i', 'n', 'G', 'r', 'i', 'd',
            'L', 'a', 'y', 'e', 'r',
            0x00, 0x80, 0x3B, 0x46, // +04h half extent 12000.0
            0x02, 0x00, 0x00, 0x00, // +08h n = 2
            0x00, 0x00, 0xC8, 0x42, // +0Ch cell size 100.0
            0x00, 0x00, 0x00, 0x40, // +10h file scalar 2.0
            0x10, 0x8F, 0x34, 0x3E, // +14h slope limit tan(10deg)
            0x01, 0x02, 0x03, 0x04, // the 2x2 grid
        };
        const bsp::TerrainGridNavFile nav_file = bsp::parse_terrain_grid_nav(nav, sizeof nav);
        check(nav_file.ok && nav_file.bytes_consumed == sizeof nav,
            "004248A0's grammar consumes a .nav file exactly");
        check(nav_file.class_name == "TerrainGrid" && nav_file.layers.size() == 1,
            "004248D9 reads the root name and 004248E7 the layer count");
        const bsp::TerrainGridLayerRecord& layer = nav_file.layers.front();
        check(layer.dimension == 2 && layer.grid.size() == 4 && layer.grid[3] == 4,
            "00423A30 is the dimension and 00423A9D reads n*n grid bytes");
        check(std::fabs(layer.half_extent - 12000.0f) < 1e-3f
                  && std::fabs(layer.cell_size - 100.0f) < 1e-3f
                  && std::fabs(layer.file_scalar - 2.0f) < 1e-6f
                  && std::fabs(layer.slope_limit - 0.176327f) < 1e-6f,
            "the four scalars land in 004239E0's order");
        // 0041DF40 keeps the largest limit strictly below the requested slope.
        std::vector<bsp::TerrainGridLayerRecord> layers(3);
        layers[0].slope_limit = 0.176327f;
        layers[1].slope_limit = 0.363970f;
        layers[2].slope_limit = 2.747478f;
        check(bsp::select_terrain_grid_layer(layers, 1.0f, false) == 1,
            "0041DF40 picks the coarsest layer a slope of 1.0 still clears");
        check(bsp::select_terrain_grid_layer(layers, 0.0f, false) == 0,
            "0041DF88 leaves the first layer in EBX when nothing qualifies");
    }

    {
        // 004C3840's round-robin. The risk is the cursor: it lives in the frame
        // across the whole candidate walk (004C39B0 reads it, 004C3A3C writes it
        // back), so two units vacated from the same party must land on different
        // players, and the third must wrap to the first.
        bsp::MissionSlotBinding slots[4]{};
        slots[0].present_08 = true;              // bound, party 0
        slots[1].present_08 = true;              // bound, party 0
        slots[2].present_08 = true;              // bound, party 1
        slots[2].party_28 = 1;
        slots[3].present_08 = true;              // vacant: +9h set, +0Ah clear
        slots[3].flag_09 = true;
        bsp::MissionPartyRoster roster =
            bsp::build_mission_party_roster(slots, 4);
        check(roster.member_count[0] == 2 && roster.member_count[1] == 1,
            "004C38A2 enrols the bound slots and drops the one with +9h set and +0Ah clear");

        bsp::PartyOwnerCandidate unit{};
        unit.active_5c = true;
        unit.owner_slot_188 = 3;                 // the vacant slot, party 0
        const bsp::PartyReassignDecision first =
            bsp::decide_party_reassignment(unit, slots, 4, roster);
        const bsp::PartyReassignDecision second =
            bsp::decide_party_reassignment(unit, slots, 4, roster);
        const bsp::PartyReassignDecision third =
            bsp::decide_party_reassignment(unit, slots, 4, roster);
        check(first.action == bsp::PartyReassignAction::kReassign && first.to_slot == 0,
            "the first unit off a vacant slot goes to its party's first member");
        check(second.to_slot == 1 && third.to_slot == 0,
            "004C3A21's remainder advances the cursor and wraps at the member count");

        // A party with no bound player unbinds the owner instead (004C3996).
        slots[2].flag_09 = true;                 // slot 2 is now vacant too
        slots[3].party_28 = 1;                   // and the owner slot is party 1
        bsp::MissionPartyRoster empty_party =
            bsp::build_mission_party_roster(slots, 4);
        const bsp::PartyReassignDecision unbound =
            bsp::decide_party_reassignment(unit, slots, 4, empty_party);
        check(unbound.action == bsp::PartyReassignAction::kUnbindOwner,
            "an empty party roster takes the vtable +148h(1FFh, 8) arm");
    }

    {
        // 0098AAE0's node reject, 0098AB4B-0098ABA2. Every branch rejects only on
        // a strict separation, so a sphere that exactly touches the node bounds
        // still reaches the shape array. An off-by-one to >= here would silently
        // drop the grazing blast, which is the reason for this case.
        const float node_min[3] = {0.0f, 0.0f, 0.0f};
        const float node_max[3] = {10.0f, 10.0f, 10.0f};
        const float touching[3] = {-2.0f, 5.0f, 5.0f};
        const float clear[3] = {-3.0f, 5.0f, 5.0f};
        check(bsp::collision_node_sphere_aabb_overlap(node_min, node_max, touching, 2.0f),
            "0098AB62 keeps a sphere whose max exactly meets the node min");
        check(!bsp::collision_node_sphere_aabb_overlap(node_min, node_max, clear, 2.0f),
            "0098AB62 rejects once the sphere clears the node min");
    }

    {
        // 00836A6C's arrival test. FLD double ptr [00D09FE8] loads 4000000.0 and
        // FCOMIP/JBE raises stage 2 only on a strict less-than, so a unit exactly
        // 2000 units out has not arrived. The distance uses x and z only
        // (00836A40..00836A62 reads pfVar7[0] and pfVar7[2]); a y term or a
        // non-strict compare would each complete the command a frame early.
        check(!bsp::command_arrival_reached(0.0f, 0.0f, 2000.0f, 0.0f),
            "00836A76 keeps the command at exactly the 2000-unit arrival radius");
        check(bsp::command_arrival_reached(0.0f, 0.0f, 1999.0f, 0.0f),
            "00836A76 completes the command just inside the radius");
        check(bsp::command_arrival_reached(0.0f, 0.0f, 0.0f, 1999.0f),
            "the arrival distance is built from x and z, not x and y");
    }

    {
        // 009F5B70's score, 009F5CB2-009F5CD5. The tier is worth 10000 metres
        // (00CE4BD8) and the running best starts at 0.0f (009F5D49), so the
        // bottom tier of any list is unreachable however close the candidate
        // is. Getting the 1-based counter wrong by one would silently shift
        // every priority list by a tier, which is why this case exists.
        const int count = 7;
        check(bsp::auto_target_score_009f5b70(count, 0, 500.0f) == 59500.0f,
            "009F5CBE gives the top tier six tier weights minus the distance");
        check(bsp::auto_target_score_009f5b70(count, 6, 1.0f) == -1.0f,
            "the bottom tier scores minus the distance");
        check(!bsp::auto_target_score_beats_best_009f5b70(
                  bsp::auto_target_score_009f5b70(count, 6, 1.0f), 0.0f),
            "009F5CD5 rejects the bottom tier against the 0.0f start");
        // 009F52F0 against the FLT_MAX seed and against a held target.
        check(bsp::auto_target_switch_allowed_009f52f0(
                  59500.0f, bsp::kAutoTargetRetainedScoreReset),
            "the FLT_MAX seed lets the first candidate through");
        check(!bsp::auto_target_switch_allowed_009f52f0(59500.0f, 59500.0f),
            "009F52F8 keeps the held target against an equal score");
    }

    {
        // 007EEC50's preference order. A unit that carries every ordnance and
        // has guns matches seven classes at once, so only the order decides
        // which one it flies. The chain is levelbomb, dropkamikaze, divebomb,
        // torpedo, rocket, kamikaze, depthcharge (007EECCE..007EED5C), and the
        // gun pass runs only when the ordnance pass found nothing or the caller
        // cleared prefer_ordnance (007EED84..007EEDB2). A reordering would be
        // silent: every candidate is individually applicable.
        bsp::AttackFeasibilityInputs in;
        in.unit_has_weapon_controller = true;
        in.target_present = true;
        in.unit_side = 1;
        in.target_side = 3;
        in.target_is_surface = true;
        in.self_is_level_bomber = true;
        in.target_is_structure = true;
        in.has_level_bomb_ordnance = true;
        in.has_general_bomb_ordnance = true;
        in.has_drop_kamikaze_ordnance = true;
        in.has_torpedo_ordnance = true;
        in.has_rocket_ordnance = true;
        in.has_depth_charge_ordnance = true;
        in.target_is_submarine = true;
        in.guns_available = true;
        check(bsp::attack_command_choose(in, true, true) == bsp::kAttackCmdLevelBomb,
            "007EEC50 tries levelbomb before every other ordnance class");
        in.has_level_bomb_ordnance = false;
        in.self_is_level_bomber = false;
        check(bsp::attack_command_choose(in, true, true) == bsp::kAttackCmdDropKamikaze,
            "dropkamikaze is tried before divebomb and torpedo");
        in.has_drop_kamikaze_ordnance = false;
        check(bsp::attack_command_choose(in, true, true) == bsp::kAttackCmdDiveBomb,
            "divebomb is tried before torpedo and rocket");
        // With prefer_ordnance clear the gun pass wins even though divebomb
        // still applies, and strafe is the fallback because the target is not
        // airborne so dogfight cannot match.
        check(bsp::attack_command_choose(in, false, true) == bsp::kAttackCmdStrafe,
            "a cleared prefer_ordnance sends 007EEC50 to the gun pass");
        check(bsp::attack_command_choose(in, false, false) == 0u,
            "a cleared allow_guns discards the gun-only answer");
    }

    {
        // docs/DYN_LCP_IMPULSE_MATH.md: the shipped solver chain 00C4DE40 then ten
        // passes of 00C42530 and 00C42230, for a 1000 kg hull resting on a fixed box one
        // substep after gravity. The concrete risk this guards is the sign convention:
        // body A takes the negated half of the Jacobian (00C4E1F7) while both apply
        // steps add (00C425EB), so a flipped sign would leave the hull sinking through
        // the box with an impulse that still looked plausible.
        const float dt = 1.0f / 60.0f;
        const float mass = 1000.0f;

        bsp::DynConstraintBuildInput input;
        input.body_a.solver_index = 0;                  // the fixed box, the static slot
        input.body_b.solver_index = 1;                  // the hull
        input.body_b.inverse_mass = 1.0f / mass;
        input.body_b.position[1] = 1.0f;
        input.body_b.linear_velocity[1] = -10.0f * dt;  // gravity already integrated
        for (int i = 0; i < 9; i += 4) input.body_b.inverse_inertia[i] = 1.0e-4f;
        input.point.normal[1] = 1.0f;                   // from the box up to the hull
        input.point.local_point_b[1] = -1.0f;           // the keel, under the origin
        input.point.depth = 0.02f;
        input.friction = 0.5f;

        bsp::DynConstraintRow rows[2];
        const bsp::DynSolverWorldSettings settings;     // the shipped 0.1 / 1.0 / 0.5 / 10
        bsp::dyn_build_contact_rows_00c4de40(input, settings, dt, rows[0], rows[1]);

        bsp::DynSolverBodyVelocity velocities[2];
        bsp::DynConstraintBatch batch;
        batch.rows = rows;
        batch.velocities = velocities;
        batch.velocity_count = 2;
        batch.normal_row_count = 1;
        batch.friction_row_base = 1;
        bsp::dyn_apply_warm_start_00c42ba0(batch);
        bsp::dyn_solve_group_00403720(batch, settings.iterations);

        const float weight_impulse = mass * 10.0f * dt;
        check(std::fabs(rows[0].impulse - weight_impulse) < 0.01f,
            "ten iterations of 00C42530 hold the hull up with its substep weight");
        check(rows[1].impulse == 0.0f,
            "00C42230 leaves friction at zero while nothing slides");
    }

    {
        // 006CD350's launch branch: the plane limit, the free stock and the slot's
        // own request all clamp the count, and a scripted launch leaves the 5.0
        // second cooldown from 00CE3850 behind.
        bsp::AirOpsLaunchInputs input;
        input.plane_limit = 12;
        input.committed_planes = 4;
        input.class_stock = 6;
        input.slot_requested = 3;
        input.launch_requested = true;
        const bsp::AirOpsLaunchDecision decision = bsp::air_ops_slot_launch_006cd350(input);
        check(decision.launch_count == 3, "006CD40B clamps the launch to the slot's request");
        check(decision.next_timer == bsp::kAirOpsSlotCooldownSeconds,
            "006CD41F starts the 5.0 second cooldown a requested launch asks for");

        input.class_stock = 0;
        const bsp::AirOpsLaunchDecision starved = bsp::air_ops_slot_launch_006cd350(input);
        check(starved.launch_count == 0 && starved.clear_slot,
            "006CD414 empties the slot when no stock is free");
    }

    {
        // 007F473A..007F476C: an absent WingCount is 3, not the " 1" the
        // plane.props descriptor declares, and a present value below 1 is
        // raised to 1. There is no upper clamp, so only the authored enum
        // PlaneWingCount (1..5) keeps 007F4B55 inside the five-slot array.
        bsp::SquadronSpawnProperties props;
        check(bsp::squadron_resolve_wing_count_007f4747(props) == bsp::kSquadronDefaultWingCount,
            "007F473A defaults an absent WingCount to 3");
        props.wing_count_present = true;
        props.wing_count_raw = 0;
        check(bsp::squadron_resolve_wing_count_007f4747(props) == 1,
            "007F4764 raises a WingCount below 1 to 1");
        props.wing_count_raw = 6;
        check(bsp::squadron_resolve_wing_count_007f4747(props) == 6,
            "007F476E has no upper clamp on WingCount");
        check(!bsp::squadron_wing_count_fits_array_007f4b55(6),
            "six wings would write past the five pointers at squadron+3D0h");
    }

    {
        // 007F669F is the one branch of 007F6530 that changes a delta's sign
        // rather than its magnitude, so it is the branch worth pinning. Four
        // windows partition the circle; the gun sits in the second at -1.5 rad
        // and the target is at +1.5 rad in the fourth. The short way is +3.0
        // rad and runs through the third window, so blocking that window has to
        // send the gun the other way instead.
        bsp::GunFiringArc windows[4] = {};
        for (bsp::GunFiringArc& w : windows) {
            w.flags = bsp::kGunArcFlagTraverse;
            w.min_vert = -1.0f;
            w.max_vert = 1.0f;
        }
        windows[0].min_horz = -3.14f; windows[0].max_horz = -2.0f;
        windows[1].min_horz = -2.0f;  windows[1].max_horz = -1.0f;
        windows[2].min_horz = -1.0f;  windows[2].max_horz = 1.0f;
        windows[3].min_horz = 1.0f;   windows[3].max_horz = 3.14f;
        const bsp::GunPlatformArcs arcs{windows, 4};

        const bsp::GunArcRouteOutcome clear =
            bsp::gun_arc_route_deltas_007f6530(arcs, -1.5f, 0.0f, 1.5f, 0.0f);
        check(!clear.routed_around && clear.deltas.horz > 0.0f,
            "007F6530 keeps the short way when every window on it allows traverse");

        windows[2].flags = 0; // the third window forbids traversal
        const bsp::GunArcRouteOutcome blocked =
            bsp::gun_arc_route_deltas_007f6530(arcs, -1.5f, 0.0f, 1.5f, 0.0f);
        check(blocked.routed_around && blocked.deltas.horz < 0.0f,
            "007F669F reverses the traverse when a blocked window is in the way");
    }

    {
        // 0085B0F0's early stores must survive the second arc-search failure;
        // success below the horizontal snap threshold still snaps elevation
        // without an elevation-window test and commits both target fields.
        struct ImmediateAngles final : bsp::GunImmediateAngleHost {
            bsp::GunFiringArc arc{bsp::kGunArcFlagTraverse, -0.5f, 0.5f, -0.1f, 0.1f};
            int platforms{}, applied{};
            bsp::GunPlatformArcs platform_arcs_005471b0() override {
                ++platforms;
                return {&arc, 1};
            }
            void apply_angles_to_nodes_00859550() override { ++applied; }
        } host;
        bsp::GunTurningAngles angles{0.0f, 0.0f, -0.2f, -0.3f};
        float previous_h = -0.4f, previous_v = -0.5f, received[4]{};
        const bsp::GunImmediateAngleState state{angles, previous_h, previous_v, received};
        bsp::gun_set_angles_immediate_0085b0f0(state, {0.1f, 0.2f, 2.0f, 2.0f}, host);
        check(received[0] == 2.0f && received[1] == 2.0f
                && received[2] == 0.1f && received[3] == 0.2f
                && angles.horz == 0.0f && angles.vert == 0.0f
                && angles.target_horz == -0.2f && host.applied == 0 && host.platforms == 2,
            "0085B0F0 keeps received stores but aborts before targets on a current-H arc miss");
        bsp::gun_set_angles_immediate_0085b0f0(state, {0.1f, 0.2f, 0.25f, 2.0f}, host);
        check(angles.horz == 0.0f && previous_h == -0.4f
                && angles.vert == 2.0f && previous_v == 2.0f
                && angles.target_horz == 0.1f && angles.target_vert == 0.2f
                && host.applied == 1 && host.platforms == 3,
            "0085B0F0 commits targets and the large vertical snap without testing elevation arcs");
    }

    {
        // 009032A8..009032E9: only a depression steeper than -0.02 rad is
        // corrected, and the correction is v - (0.02 + v) * 0.5, which is
        // 0.5v - 0.01, not a plain halving. A target of kind 0Fh flattens the
        // shot instead (009032B5).
        check(bsp::gun_bot_ballistic_vertical_correction_009030c0(0.25f, false) == 0.25f,
            "009032A8 leaves an elevation untouched");
        check(bsp::gun_bot_ballistic_vertical_correction_009030c0(-0.01f, false) == -0.01f,
            "009032CE leaves a depression shallower than -0.02 rad untouched");
        check(std::fabs(bsp::gun_bot_ballistic_vertical_correction_009030c0(-0.10f, false) -
                        -0.06f) < 1e-6f,
            "009032E9 maps -0.10 rad to 0.5v - 0.01 = -0.06 rad");
        check(bsp::gun_bot_ballistic_vertical_correction_009030c0(-0.10f, true) == 0.0f,
            "009032B5 flattens the shot when the target answers IsKindOf(0Fh)");
    }

    {
        // 0071F290's arm order, the one rule in this packet worth pinning: a
        // begin-command refusal terminates that command by raising its stage to
        // 2 (0071F346 / 0071F36A), and the session-mode-2 return at 0071F37F
        // sits *after* both begin arms but *before* the two steps. A run in
        // mode 2 therefore still terminates a refused command.
        struct RefusingHost final : bsp::CommandControllerUpdateHost {
            int begins = 0;
            void reset_path_vector() override {}
            bool begin_command(int) override { ++begins; return false; }
            void raise_override_stage(int) override {}
            void raise_queue_stage(int) override {}
            void step_auto_target(float) override {}
            void step_commands() override {}
        };
        bsp::CommandControllerUpdateState state;
        state.session_present = true;
        state.session_flags.flag_5c = true;
        state.slot0_occupied = true;
        state.override_command_present = true;
        state.auto_target_present = true;
        state.session_mode = bsp::kSessionModeNoSimulation;

        RefusingHost host;
        const bsp::CommandControllerUpdateTrace trace =
            bsp::run_command_controller_update(state, 0.05f, host);
        check(trace.mode_promoted && trace.mode == 1,
            "0071F323 promotes an idle controller with an occupied slot 0 to mode 1");
        check(host.begins == 2 && trace.override_terminated && trace.queue_terminated,
            "0071F346 and 0071F36A terminate a command whose begin was refused");
        check(!trace.auto_target_stepped && !trace.commands_stepped,
            "0071F37F returns before both steps when the session mode is 2");
    }

    {
        // 008FB8D0 builds the intercept quadratic with a linear coefficient of
        // dot(d, v) where the exact equation needs 2 dot(d, v), so the solution
        // under-leads a receding target. A ship 1000 m away running straight
        // away at half the torpedo's speed should be met at 100 s; the native
        // answers 76.76 s. The pair is pinned so the deviation is not silently
        // corrected into a "fix" that stops matching the game.
        const std::array<float, 3> shooter{{0.0f, 0.0f, 0.0f}};
        const std::array<float, 3> target{{1000.0f, 0.0f, 0.0f}};
        const std::array<float, 3> velocity{{10.0f, 0.0f, 0.0f}};
        const bsp::TorpedoInterceptRoots roots =
            bsp::torpedo_intercept_time_008fb8d0(shooter, target, 20.0f, velocity);
        check(roots.root_count == 1,
            "008FBAE4 reports one root when the torpedo outruns the target");
        check(std::fabs(roots.first - 76.7592f) < 1e-2f,
            "008FBA95 under-leads a receding target: 76.76 s, not the exact 100 s");
        check(std::fabs(bsp::torpedo_intercept_time_exact(shooter, target, 20.0f, velocity) -
                        100.0f) < 1e-3f,
            "the exact intercept of the same shot is 1000 / (20 - 10)");

        std::array<float, 3> point{};
        check(bsp::torpedo_intercept_point_008fbb00(shooter, target, 20.0f, velocity, point) &&
              std::fabs(point[0] - 1767.59f) < 1e-1f,
            "008FBB9E aims 1767.6 m out, 232 m short of the exact intercept");

        // The one case the missing factor cannot spoil: a stationary target,
        // where dot(d, v) is zero and both forms agree.
        const std::array<float, 3> still{{0.0f, 0.0f, 0.0f}};
        const bsp::TorpedoInterceptRoots parked =
            bsp::torpedo_intercept_time_008fb8d0(shooter, target, 20.0f, still);
        check(parked.root_count >= 1 && std::fabs(parked.first - 50.0f) < 1e-3f,
            "008FB8D0 is exact against a stationary target: 1000 / 20");
    }

    {
        // 008065FF..0080672B: the recon membership relation over every
        // (slot index, unit party) pair the three slots can see. The slot 2 arm
        // is its own branch, not the general rule, so it is pinned here.
        using bsp::ReconRelation;
        using bsp::recon_relation_for_008065ff;
        check(recon_relation_for_008065ff(0, 0) == ReconRelation::own
                && recon_relation_for_008065ff(0, 1) == ReconRelation::enemy
                && recon_relation_for_008065ff(0, 2) == ReconRelation::neutral,
            "008065FF: slot 0 sees party 0 own, party 1 enemy, party 2 neutral");
        check(recon_relation_for_008065ff(1, 1) == ReconRelation::own
                && recon_relation_for_008065ff(1, 0) == ReconRelation::enemy
                && recon_relation_for_008065ff(1, 2) == ReconRelation::neutral,
            "008065FF: slot 1 sees party 1 own, party 0 enemy, party 2 neutral");
        check(recon_relation_for_008065ff(2, 2) == ReconRelation::own
                && recon_relation_for_008065ff(2, 0) == ReconRelation::neutral
                && recon_relation_for_008065ff(2, 1) == ReconRelation::neutral,
            "00806721: slot 2 has no enemy, every other party is neutral");
    }

    {
        // Packet cc_cruise_speed_setting. usn_2_java.scn authors
        // `StartSpeed = F 12.0000 ;` on all fourteen of its `Cruise` units; the
        // DeRuyter class MaxSpeed is 16.4622 m/s with the gameplay scale at 1.0.
        // The risk this pins is 008235E1: the value 0092D770 receives is the
        // reference speed multiplied back by the float32 ratio, not the authored
        // speed passed straight through, and the two differ in the last bits.
        bsp::SceneStartSpeedProperty record{};
        record.present = true;
        record.type = bsp::ScenePropertyType::Float;
        record.value_float = 12.0f;

        bsp::UnitOrderRing ring{};
        const bsp::CruiseSpeedSettingOutcome cruise_seed =
            bsp::scene_cruise_ship_speed(record, ring, 16.4622f, 1.25f, 0.0f);

        check(std::fabs(cruise_seed.seed.ring_throttle - 0.7289426f) < 1e-6f,
            "008235BF seeds ring +148h with 12.0 / 16.4622, not with 12.0");
        // A reference speed that makes the round trip lossy, so the check tells
        // the native's shape apart from passing the authored speed through. The
        // authored 7.71667 m/s is the commonest StartSpeed in the shipped scenes.
        const bsp::SceneStartSpeedSeed lossy =
            bsp::scene_start_speed_seed_008235b0(7.71667f, 15.0f, 15.0f);
        check(lossy.axial_speed != 7.71667f && std::fabs(lossy.axial_speed - 7.71667f) < 1e-5f,
            "008235E1 hands 0092D770 the reference speed times the ratio, not the authored speed");
        check(cruise_seed.latched.is_heading && std::fabs(cruise_seed.latched.steer_or_heading - 1.25f) < 1e-6f,
            "00835AE0 keeps the spawn heading: the seed never touches the ring's rudder");
        check(std::fabs(cruise_seed.ordered.throttle - cruise_seed.seed.ring_throttle) < 1e-6f &&
              cruise_seed.ordered.mode == bsp::CruiseSteerMode::Heading,
            "009E1265 orders the seeded throttle, so a scene `Cruise` ship makes way");
    }

    {
        // 00727BD0 over the installed preference lists at 00E092C8. Pins the
        // unclamped write index: category 0's single id 61h is one past the
        // class-id space, so its rank lands on row 1 slot 0 and the next
        // iteration's zeroing must erase it before row 1 is filled.
        static std::array<int, bsp::kUnitGunneryCategoryCount * bsp::kUnitGunneryClassIdCount> ranks{};
        bsp::build_rank_table_00727bd0(bsp::kGunneryPreferenceLists, ranks.data());
        check(bsp::gunnery_rank(ranks.data(), 1, 0x17) == 1
                && bsp::gunnery_rank(ranks.data(), 1, 0x41) == 12
                && bsp::gunnery_rank(ranks.data(), 1, 0x07) == 0,
            "00727BD0: AAMACHINEGUN ranks the kamikaze first, the navpoint last, no destroyer");
        check(bsp::gunnery_rank(ranks.data(), 4, 0x0D) == 1
                && bsp::gunnery_rank(ranks.data(), 2, 0x0D) == 8,
            "00727BD0: HEAVYARTILLERY ranks the battleship first, LIGHTARTILLERY eighth");
        check(bsp::gunnery_rank(ranks.data(), 1, 0) == 0,
            "00727BEC: category 0's id 61h aliases row 1 slot 0 and is zeroed again");
    }

    {
        // 009E76D0 -> 009E5E90 -> the 009E6A90 sentinel arm. The executable
        // sailed its six attackmove ships on the 9999.0f that 009F1BF7 seeds
        // into nested+1210h every frame, so the risk worth pinning is that the
        // ring scan still hands 009E5E90 a real bearing and that the sentinel
        // never survives to the throttle 009F3635 forwards.
        struct ClearWaterHost final : bsp::ShipAiRingScanHost {
            float committed_bearing = -1.0f;
            bool committed = false;
            float wrap_phase_00605070(float value) override { return value; }
            std::uint32_t probe_space_vtable_0218() override { return 1u; }
            bool unit_pose_fresh_00c8() override { return true; }
            void refresh_unit_pose_00414db0() override {}
            bsp::ShipAiAttackMoveXZ unit_world_xz() override { return {}; }
            bsp::ShipAiAttackMoveXZ probe_origin_00417b10(std::uint32_t,
                                                          const bsp::ShipAiAttackMoveXZ& point,
                                                          float, int) override {
                return point;
            }
            bool probe_hit_0041b4e0(std::uint32_t, const bsp::ShipAiAttackMoveXZ&,
                                    const bsp::ShipAiAttackMoveXZ&,
                                    bsp::ShipAiAttackMoveXZ&) override {
                return false;
            }
            float planar_length_00414c60(const bsp::ShipAiAttackMoveXZ&) override { return 0.0f; }
            float tune_reject_penalty_04() override { return 0.5f; }
            void rebuild_unit_world_matrix() override {}
            bsp::ShipAiAttackMoveXZ brain_goal_0b2c() override { return {}; }
            float unit_cruise_speed_0490() override { return 0.0f; }
            void commit_bearing_009e5e90(float bearing, float) override {
                committed_bearing = bearing;
                committed = true;
            }
        };

        bsp::ShipAiAttackMoveRingSlot ring[bsp::kShipAiApproachSlotCount];
        for (int i = 0; i < bsp::kShipAiApproachSlotCount; ++i) {
            ring[i] = bsp::ship_ai_attackmove_ring_slot_009e5530(i, 1u, 2u);
        }
        bsp::ShipAiApproachSlotScore slots[bsp::kShipAiApproachSlotCount];
        bsp::ShipAiApproachState state;
        state.turn_radius_11f0 = 500.0f;
        state.commanded_throttle_1210 = bsp::kApproachCommandUnset; // 009F1BF7
        ClearWaterHost host;
        bsp::ship_ai_ring_scan_009e76d0(state, ring, slots, 0.1f, host);

        bool every_slot_accepted = true;
        for (int i = 0; i < bsp::kShipAiApproachSlotCount; ++i) {
            // 009E784B would have written -0.0f - tune+4h on a rejection.
            if (slots[i].blocked_40 || slots[i].penalty_30 != 0.0f) every_slot_accepted = false;
        }
        check(every_slot_accepted,
            "009E6640: a clear probe scores 1.0, so 009E7822 accepts every slot");
        check(host.committed && host.committed_bearing == state.selected_bearing_11f8,
            "009E7ECB: the ring scan hands 009E5E90 the winning slot's bearing");

        bool blocked[bsp::kShipAiApproachSlotCount] = {};
        bsp::ship_ai_approach_commit_bearing_009e5e90(state, blocked,
                                                      host.committed_bearing);
        const float error = std::fabs(
            bsp::wrapped_angle_subtract_00438b10(0.0f, state.commanded_heading_120c));
        // 009E6ADE: the sentinel is above 1000, so 009E6B12 replaces it.
        check(state.commanded_throttle_1210 > 1000.0f,
            "009F1BF7: the throttle still holds the 9999.0f sentinel here");
        const float seeded = bsp::ship_ai_approach_throttle_seed_009e6a90(error);
        const float limited = bsp::ship_ai_approach_command_limit_009e6a90(seeded, 1.0f);
        check(limited >= -1.0f && limited <= 1.0f && limited != bsp::kApproachCommandUnset,
            "009E6A90: the sentinel never reaches the throttle 009F3635 forwards");
    }

    {
        // 00903F44 / 00924724 are signed JG, so a sole live node must be removed
        // even though its previous and next links are both zero.
        bsp::IntrusiveChain singleton{1, 1, 1};
        std::vector<bsp::IntrusiveChainNode> singleton_nodes(2);
        bsp::chain_unlink_00903f30(singleton, 1, singleton_nodes);
        check(singleton.first == 0 && singleton.last == 0 && singleton.count == 0,
            "00903F44: unlinking the sole node empties the chain");

        // 00925825: the world+0Ch unit list is only left from the parentless
        // branch of 00925780, so a child entity never reaches 004845A0 however
        // large its +B8h counter is.
        bsp::EntityReleaseState child{};
        child.has_parent = true;
        child.world_list_counter = 3;
        const auto child_steps = bsp::entity_release_steps_006fe570(child);
        check(std::find(child_steps.begin(), child_steps.end(),
                        bsp::EntityReleaseStep::remove_from_world_unit_list) == child_steps.end(),
            "00925835 is unreachable for an entity with a parent");

        bsp::EntityReleaseState root = child;
        root.has_parent = false;
        const auto root_steps = bsp::entity_release_steps_006fe570(root);
        check(std::find(root_steps.begin(), root_steps.end(),
                        bsp::EntityReleaseStep::remove_from_world_unit_list) != root_steps.end(),
            "00925835 runs for a parentless entity whose +B8h counter is set");
    }

    {
        // 00852B90: a submarine's sensor category is a depth state, and the
        // three comparisons run in one direction only. Getting a boundary
        // backwards silently makes a surfaced boat sonar-visible or a deep one
        // detectable by sight, with nothing else in the build to catch it.
        bsp::SubmarineDepthBands bands;
        bands.band_1200 = -2.0f;
        bands.band_1204 = -4.0f;  // surface limit (-4 + -2) / 3 = -2
        bands.band_1208 = -20.0f;
        bands.band_120c = -30.0f; // deep limit (-30 + -20) * 0.5 = -25
        using bsp::SensorCategory;
        check(bsp::sensor_category_submarine_00852b90(0.0f, bands) == SensorCategory::surface &&
              bsp::sensor_category_submarine_00852b90(-5.0f, bands) == SensorCategory::periscope_in &&
              bsp::sensor_category_submarine_00852b90(-15.0f, bands) == SensorCategory::underwater &&
              bsp::sensor_category_submarine_00852b90(-30.0f, bands) == SensorCategory::deep_underwater,
            "00852B90 walks surface, periscope, underwater, deep as the hull sinks");
        bands.periscope_raised = true;
        check(bsp::sensor_category_submarine_00852b90(-5.0f, bands) == SensorCategory::periscope_out,
            "00852C4C picks PeriscopeOut from the +1234h flag, PeriscopeIn without it");
    }

    {
        // 009FB800 over a Shooting Star row (DropAngle 0.698132, no ClimbAngle key so
        // class+1ECh is 0). Pins the asymmetry the two arms have: the climb arm falls
        // back to the DEG(40) floor and so commands nothing, while the dive arm's
        // DropAngle carries both the gain and the cap.
        bsp::PlanePitchCommandInputs climb;
        climb.desired_altitude = 400.0f;
        climb.reference = 1.0f;
        climb.unit_world_y = 100.0f;
        climb.class_drop_angle = 0.698132f;
        const float climb_demand = bsp::pitch_command_009fb800(climb);

        bsp::PlanePitchCommandInputs dive = climb;
        dive.desired_altitude = 100.0f;
        dive.unit_world_y = 400.0f;
        const float dive_demand = bsp::pitch_command_009fb800(dive);

        bsp::PlanePitchCommandInputs at_ceiling = climb;
        at_ceiling.desired_altitude = 9000.0f;
        at_ceiling.unit_world_y = 1450.0f;
        const float ceiling_demand = bsp::pitch_command_009fb800(at_ceiling);

        check(climb_demand == 0.0f,
            "009FB88D: class+1ECh has no Lua key, so every shipped plane climbs on a zero gain");
        check(dive_demand < 0.0f && std::fabs(dive_demand + 0.698132f) < 1e-5f,
            "009FB9B8: -min(DropAngle * clamp(-x/200, 0, ref), max(DropAngle*1.6, DEG(60)))");
        check(ceiling_demand == 0.0f,
            "009FB809: Dynamics/Ceiling minus 50 caps the demand before the error is taken");
    }

    {
        // 007BB6E0, the signed-byte channel between the pilot command block unit+9FCh and
        // the pilot control block unit+9E4h. Worth pinning because the saturation is on the
        // quantised integer, not the float: the bias 128.5 with a truncating ftol makes
        // q <= 1 and q >= 0FFh unreachable from any command inside [-1, 1] except at the
        // ends, and the reconstructed value is quantised, never the command itself.
        const float centre = bsp::quantize_pilot_axis_007bb6e0(0.0f);
        const float high = bsp::quantize_pilot_axis_007bb6e0(1.0f);
        const float low = bsp::quantize_pilot_axis_007bb6e0(-1.0f);
        const float below_step = bsp::quantize_pilot_axis_007bb6e0(0.004f);

        check(centre == 0.0f, "007BB6E0: ftol(0*127 + 128.5) = 128, so the centre survives");
        check(high == 1.0f && low == -1.0f,
            "007BB6E0: q >= 0FFh gives 1.0f and q <= 1 gives -1.0f, the two saturations");
        check(below_step == 1.0f / bsp::kPilotQuantizeScale,
            "007BB6E0: the 128.5 bias snaps a sub-step command up to exactly one step, 1/127");
    }

    {
        // 004179D0 and 00422500, the two halves of the search's zone side. The
        // risk worth one case is that the two ends of the native's walk are
        // easy to swap: 004179D0 reports the crossing nearest its FIRST point
        // argument (00416DD0 moves the running point toward it), and 00422500's
        // backward index names the edge that leaves the corner it published,
        // one below that corner's own index, while the forward index is the
        // corner's own. A square and a segment straight through it pins both.
        std::vector<std::array<float, 3>> outline = {
            {{-100.0f, 0.0f, -100.0f}}, {{100.0f, 0.0f, -100.0f}},
            {{100.0f, 0.0f, 100.0f}}, {{-100.0f, 0.0f, 100.0f}}};
        const AvoidZonePolygon square = avoid_zone_from_path_points(outline, 0);
        float turn_sum = 0.0f;
        for (const AvoidZoneCorner& corner : square.corners) turn_sum += corner.turn_angle;

        check(square.corners.size() == 4, "0041CCD0: four path points more than five units apart");
        check(std::fabs(turn_sum + kAvoidZoneTwoPi) < 1e-4f,
            "0041A200: the finished winding sums to -2*pi, so 0041A47E did not reverse it");

        AvoidZoneTable table;
        const std::int32_t group = avoid_zone_group_find_or_create_00417ca0(table, 0);
        table.groups[static_cast<std::size_t>(group)].zones.push_back(square);

        const std::array<float, 2> west = {{-500.0f, 0.0f}};
        const std::array<float, 2> east = {{500.0f, 0.0f}};
        const AvoidZoneGroupHit hit = avoid_zone_group_segment_hit_004179d0(
            table.groups[static_cast<std::size_t>(group)], west, east);

        check(hit.hit && hit.edge_index == 3,
            "004179D0: the crossing nearest the first point is the west edge, corner 3 to 0");
        check(std::fabs(hit.point[0] + 100.0f) < 1e-3f && std::fabs(hit.point[1]) < 1e-3f,
            "00416DD0: the reported point is that crossing, not the far one at x = +100");

        const AvoidZoneTangentCorners detour =
            avoid_zone_tangent_corners_00422500(table, square, east, hit.edge_index, -1, 0, 30.0f);
        const float offset = 100.0f + 30.0f * 0.70710678f;

        check(detour.has_forward && detour.has_backward && detour.side_code == 0,
            "00422500: both passes find a corner, so the side code is 0");
        check(std::fabs(detour.forward_point[0] - offset) < 1e-2f
                && std::fabs(detour.forward_point[1] + offset) < 1e-2f,
            "00422500: the forward corner is (100, -100) pushed 30 units along its bisector");
        check(std::fabs(detour.backward_point[0] - offset) < 1e-2f
                && std::fabs(detour.backward_point[1] - offset) < 1e-2f,
            "00422500: the backward corner is (100, 100) pushed the same way");
        check(detour.forward_index == 1 && detour.backward_index == 1,
            "00422BCA and 00423173: corner 1 forward, corner 2 backward reported as edge 1");

        // Chain 546: the ship-placement query must measure from the polygon,
        // rather than treating its 800-unit broad-phase slack as clearance.
        // Reuse this fixture's derived corners through the production snapshot
        // adapter, including the half-open upper broad-phase boundary.
        std::array<bsp::ShipAiPathLateralRecord, 4> records;
        std::array<bsp::ShipAiPathLateralRecord*, 4> pointers;
        for (std::size_t i = 0; i < records.size(); ++i) {
            const auto& c = square.corners[i];
            records[i] = {c.position[0], c.position[1], c.edge_direction[0], c.edge_direction[1],
                c.normal[0], c.normal[1], c.edge_length, c.turn_angle, c.spare};
            pointers[i] = &records[i];
        }
        const bsp::AvoidZoneNativeStorage native{{pointers.data(), 4}, 4, 0, nullptr,
            -100.0f, -100.0f, 100.0f, 100.0f};
        const volatile std::uint32_t sqrt_bypass = 1;
        const bsp::CameraAxesCrtAccess crt{&sqrt_bypass,
            [](std::int32_t, bsp::CameraAxesCrtException*, std::uint16_t*) {
                throw std::logic_error("finite clearance fixture reached legacy CRT error dispatch");
            }};
        bsp::AvoidZoneBoundaryQueries distances({&native}, crt);
        check(distances.distance_within_slack({150.0f, 0.0f}, 800.0f) == 50.0f
                && distances.distance_within_slack({0.0f, 0.0f}, 800.0f) == 0.0f
                && distances.distance_within_slack({900.0f, 0.0f}, 800.0f)
                    == (std::numeric_limits<float>::max)(),
            "0041B680/0041B120: ship placement uses shoreline distance, inside zero, and half-open slack bounds");
    }

    {
        // 00855250, the submarine air model. Worth pinning because two rules are
        // easy to lose: the critical edge at SubmarineAirNeedLimit suppresses the
        // low edge at SubmarineAirWarningLimit on a step that crosses both, and the
        // needAir latch returns immediately after arming so the disarm test is
        // skipped on that step. docs/SUBMARINE_MODEL.md section 4.
        const bsp::SubmarineDepthSettings settings{};   // installed 0.35 / 0.16 / 0.5
        const bsp::SubmarineAirRates rates{};           // 120.0 / 5.0 seconds
        const float deep = -50.0f;                      // below the -4.0 breathing line

        bsp::SubmarineAirState both{};
        both.air = 0.40f;  // one 30-second step crosses 0.35 and 0.16 together
        const auto crossed_both = bsp::submarine_step_air_00855250(
            both, 30.0f, deep, -20.0f, rates, settings, false);
        check(crossed_both.warning == bsp::SubmarineAirWarning::critical,
            "00855250: a step past both limits reports critical only, never low as well");
        check(crossed_both.state.need_air,
            "00855250: the same step arms the needAir latch below SubmarineAirNeedLimit");

        bsp::SubmarineAirState armed{};
        armed.air = 0.10f;
        armed.need_air = false;
        const auto arming = bsp::submarine_step_air_00855250(
            armed, 0.0f, 0.0f, -20.0f, rates, settings, false);
        check(arming.state.need_air,
            "00855250: arming is unconditional below the need limit, surfaced or not");

        bsp::SubmarineAirState latched{};
        latched.air = 0.49f;
        latched.need_air = true;
        const auto still_latched = bsp::submarine_step_air_00855250(
            latched, 0.0f, 5.0f, -20.0f, rates, settings, false);
        check(still_latched.state.need_air,
            "00855250: the latch holds until air passes SubmarineAirEnoughLimit, not the need limit");

        // The breathing line is the flat fallback for every shipped class, because
        // periscope depth plus three metres is still below the waterline.
        check(bsp::submarine_air_breathing_line_00855250(-20.0f) ==
                  bsp::kSubAirBreathingLineFallback,
            "00855250: a -20 periscope band gives the -4.0 breathing line, not -17.0");
    }

    {
        // The LandConvoy's two wraps, 00743060 and 00742400. Worth pinning because
        // the two are deliberately different: the element slot wraps with loops
        // (007430DB, 007430FE) while the placement pass wraps with one conditional
        // each way (00742581-007425A9), so a row gap wider than the path leaves the
        // group arc outside [0, length). The lateral centring on
        // (columns - 1) * 0.5 is the other silent-failure candidate.
        bsp::LandConvoyFormation f;
        f.rows = 3;
        f.columns = 4;
        f.row_gap = 30.0f;
        f.column_gap = 10.0f;
        f.speed = 100.0f;
        f.path_length = 50.0f;

        const bsp::LandConvoyArcStep stepped =
            bsp::land_convoy_advance_arc_00743060(f, 40.0f, 0.0f, 0.05f);
        check(std::fabs(stepped.live_arc - 45.0f) < 1e-4f,
            "00743060: 40 + 0.05*100 = 45 stays inside [0, 50) with no wrap");
        const bsp::LandConvoyArcStep wrapped =
            bsp::land_convoy_advance_arc_00743060(f, 48.0f, 0.0f, 0.05f);
        check(std::fabs(wrapped.live_arc - 3.0f) < 1e-4f,
            "00743060: the subtract loop brings 53 back to 3");

        f.reverse = true;
        const bsp::LandConvoyArcStep reversed =
            bsp::land_convoy_advance_arc_00743060(f, 2.0f, 7.0f, 0.05f);
        check(std::fabs(reversed.live_arc - 47.0f) < 1e-4f,
            "00743060: 00D7A260 = -1 sends 2 to -3, and the add loop lifts it to 47");
        check(std::fabs(reversed.odometer - 12.0f) < 1e-4f,
            "00743126: the odometer takes |speed|, so a reversed convoy still counts up");
        f.reverse = false;

        // One conditional only: 0 + 2*30 = 60 wraps once to 10, but group 3 would
        // reach 90 and wrap only to 40; the loop version would not agree.
        check(std::fabs(bsp::land_convoy_group_arc_00742400(f, 0.0f, 2) - 10.0f) < 1e-4f,
            "00742400: the single subtract takes group 2's arc 60 to 10");

        check(std::fabs(bsp::land_convoy_lateral_offset_00742400(f, 0) + 15.0f) < 1e-4f,
            "00742400: file 0 of four sits at -(4-1)*0.5*10 = -15, the formation is centred");
        check(std::fabs(bsp::land_convoy_lateral_offset_00742400(f, 3) - 15.0f) < 1e-4f,
            "00742400: file 3 of four mirrors it at +15");

        const bsp::LandConvoySlot slot = bsp::land_convoy_slot_00742400(f, 9);
        check(slot.group == 2 && slot.lane == 1,
            "00742400: index 9 of a 3x4 block is rank 2, file 1");
    }

    {
        // 009329C0: a hull carrying pure lateral velocity must lose it while a hull
        // carrying the same speed forward keeps almost all of it. That asymmetry is the
        // whole point of the routine and it rests on two readings of the listing that a
        // sign or an axis swap would silently break: the lateral pair at settings+4F0h
        // and +4F4h drives the body's row-0 axis (00933199) while the forward axis takes
        // the linear term at +4F8h alone (0093324E). With the shipped `Ship` row the
        // lateral coefficients are a hundred times the forward one.
        struct Host final : bsp::ShipHydroHost {
            bsp::OceanVec3 velocity{};
            bsp::OceanVec3 force{};
            bsp::OceanVec3 body_linear_velocity_00c31f40() override { return velocity; }
            bsp::OceanVec3 body_angular_velocity_00c31f20() override { return {}; }
            bsp::ShipHydroTransform body_world_transform_00c33650() override { return {}; }
            bool unit_category_8_vtable5c() override { return false; }
            float water_height_0078cf20(float, float) override { return 0.0f; }
            void leak_tick_0074f930(float) override {}
            bsp::OceanVec3 leak_heel_torque_0074f2e0() override { return {}; }
            void add_force_00c35360(const bsp::OceanVec3& f) override { force = f; }
            void add_torque_00c35330(const bsp::OceanVec3&) override {}
            void set_linear_velocity_00c37e50(const bsp::OceanVec3&) override {}
            void set_angular_velocity_00c37e20(const bsp::OceanVec3&) override {}
            void set_body_no_gravity_flag_00932a16() override {}
        };

        bsp::ShipBuoyancyElement element{};
        element.coefficient = 0.0f;   // no buoyancy, so only the drag shows
        element.level_base = 0.0f;
        element.level_draft = 4.0f;   // fully submerged: the point sits at the waterline
        element.level_top = 8.0f;

        bsp::ShipHydroInputs in{};
        in.material = bsp::ShipPhysicsMaterial::kShip;
        in.record = bsp::ship_physics_material_shipped(in.material);
        in.class_mass = 1000.0f;
        in.elements = &element;
        in.element_count = 1;

        Host lateral{};
        lateral.velocity.x = 10.0f;  // pure row-0 motion
        const auto lateral_result = bsp::ship_hydro_apply_forces_009329c0(in, 0.05f, lateral);
        Host forward{};
        forward.velocity.z = 10.0f;  // pure row-2 motion, same speed
        const auto forward_result = bsp::ship_hydro_apply_forces_009329c0(in, 0.05f, forward);

        check(lateral_result.force.x < -1.0f,
            "009329C0: lateral velocity meets a drag force opposing it");
        check(forward_result.force.z < 0.0f,
            "009329C0: forward velocity meets a drag force opposing it too");
        check(-lateral_result.force.x > -forward_result.force.z * 10.0f,
            "009329C0: the lateral axis takes KozegellenallasiEgyutthatoLOldalra/NOldalra "
            "and the forward axis only the linear LElore, so lateral drag dominates");
        // dv = F * dt / mass. The clamp at 009335DD caps a single element at the whole
        // component, never past it, so the lateral speed falls toward zero and never
        // reverses.
        const float lateral_dv = lateral_result.force.x * 0.05f / in.class_mass;
        check(lateral_dv < 0.0f && lateral_dv >= -10.0f,
            "009335DD: one element's impulse cannot reverse the velocity it opposes");
    }

    {
        // 00A1E250. The Capture_MinimalCBTargetWeight floor is applied to
        // (a - b/2) + max(0, c - d/2) BEFORE StrategicGain is added, so a target
        // whose unit terms are deeply negative still prices at floor + gain and
        // the printed decomposition at 00D22CC8 does not add up to the total.
        bsp::AiTailCaptureScoreInputs in;
        in.own_value = 1.0f;
        in.enemy_value = 40.0f;          // (a - b/2) = -19
        in.own_resources = 0.0f;
        in.enemy_resources = 0.0f;       // (c - d/2) = 0, clamped arm not needed
        in.strategic_gain = 7.0f;
        in.min_cb_target_weight = 0.5f;
        const bsp::AiTailCaptureScoreTerms t = bsp::ai_tail_capture_score(in);
        check(std::fabs(t.total - 7.5f) < 1e-4f,
            "00A1E74B: the floor bites before 00A1E81D adds StrategicGain");
        check(std::fabs(t.own_value - 1.0f) < 1e-4f && std::fabs(t.enemy_value - 40.0f) < 1e-4f,
            "00A1E836: the six printed terms keep their unclamped values");
    }

    {
        // 0082D040: the buoyancy element list the ship class builds from its model's
        // deckline and bottomline. The shipped DeRuyter (VehicleClass[20]) numbers, the
        // class row for the scalars and Deruyter.mmod for the two polylines. Two things
        // the reconstruction could get wrong and the game would sink over: the station at
        // z = -85.5 lies before the bottom line's first point at z = -81.63, so it must
        // clamp rather than extrapolate, and the coefficients must normalise so that the
        // sum of coefficient * draught over the whole list is exactly 10 * Mass.
        bsp::ShipBuoyancyHullInputs hull{};
        hull.length = 171.0f;
        hull.width = 16.0f;
        hull.mass = 7688.0f;
        hull.hull_water_line_ratio = 0.55f;
        hull.hull_segments = 5;
        const std::vector<bsp::OceanVec3> deck{
            {-0.0953f, 6.3400f, 52.4409f}, {-0.0953f, 6.1480f, 11.9249f},
            {-0.0953f, 6.0555f, 9.2761f},  {-0.0953f, 6.1280f, -90.4472f}};
        const std::vector<bsp::OceanVec3> bottom{
            {-0.0487f, -4.9100f, 63.5168f},  {-0.0487f, -5.0100f, 55.5529f},
            {-0.0487f, -4.9620f, 29.7489f},  {-0.0487f, -4.9620f, -45.8951f},
            {-0.0487f, -4.9980f, -71.1791f}, {-0.0487f, -4.8860f, -81.6272f}};
        const auto elements = bsp::ship_buoyancy_build_list_0082d4ca(hull, deck, bottom);
        check(elements.size() == 10,
              "0082D040: two elements per station, 2 * Hull.Segments in all");
        check(std::fabs(elements[0].position.z + 85.5f) < 1e-3f &&
                  std::fabs(bsp::ship_buoyancy_element_bottom_level(elements[0]) + 4.8860f) <
                      1e-4f,
              "0082A920: a station before the polyline's first point clamps to its y");
        check(elements[0].position.x == 8.0f && elements[1].position.x == -8.0f,
              "0082D65D: the second push of a station negates the lateral offset");
        float displacement = 0.0f;
        for (const auto& element : elements) {
            displacement += element.coefficient * bsp::ship_buoyancy_element_draught(element);
        }
        check(std::fabs(displacement - 10.0f * hull.mass) < 1.0f,
              "0082D64B: the list's coefficients normalise to 10 * Mass, so 00937C90's "
              "displacement sum cancels Gravitacio * Mass exactly");
    }

    {
        // 009ED3E0's head: docs/SHIP_AI_PATH_CORRIDOR.md states that a plan is
        // thrown away only once an extent moves by more than 25 units, which is
        // two image rules composed - the +20 margin at 00CE3D88 and the 25.0
        // epsilon at 00CE3880 - plus the floor at 00CE3850. The boundary is the
        // number a reader will rely on, so it is the one worth pinning.
        bsp::ShipAiUnitGroupMember member{};
        member.lateral[0] = 30.0f;
        bsp::ShipAiUnitGroupView group{};
        group.members = &member;
        group.count = 1;
        group.column = 0;
        check(bsp::ship_ai_unit_group_extent_positive_0070d400(group) == 30.0f &&
                  bsp::ship_ai_unit_group_extent_negative_0070d5d0(group) == 5.0f,
              "0070D400 / 0070D5D0: a lone member 30 units to one side gives that side's "
              "extent and leaves the other at the 5.0f floor at 00CE3850");
        check(bsp::ship_ai_group_corridor_width_009ed41f(30.0f) == 50.0f,
              "009ED41F: a corridor width is the extent plus the 20.0 at 00CE3D88");
        check(!bsp::ship_ai_group_corridor_extent_change_invalidates(5.0f, 30.0f) &&
                  bsp::ship_ai_group_corridor_extent_change_invalidates(5.0f, 31.0f),
              "009D9DE0: the JA at 009D9E09 is strict, so an extent moving exactly 25 keeps "
              "the plan and 26 throws it away");
    }

    {
        // 00B63D50 is the one link docs/GUN_GRAVITY_ARC.md left unread, and the
        // claim that it inverts had no standing coverage. It is worth pinning
        // because every gun's commanded pitch and yaw passes through it: the
        // round trip has to come back to the identity, and it has to do so for
        // a non-unit scale, which is what separates this routine from a plain
        // transpose. The third case is the precondition - a sheared basis is
        // not inverted, and nothing in the routine says so.
        // docs/MATRIX_ORTHOGONAL_INVERSE.md.
        auto round_trip_deviation = [](const bsp::CameraMatrix& m) {
            bsp::CameraMatrix inverse{};
            bsp::derive_pose_affine_inverse_00b63d50(inverse, m);
            float worst = 0.0f;
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    double sum = 0.0;
                    for (int k = 0; k < 4; ++k) {
                        sum += static_cast<double>(m[static_cast<std::size_t>(r * 4 + k)]) *
                               static_cast<double>(inverse[static_cast<std::size_t>(k * 4 + c)]);
                    }
                    const float deviation =
                        std::fabs(static_cast<float>(sum) - (r == c ? 1.0f : 0.0f));
                    if (deviation > worst) worst = deviation;
                }
            }
            return worst;
        };
        // A yaw of 40 degrees about Y, translated - rows orthonormal.
        const float c40 = std::cos(0.6981317f);
        const float s40 = std::sin(0.6981317f);
        const bsp::CameraMatrix rotated{c40,  0.0f, -s40, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f,
                                        s40,  0.0f, c40,  0.0f, 3.5f, -12.25f, 7.0f, 1.0f};
        // The same basis with a per-axis scale, the case a transpose cannot
        // invert and the division by each row's own squared length can.
        const bsp::CameraMatrix scaled{c40 * 0.5f, 0.0f, -s40 * 0.5f, 0.0f,
                                       0.0f,       3.0f, 0.0f,        0.0f,
                                       s40 * 1.75f, 0.0f, c40 * 1.75f, 0.0f,
                                       -40.0f,     6.0f, 0.125f,      1.0f};
        // Rows 0 and 1 at 17 degrees to each other instead of 90.
        const bsp::CameraMatrix sheared{1.0f, 0.0f, 0.0f, 0.0f, 0.3f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        check(round_trip_deviation(rotated) < 1.0e-6f && round_trip_deviation(scaled) < 1.0e-5f &&
                  round_trip_deviation(sheared) > 0.25f,
              "00B63D50: M * inverse(M) is the identity to float tolerance for an orthonormal "
              "basis and for a per-axis-scaled one, and is not the identity for a sheared one - "
              "orthogonal rows are a precondition the branch-free routine never checks");
    }

    {
        // 007DB680's free-flight arm: what holds a plane up. Lift is
        // min(clamp((1 + aoa) * qq, -2, 2) * AccelCheatMul, ctl+90h) * 9.81 along
        // body up and gravity is AccelCheatMul * 9.81 along world down, so above
        // q == 1 - forward speed 1.8 * StallSpd - the two are the same number and
        // cancel. The installed Fighter row's StallSpd is 17.5 (desc+184h), which
        // puts the threshold at 31.5 m/s against a 141.666672 spawn airspeed.
        // XDrag and YDrag are left at zero: the installed values were not read,
        // and at zero body vertical velocity the damping term is zero either way.
        bsp::PlaneFreeFlightClass fighter;
        fighter.stall_spd = 17.5f;
        const bsp::PlaneFreeFlightTuning tuning;  // the PlaneGlobals.lua defaults
        bsp::PlaneFreeFlightState level;
        level.forward_speed = 141.666672f;
        level.body_velocity[2] = 141.666672f;
        level.world_velocity[2] = 141.666672f;
        level.world_altitude = 500.0f;  // below Ceiling, so no ceiling push

        const float level_accel =
            bsp::free_flight_world_up_acceleration(level, fighter, tuning, 1.0f / 30.0f);

        bsp::PlaneFreeFlightState stalled = level;
        stalled.forward_speed = 20.0f;  // under 1.8 * 17.5 == 31.5
        stalled.body_velocity[2] = 20.0f;
        stalled.world_velocity[2] = 20.0f;
        const float stalled_accel =
            bsp::free_flight_world_up_acceleration(stalled, fighter, tuning, 1.0f / 30.0f);

        // The self-correction: sinking makes ctl+40h negative, aoa is -vy / vz,
        // so the same airspeed now produces more lift than gravity takes away.
        bsp::PlaneFreeFlightState sinking = level;
        sinking.body_velocity[1] = -10.0f;
        sinking.world_velocity[1] = -10.0f;
        const float sinking_accel =
            bsp::free_flight_world_up_acceleration(sinking, fighter, tuning, 1.0f / 30.0f);

        check(std::fabs(level_accel) < 1.0e-4f && stalled_accel < -8.0f && sinking_accel > 1.0f,
              "007DB680: lift cancels gravity exactly at and above 1.8 * StallSpd, the plane "
              "sinks below it, and a downward perturbation raises the angle of attack until "
              "lift exceeds gravity again - the flight model needs no authored data beyond "
              "StallSpd and the Dynamics/* rows");
    }

    {
        // Packet cc8_ship_ai_approach_curves. 00955A40's index rule is what the
        // 119-step standoff scan at 009E71A5 walks, so an off-by-one there
        // moves every ship's chosen range by 50 m. samples[i] holds the value
        // at 50*(i+1) metres, so a ramp of samples[i] = i reads back as
        // (x/50 - 1) wherever the interpolation is exact.
        bsp::ShipAiApproachRangeCurve curve{};
        bsp::ship_ai_approach_curve_clear_00954940(curve);
        for (int i = 0; i < bsp::kShipAiApproachCurveSamples; ++i) {
            curve.samples[i] = static_cast<float>(i);
        }
        const float below = bsp::ship_ai_approach_curve_sample_00955a40(curve, 25.0f);
        const float first = bsp::ship_ai_approach_curve_sample_00955a40(curve, 50.0f);
        const float mid = bsp::ship_ai_approach_curve_sample_00955a40(curve, 175.0f);
        const float last = bsp::ship_ai_approach_curve_sample_00955a40(curve, 3000.0f);
        const float above = bsp::ship_ai_approach_curve_sample_00955a40(curve, 5000.0f);
        const float peak = bsp::ship_ai_approach_curve_peak_009523c0(curve);
        const float reach = bsp::ship_ai_approach_curve_effective_range_00952530(curve);

        bsp::ShipAiApproachRangeCurve empty{};
        bsp::ship_ai_approach_curve_clear_00954940(empty);
        const float empty_reach =
            bsp::ship_ai_approach_curve_effective_range_00952530(empty);

        check(std::fabs(below) < 1.0e-6f && std::fabs(first) < 1.0e-6f
                  && std::fabs(mid - 2.5f) < 1.0e-6f && std::fabs(last - 59.0f) < 1.0e-6f
                  && std::fabs(above - 59.0f) < 1.0e-6f && std::fabs(peak - 59.0f) < 1.0e-6f
                  && std::fabs(reach - 3000.0f) < 1.0e-3f
                  && std::fabs(empty_reach) < 1.0e-6f,
              "00955A40 samples the 60-point range curve at 50*(i+1) metres: at or below 50 m it "
              "returns samples[0] (00955A63), at 175 m it interpolates halfway between samples[2] "
              "and samples[3], and at or above 3000 m it saturates on samples[59] ([ECX+0ECh] at "
              "00955A83); 009523C0 answers the peak and 00952530 the last positive sample's range, "
              "or 0.0 when the curve is the zero 00954940 leaves");
    }

    {
        // 009D22FF-009D236E, the aim-complete byte state+2Ch. Both clauses are
        // strict `>` on operands the listing pushes in a fixed order, and a
        // reversed pair would be silent: the byte would latch on the first tick
        // or never. These four cases pin each direction.
        using bsp::torpedo_aim_complete_009d22ff;
        // The USN01 tick on which Mav2 first set the byte: clause 2 only.
        const bool turn_only = torpedo_aim_complete_009d22ff(
            78.04f, 1444.05f, 2.5755f, 2.5734f, 500.0f);
        // The same tick a frame earlier, the delta just under the time.
        const bool neither = torpedo_aim_complete_009d22ff(
            78.04f, 1444.05f, 2.5700f, 2.5734f, 500.0f);
        // Closed to inside the threshold: clause 1 alone.
        const bool range_only = torpedo_aim_complete_009d22ff(
            78.04f, 100.0f, 0.05f, 0.1f, 500.0f);
        // The ramp saturates at half the commanded speed, so a large time does
        // not keep growing it and the range clause must still refuse at 2000 m.
        const bool ramp_clamped = torpedo_aim_complete_009d22ff(
            78.04f, 2000.0f, 0.05f, 4.0f, 500.0f);
        check(turn_only && !neither && range_only && !ramp_clamped,
              "009D236E sets state+2Ch when 009D235A finds the 009D1DED release "
              "threshold above the range minus the 009D2345 ramp, or when "
              "009D2368 finds the steering delta above the 009D1500 time to "
              "target; the ramp is InterpolateClamped(0, 0, 1, speed/2, time) "
              "so it saturates at half the commanded speed");
    }

    {
        // 007CE9FD, the plane fixed step's release-order issue stage. USN01
        // only ever reaches the waiting arm, because unit+C20h stays at zero
        // all mission, so the issue and cleanup arms have no run-time cover.
        // The two things that would be silently wrong are the countdown's
        // position relative to the guards (007CEA51 is past them, so a
        // suppressed step must not advance it) and the timer park at 007CEAEE,
        // which the listing puts before the device walk and before the count
        // test, so a blocked cleanup still retries.
        using bsp::PlaneReleaseIssueStageArm;
        bsp::PlaneReleaseIssueStageInputs in;
        in.control_mode_900 = bsp::kIssueModeFreeFlight_7;
        in.step_seconds = 0.05f;
        in.interval_timer_c28 = 0.0f;
        in.issue_requests_c20 = 1;

        bsp::PlaneReleaseIssueStageInputs suppressed = in;
        suppressed.blocked_c3a = true;  // 007CEA1C
        const bsp::PlaneReleaseIssueStageResult sup =
            bsp::plane_release_issue_stage_007ce9fd(suppressed);

        const bsp::PlaneReleaseIssueStageResult issue =
            bsp::plane_release_issue_stage_007ce9fd(in);

        bsp::PlaneReleaseIssueStageInputs blocked = in;
        blocked.issue_requests_c20 = 0;
        blocked.release_pending_c25 = true;
        blocked.any_device_busy_1fc = true;
        blocked.interval_timer_c28 = -1.3f;  // already past 00D05EA4
        const bsp::PlaneReleaseIssueStageResult clean =
            bsp::plane_release_issue_stage_007ce9fd(blocked);

        check(sup.arm == PlaneReleaseIssueStageArm::kSuppressed &&
                  sup.next_interval_timer_c28 == 0.0f &&
                  sup.next_issue_requests_c20 == 1 &&
                  issue.arm == PlaneReleaseIssueStageArm::kIssue &&
                  issue.call_issue_007c0d90 &&
                  issue.next_issue_requests_c20 == 0 &&
                  clean.arm == PlaneReleaseIssueStageArm::kCleanupBlocked &&
                  clean.next_interval_timer_c28 == bsp::kCleanupRetry_00ce69d0 &&
                  !clean.clear_release_pending_c25,
              "007CE9FD leaves unit+C28h alone when a guard rejects, spends one "
              "unit+C20h and calls 007C0D90 on a countdown that has gone "
              "negative, and parks unit+C28h at 00CE69D0 even when a device "
              "answers vtable +1FCh true, because 007CEAEE precedes 007CEB00");
    }

    {
        // Retaining 17 authored records crosses the native 16-pointer growth
        // boundary. The muzzle reader's value extraction must preserve their
        // order after its temporary model and raw pointer backing are gone.
        auto word = [](std::vector<std::uint8_t>& bytes, std::uint32_t value) {
            for (unsigned shift = 0; shift < 32; shift += 8)
                bytes.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        auto chunk = [&](const char* tag, const std::vector<std::uint8_t>& payload) {
            std::vector<std::uint8_t> bytes;
            const auto length = static_cast<std::uint32_t>(std::strlen(tag));
            word(bytes, length);
            bytes.insert(bytes.end(), tag, tag + length);
            word(bytes, static_cast<std::uint32_t>(payload.size()));
            bytes.insert(bytes.end(), payload.begin(), payload.end());
            return bytes;
        };
        std::vector<std::uint8_t> hierarchy;
        for (std::uint32_t index = 0; index != 17; ++index) {
            std::vector<std::uint8_t> parent;
            word(parent, index);
            const auto item = chunk("Item", chunk("Parent", parent));
            hierarchy.insert(hierarchy.end(), item.begin(), item.end());
        }
        std::vector<std::uint8_t> root;
        word(root, 14);
        const auto section = chunk("Hierarchy", hierarchy);
        root.insert(root.end(), section.begin(), section.end());
        std::vector<bsp::HierarchyItem> items;
        std::string error;
        bool retained = false;
        {
            const auto bytes = chunk("MMOD", root);
            auto stream = std::make_shared<bsp::MemoryStream>(
                bsp::memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
            bsp::StructuredReader reader(stream);
            auto model_root = reader.read_root_00bea700();
            if (model_root && model_root->read_control_00be9a40()) {
                auto hierarchy_node = model_root->read_child_00bea680();
                bsp::StructuredModel model;
                retained = hierarchy_node && bsp::parse_hierarchy_00b7f100(
                    *hierarchy_node, model, error);
                if (retained) items = std::move(model.hierarchy);
                retained = retained && hierarchy_node->close() && model_root->close();
            }
        }
        retained = retained && items.size() == 17;
        for (std::size_t index = 0; retained && index != items.size(); ++index)
            retained = items[index].parent == index;
        check(retained, "00B87AE0/00B87350 retain 17 model records through muzzle extraction and teardown");
    }

    {
        // One serialized ownership regression: cross sixteen resource slots,
        // preserve an unsupported ordinal, then destroy the stream and moved-
        // from model before reading the retained records through the real array.
        auto word = [](std::vector<std::uint8_t>& bytes, std::uint32_t value) {
            for (unsigned shift = 0; shift < 32; shift += 8)
                bytes.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        auto chunk = [&](const char* tag, const std::vector<std::uint8_t>& payload) {
            std::vector<std::uint8_t> bytes;
            const auto length = static_cast<std::uint32_t>(std::strlen(tag));
            word(bytes, length);
            bytes.insert(bytes.end(), tag, tag + length);
            word(bytes, static_cast<std::uint32_t>(payload.size()));
            bytes.insert(bytes.end(), payload.begin(), payload.end());
            return bytes;
        };
        std::vector<std::uint8_t> resources;
        for (std::uint32_t index = 0; index != 17; ++index) {
            std::vector<std::uint8_t> payload;
            word(payload, 0x3f000000u + index);
            const auto item = chunk(index == 8 ? "Unsupported" : "GroupParams", payload);
            resources.insert(resources.end(), item.begin(), item.end());
        }
        std::vector<std::uint8_t> root_payload;
        word(root_payload, 14);
        const auto section = chunk("Resource", resources);
        root_payload.insert(root_payload.end(), section.begin(), section.end());
        bsp::StructuredResourceStorage retained;
        bool ordered = false;
        {
            const auto bytes = chunk("MMOD", root_payload);
            auto stream = std::make_shared<bsp::MemoryStream>(
                bsp::memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
            bsp::StructuredReader reader(stream);
            auto root = reader.read_root_00bea700();
            bsp::GroupParamsStructuredResourceParser parser;
            bsp::StructuredResourceRegistry registry;
            registry.register_parser(parser);
            bsp::RendererRootHooks_00d5f0a8 hooks;
            bsp::StructuredModel model;
            std::string error;
            ordered = root && bsp::dispatch_root_00b7f430(*root, registry, model, hooks, error)
                && root->close() && model.resources.size() == 17;
            if (ordered) {
                const auto* first = &model.resources[0];
                retained = std::move(model.resources);
                ordered = model.resources.empty() && &retained[0] == first;
            }
        }
        std::uint32_t index = 0;
        for (const auto& item : retained) {
            if (index == 8) {
                ordered = ordered && item.type_name == "Unsupported" && !item.payload;
            } else {
                const auto* group = item.payload
                    ? std::get_if<bsp::GroupParamsResourcePayload>(&*item.payload) : nullptr;
                std::uint32_t bits = 0;
                if (group) std::memcpy(&bits, &group->value, sizeof bits);
                ordered = ordered && item.type_name == "GroupParams" && group
                    && bits == 0x3f000000u + index;
            }
            ++index;
        }
        check(ordered && index == 17,
            "00B87AA0/00B872F0 resource growth preserves ordinals, payloads and ownership after move");
    }

    {
        // Chain426 regression: a large nested ConvexObject still uses the
        // root's format version. Version 11 omits the optional first DWORD;
        // version 12 includes it, independently of the payload's byte count.
        auto word = [](std::vector<std::uint8_t>& bytes, std::uint32_t value) {
            for (unsigned shift = 0; shift < 32; shift += 8)
                bytes.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        auto chunk = [&](const char* tag, const std::vector<std::uint8_t>& payload) {
            std::vector<std::uint8_t> bytes;
            const auto length = static_cast<std::uint32_t>(std::strlen(tag));
            word(bytes, length);
            bytes.insert(bytes.end(), tag, tag + length);
            word(bytes, static_cast<std::uint32_t>(payload.size()));
            bytes.insert(bytes.end(), payload.begin(), payload.end());
            return bytes;
        };
        bool versions_ok = true;
        for (const std::uint32_t version : {11u, 12u}) {
            std::vector<std::uint8_t> payload;
            if (version >= 12) word(payload, 0x1234u);
            word(payload, 3); // three nondegenerate vertices, IEEE-754 bits
            for (unsigned point = 0; point < 3; ++point) {
                word(payload, point == 1 ? 0x3f800000u : 0u);
                word(payload, point == 2 ? 0x3f800000u : 0u);
                word(payload, 0);
                for (int list = 0; list < 3; ++list) word(payload, 0);
            }
            auto resource = chunk("Resource", chunk("ConvexObject", payload));
            std::vector<std::uint8_t> reference;
            word(reference, 0); // hierarchy record zero owns resource zero
            auto hierarchy = chunk("Hierarchy", chunk("Item", chunk("Resource", reference)));
            std::vector<std::uint8_t> root_payload;
            word(root_payload, version);
            root_payload.insert(root_payload.end(), resource.begin(), resource.end());
            root_payload.insert(root_payload.end(), hierarchy.begin(), hierarchy.end());
            const auto bytes = chunk("MMOD", root_payload);
            bsp::MmodHullConvexBox box;
            std::string error;
            const bool hull_read = bsp::read_mmod_hull_convex_box(bytes, box, error);
            versions_ok = versions_ok && hull_read && box.shape_count == 1 &&
                box.point_count == 3 && box.shape_points[0][1].x == 1.0f &&
                box.shape_points[0][2].y == 1.0f;
        }
        check(versions_ok, "00BE9B20 uses the shared root version for ConvexObject format gates");
    }

    {
        // cc9_mmod_hull_convex_box (docs/GUNNERY_OPEN_ITEMS.md 49.5-49.7): the hull body's box
        // from this installation's models/ships/us/deruyter.mmod (mtime 2024-07-13 11:25:14
        // -0700, 46,865,813 bytes). Hierarchy record 0 (GroupRoot_Deruyter, the node at
        // instance+0Ch) lists one ConvexObject, resource 125 at file+2C5AC00, 117 points,
        // raw box (-8.23195, -5.65156, -96.90411)..(8.23195, 12.19929, 77.02396); widened
        // by 0.02 (00C57C40) the extent is 16.50 x 17.89 x 173.97. The game installation is
        // outside the repository: the path comes from config/target.json's `binary`, and the
        // check is skipped, not failed, when that file is absent.
        std::filesystem::path model;
        {
            const std::filesystem::path config =
                std::filesystem::path(__FILE__).parent_path().parent_path() / "config" / "target.json";
            std::ifstream in(config);
            const std::string config_text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const std::size_t key = config_text.find("\"binary\"");
            const std::size_t open = key == std::string::npos ? key : config_text.find('"', config_text.find(':', key));
            const std::size_t close = open == std::string::npos ? open : config_text.find('"', open + 1);
            if (close != std::string::npos) {
                model = std::filesystem::path(config_text.substr(open + 1, close - open - 1)).parent_path() /
                        "models" / "ships" / "us" / "deruyter.mmod";
            }
        }
        std::ifstream file(model, std::ios::binary);
        if (!file) {
            std::cout << "skipped: deruyter.mmod not found (" << model.string() << ")\n";
        } else {
            const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                                                  std::istreambuf_iterator<char>());
            bsp::MmodHullConvexBox box;
            std::string error;
            const bool ok = bsp::read_mmod_hull_convex_box(bytes, box, error);
            check(ok && box.shape_count == 1 && box.point_count == 117 &&
                      std::fabs((box.max.x - box.min.x) - 16.5039f) < 1e-3f &&
                      std::fabs((box.max.y - box.min.y) - 17.8909f) < 1e-3f &&
                      std::fabs((box.max.z - box.min.z) - 173.9681f) < 1e-3f &&
                      std::fabs(box.min.z + 96.92411f) < 1e-3f,
                  "0071B710/00938F61: deruyter.mmod keeps only record 0's hull ConvexObject, "
                  "box widened by 0.02 per 00C57C40");
        }
    }

    {
        // cc13-rendershapes: one case per former render_fx row shape.
        using namespace bsp::cc13_render1;
        alignas(8) std::uint8_t obj[0x150]{};
        for (std::size_t i = 0; i < sizeof(obj); ++i) obj[i] = static_cast<std::uint8_t>(i * 7 + 1);
        std::uint32_t v = 0;
        std::memcpy(&v, obj + 0x24, 4);
        check(d3d9_surface_sample_count_00b3cd30(obj) == v, "00B3CD30 reads +24h");
        d3d9_surface_set_sample_count_00b3ce20(obj, 0x12345678u);
        check(d3d9_surface_sample_count_00b3cd30(obj) == 0x12345678u, "00B3CE20 writes +24h");
        check(material_effect_byte13c_00b400b0(obj) == obj[0x13c], "00B400B0 reads +13Ch");
        check(shader_effect_pass_record_00b40050(obj, 2) == obj + 0x144 + 0x80, "00B40050 +144h+40h*i");
        std::uint8_t* const base = obj + 0x40;
        std::memcpy(obj + 0x0c, &base, 4);
        check(vertex_declaration_element_at_00b47c80(obj, 3) == base + 0x3c, "00B47C80 [+0Ch]+14h*i");
        check(pod_pair_zero_init_00b4ec30(obj + 0x100) == obj + 0x100 && obj[0x100] == 0 && obj[0x107] == 0
                  && obj[0x108] != 0, "00B4EC30 zeroes 8 bytes and returns self");
        post_effect_owner_noop_slot0c_00b4ec40(obj);
        check(d3d9_texture2d_kind_00b3ce40(obj) == 0, "00B3CE40 returns 0");
        check(volume_texture_kind_00b3d080(obj) == 2, "00B3D080 returns 2");

        struct LockObject { CRITICAL_SECTION cs; std::uint32_t depth; } lock{};
        static_assert(offsetof(LockObject, depth) == 0x18, "depth dword at +18h");
        InitializeCriticalSection(&lock.cs);
        lock.depth = 5;
        LockObject* const lp = &lock;
        std::memcpy(obj + 0x48, &lp, 4);
        std::memcpy(obj + 0x4c, &lp, 4);
        std::uint32_t f40 = 0;
        std::memcpy(&f40, obj + 0x40, 4);
        check(locked_object_field40_00b5df90(obj) == f40 && lock.depth == 5, "00B5DF90 locked +40h");
        check(locked_object_byte45_00b5e020(obj) == obj[0x45] && lock.depth == 5, "00B5E020 locked +45h");
        DeleteCriticalSection(&lock.cs);

        struct Host final : bsp::cc13_render_sprites::SlabPoolHost {
            void enter(void*) override {}
            void leave(void*) override {}
            void* operator_new(std::size_t n) override { return ::operator new(n); }
            void free(void* p) override { ::operator delete(p); }
        } host;
        alignas(8) std::uint8_t pool[0x40]{};
        std::memset(pool + 0x34, 0xff, 4);   // no free slab
        void* const slot = hardware_layout_pool_allocate_raw_slot_stub_00b488d0(pool, host);
        std::uint8_t** slabs = nullptr;
        std::memcpy(&slabs, pool + 0x28, 4);
        std::uint16_t before = 0, after = 0;
        std::memcpy(&before, slabs[0] + 0x8c0, 2);
        hardware_layout_return_raw_slot_00b47c10(pool, slot, host);
        std::memcpy(&after, slabs[0] + 0x8c0, 2);
        check(slot != nullptr && after == before + 1, "00B488D0/00B47C10 forward to 00B48780/00B479C0");
        ::operator delete(slabs[0]);
        ::operator delete(slabs);
    }

    {
        // Chain 271: one synthetic sequence uses the complete native context
        // operation and two actual action records. Check table writes, maximum
        // priority AND action activation through publish/clear. No gameplay proof.
        struct Records final : bsp::NativeInputActionRecordCalls {
            void call_listener_slot0(void*, std::uint32_t) override {
                throw std::logic_error("fixture has no listener allocation");
            }
        } records_calls;
        struct Devices final : bsp::NativeInputActionBindingCalls {
            std::uint8_t relative_vslot1c(void*, std::uint32_t, std::uint32_t) override {
                throw std::logic_error("fixture has no device bindings");
            }
            std::uint8_t query_vslot20(void*, std::uint32_t, std::uint32_t) override {
                throw std::logic_error("fixture has no device bindings");
            }
            float value_vslot24(void*, std::uint32_t, std::uint32_t) override {
                throw std::logic_error("fixture has no device bindings");
            }
        } devices;
        const volatile float half = 0.5f, negative_zero = -0.0f, one = 1.0f;
        const volatile double quarter = 0.25, half_double = 0.5, gain = 0.875, eighth = 0.125;
        const volatile std::uint32_t one_bits = 0x3f800000, bypass = 1;
        const bsp::CameraAxesCrtAccess crt{&bypass,
            [](std::int32_t, bsp::CameraAxesCrtException*, std::uint16_t*) {
                throw std::logic_error("fixture has no arithmetic device bindings");
            }};
        bsp::NativeInputActionBindingContext bindings{devices, crt,
            {half, negative_zero, one, quarter, half_double, gain, eighth}};
        bsp::NativeInputActionRecordsContext records{{one_bits}, records_calls};
        bsp::NativeInputActionStorageCalls action_storage(records);
        bsp::InputActionTimingThresholds timing;
        bsp::NativeInputActionConfigurationContext configuration{records, bindings,
            {timing.quick_edge_and_hold, timing.repeat_delay, timing.repeat_step}};
        bsp::NativeInputActionOwnerStorage owner;
        auto* const raw = static_cast<std::uint8_t*>(
            bsp::construct_native_input_action_owner_00a93da0(&owner));
        void* volatile publication = raw;
        void* volatile unused_manager = nullptr; // owner getter takes its nonnull fast path
        bsp::NativeInputActionOwnerContext owner_context{publication,
            bsp::SoundLifetimeAccess(unused_manager), action_storage};
        bsp::GameInputContextManagerBinding manager{owner_context, configuration};
        manager.prepare();
        const std::uint32_t action_contexts[] = {6, 0x14};
        for (std::uint32_t index = 0; index < 2; ++index) {
            const bsp::NativeInputArrayHeader source{const_cast<std::uint32_t*>(&action_contexts[index]), 1, 1};
            bsp::configure_native_input_action_00a93c80(raw, index, &source, 0, configuration);
        }
        const auto level = [&](int id) {
            return bsp::sound_cc13::input_manager_context_level_00a92290(raw, id);
        };
        const auto active = [&](unsigned index) {
            const auto actions = bsp::cc13_seg10::native_at<std::uint8_t*>(raw, 4);
            return actions[index * 0x30 + 1] != 0;
        };
        bsp::GameInputContextSetStack contexts;
        contexts.manager = &manager;
        const int top[] = {8, 0};
        const int second[] = {4, 6, 0, 9};
        const int third[] = {6, 0x14}; // bounded host list need not carry a zero
        const int none[] = {0};
        bsp::set_game_input_context_set_004d8c00(contexts, top, 2);
        bsp::publish_native_input_context_set(contexts, 2, second, 4);
        check(contexts.levels[1] == std::vector<int>({4, 6})
            && level(6) == 2 && level(9) == 0 && level(0x1D) == 4
            && bsp::cc13_seg10::native_at<std::uint32_t>(raw, 0x20) == 4
            && !active(0) && !active(1), "chain271 actual level-4 maximum disables lower actions");
        bsp::set_game_input_context_set_004d8c00(contexts, none, 1);
        bsp::publish_native_input_context_set(contexts, 3, third, 2);
        check(level(6) == 3 && level(0x14) == 3 && level(0x1D) == 3
            && bsp::cc13_seg10::native_at<std::uint32_t>(raw, 0x20) == 3
            && active(0) && active(1), "chain271 level-3 publication activates actual actions");
        bsp::publish_native_input_context_set(contexts, 3, nullptr, 1);
        check(contexts.levels[2].empty() && level(6) == 2 && level(0x14) == 0 && level(0x1D) == 2
            && bsp::cc13_seg10::native_at<std::uint32_t>(raw, 0x20) == 2
            && active(0) && !active(1), "chain271 clearing level 3 refreshes action activation");
        bsp::publish_native_input_context_set(contexts, 2, none, 1);
        check(contexts.levels[1].empty() && level(6) == 0 && level(0x1D) == 1
            && bsp::cc13_seg10::native_at<std::uint32_t>(raw, 0x20) == 1
            && !active(0) && !active(1) && contexts.context_level[6] == 0,
            "chain271 empty stacks refresh actions at floor 1 without a menu mirror");
        bsp::destroy_native_input_action_owner_00a93dd0(raw, owner_context);
    }

    {   // Chain 342: the live dispatch boundary must retain name/owner identity
        // and consume exactly one reference, including the nonterminal case.
        struct Stream { std::uint32_t table; LONG references; } stream{0x00d691b0u, 2};
        struct Calls final : bsp::engine_services_cc13::StreamReferenceCalls {
            void* manager{};
            std::uint32_t name{}, flags{};
            void* result{};
            void* destroyed{};
            unsigned terminal_calls{};
            bool fail_open{};
            void* open(void* actual_manager, std::uint32_t actual_name,
                std::uint32_t actual_flags) override {
                manager = actual_manager; name = actual_name; flags = actual_flags;
                if (fail_open) throw std::runtime_error("open failed");
                return result;
            }
            void zero_reference(void* object) override {
                destroyed = object;
                ++terminal_calls;
            }
        } calls;
        calls.result = &stream;
        RecordingStorage name_storage;
        bsp::NativeString name;
        name.assign_0041e870(name_storage, "nav");
        std::uint32_t manager[2]{};
        void* slot = nullptr;
        auto* const returned = bsp::engine_services_cc13::vfs_open_stream_into_00be4380(
            &slot, manager, reinterpret_cast<std::uint32_t>(&name), 0x32, &calls);
        check(returned == &slot && slot == &stream && calls.manager == manager
            && calls.name == reinterpret_cast<std::uint32_t>(&name) && calls.flags == 0x32,
            "chain342 open forwards actual header, manager, flags and returned owner");
        bsp::engine_services_cc13::release_ref_slot_unchecked_00be4500(&slot, &calls);
        check(slot == nullptr && stream.references == 1 && calls.terminal_calls == 0,
            "chain342 nonterminal release clears slot without destruction");
        slot = &stream; // The remaining real owner releases its own reference.
        bsp::engine_services_cc13::release_ref_slot_unchecked_00be4500(&slot, &calls);
        check(slot == nullptr && stream.references == 0 && calls.terminal_calls == 1
            && calls.destroyed == &stream, "chain342 terminal release destroys the same stream once");
        calls.result = nullptr;
        bsp::engine_services_cc13::vfs_open_stream_into_00be4380(
            &slot, manager, reinterpret_cast<std::uint32_t>(&name), 2, &calls);
        check(slot == nullptr, "chain342 null open is preserved");
        slot = &stream;
        calls.fail_open = true;
        bool threw = false;
        try {
            bsp::engine_services_cc13::vfs_open_stream_into_00be4380(
                &slot, manager, reinterpret_cast<std::uint32_t>(&name), 2, &calls);
        } catch (const std::runtime_error&) { threw = true; }
        check(threw && slot == &stream, "chain342 failed open propagates before storing a result");
        name.release_to(name_storage);
    }

    {
        // Regression: the original 0088DC10 table-building tail must preserve
        // all five field names and values, rather than returning one scalar.
        struct FirepowerHost final : bsp::UnitFirepowerLuaHost {
            bsp::UnitFirepowerReport report;
            std::vector<std::pair<std::string, float>> fields;
            bool refreshed{}, table{};
            bsp::NativeHandle entity_from_lua_argument(int index) override {
                check(index == 0, "GetFirepower resolves argument zero"); return 7;
            }
            bsp::UnitFirepowerReport refresh_firepower(bsp::NativeHandle unit) override {
                check(unit == 7 && !table, "GetFirepower refresh precedes table");
                refreshed = true; return report;
            }
            void lua_new_result_table() override {
                check(refreshed, "GetFirepower refreshed real report"); table = true;
            }
            void lua_result_number_field(const char* key, float value) override {
                check(table, "GetFirepower field belongs to result table");
                fields.emplace_back(key, value);
            }
            int lua_result_count() override { return 1; }
        } host;
        host.report = {{0.0f, 0.5f, 0.0f, 1.0f}, 0.0f};
        check(bsp::lua_get_firepower_0088dc10(host) == 1 && host.fields ==
            std::vector<std::pair<std::string, float>>{
                {"ArtilleryFirePowerRatio", 0.0f}, {"AAFirePowerRatio", 0.5f},
                {"TorpedoFirePowerRatio", 0.0f}, {"DCFirePowerRatio", 1.0f},
                {"Manoeuvrability", 0.0f}},
            "GetFirepower returns the original five-field table");
    }

    {
        // Chain 457: native insertion must leave a balanced extension multimap
        // whose strings survive the normalized temporary pairs being released.
        using namespace bsp::engine_services_cc13;
        RecordingStorage strings;
        PrefixTreeNode head{};
        head.left = head.parent = head.right = &head;
        head.color_1c = head.isnil_1d = 1;
        NativeContainerHeader tree{0, &head, 0};
        struct Pair { bsp::NativeString key, value; };
        static_assert(sizeof(Pair) == 16);
        const char* keys[] = {"m", "a", "z", "b", "a", ""};
        const char* values[] = {"middle", "first", "last", "second", "other", "fallback"};
        for (std::size_t i = 0; i < 6; ++i) {
            Pair pair;
            pair.key.assign_0041e870(strings, keys[i]);
            pair.value.assign_0041e870(strings, values[i]);
            InsertResult result{};
            prefix_tree_insert_equal_00be0b90(tree, result, &pair, strings);
            const auto* node = static_cast<const PrefixTreeNode*>(result.it.node);
            check(result.inserted && result.it.container == &tree
                && (pair.key.length() == 0 ? node->key_0c.length() == 0
                    : node->key_0c.data() != pair.key.data())
                && node->value_14.data() != pair.value.data(),
                "chain457 insertion owns deep copies of both headers");
            pair.value.release_to(strings);
            pair.key.release_to(strings);
        }
        std::vector<std::string> ordered;
        const auto inspect = [&](auto&& self, PrefixTreeNode* node) -> int {
            if (node == &head) return 1;
            check(!node->isnil_1d && (node->left == &head || node->left->parent == node)
                && (node->right == &head || node->right->parent == node),
                "chain457 insertion preserves parent and sentinel links");
            if (node->color_1c == 0)
                check(node->left->color_1c == 1 && node->right->color_1c == 1,
                    "chain457 red nodes have black children");
            const int left = self(self, node->left);
            ordered.push_back(std::string(node->key_0c.data() ? node->key_0c.data() : "")
                + ":" + node->value_14.data());
            const int right = self(self, node->right);
            check(left == right, "chain457 insertion preserves black height");
            return left + (node->color_1c == 1);
        };
        inspect(inspect, head.parent);
        check(tree.size == 6 && head.parent->parent == &head && head.parent->color_1c == 1
            && head.left->key_0c.length() == 0 && std::strcmp(head.right->key_0c.data(), "z") == 0
            && ordered == std::vector<std::string>{":fallback", "a:first", "a:other",
                "b:second", "m:middle", "z:last"},
            "chain457 empty and equal extension keys retain native order and values");
        const auto retire = [&](auto&& self, PrefixTreeNode* node) -> void {
            if (node == &head) return;
            self(self, node->left);
            self(self, node->right);
            node->value_14.release_to(strings);
            node->key_0c.release_to(strings);
            bsp::singleton_lifetime_free(node);
        };
        retire(retire, head.parent);
        check(strings.allocated.size() == strings.released.size(),
            "chain457 temporary and tree string ownership balances");

    }

    {
        // Chain 522 review: the native PC24 FMUL/FADD and FMUL/FDIV each
        // round separately. One Lights fixture covers the 35-degree one-ULP
        // regression and cancellation that exposes a retained double product.
        using namespace bsp::scene_weather_cc13;
        bsp::ScenePropertyBagModel model;
        const auto number = [&](int bag, const char* key, float value) {
            bsp::ScenePropertyEntry entry;
            entry.key = key;
            entry.value.type = bsp::ScenePropertyType::Float;
            entry.value.float_value = value;
            model.bags[static_cast<std::size_t>(bag)].entries.push_back(entry);
        };
        const auto child = [&](int parent, const char* key) {
            const int bag = static_cast<int>(model.bags.size());
            model.bags.emplace_back();
            bsp::ScenePropertyEntry entry;
            entry.key = key;
            entry.value.type = bsp::ScenePropertyType::SubBag;
            entry.sub_bag = bag;
            model.bags[static_cast<std::size_t>(parent)].entries.push_back(entry);
            return bag;
        };
        number(0, "ParticleMultiplier", 1.0f);
        number(0, "BushShadowAllowed", 0.0f);
        number(0, "BushDiffuseFactor", 1.0f);
        number(0, "BushNormalAllowed", 0.0f);
        bsp::ScenePropertyEntry texture;
        texture.key = "BushTextureName";
        texture.value.type = bsp::ScenePropertyType::String;
        texture.value.text = "fixture.dds";
        model.bags[0].entries.push_back(texture);
        // (1+2^-23)^2 rounds to 1+2^-22 at PC24 before adding -(1+2^-22).
        // Native alpha is +0; retaining a double product yields 2^-46 instead.
        constexpr float luminance = 1.00000011920928955078125f;
        number(0, "AmbientLumScale", luminance);
        number(0, "AmbientLumOffset", -1.0000002384185791015625f);
        const auto colour = [&](int bag) {
            number(bag, "R", 1.0f); number(bag, "G", 1.0f); number(bag, "B", 1.0f);
            number(bag, "L", luminance);
        };
        for (const char* name : {"Ambient", "UWAmbient", "Diffuse", "UWDiffuse", "Specular"})
            colour(child(0, name));
        const int hdr = child(child(0, "AmbientCube"), "HDR");
        for (const char* face : {"Left", "Right", "Top", "Bottom", "Front", "Back"})
            colour(child(hdr, face));
        const int angles = child(0, "Angles");
        number(angles, "HorizontalAngle", 35.0f);
        number(angles, "VerticalAngle", 35.0f);
        struct SceneSettings final : SceneLightsHost {
            float diffuse{};
            std::string texture_name;
            std::uint8_t normal{};
            void set_bush_diffuse_factor_00ad7140(float value) override { diffuse = value; }
            void set_bush_texture_name_00ad7fe0(const std::string& value) override { texture_name = value; }
            void set_bush_normal_allowed_00ad5730(std::uint8_t value) override { normal = value; }
        } settings;
        SceneLightsFields fields;
        std::string error;
        const bool loaded = read_sun_light_properties_004ea870(model, 0, false, fields, settings, error);
        const auto scene_word = [](float value) {
            std::uint32_t word;
            std::memcpy(&word, &value, sizeof word);
            return word;
        };
        check(loaded && error.empty() && scene_word(fields.ambient_f48.a) == 0
            && scene_word(fields.ambient_cube_f68[0].a) == 0
            && scene_word(fields.horizontal_angle_1004) == 0x3f1c61abu
            && scene_word(fields.vertical_angle_1008) == 0x3f1c61abu,
            "chain522 scene luminance and 35-degree angles round each native PC24 operation");
    }

    {
        // c688: cross the native cache's first occupied growth boundary.
        // Resource ownership must not change while record-owned lists move.
        bsp::SoundResourceOwner owner;
        bsp::SoundOwnedResource resource;
        resource.references_04 = 7;
        for (std::uint32_t i = 0; i != 65; ++i) {
            bsp::SoundResourceCacheRecord record;
            record.name_00 = "bank-" + std::to_string(i);
            record.aliases_0c = {record.name_00, "request-" + std::to_string(i)};
            record.field_08 = i;
            record.metadata_14 = {i, i + 1, i + 2, i + 3, i + 4};
            record.resource_28 = &resource;
            bsp::append_sound_resource_cache_record_00a84530(owner, record);
        }
        bool preserved = owner.capacity_0c == 128 && owner.records_04.size() == 65
            && resource.references_04 == 7;
        for (std::uint32_t i = 0; i != owner.records_04.size(); ++i) {
            const auto& record = owner.records_04[i];
            preserved = preserved && record.name_00 == "bank-" + std::to_string(i)
                && record.aliases_0c == std::list<std::string>{record.name_00,
                    "request-" + std::to_string(i)}
                && record.field_08 == i
                && record.metadata_14 == std::array<std::uint32_t, 5>{i, i + 1, i + 2, i + 3, i + 4}
                && record.resource_28 == &resource;
        }
        const auto* const data = owner.records_04.data();
        bsp::sound_cc13::sound_cache_records_reserve_00a84450(owner, 1);
        check(preserved && owner.records_04.data() == data && owner.capacity_0c == 128,
            "c688 sound-cache growth preserves aliases, metadata and borrowed resource ownership");

        // c831: removal must assign the tail without moving its list allocator
        // or releasing a borrowed resource, and then pop only record storage.
        bsp::sound_cc13::sound_cache_records_erase_swap_last_00a84d20(owner, owner.records_04[2]);
        const auto& replaced = owner.records_04[2];
        const bool erased = owner.records_04.size() == 64 && replaced.name_00 == "bank-64"
            && replaced.aliases_0c == std::list<std::string>{"bank-64", "request-64"}
            && replaced.field_08 == 2 && replaced.resource_28 == &resource
            && replaced.metadata_14 == std::array<std::uint32_t, 5>{64, 65, 66, 67, 68};
        bsp::sound_cc13::sound_cache_records_pop_back_00a84380(owner);
        check(erased && owner.records_04.size() == 63 && owner.records_04.data() == data
            && owner.capacity_0c == 128 && resource.references_04 == 7,
            "c831 sound-cache erase/pop preserve allocator and borrowed resource identities");

        // c842: canonical resize must use the same owner, preserve retained
        // records across both kinds of growth, and destroy storage without
        // releasing borrowed resources or changing cache byte accounting.
        owner.total_resource_size_10 = 123;
        bsp::oct18_tail_c::sound_cache_resize_00a845a0(owner, 65);
        const bool spare_growth = owner.records_04.data() == data
            && owner.records_04.size() == 65 && owner.capacity_0c == 128
            && owner.records_04.back().name_00.empty()
            && owner.records_04.back().aliases_0c.empty()
            && owner.records_04.back().metadata_14 == std::array<std::uint32_t, 5>{};
        bsp::oct18_tail_c::sound_cache_resize_00a845a0(owner, 1);
        bsp::oct18_tail_c::sound_cache_resize_00a845a0(owner, 130);
        const bool retained = owner.records_04.size() == 130 && owner.capacity_0c == 130
            && owner.records_04.front().name_00 == "bank-0"
            && owner.records_04.front().aliases_0c == std::list<std::string>{"bank-0", "request-0"}
            && owner.records_04.front().resource_28 == &resource;
        bsp::oct18_tail_c::sound_cache_resize_00a845a0(owner, 0);
        check(spare_growth && retained && owner.records_04.empty()
            && owner.capacity_0c == 130 && resource.references_04 == 7
            && owner.total_resource_size_10 == 123,
            "c842 canonical sound-cache resize preserves resource ownership and accounting");
    }

    {
        // c732 review: a message must follow the retained mission selection,
        // even when a context predates slot publication/selection changes.
        bsp::SessionParticipantPools participants(
            bsp::ParticipantPoolInitialization::ZeroedGameAllocation);
        bsp::ParticipantRecordId claimed;
        const bool initialized = participants.reset_and_claim_local_flags(true, 2, claimed)
            == bsp::ParticipantClaimResult::Claimed;
        const void* const player = participants.active_record_identity(0);
        const void* const mission = participants.active_record_identity(1);
        std::size_t local_slot = 0;
        bsp::NativeSessionMessageParticipantOwner selected(participants, local_slot);
        // Deliberately unusable: the typed context must never fall back to a
        // second raw game image while the retained owner is supplied.
        void* volatile no_raw_game = nullptr;
        const bsp::NativeSessionMessageContext context{no_raw_game, &selected};
        bsp::NativeSessionMessageStorage message{};
        bsp::construct_native_session_message_0075b430(&message, 0xa6, context);
        bool follows_owner = initialized && player != nullptr && mission != nullptr
            && player != mission && message.selected_owner_14 == player;
        follows_owner = participants.publish_active_record(0, {bsp::ParticipantPool::Mission, 1})
            && follows_owner;
        bsp::construct_native_session_message_0075b430(&message, 0xa6, context);
        follows_owner = follows_owner && message.selected_owner_14 == mission;
        follows_owner = participants.publish_active_record(1, claimed) && follows_owner;
        local_slot = 1;
        bsp::construct_native_session_message_0075b430(&message, 0xa6, context);
        follows_owner = follows_owner && message.selected_owner_14 == player;
        local_slot = bsp::kSceneSlotRecordCount;
        bsp::construct_native_session_message_0075b430(&message, 0xa6, context);
        check(follows_owner && message.selected_owner_14 == nullptr,
            "c732 message owner borrows current mission slot and canonical participant identity");
    }

    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
