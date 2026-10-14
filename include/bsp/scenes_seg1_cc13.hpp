#pragma once
// Segment 1 (00413D10-0042A7E0), cc13-seg1: the automation driver's per-screen
// step and its MPakScenes walker, continuing include/bsp/seg1_autotest_cc13.hpp
// (cc13-camera2). docs/CC13_SEG1.md has the evidence. Names are hypotheses,
// not recovered symbols. Every engine call the image makes is a named host
// method invoked at the same point; nothing here runs in the game yet.
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/scene_contents_hosts.hpp"
#include "bsp/seg1_autotest_cc13.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp::seg1_cc13 {

// The six campaign names 00426650 indexes by driver+18h: the pointer table at
// 00E08048 -> 00CE39DC "us", 00CE39D4 "japan", 00CE39CC "multi",
// 00CE39C0 "training", 00CE39B8 "dlcusa", 00CE39B0 "dlcjap".
inline constexpr const char* kMpakCampaignNames_00e08048[6] = {
    "us", "japan", "multi", "training", "dlcusa", "dlcjap"};

// 00425EB0 __thiscall(driver, listener, listbox, value, by_index) RET 10h ->
// AL. The driver is not read. by_index == 0: when 00A9C990(listbox) == value,
// listener->vtable[4](00425E50(listbox), listbox) and return 1, else
// 00A9C860(listbox, value) and return 0. by_index != 0: the same with
// 00A9C920 and 00A9C7C0.
bool autotest_select_listbox_item_00425eb0(void* listener, void* listbox, std::int32_t value,
                                           bool by_index, ListboxSelectHost& host);

// The Lua objects 00426650 walks (the LuaObject handles are the host's).
struct MpakScenesLuaHost {
    virtual ~MpakScenesLuaHost() = default;
    // 00B67980 on [00E188A8]+1A0Ch (globals) then 00B68D70 ResolveDotPath(path,
    // &found); the globals temporary is destroyed (00B67700) right after.
    virtual void* lua_resolve_global_path_00b68d70(const std::string& path, bool& found) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual bool lua_is_nil_00b65fb0(void* object) = 0;
    virtual bool lua_is_integer_00b66a60(void* object) = 0;
    virtual std::int32_t lua_get_integer_00b66290(void* object) = 0;
    // 00B67720 __thiscall(table, out, index) RET 8.
    virtual void* lua_get_by_index_00b67720(void* table, std::int32_t index) = 0;
    virtual void lua_destroy_00b67700(void* object) = 0;
};

// 00426650 __fastcall(driver): advance (+18h campaign, +1Ch mission, +28h
// multiplayer mode 0..6) to the next entry MPakScenes enables. Loops until it
// returns:
//   campaign 2 (multi): while +28h < 7, look up MPakScenes.multi.<+1Ch>; when
//     it is a table, its [+28h + 1] element (not nil, GetInteger != 0) ends
//     the scan, else +28h++; otherwise MPakScenes.Modes.<+28h> (found,
//     GetInteger != 0) ends it, else +28h++. Then +28h >= 7 -> +1Ch++, +28h = 0.
//   +1Ch >= 100 -> +18h++, +1Ch = 0; return when +18h >= 6 or the mode (+8h)
//   is not 2. MPakScenes.<campaign>.<+1Ch> found and (an integer != 0, or a
//   table while +18h == 2) returns; else +1Ch++, +28h = 0 and loop.
void autotest_next_mpak_scene_00426650(AutoTestDriver& d, MpakScenesLuaHost& host);

// 00425500 __fastcall(lobby): 005D67E0(lobby, [lobby+8Ch]) then 005D5870(lobby, 1).
struct LobbyFocusHost {
    virtual ~LobbyFocusHost() = default;
    virtual std::uint32_t lobby_field_8c(void* lobby) = 0;                 // [lobby+8Ch]
    virtual void lobby_set_focus_005d67e0(void* lobby, std::uint32_t focus) = 0;
    virtual void lobby_set_editable_005d5870(void* lobby, std::int32_t editable) = 0;
};
void autotest_focus_lobby_00425500(void* lobby, LobbyFocusHost& host);

// The engine side of 00427190. Global singletons: game [00E188A8], GUI roots
// [00E198AC], [00E198B4], [00E198C4], movie player [00E18D48].
struct AutoTestStepHost : AutoTestHost, ScreenHost, MpakScenesLuaHost,
                          LobbyFocusHost {
    // [[00E188A8]+38h] != 0 and its +60h == 1: a movie is playing.
    virtual bool movie_playing_game_38() = 0;
    virtual void movie_stop_0045f600() = 0;                   // ECX = [[00E188A8]+38h]
    // 00425C20 (profile manager singleton) then 004374F0 on it.
    virtual void profile_refresh_checkpoint_004374f0() = 0;
    virtual void title_screen_skip_0068d8a0() = 0;
    // byte 00E08070: run SCRIPTS/datatables/MPakScenes.lua once.
    virtual bool mpak_scenes_script_pending_00e08070() = 0;
    virtual void clear_mpak_scenes_script_pending_00e08070() = 0;
    // 00B69D40 __thiscall([00E188A8]+1A0Ch, const NativeString* path, 0).
    virtual void run_script_with_overrides_00b69d40(const std::string& path, std::int32_t flags) = 0;
    virtual bool flag_00e17659() = 0;
    virtual void increment_00e1765c() = 0;                    // ADD [00E1765C],1
    virtual bool is_loading_state_0057d430() = 0;             // ECX = [[00E198AC]+58h]
    virtual void build_top_level_page_00584ae0(std::int32_t page) = 0;  // ECX = [[00E198AC]+58h]
    // 00425D10 (menu command screen singleton) and its +1D0h.
    virtual std::int32_t menu_command_screen_1d0_00425d10() = 0;
    // 00425D10 again, then 00532C50(value) on it.
    virtual void menu_prompt_complete_00532c50(std::int32_t value) = 0;
    virtual void build_objective_rows_00594bf0() = 0;         // ECX = [[00E198AC]+58h]
    virtual void open_tactical_library_005885d0() = 0;        // ECX = [[00E198AC]+58h]
    virtual void return_to_mission_list_00599340() = 0;       // ECX = [[00E198AC]+58h]
    virtual void show_difficulty_select_004ff6f0() = 0;       // ECX = [[00E198AC]+70h]
    virtual void play_selected_mission_005922f0() = 0;        // ECX = [[00E198AC]+58h]
    virtual void* multi_mode_selector_e198b4_44() = 0;        // [[00E198B4]+44h]
    virtual void* multi_lobby_e198b4_48() = 0;                // [[00E198B4]+48h]
    virtual void multi_mode_go_back_005eca90() = 0;           // ECX = [[00E198B4]+44h]
    virtual bool lobby_check_client_compat_005d71b0(void* lobby) = 0;
    virtual void lobby_set_setting_row1_005d6350(void* lobby, std::int32_t value) = 0;
    virtual void select_scene_record_004c6890(std::int32_t index) = 0;  // ECX = [00E188A8]
    virtual void* lobby_listbox_228(void* lobby) = 0;          // [lobby+228h]
    virtual void lobby_step_setting_005e1cf0(void* lobby, std::int32_t step) = 0;
    virtual void lobby_apply_selected_map_005da530(void* lobby) = 0;
    virtual void lobby_vslot20(void* lobby, float value) = 0;  // lobby->vtable[20h](float)
    virtual void enter_mission_state_004da6c0() = 0;          // ECX = [00E188A8]
    // 008875D0 result sink, 006B8AD0 on [[00E188A8]+1A08h]+4 (code, &sink,
    // &NativeString{0,0}, 2), then 0041DD20 and 00887670 on the temporaries.
    virtual void lua_run_string_006b8ad0(const char* code, std::int32_t results) = 0;
    virtual void toggle_pause_menu_004db030() = 0;            // ECX = [00E188A8]
    virtual void movie_player_skip_004f8960() = 0;            // ECX = [00E18D48] -> 00AB04A0
    // [00F8A304], null when 0; 00425B20 counts its flagged records.
    virtual const std::vector<FlaggedRecord>* records_00f8a304() = 0;
    virtual void* screen_e198c4_d8() = 0;                     // [[00E198C4]+D8h]
    virtual void* screen_e198c4_e4() = 0;                     // [[00E198C4]+E4h]
    // A dword field of a GUI object: the list boxes at screen+20h / +13Ch.
    virtual void* gui_field(void* object, std::uint32_t offset) = 0;
};

// 00427190 __thiscall(driver, int screen) RET 4 -> AL, called by 13 screen
// Update functions with their id. Mode 0 returns 0 at once. Screen 0Ch with
// a movie playing counts +10h and stops the movie past 2; screen 0Eh returns
// 1; every other call clears +10h (and counts +30h on screen 0Ch in mode 3),
// decrements +Ch, and at 0 reloads 50 and runs the screen's step. Returns 0.
bool autotest_step_00427190(AutoTestDriver& d, std::int32_t screen, AutoTestStepHost& host);

// ---------------------------------------------------------------------------
// TerrainGrid / TerrainGridLayer stream I/O (the .nav element grammar). The
// stream is the object behind the holder the image passes (`*holder`, re-read
// before every call); the slots are its vtable offsets. Every call passes a
// trailing 0 (the key/name argument), kept as `key`.
struct TerrainGridStreamHost {
    virtual ~TerrainGridStreamHost() = default;
    virtual void write_string_64(const std::string& value, std::int32_t key) = 0;
    virtual void write_float_58(float value, std::int32_t key) = 0;   // FLD m32; FSTP [esp]
    virtual void write_int_54(std::int32_t value, std::int32_t key) = 0;
    virtual void write_bytes_28(const std::uint8_t* data, std::int32_t count, std::int32_t key) = 0;
    // vtable +60h(&NativeString out, key): the class name, read into a local
    // that the reader destroys without using.
    virtual std::string read_string_60(std::int32_t key) = 0;
    virtual float read_float_44(std::int32_t key) = 0;               // ST0, FSTP m32
    virtual std::int32_t read_int_38(std::int32_t key) = 0;
    virtual void read_bytes_24(std::uint8_t* data, std::int32_t count, std::int32_t key) = 0;
    // 00BF6713 (_invalid_parameter_noinfo): the checked `&grid[0]` on an
    // empty byte vector, before the bytes call.
    virtual void invalid_parameter_00bf6713() = 0;
};

// 0041EF90 __thiscall(layer, stream holder) RET 4: "TerrainGridLayer"
// (00CE38D0) through +64h, then +04h (+58h), +08h n (+54h), +0Ch, +10h, +14h
// (+58h each) and the n*n grid bytes at +1Ch (+28h).
void terrain_grid_layer_serialize_0041ef90(const TerrainGridLayerRecord& layer, TerrainGridStreamHost& stream);
// 0041F130 __thiscall(registry, stream holder) RET 4: "TerrainGrid"
// (00CE38E8) through +64h, the list size (registry+0Ch) through +54h, then
// 0041EF90 on each list node's layer in list order.
void terrain_grid_save_layers_0041f130(const std::vector<TerrainGridLayerRecord>& layers,
                                       TerrainGridStreamHost& stream);
// 004239E0 __thiscall(layer, stream holder) RET 4: +60h class name (dropped),
// +44h -> +04h, +38h -> +08h n, +44h -> +0Ch, +10h, +14h, 004219A0 resizes
// the grid to n*n (new bytes 0), then +24h reads n*n bytes into it.
void terrain_grid_layer_deserialize_004239e0(TerrainGridLayerRecord& layer, TerrainGridStreamHost& stream);
// 004246E0 __thiscall(layer, stream holder) RET 4: vtable 00CE38CC, the grid
// vector zeroed (+1Ch..+24h), then 004239E0. Returns the layer.
TerrainGridLayerRecord terrain_grid_layer_construct_from_stream_004246e0(TerrainGridStreamHost& stream);

// 004180A0 __thiscall(layer, float out[2], const float in[2]) RET 8: grid to
// world, the inverse of 00417FA0: out = in * cell_size (+0Ch) - half_extent
// (+04h), each product stored to float before the subtraction. No Ghidra
// callers (no direct call or xref found).
void terrain_grid_cell_to_world_004180a0(const TerrainGridLayerRecord& layer, float out[2],
                                         const float in[2]) noexcept;

// ---------------------------------------------------------------------------
// Session bit-stream helpers.

// 00429140 __thiscall(cursor, const uint16_t* words, uint32_t count, uint32_t
// bits) RET 0Ch: each word zero-extended into a dword temporary (the count
// argument's stack slot) and written with 00428F50(cursor, &temp, bits).
void write_native_word_array_00429140(NativeBitCursor* cursor, const std::uint16_t* words,
                                      std::uint32_t count, std::uint32_t bits);

// The MT_VOICEDATA payload: a 64-bit header at +0 and a std::vector of 10-byte
// frames at +8 (begin +0Ch, end +10h).
struct VoiceDataPayload {
    std::uint32_t header_00[2]{};
    std::vector<std::array<std::uint8_t, 10>> frames_08;
};
// 00429B00 __thiscall(cursor, const payload*) RET 4: 64 header bits, the frame
// count truncated to a byte (8 bits), then that many frames of 80 bits.
void write_voice_data_payload_00429b00(NativeBitCursor* cursor, const VoiceDataPayload& payload);
// 0042A480 __thiscall(cursor, payload*) RET 4: 64 header bits, an 8-bit count
// (byte zeroed first), 0042A380 resizes the frames to it (new frames zero),
// then each frame is read as 80 bits into a temporary and copied in.
void read_voice_data_payload_0042a480(NativeBitCursor* cursor, VoiceDataPayload& payload);

// ---------------------------------------------------------------------------
// Small leaf routines.

// 00418280 / 00425A80 __fastcall(object): the singleton-base destructor
// bodies of two singletons: clear the publication cell (00F8753C GameSettings,
// 00E17664 the automation driver) and store the base vtable 00CE3818.
inline constexpr std::uint32_t kSingletonBaseVtable_00ce3818 = 0x00CE3818u;
void singleton_base_destruct_00418280(std::uint32_t& vtable_00, void*& cell_00f8753c) noexcept;
void singleton_base_destruct_00425a80(std::uint32_t& vtable_00, void*& cell_00e17664) noexcept;

// 00425BD0 __fastcall(const NativeString*): atol (00BF8417) of the data, or of
// the empty string at 00E17668 when the data pointer is null.
std::int32_t native_string_to_long_00425bd0(const char* data_04) noexcept;

// 00429FD0 __thiscall(string, const char*) RET 4: strlen, then 00408720
// assign-counted; returns its result (the string).
NativeLegacySboStringStorage& native_legacy_sbo_string_assign_cstring_00429fd0(
    NativeLegacySboStringStorage& string, const char* text);

}  // namespace bsp::seg1_cc13
