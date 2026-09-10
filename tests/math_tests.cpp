#include "bsp/app_bootstrap.hpp"
#include "bsp/award_grant.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/blocking_screen.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/frontend_entry.hpp"
#include "bsp/frontend_screen_animation.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/game_render_frame.hpp"
#include "bsp/math.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/title_init.hpp"
#include "bsp/native_string.hpp"
#include "bsp/renderer_startup.hpp"
#include "bsp/input_settings.hpp"
#include "bsp/loading_screen_elements.hpp"
#include "bsp/main_menu_screen.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/press_start_screen.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_effects_startup.hpp"
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
        // poll moves the state off 3 the shell is abandoned: no manager is
        // created and 004e4279 never writes 5. Reading that gate as "still 4"
        // would build the front end after a sign-out and leave the state at 3.
        struct ShellHost final : bsp::FrontEndShellHost {
            std::int32_t state_after_poll = 3;
            int managers_created = 0;
            int end_loading_calls = 0;
            bsp::LoadingScreenConfig globals{};
            void renderer_set_budget(std::uint32_t) override {}
            void probe_texture_memory(const char*) override {}
            void probe_sound_memory(const char*) override {}
            void probe_effect_memory(const char*) override {}
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
            void begin_section(const char*) override {}
            void end_section() override {}
            void write_field(const char* key, const bsp::SettingsValue&) override
            {
                keys.emplace_back(key);
            }
            void write_keyboard_setup() override {}
        } recorder;
        bsp::write_settings_008d64a0(settings, recorder);
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

    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
