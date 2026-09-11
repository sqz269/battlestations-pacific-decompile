#pragma once
#include <cstdint>

#include "bsp/frontend_states.hpp"

// Press-start screen of the title state (`FE_initial`).
//
// Addresses: 0067c840 construct, 0067c860 screen id, 0067c870 exit,
// 0067ca60 scalar deleting destructor, 0067c9e0 destructor, 0067ca80 register,
// 0067cb40 enter, 0067cfb0 update, 0067d860 collect-layouts, 0067cc60 sign-in
// handler, 0067cf50 invite fast path, 0067c880 device button probe,
// 0067ca40 storage completion callback. Vtable 00CF6D90.
//
// Every name below is a hypothesis, not a recovered symbol.
namespace bsp {

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------

// Vtable slot +0h is `mov eax, 5Ch; ret` at 0067c860. 004f71d0 uses the value as
// the index into the 95-slot registry 00E18B60, so the screen owns 00E18CD0.
// It is above the 1..1Fh interface-id map in docs/FRONTEND_SCREEN_SETS.md, so
// this screen is never raised through a screen-set request; 0068d8d0 constructs
// and enters it directly.
inline constexpr int kPressStartScreenId = 0x5C;

// GUI layout loaded by the register slot into screen+8h (0067ca80).
inline constexpr const char* kPressStartLayoutName = "FE_initial";

// Element resolved out of that layout by the enter slot into screen+0Ch
// (0067cb40, literal at 00CF6DC4).
inline constexpr const char* kPressStartElementName = "press_start_Text";

// Input action index pushed to 004c43c0 at 0067d2dd. The action table entry is
// not identified, so this is a raw index, not a named control.
inline constexpr int kPressStartInputAction = 0x4E;

// Localisation keys of the two prompts this screen raises.
// 00CE7C38, slot 2, kind 2: the signed-in profile changed underneath the title.
inline constexpr const char* kProfileChangedMessageKey = "FE_xbox.xsm_profilechanged";
// 00CF6DD8, slot 0, kind 3: the save device is busy or unavailable.
inline constexpr const char* kSavingDeviceMessageKey = "globals.saving_xbox";

// sprintf format at 00CF3AE4; the 64-bit id from 00a3e5d0 becomes the save name.
inline constexpr const char* kSaveNameFormat = "%llx";

// Prompt-alpha pulse constants read by 0067cfc8..0067d005.
inline constexpr double kPromptPulseRate = 2.5; // 00CE3DE0
inline constexpr double kPromptPulsePeriod = 6.283185307179586; // 00CE3828
inline constexpr float kPromptPulseRgb = 1.0f; // 00D7A24C

// ---------------------------------------------------------------------------
// Object layout
// ---------------------------------------------------------------------------

// The 18h allocation 0068d8d0 hands to 0067c840. +0h is the vtable, +4h/+5h come
// from BSP_FrontEndScreen_Construct 004f7180, and the constructor clears +10h
// and +14h. Sizes are fixed by the accesses listed per field.
struct PressStartScreen {
    FrontEndScreen base{}; // +4h wanted, +5h active
    // +8h. Handle returned by 00aa5840 in the register slot and released
    // through 00aa31f0 in the destructor. Slot +24h publishes it.
    std::uint32_t layout{0};
    // +0Ch. Element handle from 00aa7e00; the update calls its vtable +50h
    // (colour) and +34h (a bool) every frame.
    std::uint32_t prompt_element{0};
    // +10h. 0 until the phase-2 arm runs the sign-in handler once (0067d45b).
    std::int32_t sign_in_handled{0};
    // +14h. Set when a message screen appears while the sign-in handler is
    // running (0067d584) and consumed at 0067d541 to close it and leave.
    bool close_prompt_pending{false};
};

// ---------------------------------------------------------------------------
// XenonSystemManager handshake
// ---------------------------------------------------------------------------

// Value of manager+28h, read by 00a3e500 and written by 00a3f3d0 (to 1) and by
// the manager pump 00a40510 at 00a4096f (to 2, alongside its own state 7).
enum class SignInPhase : int {
    Idle = 0,        // nothing requested; the screen waits for a press
    Requested = 1,   // 00a3f3d0 asked for a sign-in and the blade may be up
    Completed = 2,   // a user is selected; the screen may build the profile
};

// The five title-scope bytes the update owns. They are globals in the binary,
// not fields of the screen, so they survive a destroy/construct cycle.
struct PressStartGlobals {
    // 00E19880, a dword tested for non-zero. docs/GAME_TITLE_INIT.md fixes it as
    // the invite/join-in-progress flag; when set, 0067cf50 takes over the frame.
    bool invite_pending{false};
    // 00E1987C. 0067c970 sets it when it re-creates the primary input binding.
    bool primary_binding_valid{false};
    // 00E19884. A press has already been consumed by the message screen; the
    // screen waits for the release before accepting another.
    bool press_consumed_by_prompt{false};
    // 00E19885. Set while the phase is Requested; the next Idle frame rebinds
    // the primary input and consumes itself.
    bool sign_in_blade_was_up{false};
    // 00E08CC0. Set after the sign-in handler ran; re-requests a sign-in on a
    // later idle frame once no message screen is up.
    bool resume_sign_in{false};
};

// Phase accumulator 00E19888, shared by every instance of the screen.
struct PromptPulse {
    float phase{0.0f};
};

// 0067cfc8..0067d00b. phase = fmod(phase + dt * 2.5, 2*pi) computed in x87
// double, stored back as a float; the returned alpha is |sin(phase)| taken by
// masking the sign bit at 0067d026, so a NaN phase would keep its payload.
float advance_prompt_pulse_0067cfc8(PromptPulse& pulse, float delta_seconds) noexcept;

// ---------------------------------------------------------------------------
// Update outcome
// ---------------------------------------------------------------------------

// What one update frame decided. The native returns void; this names the exit
// path so a caller can see which arm ran.
enum class PressStartOutcome {
    Idle,                 // the prompt pulsed and nothing else happened
    InviteHandled,        // 0067cf50 owned the frame
    ProfileChangedPrompt, // the profile-changed prompt was raised
    SignInRequested,      // 00a3f3d0 was called for a pad
    WaitingForSignIn,     // phase Requested
    SignInApplied,        // the sign-in handler ran
    OfflineStart,         // no user selected; the offline bring-up ran
    PromptClosed,         // a message screen was dismissed
    HandedOffToMainMenu,  // BSP_TitleScreen_Skip requested state 4
};

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site the screen reaches, in body order. There are
// no default implementations: nothing here stands in for unrecovered behaviour.
struct PressStartScreenHost {
    virtual ~PressStartScreenHost() = default;

    // --- GUI layout and element, 00aa5840 / 00aa7e00 / 00aa31f0 -------------
    // 0067ca80: BSP_GuiManager_GetOrCreate 004c12b0 then 00aa5840(name, 1, 0).
    virtual std::uint32_t load_layout(const char* name) = 0;
    // 0067cb40: 00aa7e00(name, 1) on the layout at screen+8h.
    virtual std::uint32_t find_element(std::uint32_t layout, const char* name) = 0;
    // 0067c9e0: 00aa31f0 on the GUI manager with the layout handle.
    virtual void release_layout(std::uint32_t layout) = 0;
    // 004f71d0, the base register slot 0067ca80 chains first.
    virtual void register_screen(int screen_id) = 0;
    // 0067cb9f: 00518d60(0, 0, -1, "") clears the front-end page context.
    virtual void clear_page_context() = 0;
    // 0067d860 slot +24h: 004d6790(sink, screen+8h) appends the layout to the
    // list 004f83b0 walks.
    virtual void publish_layout(void* sink, std::uint32_t layout) = 0;

    // --- the pulsing prompt element ----------------------------------------
    // Element vtable +50h with {r, g, b, a}.
    virtual void set_element_color(std::uint32_t element, float r, float g, float b, float a) = 0;
    // Element vtable +34h with one bool.
    virtual void set_element_flag(std::uint32_t element, bool value) = 0;

    // --- XenonSystemManager 00F8ABE8 ---------------------------------------
    virtual SignInPhase sign_in_phase() = 0;             // 00a3e500, manager+28h
    virtual bool profile_change_pending() = 0;           // 00a3e3b0
    virtual bool has_selected_user() = 0;                // 00a3e510, manager+119h
    virtual bool user_operation_busy() = 0;              // 00a3e540, manager+120h
    virtual bool sign_in_blocked() = 0;                  // manager+3E8h
    virtual int invitee_slot() = 0;                      // 00a3e470
    virtual void request_sign_in(int pad_index) = 0;     // 00a3f3d0 -> 00a3f100
    virtual void reset_sign_in_state() = 0;              // 00a40020

    // --- input -------------------------------------------------------------
    // 004c43c0, the reviewed edge test in bsp::action_pressed_this_frame_004c43c0.
    virtual bool action_pressed_this_frame(int action) = 0;
    // 004ba6d0([00F8BBF4], 2, 0): element 0 of the checked device vector at
    // +B8h, or null when no device is attached.
    virtual void* primary_device() = 0;
    // 0067c880: device vtable +20h with 0, then with 0Eh; true if either holds.
    virtual bool device_button_held(void* device) = 0;
    // Device vtable +34h, the pad index handed to request_sign_in.
    virtual int device_pad_index(void* device) = 0;
    // 0067c970, re-creates the primary input binding and sets 00E1987C.
    virtual void rebind_primary_input() = 0;
    // 004b43b0 on [00F8BBF4]: a rising-edge latch cached at +DDh.
    virtual bool input_edge_latch() = 0;

    // --- the shared menu command screen 00425d10 ---------------------------
    virtual bool menu_screen_active() = 0;               // singleton +5h
    virtual bool menu_command_pending() = 0;             // +188h or +218h
    virtual void clear_prompt_slot(int slot) = 0;        // 00532a20
    virtual void clear_all_prompt_slots() = 0;           // 00530650, slots 0..6
    virtual void close_menu_screen() = 0;                // 004b6e50
    // 00531b00(slot, key, kind, 0, 0, "", 0.0f, 0, "", 1).
    virtual void raise_prompt(int slot, const char* message_key, int kind) = 0;

    // --- the profile block at game+650h -------------------------------------
    virtual void reset_profile_007fdb20() = 0;                  // 007fdb20
    virtual void reset_award_tracker_004374f0() = 0;            // 00425c20 then 004374f0
    virtual const char* signed_in_gamertag() = 0;               // 00a3eae0
    virtual void set_profile_name(const char* name) = 0;        // 007f9290
    virtual void set_profile_display_name(const char* name) = 0;// 007f9340
    // 00a3eb00: the 64-bit value at manager+110h+index*8.
    virtual std::uint64_t signed_in_xuid() = 0;
    // Stored at profile+48h/+4Ch, which is game+650h+48h = game+698h.
    virtual void set_profile_xuid(std::uint64_t xuid) = 0;
    // 00a3e5d0: a separate id derived from the pad at manager+3B8h. This, not
    // the XUID above, is what "%llx" formats into the save name.
    virtual std::uint64_t save_id() = 0;
    virtual void reset_save_manager_00bd3450() = 0;             // 00bd3450 on 0109CECC
    virtual void commit_profile_007fae70() = 0;                 // 007fae70

    // --- the save/storage manager at 0109CECC -------------------------------
    virtual bool storage_device_required() = 0;   // manager+21h
    virtual bool storage_busy() = 0;              // manager+8h != 0
    // Manager vtable +1Ch with (save name, 1); exact predicate remains external.
    virtual bool storage_query_1c(const char* save_name) = 0;
    // Manager vtable +14h with (save name); exact predicate remains external.
    virtual bool storage_query_14(const char* save_name) = 0;
    // 007ff100(profile, save name, 0067ca40): read/restore route.
    virtual void request_read_007ff100(const char* save_name) = 0;
    // 007fa710(profile, save name, 0067ca40, 0): conditional write route.
    virtual void request_write_007fa710(const char* save_name) = 0;

    // --- the offline bring-up in the phase-2 arm ---------------------------
    // 00425c20/004374f0, then the four 00F88980 calls 008d4820, 008d41c0,
    // 008d41f0 and 008d4520(0, 1), then BSP_Settings_ApplyAll 008d5b50.
    virtual void reset_offline_profile() = 0;
    // BSP_InputSettings_GetSingleton 005547d0 then 006ac030.
    virtual void apply_input_settings() = 0;
    // [00F8BBF4] vtable +0Ch then +8h; the +8h result feeds 008d44c0.
    virtual void refresh_input_bindings() = 0;
    // 00698a10 on the input manager + 3Ch.
    virtual void commit_input_manager() = 0;

    // --- frame tail ---------------------------------------------------------
    // 004c1e90 (0x50-byte singleton at 00E17664) then 00427190 with 0. Runs on
    // every return path of the idle arm.
    virtual void frontend_tail_event(int code) = 0;
    // 0068d8a0 BSP_TitleScreen_Skip: enqueues state request 4 when game+5E8h is
    // clear, then clears game+5ECh.
    virtual void request_main_menu_state() = 0;
};

// ---------------------------------------------------------------------------
// The virtuals
// ---------------------------------------------------------------------------

// 0067c840, __thiscall, ECX = an 18h allocation, no stack arguments, RET,
// returns the object in EAX.
PressStartScreen& construct_press_start_screen_0067c840(PressStartScreen& screen) noexcept;

// 0067ca80, vtable +10h, __thiscall, no stack arguments, RET, SEH 00C7CF78.
void register_press_start_screen_0067ca80(PressStartScreen& screen, PressStartScreenHost& host);

// 0067cb40, vtable +18h, __thiscall, no stack arguments, RET, SEH 00C7CFA0.
void enter_press_start_screen_0067cb40(PressStartScreen& screen, PressStartScreenHost& host);

// 0067c870, vtable +1Ch, a bare RET: leaving the screen does nothing.
void exit_press_start_screen_0067c870(PressStartScreen& screen) noexcept;

// 0067d860, vtable +24h, __thiscall, one stack argument, RET 4.
void collect_press_start_layouts_0067d860(const PressStartScreen& screen, void* sink,
    PressStartScreenHost& host);

// 0067c9e0, __thiscall, no stack arguments, RET, SEH 00C7CF58. The scalar
// deleting destructor 0067ca60 (vtable +0Ch, RET 4) wraps it.
void destruct_press_start_screen_0067c9e0(PressStartScreen& screen, PressStartScreenHost& host);

// 0067cf50, __thiscall, no stack arguments, RET, returns AL. False means the
// caller should run the normal press-start frame.
bool run_invite_fast_path_0067cf50(PressStartScreen& screen, PressStartGlobals& globals,
    PressStartScreenHost& host);

// 0067cc60, __thiscall, ECX = the screen, no stack arguments, RET. Runs once the
// manager reports SignInPhase::Completed with a user selected.
PressStartOutcome apply_sign_in_0067cc60(PressStartScreen& screen, PressStartScreenHost& host);

// 0067cfb0, vtable +20h, __thiscall, one float stack argument, RET 4,
// SEH 00C7D018. The whole frame of the press-start screen.
PressStartOutcome update_press_start_screen_0067cfb0(PressStartScreen& screen,
    PressStartGlobals& globals, PromptPulse& pulse, float delta_seconds,
    PressStartScreenHost& host);
}
