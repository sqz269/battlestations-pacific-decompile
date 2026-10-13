#pragma once

// Packet cc13-sys5: segment 56 (0088B410..008DCD40, without 00880000..0088FFFF) code that
// earlier passes named but did not reconstruct. Evidence: docs/CC13_SYS_5.md.
//
// Most of segment 56 is the mission Lua binding table 00E0B7B8 (config/lua_bindings.json).
// This file models the presentation, tooling, input, GUI, localisation and environment
// bindings; bindings that change gameplay state are left to the cc9 lanes (the doc lists
// them). Every binding is `__fastcall(lua_State* in ECX)`, returns its result count in EAX,
// and runs the machine prologue/epilogue include/bsp/lua_binding_core.hpp describes
// (ConstructBorrowed 00B66C00, OpenCallFrame 00B679B0, ArgumentAt 00B677E0, ResultCount
// 00B66400, Close 00B669A0). The functions here model only what happens between them.
//
// Every name is a descriptive hypothesis, not a recovered symbol. Nothing here is ABI
// compatible: the host takes field values and callbacks where native code works on raw
// objects, and host methods are named by the callee they stand for. Nothing is
// game-validated.

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "bsp/locale_tables.hpp"
#include "bsp/lua_binding_core.hpp"

namespace bsp::cc13_seg56 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// The argument accessors these bindings use beyond the core reader.
struct LuaArgs : LuaBindingArgumentReader {
    virtual bool is_boolean(int index) = 0;   // 00B66000
    virtual bool is_number(int index) = 0;    // 00B66050
    virtual Vec3 read_vector3(int index) = 0; // 00888760, reads fields x/y/z of a table
    virtual void* to_userdata(int index) = 0; // 00B662D0
    // 00B662F0 over the argument object: the boolean, or `fallback` when it is not one.
    virtual bool boolean_or(int index, bool fallback) = 0;
    // 00B67800 + 00B67720 + 00B66270: `arg[index][field][element]` as a number.
    virtual double table_field_element(int index, const char* field, int element) = 0;
    // 00B67800 + 00B66330: `arg[index][field]` as a float, `fallback` when absent.
    virtual double table_field_number_or(int index, const char* field, double fallback) = 0;
};

// What the bindings push back beyond the core writer.
struct LuaResults : LuaBindingResultWriter {
    virtual void push_float(float value) = 0;               // 00B66480
    virtual void push_string(const std::string& value) = 0; // 00B66710 / 00B672B0
    // 00B67930 NewTable, then one 00B675B0-style SetNativeStringInteger per field, in order.
    virtual void push_integer_fields(const std::vector<std::pair<std::string, int>>& fields) = 0;
    // 00B67930 NewTable + 0088BA30 SetVector3Fields: {x=, y=, z=}.
    virtual void push_vector3(const Vec3& value) = 0;
    // NewTable, then per field SetNewTable(name), GetByName(name), SetVector3Fields.
    virtual void push_vector3_fields(const std::vector<std::pair<std::string, Vec3>>& fields) = 0;
};

// ---------------------------------------------------------------------------
// Debug and tooling bindings
// ---------------------------------------------------------------------------

// The module globals four tooling bindings write. Storage belongs to the caller.
struct DebugSwitchBytes {
    // 00E0B6CE, include/bsp/fixed_step_job_waves.hpp kFrameJobPoolRunPassByte.
    std::uint8_t* job_pool_run_pass_00e0b6ce{};
    std::int32_t* ship_debug_filter_00e0e300{};
    std::int32_t* avoid_zone_debug_layer_00e08040{};
    std::uint8_t* terminate_execution_00e1ae75{};
};

// SetSingleThread 008A1890: byte 00E0B6CE = !arg0 (SETE at 008A1999). Single-threaded means
// the frame job pool's run pass is off.
int lua_set_single_thread(LuaArgs& args, DebugSwitchBytes& g);
// SetShipDebugFilter 008A15D0: dword 00E0E300 = int arg0 (008A16D8).
int lua_set_ship_debug_filter(LuaArgs& args, DebugSwitchBytes& g);
// AvoidZoneDebugLayer 008A1730: dword 00E08040 = int arg0 (008A1838).
int lua_avoid_zone_debug_layer(LuaArgs& args, DebugSwitchBytes& g);
// TerminateExecution 008C8250: byte 00E1AE75 = 1 (008C833A, BL = 1). No arguments read.
int lua_terminate_execution(DebugSwitchBytes& g);

// The four `T*` bindings and Loading_Progress open the frame and close it: no argument is
// read and nothing is called. TompiAction 008C5360, TSetCamMode 008C5490, TAddCamPos 008C55C0,
// TAddLookatPos 008C56F0, Loading_Progress 008C76C0.
int lua_empty_binding() noexcept;

struct ToolingHost {
    virtual ~ToolingHost() = default;
    // 004254B0, the variadic log sink. The bindings pass a format and its arguments; the host
    // receives the formatted text.
    virtual void log_text(const std::string& text) = 0;
    virtual std::int32_t game_tick_648() = 0;      // [[00E188A8]+648h]
    virtual float clock_seconds_00f876a4() = 0;    // float 00F876A4
    // [0109CEFC] vtable+4, answering an int the binding scales to MiB.
    virtual std::int32_t memory_manager_free_bytes() = 0;
    virtual std::string user_geography_region() = 0; // 00439100
    virtual std::int64_t query_performance_counter() = 0;   // [00CE2270]
    virtual std::int64_t query_performance_frequency() = 0; // [00CE2278]
    virtual void output_debug_string(const std::string& text) = 0; // [00CE2274]
    // 00BE0980 over [0109CEEC]: enter a file block (mpak) by name, last argument 1.
    virtual void vfs_enter_file_block(const std::string& name) = 0;
};

// dprintf 008C7E10: log "[tick: %d, time: %.3f] %s" (00D1532C) with game+648h, the float
// clock 00F876A4 widened to double, and string arg0.
int lua_dprintf(LuaArgs& args, ToolingHost& host);
// MemoryStatus 008C7BE0: log "Free memory @%s %.1f\n" (00D152E0) with string arg0 and the
// manager's free-byte count times 1/1024 twice (00CEDAE8, FMUL/FMULP at 008C7CA8).
int lua_memory_status(LuaArgs& args, ToolingHost& host);
// Region 008C7D20: push the user geography region string (00439100) through 00B66710.
int lua_region(LuaResults& results, ToolingHost& host);

// StartMeasure / StopMeasure keep a start sample in four module globals.
struct MeasureStart {
    std::int64_t counter_00f887e0{}; // QPC at start, low/high at 00F887E0/E4
    float clock_00f887e8{};          // float clock at start; never read back by StopMeasure
    std::int32_t tick_00f887ec{};    // game tick at start
};
// StartMeasure 008C7950.
int lua_start_measure(MeasureStart& state, ToolingHost& host);
// StopMeasure 008C7A70: sprintf "tickcount:%d realtime:%f ms fps:%f\n" (00D152A0) with the
// tick delta, seconds * 1000.0 (00CE47A0) and ticks / seconds, then OutputDebugStringA.
int lua_stop_measure(const MeasureStart& state, ToolingHost& host);
std::string stop_measure_text(std::int32_t ticks, std::int64_t counter_delta,
    std::int64_t frequency);

// Mpak 008C5820: enter the file block named by string arg0.
int lua_mpak(LuaArgs& args, ToolingHost& host);

// The perf-timing registry, singleton 00F87908 built by 008D1B80 under the singleton
// lifetime lock (operator new 1Ch, constructor 008D1A10, vtable 00D15860). Two trees:
// +04h a case-insensitive name -> nesting count map (005070C0 indexes it), +10h a
// name -> one-byte timer block map (008CFB20 subscripts it). The destructor 008D1AB0 erases
// both, clears 00F87908 and restores the singleton-base vtable 00CE3818.
class PerfTimingRegistry {
public:
    struct CaseInsensitiveLess {
        bool operator()(const std::string& a, const std::string& b) const noexcept;
    };
    // PerfTimingStart 008D1C50: a zero count allocates the block (operator new(1)), then the
    // count is incremented.
    void start(const std::string& name);
    // PerfTimingEnd 008D1DD0: the count is decremented; on reaching zero the block is freed and
    // the slot nulled. An unknown name inserts a zero count and leaves it at -1.
    void end(const std::string& name);
    int count(const std::string& name) const;
    bool has_block(const std::string& name) const;

private:
    std::map<std::string, int, CaseInsensitiveLess> counts_;
    // The comparator of the 008CFB20 tree was not read; it only orders iteration.
    std::map<std::string, std::unique_ptr<std::uint8_t>> blocks_;
};
int lua_perf_timing_start(LuaArgs& args, PerfTimingRegistry& registry);
int lua_perf_timing_end(LuaArgs& args, PerfTimingRegistry& registry);

// ---------------------------------------------------------------------------
// Camera bindings
// ---------------------------------------------------------------------------

struct CameraHost {
    virtual ~CameraHost() = default;
    // 00452A50 __thiscall on 00E081A0: depth of field near/far.
    virtual void depth_of_field_set_range(float near_value, float far_value) = 0;
    // The free camera *( *(00E198C4+90h) + 24h ); null skips the whole write.
    virtual bool free_camera_present() = 0;
    virtual void free_camera_set_position(const Vec3& position) = 0; // +A4h/+A8h/+ACh
    // 00467050 __thiscall on free camera +74h: (first, second, 0.0f).
    virtual void free_camera_set_rotation_from_euler(float first, float second, float third) = 0;
    // Movie camera at [[00E198C4]+98h].
    // 005CD160 __fastcall(movcam): activates the movie camera on the current unit when it is not
    // active and answers movcam+30h, the ECX of the setter that follows.
    virtual void movcam_ensure_target() = 0;
    virtual void movcam_set_zoom(float zoom) = 0;        // 0078E480 on that target
    virtual void movcam_set_speed_limits(float a, float b) = 0; // 005CB900
    virtual void movcam_reset_speed_limits() = 0;        // 005CB970
    virtual void movcam_set_accel_time(float t) = 0;     // 005CB940
    virtual void movcam_reset_accel_time() = 0;          // 005CB9C0
    virtual void movcam_set_deviation_factor(float f) = 0; // 005CAF90
    virtual void movcam_reset_deviation_factor() = 0;    // 005CAFB0
    virtual void movcam_set_overview_height(float h) = 0;  // 005CAFA0
    virtual void movcam_reset_overview_height() = 0;     // 005CAFC0
    virtual std::int32_t game_session_mode_1fe4() = 0;   // [[00E188A8]+1FE4h]
    // 0076D660 on game+1EF0h: session broadcast of the deviation factor.
    virtual void session_broadcast_movcam_deviation_factor(float f) = 0;
    // 005CD240 on [[00E198C4]+9Ch]: the HUD movie camera, or null.
    virtual void* hud_movie_camera() = 0;
    virtual void hud_movie_camera_set_fov(void* camera, float radians) = 0; // +410h
    // 007A44D0 __thiscall(camera, LuaObject copy of arg0).
    virtual void hud_movie_camera_add_positions(void* camera, LuaArgs& args, int index) = 0;
    // Shake object [[00E188A8]+1ED4h].
    virtual void camera_shake_accumulate(float amount) = 0; // 0042C3D0
    virtual void camera_set_shake_max(float v) = 0;         // 0042A940
    virtual void camera_set_shake_base(float v) = 0;        // 0042A970
    virtual void camera_set_shake_decay(float v) = 0;       // 0042A9A0
    // The game camera [[00E188A8]+19FCh]: refresh (00B6DB70) unless byte +5Ch bit 1 is set,
    // then the world position +120h..+128h and 0042D2E0's basis angles.
    virtual Vec3 game_camera_world_position() = 0;
    virtual Vec3 game_camera_basis_angles() = 0;
};

// Degrees to radians the way these bindings write it: (x * 3.1415927410125732 (00CE3D28,
// the double of float pi)) / 180.0 (00CE3D20), narrowed to float.
float degrees_to_radians_float(double degrees) noexcept;

// SetDOF 00897210: DOF range (number arg0, number arg1); arg1 is read first.
int lua_set_dof(LuaArgs& args, CameraHost& host);
// FreeCameraSetPos 008923B0: position (args 0..2) and euler (arg4, arg3, 0) in radians.
int lua_free_camera_set_pos(LuaArgs& args, CameraHost& host);
// PlayerSetBomberView 008A0850: floats 00E17678 = arg0, 00E17674 = arg1, 00E17670 = 1.0f.
struct BomberViewFloats {
    float* value_00e17670{};
    float* second_00e17674{};
    float* first_00e17678{};
};
int lua_player_set_bomber_view(LuaArgs& args, BomberViewFloats& g);
int lua_movcam_set_zoom(LuaArgs& args, CameraHost& host);              // 008B65C0
int lua_movcam_set_speed_limits(LuaArgs& args, CameraHost& host);      // 008B6D20
int lua_movcam_reset_speed_limits(CameraHost& host);                   // 008B6ED0
int lua_movcam_set_accel_time(LuaArgs& args, CameraHost& host);        // 008B7010
int lua_movcam_reset_accel_time(CameraHost& host);                     // 008B7180
int lua_movcam_set_deviation_factor(LuaArgs& args, CameraHost& host);  // 008B72C0
int lua_movcam_reset_deviation_factor(CameraHost& host);               // 008B7460
int lua_movcam_set_overview_height(LuaArgs& args, CameraHost& host);   // 008B75A0
int lua_movcam_reset_overview_height(CameraHost& host);                // 008B7710
int lua_movcam_new_add_positions(LuaArgs& args, CameraHost& host);     // 008B7850
int lua_movcam_new_add_position(LuaArgs& args, CameraHost& host);      // 008B79F0
int lua_movcam_new_set_fov(LuaArgs& args, CameraHost& host);           // 008B7BA0
int lua_camera_shake(LuaArgs& args, CameraHost& host);                 // 008B8040
int lua_camera_set_shake_max(LuaArgs& args, CameraHost& host);         // 008B81C0
int lua_camera_set_shake_base(LuaArgs& args, CameraHost& host);        // 008B8340
int lua_camera_set_shake_decay(LuaArgs& args, CameraHost& host);       // 008B84C0
int lua_get_camera_pos_rot(LuaResults& results, CameraHost& host);     // 008B7D30

// ---------------------------------------------------------------------------
// Map screen, sound, input and GUI bindings
// ---------------------------------------------------------------------------

struct InterfaceHost {
    virtual ~InterfaceHost() = default;
    // Map screen [[00E198C4]+54h].
    virtual float map_cursor_x_554() = 0;
    virtual float map_cursor_z_558() = 0;
    virtual std::int32_t map_zoom_2c() = 0;
    virtual void map_force_cursor_pos(const Vec3& position) = 0;     // 0059AD20
    virtual void map_release_cursor_0059ad50() = 0;                  // 0059AD50
    virtual void map_set_node_visible_by_name(const std::string& name, bool flag) = 0; // 005A4020
    virtual void map_set_unit_highlight(void* entity, bool on) = 0;  // 005AAAD0
    // Streamed dialog / music manager [00F8BBCC].
    virtual void music_set_channel_byte_4e(int channel, bool value) = 0; // 00A77840
    virtual void music_set_target_level(float level) = 0;               // 00A77860
    // Black bars on the engine-movie screen [+A0h] and the movie screen [+9Ch].
    virtual void engine_movie_screen_black_bars(bool show) = 0; // 005CB0F0 show / 005CB130 hide
    virtual void movie_screen_black_bars(bool show) = 0;        // 005CBC30 show / 005CBCC0 hide
    // Input manager singleton 004BEC00.
    virtual void input_set_context_level(int context, int level) = 0; // 00A933F0
    // Input record `index` (30h-byte rows at manager+4): byte +28h and float +24h.
    virtual bool input_record_enabled(int index) = 0;
    virtual float input_record_value(int index) = 0;
    virtual void options_save_input_modifiers() = 0; // 005F0B90
    // In-mission interface manager 00E198C4 and its limbo screen +E8h.
    virtual bool interface_present() = 0;
    virtual bool limbo_screen_present() = 0;
    virtual bool limbo_screen_active_byte4() = 0;
    // 008C9F80 over the argument: the widget named by string arg `index`, or null.
    virtual void* gui_find_widget(LuaArgs& args, int index) = 0;
    virtual bool gui_widget_vcall_38(void* widget) = 0;              // widget vtable+38h
    virtual std::string gui_widget_raw_text(void* widget) = 0;       // native string +F4h
    // Marker manager [[00E188A8]+21D4h].
    virtual void marker_add_gui_highlight(void* widget, bool a, bool b) = 0; // 006DAE20
    virtual void marker_remove_gui_highlight(void* widget) = 0;              // 006DC5A0
    // 004C12B0 GUI manager, 00AA5840 LoadPage(name, 1), then the page's vtable+34h(1).
    virtual void gui_load_page_and_show(const std::string& name) = 0;
    // byte [[00E198C4]+60h]+0Ch
    virtual void objectives_toggle_disabled_byte(bool disabled) = 0;
    // HUD counters [[00E198C4]+C8h].
    virtual void hud_counters_hide_unit_hp(int id) = 0;   // 0052B570
    virtual void hud_counters_hide_score(int a, int b) = 0; // 0052B900
};

// The input-modifier bytes 00F889C1..C4 (include/bsp/native_input_configuration_parse.hpp:
// C1 general swap, C4 map swap) and the backup copy 00F889C5..C8.
struct InputModifierBytes {
    std::uint8_t current[4]{};
    std::uint8_t backup[4]{};
};

int lua_get_map_cursor_pos(LuaResults& results, InterfaceHost& host);       // 008BFEE0
int lua_get_map_zoom(LuaResults& results, InterfaceHost& host);             // 008C0260
int lua_force_map_cursor_pos(LuaArgs& args, InterfaceHost& host);           // 008C0070
int lua_map_hide_node_by_name(LuaArgs& args, InterfaceHost& host);          // 008C03B0
int lua_map_unit_highlight(LuaArgs& args, InterfaceHost& host);             // 008C27C0
int lua_music_control_suspend(LuaArgs& args, InterfaceHost& host);          // 008C4ED0
int lua_music_fade(LuaArgs& args, InterfaceHost& host);                     // 008C7190
int lua_black_bars(LuaArgs& args, InterfaceHost& host);                     // 008C7300
// EnableInput 008AFBD0: context 1Eh level = enable ? 0 : 5 ((-(b != 0) & ~4) + 5).
int lua_enable_input(LuaArgs& args, InterfaceHost& host);
int enable_input_level(bool enable) noexcept;
inline constexpr int kScriptInputContext = 0x1E;
inline constexpr int kScriptInputBlockedLevel = 5;
int lua_get_input_on(LuaArgs& args, LuaResults& results, InterfaceHost& host);  // 008BFD50
int lua_set_input_modifiers(LuaArgs& args, InputModifierBytes& bytes,
    InterfaceHost& host);                                                    // 008C1150
int lua_get_input_modifiers(LuaResults& results, const InputModifierBytes& bytes); // 008C2660
int lua_is_swap_sticks_enabled(LuaResults& results, const InputModifierBytes& bytes); // 008C4370
int lua_is_limbo_gui_active(LuaResults& results, InterfaceHost& host);      // 008AB6F0
int lua_is_gui_active(LuaArgs& args, LuaResults& results, InterfaceHost& host); // 008CA010
int lua_set_gui_highlight(LuaArgs& args, InterfaceHost& host);              // 008CA1F0
int lua_get_raw_text(LuaArgs& args, LuaResults& results, InterfaceHost& host); // 008CA450
int lua_show_safe_zone(InterfaceHost& host);                                // 008C59D0
inline constexpr const char* kSafeZoneGuiPage = "SafeZoneGui";
int lua_enable_objectives_toggle(LuaArgs& args, InterfaceHost& host);       // 008C0CA0
int lua_hide_unit_hp(LuaArgs& args, InterfaceHost& host);                   // 008C1F50
int lua_hide_score_display(LuaArgs& args, InterfaceHost& host);             // 008C24B0

// ---------------------------------------------------------------------------
// Localisation, environment, narrative, effects and class bindings
// ---------------------------------------------------------------------------

struct ContentHost {
    virtual ~ContentHost() = default;
    // Localisation manager [00F8BC4C].
    virtual void localization_register_table_name(const std::string& name) = 0; // 00AA0D30
    virtual void localization_reload_tables(int flag) = 0;                     // 00AA06D0
    virtual void atmosphere_replace_sky_box(const std::string& name) = 0; // 00BBDF60 on [game+19E8h]
    virtual void rain_set_drop_num_percent(float fraction) = 0;         // 008003B0
    virtual void rain_settings_store_70(float fraction) = 0;            // [[00F8BBD8]+78h]+70h
    // Time of day [[00E188A8]+21C4h].
    virtual void time_of_day_set(int hours, int minutes, int seconds, int ms) = 0; // 004439B0
    virtual void time_of_day_split(int& hours, int& minutes, int& seconds) = 0;     // 00443A30
    // Lighting block [[00E188A8]+5FCh].
    virtual void light_store_float(std::size_t offset, float value) = 0;
    virtual void scene_ensure_default_lighting() = 0; // 004C9EC0
    // Vehicle class registry singleton 00437F50: dword at +2010h + id*4.
    virtual std::int32_t vehicle_class_remap(int class_id) = 0;
    // Mission narrative [[00E188A8]+21E8h].
    virtual int narrative_size() = 0;  // 00733260
    virtual void narrative_clear() = 0; // 00734FA0
    virtual void narrative_send_to_party(int party, const std::string& text) = 0;   // 00734B10
    virtual void narrative_send_to_player(int player, const std::string& text) = 0; // 00734BF0
    // Dialog panel owner [[00E188A8]+21E4h].
    virtual void dialog_break(const std::string& name) = 0;  // 00451B90
    virtual void dialog_kill(const std::string& name) = 0;   // 004514A0
    // Effects.
    virtual void point_effect_stop_children(void* effect) = 0; // 00867B10
    virtual void point_effect_set_byte9(void* effect) = 0;     // effect+9 = 1
    virtual void effect_manager_stop_matching(void* effect) = 0; // 004D1100 + 008674C0
};

int lua_add_lockit_path_to_selection(LuaArgs& args, ContentHost& host); // 008C5040
int lua_load_selected_paths(ContentHost& host);                         // 008C5230
int lua_set_sky(LuaArgs& args, ContentHost& host);                      // 008C49A0
int lua_set_rain_drop_num_percent(LuaArgs& args, ContentHost& host);    // 008C7F50
int lua_set_day_time(LuaArgs& args, ContentHost& host);                 // 008B1CC0
int lua_get_day_time(LuaResults& results, ContentHost& host);           // 008B1EC0
// SetLight 008C44B0: arg0.Ambient[1..3] / 255.0 -> +F48h..+F50h, +F54h = 1.0f;
// arg0.Diffuse[1..3] / 255.0 -> +FC8h..+FD0h, +FD4h = 1.0f; HorizontalAngle and
// VerticalAngle (default 0) in radians -> +1004h, +1008h; then the default-lighting refresh.
int lua_set_light(LuaArgs& args, ContentHost& host);
inline constexpr std::size_t kLightAmbientOffset = 0xF48;
inline constexpr std::size_t kLightDiffuseOffset = 0xFC8;
inline constexpr std::size_t kLightHorizontalAngleOffset = 0x1004;
inline constexpr std::size_t kLightVerticalAngleOffset = 0x1008;
int lua_is_class_changed(LuaArgs& args, LuaResults& results, ContentHost& host); // 008CC4B0
int lua_mission_narrative_size(LuaResults& results, ContentHost& host);  // 008B0AC0
int lua_mission_narrative_clear(ContentHost& host);                      // 008B15B0
int lua_mission_narrative_party(LuaArgs& args, ContentHost& host);       // 008B1210
int lua_mission_narrative_player(LuaArgs& args, ContentHost& host);      // 008B13E0
int lua_break_dialog(LuaArgs& args, ContentHost& host);                  // 008B07C0
int lua_kill_dialog(LuaArgs& args, ContentHost& host);                   // 008B0940
int lua_stop_effect(LuaArgs& args, ContentHost& host);                   // 008A9CA0
int lua_destroy_effect(LuaArgs& args, ContentHost& host);                // 008CB5B0

// ---------------------------------------------------------------------------
// Sound, movie, listener and loading bindings
// ---------------------------------------------------------------------------

struct PresentationHost {
    virtual ~PresentationHost() = default;
    // Voice manager [[00E198C4]+A4h]: current level +D4h, and 005B9760 BeginSoundFade.
    virtual float voice_current_level_d4() = 0;
    virtual void voice_begin_sound_fade(float target, float rate, const std::string& name) = 0;
    virtual float global_config_default_fade_e0() = 0; // 00432650 singleton +E0h
    // Native string [[00E188A8]+21E0h]+19Ch.
    virtual void set_skip_movie_name(const std::string& name) = 0;
    // 00888D20 over the argument: the entity pointer field, or null.
    virtual void* lua_table_entity_ptr(LuaArgs& args, int index) = 0;
    virtual void entity_set_force_3d_visible_2f0(void* entity, bool visible) = 0;
    // Message system [00F8A0C4] listeners (00980C10 add, 00980D30 remove, 00980E00 query).
    virtual void listener_add(const std::string& first, const std::string& second,
        LuaArgs& args, int callback_index) = 0;
    virtual void listener_remove(const std::string& first, const std::string& second) = 0;
    virtual bool listener_active(const std::string& first, const std::string& second) = 0;
    // 00450750: the active dialog ids (8-byte native strings).
    virtual std::vector<std::string> dialog_active_ids() = 0;
    virtual std::int32_t game_loading_depth_644() = 0; // [[00E188A8]+644h]
    virtual void please_wait_screen_hide() = 0;        // 0060D3E0
    virtual void game_set_cinematic_mode(int a, int b, int c) = 0; // 004CD0F0
};

// Force-3D-visibility flag 00F87152.
struct Force3dVisibilityByte {
    std::uint8_t* flag_00f87152{};
};

// SoundFade 008D16B0: target = arg0; delta = level - target; duration = optional arg2
// (default GlobalConfig+E0h), raised to 0.01f (00D7A238) when 0.01 (qword 00D7A358) exceeds
// it (FCOMPI / JBE at 008D18D5); rate = |delta / duration| (AND 7FFFFFFFh at 008D190A);
// optional name arg1 (empty when absent).
int lua_sound_fade(LuaArgs& args, PresentationHost& host);
float sound_fade_rate(float level, float target, float duration) noexcept;
// SetSkipMovie 008B28C0: optional string arg0 (empty when absent) into game+21E0h -> +19Ch.
int lua_set_skip_movie(LuaArgs& args, PresentationHost& host);
// Force3dVisibility 008B2AD0: one argument stores the boolean in 00F87152; otherwise the
// entity from arg0 gets byte +2F0h = boolean arg1 when it is non-null, and 00F87152 is
// rewritten with its own value.
int lua_force_3d_visibility(LuaArgs& args, Force3dVisibilityByte& g, PresentationHost& host);
// AddListener 008C6760 (strings arg0, arg1 and the LuaObject arg2), RemoveListener 008C6990,
// IsListenerActive 008C6BB0. Argument 1 is read before argument 0 in all three.
int lua_add_listener(LuaArgs& args, PresentationHost& host);
int lua_remove_listener(LuaArgs& args, PresentationHost& host);
int lua_is_listener_active(LuaArgs& args, LuaResults& results, PresentationHost& host);
// GetActDialogIDs 008CB730: a table with t[i + 1] = id i (00B672F0 SetLStringAtIndex).
struct LuaStringListResults {
    virtual ~LuaStringListResults() = default;
    virtual void push_string_array(const std::vector<std::string>& values) = 0;
};
int lua_get_act_dialog_ids(LuaStringListResults& results, PresentationHost& host);
// Loading_Finish 008C77F0: when game+644h < 1, hide the please-wait screen and call
// 004CD0F0(0, 0, 1).
int lua_loading_finish(PresentationHost& host);
// ---------------------------------------------------------------------------
// Camera reads, message maps, configured input and class unloading
// ---------------------------------------------------------------------------

struct GlueHost {
    virtual ~GlueHost() = default;
    // Game camera world position, as in GetCameraPosRot.
    virtual Vec3 game_camera_world_position() = 0;
    // The entity's world position +FCh..+104h after 00414DB0 when byte +C8h is clear.
    virtual Vec3 entity_world_position(void* entity) = 0;
    // The entity's cached inverse world matrix (+110h, built by 00B63D50 when byte +10Ch is
    // clear, which also sets it) applied to `point` by 004142E0.
    virtual Vec3 entity_world_to_local(void* entity, const Vec3& point) = 0;
    // 00706FC0 on [[00E188A8]+21DCh].
    virtual void message_map_unload(const std::string& name, int index) = 0;
    virtual std::int32_t game_session_mode_1fe4() = 0;
    // 0088B6D0(name, index, flag), 0076A9F0 on game+1EF0h, 00765590: the MANAGE_MESSAGES
    // session message; LoadMessageMap passes flag 0 and UnloadMessageMap flag 1.
    virtual void session_manage_messages(const std::string& name, int index, int flag) = 0;
    // 006A33F0 on 005547D0's input settings: the two text lists of an action's bindings.
    virtual void input_collect_action_binding_texts(int action, std::vector<std::string>& first,
        std::vector<std::string>& second) = 0;
    // ForceEnableInput: 0069FA40 inserts into the int set at [00E188A8]+5B0h, 008CA7B0 erases.
    virtual void forced_input_set_insert(int id) = 0;
    virtual void forced_input_set_erase(int id) = 0;
    // Vehicle class registry 00437F50: slot +10h[id], cache size +8h, cache base +4h.
    virtual std::int32_t vehicle_class_slot(int class_id) = 0;
    virtual std::int32_t vehicle_class_cache_size() = 0;
    virtual void vehicle_class_resize_cache(int size) = 0;        // 00437BD0
    virtual void vehicle_class_release_cached(int slot) = 0;      // intrusive release, slot = 0
    virtual void log_text(const std::string& text) = 0;           // 004254B0
};

// GetCameraRelPosW 008BF230: camera world position minus the entity's world position, as a
// {x, y, z} table. The entity is read (00888AA0) before the table is created.
int lua_get_camera_rel_pos_w(LuaArgs& args, LuaResults& results, GlueHost& host);
// GetCameraRelPosL 008BF460: the camera world position in the entity's local frame.
int lua_get_camera_rel_pos_l(LuaArgs& args, LuaResults& results, GlueHost& host);
// UnloadMessageMap 008C6490: 00706FC0(string arg0, int arg1); when game+1FE4h == 1 the
// session MANAGE_MESSAGES path with both arguments re-read and flag 1.
int lua_unload_message_map(LuaArgs& args, GlueHost& host);
// GetConfiguredInput 008CC030: two tables of strings (t[i + 1], 00B672F0), the first from
// 006A33F0's second argument, the second from its third.
int lua_get_configured_input(LuaArgs& args, LuaStringListResults& results, GlueHost& host);
// ForceEnableInput 008CBAD0: boolean arg1 false inserts int arg0 into the set, true erases.
int lua_force_enable_input(LuaArgs& args, GlueHost& host);
// UnloadClass 008CC630: for every argument, grow the cache when the slot is past its size,
// release and clear the cached class, then log "Unloading vehicle class %d" with the argument
// read again. The count is re-read every iteration.
int lua_unload_class(LuaArgs& args, GlueHost& host);
// GetCameraState 008BF6A0 and GetDistFromCrosshair 008C0560 read the game camera directly.
struct CameraStateHost {
    virtual ~CameraStateHost() = default;
    virtual Vec3 game_camera_vector_30() = 0;            // camera +30h, after the refresh
    virtual Vec3 game_camera_basis_angles() = 0;         // 0042D2E0, radians
    virtual Vec3 game_camera_world_position() = 0;       // +120h..+128h
    virtual Vec3 game_camera_direction_110() = 0;        // +110h..+118h
    virtual float game_camera_fov_1c4() = 0;             // +1C4h
    virtual float base_fov_00f889b4() = 0;               // float 00F889B4
    virtual Vec3 entity_world_position(void* entity) = 0;
    // 00794750 (src/cc13_camera_controllers.cpp) with (camera direction, entity offset).
    virtual std::array<float, 2> camera_direction_angle_delta(const Vec3& direction,
        const Vec3& offset) = 0;
};
struct CameraStateResult {
    Vec3 position;     // "Position" (00CE68B4)
    Vec3 rotation;     // "Rotation" (00D0D734), degrees in [0, 360)
    float zoom{};      // "Zoom" (00D14988)
};
struct CameraStateResults {
    virtual ~CameraStateResults() = default;
    virtual void push_camera_state(const CameraStateResult& state) = 0;
};
// One angle of GetCameraState: (float)(a + 131.9468994140625 (qword 00D13268)), fmod by
// 6.2831854820251465 (qword 00CE3828), narrowed, then / pi (00CE3D28) * 180 (00CE3D20).
float camera_state_degrees(float radians) noexcept;
int lua_get_camera_state(CameraStateResults& results, CameraStateHost& host);
// GetDistFromCrosshair: offset = entity - camera (float per axis), (a, b) = 00794750, s =
// a*a + b*b narrowed; 0 when s <= 1e-10 (qword 00CE3820, FCOMI / JBE), else sqrt narrowed;
// pushed as s / camera FOV (00B66480).
float dist_from_crosshair(float a, float b, float fov) noexcept;
int lua_get_dist_from_crosshair(LuaArgs& args, LuaResults& results, CameraStateHost& host);
// ---------------------------------------------------------------------------
// Movie-camera mode bindings (MovCam_*)
// ---------------------------------------------------------------------------

// Most modes end with the same optional tail, read in this order: a boolean (default true,
// taken only when the argument is a boolean), a number (default 0, only when a number), a
// string (default "" from 00CE3A0C, only when a string), and, when more arguments follow,
// every remaining argument collected as Lua variants (00887370(first + 1, -1, 4) over a
// LuaStateHolder, each pushed by 006EDF00 into a new 0Ch-byte vector) or null.
struct MovCamTail {
    bool flag{true};
    float number{0.0f};
    std::string name;
    int extras_from{-1}; // first argument index of the variant vector, -1 for null
};
MovCamTail read_movcam_tail(LuaArgs& args, int first);

// Every mode calls 005CD160 first (see CameraHost::movcam_ensure_target) and then its setter
// on the returned movcam+30h.
struct MovCamModeHost {
    virtual ~MovCamModeHost() = default;
    virtual void movcam_ensure_target() = 0;                     // 005CD160
    virtual std::int32_t game_session_mode_1fe4() = 0;
    virtual void set_hand_mode_targeted(void* entity, bool a, bool b) = 0;      // 007A6B40
    virtual void broadcast_hand_mode_targeted(void* entity, bool a, bool b) = 0; // 0076D390
    // 0079F400: index of a named vehicle camera; its ECX was not traced.
    virtual int vehicle_camera_index_of_name(void* entity, const std::string& name) = 0;
    virtual void set_hand_mode_deck(void* entity, int camera_index) = 0;        // 007A6B90
    virtual void set_hand_mode_overview(void* entity, const MovCamTail& tail, LuaArgs& args) = 0; // 007A6BC0
    virtual void set_ref_pos(void* entity, const Vec3& position, const MovCamTail& tail,
        LuaArgs& args) = 0;                                                      // 007A6CD0
    virtual void set_ref_pos_polar(void* entity, float a, float b, float c, const MovCamTail& tail,
        LuaArgs& args) = 0;                                                      // 007A6E60
    virtual void broadcast_ref_pos_polar(void* entity, float a, float b, float c, bool flag,
        float number) = 0;                                                       // 0076D460
    virtual void set_ref_pos_radius(void* entity, float radius, const MovCamTail& tail,
        LuaArgs& args) = 0;                                                      // 007A7070
    virtual void broadcast_ref_pos_radius(void* entity, float radius, bool flag,
        float number) = 0;                                                       // 0076D510
    // 007AC9D0: the entity's path interface; the point count is (+0Ch - +08h) >> 2, or 0 when
    // +08h is null.
    virtual void* entity_path_interface(void* entity) = 0;
    virtual int path_point_count(void* path) = 0;
    virtual void set_path_walk(void* path, int start, int end, float speed, const MovCamTail& tail,
        LuaArgs& args) = 0;                                                      // 007A7200
    virtual void set_viktor_mode(void* first, void* second, float a, float b, const MovCamTail& tail,
        LuaArgs& args) = 0;                                                      // 007A7DE0
    virtual void set_tompi_mode_weak(void* first, const Vec3& a, void* second, const Vec3& b,
        const MovCamTail& tail, LuaArgs& args) = 0;                              // 007A7F30
    virtual void set_tompi_mode(void* first, const Vec3& a, void* second, const Vec3& b) = 0; // 007A8100
    virtual void set_secondary_target(void* entity, const MovCamTail& tail, LuaArgs& args) = 0; // 0079D870
    // operator new(18h) + 007900B0(name, a1, a2, a3, a4); null when the allocation fails.
    virtual void* interpolator_create(const std::string& name, float a1, float a2, float a3,
        float a4) = 0;
    virtual bool interpolator_allocation_succeeds() = 0;
    virtual void add_interpolator(void* interpolator) = 0;                      // 0079F6D0
    virtual void broadcast_add_interpolator(void* interpolator) = 0;            // 0076E750
    virtual void pick_deck_position(const std::string& name, void* entity, void* other) = 0; // 007A8180
};

int lua_movcam_hand_mode_targeted(LuaArgs& args, MovCamModeHost& host);   // 008B2CC0
int lua_movcam_hand_mode_deck(LuaArgs& args, MovCamModeHost& host);       // 008B2FE0
int lua_movcam_hand_mode_overview(LuaArgs& args, MovCamModeHost& host);   // 008B3270
int lua_movcam_ref_pos(LuaArgs& args, MovCamModeHost& host);              // 008B3740
int lua_movcam_ref_pos_polar(LuaArgs& args, MovCamModeHost& host);        // 008B3C70
int lua_movcam_ref_pos_radius(LuaArgs& args, MovCamModeHost& host);       // 008B4270
int lua_movcam_path_walk(LuaArgs& args, MovCamModeHost& host);            // 008B47E0
int lua_movcam_viktor_mode(LuaArgs& args, MovCamModeHost& host);          // 008B52F0
int lua_movcam_tompi_mode_weak(LuaArgs& args, MovCamModeHost& host);      // 008B5870
int lua_movcam_tompi_mode(LuaArgs& args, MovCamModeHost& host);           // 008B5DF0
int lua_movcam_set_secondary_target(LuaArgs& args, MovCamModeHost& host); // 008B60C0
int lua_movcam_add_interpolator(LuaArgs& args, MovCamModeHost& host);     // 008B6740
int lua_movcam_pick_snitt_deck_position(LuaArgs& args, MovCamModeHost& host); // 008B6AD0

// ---------------------------------------------------------------------------
// Small accessors and value types
// ---------------------------------------------------------------------------

// 008D8F10: the peeked token (delimiters 00CE3A34) is an int when sscanf("%d") assigns one
// field. 008D8F40 is the "%f" twin (delimiters 00CE4334), already modelled as parse_float.
bool scene_token_is_int(const std::string& token);
// 008DAD70: dword +28h of an objective entry (no caller in the index).
std::int32_t objective_entry_field_28(const std::uint8_t* entry) noexcept;
// 008DADA0: [world+21A4h] != 0 (no caller in the index).
bool world_has_entity_set0(const std::uint8_t* world) noexcept;
// 008DAD80, __thiscall(message, int id), RET 4: 0075B430 (session message base) with the id, dword
// +4 = 1, vtable 00D16024.
struct ObjectiveSessionMessageHeader {
    std::uint32_t vtable{};
    std::int32_t one_04{};
};
ObjectiveSessionMessageHeader objective_session_message_header() noexcept;
inline constexpr std::uint32_t kObjectiveSessionMessageVtable = 0x00D16024u;
// 008C8980, __thiscall(out, const NativeString* source), RET 4: copy the string, engaged
// byte +8 = 1. 008C9350: a nil argument leaves the caller's optional untouched, anything
// else is converted to a string and engages it.
std::optional<std::string> optional_string_from(const std::string& source);
void lua_object_get_optional_string(LuaArgs& args, int index, std::optional<std::string>& out);

// ---------------------------------------------------------------------------
// Non-binding helpers of the segment
// ---------------------------------------------------------------------------

using Rgba = std::array<float, 4>;

// 008DA920 / 008DA970: lazily initialised static colours, guard bit 1 of 00F88BD4 / 00F88BE8.
// Black (0,0,0,1) lives at 00F88BC4, red (1,0,0,1) at 00F88BD8. 1.0 is the float 00D7A24C.
Rgba static_color_black() noexcept;
Rgba static_color_red() noexcept;

// 008DAA60, __fastcall(Rgba* out in ECX, int index in EDX), plain RET, no callers in the index.
// Index 0/1/2 returns 0.5 * black + 0.5 * blue (00F88C00) / green (00F88BEC) / red through
// 00BEE140 with the scalar 0.5 (00CE3800) stored at entry of every branch; any other index
// copies black.
Rgba color_blend_from_black_by_index(int index) noexcept;

// 008DACD0, __fastcall(entry). True when the entry has no entity; otherwise reads the local
// side (game+18ECh, 0..7), that side's party row index (*(game+18CCh+side*4)+28h), and from
// the entity's 34h-byte party rows at +1E8h the dword +4h or +8h depending on byte +10h of
// the row; true when that value is above 1. An out-of-range side answers false.
struct ObjectiveRevealView {
    bool has_entity{};
    std::int32_t local_side{};          // [[00E188A8]+18ECh]
    std::int32_t side_party_row{};      // *([[00E188A8]+18CCh+side*4])+28h
    bool row_selector_byte{};           // entity+row*34h+1F8h
    std::int32_t row_value_plus4{};     // entity+row*34h+1ECh
    std::int32_t row_value_plus8{};     // entity+row*34h+1F0h
};
bool objective_entry_is_revealed(const ObjectiveRevealView& view) noexcept;

// 008DAD30, __thiscall(entry, Vec3* out), RET 4: the entity's position through its
// vtable+C4h when an entity is set, else the stored +4h/+8h/+0Ch. Returns `out`.
struct ObjectiveEntryView {
    void* entity{};
    Vec3 stored_position{};
};
struct ObjectiveEntityHost {
    virtual ~ObjectiveEntityHost() = default;
    virtual Vec3 entity_vcall_c4_position(void* entity) = 0;
};
Vec3 objective_entry_position(const ObjectiveEntryView& entry, ObjectiveEntityHost& host);

// 008DADC0, __thiscall(message, unsigned type), RET 4: accepts type 36h or the byte at
// message+10h.
bool objective_session_message_accepts(std::uint32_t type, std::uint8_t own_type) noexcept;
inline constexpr std::uint32_t kObjectiveSessionMessageAnyType = 0x36;

// 008DB990, __thiscall(out, property sink), RET 4: when the sink's vtable+14h answers true it
// is told vtable+10h(0, "entity"), otherwise vtable+10h(0, "pos") (00CF8FA4); the
// out dword is zeroed on both paths.
struct PropertyKeySink {
    virtual ~PropertyKeySink() = default;
    virtual bool vcall_14() = 0;
    virtual void vcall_10(int zero, const char* key) = 0;
};
std::uint32_t objective_entry_select_entity_or_pos_key(PropertyKeySink& sink);

// 008C8C20, __fastcall(path): the GUI widget named by a dotted path. A '.' (00CE3A70) is
// appended, the first segment names a page (00AA3140 on the GUI manager 004C12B0), and each
// further segment is looked up as a child of the current widget (00AA7E00, second argument
// 0); a missing page or child answers null, an exhausted path answers the current widget.
struct GuiTreeHost {
    virtual ~GuiTreeHost() = default;
    virtual void* find_page(const std::string& name) = 0;
    virtual void* find_child(void* widget, const std::string& name) = 0;
};
void* gui_find_widget_by_dotted_path(const std::string& path, GuiTreeHost& host);
// 008C9F80, ECX = the argument LuaObject: GetString (00B662B0) into a native string, then
// 008C8C20 on it.
void* lua_gui_find_widget_from_string_arg(LuaArgs& args, int index, GuiTreeHost& host);

// 008D76C0: clears the output vector (00427110 to 0) and appends entry +0 (the lanfile
// string) of each of the [00F88978] 20h-byte rows at [00F88974] (004CDC20).
std::vector<std::string> collect_language_lanfiles(const std::vector<LanguageEntry>& table);
} // namespace bsp::cc13_seg56
