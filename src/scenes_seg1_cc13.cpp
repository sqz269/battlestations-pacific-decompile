// cc13-seg1 (docs/CC13_SEG1.md). Evidence per function in include/bsp/scenes_seg1_cc13.hpp.
#include "bsp/scenes_seg1_cc13.hpp"

#include <cstdlib>
#include <cstring>
#include <string>

namespace bsp::seg1_cc13 {

namespace {

constexpr std::int32_t kStepReload = 50;            // 00427249 MOV [ESI+0Ch],32h
constexpr std::int32_t kStepLongReload = 200;       // 00427446 / 004275BB / 00427660 C8h
constexpr std::int32_t kMovieFramesBeforeStop = 2;  // 0042720E CMP [ESI+10h],2 / JLE
constexpr std::int32_t kCampaignCount = 6;          // 00426B55 / 004272E1 CMP 6
constexpr std::int32_t kMultiModeCount = 7;         // 00426A80 CMP [EBP+28h],7
constexpr std::int32_t kMissionLimit = 100;         // 00426B43 CMP [EBP+1Ch],64h
constexpr std::int32_t kEndedMarker = 0x29A;        // 00427687 (666)
constexpr const char* kMpakScenesScript = "SCRIPTS/datatables/MPakScenes.lua";  // 00CE3AB0
constexpr const char* kClassEnumeration = "class_enumeration()";              // 00CE3A9C

// "MPakScenes." (00CE3A64) + campaign name + "." (00CE3A70) + decimal mission
// (004261A0, 004261A0, 004263B0).
std::string mpak_scene_path(std::int32_t campaign, std::int32_t mission) {
    return std::string("MPakScenes.") + kMpakCampaignNames_00e08048[campaign] + "." +
           std::to_string(mission);
}

// screen ? screen + 8 : 0, the listener sub-object the image computes before
// every 00425EB0 call (e.g. 00427740 ADD EAX,8 after the null test).
void* listener_of(void* screen) {
    return screen != nullptr ? static_cast<void*>(static_cast<std::uint8_t*>(screen) + 8) : nullptr;
}

}  // namespace

bool autotest_select_listbox_item_00425eb0(void* listener, void* listbox, std::int32_t value,
                                           bool by_index, ListboxSelectHost& host) {
    // 00425EB0: CMP byte [esp+10h] (by_index) selects the getter and setter pair.
    const std::int32_t current = by_index ? host.listbox_selected_index_00a9c920(listbox)
                                          : host.listbox_selected_data_00a9c990(listbox);
    if (current == value) {
        // 00425ED3..00425EE1: PUSH listbox; 00425E50(listbox); PUSH EAX; listener->vtable[4].
        host.listener_activate_vslot04(listener, host.listbox_current_row_00425e50(listbox), listbox);
        return true;
    }
    if (by_index) {
        host.listbox_select_index_00a9c7c0(listbox, value);  // 00425F29
    } else {
        host.listbox_select_data_00a9c860(listbox, value);   // 00425EEE
    }
    return false;
}

void autotest_next_mpak_scene_00426650(AutoTestDriver& d, MpakScenesLuaHost& host) {
    for (;;) {  // 00426E21 JMP 00426677
        if (d.campaign_18 == 2) {
            while (d.f_28 < kMultiModeCount) {
                bool found = false;
                void* scene = host.lua_resolve_global_path_00b68d70(
                    mpak_scene_path(d.campaign_18, d.mission_1c), found);  // 0042689E
                bool done = false;
                if (found && host.lua_is_table_00b661b0(scene)) {           // 004268C5
                    void* entry = host.lua_get_by_index_00b67720(scene, d.f_28 + 1);  // 004268E1
                    if (!host.lua_is_nil_00b65fb0(entry) && host.lua_get_integer_00b66290(entry) != 0) {
                        done = true;                                        // 00426906 -> 00426A8F
                    } else {
                        d.f_28 += 1;                                        // 0042690C
                    }
                    host.lua_destroy_00b67700(entry);
                } else {
                    // 00426926..004269AE: "MPakScenes.Modes." (00CE3A50) + decimal +28h.
                    void* mode = host.lua_resolve_global_path_00b68d70(
                        "MPakScenes.Modes." + std::to_string(d.f_28), found);
                    if (found && host.lua_get_integer_00b66290(mode) != 0) {
                        done = true;                                        // 00426A25 -> 00426AE0
                    } else {
                        d.f_28 += 1;                                        // 00426A2B
                    }
                    host.lua_destroy_00b67700(mode);
                }
                host.lua_destroy_00b67700(scene);
                if (done) break;                                            // -> 00426B31
            }
            if (d.f_28 >= kMultiModeCount) {                                // 00426B31
                d.mission_1c += 1;
                d.f_28 = 0;
            }
        }
        if (d.mission_1c >= kMissionLimit) {                                // 00426B43
            d.campaign_18 += 1;
            d.mission_1c = 0;
        }
        if (d.campaign_18 >= kCampaignCount) return;                        // 00426B55
        if (d.mode_08 != 2) return;                                         // 00426B5F
        bool found = false;
        void* scene = host.lua_resolve_global_path_00b68d70(
            mpak_scene_path(d.campaign_18, d.mission_1c), found);           // 00426CBB
        // 00426DC2..00426DFD: found and (an integer != 0, or a table in campaign 2).
        if (found && ((host.lua_is_integer_00b66a60(scene) && host.lua_get_integer_00b66290(scene) != 0) ||
                      (host.lua_is_table_00b661b0(scene) && d.campaign_18 == 2))) {
            host.lua_destroy_00b67700(scene);                               // 00426E38
            return;
        }
        d.mission_1c += 1;                                                  // 00426DFF
        d.f_28 = 0;                                                         // 00426E0A
        host.lua_destroy_00b67700(scene);                                   // 00426E1C
    }
}

void autotest_focus_lobby_00425500(void* lobby, LobbyFocusHost& host) {
    host.lobby_set_focus_005d67e0(lobby, host.lobby_field_8c(lobby));  // 0042550A
    host.lobby_set_editable_005d5870(lobby, 1);                        // 00425513
}

bool autotest_step_00427190(AutoTestDriver& d, std::int32_t screen, AutoTestStepHost& host) {
    const std::int32_t mode = d.mode_08;
    if (mode == 0) return false;  // 004271B3
    if (screen == 0x0C) {
        if (host.movie_playing_game_38()) {                        // 004271F9..00427209
            d.movie_frames_10 += 1;
            if (d.movie_frames_10 > kMovieFramesBeforeStop) host.movie_stop_0045f600();  // 00427221
            return false;
        }
        d.movie_frames_10 = 0;                                     // 0042722E
        if (mode == 3) d.f_30 += 1;                                // 00427233
    } else {
        d.movie_frames_10 = 0;                                     // 004271DE
        if (screen == 0x0E) return true;                           // 004271E3 MOV AL,BL
    }
    d.countdown_0c -= 1;                                           // 00427239
    if (d.countdown_0c > 0) return false;
    d.countdown_0c = kStepReload;                                  // 00427249, before the range check
    ScreenHost& screens = host;
    switch (screen) {                                              // jump table 004277E4
    case 0x00:                                                     // 0042725D
        host.profile_refresh_checkpoint_004374f0();
        host.title_screen_skip_0068d8a0();
        break;
    case 0x01: {                                                   // 00427273
        if (host.mpak_scenes_script_pending_00e08070()) {
            host.run_script_with_overrides_00b69d40(kMpakScenesScript, 0);
            host.clear_mpak_scenes_script_pending_00e08070();     // 004272B7
        }
        bool top_level = static_cast<std::uint32_t>(mode - 2) > 1u;  // 004272C4 JA: modes 2 and 3 only
        if (!top_level) {
            autotest_next_mpak_scene_00426650(d, host);            // 004272D3
            if (host.flag_00e17659() && d.campaign_18 >= kCampaignCount) {
                // 004272E8..004272F2: start the scenes pass over and count it.
                autotest_select_mpak_scenes_00425570(d, host.flag_00e17659());
                autotest_next_mpak_scene_00426650(d, host);
                host.increment_00e1765c();
            }
            if (d.campaign_18 >= kCampaignCount) {
                top_level = true;                                  // 004272FB JGE 00427370
            } else if (!host.is_loading_state_0057d430()) {
                autotest_ensure_screen_00426350(0, screens);       // 0042730E
            } else {
                switch (d.campaign_18) {                           // jump table 00427828
                case 0: autotest_ensure_screen_00426350(1, screens); break;
                case 1: autotest_ensure_screen_00426350(0, screens); break;
                case 2: autotest_ensure_screen_00426350(5, screens); break;
                case 3: autotest_ensure_screen_00426350(4, screens); break;
                case 4: autotest_ensure_screen_00426350(2, screens); break;
                case 5: autotest_ensure_screen_00426350(3, screens); break;
                default: top_level = true; break;                  // 00427322 JA (negative)
                }
            }
        }
        if (top_level) {                                           // 00427370
            if (host.is_loading_state_0057d430() && d.campaign_18 != 2) {
                host.build_top_level_page_00584ae0(0);             // 00427393
            }
            autotest_ensure_screen_00426350(6, screens);           // 0042739B
        }
        break;
    }
    case 0x02:                                                     // 004273A5
        host.menu_prompt_complete_00532c50(host.menu_command_screen_1d0_00425d10() == 0 ? 1 : 0);
        break;
    case 0x03:                                                     // 004273D8
        autotest_next_mpak_scene_00426650(d, host);
        if (autotest_ensure_screen_00426350(d.mission_1c, screens)) d.mission_1c += 1;
        break;
    case 0x04: host.build_objective_rows_00594bf0(); break;       // 004273FA
    case 0x05:                                                     // 00427432
        if (d.campaign_18 == 2) {
            host.return_to_mission_list_00599340();
        } else {
            host.open_tactical_library_005885d0();
            d.countdown_0c = kStepLongReload;
        }
        break;
    case 0x06: host.show_difficulty_select_004ff6f0(); break;     // 0042740D
    case 0x07: host.play_selected_mission_005922f0(); break;      // 00427420
    case 0x08: {                                                   // 00427465
        autotest_next_mpak_scene_00426650(d, host);
        if (d.campaign_18 != 2 || d.mission_1c >= kMissionLimit) {
            host.multi_mode_go_back_005eca90();                    // 0042756C
            break;
        }
        const std::uint32_t mode_index = static_cast<std::uint32_t>(d.f_28);
        d.byte_20 = 0;                                             // 00427486
        if (mode_index > 6u) break;
        // Jump table 00427840: modes 0..3 pick list value 3, 0, 1, 2; 4..6 pick 4.
        static constexpr std::int32_t kModeValue[7] = {3, 0, 1, 2, 4, 4, 4};
        void* selector = host.multi_mode_selector_e198b4_44();
        autotest_select_listbox_item_00425eb0(listener_of(selector), host.gui_field(selector, 0x20),
                                              kModeValue[mode_index], false, host);  // 004277C6
        break;
    }
    case 0x09:                                                     // 00427576
        if (static_cast<std::uint32_t>(d.f_28) > 6u) break;
        if (d.byte_20 == 0) {
            autotest_focus_lobby_00425500(host.multi_lobby_e198b4_48(), host);  // 0042758F
        } else {
            void* lobby = host.multi_lobby_e198b4_48();
            if (lobby != nullptr && host.lobby_check_client_compat_005d71b0(lobby)) {
                d.f_28 += 1;                                       // 004275B8
                d.countdown_0c = kStepLongReload;
            }
        }
        break;
    case 0x0A: {                                                   // 004275C7
        const std::int32_t mode_index = d.f_28;
        d.byte_20 = 1;
        if (static_cast<std::uint32_t>(mode_index) > 6u) break;
        void* lobby = host.multi_lobby_e198b4_48();
        if (mode_index >= 4) {                                     // byte map 00427864 -> 004275E4
            d.byte_20 = 1;
            host.lobby_set_setting_row1_005d6350(lobby, mode_index - 4);
        }
        host.select_scene_record_004c6890(d.mission_1c);           // 00427602
        host.listbox_select_index_00a9c7c0(host.lobby_listbox_228(lobby), 0);  // 00427618
        host.lobby_step_setting_005e1cf0(lobby, 0);                // 00427628
        host.lobby_apply_selected_map_005da530(lobby);             // 00427636
        host.lobby_vslot20(lobby, 0.0f);                           // 0042763B FLDZ, CALL [EDX+20h]
        break;
    }
    case 0x0B:                                                     // 00427655
        host.enter_mission_state_004da6c0();
        d.countdown_0c = kStepLongReload;
        break;
    case 0x0C:                                                     // 0042767C, no movie playing
        if (mode == 1) {
            host.lua_run_string_006b8ad0(kClassEnumeration, 2);    // 004276E5
        } else if (mode == 3) {
            d.campaign_18 = kEndedMarker;                          // 0042768E
            d.mission_1c = kEndedMarker;
            autolog_memory_00427040(host);                         // 00427694
        }
        host.toggle_pause_menu_004db030();                         // 0042769F / 0042770A
        break;
    case 0x0D: host.movie_player_skip_004f8960(); break;          // 0042766C
    case 0x0F: {                                                   // 00427714
        // 00427714..0042772F: +24h < 2, mode != 1, and [00F8A304] null or
        // 00425B20 on it <= 0 (JG, signed); each test only after the previous.
        bool first_value = d.f_24 < 2 && mode != 1;
        if (first_value) {
            const std::vector<FlaggedRecord>* records = host.records_00f8a304();
            first_value = records == nullptr ||
                          static_cast<std::int32_t>(count_flagged_records_00425b20(*records)) <= 0;
        }
        void* scr = host.screen_e198c4_d8();
        if (first_value) {
            autotest_select_listbox_item_00425eb0(listener_of(scr), host.gui_field(scr, 0x13C), 4, false,
                                                  host);           // 00427761
            d.f_24 += 1;
        } else {
            autotest_select_listbox_item_00425eb0(listener_of(scr), host.gui_field(scr, 0x13C), 5, false,
                                                  host);           // 004277C6
        }
        break;
    }
    case 0x10: {                                                   // 00427799
        void* scr = host.screen_e198c4_e4();
        autotest_select_listbox_item_00425eb0(listener_of(scr), host.gui_field(scr, 0x20), 4, false, host);
        break;
    }
    default:  // 0Eh and ids above 10h: 004277CB
        break;
    }
    return false;  // 004277D1 XOR AL,AL
}

void terrain_grid_layer_serialize_0041ef90(const TerrainGridLayerRecord& layer, TerrainGridStreamHost& stream) {
    // 0041EFA0..0041EFE5: a local NativeString "TerrainGridLayer" (00CE38D0),
    // vtable +64h(&string, 0), then the string is released.
    stream.write_string_64("TerrainGridLayer", 0);
    stream.write_float_58(layer.half_extent, 0);   // 0041F03A
    stream.write_int_54(layer.dimension, 0);       // 0041F049
    stream.write_float_58(layer.cell_size, 0);     // 0041F05B
    stream.write_float_58(layer.file_scalar, 0);   // 0041F06D
    stream.write_float_58(layer.slope_limit, 0);   // 0041F07F
    // 0041F081..0041F091: checked &grid[0] (begin null or begin == end).
    if (layer.grid.empty()) stream.invalid_parameter_00bf6713();
    // 0041F096..0041F0A6: the count is n*n from +08h, not the vector's size.
    stream.write_bytes_28(layer.grid.data(), layer.dimension * layer.dimension, 0);
}

void terrain_grid_save_layers_0041f130(const std::vector<TerrainGridLayerRecord>& layers,
                                       TerrainGridStreamHost& stream) {
    stream.write_string_64("TerrainGrid", 0);                          // 0041F175 (00CE38E8)
    stream.write_int_54(static_cast<std::int32_t>(layers.size()), 0);  // 0041F1CD, registry+0Ch
    // 0041F1E0..0041F20C: list walk; the checked-iterator 00BF6713 calls in the
    // loop only fire on a corrupt list and are not modelled.
    for (const TerrainGridLayerRecord& layer : layers) {
        terrain_grid_layer_serialize_0041ef90(layer, stream);          // 0041F1FB, ECX = node+8
    }
}

void terrain_grid_layer_deserialize_004239e0(TerrainGridLayerRecord& layer, TerrainGridStreamHost& stream) {
    (void)stream.read_string_60(0);                  // 00423A0F, destroyed unread at 00423AB0
    layer.half_extent = stream.read_float_44(0);     // 00423A24
    layer.dimension = stream.read_int_38(0);         // 00423A32
    layer.cell_size = stream.read_float_44(0);       // 00423A40
    layer.file_scalar = stream.read_float_44(0);     // 00423A4E
    layer.slope_limit = stream.read_float_44(0);     // 00423A5C
    const std::int32_t cells = layer.dimension * layer.dimension;
    layer.grid.resize(static_cast<std::size_t>(static_cast<std::uint32_t>(cells)), 0);  // 00423A6D 004219A0(n*n, 0)
    if (layer.grid.empty()) stream.invalid_parameter_00bf6713();  // 00423A72..00423A82
    stream.read_bytes_24(layer.grid.data(), cells, 0);           // 00423A9D
}

TerrainGridLayerRecord terrain_grid_layer_construct_from_stream_004246e0(TerrainGridStreamHost& stream) {
    // 004246FB vtable 00CE38CC; +1Ch..+24h = 0. The layer keeps no class name
    // (class_name stays empty); 004239E0 reads and drops the stream's.
    TerrainGridLayerRecord layer;
    terrain_grid_layer_deserialize_004239e0(layer, stream);  // 00424717
    return layer;
}

void terrain_grid_cell_to_world_004180a0(const TerrainGridLayerRecord& layer, float out[2],
                                         const float in[2]) noexcept {
    // 004180A7..004180D3: x = in[0] * scale stored to float (004180C9), y =
    // scale * in[1] stored to float (004180D3); the half extent is copied as a
    // float through XMM0. Then out = product - half, stored as floats. Every
    // operation is on two floats and rounded to float right away; the x87
    // intermediate width (>= 2*24+2 bits) makes that double rounding
    // innocuous, so plain float arithmetic gives the same results.
    const float scale = layer.cell_size;
    const float half = layer.half_extent;
    const float x = in[0] * scale;
    const float y = scale * in[1];
    out[0] = x - half;   // 004180DE
    out[1] = y - half;   // 004180E8
}

void write_native_word_array_00429140(NativeBitCursor* cursor, const std::uint16_t* words,
                                      std::uint32_t count, std::uint32_t bits) {
    for (std::uint32_t i = 0; i < count; ++i) {      // 00429170 CMP ESI,EBX / JC (unsigned)
        std::uint32_t temp = words[i];               // 00429158 MOVZX, 00429161 store
        write_native_bits_00428f50(cursor, &temp, bits);
    }
}

void write_voice_data_payload_00429b00(NativeBitCursor* cursor, const VoiceDataPayload& payload) {
    std::uint32_t header[2] = {payload.header_00[0], payload.header_00[1]};  // 00429B09..00429B22
    write_native_bits_00428f50(cursor, header, 0x40);
    // 00429B2B..00429B4A: (end - begin) / 10, 0 for a null begin; kept as a byte.
    const std::uint8_t count = static_cast<std::uint8_t>(payload.frames_08.size());
    write_native_bits_00428f50(cursor, &count, 8);   // 00429B59
    // 00429B70..00429B91: the checked index test cannot fail (count <= size).
    for (std::uint32_t i = 0; i < count; ++i) {
        write_native_bits_00428f50(cursor, payload.frames_08[i].data(), 0x50);  // 00429BA2
    }
}

void read_voice_data_payload_0042a480(NativeBitCursor* cursor, VoiceDataPayload& payload) {
    std::uint32_t header[2];
    read_native_bits_00428bb0(cursor, header, 0x40);  // 0042A494
    payload.header_00[0] = header[0];                 // 0042A4AB
    payload.header_00[1] = header[1];                 // 0042A4B0
    std::uint8_t count = 0;                           // 0042A4B3
    read_native_bits_00428bb0(cursor, &count, 8);     // 0042A4B8
    payload.frames_08.resize(count, std::array<std::uint8_t, 10>{});  // 0042A4DA 0042A380(count, zero frame)
    std::array<std::uint8_t, 10> frame;
    for (std::uint32_t i = 0; i < count; ++i) {
        read_native_bits_00428bb0(cursor, frame.data(), 0x50);  // 0042A4F2
        // 0042A4F7..0042A518: the checked index test cannot fail after the resize.
        payload.frames_08[i] = frame;                           // 0042A52A..0042A53D
    }
}

void singleton_base_destruct_00418280(std::uint32_t& vtable_00, void*& cell_00f8753c) noexcept {
    cell_00f8753c = nullptr;                     // 00418280
    vtable_00 = kSingletonBaseVtable_00ce3818;   // 0041828A
}

void singleton_base_destruct_00425a80(std::uint32_t& vtable_00, void*& cell_00e17664) noexcept {
    cell_00e17664 = nullptr;                     // 00425A80
    vtable_00 = kSingletonBaseVtable_00ce3818;   // 00425A8A
}

std::int32_t native_string_to_long_00425bd0(const char* data_04) noexcept {
    return static_cast<std::int32_t>(std::atol(data_04 != nullptr ? data_04 : ""));  // 00425BDD
}

NativeLegacySboStringStorage& native_legacy_sbo_string_assign_cstring_00429fd0(
    NativeLegacySboStringStorage& string, const char* text) {
    // 00429FE0..00429FE9: the strlen loop; 00429FED tail call with ECX unchanged.
    return native_legacy_sbo_string_assign_counted_00408720(string, text,
                                                            static_cast<std::uint32_t>(std::strlen(text)));
}

}  // namespace bsp::seg1_cc13
