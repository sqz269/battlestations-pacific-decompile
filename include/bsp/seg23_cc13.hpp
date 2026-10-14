#pragma once
// Segment 23 (005E0DB0-00604A50), packet cc13-seg23: the multiplayer lobby and
// menu screens, the options screen, the in-mission order screen (screen 3Bh)
// and its command helpers, and the pause screens. docs/CC13_SEG23.md has the
// evidence table. Names are hypotheses; offsets are the image's.
//
// Style: objects are raw image layouts read and written at byte offsets; every
// call the image makes goes through a named host method at the same point.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp::seg23_cc13 {

// ===========================================================================
// In-mission order screen (screen 3Bh, vtable 00CF3B7C) command helpers.
// The controlled unit is [00E188D8]; unit vtable +5Ch is IsKindOf(kind):
// kind 6 = ship (orders component at unit+738h), 18h = squadron (component at
// unit+348h).
// ===========================================================================

// A command target as the stack blocks of 005F9AA0..005F9CF0 lay it out (18h
// bytes, passed by address to 0077D600 BSP_Entity_IssueCommand).
struct CommandTarget18 {
    std::uint8_t b0{0};
    std::uint8_t b1{0};     // 1 for "an entity" (005F9B20)
    std::uint16_t w2{0};
    std::uint32_t d4{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float f14{0.0f};
};
static_assert(sizeof(CommandTarget18) == 0x18);

// A command object address followed by its target (1Ch bytes): 005F9A50
// clears one, 005F9CF0 fills one.
struct CommandWithTarget1C {
    std::uint32_t command{0};
    CommandTarget18 target;
};
static_assert(sizeof(CommandWithTarget1C) == 0x1C);

inline constexpr std::uint32_t kCommandMoveTo = 0x00E08F68u;
inline constexpr std::uint32_t kCommandAttackMove = 0x00E08F78u;
inline constexpr std::uint32_t kCommandReturnToBase = 0x00E08F98u;
inline constexpr std::uint32_t kCommandLeave = 0x00E08FB0u;
inline constexpr std::uint32_t kCommandDisband = 0x00E08FB8u;
inline constexpr std::uint32_t kSessionMessageRootVtable = 0x00CE4974u;
inline constexpr std::uint32_t kSessionMessage81Vtable = 0x00CF3BA4u;
inline constexpr std::uint32_t kSessionMessage188Vtable = 0x00CF3BB8u;
inline constexpr std::uint32_t kSessionMessage87Vtable = 0x00CECCB4u;
inline constexpr std::string_view kAttackMoveCommandName = "attackmove";  // 00CECCA8
inline constexpr std::string_view kRegroupWarning = "regroup";            // 00CF3C94
inline constexpr std::string_view kAttackWarning = "attack";              // 00CF3C9C

// The stack message the order helpers build: 0075B430(type) constructs the
// base in place, then the caller stores +4h = 1, the class vtable at +0h and
// zero bytes at +18h (word), +1Ah, +1Ch; message BCh also has a byte +20h.
// The routing call is 0077C2A0 __thiscall(unit, message*, a, b) RET 0Ch. No
// destructor runs (the class destructors are not called on these locals).
using StackMessage = std::array<std::byte, 0x24>;

struct HudOrderCommandHost {
    virtual ~HudOrderCommandHost() = default;
    virtual void* controlled_unit_00e188d8() = 0;
    virtual void* frontend_manager_00e198c4() = 0;
    virtual void* game_00e188a8() = 0;
    virtual void* warning_manager_00f8a0c4() = 0;
    virtual float fixed_step_clock_00f876a4() = 0;
    virtual float clock_00f88c28() = 0;
    // The zero point 00F87574/78/7C (three floats).
    virtual std::array<float, 3> zero_point_00f87574() = 0;
    virtual bool unit_is_kind(void* unit, std::int32_t kind) = 0;              // vtable +5Ch
    virtual void* orders_current_target_2c(void* component) = 0;                // component vtable +2Ch
    virtual void orders_set_mode_54(void* component, std::int32_t mode) = 0;    // component vtable +54h
    // unit vtable +16Ch(name, target) -> AL: accepts the named command.
    virtual bool unit_accepts_command_16c(void* unit, std::string_view name, void* target) = 0;
    // 007EDA90 __thiscall(squadron) -> AL (ledger BSP_PlaneSquadron_LeaderIsUncommittedKamikaze).
    virtual bool squadron_leader_uncommitted_kamikaze_007eda90(void* unit) = 0;
    // 0071C060 __fastcall(ECX = mask, EDX = unit) -> AL.
    virtual bool director_flags_allow_0071c060(std::uint32_t mask, void* unit) = 0;
    // 004F8670 cdecl(a, b, c, d): front-end level-3 set.
    virtual void frontend_set_level3_004f8670(std::int32_t a, std::int32_t b, std::int32_t c, std::int32_t d) = 0;
    // 004D8B70 cdecl(game, a, b, c, d): push an input-context level.
    virtual void input_push_level_004d8b70(void* game, std::int32_t a, std::int32_t b, std::int32_t c,
                                           std::int32_t d) = 0;
    // 00414DB0 __thiscall(entity): refresh the world pose (+FCh..+104h) when +C8h is clear.
    virtual void entity_refresh_world_pose_00414db0(void* entity) = 0;
    // 0077D600 __thiscall(unit, command, target*, flags) RET 0Ch.
    virtual void entity_issue_command_0077d600(void* unit, std::uint32_t command, const void* target,
                                               std::int32_t flags) = 0;
    // 0077C470 __thiscall(unit, role, zero) RET 8.
    virtual void unit_send_role_transfer_0077c470(void* unit, std::uint32_t role, std::uint32_t zero) = 0;
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;   // __thiscall -> AL
    virtual bool unit_is_group_leader_00778890(void* unit) = 0;         // __thiscall -> AL
    // 007F16D0 __thiscall(squadron, CommandWithTarget1C* out) RET 4.
    virtual void squadron_resolve_return_to_base_007f16d0(void* unit, CommandWithTarget1C* out) = 0;
    // 0075B430 __thiscall(message, type) RET 4: the session message base.
    virtual void session_message_construct_base_0075b430(void* message, std::int32_t type) = 0;
    // 0077C2A0 __thiscall(unit, message, a, b) RET 0Ch.
    virtual void entity_route_message_0077c2a0(void* unit, const void* message, std::int32_t a,
                                               std::int32_t b) = 0;
    // 00977BA0 __thiscall(warnings, unit, const NativeString*) RET 8. The
    // NativeString temporary (0041DD40/memcpy or 0041E870, released to the
    // sized pool through 00419CC0/00BD1510) is the host's.
    virtual void warning_report_confirm_00977ba0(void* warnings, void* unit, std::string_view text) = 0;
    // 00465080 __thiscall(CommandTarget18* out, entity, float) RET 8 -> out.
    virtual const void* command_target_from_entity_00465080(CommandTarget18* out, void* entity, float range) = 0;
    // 00565FB0 __thiscall(limbo screen, state) RET 4.
    virtual void hud_limbo_set_state_00565fb0(void* limbo_screen, std::int32_t state) = 0;
    // 004CC460 __thiscall(manager, interface, arg) RET 8.
    virtual void frontend_push_interface_request_004cc460(void* manager, std::int32_t id, std::int32_t arg) = 0;
};

// 005F9470 __thiscall(screen, bool) RET 4: screen+8Ch = value (caller
// 00891070, the Lua binding SetForceSellableUnits).
void hud_order_set_force_sellable_005f9470(void* screen, std::uint8_t value) noexcept;
// 005F9580 __fastcall(ECX = screen): toggles the permanent-order flag +83h and
// pushes it to the controlled unit's orders component (ship: unit+738h;
// squadron: unit+348h, where an uncommitted kamikaze leader forces the flag
// clear first) through component vtable +54h(flag). [00E188D8] is not null
// checked.
void hud_order_toggle_permanent_005f9580(void* screen, HudOrderCommandHost& host);
// 005F9710 __fastcall(ECX = unit) -> EAX 0/1: IsKindOf 9, 45h or 46h.
bool unit_is_kind_9_45_46_005f9710(void* unit, HudOrderCommandHost& host);
// 005F9790 __fastcall(ECX = message): the message base destructor body
// (vtable = 00CE4974); used from unwind funclets only.
void session_message_reset_vtable_005f9790(void* message) noexcept;
// 005F9820 cdecl -> EAX: 1 when [00F88C28] > [00F876A4] (FCOMIP; unordered -> 0).
bool clock_f88c28_ahead_005f9820(HudOrderCommandHost& host);
// 005F9840 cdecl -> AL: the controlled ship can attack-move at its orders
// component's current target.
bool hud_order_can_attack_move_005f9840(HudOrderCommandHost& host);
// 005F98B0 cdecl: open the order sub-menu (levels 3Bh/2Ah/49h, input level
// 18h/0Fh/19h) and clear [[00E198C4]+58h]+4.
void hud_order_open_submenu_005f98b0(HudOrderCommandHost& host);
// 005F9A50 __fastcall(ECX = out) -> EAX = out: no command, the empty target.
CommandWithTarget1C* command_with_target_clear_005f9a50(CommandWithTarget1C* out, HudOrderCommandHost& host);
// 005F9AA0 cdecl: return-to-base at the zero point, then [[00E198C4]+68h]+190h = 1.
void hud_order_issue_return_to_base_005f9aa0(HudOrderCommandHost& host);
// 005F9B20 __stdcall(entity) RET 4: moveto the entity's world position.
void hud_order_issue_moveto_entity_005f9b20(void* entity, HudOrderCommandHost& host);
// 005F9BB0 cdecl: role transfer 1FFh, then the 005F9AA0 return-to-base.
void hud_order_release_and_return_005f9bb0(HudOrderCommandHost& host);
// 005F9C40 cdecl: leave (formation follower) or disband (group leader).
void hud_order_leave_or_disband_005f9c40(HudOrderCommandHost& host);
// 005F9CF0 __stdcall(out) RET 4 -> EAX = out: a squadron's return-to-base
// command (007F16D0), otherwise the cleared pair.
CommandWithTarget1C* hud_order_resolve_return_to_base_005f9cf0(CommandWithTarget1C* out, HudOrderCommandHost& host);
// 005FA9F0 cdecl: message BCh (+20h = 0) routed (0, 0), then warning "regroup".
void hud_order_regroup_005fa9f0(HudOrderCommandHost& host);
// 005FAAE0 cdecl: attack. Ship: with a current target, role transfer 2,
// message 57h routed (0, 0), attackmove at 00465080(target, 0.0f). Squadron:
// message BCh (+20h = 1) routed (0, 0), then warning "attack".
void hud_order_attack_005faae0(HudOrderCommandHost& host);
// 005FACB0 cdecl: with a controlled unit, limbo state 3, interface 34h when the
// manager's current (+4h, +1Ch) equals (+20h, +38h), and message 51h routed (7, 0).
void hud_order_leave_unit_005facb0(HudOrderCommandHost& host);
// 005FD420 __fastcall(ECX = object) -> EAX: six dwords +0h..+14h = 0.
void* zero_six_dwords_005fd420(void* object) noexcept;

// ===========================================================================
// Options screen (vtable 00CF3940; page index 00E19650) and its listeners.
// String temporaries the image builds (0041DD40 + memcpy, 0041E870) and
// returns to the sized pool (00419CC0 / 00BD1510) are the host's: a method
// taking std::string_view stands for "build the NativeString, call, release".
// ===========================================================================

inline constexpr float kOptionsFocusAlphaOn = 0.5f;    // 00CE3800
inline constexpr float kOptionsFocusAlphaOff = 1.0f;   // 00D7A24C
inline constexpr std::string_view kOptionsVerifyLocale = "en-US";               // 00CF38A0
inline constexpr std::string_view kOptionsRestartMessage = "FE.opt_restart";    // 00CF38A8
inline constexpr std::string_view kOptionsSaveConfirm = "FE_xbox.xsm_saveconfirm";  // 00CF3908
inline constexpr std::string_view kRowValueTextChild = "var_Text";              // 00CEE66C
inline constexpr std::string_view kRowIconChild = "var_Icon";                   // 00CF38FC
inline constexpr std::string_view kRowAutoControlGroup = "dodgeListboxAutoControl_Group";  // 00CEE6D0
inline constexpr std::string_view kNewContentText = "FE.opt_newcontent";        // 00CF38E8
inline constexpr std::string_view kDamagedContentText = "FE.opt_damagedcontent";  // 00CF38D0
inline constexpr std::string_view kArrowLeftIcon = "arrow_left_Icon";     // 00CEB278
inline constexpr std::string_view kArrowRightIcon = "arrow_right_Icon";   // 00CEB264
inline constexpr std::string_view kSliderIconPrefix = "slider0";          // 00CF39E0
inline constexpr std::string_view kSliderIconSuffix = "_Icon";            // 00CED1BC
inline constexpr std::uint32_t kPromptApplyAndSaveSettings = 0x005F01C0u;
inline constexpr std::uint32_t kStorageEmptyContinuation = 0x00BD53C0u;
// The help-line label tables 005F0220 indexes, by page.
inline constexpr std::uint32_t kHelpTablePage0 = 0x00E08A68u;
inline constexpr std::uint32_t kHelpTablePage1 = 0x00E08C10u;
inline constexpr std::uint32_t kHelpTablePage2 = 0x00E08ADCu;
inline constexpr std::uint32_t kHelpTablePage3 = 0x00E08B38u;
inline constexpr std::uint32_t kHelpTablePage4 = 0x00E08B9Cu;

struct OptionsScreenHost {
    virtual ~OptionsScreenHost() = default;
    virtual std::int32_t options_page_00e19650() = 0;
    virtual void* xenon_system_manager_00f8abe8() = 0;
    virtual void* command_bar_00e1930c() = 0;
    virtual bool settings_dirty_00f88a30() = 0;               // byte 00F88A30
    // [00F88A38], or 00E19654 (an empty string) when null.
    virtual const char* keyboard_default_text_00f88a38() = 0;
    // CRT / Xbox emulation imports.
    virtual void memset_00bf79f0(void* dst, int value, std::uint32_t size) = 0;
    // 00A4D464 XStringVerify(flags, locale, count, STRING_DATA*, size, results, overlapped).
    virtual std::uint32_t xstring_verify_00a4d464(std::uint32_t flags, std::string_view locale, std::uint32_t count,
                                                  const void* string_data, std::uint32_t result_size,
                                                  void* results, void* overlapped) = 0;
    // 00BF9DAA mbstowcs_s(&converted, dst, size_in_words, src, count).
    virtual int mbstowcs_s_00bf9daa(std::uint32_t* converted, wchar_t* dst, std::uint32_t size_in_words,
                                    const char* src, std::uint32_t count) = 0;
    // 00A4D43A XShowKeyboardUI(user, flags, default, title, description, result, size, overlapped).
    virtual std::uint32_t xshow_keyboard_ui_00a4d43a(std::uint32_t user, std::uint32_t flags,
                                                     const wchar_t* default_text, const wchar_t* title,
                                                     const wchar_t* description, wchar_t* result,
                                                     std::uint32_t result_size, void* overlapped) = 0;
    virtual bool xenon_has_selected_user_00a3e510(void* manager) = 0;     // __thiscall -> AL
    virtual std::uint32_t xenon_active_user_slot_00a3eac0(void* manager) = 0;
    // Widget virtuals.
    virtual std::int32_t widget_kind_5c(void* widget) = 0;                 // vtable +5Ch
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;       // vtable +4Ch
    virtual void widget_show_88(void* widget, std::int32_t state, std::int32_t zero, float alpha) = 0;  // vtable +88h
    virtual void widget_enable_34(void* widget, std::int32_t value) = 0;   // vtable +34h
    // 00425D10 then 00531B00 on it (ten arguments, RET 2Ch): slot, message,
    // kind, callback, unused flag, title, timeout, timeout result, countdown key
    // (by value), dismissible.
    virtual void raise_prompt_00531b00(std::int32_t slot, std::string_view message, std::int32_t kind,
                                       std::uint32_t callback, std::int32_t unused, std::string_view title,
                                       float timeout, std::int32_t timeout_result, std::string_view countdown_key,
                                       std::int32_t dismissible) = 0;
    // 00A9CDF0 __fastcall(ECX = listbox, EDX = other1, [stack] other2) RET 4.
    virtual void listbox_clear_other_two_00a9cdf0(void* listbox, void* other1, void* other2) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // 005F0220 __fastcall(ECX = out, EDX = table, [stack] index): table[index] copy.
    virtual std::string_view options_row_label_at_005f0220(std::uint32_t table, std::int32_t index) = 0;
    // 0054A0C0 __thiscall(bar, const NativeString*) RET 4.
    virtual void command_bar_set_help_line_0054a0c0(void* bar, std::string_view text) = 0;
    // 00A9BE00 __thiscall(listbox, row) RET 4 -> the row widget.
    virtual void* listbox_row_widget_00a9be00(void* listbox, std::int32_t row) = 0;
    // 00AA7E00 __thiscall(widget, const NativeString* name, 1) RET 8 -> child.
    virtual void* widget_find_child_00aa7e00(void* widget, std::string_view name, std::int32_t recurse) = 0;
    // 00ABAED0 __thiscall(text, const NativeString*, 1) RET 8: localised source.
    virtual void text_set_localised_source_00abaed0(void* text, std::string_view key, std::int32_t flag) = 0;
    // 00ABBE50 __thiscall(text, const char*, 1) RET 8.
    virtual void text_set_localised_cstring_00abbe50(void* text, const char* key, std::int32_t flag) = 0;
    // 004C1710 __stdcall(1) -> media manager; 00A4C2D0 (ECX = it) is a bare RET.
    virtual void* media_manager_get_or_create_004c1710(std::int32_t create) = 0;
    virtual void media_manager_noop_00a4c2d0(void* media) = 0;
    // 008D5B50 / 008D6C00(callback) on the settings object 00F88980.
    virtual void settings_apply_all_008d5b50() = 0;
    virtual void settings_save_to_profile_008d6c00(std::uint32_t continuation) = 0;
    // 00AA6BC0 __thiscall(widget, listener, byte) RET 8.
    virtual void widget_set_listener_fields_00aa6bc0(void* widget, std::uint32_t listener, std::uint8_t flag) = 0;
    virtual bool widget_contains_pointer_00aa6a40(void* widget) = 0;      // __thiscall -> AL
    virtual std::int16_t widget_state_94(void* widget) = 0;                // vtable +94h -> AX
    virtual void* control_binding_manager_00f8bbf4() = 0;
    virtual bool control_entry_active_20(void* entry, std::int32_t arg) = 0;  // entry vtable +20h -> AL
    // 00A9CEB0 __thiscall(listbox, float, int) RET 8 (ledger BSP_GuiListbox_SetField124).
    virtual void listbox_set_field124_00a9ceb0(void* listbox, float value, std::int32_t flag) = 0;
    // 005F0390 __fastcall(ECX = parent, EDX = listener): the auto-control group lookup.
    virtual void options_find_auto_control_group_005f0390(void* parent, void* listener) = 0;
    // 005F1BB0 __fastcall(ECX = screen): the options screen destructor body.
    virtual void options_screen_destruct_005f1bb0(void* screen) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // A const char* read out of a pointer table in .data (table + index * 4).
    virtual const char* string_table_entry(std::uint32_t table, std::uint32_t index) = 0;
    // 008D48C0 / 008D4260 __thiscall(settings copy) -> const char*.
    virtual const char* settings_language_lockit_id_008d48c0(void* settings) = 0;
    virtual const char* settings_unit_label_key_008d4260(void* settings) = 0;
    virtual void* content_manager_00f8a304() = 0;   // [00F8A304]; +4h = the 18h-byte content entries
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 004F9A80 cdecl(buffer, format, ...) into a stack buffer (BSP_FormatString128).
    virtual std::string format_004f9a80(const char* format, std::int32_t a, std::int32_t b) = 0;
    virtual std::string format_004f9a80(const char* format, std::int32_t a) = 0;
    virtual std::string format_004f9a80(const char* format, double a) = 0;
    virtual void* resolution_table_00f8895c() = 0;   // [00F8895C]: 8-byte {width, height} entries
    virtual void* renderer_00f8d394() = 0;           // [00F8D394]
    virtual void renderer_set_gamma_f0(void* renderer, float value) = 0;  // vtable +F0h
    virtual void options_commit_changes_005f65c0(void* screen) = 0;      // __fastcall(ECX = screen)
};
inline constexpr std::string_view kDlcChangedConfirm = "FE_xbox.xsm_dlcchanged";   // 00CF3B28
inline constexpr std::string_view kVideoResolutionConfirm = "FE.opt_vid_res_confirm";  // 00CF3B10
inline constexpr std::string_view kResolutionRevertTimer = "ResolutionRevertTimer";    // 00CF3AF8
inline constexpr float kResolutionRevertSeconds = 15.0f;                               // 00CE5380
inline constexpr float kOptionsConfirmDelay = 0.1f;                                    // 00D7A2F0
inline constexpr std::uint32_t kPromptOptionsApply = 0x005F6FB0u;
inline constexpr std::uint32_t kPromptVideoConfirm = 0x005F6FE0u;
inline constexpr const char* kResolutionFormat = ".%dx%d";     // 00CF39F8
inline constexpr const char* kMultisampleFormat = ".%d X";     // 00CF39F0
inline constexpr const char* kGammaFormat = ".%01.2f";         // 00CF39E8
inline constexpr std::uint32_t kOnOffTexts = 0x00E089F4u;      // the 00E089F4 table again
inline constexpr std::uint32_t kVideoQualityTexts = 0x00E089FCu;
inline constexpr std::uint32_t kVideoDetailTexts = 0x00E08A20u;
inline constexpr std::uint32_t kGamePageRow3UnitTexts = 0x00E08A2Cu;  // 005F4C90 row 3, dword +E4h
inline constexpr std::uint32_t kControlPageTextsA = 0x00E08A04u;      // 005F34D0 rows 2-3
inline constexpr std::uint32_t kControlPageTextsB = 0x00E08A0Cu;      // 005F34D0 rows 4-5
inline constexpr double kSliderScale = 10.0;   // 00CE3DC0
inline constexpr double kSliderRound = 0.5;    // 00D7A280
inline constexpr std::uint32_t kGamePageRow3Texts = 0x00E089F4u;  // indexed by byte +165h
inline constexpr std::uint32_t kGamePageRow4Texts = 0x00E08A38u;  // indexed by dword +168h

// 005EF550 __fastcall(ECX = screen): +2Ch..+44h (seven dwords, an XOVERLAPPED)
// zeroed; memset(+26Ch, 0, +70h); a STRING_DATA {length of the wide string at
// +48h counting its NUL, +48h} verified against "en-US" through XStringVerify
// (0, "en-US", 1, &data, +70h, +26Ch, +2Ch) after zeroing +2Ch..+44h again;
// +265h = 1.
void options_verify_entered_name_005ef550(void* screen, OptionsScreenHost& host);
// 005EF600 __thiscall(listener, widget, bool focused) RET 8 (listener slot 6
// of 00CF3940; ECX unused): kind 3 text gets alpha 0.5 (focused) or 1.0; kind
// 6 gets vtable +88h(focused, 0, 1.0f); kind 2 and others nothing.
void options_listener_on_focus_005ef600(void* widget, bool focused, OptionsScreenHost& host);
// 005EFAB0 __fastcall(ECX = screen): open the on-screen keyboard. The user slot
// (0 unless the Xenon manager has a selected user), +10h..+28h (an
// XOVERLAPPED) zeroed, the default text [00F88A38] widened into +B0h, +48h/+4Ah/
// +4Ch = 0, XShowKeyboardUI(user, 0, +B0h, 0, 0, +48h, 4, +10h); +264h = 1.
void options_show_keyboard_005efab0(void* screen, OptionsScreenHost& host);
// 005F0090 cdecl: the "FE.opt_restart" notice: prompt slot 0, kind 2, no
// callback, empty title, timeout 0.0, dismissible 1.
void options_show_restart_notice_005f0090(OptionsScreenHost& host);
// 005F0440 __thiscall(screen, unused, listbox) RET 8 (listener slot 10):
// clears the other two listboxes (+7Ch, +80h), then sets the command-bar help
// line to the selected row's label for pages 0, 4, 2, 3, 1 (tables
// 00E08A68/00E08B9C/00E08ADC/00E08B38/00E08C10) or "" for 7 and the rest. On
// page 2, row 2 enables +9Ch, row 3 enables +98h, any other row disables both.
void options_listener_on_list_activate_005f0440(void* screen, void* listbox, OptionsScreenHost& host);
// 005F0800 __thiscall(screen, row, const NativeString* text, bool new_content,
// bool damaged) RET 10h: the row's "var_Text" child gets "FE.opt_damagedcontent"
// (C string) when damaged, else "FE.opt_newcontent" when new, else the text.
void options_set_row_value_text_005f0800(void* screen, std::int32_t row, std::string_view text, bool new_content,
                                         bool damaged, OptionsScreenHost& host);
// 005F0940 __thiscall(screen, row, bool shown) RET 8: the row's
// "dodgeListboxAutoControl_Group" / "var_Icon" child gets vtable +88h(shown, 0, 1.0f).
void options_set_row_icon_005f0940(void* screen, std::int32_t row, bool shown, OptionsScreenHost& host);
// 005F0B90 cdecl (caller BSP_LuaBinding_SetInputModifiers): with 00F88A30 set
// and a selected Xenon user, raise "FE_xbox.xsm_saveconfirm" (slot 0, kind 1,
// callback 005F01C0); otherwise 004C1710(1) / 00A4C2D0, apply the settings and
// save them with the empty continuation 00BD53C0.
void options_save_input_modifiers_005f0b90(OptionsScreenHost& host);
// 005F1620 __stdcall(row widget) RET 4: +77h = 1; the "arrow_left_Icon" and
// "arrow_right_Icon" children each get 00AA6BC0(0, 0).
void options_row_show_value_arrows_005f1620(void* row, OptionsScreenHost& host);
// 005F1CC0 __fastcall(ECX = screen), slot +14h of 00CF3974: binds the page
// widgets. +84h Main_Listbox (in +7Ch), +8Ch MainListbox_Text (in +84h), +88h
// Settings_Listbox (in +80h) with 00A9CEB0(0.5f, 0) and +128h = 0.0055555557f,
// +254h New_FrameBox and +258h New_Text (in +80h, each vtable +34h(0)), +90h
// text_Group, +94h slider_Group, +98h toggle_chkbox_Group, +9Ch
// toggle_text_Group (in +88h, then each vtable +34h(0)), 005F0390(+94h,
// screen+0Ch), +A0h SfxVolume_Sound and +A4h SpeechVolume_Sound (in +7Ch).
void options_bind_widgets_005f1cc0(void* screen, OptionsScreenHost& host);
inline constexpr float kSettingsListboxField128 = 0.0055555557f;  // 00CF2548 (0x3BB60B61)
// 005F3B30 __thiscall(screen, flags) RET 4 -> EAX = screen: 005F1BB0, then
// _free when flags bit 0 is set (the scalar deleting destructor).
void* options_screen_scalar_delete_005f3b30(void* screen, std::uint32_t flags, OptionsScreenHost& host);
// 005F4FA0 __fastcall(ECX = screen), the audio page refresh: rows 0..3 get
// 005F3750 at position _ftol(field * 10.0 + 0.5) for +F8h, +100h, +108h, +104h.
// The x87 product and sum are 80-bit; double arithmetic here (MINOR).
void options_refresh_audio_rows_005f4fa0(void* screen, OptionsScreenHost& host);
// 005F5070 __fastcall(ECX = screen): rows 0..2 sliders for +100h, +108h, +104h,
// then the value texts of row 3 (00E089F4[byte +165h]) and row 4
// (00E08A38[+168h]) through 005F0800(row, text, 0, 0).
void options_refresh_game_page_rows_005f5070(void* screen, OptionsScreenHost& host);
// 005F4C90 __fastcall(ECX = screen), the game page refresh. Value texts
// through 005F0800(row, text, 0, 0): row 0 008D48C0(+D8h), row 1 008D4260(+D8h),
// row 2 00E089F4[byte +E1h], row 3 00E08A2C[+E4h], row 4 00E089F4[byte +E8h],
// row 5 00E089F4[byte +154h]; row 6 the slider at _ftol((f + f) * 10.0 + 0.5)
// for f = +158h (FADD ST0,ST0); row 7 00E089F4[byte +122h], row 8
// 00E089F4[byte +18Ah].
void options_refresh_game_rows_005f4c90(void* screen, OptionsScreenHost& host);
// 005F34D0 __fastcall(ECX = screen), the control page refresh. Value texts
// through 005F0800(row, text, 0, 0): row 0 00E089F4[byte +188h], row 1
// 00E089F4[byte +118h == 0], rows 2/3 00E08A04[byte +11Ah / +11Bh], rows 4/5
// 00E08A0C[byte +119h / +11Ch]. Then row 1's +77h = 0, and every row from 2
// below the listbox count ([+88h]+104h) gets +77h = (byte +188h == 0).
void options_refresh_control_rows_005f34d0(void* screen, OptionsScreenHost& host);
// 005F2D20 __fastcall(ECX = screen), the downloaded-content page refresh. For
// each listbox row r (count [+88h]+104h), with entry e = [[00F8A304]+4h] +
// r * 18h ({length, text} NativeString at +0h, damaged byte +12h): disabled
// when e's name equals (equal lengths, then __stricmp; two empty names match)
// any of the +174h names at +170h (8-byte NativeStrings), new when it equals
// any of the +180h names at +17Ch; then 005F0800(r, 00E089F4[!disabled], new,
// e+12h).
void options_refresh_content_rows_005f2d20(void* screen, OptionsScreenHost& host);
// 005F2F20 __fastcall(ECX = screen), the video page refresh. Value texts via
// 005F0800(row, text, 0, 0): 0 ".%dx%d" of the [00F8895C] entry +150h; 1
// 00E089FC[byte +F6h]; 2 00E089F4[0] when +130h is 0, else ".%d X"(+130h); 3
// 00E089F4[byte +138h]; 4 ".%01.2f"((double)+13Ch); 6 00E08A20[+12Ch]; 5
// 00E08A20[+140h] (row 6 is written before row 5); 7..11 00E089F4[byte +15Ch,
// +144h, +14Ch, +15Eh, +165h]; 12 00E08A38[+168h]. Last, [00F8D394] vtable
// +F0h(+13Ch).
void options_refresh_video_rows_005f2f20(void* screen, OptionsScreenHost& host);
// 005F7050 __thiscall(screen, bool video_changed) RET 4. With the content
// flag +252h set (checked when 00F88A30 is set or first): prompt slot 0
// "FE_xbox.xsm_dlcchanged", kind 1, callback 005F6FB0. Else with a video change:
// "FE.opt_vid_res_confirm", kind 1, callback 005F6FE0, timeout 15.0f,
// countdown key "ResolutionRevertTimer". Else, only when 00F88A30 is set and the
// Xenon manager has a selected user: "FE_xbox.xsm_saveconfirm", kind 1,
// callback 005F6FB0. Every other case commits at once (005F65C0). All prompts
// have an empty title, timeout result 0 and dismissible 1. Finally +260h = 0.1f.
void options_confirm_changes_005f7050(void* screen, bool video_changed, OptionsScreenHost& host);
// 005F2290 __fastcall(ECX = slider widget) -> EAX: the step a click asks for.
// Only for a kind-2 widget while the front entry of the control list
// ([00F8BBF4]+94h) answers vtable +20h(0): the first of "slider01_Icon" ..
// "slider09_Icon" (children of the widget) under the pointer decides; with
// state (vtable +94h) 0: i != 1; state 1: -(i != 9); otherwise i == 1 -> -1,
// else i == 9. No hit, no entry, or another kind: 0.
std::int32_t control_slider_step_from_pointer_005f2290(void* widget, OptionsScreenHost& host);
// 005F3750 __fastcall(ECX = row, EDX = position): the nine slider icons under
// the row's "dodgeListboxAutoControl_Group" get vtable +88h(state, 0, 1.0f),
// state 1 below the position, 2 at it, 0 above.
void control_slider_set_row_005f3750(void* row, std::int32_t position, OptionsScreenHost& host);

// ===========================================================================
// 005F6190 __fastcall(ECX = vector<bool>) RET: clear() as erase(begin, end).
// Layout +0h bit count, +8h first word, +Ch last word. Each iterator is
// {container, word, bit offset}; the end one is advanced by the count through
// 0048D3B0 (only when the count is above zero); 00499700 erases. The two
// checked-iterator guards (+8h > +Ch, null container) call 00BF6713.
struct BitVectorClearHost {
    virtual ~BitVectorClearHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    // 0048D3B0 __thiscall(iterator, count) RET 4.
    virtual void bit_iterator_advance_0048d3b0(std::array<std::uint32_t, 3>& iterator, std::uint32_t count) = 0;
    // 00499700 __thiscall(vector, out, first, last) RET 1Ch.
    virtual void bit_vector_erase_00499700(void* vector, std::array<std::uint32_t, 3>& out,
                                           const std::array<std::uint32_t, 3>& first,
                                           const std::array<std::uint32_t, 3>& last) = 0;
    virtual std::uint32_t address_of(const void* object) = 0;  // the image address of the container
};
void bit_vector_clear_005f6190(void* vector, BitVectorClearHost& host);

// 005F6EF0 __fastcall(ECX = screen): the vector<bool> at +C4h is cleared
// (005F6190) and gets one element (a loop of exactly one pass, EBX 0..0):
// true when the input manager (004BEC00) reports action A4h sharing a binding
// with 118h, or else with 119h (00A92910 __thiscall(manager, a, b) RET 8 ->
// AL). The element is inserted at end() through 0049E150 (ECX = vector,
// &out, the end iterator by value, the flag) after the same iterator
// construction as 005F6190.
struct OptionsProfileSyncHost : BitVectorClearHost {
    virtual void* input_manager_004bec00() = 0;
    virtual bool input_actions_share_binding_00a92910(void* manager, std::int32_t a, std::int32_t b) = 0;
    virtual void bit_vector_insert_0049e150(void* vector, std::array<std::uint32_t, 3>& out,
                                            const std::array<std::uint32_t, 3>& where, bool value) = 0;
};
void options_sync_profile_values_005f6ef0(void* screen, OptionsProfileSyncHost& host);

// ===========================================================================
// 005F52C0 __thiscall(dst, const src*) RET 4 -> EAX = dst: the options settings
// copy-assignment (00F88980 <-> screen+D8h). Copies the dword fields 4h, Ch,
// 14h, 18h, 20h, 28h..34h, 3Ch, 50h..5Ch, 64h, 68h, 70h, 78h, 80h, 88h, 90h
// and the byte fields 8h, 9h, 10h, 1Ch..1Eh, 24h, 38h, 40h..4Ch, 60h, 6Ch,
// 74h, 7Ch, 84h..86h, 8Ch, 8Dh, 94h, 95h; the two NativeString vectors +98h
// and +A4h through 00543E50 (ECX = dst member, src member); bytes B0h..B2h;
// then the NativeString +B4h unless it is the same object: 0041DD40(src
// length, 1) and, when the source length is not zero, memcpy(dst text, src
// text, dst length). +0h and the padding bytes are not touched.
struct SettingsCopyHost {
    virtual ~SettingsCopyHost() = default;
    virtual void native_string_vector_copy_00543e50(void* dst, const void* src) = 0;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t keep) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
};
void* options_settings_copy_005f52c0(void* dst, const void* src, SettingsCopyHost& host);

struct HintDefinitionAssignHost : SettingsCopyHost {
    // 005FD6F0 __thiscall(dst entry, const src entry*) RET 4: one 18h-byte sub-record.
    virtual void hint_entry_assign_005fd6f0(void* dst, const void* src) = 0;
};
// 005FDD90 __thiscall(dst, const src*) RET 4 -> EAX = dst: the hint definition
// copy-assignment. Ten NativeStrings at +0h..+48h (each: skipped when it is
// the same object, else 0041DD40(src length, 1) and memcpy of dst length
// bytes when the source is not empty), byte +50h, eight 18h-byte entries from
// +54h through 005FD6F0, bytes +114h..+116h, the NativeString +118h, dwords
// +120h and +124h.
void* hint_definition_assign_005fdd90(void* dst, const void* src, HintDefinitionAssignHost& host);

// ===========================================================================
// Multiplayer menu and pause screen pieces.
// ===========================================================================
struct MenuMiscHost {
    virtual ~MenuMiscHost() = default;
    virtual void* online_service_00f8a2fc() = 0;                 // [00F8A2FC]
    virtual void online_open_friends_154(void* service) = 0;     // vtable +154h
    virtual bool online_signed_in_198(void* service) = 0;        // vtable +198h -> AL
    virtual void* multi_menu_00e198b4() = 0;                     // [00E198B4]
    virtual void set_friends_opened_00e19610(std::int32_t value) = 0;
    // 00688AD0 __thiscall(multi menu, interface) RET 4.
    virtual void multi_menu_push_request_interface_00688ad0(void* menu, std::int32_t id) = 0;
    // Imports (stdcall).
    virtual std::int32_t xnet_qos_release_00a4d45e(void* qos) = 0;
    virtual std::int32_t xnet_qos_service_lookup_00a4d458(std::uint32_t flags, void* event, void** out) = 0;
    virtual std::int32_t xnet_get_opt_00a4d452(std::uint32_t option, void* value, std::int32_t* size) = 0;
    // Widget virtuals.
    virtual std::int32_t widget_kind_5c(void* widget) = 0;
    virtual void widget_set_alpha_4c(void* widget, float alpha) = 0;
    virtual void widget_show_88(void* widget, std::int32_t state, std::int32_t zero, float alpha) = 0;
    virtual void widget_check_84(void* widget, bool checked) = 0;          // vtable +84h
    virtual void scroller_end_thumb_drag_00683320(void* scroller) = 0;     // __thiscall
    virtual void* game_00e188a8() = 0;
    virtual void* xenon_system_manager_00f8abe8() = 0;
    virtual void* scene_catalog_00e198ac() = 0;          // [00E198AC]; +5Ch the scene list owner
    virtual std::int32_t scene_count_00f1af2c() = 0;     // [00F1AF2C]
    virtual std::int32_t lobby_mode_00e19564() = 0;      // [00E19564]
    virtual void set_selected_scene_00e19594(std::int32_t index) = 0;
    virtual bool join_set_mode5_flag_00e08700() = 0;     // byte 00E08700
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void* lobby_option_registry_00576b10() = 0;  // cdecl singleton
    // 005C6850 __thiscall(tree, const int* key, const int* mode) RET 8 -> node
    // value; 005C49C0 __thiscall(that) -> byte* (the scene's enabled flag).
    virtual void* int_tree_subscript_005c6850(void* tree, const std::int32_t* key, const std::int32_t* mode) = 0;
    virtual const std::uint8_t* map_subscript_005c49c0(void* value) = 0;
    virtual void game_select_scene_record_004c6890(void* game, std::int32_t index) = 0;
    virtual void session_set_mode_0076fad0(void* session, std::int32_t mode) = 0;
    virtual void* online_session_list_80(void* service) = 0;              // vtable +80h
    virtual bool online_create_session_16c(void* service, void* settings) = 0;  // vtable +16Ch -> AL
    virtual void mode_selector_set_mode_005d5660(std::int32_t mode) = 0;  // __fastcall(ECX = mode)
    virtual std::int32_t game_effective_mode_004bca50(void* game) = 0;
    virtual std::uint8_t default_event_code_00e19618() = 0;   // the byte 00E19618 (widget+F0h null)
    // 00683300 __thiscall(scroller, direction, step, override) RET 0Ch.
    virtual void scroller_set_auto_scroll_00683300(void* scroller, std::int32_t direction, float step,
                                                   std::int32_t override_flag) = 0;
    // 005E2320 __fastcall(ECX = out, EDX = slot) -> the option label (slot 0 the players count).
    virtual std::string lobby_settings_option_label_005e2320(std::int32_t slot) = 0;
    // 00ABAED0 __thiscall(text, const NativeString*, 1) RET 8.
    virtual void text_set_localised_source_00abaed0(void* text, std::string_view key, std::int32_t flag) = 0;
    virtual std::uint8_t session_rank_setting_00574050(void* filter) = 0;    // __thiscall(game+650h) -> AL
    virtual void mode_selector_refresh_server_info_005ebdd0(void* screen) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    virtual std::int32_t lobby_option_value_00e1955c(std::int32_t slot) = 0;
    virtual void set_lobby_option_value_00e1955c(std::int32_t slot, std::int32_t value) = 0;
    // 00575E80 __thiscall(registry+10h, const int* slot) RET 4 -> the option record (+8h the value count).
    virtual void* lobby_option_record_00575e80(void* map, const std::int32_t* slot) = 0;
    virtual void* lobby_option_table_005e1990() = 0;                        // cdecl -> the option tree
    virtual std::int32_t selected_scene_00e19594() = 0;
    virtual std::int32_t scene_index_005d7070(std::int32_t selected) = 0;   // __fastcall(ECX)
    virtual void* scene_list_at_004b4e60(void* list, std::int32_t index) = 0;  // __thiscall RET 4
    // Online service virtuals ([00F8A2FC]).
    virtual bool online_connected_14(void* service) = 0;                    // vtable +14h -> AL
    virtual void online_set_live_mode_1a0(void* service, std::int32_t on) = 0;  // vtable +1A0h
    virtual void online_reset_d4(void* service, std::int32_t arg) = 0;      // vtable +D4h
    virtual void online_reset_d8(void* service) = 0;                        // vtable +D8h
    virtual void online_refresh_38(void* service) = 0;                      // vtable +38h
    virtual void online_set_e0(void* service, std::uint32_t value) = 0;     // vtable +E0h
    virtual std::uint32_t game_value_004bc830(void* game) = 0;              // __thiscall(game) -> EAX
    // 007F9340 __thiscall(profile = game+650h, const NativeString*) RET 4.
    virtual void player_profile_set_display_name_007f9340(void* profile, std::string_view name) = 0;
    virtual void game_request_state_004d7920(void* game, std::int32_t state) = 0;
    virtual bool xenon_has_selected_user_00a3e510(void* manager) = 0;
    virtual std::int32_t xenon_sign_in_state_00a3ead0(void* manager) = 0;
    // 00A3EB30 __thiscall(manager, byte* out) RET 4 -> AL.
    virtual std::uint8_t xenon_communication_privilege_00a3eb30(void* manager, std::uint8_t* out) = 0;
    // 00425D10 then 00531B00 (see OptionsScreenHost::raise_prompt_00531b00).
    virtual void raise_prompt_00531b00(std::int32_t slot, std::string_view message, std::int32_t kind,
                                       std::uint32_t callback, std::int32_t unused, std::string_view title,
                                       float timeout, std::int32_t timeout_result, std::string_view countdown_key,
                                       std::int32_t dismissible) = 0;
};
struct MultiMainOnlineHost : MenuMiscHost {
    virtual void widget_set_scale_48(void* widget, const float* pair) = 0;    // vtable +48h(&{x, y})
    virtual void widget_enable_34(void* widget, std::int32_t value) = 0;      // vtable +34h
    virtual void widget_focusable_60(void* widget, std::int32_t value) = 0;   // vtable +60h
    virtual void multi_main_set_buttons_visible_005e6b10(void* screen, std::int32_t visible) = 0;
    virtual std::int32_t listbox_selected_data_00a9c990(void* listbox) = 0;
    virtual const char* help_table_entry_00e0896c(std::int32_t index) = 0;
    virtual void* command_bar_00e1930c() = 0;
    virtual void command_bar_set_help_line_0054a0c0(void* bar, std::string_view text) = 0;
    virtual void online_sign_out_dc(void* service) = 0;                       // vtable +DCh
};
struct LobbyHintHost : MenuMiscHost {
    // 005D86B0 __thiscall(lobby screen) -> the current scene record (+98h its map name).
    virtual void* lobby_current_scene_005d86b0(void* screen) = 0;
    virtual std::int32_t lobby_submode_00e19568() = 0;
    // 00575E80 (MenuMiscHost) on registry+34h, then 005732C0(that, const int* value) -> const char**.
    virtual const char* const* lobby_option_text_005732c0(void* record, const std::int32_t* value) = 0;
    virtual void text_set_localised_cstring_00abbe50(void* text, const char* key, std::int32_t flag) = 0;
};
inline constexpr std::string_view kCableLostLive = "FE.multi_cablelost_live";   // 00CE7990
inline constexpr std::string_view kRequiresLive = "FE_pc.xsm_requireslive";     // 00CE7CF0
inline constexpr std::string_view kLobbyPlayersTitle = "FE.multi_lobby_players|.: ";  // 00CF2518

// 005E6C70 cdecl: [00F8A2FC] vtable +154h(), 00E19610 = 1, then interface 10h
// pushed on [00E198B4] (00688AD0).
void multi_main_open_friends_005e6c70(MenuMiscHost& host);
// 005EA810 __fastcall(ECX = browser): with an online service that answers
// vtable +198h, release the QoS handle +10h (XNetQosRelease, then +10h = 0)
// when set, XNetQosServiceLookup(0, 0, &+10h), and when that returns 0,
// XNetGetOpt(6, &buffer, &size) with size = 8 (the result is unused).
void multi_session_browser_refresh_qos_005ea810(void* browser, MenuMiscHost& host);
// 005E20A0 cdecl: from game+60Ch modulo [00F1AF2C] (signed IDIV), step
// through the scene list ([[00E198AC]+5Ch]+28h, 434h-byte entries; an index
// past the end calls 00BF6713) until an entry whose lookup answers 1: entries
// with byte +424h use their own tree +428h, the others the lobby option
// registry's +28h; key (2, [00E19564]) through 005C6850 then 005C49C0. At most
// [00F1AF2C] entries are tried, index = (count + index + 1) % count. When the
// result differs from game+60Ch: 00E19594 = index and 004C6890(game, index).
void multi_lobby_select_available_scene_005e20a0(MenuMiscHost& host);
// 005EAC60 __fastcall(ECX = screen), the mode selector's join step. Session
// (game+1EF0h) mode 1; without a signed-in service (vtable +198h) screen+F8h =
// 2. Otherwise the service's session list (vtable +80h) must not be empty
// (00BF6713), mode 5 when byte 00E08700 is set, then the settings block
// game+1FF0h gets +38h = 004BCA50(game), +7Ch = max([00F8ABE8]+35Ch,
// game+740h), +78h = max([00F8ABE8]+358h, game+73Ch) (unsigned), +80h/+84h =
// game+698h/+69Ch, and vtable +16Ch(block) answering 1 sets screen+F8h = 1.
void multi_mode_selector_begin_join_005eac60(void* screen, MenuMiscHost& host);
// 005EA990 __thiscall(listener, widget) RET 4, the mode selector widget
// refresh slot (+0Ch listener). Kind 3: only for event codes A2h / A3h (the
// byte at [widget+F0h], or 00E19618 when null) alpha 0.5 / 1.0 by +D4h. Other
// kinds: with the listener's +D4h set only the widgets +A4h / +A8h go on;
// otherwise +28h / +24h first get 00683300(listener+30h, 0, 0.0f, 0) and +90h
// gets 00683320(listener+30h); then kind 6 vtable +88h(+D4h != 0, 0, 1.0f) or
// kind 12h vtable +84h(+D4h != 0).
void multi_mode_selector_widget_refresh_005ea990(void* listener, void* widget, MenuMiscHost& host);
// 005E4FC0 __fastcall(ECX = lobby screen): +210h gets the localised source
// "FE.multi_lobby_players|.: " + 005E2320(0) through 00ABAED0(text, 1).
void multi_lobby_update_players_title_005e4fc0(void* screen, MenuMiscHost& host);
// 005ECBA0 __thiscall(screen, delta) RET 4, the mode selector's option step.
// Nothing for delta 0 or no selected row (+A8h). step = delta > 0 ? 1 : -1.
// Row 0: +E4h += step clamped to [2, 8], then +E8h = +E4h - 1 when +E8h >=
// +E4h. Row 1: +E8h += step, then +E4h - 1 when >= +E4h, 0 when negative.
// Row 2: +F0h += step; above min(+ECh, 00574050(game+650h) - 1) it is set to
// that bound (00574050 read again), below 0 to 0. Row 3: +ECh += step, capped
// at 13h, raised to +F0h. Rows above 3 change nothing. Every path that does
// not return early ends in 005EBDD0.
void multi_mode_selector_step_option_005ecba0(void* screen, std::int32_t delta, MenuMiscHost& host);
// 005E21B0 __fastcall(ECX = slot), the lobby's option step. Skipped when
// game+1FE4h is 2 or game+5FCh is 0. The value 00E1955C[slot] advances by one
// modulo the record's value count (00576B10 registry +10h, 00575E80, +8h;
// unsigned DIV); when the option table (005E1990) marks (slot, value) enabled
// the step stands. Otherwise up to count further values are tried, each
// checked against the current scene's table (+428h of the 004B4E60 entry of
// [[00E198AC]+5Ch]+24h at 005D7070([00E19594]) when its +424h is set, else
// the registry +28h); none enabled restores the old value. The divisor stays
// the first record's count.
void lobby_settings_step_option_005e21b0(std::int32_t slot, MenuMiscHost& host);
// 005E74B0 cdecl -> AL, the main menu's live-connection gate. Not connected
// (service vtable +14h): prompt slot 2, "FE.multi_cablelost_live", kind 2, no
// callback, empty title, timeout 0.0, dismissible 1; returns 0. Connected:
// vtable +1A0h(1), service byte +4Dh = 0, vtable +D4h(1), vtable
// +E0h(004BC830(game)); the front session of vtable +80h (empty list ->
// 00BF6713) copies +28h/+2Ch into game+650h+48h/+4Ch; game+6A9h = 0,
// game+6A8h = front+38h; 007F9340(game+650h, ""); game states 6 then 7
// (004D7920); returns 1.
bool main_menu_check_live_connection_005e74b0(MenuMiscHost& host);
// 005E76D0 cdecl -> AL, the sign-in gate: unless the Xenon manager has a
// selected user whose sign-in state (00A3EAD0) is 2, prompt slot 2
// "FE_pc.xsm_requireslive" as above and return 0. Otherwise vtable +1A0h(0),
// +D8h(), +38h(); the front session copies +28h/+2Ch as above;
// 007F9340(game+650h, the session's std::string at +0Ch, SSO when +20h < 10h);
// game+650h+59h = 00A3EB30(manager, &scratch); game+6A8h = front+38h; game
// states 6 then 7; returns 1.
bool main_menu_check_live_sign_in_005e76d0(MenuMiscHost& host);
// 005E7900 __fastcall(ECX = multi main menu screen): with a signed-in service
// (vtable +198h) and [+24h]+85h set: when [+24h]+77h is set, [+E8h] vtable
// +48h(&{1.0f, 1.0f}) and [+D4h] vtable +34h(0); then [+20h] +60h(1), +34h(1),
// +77h = 0; [+24h] +60h(0), +34h(0), +77h = 1; 005E6B10(screen, 0); the help
// line 00E0896C[00A9C990([+20h])]. Otherwise vtable +1A0h(2), +DCh(),
// 007F9340(game+650h, ""), game states 9 then 4, +F4h = -1.
void multi_main_refresh_online_state_005e7900(void* screen, MultiMainOnlineHost& host);
// 005E19E0 __thiscall(listener = lobby screen+8h, unused, listbox) RET 8, the
// lobby settings list's selection hint. Only for the settings listbox
// (+220h; while its +85h is set no other widget either), never +8Ch. +26Ch =
// the selected row (00A9C920; -1 ends). By the row's kind (+1B4h[row]):
// 1 the map description: a copy of the scene's name (005D86B0(screen)+98h)
// replaced by "mp.hint_ic_desc_map" for modes 0..3 (00E19564, when 00E19568 is
// 0..3) or "mp.hint_duel_desc_map" for mode 4, or extended by
// "_escort_desc_map" / "_siege_desc_map" / "_comp_desc_map" for modes 5 / 6 /
// 7, set through 00ABAED0(text, 1) on +224h; 0Dh "FE.multi_weather_desc";
// 8 with mode <= 3 "FE.multi_victorypointlimit_desc"; anything else the text
// of value 00E1955C[kind] in the registry's +34h records (00575E80, 005732C0),
// each through 00ABBE50(text, 1) on +224h.
void lobby_show_selection_hint_005e19e0(void* listener, void* listbox, LobbyHintHost& host);
// 005FCAE0 __thiscall(listener, widget) RET 4, the pause screen's widget
// refresh slot: kind 3 alpha 0.5 / 1.0 by widget+D4h; otherwise, when the
// widget is the listener's +70h, 00683320 on listener+90h, then kind 6
// vtable +88h(+D4h != 0, 0, 1.0f) or kind 12h vtable +84h(+D4h != 0).
void hud_pause_on_widget_refresh_005fcae0(void* listener, void* widget, MenuMiscHost& host);

}  // namespace bsp::seg23_cc13
