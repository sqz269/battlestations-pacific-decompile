// HUD and GUI screens and the hint system (packet cc13_gui5_screens3 and its follow-ups):
//   - the hints screen 34h (hud_hints_*, 0054D860..0054E6B0), with hud_hints_show_0054e020;
//   - the hint tracker: TriggerHint 00690FD0, the stored-hint and history helpers
//     (00690B20, 00690C30, 0068EE00, 00690CA0, 00690CD0), the loader 0068F030 and the
//     reload 006921B0;
//   - the idle camera screen 2Ch (0054EFC0..00550410) and its shot loader 00551440;
//   - GUI parking 004D6480, the hint/system pause setters 004D94F0/004D95F0, the GUI layer
//     visibility 004D9830 and the scene cloud-area loader 004E7220;
//   - the HintSystem Lua wrappers (008D1F50..008D2BB0, 0088CEC0).
// Evidence and uncertainty: docs/CC13_GUI_SCREENS_3.md. Names are hypotheses; each routine
// carries its address, original ABI and the listing range it was checked against.
// Wiring: game_hint_passes.cpp (packet cc15-hint2, docs/CC15_OBJECT_PROFILE_HINTS.md section
// 6) drives the hints screen, TriggerHint and 004D9830 from the game host, and
// game_profile_owner.cpp calls the hint loader, history rebuild and reload. The idle camera,
// parking, pause setters, cloud loader, Lua wrappers and stored-hint helpers have no callers
// outside this file (frontend_*_cc13.cpp reach 004D95F0 / 004D6480 through their own host
// interfaces, not these bodies).
// Switches: kPc24HudGuiCc13Bound (OFF, include/bsp/pc24_hudgui_cc13.hpp, packet
// pc24w1-hudgui-2) rounds 0054E020's pause title and button y per op (0054E1D0..E1E2,
// 0054E206..E216); its OFF arm is the earlier double chain.

#include "bsp/gui_screens_3.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_hudgui_cc13.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

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
        float title_y = 0.0F;
        if constexpr (kPc24HudGuiCc13Bound) {
            // PC24: 0054E1D0 FMUL 0.5, 0054E1DC FADD gap, 0054E1E2 FSUBR y, each rounded.
            using namespace cc13x87;
            title_y = static_cast<float>(x87_sub24(y, x87_add24(x87_mul24(h1, 0.5), kPauseTitleGap)));
        } else {
            title_y = static_cast<float>(y - (h1 * 0.5 + kPauseTitleGap));
        }
        host.widget_set_local_xy_00aa7d00(screen.pause_title_28, frame[0], title_y);  // 0054E1F9
        const double h2 = host.text_normalized_height_00ab6bd0(screen.pause_text_2c);  // 0054E201
        float button_y = 0.0F;
        if constexpr (kPc24HudGuiCc13Bound) {
            // PC24: 0054E206 FMUL 0.5, 0054E212 FADD frame y, 0054E216 FADD gap, each rounded.
            using namespace cc13x87;
            button_y = static_cast<float>(
                x87_add24(x87_add24(x87_mul24(h2, 0.5), frame[1]), kPauseButtonGap));
        } else {
            button_y = static_cast<float>(h2 * 0.5 + static_cast<double>(frame[1]) + kPauseButtonGap);
        }
        host.widget_set_local_xy_00aa7d00(screen.pause_button_38, frame[0], button_y);  // 0054E22F
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
            host.game_set_gui_layers_visible_004d9830(true);                                   // 0054E674
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
            host.game_set_gui_layers_visible_004d9830(true);                                       // 0054EA52
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

namespace {

// 0054FA44..005501D2 (one loop per nation, the japanese one at 0054FE20): resolve both names,
// observe each found entity once, enable the shot only when both were found.
void idle_camera_resolve_shots(IdleCameraHost& host, std::vector<IdleCameraShot>* shots) {
    if (shots == nullptr) return;
    for (IdleCameraShot& shot : *shots) {
        shot.eye_entity = host.find_entity_00925a90(shot.eye_name);  // 0054FB1F
        if (shot.eye_entity == 0) {
            shot.enabled = false;
            continue;
        }
        if (!host.observer_registered_00694af0(shot.eye_entity)) host.observer_register_00694a60(shot.eye_entity);
        shot.target_entity = host.find_entity_00925a90(shot.target_name);
        if (shot.target_entity == 0) {
            shot.enabled = false;
            continue;
        }
        if (!host.observer_registered_00694af0(shot.target_entity)) host.observer_register_00694a60(shot.target_entity);
        shot.enabled = true;
    }
}

void idle_camera_unobserve_shots(IdleCameraHost& host, std::vector<IdleCameraShot>* shots) {
    if (shots == nullptr) return;
    for (const IdleCameraShot& shot : *shots) {
        if (shot.eye_entity != 0 && host.observer_registered_00694af0(shot.eye_entity)) {
            host.observer_unregister_006952a0(shot.eye_entity);
        }
        if (shot.target_entity != 0 && host.observer_registered_00694af0(shot.target_entity)) {
            host.observer_unregister_006952a0(shot.target_entity);
        }
    }
}

void idle_camera_forget_entity(std::vector<IdleCameraShot>* shots, std::uintptr_t observed) {
    if (shots == nullptr) return;
    for (IdleCameraShot& shot : *shots) {
        if (shot.eye_entity == observed) {
            shot.eye_entity = 0;
            shot.enabled = false;
        }
        if (shot.target_entity == observed) {
            shot.target_entity = 0;
            shot.enabled = false;
        }
    }
}

}  // namespace

// 0054F900..005501E9. The 900-byte camera node is made once and kept across Enter calls.
void idle_camera_enter_0054f900(IdleCameraState& screen, IdleCameraHost& host) {
    if (screen.camera_1c == 0) {
        screen.camera_1c = host.create_camera_00432750();      // 0054F932..0054F96A
        host.camera_attach_00923870(screen.camera_1c);         // 0054F9EA, identity matrix
        host.observer_register_00694a60(screen.camera_1c);     // 0054F9F5
    }
    host.camera_copy_game_view_19fc(screen.camera_1c);         // 0054F9FA..0054FA20
    screen.timer_20 = kIdleCameraStartTimer;                   // 0054FA30
    screen.shot_24 = 0;
    host.game_set_camera_mover_004bc410(screen.camera_1c);     // 0054FA3F
    const std::int32_t mode = host.game_effective_mode_004bca50();
    idle_camera_resolve_shots(host, host.shots(0, mode));
    idle_camera_resolve_shots(host, host.shots(1, mode));
}

// 0054EFC0..0054F3D9: stops observing every shot entity; the entity pointers, the camera and
// the camera mover are left as they are.
void idle_camera_exit_0054efc0(IdleCameraState&, IdleCameraHost& host) {
    const std::int32_t mode = host.game_effective_mode_004bca50();
    idle_camera_unobserve_shots(host, host.shots(0, mode));
    idle_camera_unobserve_shots(host, host.shots(1, mode));
}

// 0054F3E0..0054F735: the camera itself, else every shot naming the released entity.
void idle_camera_on_released_0054f3e0(IdleCameraState& screen, IdleCameraHost& host, std::uintptr_t observed) {
    const std::int32_t mode = host.game_effective_mode_004bca50();
    if (observed == screen.camera_1c) {
        screen.camera_1c = 0;
        return;
    }
    idle_camera_forget_entity(host.shots(0, mode), observed);
    idle_camera_forget_entity(host.shots(1, mode), observed);
}

// 00550410..00550662. The steps run in the native order; the shot list must hold at least one
// enabled shot or the native search loops forever (00550560 / 005505A0 have no exit).
void idle_camera_update_00550410(IdleCameraState& screen, IdleCameraHost& host, float dt) {
    if (screen.camera_1c == 0) return;
    screen.timer_20 = dt + screen.timer_20;                    // 00550425..0055042F
    const std::int32_t mode = host.game_effective_mode_004bca50();
    std::vector<IdleCameraShot>* shots = host.shots(host.local_side_nation_28() == 0 ? 0 : 1, mode);
    if (shots == nullptr || shots->empty()) return;            // 00550464 / 0055049D
    const std::int32_t count = static_cast<std::int32_t>(shots->size());
    auto& v = *shots;
    auto enabled = [&v](std::int32_t i) { return v[static_cast<std::size_t>(i)].enabled; };
    bool step_next = screen.timer_20 > kIdleCameraShotSeconds || !enabled(screen.shot_24);  // 005504C1..005504E1
    if (!step_next && host.input_action_pressed_004c43c0(kIdleCameraNextAction)) {
        step_next = !host.hud_bc_blocks_input_006529e0();      // 00550506
    }
    if (step_next) {
        do {
            screen.shot_24 = (screen.shot_24 + 1) % count;     // 0055057E..005505B9
        } while (!enabled(screen.shot_24));
        screen.timer_20 = 0.0F;
    } else if (host.input_action_pressed_004c43c0(kIdleCameraPrevAction) && !host.hud_bc_blocks_input_006529e0()) {
        do {
            screen.shot_24 = (count - 1 + screen.shot_24) % count;  // 00550540..0055057A
        } while (!enabled(screen.shot_24));
        screen.timer_20 = 0.0F;
    }
    // 005505C3..00550654: eye = shot+0 entity, target = shot+4 entity, world up (0, 1, 0).
    const IdleCameraShot& shot = v[static_cast<std::size_t>(screen.shot_24)];
    if (!host.entity_pose_fresh_c8(shot.target_entity)) host.entity_refresh_pose_00414db0(shot.target_entity);
    if (!host.entity_pose_fresh_c8(shot.eye_entity)) host.entity_refresh_pose_00414db0(shot.eye_entity);
    const IdleCameraHost::Matrix view =
        host.look_at_00b63f10(host.entity_position_fc(shot.eye_entity), host.entity_position_fc(shot.target_entity),
                              {0.0F, 1.0F, 0.0F});
    host.camera_set_world_004134f0(screen.camera_1c, host.invert_affine_00b63b30(view));
    host.invalidate_subtree_0042ed50(screen.camera_1c);
}

// 00690B20..00690C29. A known hint is stored under its rule unless it was already shown.
void hint_tracker_add_stored_00690b20(HintStoreHost& host, const std::string& name) {
    const HintDefinition* def = host.find_definition_0068e450(name);  // 00690B5A
    if (def == nullptr) return;
    if (equals_insensitive(def->storing_rule, kHintRuleUnique) && !host.has_been_shown_0068e600(name)) {
        host.profile_mark_seen_007fae20(name);                        // 00690BC9
    } else if (equals_insensitive(def->storing_rule, kHintRuleMissionUnique) && !host.has_been_shown_0068e600(name)) {
        host.mission_seen_push_back_48(name);                         // 00690BF5
    }
}

// 00690C30..00690C99. Forget a shown hint: reset the unit-class tracker when it holds the
// player unit's class, then drop the name from the profile or else from the mission list.
void hint_tracker_remove_stored_00690c30(HintStoreState& tracker, HintStoreHost& host, const std::string& name) {
    if (!host.has_been_shown_0068e600(name)) return;
    if (host.player_unit_present_e188d8() && tracker.last_unit_class_18 == host.player_unit_class_170()) {
        tracker.last_unit_class_18 = kHintUnitClassReset;  // 00690C5C
    }
    if (host.profile_has_seen_007f8e00(name)) {
        host.profile_unmark_007fae60(name);                // 00690C86
    } else {
        hint_tracker_drop_mission_seen_0068ee00(host, name);  // 00690C92
    }
}

// 0068EE00..0068F02B. Copies every entry that differs from the name (case-insensitive; an empty
// entry differs from a non-empty name) into a fresh list and swaps it into +48h.
void hint_tracker_drop_mission_seen_0068ee00(HintStoreHost& host, const std::string& name) {
    std::vector<std::string>& seen = host.mission_seen_48();
    std::vector<std::string> kept;
    for (const std::string& entry : seen) {
        if (!equals_insensitive(entry, name)) kept.push_back(entry);
    }
    seen.swap(kept);
}

// 00690CA0..00690CCF: empties the history map +7Ch (the names new since the pause screen).
void hint_tracker_clear_new_history_00690ca0(HintStoreHost& host) { host.history7c_clear_00690ca0(); }

// 00690CD0..00690FA9. Rebuilds the history map +94h from the profile's shown-hint list: every
// name whose definition is not missionspecific (+116h) is appended under its Type. No AAAAAA
// prefix and no +7Ch entry here, unlike 00690FD0. Ghidra shows a return after the new-key arm;
// the listing continues the loop (00690F92 JMP 00690D40).
void hint_tracker_rebuild_history_00690cd0(HintStoreHost& host) {
    host.history94_clear();
    for (const std::string& name : host.profile_shown_names_65c()) {
        const HintDefinition* def = host.find_definition_0068e450(name);  // 005FFB20
        if (def == nullptr || def->mission_specific) continue;
        if (!host.history94_has_key(def->type)) {
            host.history94_assign_single(def->type, name);   // 00690E40..00690F2E
        } else {
            host.history94_push_back(def->type, name);
        }
    }
}

namespace {

std::string hint_string_or_empty(HintLuaEntry& e, std::string_view key) { return e.is_nil(key) ? std::string() : e.string(key); }

}  // namespace

// 0068F030..00690AF6, read from the listing (`local/g5_fields.py` summary). One stack definition
// at +98h is default-constructed per entry (005FD650 clears the strings only), filled, then
// assigned to map[id] (005FFB20 + 005FDD90). The initial map find on the id (0068F136) is kept
// in a slot and never used.
// cooldowntime: 005FD650 does not clear +120h, but the loader writes it on both branches:
// 0068FFF4 stores EDI when the key is nil (0 on every path: XOR EDI,EDI at 0068FF7B, or at
// 0068FD78 through 0068FF27) and 00690029 the integer otherwise. A missing key gives 0.
// TitleDescList rows are written in iteration order with no bound (0069090F loop): a ninth
// element would overwrite +114h onwards. The reconstruction stops at eight and says so here.
void hint_tracker_load_definitions_0068f030(HintTableHost& host, bool other_category) {
    while (HintLuaEntry* e = host.next_entry()) {
        const std::string id = e->id();                                 // 0068F0CE
        HintDefinition def;
        def.cooldown_time = 0;                                          // 0068FFF4, EDI = 0
        def.title = hint_string_or_empty(*e, "Title");                  // 0068F1EB (nil -> 00CE3A0C "")
        def.description = hint_string_or_empty(*e, "Description");
        def.lua_command = hint_string_or_empty(*e, "LuaCommand");
        def.allied_title = hint_string_or_empty(*e, "AlliedTitle");
        def.allied_lua_command = hint_string_or_empty(*e, "AlliedLuaCommand");
        def.allied_description = hint_string_or_empty(*e, "AlliedDescription");
        def.japanese_title = hint_string_or_empty(*e, "JapaneseTitle");
        def.japanese_lua_command = hint_string_or_empty(*e, "JapaneseLuaCommand");
        def.japanese_description = hint_string_or_empty(*e, "JapaneseDescription");
        def.critical = e->is_nil("critical") ? false : e->boolean("critical");  // 0068FDCB
        def.loader_flag_50 = other_category;                            // 0068FE65..0068FE7A
        def.storing_rule = e->is_nil("storingrule") ? std::string(kHintDefaultRule) : e->string("storingrule");
        if (!e->is_nil("cooldowntime")) def.cooldown_time = e->integer("cooldowntime");  // 00690029
        def.duration = e->is_nil("duration") ? kHintDefaultDuration : e->integer("duration");  // 00690067
        def.type = e->is_nil("Type") ? std::string(kHintDefaultType) : e->string("Type");      // 00690104
        def.hidden = e->is_nil("hidden") ? false : e->boolean("hidden");                         // 0069025F
        def.mission_specific = e->is_nil("missionspecific") ? false : e->boolean("missionspecific");
        if (def.mission_specific) def.type = std::string(kHintMissionType);  // 00690393..00690408
        if (!e->is_nil("TitleDescList")) {                                    // 00690410
            const std::size_t n = std::min(e->row_count(), kHintTitleDescRows);
            for (std::size_t i = 0; i < n; ++i) {
                HintTitleDescRow& row = def.title_desc_list[i];
                row.title = e->row_is_nil(i, "Title") ? std::string() : e->row_string(i, "Title");
                row.description = e->row_is_nil(i, "Description") ? std::string() : e->row_string(i, "Description");
                row.lua_command = e->row_is_nil(i, "LuaCommand") ? std::string() : e->row_string(i, "LuaCommand");
            }
        } else {
            def.title_desc_list.fill(HintTitleDescRow{});                     // 00690944..00690982
        }
        host.store_definition_005fdd90(id, def);                              // 00690A1A..00690A21
    }
}

// 006921B0..00692578. Reload of the hint tables; the three SYSTEM groups are general hints,
// the two OTHER groups are loaded with the +50h flag set.
void hint_tracker_reload_006921b0(HintReloadState& tracker, HintReloadHost& host) {
    host.history7c_clear();
    host.run_script_with_overrides(kHintScript);
    host.definitions_clear_004d7370();
    host.select_table("SYSTEM", "Weapon");
    host.load_selected_0068f030(false);    // 0069233A
    host.select_table("SYSTEM", "Unit");
    host.load_selected_0068f030(false);    // 0069238A
    host.select_table("SYSTEM", "Gameplay");
    host.load_selected_0068f030(false);    // 006923DA
    host.select_table("OTHER", "Single");
    host.load_selected_0068f030(true);     // 00692460
    host.select_table("OTHER", "Multi");
    host.load_selected_0068f030(true);     // 006924B1
    host.rebuild_history_00690cd0();
    tracker.active_60.clear();             // 006924C8..006924DE
    tracker.last_unit_class_18 = kHintUnitClassReset;
    tracker.last_timed_1c.clear();
    tracker.last_timed_seconds_24 = 0.0F;
    tracker.force_next_04 = false;
    host.close_script();
}

// 004D6480..004D6740. For each of the 5Fh screen slots, every collected widget is parked
// 1000 units to the left (+121h set) when parking and the slot is not kept, and returned
// otherwise. The y and z components go through FADD 0.0 (a -0.0 becomes +0.0); x is computed in
// double from the float and stored as float.
void gui_park_screens_004d6480(GuiParkHost& host, const std::vector<std::int32_t>& kept, bool park) {
    for (std::int32_t slot = 0; slot < kGuiScreenSlots; ++slot) {
        bool is_kept = false;
        for (std::int32_t id : kept) {
            if (id == slot) is_kept = true;  // 004D64CD..004D64D5 (no early exit)
        }
        if (!host.screen_present_e18b60(slot)) continue;
        for (std::uintptr_t widget : host.screen_collect_24(slot)) {   // 004D6511
            if (widget == 0) continue;
            const bool hide = park && !is_kept;
            if (hide == host.widget_parked_121(widget)) continue;      // 004D6573 / 004D6640
            host.widget_set_parked_121(widget, hide);
            const std::array<float, 3> p = host.widget_resolved_position_00aa6750(widget);
            const double x = hide ? static_cast<double>(p[0]) - kGuiParkOffsetX : static_cast<double>(p[0]) + kGuiParkOffsetX;
            host.widget_set_resolved_position_00aa8240(widget, {static_cast<float>(x), p[1] + 0.0F, p[2] + 0.0F});
        }
    }
}

// 004D94F0..004D95E1.
void game_set_hint_pause_004d94f0(GamePauseState& game, GamePauseHost& host, bool paused) {
    if (game.hint_pause_620 == paused || game.pause_lock_624 != 0) return;
    game.hint_pause_620 = paused;
    const std::vector<std::int32_t> kept(kHintPauseKeptScreens.begin(), kHintPauseKeptScreens.end());
    gui_park_screens_004d6480(host, kept, paused);  // 004D95B7
}

// 004D95F0..004D9829. Setting the pause while a hint pauses advances the tutorial step; leaving
// it in a session with the HUD +E8h screen up forces an input-context refresh.
void game_set_system_pause_004d95f0(GamePauseState& game, GamePauseHost& host, bool paused) {
    if (game.system_pause_61f == paused) return;
    game.system_pause_61f = paused;
    if (paused && game.hint_pause_620) host.hints_advance_basic_step_0054e440();  // 004D963B
    if (game.session_kind_1fe4 != 0 && !game.system_pause_61f && host.hud_screen_e8_flag()) {
        host.input_force_refresh();                                            // 004D966C..004D967F
    }
    const std::vector<std::int32_t> kept(kSystemPauseKeptScreens.begin(), kSystemPauseKeptScreens.end());
    gui_park_screens_004d6480(host, kept, paused);
}

// 00551440..00551AF4, read whole from the listing. Per mode: the allied pairs, then (also when
// an allied pair breaks the loop, 0055184B) the japanese pairs.
void idle_camera_load_shots_00551440(IdleCameraShotArray& allied_e19328, IdleCameraShotArray& japanese_e193a8,
                                     const IdleCameraPosTable* root, std::int32_t root_kind) {
    // 0055146B..005514F8: every vector of both arrays is emptied first (00550E70 erase).
    for (std::size_t mode = 0; mode < kIdleCameraModeKeys.size(); ++mode) {
        allied_e19328[mode].clear();
        japanese_e193a8[mode].clear();
    }
    if (root == nullptr || root_kind != 6) return;  // 005514FD..00551503
    // 00551510..00551590: all eight mode tables are looked up before the loop.
    std::array<const IdleCameraPosTable*, 8> modes{};
    for (std::size_t mode = 0; mode < modes.size(); ++mode) modes[mode] = root->table(kIdleCameraModeKeys[mode]);

    const auto load_side = [](std::vector<IdleCameraShot>& shots, const IdleCameraPosTable* side) {
        for (std::int32_t pair = 1; pair <= kIdleCameraMaxPairs; ++pair) {
            char key[16];
            std::snprintf(key, sizeof key, "Pair%02d", pair);           // 0055160D / 00551880 sprintf
            const IdleCameraPosTable* entry = side->table(key);
            const char* from = entry->string(kIdleCameraFromKey);
            if (from == nullptr || *from == '\0') return;               // 00551642..0055164D
            // 00551653..0055169A: a temporary with only its two strings (+08h..+14h) zeroed is
            // pushed (00551390); +00h, +04h and +18h are uninitialised stack in the image.
            shots.push_back(IdleCameraShot{});
            shots.back().eye_name = from;
            const char* to = entry->string(kIdleCameraToKey);           // 00551780..0055179E
            if (to == nullptr || *to == '\0') return;                   // the shot keeps eye only
            shots.back().target_name = to;
            shots.back().enabled = true;                                // 0055183D MOV [ESI+18h],1
        }
    };
    for (std::size_t mode = 0; mode < modes.size(); ++mode) {
        load_side(allied_e19328[mode], modes[mode]->table(kIdleCameraAlliedKey));      // 005515E7
        load_side(japanese_e193a8[mode], modes[mode]->table(kIdleCameraJapaneseKey));  // 00551854
    }
}

// 004E7220..004E7407, read whole from the listing (SSE conversions, no x87).
void scene_load_cloud_areas_004e7220(SceneCloudRecord& record, SceneCloudHost& host, const ScenePropertyTable& cloud) {
    record.area_count_c84 = 0;  // 004E722F
    const auto number = [](const ScenePropertyValue& v) {  // 004E72AE..004E72BB
        return v.kind == 0 ? static_cast<float>(v.int_value) : v.float_value;
    };
    for (std::int32_t slot = 0; slot < kCloudAreaSlots; ++slot) {
        char key[16];
        std::snprintf(key, sizeof key, "Area%02d", slot);  // 004E7240 (004E60E0)
        const ScenePropertyTable* area = cloud.table(key);  // 004E7251..004E726F
        if (area == nullptr) continue;
        if (area->value("Num").int_value == 0) continue;   // 004E727A, raw +0Ch test
        SceneCloudArea& out = record.areas[static_cast<std::size_t>(record.area_count_c84++)];  // 004E7289..004E72A3
        out.box_min = {number(area->value("BoxMinX")), number(area->value("BoxMinY")), number(area->value("BoxMinZ"))};
        out.box_max = {number(area->value("BoxMaxX")), number(area->value("BoxMaxY")), number(area->value("BoxMaxZ"))};
        out.num = area->value("Num").int_value;             // 004E7389, raw copy
        out.small_weight = number(area->value("SmallWeight"));
        out.medium_weight = number(area->value("MediumWeight"));
        out.big_weight = number(area->value("BigWeight"));   // 004E73E9
    }
    host.scatter_clouds_004ba870();  // 004E7403 JMP
}

// 004D9830..004D9B30, read from the listing. Hiding stores each node's factor through
// operator[] (004D9AEE), so hiding twice overwrites the saved factors with the zeros of the
// first hide; the image has the same property (004DC020 restores with the saved +61Eh value).
void game_set_gui_layers_visible_004d9830(GuiLayerVisibilityState& game, GuiLayerVisibilityHost& host,
                                          bool visible) {
    game.gui_visible_61e = visible;  // 004D985C
    if (visible) {
        // 004D9890..004D98C6: every saved node gets its factor back (FLD [entry+10h]), no recursion.
        for (const auto& [node, factor] : game.saved_628) host.node_set_visibility_00b6da70(node, factor, false);
        game.saved_628.clear();  // 004D98CB..004D9919, the inlined tree clear
        return;
    }
    // 004D992E..004D9B1A: all layer nodes except "_Console" and "_Fps" (case-insensitive).
    for (const std::uintptr_t node : host.gui_layer_nodes()) {
        const std::string name = host.node_name_00b6d800(node);
        if (equals_insensitive(name, kGuiKeepConsoleNode) || equals_insensitive(name, kGuiKeepFpsNode)) continue;
        game.saved_628[node] = host.node_visibility_ac(node);  // 004D9AD5..004D9B02
        host.node_set_visibility_00b6da70(node, 0.0F, false);  // 004D9AF3 FLDZ, 004D9B06
    }
}

// ---------------------------------------------------------------------------
// HintSystem mission-Lua wrappers. Argument 0 is the index pushed first before 00B677E0 (EBP,
// zeroed in each prologue). A missing argument reaches 00B662B0 as nil; the image does not test
// for it, and the host's argument_string decides what that yields.
// ---------------------------------------------------------------------------

std::int32_t lua_show_hint_008d1f50(HintLuaHost& host) {
    // 008D202D..008D2041: CMP [game+624h], 0; JNZ to the result count at 008D2140.
    if (host.game_termination_reason_624() == 0) {
        host.tracker_trigger_hint_00690fd0(host.argument_string(0), true);  // 008D2070 PUSH 1; 008D2086
        // 008D20CC re-reads argument 0; 008D20EB..008D2104 ECX = game+1EF0h.
        host.session_send_hint_00770870(host.argument_string(0));
    }
    return host.result_count_00b66400();  // 008D2144
}

std::int32_t lua_show_hint_forced_008d2190(HintLuaHost& host) {
    host.tracker_trigger_forced_00692fb0(host.argument_string(0));  // 008D22B2
    return host.result_count_00b66400();
}

std::int32_t lua_add_stored_hint_008d24f0(HintLuaHost& host) {
    host.tracker_add_stored_00690b20(host.argument_string(0));  // 008D2612
    return host.result_count_00b66400();
}

std::int32_t lua_remove_stored_hint_008d2340(HintLuaHost& host) {
    host.tracker_remove_stored_00690c30(host.argument_string(0));  // 008D2462
    return host.result_count_00b66400();
}

std::int32_t lua_hide_hint_0088cec0(HintLuaHost& host) {
    host.hints_dismiss_0054e610();  // 0088CF9E..0088CFB2, ECX = *(*(00E198C4)+A8h)
    return host.result_count_00b66400();
}

std::int32_t lua_is_hint_active_008d2850(HintLuaHost& host) {
    if (host.argument_count_00b663f0() == 0) {  // 008D2939, JNZ 008D2940
        // 008D2968 active name, 008D2974 00449AF0 case-insensitive "!= """, then a table
        // {HintID = active name (008D2A03), duration = seconds left (008D2A50, 008D2A5F)}.
        const std::string active = host.tracker_active_hint_0068e5b0();
        if (!active.empty()) host.push_hint_table(active, host.hints_seconds_left_0054d3c0());
    } else {
        // 008D2AAA..008D2B3F: 008D2AE6 00435C40, the argument against the active name,
        // case-insensitively.
        const std::string name = host.argument_string(0);
        if (equals_insensitive(name, host.tracker_active_hint_0068e5b0())) {
            host.push_integer(host.hints_seconds_left_0054d3c0());  // 008D2B53, 008D2B5D
        }
    }
    return host.result_count_00b66400();  // 008D2B66; 0 results reads as nil in the script
}

std::int32_t lua_is_hint_critical_008d26a0(HintLuaHost& host) {
    // 008D27C0: 00690B00 returns 0 for an empty name, else definition(name)+114h.
    host.push_boolean(host.tracker_definition_critical_00690b00(host.argument_string(0)));  // 008D27CA
    return host.result_count_00b66400();
}

std::int32_t lua_is_hint_stored_008d2bb0(HintLuaHost& host) {
    // 008D2CD0: 0068E600, the profile seen set (game+650h) or the mission list tracker+48h.
    host.push_boolean(host.tracker_has_been_shown_0068e600(host.argument_string(0)));  // 008D2CDA
    return host.result_count_00b66400();
}

}  // namespace bsp
