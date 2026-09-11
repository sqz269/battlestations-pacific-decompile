#include "bsp/app_bootstrap.hpp"
#include "bsp/award_grant.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/blocking_screen.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/gui_icon.hpp"
#include "bsp/gui_layer.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_render_order.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/gui_widget_scene.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/frontend_entry.hpp"
#include "bsp/frontend_screen_animation.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/hud_screens.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/ingame_interface.hpp"
#include "bsp/game_render_frame.hpp"
#include "bsp/lua_binding_entity_lookup.hpp"
#include "bsp/math.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/title_init.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/native_string.hpp"
#include "bsp/renderer_startup.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_unit_creators.hpp"
#include "bsp/unit_controller.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/input_settings.hpp"
#include "bsp/loading_screen_elements.hpp"
#include "bsp/main_menu_screen.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/press_start_screen.hpp"
#include "bsp/profile_unlock.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/world_construct.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/mission_events.hpp"
#include "bsp/mission_result.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_named_call_args.hpp"
#include "bsp/mission_tree_data.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_effects_startup.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/plane_class_fields.hpp"
#include "bsp/vehicle_class_fields.hpp"
#include "bsp/vehicle_class_lua_load.hpp"
#include "bsp/scene_property_bag.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>
#include <string>

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
        // 004cd7f0 keys every numbered VFS block of the mission load, and the
        // order of its two tail steps is the part that is easy to get wrong:
        // the truncation happens at the SECOND underscore, and only a name that
        // survives it still carries the ".scn" the last step drops.
        check(bsp::derive_scene_short_name("universe\\scenes\\pearl_harbor_01.scn")
                == "pearl_harbor",
            "the short name is cut at the second underscore, before the extension test");
        check(bsp::derive_scene_short_name("universe/scenes/midway.scn") == "midway",
            "a name with no second underscore keeps its stem and loses .scn");
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
            "    FilePath = S \"islands/a\" ;\n"
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
        };

        MapSizeView absent;
        absent.table_present = false;
        bsp::MissionRecordData without{};
        bsp::read_mission_record_005c6a70(absent, without);

        MapSizeView present;
        bsp::MissionRecordData with{};
        bsp::read_mission_record_005c6a70(present, with);

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

    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
