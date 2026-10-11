// HUD and GUI screens of partition segments 16, 28 and 4 (packet cc13_gui5_screens3).
// Evidence and uncertainty: docs/CC13_GUI_SCREENS_3.md. Names are hypotheses; each routine
// carries its address, original ABI and the listing range it was checked against. None of
// this is wired into the game host; it is reconstructed and build-tested only.

#include "bsp/gui_screens_3.hpp"

#include <cctype>

namespace bsp {
namespace {

bool equals_insensitive(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

// The x87 sequences widen these float constants to double before use (00CEE370, 00CEE368).
constexpr double kPauseTitleGap = static_cast<double>(0.027F);   // 00CEE370
constexpr double kPauseButtonGap = static_cast<double>(0.037F);  // 00CEE368
constexpr double kPauseFramePad = static_cast<double>(0.15F);    // 00CE6618
constexpr double kNormalFramePad = 0.006;                        // 00CEE360
constexpr double kTextWidthScale = 960.0;                        // 00CEC380
constexpr double kNoTimeoutShift = static_cast<double>(0.025F);  // 00D7A380
constexpr float kNormalFrameWidthTimed = 0.3F;                   // 00CE69C8
constexpr float kNormalFrameWidthUntimed = 0.25F;                // 00CE3868

// 0054E08E..0054E0BB / 0054E259..0054E286: -1 asks for the subtitle reading time.
float hint_seconds(HudHintsHost& host, std::uintptr_t text, std::int32_t seconds) {
    if (seconds != -1) return static_cast<float>(seconds);
    const float once = static_cast<float>(host.text_line_count_ec(text)) * host.subtitle_seconds_per_line_7c() +
                       host.subtitle_seconds_base_80();
    return once + once;
}

}  // namespace

std::string_view hud_hints_next_basic_step(std::string_view active) {
    if (equals_insensitive(active, "BASICPLANE")) return "BASICPLANE2";   // 00CEE3C0 -> 00CEE3B4
    if (equals_insensitive(active, "BASICPLANE2")) return "BASICPLANE3";  // 00CEE3B4 -> 00CEE3A8
    if (equals_insensitive(active, "BASICSHIP")) return "BASICSHIP2";     // 00CEE39C -> 00CEE390
    if (equals_insensitive(active, "BASICSUB")) return "BASICSUB2";       // 00CEE384 -> 00CEE378
    return {};
}

// 0054D860..0054D972. Base register 004F71D0, page load into +1Ch, field reset, the layout
// virtual, then the sizes and position the show/grow code animates from.
void hud_hints_register_0054d860(HudHintsState& screen, HudHintsHost& host) {
    host.register_front_end_screen_004f71d0();
    screen.page_1c = host.load_page_00aa5840(kHudHintsPage);  // 0054D8C9
    screen.remaining_54 = 0.0F;                                // 0054D900
    screen.pausing_08 = false;                                 // 0054D90F
    screen.byte_09 = false;
    screen.grow_seconds_58 = kHudHintsGrowSeconds;             // 0054D915
    screen.persistent_50 = false;                              // 0054D91A
    host.screen_layout_14();                                   // 0054D91D CALL EDX (vtable +14h)
    screen.pause_frame_size_60 = host.widget_size_00aa6740(screen.pause_frame_48);    // 0054D922
    screen.normal_frame_size_68 = host.widget_size_00aa6740(screen.normal_frame_4c);  // 0054D935
    screen.normal_text_pos_70 = host.widget_local_position_00aa67f0(screen.normal_text_34);  // 0054D948
    screen.registered_04 = true;                               // 0054D962
}

// 0054D980..0054DE5F. Parents checked in the listing: page for the two groups, pause group
// for 28h..48h, normal group for 34h, 4Ch and 44h (in that order, 0054DD57..0054DE4E).
void hud_hints_layout_0054d980(HudHintsState& screen, HudHintsHost& host) {
    screen.pause_group_20 = host.find_child_00aa7e00(screen.page_1c, "hints_pause_Group");             // 0054D9E6
    screen.pause_title_28 = host.find_child_00aa7e00(screen.pause_group_20, "hints_title_Text");        // 0054DA59
    screen.pause_text_2c = host.find_child_00aa7e00(screen.pause_group_20, "hints_description_Text");   // 0054DAC9
    screen.pause_button_38 = host.find_child_00aa7e00(screen.pause_group_20, "hints_button_Text");      // 0054DB39
    screen.pause_icon_3c = host.find_child_00aa7e00(screen.pause_group_20, "hints_1_Icon");             // 0054DBA9
    screen.pause_icon_40 = host.find_child_00aa7e00(screen.pause_group_20, "hints_2_Icon");             // 0054DC19
    screen.pause_frame_48 = host.find_child_00aa7e00(screen.pause_group_20, "hints_FrameBox");          // 0054DC89
    screen.normal_group_24 = host.find_child_00aa7e00(screen.page_1c, "hints_normal_Group");           // 0054DCF9
    screen.normal_text_34 = host.find_child_00aa7e00(screen.normal_group_24, "hints_description_Text"); // 0054DD69
    screen.normal_frame_4c = host.find_child_00aa7e00(screen.normal_group_24, "hints_FrameBox");        // 0054DDD9
    screen.normal_icon_44 = host.find_child_00aa7e00(screen.normal_group_24, "hints_1_Icon");           // 0054DE49
}

// 0054E380..0054E3BC (tail jump to 0068EA00 with ECX = tracker).
void hud_hints_enter_0054e380(HudHintsState& screen, HudHintsHost& host) {
    host.widget_show_34(screen.page_1c, true);
    host.widget_show_34(screen.pause_group_20, false);
    host.game_hint_pause_004d94f0(false);   // 0054E3A3
    host.widget_show_34(screen.normal_group_24, false);
    host.tracker_clear_queue_0068ea00();    // 0054E3BC
}

// 0054E3D0..0054E43A.
void hud_hints_exit_0054e3d0(HudHintsState& screen, HudHintsHost& host) {
    host.widget_show_34(screen.pause_group_20, false);
    host.game_hint_pause_004d94f0(false);   // 0054E3E8
    host.tracker_clear_list_48_004d05e0();  // 0054E3F7, ECX = tracker+48h
    host.tracker_clear_active_name_1c();    // 0054E405.., 0041DD40(0, 0) then copy of 00CE3A0C
    host.widget_show_34(screen.normal_group_24, false);
}

// 0054E370..0054E37D: 004D6790(list, &screen+1Ch), RET 4.
void hud_hints_collect_0054e370(HudHintsHost& host, std::uintptr_t list, std::uintptr_t page_slot) {
    host.collect_page_004d6790(list, page_slot);
}

// 0054DE90..0054E012. Starts cue 0 or 1 unless its tracked sound slot is already set
// (0054DEB0 NEG/SBB/TEST: any non-null slot skips). The sound class is looked up once
// (guard bit 1 of 00E19318) and kept in 00E19314; the host owns that cache.
void hud_hints_play_cue_0054de90(std::int32_t cue, HudHintsHost& host) {
    if (host.cue_playing(cue)) return;
    host.cue_start_00a7e490(cue, kHudHintsCueClass, kHudHintsCueType);  // 0054DF6E, ECX = 00F8BBD8
}

// 0054E020..0054E362. Gate: in a session (game+1FE4h non-zero) without +218Ch, a set +61Fh
// suppresses every hint (0054E028..0054E044).
void hud_hints_show_0054e020(HudHintsState& screen, HudHintsHost& host, bool pausing,
                             const std::string& text, const std::string& title, std::int32_t seconds) {
    if (host.game_session_kind_1fe4() != 0 && !host.game_byte_218c() && host.game_byte_61f()) return;
    screen.persistent_50 = seconds == 0;           // 0054E060
    screen.grow_seconds_58 = kHudHintsGrowSeconds; // 0054E069
    screen.pausing_08 = pausing;                   // 0054E073
    screen.byte_09 = false;
    screen.grow_elapsed_5c = 0.0F;                 // 0054E07A
    if (pausing) {
        host.text_set_source_00abaed0(screen.pause_text_2c, text);    // 0054E089
        screen.remaining_54 = hint_seconds(host, screen.pause_text_2c, seconds);
        host.text_set_source_00abaed0(screen.pause_title_28, title);  // 0054E0D3
        host.widget_show_34(screen.pause_group_20, true);
        host.game_hint_pause_004d94f0(true);                          // 0054E0EC
        host.widget_show_34(screen.pause_text_2c, false);
        host.widget_show_34(screen.pause_icon_3c, false);
        host.widget_show_34(screen.pause_icon_40, false);
        host.widget_show_34(screen.pause_button_38, false);
        host.widget_show_34(screen.pause_title_28, false);
        // 0054E12D..0054E15E: +60h is reloaded unchanged; +64h becomes the text height + 0.15.
        screen.pause_frame_size_60[1] =
            static_cast<float>(static_cast<double>(host.text_normalized_height_00ab6bd0(screen.pause_text_2c)) + kPauseFramePad);
        // 0054E164..0054E1A9: size = target * elapsed / grow time (elapsed is 0 here).
        const float t = screen.grow_elapsed_5c;
        const float dur = screen.grow_seconds_58;
        host.widget_set_size_58(screen.pause_frame_48, {screen.pause_frame_size_60[0] * t / dur,
                                                        t * screen.pause_frame_size_60[1] / dur});
        const std::array<float, 3> frame = host.widget_local_position_00aa67f0(screen.pause_frame_48);  // 0054E1AE
        const double y = frame[1];
        const double h1 = host.text_normalized_height_00ab6bd0(screen.pause_text_2c);  // 0054E1CB
        host.widget_set_local_xy_00aa7d00(screen.pause_title_28, frame[0],
                                          static_cast<float>(y - (h1 * 0.5 + kPauseTitleGap)));  // 0054E1F9
        const double h2 = host.text_normalized_height_00ab6bd0(screen.pause_text_2c);  // 0054E201
        host.widget_set_local_xy_00aa7d00(screen.pause_button_38, frame[0],
                                          static_cast<float>(h2 * 0.5 + static_cast<double>(frame[1]) + kPauseButtonGap));  // 0054E22F
        hud_hints_play_cue_0054de90(1, host);                         // 0054E238
        return;
    }
    host.text_set_source_00abaed0(screen.normal_text_34, text);       // 0054E248
    host.widget_show_34(screen.normal_text_34, true);
    screen.remaining_54 = hint_seconds(host, screen.normal_text_34, seconds);
    // 0054E294..0054E2E2: an untimed hint shifts the text left by 0.025 and uses the narrower pad.
    float pad = kNormalFrameWidthTimed;
    float x = screen.normal_text_pos_70[0];
    if (screen.remaining_54 == 0.0F) {
        pad = kNormalFrameWidthUntimed;
        x = static_cast<float>(static_cast<double>(screen.normal_text_pos_70[0]) - kNoTimeoutShift);
    }
    host.widget_set_local_xy_00aa7d00(screen.normal_text_34, x, screen.normal_text_pos_70[1]);
    const float width = static_cast<float>(static_cast<double>(host.text_width_114(screen.normal_text_34)) / kTextWidthScale);
    screen.normal_frame_size_68[0] = width + pad;  // 0054E2FA..0054E31D
    screen.normal_frame_size_68[1] =
        static_cast<float>(static_cast<double>(host.text_normalized_height_00ab6bd0(screen.normal_text_34)) + kNormalFramePad);
    host.widget_set_size_58(screen.normal_frame_4c, screen.normal_frame_size_68);  // 0054E32E
    host.widget_local_position_00aa67f0(screen.normal_frame_4c);                  // 0054E333, result unused
    host.widget_show_34(screen.normal_group_24, true);
    host.widget_show_34(screen.normal_icon_44, false);
    if (seconds != 0) hud_hints_play_cue_0054de90(0, host);                      // 0054E358
}

// 0054E440..0054E60D. Ghidra drops the TriggerHint tail as unreachable; the listing keeps it
// (0054E59B: CMP [length], 0; 0054E5AE CALL 00690FD0 with forced = 0).
void hud_hints_advance_basic_step_0054e440(HudHintsState& screen, HudHintsHost& host) {
    screen.pausing_08 = false;
    screen.byte_09 = false;
    host.widget_show_34(screen.pause_group_20, false);
    host.game_hint_pause_004d94f0(false);                  // 0054E476
    screen.remaining_54 = 0.0F;
    host.input_action_reset_00a91e20(kHudHintsResetAction);    // 0054E491, record +2FD0h
    host.input_action_reset_00a91e20(kHudHintsConfirmAction);  // 0054E4A4, record +DE0h
    const std::string active = host.tracker_active_hint_0068e5b0();  // 0054E4EB
    const std::string next(hud_hints_next_basic_step(active));
    host.tracker_finish_active_0068e420();                 // 0054E596
    if (!next.empty()) host.tracker_trigger_hint_00690fd0(next, false);
}

// 0054E610..0054E6AA. Called by HudFormation Enter/Exit and eight other sites.
void hud_hints_dismiss_0054e610(HudHintsState& screen, HudHintsHost& host) {
    screen.remaining_54 = 0.0F;
    if (host.widget_shown_38(screen.pause_group_20)) {
        host.widget_show_34(screen.pause_group_20, false);
        host.game_hint_pause_004d94f0(false);              // 0054E63D
        if (host.game_cinematic_hidden_634()) {
            host.game_set_cinematic_004cd0f0(false, false, true);       // 0054E657
            host.input_context_level_00a933f0(kHudHintsInputContext, 0);  // 0054E667
            host.game_004d9830(true);                                   // 0054E674
        }
    } else if (host.widget_shown_38(screen.normal_group_24)) {
        host.widget_show_34(screen.normal_group_24, false);
    }
    host.tracker_finish_active_0068e420();
}

// 0054E6B0..0054EC87.
void hud_hints_update_0054e6b0(HudHintsState& screen, HudHintsHost& host, float dt) {
    if (host.tracker_active_hint_0068e5b0().empty()) return;  // 0054E6E3: length field tested
    const float dur = screen.grow_seconds_58;
    if (screen.grow_elapsed_5c < dur) {                       // 0054E727 FCOMI, JB
        const float t = screen.grow_elapsed_5c + dt;
        screen.grow_elapsed_5c = t;
        if (!screen.pausing_08) {
            const auto& s = screen.normal_frame_size_68;
            host.widget_set_size_58(screen.normal_frame_4c, {(t * s[0] * 0.5F) / dur + s[0] * 0.5F,
                                                             (t * 0.5F * s[1]) / dur + s[1] * 0.5F});
        } else {
            const auto& s = screen.pause_frame_size_60;
            host.widget_set_size_58(screen.pause_frame_48, {(t * s[0] * 0.5F) / dur + s[0] * 0.5F,
                                                            (t * 0.5F * s[1]) / dur + s[1] * 0.5F});
        }
    } else if (!screen.pausing_08) {
        // 0054E801..: grown; the normal branch does not latch +5Ch, so it repeats every frame.
        host.widget_show_34(screen.normal_text_34, true);
        if (screen.remaining_54 != 0.0F) host.widget_show_34(screen.normal_icon_44, true);
        host.widget_set_size_58(screen.normal_frame_4c, screen.normal_frame_size_68);
        host.cue_release_0054d510(0);                         // 0054E83B, slot +14h
    } else {
        host.widget_set_size_58(screen.pause_frame_48, screen.pause_frame_size_60);  // 0054E748
        screen.grow_elapsed_5c = screen.grow_seconds_58;      // 0054E750
        host.widget_show_34(screen.pause_text_2c, true);
        host.widget_show_34(screen.pause_icon_3c, true);
        host.widget_show_34(screen.pause_icon_40, true);
        const std::string key(host.pad_prompts_00f88a30() ? kHudHintsPressPad : kHudHintsPressKey);  // 0054E782
        host.text_set_source_00abaed0(screen.pause_button_38, key);  // 0054E7AA
        host.widget_show_34(screen.pause_button_38, true);
        host.widget_show_34(screen.pause_title_28, true);
        host.cue_release_0054d510(1);                         // 0054E7C9, slot +18h
    }

    bool advance = false;
    if (screen.pausing_08) {                                  // 0054E99F
        if (host.game_effective_mode_004bca50() == kHudHintsTutorialGameMode) {
            // 0054E9BD..0054EB95: the tutorial mode freezes the scene once the box is grown and
            // waits for the confirm action (or profile query 0Eh) before chaining the next step.
            if (!host.game_cinematic_hidden_634() && screen.grow_elapsed_5c == screen.grow_seconds_58) {
                host.game_set_cinematic_004cd0f0(true, true, true);        // 0054E9E3
                host.input_context_level5_004c5140(kHudHintsInputContext); // 0054E9F0
            }
            if (!host.input_action_pressed_00535ee0(kHudHintsConfirmAction) &&
                !host.profile_hints_query_00427190(kHudHintsProfileQuery)) {
                return;
            }
            host.game_set_cinematic_004cd0f0(false, false, true);           // 0054EA28
            host.game_hint_pause_004d94f0(true);                            // 0054EA35
            host.input_context_level_00a933f0(kHudHintsInputContext, 0);    // 0054EA45
            host.game_004d9830(true);                                       // 0054EA52
            screen.pausing_08 = false;
            screen.byte_09 = false;
            host.widget_show_34(screen.pause_group_20, false);
            host.game_hint_pause_004d94f0(false);                           // 0054EA73
            screen.remaining_54 = 0.0F;
            const std::string active = host.tracker_active_hint_0068e5b0(); // 0054EAA2
            const std::string next(hud_hints_next_basic_step(active));
            host.tracker_finish_active_0068e420();                          // 0054EB17
            if (!next.empty()) host.tracker_trigger_hint_00690fd0(next, false);  // 0054EB31
            return;
        }
        // 0054EB9E..0054EBD1: the confirm action record tested inline (same test as 00535EE0).
        advance = host.input_action_pressed_00535ee0(kHudHintsConfirmAction);
    }
    if (!advance) advance = host.profile_hints_query_00427190(kHudHintsProfileQuery);  // 0054EBD3
    if (advance) hud_hints_advance_basic_step_0054e440(screen, host);                  // 0054EBE7

    if (screen.persistent_50) return;                         // 0054EBEC
    const float left = screen.remaining_54;
    screen.remaining_54 = left > 0.0F ? left - dt : 0.0F;     // 0054EBF6..0054EC15
    if (screen.remaining_54 != 0.0F) return;
    if (host.widget_shown_38(screen.pause_group_20)) {
        host.widget_show_34(screen.pause_group_20, false);
        host.game_hint_pause_004d94f0(false);                 // 0054EC4A
    } else if (host.widget_shown_38(screen.normal_group_24)) {
        host.widget_show_34(screen.normal_group_24, false);
    }
    host.tracker_finish_active_0068e420();                    // 0054EC72
}

namespace {

// 006912F4..006914AB and the faction arms 006914B0..00691A38: the "<description>_pc" key wins
// on PC prompts when the localisation table knows it.
void hint_prefer_pc_key(HintTrackerHost& host, std::string& description) {
    if (host.pad_prompts_00f88a30()) return;
    std::string pc = description + std::string(kHintPcSuffix);  // 004261A0
    if (host.localised_key_exists(pc)) description = pc;         // 00449AF0 true -> 00425F40
}

}  // namespace

// 00690FD0..006921A1, read whole from the listing (local/g5_l_00690fd0.txt). Steady frame: the
// new definition copy at S+68h, the active/previous definition copy at S+190h, title S+38h,
// text S+24h, Lua command S+1Ch.
void hint_tracker_trigger_00690fd0(HintTrackerState& tracker, HintTrackerHost& host, const std::string& name,
                                   bool forced) {
    // 00690FF4..0069101E: +04h forces one trigger past the hints setting.
    if (!tracker.force_next_04) {
        const std::int32_t setting = host.hints_setting_00f8898c();
        if (setting == 0) return;
        if (!forced && setting == 1) return;
    } else {
        tracker.force_next_04 = false;
    }
    const HintDefinition* found = host.find_definition_0068e450(name);  // 00691037
    if (found == nullptr) return;
    const HintDefinition def = *found;                                  // 0069106A (005FE010)

    // 0069106F..00691123: an active hint either yields to a critical one, queues this one, or
    // swallows it.
    if (!tracker.active_60.empty()) {
        const HintDefinition* active = host.find_definition_0068e450(tracker.active_60);  // 0069108E (005FFB20)
        const bool active_critical = active != nullptr && active->critical;
        const bool active_flag = active != nullptr && active->loader_flag_50;
        if (active_critical || !def.critical) {
            if ((forced && def.loader_flag_50 && active_flag) || host.hud_screen_e4_busy()) {
                host.queue_push_back_0068eb90(name);  // 006910EE
                return;
            }
            if (!def.loader_flag_50 && active_flag) return;  // 0069110C..00691112
        }
    }
    // 00691128..0069122C: storing rules that refuse a hint already seen.
    if (equals_insensitive(def.storing_rule, kHintRuleUnique) && host.profile_has_seen_007f8e00(name)) return;
    if (equals_insensitive(def.storing_rule, kHintRuleMissionUnique) && host.mission_seen_contains_48(name)) return;
    // 00691231..006912B6: a cooldown rule refuses while the key is cooling down, else starts it.
    if (equals_insensitive(def.storing_rule, kHintRuleCooldown) && def.cooldown_time != 0) {
        if (host.cooldown_contains_0068e4c0(name)) return;
        host.cooldown_set_00444be0(name, static_cast<float>(def.cooldown_time));
    }

    // 006912BA..00691A38: text selection. The generic description wins; otherwise the local
    // side's nation (0 allied, 1 japanese) picks its fields, then the player-slot row. When
    // neither applies but the other nation has a description, the hint is not for this side.
    std::string title;
    std::string text;
    std::string lua_command;
    if (!def.description.empty()) {
        title = def.title;
        text = def.description;
        hint_prefer_pc_key(host, text);
        lua_command = def.lua_command;
    } else {
        const std::int32_t nation = host.local_side_nation_28();
        const std::int32_t slot = host.local_player_slot_18ec();
        const HintTitleDescRow* row = slot >= 0 && slot < static_cast<std::int32_t>(def.title_desc_list.size())
                                          ? &def.title_desc_list[static_cast<std::size_t>(slot)]
                                          : nullptr;
        if (nation == 0 || nation == 1) {
            const bool allied = nation == 0;
            const std::string& own_desc = allied ? def.allied_description : def.japanese_description;
            const std::string& other_desc = allied ? def.japanese_description : def.allied_description;
            if (!own_desc.empty()) {
                title = allied ? def.allied_title : def.japanese_title;
                text = own_desc;
                hint_prefer_pc_key(host, text);
                lua_command = allied ? def.allied_lua_command : def.japanese_lua_command;
            } else if (row != nullptr && !row->description.empty()) {
                title = row->title;
                text = row->description;
                hint_prefer_pc_key(host, text);
                lua_command = row->lua_command;
            } else if (!other_desc.empty()) {
                return;  // 006917BE / 00691A3D -> 006917CB
            }
        }
    }

    // 00691704..00691AA1: the previous timed hint counts as seen once less than a quarter of its
    // seconds remained; it is then stored under its rule and an identical re-trigger is dropped.
    if (!tracker.last_timed_1c.empty()) {
        const HintDefinition* previous = host.find_definition_0068e450(tracker.last_timed_1c);  // 00691724
        const float left = static_cast<float>(host.hints_seconds_left_0054d3c0());              // 0069174A, FILD
        if (previous != nullptr &&
            static_cast<double>(tracker.last_timed_seconds_24) * kHintSeenFraction > static_cast<double>(left)) {
            if (equals_insensitive(previous->storing_rule, kHintRuleUnique)) {
                host.profile_mark_seen_007fae20(tracker.last_timed_1c);   // 00691790
                if (equals_insensitive(tracker.last_timed_1c, name)) return;
            } else if (equals_insensitive(previous->storing_rule, kHintRuleMissionUnique)) {
                host.mission_seen_push_back_48(tracker.last_timed_1c);    // 00691A62
                if (equals_insensitive(tracker.last_timed_1c, name)) return;
            }
        }
    }

    // 00691AA3..00691B35: replace whatever is shown, run the hint's Lua command, show it.
    host.hints_dismiss_0054e610();
    if (!lua_command.empty()) {
        const bool failed = host.lua_run_string_006b8ad0(lua_command) != 0;  // 00691ADC
        host.diagnostic_lua_result_004254b0(failed, lua_command);
    }
    if (!def.hidden) host.hints_show_0054e020(def.critical, text, title, def.duration);  // 00691B35

    // 00691B3A..00691EB9: history keyed by Type; mission hints sort first under "AAAAAA".
    std::string key = def.type;
    if (equals_insensitive(def.type, kHintMissionType)) key = std::string(kHintMissionHistoryPrefix) + key;  // 0043C130
    if (!host.history94_has_key(key)) {
        host.history_assign_single(key, name);  // 00691D52..00691EB9
    } else {
        if (!host.history94_erase_name(key, name)) host.history7c_push_back(key, name);  // 00691CD6
        host.history94_push_back(key, name);    // 00691D17
    }

    // 00691EBD..006920AE: bookkeeping by kind.
    if (def.critical || def.duration != 0 || def.hidden) {
        if (!def.mission_specific) {
            host.profile_mark_seen_007fae20(name);  // 00691EF5
        } else {
            host.touch_recent_0068ea70(name);       // 00691F3C, name passed by value
        }
        if (def.critical || def.hidden) {
            if (equals_insensitive(def.storing_rule, kHintRuleUnique)) {
                host.profile_mark_seen_007fae20(name);           // 00692023
            } else if (equals_insensitive(def.storing_rule, kHintRuleMissionUnique)) {
                host.mission_seen_push_back_48(name);            // 0069205E
            }
            tracker.last_timed_1c.clear();                       // 00692089
            tracker.last_timed_seconds_24 = 0.0F;
        } else {
            tracker.last_timed_1c = name;                        // 00691FAC
            tracker.last_timed_seconds_24 = static_cast<float>(host.hints_seconds_left_0054d3c0());  // 00691FE2
        }
    } else {
        tracker.last_timed_1c.clear();                           // 00691F5F
        tracker.last_timed_seconds_24 = 0.0F;
        tracker.persistent_28 = name;                            // 00691F6F
    }
    tracker.active_60 = name;                                    // 006920AE
}

}  // namespace bsp
