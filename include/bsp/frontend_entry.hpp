#pragma once

#include "bsp/gameplay_effect_manager.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/frontend_states.hpp"

// Entry into the main-menu shell: BSP_Game_EnterFrontEndShell (004e4000), the
// handler the state-request drain selects for request 4, and the loading screen
// it raises while the front-end resources load (0057cb60, 0057bec0, 0057c250,
// 0057cff0, 0057d0d0). Evidence and uncertainties: docs/GAME_FRONTEND_ENTRY.md.
//
// The one finding that changes the state map: game state 4 never survives this
// call. GGame::OnInit (004e3aa0) writes game+5D4h = 3 as its first instruction,
// and the shell settles on 5 at 004e4279. State 4 is the request value only.
namespace bsp {

// The two VFS file-block labels 004e4000 builds at 004e407f..004e40ab. These are
// the only "SLM_" strings in the image, so the loading state machine named in
// them has exactly these two front-end modes. The label is passed to
// BSP_FileBlock_Construct (00be0a30), which opens a named VFS block for the
// duration of the load, so it is a resource-grouping scope, not a state field.
enum class FrontEndLoadBlock {
    Cold, // "GILoading::SLM_LOAD_FRONTEND", 00CE8254; game+719Ch clear
    Return, // "GILoading::SLM_LOAD_FRONTEND_RETURN", 00CE8274; game+719Ch set
};
const char* front_end_load_block_label(FrontEndLoadBlock block) noexcept;

// How 004e407f..004e40ab resolves the load. mission_init_done is game+719Ch, the
// once guard docs/GAME_FRONTEND_STATES.md identifies. The skip arm is the reason
// the two conditions are not a plain if/else: when the guard is set and both
// front-end managers are still alive, the whole block at 004e40b0..004e412b is
// jumped over at 004e409c, so no file block is opened and no loading screen is
// raised. The managers are the ones step 8 of enter_front_end_shell creates.
enum class FrontEndLoadDecision {
    LoadCold, // 004e40ab
    LoadReturn, // 004e40a4
    SkipAlreadyResident, // 004e409c -> 004e412d
};
FrontEndLoadDecision decide_front_end_load(
    bool mission_init_done, bool manager_b8_present, bool manager_ac_present) noexcept;

// Mode argument of 0057cb60. Native __fastcall with ECX holding the mode, not a
// this pointer: 0057cb7a moves ECX to EBP and the body never dereferences it.
// 004e4000 passes 0 (XOR ECX,ECX at 004e4114). Any other value creates the
// screen and starts its render worker without configuring the elements.
enum class LoadingScreenMode : int {
    MenuBackground = 0, // loads interface/textures/menu.ats, 0057cccc
    LoadingImage = 1, // draws the picked mp.loading_NN image, 0057cd6a
};

// The global loading-screen configuration block at 00E08780, 0057cff0's target.
// 00E08780+0h is never written on this path and is not identified. The strings
// are the native NativeString {int length; char* data;} pair at 00E08790/94.
struct LoadingScreenConfig {
    std::vector<std::string> images{}; // +4h..+0Ch, a vector of 8-byte strings
    std::string picked{}; // +10h/+14h, the image 0057cb60 mode 1 binds
    bool image_visible{false}; // +18h -> 00E08798, the mode 1 visibility argument
};

// 0057d0d0. __thiscall, ECX = an uninitialised local of this shape, RET 0,
// returns this in EAX. Seeds the three front-end loading images and copies the
// live value of 00E08798 back into +18h, so publishing it preserves whatever
// visibility the previous load left behind. 004e4000 destroys it at 004d2a80.
LoadingScreenConfig default_front_end_loading_config(bool current_image_visible);

// 0057cff0. __thiscall, ECX = the config above, RET 0. Assigns the vector
// through 00506c80, resizes the global string and memcpy's the picked name, then
// stores the byte. The self-assignment guards at 0057d002 and inside 00506c80
// make publishing the globals onto themselves a no-op.
void publish_loading_screen_config(LoadingScreenConfig& globals, const LoadingScreenConfig& source);

// The 0x58-byte loading-screen singleton at 00E194B4, built by 0057c1c0. It
// derives from FrontEndScreen (004f7180 stores vtable 00CEAE54 and clears the
// two flag bytes; 0057c1c0 overwrites +0h with 00CEF264) and from a singleton
// lifetime hook at +8h whose constructor 0057bff0 is what assigns the global,
// under the lifetime manager's critical section. Fields +1Ch..+28h are four GUI
// element pointers that vtable +10h fills; only their use is recovered.
struct LoadingScreen {
    FrontEndScreen base{}; // +4h wanted, +5h active
    std::int32_t progress_units{0}; // +2Ch, signed max of converted incoming*128
    float progress{0.0f}; // +30h, incoming wins when equal or unordered
    LoadingScreenMode mode{LoadingScreenMode::MenuBackground};
    bool configured{false}; // false when the mode matched neither 0 nor 1
    // Stands for "00E194B4 is not null" at 0057cc8d, which is the allocation
    // test, not a field of the object. 0057c250 nulls the global, so every
    // enter_front_end_shell finds it false and rebuilds the screen.
    bool created{false};
};

// Required existing CRT BF7420 dispatch. Input is x87 ST0, not a C++ argument;
// consume exactly that input and return native EAX. ECX identifies the ACTUAL
// current0109EEA4 global, which the service must read at conversion time:
// nonzero FSTP double/CVTTSD2SI; zero existing BF7456 x87 conversion. Preserve
// native control/status/exception effects. Never substitute a clock or cast.
using LoadingProgressConvertSt0 = std::int32_t (__fastcall*)(
    const volatile std::uint32_t* actual_0109eea4) noexcept;
struct LoadingProgressCrtAccess {
    const volatile double* scale_00cef258; // Actual shipped value128.0.
    const volatile std::uint32_t* sse2_conversion_0109eea4;
    // Same concrete converter as GUI bounds; override only with an equivalent
    // original-ABI provider. Actual scale/global bindings remain required.
    LoadingProgressConvertSt0 convert_st0_00bf7420{native_crt_truncate_st0_00bf7420};
};

// Whole0057BEC0..0057BF0F; original stdcall(float), RET4, null global no-op.
// Preserve old FLD/FSTP, incoming FLD, old FLD/FCOMIP, JBE selecting incoming
// for equal/unordered values; +30 is stored BEFORE conversion. ST0 still holds
// the ORIGINAL incoming, multiplied by actualCEF258; +2C is signed max(old,
// converted). These fields need not agree for NaNs or decreasing reports.
// New C++ service ABI; no BF7420 kernel duplication or actual screen overlay.
// Null screen bypasses all service validation and performs no floating work.
void report_loading_progress(LoadingScreen* screen, float progress,
    const LoadingProgressCrtAccess&);

// One method per native call site the loading screen reaches. Nothing here has a
// default: none of it stands in for unrecovered behaviour.
struct LoadingScreenHost {
    virtual ~LoadingScreenHost() = default;
    // 00F8D394 vtable +0Ch then +14h. 0057cb60 runs the pair twice, at
    // 0057cb7c..0057cbb0, so both swap-chain buffers are cleared and presented.
    virtual void renderer_begin_frame() = 0;
    virtual void renderer_end_frame() = 0;
    virtual void renderer_begin_worker_mode() = 0; // 00F8D394 vtable +1Ch, 0057ce9d
    virtual bool game_object_present() = 0; // 00E188A8, checked at 0057cbb2
    virtual void session_suspend() = 0; // 0076c190, ECX = game+1EF0h
    virtual void session_resume() = 0; // 0076c230, same object, from 0057c250
    virtual void gui_set_enabled(bool enabled) = 0; // 004c12b0 then 00aa0e00
    virtual void gui_update(float seconds, int mode) = 0; // 004c12b0 then 00aa4f80
    virtual void gui_refresh() = 0; // 004c12b0 then 00aa2b80, 0057c250 only
    virtual void screen_registry_commit() = 0; // 004f83b0
    virtual void screen_pump_reset() = 0; // 004f8ac0, ECX = 00E18D48
    // 00E19698, the screen that is torn down before the loading screen appears:
    // vtable +1Ch when its active byte is set, then both bytes cleared.
    virtual void close_previous_screen() = 0;
    virtual void load_texture_atlas(const char* path) = 0; // 00af0060, ECX = 00F8C26C
    virtual void create_screen() = 0; // operator new(0x58) then 0057c1c0
    virtual void screen_init() = 0; // vtable +10h, builds the GUI elements
    virtual void screen_show() = 0; // vtable +18h
    virtual void screen_stop_worker() = 0; // vtable +1Ch, when the active byte is set
    virtual void screen_destroy() = 0; // vtable +0Ch with 1, then null the global
    virtual void lifetime_unregister() = 0; // 00415350 then 00bcfca0 on screen+8h
    // 004c1ac0 then 00518250, both with the same pair. (2,1) for mode 0 at
    // 0057cd2b, (1,1) for mode 1. The pair's meaning is not recovered.
    virtual void select_menu_layout(int layout, int flag) = 0;
    virtual void element_set_image(int element, int index, float alpha) = 0; // vtable +88h
    virtual void element_set_visible(int element, bool visible) = 0; // vtable +34h
    virtual void element_set_rect(int element) = 0; // vtable +58h, mode 1 only
    virtual void bind_loading_image(const std::string& name) = 0; // 00abaed0(&00E08790, 1)
    virtual float element_aspect_source() = 0; // *(screen+24h)+114h
    virtual void prepare_extra_elements() = 0; // 0057c990
    // 00aa4040: render queue mode 1, descriptor prepare, mode 2, then the worker
    // with callback 0057ca00, context screen+10h and rate 19h (25).
    virtual void start_render_worker(int rate) = 0;
};

// Native rate argument at 0057ce9d.
inline constexpr int kLoadingScreenWorkerRate = 0x19;

// 0057cb60. Tears the previous screen down, loads interface/textures/allbutingame.ats,
// creates the singleton if absent, configures it for the mode, and starts the
// render worker that animates it while the caller blocks. mode 1 needs the
// published globals, which is why 004e4000 publishes the config first.
void begin_loading_screen(LoadingScreen& screen, LoadingScreenMode mode,
    const LoadingScreenConfig& globals, LoadingScreenHost& host);

// 0057c250. __cdecl(void), RET 0. Re-enables the GUI, resumes the session tick,
// stops the worker, unregisters the lifetime hook and deletes the screen. Runs
// on both exits of 004e4000, so the screen never outlives the call.
void end_loading_screen(LoadingScreen* screen, LoadingScreenHost& host);

// Fields of the game object (00E188A8) that 004e4000 reads or writes.
struct FrontEndShellState {
    std::int32_t state{0}; // +5D4h; 4 on entry, 3 after OnInit, 5 on success
    bool mission_init_done{false}; // +719Ch, the once guard that picks the label
    bool lua_vm_ready{false}; // *(game+1A08h)+4h != 0
    bool network_quit_pending{false}; // +216Dh, cleared at 004e440d
    bool award_ga_hm_recorded{false}; // "GA_HM" present in the map at game+6F0h
};

// Native game+5D4h values this packet establishes. 4 and 3 are only ever seen
// inside 004e4000; 5 is what the next frame observes. The drain's table in
// docs/GAME_FRAME_CONTROL.md lists request 5 as "no call", so 5 is a resting
// state rather than a request the drain would re-dispatch.
inline constexpr std::int32_t kGameStateFrontEndRequest = 4; // GameFrontEndState::FrontEndShell
inline constexpr std::int32_t kGameStateFrontEndInit = 3; // written by 004e3aa0
inline constexpr std::int32_t kGameStateFrontEndShellReady = 5; // written at 004e4279

// The float at 00D7A2F0 the shell reports at 004e4130, before OnInit and on both
// the load and the already-resident arms.
inline constexpr float kFrontEndInitialProgress = 0.1f;

// The single-award key at 00CE824C, referenced only by 004e4000.
inline constexpr const char* kFirstMainMenuAwardKey = "GA_HM";
inline constexpr int kAwardIdMin = 1; // 004e437a..004e4380, LEA/CMP 62h/JA
inline constexpr int kAwardIdMax = 99;

// One method per native call site 004e4000 reaches, in body order.
struct FrontEndShellHost {
    virtual ~FrontEndShellHost() = default;
    // 004e4022..004e4062. 00F8D394+18h = 20000000h, then three labelled memory
    // probes. The sound one (00a7a460) only samples FMOD::Memory_GetStats and
    // discards both results and the label, so these are stripped instrumentation.
    virtual void renderer_set_budget(std::uint32_t value) = 0;
    virtual void probe_texture_memory(const char* label) = 0; // vtable +70h
    virtual void probe_sound_memory(const char* label) = 0; // 00a7a460
    virtual GameplayEffectManagerContext& effect_manager_context() = 0;
    virtual bool title_screen_present() = 0; // 00E198C8
    virtual void destroy_title_screen() = 0; // vtable +0h with 1, then null it
    virtual bool front_end_manager_b8_present() = 0; // 00E198B8
    virtual bool front_end_manager_ac_present() = 0; // 00E198AC
    virtual bool front_end_manager_b4_present() = 0; // 00E198B4
    virtual void open_load_block(const char* label) = 0; // 00be0a30 with flag 1
    virtual void close_load_block() = 0; // 00bdcb30
    virtual LoadingScreenConfig& loading_globals() = 0; // 00E08780
    virtual void begin_loading(LoadingScreenMode mode) = 0; // 0057cb60
    virtual void report_progress(float progress) = 0; // 0057bec0
    virtual void end_loading() = 0; // 0057c250
    virtual void game_on_init() = 0; // 004e3aa0, ECX = 00E188A8; writes 5D4h = 3
    virtual void poll_platform_session_events() = 0; // 004db290, ECX = game
    virtual std::int32_t game_state() = 0; // game+5D4h, re-read at 004e4151
    // Each is operator new of the listed size, the constructor when the
    // allocation succeeded, the store into the global, then vtable +4h. The
    // native code takes the vtable through the global unconditionally, so a
    // failed allocation dereferences null at 004e41a3, 004e41e3 and 004e4222.
    // That fault is a property of the original code, not modelled here.
    virtual void create_manager_b8() = 0; // 4Ch, 00689800; drain 16h destroys it
    virtual void create_manager_ac() = 0; // 78h, 00686170; drain 06h destroys it
    virtual void create_manager_b4() = 0; // 68h, 006887E0; drain 09h destroys it
    virtual void lua_collect_garbage() = 0; // 006b8ad0("collectgarbage(\"collect\")",0,0,2)
    virtual int manager_ac_mode() = 0; // *(00E198AC)+4h
    virtual void reset_manager_ac_mode() = 0; // 004cc460(1,0) when the mode is not 4
    virtual void manager_ac_enter() = 0; // vtable +8h
    virtual void manager_ac_start_sub() = 0; // 005884a0, ECX = *(00E198AC)+58h
    virtual bool post_state_hook_wanted() = 0; // 0067d6e0
    virtual void post_state_hook() = 0; // 004bfc70, ECX = game; drain 07h runs it too
    virtual bool award_gate_open() = 0; // 0090c5d0, ECX = *(00E188A8)+21A0h
    virtual int award_id(const char* key) = 0; // 006b8da0, ECX = 00E19900
    virtual bool award_system_ready() = 0; // 00a3e520, ECX = 00F8ABE8
    virtual bool award_session_ready() = 0; // 004b44f0, ECX = 00F8ABE8
    virtual void grant_award(int id) = 0; // 00a410a0, ECX = 00F8ABE8
    virtual void record_award(const char* key, int value) = 0; // 007fbe20, ECX = game+650h
    virtual void send_network_quit() = 0; // 0076fad0(0), ECX = game+1EF0h
};

// How 004e4000 left.
enum class FrontEndShellOutcome {
    // 004e4159. The platform poll moved game+5D4h off the 3 that OnInit wrote,
    // so the shell is abandoned: the loading screen comes down and the managers,
    // the Lua collection, the state write and the award block are all skipped.
    AbortedByPlatformEvent,
    // 004e43e7. game+5D4h is 5 and the front-end managers exist.
    ShellReady,
};

// 004e4000. __thiscall, ECX = the game object, no stack arguments, RET 0, no
// return value; the drain at 004e44fa ignores it. Runs to completion inside one
// call: the front-end resources load synchronously on this thread while the
// render worker started by begin_loading_screen animates the loading screen.
FrontEndShellOutcome enter_front_end_shell(FrontEndShellState& state, FrontEndShellHost& host);
}
