// Packet cc13-sys5: segment 56 bindings and helpers. See docs/CC13_SYS_5.md and the header for
// addresses, ABI and evidence. Comments cite the instruction each statement stands for.
#include "bsp/engine_seg56_cc13.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>

#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace bsp::cc13_seg56 {
// Complete normal 008B01B0..008B0465. Evidence: failed-single-player gates
// 008B025E..0286, tree receiver/key setup028C..0299, both prompt call stacks,
// focus calls and end-scene push0 at044B. Names reflect existing prompt API;
// CRT/tree/GUI dependencies remain explicit and native FH3 is unproven.
int lua_game_end_scene_008b01b0(void* const& game_global,
    std::uint32_t& flags, std::uint32_t& debug,
    NativeStringRawPoolContext& strings, EndSceneHost& host) {
    struct Temporary {
        EndSceneString value;
        NativeStringRawPoolContext& strings;
        Temporary(const char* text, NativeStringRawPoolContext& context) : strings(context) {
            construct_native_string_header_0041e870(&value, strings, text);
        }
        ~Temporary() { destroy_native_string_header_0041dd20(&value, strings); }
    };
    { Temporary failure("luaMW_EndScene failed:", strings); }
    if ((flags & 1) == 0) {
        flags |= 1;
        Temporary tag("luakod", strings);
        debug = 0;
    }
    auto* game = static_cast<unsigned char*>(game_global);
    auto* commits = *reinterpret_cast<unsigned char**>(game + 0x21A0);
    const std::uint32_t index = *reinterpret_cast<std::uint32_t*>(commits + 0x1424);
    if (*reinterpret_cast<std::int32_t*>(commits + index * 0x284u + 4) == 0 &&
        *reinterpret_cast<std::int32_t*>(game + 0x1FE4) == 0) {
        const bool checkpoint = host.contains_007f8d60(game + 0x650, game + 0x2198);
        {
            Temporary title("", strings);
            Temporary message(checkpoint ? "globals.restartfromcheckpoint" :
                "globals.restartmission|.\n|FE_xbox.ingame_quitting|.\n|globals.areyousure", strings);
            Temporary owned("", strings);
            void* menu = host.menu_00425d10();
            EndSceneString transferred = owned.value;
            owned.value = {};
            host.prompt_00531b00(menu, checkpoint ? 5 : 4, &message.value,
                checkpoint ? 4 : 1, 0x0088B360, 0, &title.value, 0.0f, 0, transferred, 1);
        }
        void* menu = host.menu_00425d10();
        if (checkpoint) host.focus_first_00530670(menu);
        else host.focus_last_00530c20(menu);
        return 0;
    }
    host.end_scene_004d7970(0);
    return 0;
}

namespace {
template<class T> T& perf_raw(void* object, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(object) + offset);
}
void* perf_bytes(void* object, std::size_t offset) {
    return static_cast<unsigned char*>(object) + offset;
}
void perf_empty_head(void* head) {
    perf_raw<std::uint8_t>(head, 0x19) = 1;
    perf_raw<void*>(head, 4) = head;
    perf_raw<void*>(head, 0) = head;
    perf_raw<void*>(head, 8) = head;
}
}

// 008D1A10 complete normal listing: +4/+10 are tree owners; +8/+14 are
// their sentinel pointers, +C/+18 sizes. Original ECX=this, EAX=this, RET0.
// Library allocation/erase remain explicit bindings, not reconstructed credit.
void* construct_perf_timing_registry_008d1a10(void* self, PerfTimingRawHost& host) {
    perf_raw<std::uint32_t>(self, 0) = 0x00D15860;
    perf_raw<void*>(self, 8) = host.buy_count_head_0088a520(perf_bytes(self, 4));
    perf_empty_head(perf_raw<void*>(self, 8));
    perf_raw<std::uint32_t>(self, 0xC) = 0;
    perf_raw<void*>(self, 0x14) = host.buy_timer_head_005826b0(perf_bytes(self, 0x10));
    perf_empty_head(perf_raw<void*>(self, 0x14));
    perf_raw<std::uint32_t>(self, 0x18) = 0;
    return self;
}

// 008D1AB0 complete normal body: timer range/head, count range/head, global
// slot clear, base vptr. Original ECX=this/RET0; native FH3 is unproven.
void destruct_perf_timing_registry_008d1ab0(void* self, void*& slot,
    PerfTimingRawHost& host) {
    std::uint32_t output[2];
    void* tree = perf_bytes(self, 0x10);
    void* head = perf_raw<void*>(self, 0x14);
    host.erase_timers_0058d860(tree, output, tree, perf_raw<void*>(head, 0), tree, head);
    host.free_00bf65ac(perf_raw<void*>(self, 0x14));
    perf_raw<void*>(self, 0x14) = nullptr;
    perf_raw<std::uint32_t>(self, 0x18) = 0;
    tree = perf_bytes(self, 4);
    head = perf_raw<void*>(self, 8);
    host.erase_counts_008cfd30(tree, output, tree, perf_raw<void*>(head, 0), tree, head);
    host.free_00bf65ac(perf_raw<void*>(self, 8));
    perf_raw<void*>(self, 8) = nullptr;
    perf_raw<std::uint32_t>(self, 0xC) = 0;
    slot = nullptr;
    perf_raw<std::uint32_t>(self, 0) = 0x00CE3818;
}

// 008D1B80..008D1C3C listing: double check under manager+10 lock, update
// the lock's custom recursion word+18, allocate1Ch, construct, publish then
// register (including a null result). Original no arguments, RET0/EAX=slot.
// Names provisional; explicit raw storage and Win32 imports, no game wiring.
void* get_perf_timing_registry_008d1b80(void*& slot, PerfTimingRawHost& host) {
    if (slot == nullptr) {
        void* manager = host.singleton_manager_00415350();
        void* lock = perf_raw<void*>(manager, 0x10);
        if (lock) {
            EnterCriticalSection(static_cast<CRITICAL_SECTION*>(lock));
            ++perf_raw<std::uint32_t>(lock, 0x18);
        }
        struct Guard {
            void* lock;
            ~Guard() {
                if (lock) {
                    --perf_raw<std::uint32_t>(lock, 0x18);
                    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(lock));
                }
            }
        } guard{lock};
        if (slot == nullptr) {
            void* memory = host.allocate_00bf681b(0x1C);
            try {
                slot = memory ? construct_perf_timing_registry_008d1a10(memory, host) : nullptr;
            } catch (...) {
                host.free_00bf65ac(memory);
                throw;
            }
            manager = host.singleton_manager_00415350();
            host.register_00bd0c30(manager, slot);
        }
    }
    return slot;
}

namespace {

std::string format_text(const char* format, ...)
{
    char buffer[512];
    va_list list;
    va_start(list, format);
    std::vsnprintf(buffer, sizeof buffer, format, list);
    va_end(list);
    return std::string(buffer);
}

float narrow(double value) noexcept
{
    return static_cast<float>(value); // every float argument goes through FSTP dword
}

} // namespace

// ---------------------------------------------------------------------------
// Debug and tooling
// ---------------------------------------------------------------------------

int lua_set_single_thread(LuaArgs& args, DebugSwitchBytes& g)
{
    *g.job_pool_run_pass_00e0b6ce = args.get_boolean(0) ? 0 : 1; // SETE, stored at 008A199D
    return 0;
}

int lua_set_ship_debug_filter(LuaArgs& args, DebugSwitchBytes& g)
{
    *g.ship_debug_filter_00e0e300 = args.get_integer(0); // 008A16D8
    return 0;
}

int lua_avoid_zone_debug_layer(LuaArgs& args, DebugSwitchBytes& g)
{
    *g.avoid_zone_debug_layer_00e08040 = args.get_integer(0); // 008A1838
    return 0;
}

int lua_terminate_execution(DebugSwitchBytes& g)
{
    *g.terminate_execution_00e1ae75 = 1; // 008C833A
    return 0;
}

int lua_empty_binding() noexcept
{
    return 0;
}

int lua_dprintf(LuaArgs& args, ToolingHost& host)
{
    const float clock = host.clock_seconds_00f876a4(); // 008C7EAC, read before the string
    const std::string text = args.get_string(0);
    host.log_text(format_text("[tick: %d, time: %.3f] %s", host.game_tick_648(),
        static_cast<double>(clock), text.c_str())); // 004254B0
    return 0;
}

int lua_memory_status(LuaArgs& args, ToolingHost& host)
{
    const std::int32_t free_bytes = host.memory_manager_free_bytes(); // [0109CEFC] vtable+4
    const double kKiB = 0.0009765625;                                 // 00CEDAE8
    const double mib = static_cast<double>(free_bytes) * kKiB * kKiB; // 008C7C99..008C7CAA
    const std::string label = args.get_string(0);
    host.log_text(format_text("Free memory @%s %.1f\n", label.c_str(), mib));
    return 0;
}

int lua_region(LuaResults& results, ToolingHost& host)
{
    results.push_string(host.user_geography_region()); // 00439100 -> 00B66710
    return 1;
}

int lua_start_measure(MeasureStart& state, ToolingHost& host)
{
    state.tick_00f887ec = host.game_tick_648();           // 008C79F6
    state.clock_00f887e8 = host.clock_seconds_00f876a4(); // 008C79FC
    state.counter_00f887e0 = host.query_performance_counter(); // 008C7A12/008C7A1C
    return 0;
}

std::string stop_measure_text(std::int32_t ticks, std::int64_t counter_delta,
    std::int64_t frequency)
{
    // 008C7B4B..008C7B6C: FILD the 64-bit delta and frequency, FDIVP, then ticks / seconds
    // and seconds * 1000.0 (00CE47A0).
    const double seconds = static_cast<double>(counter_delta) / static_cast<double>(frequency);
    return format_text("tickcount:%d realtime:%f ms fps:%f\n", ticks, seconds * 1000.0,
        static_cast<double>(ticks) / seconds);
}

int lua_stop_measure(const MeasureStart& state, ToolingHost& host)
{
    const std::int32_t ticks = host.game_tick_648() - state.tick_00f887ec; // 008C7B07
    const std::int64_t delta = host.query_performance_counter() - state.counter_00f887e0;
    const std::int64_t frequency = host.query_performance_frequency();
    host.output_debug_string(stop_measure_text(ticks, delta, frequency)); // 00BF7A6A, [00CE2274]
    return 0;
}

int lua_mpak(LuaArgs& args, ToolingHost& host)
{
    host.vfs_enter_file_block(args.get_string(0)); // 00BE0980, ECX = [0109CEEC]
    return 0;
}

bool PerfTimingRegistry::CaseInsensitiveLess::operator()(
    const std::string& a, const std::string& b) const noexcept
{
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) {
        const int lhs = std::tolower(static_cast<unsigned char>(a[i]));
        const int rhs = std::tolower(static_cast<unsigned char>(b[i]));
        if (lhs != rhs) {
            return lhs < rhs;
        }
    }
    return a.size() < b.size();
}

void PerfTimingRegistry::start(const std::string& name)
{
    int& count = counts_[name]; // 005070C0
    if (count == 0) {
        blocks_[name].reset(new std::uint8_t(0)); // operator new(1) -> 008CFB20 slot
    }
    ++count;
}

void PerfTimingRegistry::end(const std::string& name)
{
    int& count = counts_[name];
    --count;
    if (count == 0) {
        blocks_[name].reset(); // _free and the slot set to 0
    }
}

int PerfTimingRegistry::count(const std::string& name) const
{
    const auto it = counts_.find(name);
    return it == counts_.end() ? 0 : it->second;
}

bool PerfTimingRegistry::has_block(const std::string& name) const
{
    const auto it = blocks_.find(name);
    return it != blocks_.end() && it->second != nullptr;
}

int lua_perf_timing_start(LuaArgs& args, PerfTimingRegistry& registry)
{
    registry.start(args.get_string(0));
    return 0;
}

int lua_perf_timing_end(LuaArgs& args, PerfTimingRegistry& registry)
{
    registry.end(args.get_string(0));
    return 0;
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------

float degrees_to_radians_float(double degrees) noexcept
{
    return narrow((degrees * 3.1415927410125732) / 180.0); // FMUL [00CE3D28], FDIV [00CE3D20]
}

int lua_set_dof(LuaArgs& args, CameraHost& host)
{
    const float far_value = narrow(args.get_number(1));  // 0089732C, ESI = argument 1
    const float near_value = narrow(args.get_number(0)); // 00897337, EDI = argument 0
    host.depth_of_field_set_range(near_value, far_value); // 00897345, ECX = 00E081A0
    return 0;
}

int lua_free_camera_set_pos(LuaArgs& args, CameraHost& host)
{
    Vec3 position;
    position.x = narrow(args.get_number(0));
    position.y = narrow(args.get_number(1));
    position.z = narrow(args.get_number(2));
    const float third = degrees_to_radians_float(args.get_number(3));  // 00892566 [esp+38h]
    const float fourth = degrees_to_radians_float(args.get_number(4)); // 008925A5 [esp+10h]
    if (!host.free_camera_present()) {
        return 0;
    }
    host.free_camera_set_position(position);
    host.free_camera_set_rotation_from_euler(fourth, third, 0.0f); // 00467050, FLDZ at 008925C6
    return 0;
}

int lua_player_set_bomber_view(LuaArgs& args, BomberViewFloats& g)
{
    const float first = narrow(args.get_number(0));
    const float second = narrow(args.get_number(1));
    *g.first_00e17678 = first;  // 008A09A7
    *g.second_00e17674 = second; // 008A09B5
    *g.value_00e17670 = 1.0f;    // 008A09BB, 00D7A24C
    return 0;
}

int lua_movcam_set_zoom(LuaArgs& args, CameraHost& host)
{
    const float zoom = narrow(args.get_number(0));
    host.movcam_ensure_target(); // 005CD160; the float was pushed before it for the next call
    host.movcam_set_zoom(zoom);  // 0078E480, ECX = the returned movcam+30h
    return 0;
}

int lua_movcam_set_speed_limits(LuaArgs& args, CameraHost& host)
{
    const float second = narrow(args.get_number(1));
    const float first = narrow(args.get_number(0));
    host.movcam_set_speed_limits(first, second);
    return 0;
}

int lua_movcam_reset_speed_limits(CameraHost& host)
{
    host.movcam_reset_speed_limits();
    return 0;
}

int lua_movcam_set_accel_time(LuaArgs& args, CameraHost& host)
{
    host.movcam_set_accel_time(narrow(args.get_number(0)));
    return 0;
}

int lua_movcam_reset_accel_time(CameraHost& host)
{
    host.movcam_reset_accel_time();
    return 0;
}

int lua_movcam_set_deviation_factor(LuaArgs& args, CameraHost& host)
{
    const float factor = narrow(args.get_number(0));
    host.movcam_set_deviation_factor(factor);
    if (host.game_session_mode_1fe4() == 1) { // non-zero test, then == 1
        host.session_broadcast_movcam_deviation_factor(factor); // 0076D660
    }
    return 0;
}

int lua_movcam_reset_deviation_factor(CameraHost& host)
{
    host.movcam_reset_deviation_factor();
    return 0;
}

int lua_movcam_set_overview_height(LuaArgs& args, CameraHost& host)
{
    host.movcam_set_overview_height(narrow(args.get_number(0)));
    return 0;
}

int lua_movcam_reset_overview_height(CameraHost& host)
{
    host.movcam_reset_overview_height();
    return 0;
}

int lua_movcam_new_add_positions(LuaArgs& args, CameraHost& host)
{
    void* camera = host.hud_movie_camera();
    host.hud_movie_camera_add_positions(camera, args, 0); // no null test in this one
    return 0;
}

int lua_movcam_new_add_position(LuaArgs& args, CameraHost& host)
{
    void* camera = host.hud_movie_camera();
    if (camera != nullptr) { // CMP ESI,EBP / JE 008B7B4C
        host.hud_movie_camera_add_positions(camera, args, 0);
    }
    return 0;
}

int lua_movcam_new_set_fov(LuaArgs& args, CameraHost& host)
{
    void* camera = host.hud_movie_camera();
    if (camera != nullptr) {
        host.hud_movie_camera_set_fov(camera, degrees_to_radians_float(args.get_number(0)));
    }
    return 0;
}

int lua_camera_shake(LuaArgs& args, CameraHost& host)
{
    host.camera_shake_accumulate(narrow(args.get_number(0)));
    return 0;
}

int lua_camera_set_shake_max(LuaArgs& args, CameraHost& host)
{
    host.camera_set_shake_max(narrow(args.get_number(0)));
    return 0;
}

int lua_camera_set_shake_base(LuaArgs& args, CameraHost& host)
{
    host.camera_set_shake_base(narrow(args.get_number(0)));
    return 0;
}

int lua_camera_set_shake_decay(LuaArgs& args, CameraHost& host)
{
    host.camera_set_shake_decay(narrow(args.get_number(0)));
    return 0;
}

int lua_get_camera_pos_rot(LuaResults& results, CameraHost& host)
{
    const Vec3 position = host.game_camera_world_position();
    const Vec3 angles = host.game_camera_basis_angles();
    results.push_vector3_fields({{"pos", position}, {"rot", angles}}); // 00CF8FA4, 00D14188
    return 1;
}

// ---------------------------------------------------------------------------
// Map screen, sound, input and GUI
// ---------------------------------------------------------------------------

int lua_get_map_cursor_pos(LuaResults& results, InterfaceHost& host)
{
    Vec3 cursor;
    cursor.x = host.map_cursor_x_554();
    cursor.y = 0.0f;
    cursor.z = host.map_cursor_z_558();
    results.push_vector3(cursor);
    return 1;
}

int lua_get_map_zoom(LuaResults& results, InterfaceHost& host)
{
    results.push_number(host.map_zoom_2c()); // 00B664B0
    return 1;
}

int lua_force_map_cursor_pos(LuaArgs& args, InterfaceHost& host)
{
    if (!args.is_boolean(0)) {
        host.map_force_cursor_pos(args.read_vector3(0));
        return 0;
    }
    if (!args.get_boolean(0)) { // true does nothing
        host.map_release_cursor_0059ad50();
    }
    return 0;
}

int lua_map_hide_node_by_name(LuaArgs& args, InterfaceHost& host)
{
    const bool flag = args.boolean_or(1, false); // 008C04B1, PUSH EBP = 0
    host.map_set_node_visible_by_name(args.get_string(0), flag);
    return 0;
}

int lua_map_unit_highlight(LuaArgs& args, InterfaceHost& host)
{
    void* entity = args.entity_at(0);
    const bool on = args.get_boolean(1);
    host.map_set_unit_highlight(entity, on);
    return 0;
}

int lua_music_control_suspend(LuaArgs& args, InterfaceHost& host)
{
    host.music_set_channel_byte_4e(0, args.get_boolean(0)); // PUSH EAX, PUSH EBP = 0
    return 0;
}

int lua_music_fade(LuaArgs& args, InterfaceHost& host)
{
    host.music_set_target_level(narrow(args.get_number(0)));
    return 0;
}

int lua_black_bars(LuaArgs& args, InterfaceHost& host)
{
    const bool show = args.get_boolean(0);
    host.engine_movie_screen_black_bars(show); // engine-movie screen first on both paths
    host.movie_screen_black_bars(show);
    return 0;
}

int enable_input_level(bool enable) noexcept
{
    const std::uint32_t mask = enable ? 0xFFFFFFFFu : 0u; // NEG/SBB of the boolean
    return static_cast<int>((mask & 0xFFFFFFFBu) + 5u);   // AND EAX,FFFFFFFBh; ADD EAX,5
}

int lua_enable_input(LuaArgs& args, InterfaceHost& host)
{
    host.input_set_context_level(kScriptInputContext, enable_input_level(args.get_boolean(0)));
    return 0;
}

int lua_get_input_on(LuaArgs& args, LuaResults& results, InterfaceHost& host)
{
    const int index = args.get_integer(0);
    // 008BFE6B COMISS against the float 0.0 at 00D7A218: on only when strictly positive.
    const bool on = host.input_record_enabled(index) && host.input_record_value(index) > 0.0f;
    results.push_boolean(on);
    return 1;
}

int lua_set_input_modifiers(LuaArgs& args, InputModifierBytes& bytes, InterfaceHost& host)
{
    for (int i = 0; i < 4; ++i) {
        bytes.backup[i] = bytes.current[i]; // 008C122D..008C126E
    }
    bytes.current[0] = args.get_boolean(0) ? 1 : 0; // 008C128B
    bytes.current[1] = args.get_boolean(1) ? 1 : 0; // 008C12BF
    bytes.current[2] = args.get_boolean(2) ? 1 : 0; // 008C12F2
    host.options_save_input_modifiers();
    return 0;
}

int lua_get_input_modifiers(LuaResults& results, const InputModifierBytes& bytes)
{
    results.push_boolean(bytes.current[0] != 0);
    results.push_boolean(bytes.current[1] != 0);
    results.push_boolean(bytes.current[2] != 0);
    return 3;
}

int lua_is_swap_sticks_enabled(LuaResults& results, const InputModifierBytes& bytes)
{
    results.push_boolean(bytes.current[0] != 0); // 008C444E, byte 00F889C1
    return 1;
}

int lua_is_limbo_gui_active(LuaResults& results, InterfaceHost& host)
{
    const bool active = host.interface_present() && host.limbo_screen_present()
        && host.limbo_screen_active_byte4();
    results.push_boolean(active);
    return 1;
}

int lua_is_gui_active(LuaArgs& args, LuaResults& results, InterfaceHost& host)
{
    bool active = false;
    if (host.gui_find_widget(args, 0) != nullptr) {
        void* widget = host.gui_find_widget(args, 0); // looked up a second time
        active = host.gui_widget_vcall_38(widget);
    }
    results.push_boolean(active);
    return 1;
}

int lua_set_gui_highlight(LuaArgs& args, InterfaceHost& host)
{
    void* widget = host.gui_find_widget(args, 0);
    if (widget == nullptr) {
        return 0;
    }
    const bool on = args.get_boolean(1);
    const bool a = args.count() > 2 ? args.get_boolean(2) : false;
    const bool b = args.count() > 3 ? args.get_boolean(3) : false;
    if (on) {
        host.marker_add_gui_highlight(widget, a, b);
    } else {
        host.marker_remove_gui_highlight(widget);
    }
    return 0;
}

int lua_get_raw_text(LuaArgs& args, LuaResults& results, InterfaceHost& host)
{
    // The native adds F4h to the lookup result without a null test (ADD ESI,0F4h).
    results.push_string(host.gui_widget_raw_text(host.gui_find_widget(args, 0)));
    return 1;
}

int lua_show_safe_zone(InterfaceHost& host)
{
    host.gui_load_page_and_show(kSafeZoneGuiPage);
    return 0;
}

int lua_enable_objectives_toggle(LuaArgs& args, InterfaceHost& host)
{
    host.objectives_toggle_disabled_byte(!args.get_boolean(0));
    return 0;
}

int lua_hide_unit_hp(LuaArgs& args, InterfaceHost& host)
{
    const int id = args.count() > 0 ? args.get_integer(0) : 0;
    host.hud_counters_hide_unit_hp(id);
    return 0;
}

int lua_hide_score_display(LuaArgs& args, InterfaceHost& host)
{
    const int second = args.get_integer(1);
    const int first = args.get_integer(0);
    host.hud_counters_hide_score(first, second);
    return 0;
}

int lua_display_scores(LuaArgs& args, HudScoreHost& host)
{
    if (args.count() == 6) {
        const int f = args.get_integer(5);
        const int e = args.get_integer(4);
        const std::string d = args.get_string(3);
        const std::string c = args.get_string(2);
        const int b = args.get_integer(1); // ESI, read first of the two
        const int a = args.get_integer(0);
        host.display_score_pair(a, b, c, d, e, f); // 008C22DA
    } else if (args.count() == 4) {
        const std::string d = args.get_string(3);
        const std::string c = args.get_string(2);
        const int b = args.get_integer(1);
        const int a = args.get_integer(0);
        host.display_score(a, b, c, d); // 008C241A
    }
    return 0;
}

int lua_display_unit_hp(LuaArgs& args, HudScoreHost& host)
{
    int id = 0;
    int next = 0;
    if (args.count() >= 1 && args.is_integer(0)) {
        id = args.get_integer(0);
        next = 1;
    }
    std::vector<void*> entities;
    std::string text;
    if (args.count() > next && args.is_table(next)) {
        entities = host.entity_list_0088a900(args, next);
        if (args.count() > next + 1 && args.is_string(next + 1)) {
            text = args.get_string(next + 1);
        }
    }
    host.display_unit_hp(id, entities, text);
    return 0;
}
// ---------------------------------------------------------------------------
// Localisation, environment, narrative, effects and classes
// ---------------------------------------------------------------------------

int lua_add_lockit_path_to_selection(LuaArgs& args, ContentHost& host)
{
    // The native copies the string once more, substituting the empty buffer 00F8790C for a
    // null pointer, before 00AA0D30.
    host.localization_register_table_name(args.get_string(0));
    return 0;
}

int lua_load_selected_paths(ContentHost& host)
{
    host.localization_reload_tables(0); // PUSH EBP = 0; the binding returns a literal 0
    return 0;
}

int lua_set_sky(LuaArgs& args, ContentHost& host)
{
    host.atmosphere_replace_sky_box(args.get_string(0));
    return 0;
}

int lua_set_rain_drop_num_percent(LuaArgs& args, ContentHost& host)
{
    const float percent = narrow(args.get_number(0));        // 008C7FFB
    const float fraction = narrow(percent / 100.0);          // FDIV qword 00D7A220
    host.rain_set_drop_num_percent(fraction);                // 008003B0
    host.rain_settings_store_70(fraction);                   // 008C803C
    return 0;
}

int lua_set_day_time(LuaArgs& args, ContentHost& host)
{
    const int hours = args.get_integer(0);
    const int minutes = args.count() < 2 ? 0 : args.get_integer(1);
    const int seconds = args.count() < 3 ? 0 : args.get_integer(2);
    host.time_of_day_set(hours, minutes, seconds, 0);
    return 0;
}

int lua_get_day_time(LuaResults& results, ContentHost& host)
{
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    host.time_of_day_split(hours, minutes, seconds);
    results.push_integer_fields({{"hours", hours}, {"minutes", minutes}, {"seconds", seconds}});
    return 1;
}

int lua_set_light(LuaArgs& args, ContentHost& host)
{
    const double kByte = 255.0; // 00CE4B48
    for (int i = 0; i < 3; ++i) {
        host.light_store_float(kLightAmbientOffset + 4 * i,
            narrow(args.table_field_element(0, "Ambient", i + 1) / kByte));
    }
    host.light_store_float(kLightAmbientOffset + 12, 1.0f);
    for (int i = 0; i < 3; ++i) {
        host.light_store_float(kLightDiffuseOffset + 4 * i,
            narrow(args.table_field_element(0, "Diffuse", i + 1) / kByte));
    }
    host.light_store_float(kLightDiffuseOffset + 12, 1.0f);
    host.light_store_float(kLightHorizontalAngleOffset,
        degrees_to_radians_float(args.table_field_number_or(0, "HorizontalAngle", 0.0)));
    host.light_store_float(kLightVerticalAngleOffset,
        degrees_to_radians_float(args.table_field_number_or(0, "VerticalAngle", 0.0)));
    host.scene_ensure_default_lighting();
    return 0;
}

int lua_is_class_changed(LuaArgs& args, LuaResults& results, ContentHost& host)
{
    const int class_id = args.get_integer(0);
    results.push_boolean(host.vehicle_class_remap(class_id) != class_id); // CMP/SETNE
    return 1;
}

int lua_mission_narrative_size(LuaResults& results, ContentHost& host)
{
    results.push_number(host.narrative_size());
    return 1;
}

int lua_mission_narrative_clear(ContentHost& host)
{
    host.narrative_clear(); // through the LuaStateHolder path; returns a literal 0
    return 0;
}

int lua_mission_narrative_party(LuaArgs& args, ContentHost& host)
{
    const std::string text = args.get_string(1); // read first
    host.narrative_send_to_party(args.get_integer(0), text);
    return 0;
}

int lua_mission_narrative_player(LuaArgs& args, ContentHost& host)
{
    const std::string text = args.get_string(1);
    host.narrative_send_to_player(args.get_integer(0), text);
    return 0;
}

int lua_break_dialog(LuaArgs& args, ContentHost& host)
{
    host.dialog_break(args.get_string(0));
    return 0;
}

int lua_kill_dialog(LuaArgs& args, ContentHost& host)
{
    host.dialog_kill(args.get_string(0));
    return 0;
}

int lua_start_dialog(LuaArgs& args, DialogStartHost& host)
{
    host.dialog_start_from_lua(args.get_string(0), args, 1);
    if (host.game_session_mode_1fe4() == 1) {
        host.session_send_dialog_e1(args.get_string(0), args, 1);
    }
    return 0;
}
int lua_stop_effect(LuaArgs& args, ContentHost& host)
{
    void* effect = args.to_userdata(0);
    host.point_effect_stop_children(effect); // no null test
    host.point_effect_set_byte9(effect);
    return 0;
}

int lua_destroy_effect(LuaArgs& args, ContentHost& host)
{
    void* effect = args.count() != 0 ? args.to_userdata(0) : nullptr;
    host.effect_manager_stop_matching(effect);
    return 0;
}

// ---------------------------------------------------------------------------
// Sound, movie, listener and loading
// ---------------------------------------------------------------------------

float sound_fade_rate(float level, float target, float duration) noexcept
{
    const float delta = level - target;               // 008D17E2 FSUB, stored as float
    if (0.009999999776482582 > static_cast<double>(duration)) {
        duration = 0.01f;                             // 00D7A238
    }
    const float quotient = narrow(static_cast<double>(delta) / duration); // 008D18F1
    return quotient < 0.0f ? -quotient : quotient;    // sign bit cleared
}

int lua_sound_fade(LuaArgs& args, PresentationHost& host)
{
    const float target = narrow(args.get_number(0));
    const float level = host.voice_current_level_d4();
    float duration = host.global_config_default_fade_e0();
    std::string name;
    if (args.count() > 1) {
        name = args.get_string(1);
    }
    if (args.count() > 2) {
        duration = narrow(args.get_number(2));
    }
    host.voice_begin_sound_fade(target, sound_fade_rate(level, target, duration), name);
    return 0;
}

int lua_set_skip_movie(LuaArgs& args, PresentationHost& host)
{
    std::string name;
    if (args.count() > 0) {
        name = args.get_string(0);
    }
    host.set_skip_movie_name(name);
    return 0;
}

int lua_force_3d_visibility(LuaArgs& args, Force3dVisibilityByte& g, PresentationHost& host)
{
    if (args.count() == 1) {
        *g.flag_00f87152 = args.get_boolean(0) ? 1 : 0;
        return 0;
    }
    void* entity = host.lua_table_entity_ptr(args, 0);
    const bool visible = args.get_boolean(1);
    if (entity != nullptr) {
        host.entity_set_force_3d_visible_2f0(entity, visible);
    }
    return 0; // the native stores 00F87152 back into itself on this path
}

int lua_add_listener(LuaArgs& args, PresentationHost& host)
{
    const std::string second = args.get_string(1); // 008C683D PUSH 1, read first
    const std::string first = args.get_string(0);
    // 008C68CC: 00980C10 on [00F8A0C4] with (arg0, arg1, LuaObject arg2 built in place).
    host.listener_add(first, second, args, 2);
    return 0;
}

int lua_remove_listener(LuaArgs& args, PresentationHost& host)
{
    const std::string second = args.get_string(1);
    const std::string first = args.get_string(0);
    host.listener_remove(first, second);
    return 0;
}

int lua_is_listener_active(LuaArgs& args, LuaResults& results, PresentationHost& host)
{
    const std::string second = args.get_string(1);
    const std::string first = args.get_string(0);
    results.push_boolean(host.listener_active(first, second));
    return 1;
}

int lua_get_act_dialog_ids(LuaStringListResults& results, PresentationHost& host)
{
    results.push_string_array(host.dialog_active_ids());
    return 1;
}

int lua_loading_finish(PresentationHost& host)
{
    if (host.game_loading_depth_644() < 1) {
        host.please_wait_screen_hide();
        host.game_set_cinematic_mode(0, 0, 1);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Camera reads, message maps, configured input and class unloading
// ---------------------------------------------------------------------------

int lua_get_camera_rel_pos_w(LuaArgs& args, LuaResults& results, GlueHost& host)
{
    void* entity = args.entity_at(0);
    const Vec3 camera = host.game_camera_world_position();
    const Vec3 position = host.entity_world_position(entity);
    Vec3 relative;
    relative.x = camera.x - position.x;
    relative.y = camera.y - position.y;
    relative.z = camera.z - position.z;
    results.push_vector3(relative);
    return 1;
}

int lua_get_camera_rel_pos_l(LuaArgs& args, LuaResults& results, GlueHost& host)
{
    void* entity = args.entity_at(0);
    const Vec3 camera = host.game_camera_world_position();
    results.push_vector3(host.entity_world_to_local(entity, camera));
    return 1;
}

int lua_unload_message_map(LuaArgs& args, GlueHost& host)
{
    host.message_map_unload(args.get_string(0), args.get_integer(1));
    if (host.game_session_mode_1fe4() == 1) {
        const std::string name = args.get_string(0);
        host.session_manage_messages(name, args.get_integer(1), 1); // PUSH ESI = 1
    }
    return 0;
}

int lua_get_configured_input(LuaArgs& args, LuaStringListResults& results, GlueHost& host)
{
    std::vector<std::string> first;
    std::vector<std::string> second;
    host.input_collect_action_binding_texts(args.get_integer(0), first, second);
    results.push_string_array(first);
    results.push_string_array(second);
    return 2;
}

int lua_force_enable_input(LuaArgs& args, GlueHost& host)
{
    if (!args.get_boolean(1)) {
        host.forced_input_set_insert(args.get_integer(0));
    } else {
        host.forced_input_set_erase(args.get_integer(0));
    }
    return 0;
}

int lua_unload_class(LuaArgs& args, GlueHost& host)
{
    for (int index = 0; index < args.count(); ++index) {
        const std::int32_t slot = host.vehicle_class_slot(args.get_integer(index));
        if (host.vehicle_class_cache_size() <= slot) {
            host.vehicle_class_resize_cache(slot + 1);
        }
        host.vehicle_class_release_cached(slot);
        host.log_text(format_text("Unloading vehicle class %d", args.get_integer(index)));
    }
    return 0;
}

float camera_state_degrees(float radians) noexcept
{
    const float shifted = narrow(static_cast<double>(radians) + 131.9468994140625); // 008BF874
    const float wrapped = narrow(std::fmod(static_cast<double>(shifted), 6.2831854820251465));
    return narrow((static_cast<double>(wrapped) / 3.1415927410125732) * 180.0); // 008BF8A3
}

int lua_get_camera_state(CameraStateResults& results, CameraStateHost& host)
{
    CameraStateResult state;
    state.position = host.game_camera_vector_30(); // LEA EDX,[ESI+30h] into 0088BA30
    const Vec3 angles = host.game_camera_basis_angles();
    state.rotation.x = camera_state_degrees(angles.x);
    state.rotation.y = camera_state_degrees(angles.y);
    state.rotation.z = camera_state_degrees(angles.z);
    state.zoom = host.base_fov_00f889b4() / host.game_camera_fov_1c4(); // 008BF980..008BF996
    results.push_camera_state(state);
    return 1;
}

float dist_from_crosshair(float a, float b, float fov) noexcept
{
    const float sum = narrow(static_cast<double>(a) * a + static_cast<double>(b) * b);
    float distance = 0.0f;
    if (static_cast<double>(sum) > 1e-10) { // FCOMI with qword 00CE3820, JBE skips sqrt
        distance = narrow(std::sqrt(static_cast<double>(sum)));
    }
    return narrow(static_cast<double>(distance) / fov); // 008C07B4
}

int lua_get_dist_from_crosshair(LuaArgs& args, LuaResults& results, CameraStateHost& host)
{
    void* entity = args.entity_at(0);
    const Vec3 camera = host.game_camera_world_position();
    const Vec3 position = host.entity_world_position(entity);
    Vec3 offset;
    offset.x = position.x - camera.x; // 008C06D2
    offset.y = position.y - camera.y;
    offset.z = position.z - camera.z;
    const std::array<float, 2> delta =
        host.camera_direction_angle_delta(host.game_camera_direction_110(), offset);
    results.push_float(dist_from_crosshair(delta[0], delta[1], host.game_camera_fov_1c4()));
    return 1;
}
// ---------------------------------------------------------------------------
// Movie-camera modes
// ---------------------------------------------------------------------------

MovCamTail read_movcam_tail(LuaArgs& args, int first)
{
    MovCamTail tail;
    if (args.count() > first && args.is_boolean(first)) {
        tail.flag = args.get_boolean(first);
    }
    if (args.count() > first + 1 && args.is_number(first + 1)) {
        tail.number = narrow(args.get_number(first + 1));
    }
    if (args.count() > first + 2 && args.is_string(first + 2)) {
        tail.name = args.get_string(first + 2);
    }
    if (args.count() > first + 3) {
        tail.extras_from = first + 3;
    }
    return tail;
}

namespace {

void* entity_or_null(LuaArgs& args, int index)
{
    return args.is_entity_table(index) ? args.entity_at(index) : nullptr; // 008889C0 first
}

} // namespace

int lua_movcam_hand_mode_targeted(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = entity_or_null(args, 0);
    bool a = true;
    if (args.count() >= 2 && args.is_boolean(1)) {
        a = args.get_boolean(1);
    }
    bool b = true;
    if (args.count() > 2 && args.is_boolean(2)) {
        b = args.get_boolean(2);
    }
    host.movcam_ensure_target();
    host.set_hand_mode_targeted(entity, a, b);
    if (host.game_session_mode_1fe4() == 1) {
        host.broadcast_hand_mode_targeted(entity, a, b);
    }
    return 0;
}

int lua_movcam_hand_mode_deck(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = args.entity_at(0);
    int camera_index = 0;
    if (args.is_number(1)) {
        camera_index = args.get_integer(1) - 1;
    } else if (args.is_string(1)) {
        camera_index = host.vehicle_camera_index_of_name(entity, args.get_string(1));
    }
    host.movcam_ensure_target();
    host.set_hand_mode_deck(entity, camera_index);
    return 0;
}

int lua_movcam_hand_mode_overview(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = args.entity_at(0); // no entity test in this one
    const MovCamTail tail = read_movcam_tail(args, 1);
    host.movcam_ensure_target();
    host.set_hand_mode_overview(entity, tail, args);
    return 0;
}

int lua_movcam_ref_pos(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = entity_or_null(args, 0);
    const Vec3 position = args.read_vector3(1);
    const MovCamTail tail = read_movcam_tail(args, 2);
    host.movcam_ensure_target();
    host.set_ref_pos(entity, position, tail, args);
    return 0;
}

int lua_movcam_ref_pos_polar(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = entity_or_null(args, 0);
    const float a = narrow(args.get_number(1));
    const float b = narrow(args.get_number(2));
    const float c = narrow(args.get_number(3));
    const MovCamTail tail = read_movcam_tail(args, 4);
    host.movcam_ensure_target();
    host.set_ref_pos_polar(entity, a, b, c, tail, args);
    if (host.game_session_mode_1fe4() == 1) {
        host.broadcast_ref_pos_polar(entity, a, b, c, tail.flag, tail.number);
    }
    return 0;
}

int lua_movcam_ref_pos_radius(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = entity_or_null(args, 0);
    const float radius = narrow(args.get_number(1));
    const MovCamTail tail = read_movcam_tail(args, 2);
    host.movcam_ensure_target();
    host.set_ref_pos_radius(entity, radius, tail, args);
    if (host.game_session_mode_1fe4() == 1) {
        host.broadcast_ref_pos_radius(entity, radius, tail.flag, tail.number);
    }
    return 0;
}

int lua_movcam_path_walk(LuaArgs& args, MovCamModeHost& host)
{
    void* path = host.entity_path_interface(args.entity_at(0));
    int start = args.get_integer(1) - 1;
    if (start == -1) {
        start = 0;
    }
    int end = args.get_integer(2) - 1;
    if (end == -1) {
        end = host.path_point_count(path) - 1; // 0 means "to the last point"
    }
    const float speed = narrow(args.get_number(3));
    const MovCamTail tail = read_movcam_tail(args, 4);
    host.movcam_ensure_target();
    host.set_path_walk(path, start, end, speed, tail, args);
    return 0;
}

int lua_movcam_viktor_mode(LuaArgs& args, MovCamModeHost& host)
{
    void* first = args.entity_at(0);
    void* second = args.entity_at(1);
    const float a = narrow(args.get_number(2));
    const float b = narrow(args.get_number(3));
    const MovCamTail tail = read_movcam_tail(args, 4);
    host.movcam_ensure_target();
    host.set_viktor_mode(first, second, a, b, tail, args);
    return 0;
}

int lua_movcam_tompi_mode_weak(LuaArgs& args, MovCamModeHost& host)
{
    void* first = args.entity_at(0);
    const Vec3 a = args.read_vector3(1);
    void* second = args.entity_at(2);
    const Vec3 b = args.read_vector3(3);
    const MovCamTail tail = read_movcam_tail(args, 4);
    host.movcam_ensure_target();
    host.set_tompi_mode_weak(first, a, second, b, tail, args);
    return 0;
}

int lua_movcam_tompi_mode(LuaArgs& args, MovCamModeHost& host)
{
    void* first = args.is_nil(0) ? nullptr : args.entity_at(0); // nil test, not entity test
    const Vec3 a = args.read_vector3(1);
    void* second = args.is_nil(2) ? nullptr : args.entity_at(2);
    const Vec3 b = args.read_vector3(3);
    host.movcam_ensure_target();
    host.set_tompi_mode(first, a, second, b);
    return 0;
}

int lua_movcam_set_secondary_target(LuaArgs& args, MovCamModeHost& host)
{
    void* entity = entity_or_null(args, 0);
    const MovCamTail tail = read_movcam_tail(args, 1);
    host.movcam_ensure_target();
    host.set_secondary_target(entity, tail, args);
    return 0;
}

int lua_movcam_add_interpolator(LuaArgs& args, MovCamModeHost& host)
{
    void* interpolator = nullptr;
    if (host.interpolator_allocation_succeeds()) {
        const std::string name = args.get_string(0);
        // EDI/EBP/EBX/[ESP+24h] hold arguments 4/3/2/1 and are read in that order.
        const float a4 = narrow(args.get_number(4));
        const float a3 = narrow(args.get_number(3));
        const float a2 = narrow(args.get_number(2));
        const float a1 = narrow(args.get_number(1));
        interpolator = host.interpolator_create(name, a1, a2, a3, a4);
    }
    host.movcam_ensure_target();
    host.add_interpolator(interpolator);
    if (host.game_session_mode_1fe4() == 1) {
        host.broadcast_add_interpolator(interpolator);
    }
    return 0;
}

int lua_movcam_pick_snitt_deck_position(LuaArgs& args, MovCamModeHost& host)
{
    const std::string name = args.get_string(0);
    void* entity = args.entity_at(1);
    void* other = entity_or_null(args, 2);
    host.movcam_ensure_target();
    host.pick_deck_position(name, entity, other);
    return 0;
}
int lua_movcam_set_pos_rot_zoom(LuaArgs& args, MovCamPoseHost& host)
{
    const Vec3 position = args.read_vector3(0);
    const Vec3 rotation = args.read_vector3(1);
    const float zoom = narrow(args.get_number(2));
    host.movcam_ensure_target();
    host.movcam_set_pos_rot(position, rotation); // 008BFCCE
    host.movcam_ensure_target();
    host.movcam_set_zoom(zoom);
    return 0;
}
// ---------------------------------------------------------------------------
// Clouds
// ---------------------------------------------------------------------------

int lua_set_old_cloud_alpha(LuaArgs& args, CloudSceneHost& host)
{
    const float alpha = narrow(args.get_number(0));
    for (void* node = host.old_cloud_root_first(); node != nullptr;
         node = host.node_next_sibling_3c(node)) {
        if (host.node_is_cloud_instance(node)) {
            host.cloud_instance_set_alpha(node, alpha);
        }
        for (void* child = host.node_first_child_34(node); child != nullptr;
             child = host.node_next_sibling_3c(child)) {
            if (host.node_is_cloud_instance(child)) {
                host.cloud_instance_set_alpha(child, alpha);
            }
            for (void* grandchild = host.node_first_child_34(child); grandchild != nullptr;
                 grandchild = host.node_next_sibling_3c(grandchild)) {
                host.grandchild_set_alpha_00888fc0(grandchild, alpha);
            }
        }
    }
    return 0;
}

bool point_in_closed_box(const Vec3& point, const Vec3& lo, const Vec3& hi) noexcept
{
    return lo.x <= point.x && lo.y <= point.y && lo.z <= point.z && point.x <= hi.x
        && point.y <= hi.y && point.z <= hi.z;
}

int lua_set_cloud_visibility(LuaArgs& args, CloudSceneHost& host)
{
    const Vec3 lo = args.read_vector3(0);
    const Vec3 hi = args.read_vector3(1);
    const float value = narrow(args.get_number(2));
    for (void* node = host.visibility_list_first(); node != nullptr;
         node = host.visibility_list_next_38(node)) {
        host.node_ensure_world_matrix(node);
        if (host.node_vcall_5c(node, kCloudVisibilityNodeSelector)
            && point_in_closed_box(host.node_world_position(node), lo, hi)) {
            host.node_set_visibility_1c4(node, value);
        }
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Look-at and GUI name
// ---------------------------------------------------------------------------

int lua_game_camera_look_at_target(LuaArgs& args, LookAtHost& host)
{
    bool use_entity = false;
    void* entity = nullptr;
    Vec3 position;
    if (args.is_entity_table(0)) {
        use_entity = true;
        entity = args.entity_at(0);
    } else if (args.is_vector3_table(0)) {
        position = args.read_vector3(0);
    }
    float a1 = kLookAtDefaultFirst;
    float a2 = kLookAtDefaultSecond;
    float a3 = 0.0f;
    float a4 = 0.0f;
    if (args.count() > 1) {
        a1 = narrow(args.get_number(1));
    }
    if (args.count() > 2) {
        a2 = narrow(args.get_number(2));
    }
    if (args.count() > 3) {
        a3 = narrow(args.get_number(3));
    }
    if (args.count() > 4) {
        a4 = narrow(args.get_number(4));
    }
    if (!host.camera_controller_present()) {
        return 0;
    }
    if (use_entity) {
        host.controller_look_at_entity(entity, a1, a2, a3, a4);    // 00892309
    } else {
        host.controller_look_at_position(position, a1, a2, a3, a4); // 0089235A
    }
    return 0;
}

int lua_set_gui_name(LuaArgs& args, LookAtHost& host)
{
    const std::string name = args.get_string(1);
    void* entity = args.entity_at(0);
    const bool flag = args.count() > 2 ? args.get_boolean(2) : false;
    host.entity_set_gui_name(entity, name, flag);
    return 0;
}
// ---------------------------------------------------------------------------
// Ambient, animation and terrain nodes
// ---------------------------------------------------------------------------

int lua_enable_engine_sound(LuaArgs& args, AmbientHost& host)
{
    void* entity = args.entity_at(0);
    host.entity_set_engine_sound_9c4(entity, args.get_boolean(1));
    return 0;
}

int lua_start_vehicle_anim(LuaArgs& args, AmbientHost& host)
{
    void* entity = args.entity_at(0);
    std::string name;
    if (args.count() > 1 && args.is_string(1)) {
        name = args.get_string(1);
    }
    const float value = args.count() < 3 ? 0.0f : narrow(args.get_number(2));
    host.entity_start_anim_0088b4b0(entity, name, value);
    return 0;
}

int lua_stop_vehicle_anim(LuaArgs& args, AmbientHost& host)
{
    host.entity_stop_anim_70c(args.entity_at(0));
    return 0;
}

int lua_get_visibility_terrain_node(LuaArgs& args, LuaResults& results, AmbientHost& host)
{
    void* node = host.scene_find_node(args.get_string(0));
    results.push_boolean(node != nullptr && host.scene_node_visibility_ac(node) > 0.0f);
    return 1;
}

int lua_insert_fish_school(LuaArgs& args, AmbientHost& host)
{
    const int type = args.get_integer(0);
    host.fish_school_create(type, args.read_vector3(1));
    return 0;
}

int lua_temp_add_floatsam(LuaArgs& args, AmbientHost& host)
{
    host.temp_add_floatsam_004a5f30(args, 0);
    return 0;
}

int lua_temp_add_anim(LuaArgs& args, AmbientHost& host)
{
    host.temp_add_anim_0049dc00(args, 0);
    return 0;
}

int lua_remove_temp_added_stuff(LuaArgs& args, AmbientHost& host)
{
    host.remove_temp_added_0049de90(args.get_string(0));
    return 0;
}
int lua_set_tip_effect(LuaArgs& args, EngineEffectHost& host)
{
    void* entity = args.entity_at(0);
    for (int i = 0; i < host.squadron_member_count_3cc(entity); ++i) {
        void* plane = i < kSquadronMemberSlots ? host.squadron_member_3d0(entity, i) : nullptr;
        const bool on = args.get_boolean(1);
        host.plane_update_engine_effects(plane, on ? 1.0f : 0.0f); // CVTSI2SS of the byte
    }
    return 0;
}
int lua_add_animated_part_effect(LuaArgs& args, PartEffectHost& host)
{
    void* object = host.lua_object_00888960(args, 0);
    if (host.object_vcall_20(object)) {
        const std::string name = args.get_string(2);
        host.unit_part_effect_start_by_index(object, args.get_integer(1), name);
    }
    return 0;
}
int lua_effect(LuaArgs& args, EffectPointerResults& results, EffectSpawnHost& host)
{
    void* effect = nullptr;
    if (args.count() > 2 && args.is_entity_table(2)) {
        void* entity = args.entity_at(2);
        const std::string name = args.get_string(0);
        effect = host.effect_create_on_entity(name, entity, args.read_vector3(1));
    } else {
        const std::string name = args.get_string(0);
        effect = host.effect_create_at_position(name, args.read_vector3(1));
    }
    if (effect == nullptr) {
        return 0;
    }
    host.effect_set_byte9(effect);
    if (args.count() > 3) {
        float remaining = narrow(args.get_number(3));
        while (remaining > 0.0f) {
            const float step = kEffectPrerunStep < remaining ? kEffectPrerunStep : remaining;
            host.effect_update_children(effect, step);
            remaining = narrow(static_cast<double>(remaining) - kEffectPrerunStep);
        }
    }
    results.push_pointer_table(effect);
    return 1;
}
int lua_set_visibility(LuaArgs& args, VisibilityHost& host)
{
    const bool visible = args.get_boolean(1);
    void* object = host.lua_table_entity_ptr(args, 0);
    if (host.object_vcall_5c(object, kObjectKindSquadron)) {
        for (int i = 0; i < host.squadron_member_count_3cc(object); ++i) {
            void* plane = i < kSquadronMemberSlots ? host.squadron_member_3d0(object, i) : nullptr;
            host.plane_set_visible_00951f40(plane, visible); // 008A154D
        }
    } else if (host.object_vcall_5c(object, kObjectKindPlane)) {
        host.plane_set_visible_00951f40(object, visible); // 008A1578
    }
    return 0;
}

int lua_fake_ping(LuaArgs& args, MapPingHost& host)
{
    const int type = args.get_integer(0); // ESI
    host.map_ping_add_local(type, args.read_vector3(1));
    return 0;
}
// ---------------------------------------------------------------------------
// Read-only queries
// ---------------------------------------------------------------------------

int lua_is_landscape(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    float height = 0.0f;
    if (host.world_ground_height_at(args.read_vector3(0), height) && height > 0.0f) {
        results.push_float(height);
    } else {
        results.push_nil();
    }
    return 1;
}

int lua_is_ship(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    const Vec3 point = args.read_vector3(0);
    Vec3 top = point;
    top.y = kIsShipProbeHeight;
    Vec3 bottom = point;
    bottom.y = 0.0f; // XORPS
    bool kind6 = false;
    float value = 0.0f;
    if (host.spatial_query_segment(top, bottom, kind6, value) && kind6) {
        results.push_float(value);
    } else {
        results.push_nil();
    }
    return 1;
}

int lua_do_vector3(LuaArgs& args, LuaResults& results)
{
    Vec3 v;
    v.x = narrow(args.get_number(0));
    v.y = narrow(args.get_number(1));
    v.z = narrow(args.get_number(2));
    results.push_vector3(v);
    return 1;
}

int lua_get_entity_speed(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    results.push_float(host.entity_vcall_38_speed(args.entity_at(0)));
    return 1;
}

int lua_get_direction(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    results.push_vector3(host.entity_world_row_ec(args.entity_at(0)));
    return 1;
}

int lua_relative_position(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    void* entity = args.entity_at(0);
    const Vec3 point = args.read_vector3(1);
    results.push_vector3(host.entity_local_to_world(entity, point));
    return 1;
}

int lua_get_bounding(LuaArgs& args, LuaResults& results, QueryHost& host)
{
    Vec3 lo;
    Vec3 hi;
    host.entity_bounding_box(args.entity_at(0), lo, hi);
    results.push_vector3(lo);
    results.push_vector3(hi);
    return 2;
}
// ---------------------------------------------------------------------------
// Variant-argument bindings (cc13-sys5b)
// ---------------------------------------------------------------------------

int lua_mission_narrative_post(std::uint32_t entry_point, LuaArgs& args, NarrativeHost& host)
{
    const std::string text = args.get_string(0); // variant 0, +8h, passed as the pointer
    std::string callback;
    if (args.count() > 1) {
        callback = args.get_string(1); // variant 1's string copied into a local
    }
    const int extras_from = args.count() > 2 ? 2 : -1; // CMP [count],3 / JL skips the vector
    host.narrative_post(entry_point, text, callback, args, extras_from);
    return 0;
}

int lua_message(LuaArgs& args, NarrativeHost& host)
{
    void* entity = args.entity_at(0);
    host.entity_publish_lua_message(entity, args, 1);
    return 0;
}
int lua_debug_trap(LuaArgs& args, DebugTrapHost& host)
{
    const std::string traceback = host.call_named_results_text(kDebugTrapFunction); // 008C843E
    if (args.count() != 0) {
        host.log_format_string(args.get_string(0)); // 008C84B4, the script text is the format
    }
    host.log_format_string(traceback);
    return 0;
}
int lua_override_message(LuaArgs& args, OverrideMessageHost& host)
{
    std::vector<std::string> lines;
    if (args.count() > 0) {
        if (args.is_string(0)) {
            lines.push_back(args.get_string(0));
        } else {
            lines = host.table_entry_strings(args, 0); // no table test: any non-string iterates
        }
    }
    void* first = args.count() > 1 ? args.entity_at(1) : nullptr;
    void* second = args.count() > 2 ? args.entity_at(2) : nullptr;
    host.warning_report_string_list(lines, first, second);
    return 0;
}
HackInputPress make_hack_input_press(float duration, const std::string& name, float delay,
    float clock) noexcept
{
    HackInputPress press;
    press.duration_00 = duration;           // 008C89DF
    press.start_time_04 = clock + delay;    // 008C89E3 FLD 00F876A4, FADD
    press.name_08 = name;
    press.flag_10 = false;                  // byte +10h = 0
    return press;
}

void hack_input_queue_press(int input, float duration, const std::string& name, float delay,
    HackInputHost& host)
{
    host.queue_press(input, make_hack_input_press(duration, name, delay,
        host.clock_seconds_00f876a4()));
}

int lua_hack_press_input(LuaArgs& args, HackInputHost& host)
{
    const int input = args.get_integer(0);
    float duration = 1.0f;
    if (args.count() > 1) {
        duration = narrow(args.get_number(1));
    }
    std::string name;
    if (args.count() > 2) {
        name = args.get_string(2);
    }
    float delay = 0.0f;
    if (args.count() > 3) {
        delay = narrow(args.get_number(3));
    }
    hack_input_queue_press(input, duration, name, delay, host);
    return 0;
}
// ---------------------------------------------------------------------------
// Small accessors and value types
// ---------------------------------------------------------------------------

bool scene_token_is_int(const std::string& token)
{
    // sscanf("%d") assigns a field when, after optional white space and one sign, a digit
    // follows; overflow still counts as an assignment.
    std::size_t i = 0;
    while (i < token.size() && std::isspace(static_cast<unsigned char>(token[i]))) {
        ++i;
    }
    if (i < token.size() && (token[i] == '+' || token[i] == '-')) {
        ++i;
    }
    return i < token.size() && std::isdigit(static_cast<unsigned char>(token[i])) != 0;
}

namespace {

bool equal_ci(const std::string& a, const std::string& b) noexcept
{
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

} // namespace

float scene_convert_by_unit_token(float value, const std::string& unit,
    const std::vector<UnitConversionRow>& rows, UnitTokenizerHost& host)
{
    host.peek_token();
    if (host.end_flag_80a() || host.current_token_empty_and_byte4_clear()) {
        return value;
    }
    if (equal_ci(host.peek_token(), unit)) {
        host.consume_token();
        return value;
    }
    for (const UnitConversionRow& row : rows) {
        if (equal_ci(host.peek_token(), row.source) && equal_ci(unit, row.target)) {
            host.retain_current_token_405();
            return row.reciprocal ? narrow(static_cast<double>(row.factor) / value)
                                  : narrow(static_cast<double>(row.factor) * value);
        }
        if (equal_ci(host.peek_token(), row.target) && equal_ci(unit, row.source)) {
            host.consume_token();
            // x87 keeps 1 / value in extended precision before the second divide; double here.
            return row.reciprocal ? narrow((1.0 / value) / row.factor)
                                  : narrow(static_cast<double>(value) / row.factor);
        }
    }
    return value;
}
std::int32_t objective_entry_field_28(const std::uint8_t* entry) noexcept
{
    std::int32_t value = 0;
    std::memcpy(&value, entry + 0x28, sizeof value);
    return value;
}

bool world_has_entity_set0(const std::uint8_t* world) noexcept
{
    std::uint32_t value = 0;
    std::memcpy(&value, world + 0x21A4, sizeof value);
    return value != 0;
}

void nav_point_sentity_init(void* nav_point, NavPointInitHost& host)
{
    host.entity_inherit_race_and_activate(nav_point); // 008DA103
    host.entity_set_dword_58(nav_point, 0);           // 008DA108
}
ObjectiveSessionMessageHeader objective_session_message_header() noexcept
{
    ObjectiveSessionMessageHeader header;
    header.vtable = kObjectiveSessionMessageVtable;
    header.one_04 = 1;
    return header;
}

std::optional<std::string> optional_string_from(const std::string& source)
{
    return std::optional<std::string>(source);
}

void lua_object_get_optional_string(LuaArgs& args, int index, std::optional<std::string>& out)
{
    if (args.is_nil(index)) {
        return;
    }
    out = args.get_string(index);
}
// ---------------------------------------------------------------------------
// Non-binding helpers
// ---------------------------------------------------------------------------

Rgba static_color_black() noexcept
{
    return Rgba{0.0f, 0.0f, 0.0f, 1.0f};
}

Rgba static_color_red() noexcept
{
    return Rgba{1.0f, 0.0f, 0.0f, 1.0f};
}

Rgba color_blend_from_black_by_index(int index) noexcept
{
    const float kHalf = 0.5f; // 00CE3800
    Rgba other;
    switch (index) {
    case 0: other = Rgba{0.0f, 0.0f, 1.0f, 1.0f}; break; // blue, 00F88C00
    case 1: other = Rgba{0.0f, 1.0f, 0.0f, 1.0f}; break; // green, 00F88BEC
    case 2: other = static_color_red(); break;
    default: return static_color_black();
    }
    const Rgba black = static_color_black();
    Rgba out;
    for (std::size_t i = 0; i < 4; ++i) {
        out[i] = black[i] * kHalf + other[i] * kHalf; // two 00BEE140 scales, then FADD
    }
    return out;
}

bool objective_entry_is_revealed(const ObjectiveRevealView& view) noexcept
{
    if (!view.has_entity) {
        return true;
    }
    if (view.local_side < 0 || view.local_side >= 8) {
        return false;
    }
    const std::int32_t value = view.row_selector_byte ? view.row_value_plus8 : view.row_value_plus4;
    return value > 1;
}

Vec3 objective_entry_position(const ObjectiveEntryView& entry, ObjectiveEntityHost& host)
{
    if (entry.entity != nullptr) {
        return host.entity_vcall_c4_position(entry.entity);
    }
    return entry.stored_position;
}

bool objective_session_message_accepts(std::uint32_t type, std::uint8_t own_type) noexcept
{
    return type == kObjectiveSessionMessageAnyType || type == own_type;
}

std::uint32_t objective_entry_select_entity_or_pos_key(PropertyKeySink& sink)
{
    if (sink.vcall_14()) {
        sink.vcall_10(0, "entity");
        return 0;
    }
    sink.vcall_10(0, "pos");
    return 0;
}

void map_ping_entry_describe_properties(const MapPingEntryView& entry, PropertyWriter& sink)
{
    sink.set_string("ID", entry.id_08);        // 00CE59B4
    sink.set_string("text", entry.text_10);    // 00CF1010
    sink.set_bool("quiet", entry.quiet_14);    // 00D16048, value tag 3
    sink.set_int("type", entry.type_18);       // 00CE4AD0, value tag 1
    sink.set_int("state", entry.state_1c);     // 00CE7324
    sink.open_name("positions");               // 00D1603C, vtable+4
    int index = 0;
    for (const MapPingPosition& position : entry.positions_20) {
        sink.open_index(index);                // key tag 1
        if (position.entity != nullptr) {
            sink.set_entity("entity", position.entity); // 00CE5818, value tag 4
        } else {
            sink.set_vector("pos", position.pos);       // 00CF8FA4, value tag 5
        }
        sink.close();                          // vtable+8
        ++index;
    }
    sink.close();
}
void* gui_find_widget_by_dotted_path(const std::string& path, GuiTreeHost& host)
{
    // `split` stands for the strcspn test: an empty rest or no '.' gives npos.
    auto split = [](const std::string& rest) {
        const std::size_t pos = rest.find('.');
        return rest.empty() ? std::string::npos : pos;
    };
    std::string rest = path + ".";
    std::size_t pos = split(rest);
    void* widget = host.find_page(rest.substr(0, pos));
    if (widget == nullptr) {
        return nullptr;
    }
    rest = pos == std::string::npos ? std::string() : rest.substr(pos + 1);
    while (!rest.empty()) {
        pos = split(rest);
        widget = host.find_child(widget, rest.substr(0, pos));
        if (widget == nullptr) {
            return nullptr;
        }
        rest = pos == std::string::npos ? std::string() : rest.substr(pos + 1);
    }
    return widget;
}

void* lua_gui_find_widget_from_string_arg(LuaArgs& args, int index, GuiTreeHost& host)
{
    return gui_find_widget_by_dotted_path(args.get_string(index), host);
}

std::vector<std::string> collect_language_lanfiles(const std::vector<LanguageEntry>& table)
{
    std::vector<std::string> names;
    names.reserve(table.size());
    for (const LanguageEntry& entry : table) {
        names.push_back(entry.lanfile);
    }
    return names;
}
} // namespace bsp::cc13_seg56
