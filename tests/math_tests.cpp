#include "bsp/app_bootstrap.hpp"
#include "bsp/game_entry.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/math.hpp"
#include "bsp/native_string.hpp"
#include "bsp/input_settings.hpp"
#include "bsp/input_tick.hpp"
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

    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
