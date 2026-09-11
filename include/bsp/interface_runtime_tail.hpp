#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/frontend_managers.hpp"
#include "bsp/main_menu_screens_runtime.hpp"

// The tail of the interface runtime around the state-5 front-end pump (packet
// cc_interface_runtime). Evidence, ABI and uncertainties:
// docs/INTERFACE_RUNTIME_TAIL.md.
//
// docs/MAIN_MENU_SCREENS_RUNTIME.md closed 004C40F0 with three named unknowns.
// This header supplies bodies for all three:
//
//   1. 005B6960, the routine 004C40F0 tail-jumps to at 004C42EF. Its receiver is
//      the registry slot 33h screen, its `+BCh` child is that screen's
//      `Blackout_Icon` widget, and the two virtuals it drives are the widget's
//      set-visible `+34h` and set-colour `+50h`. The effect is a full-screen
//      opaque black fill at the end of a mission.
//   2. 005CD1A0, the only routine that raises the interface lock 00E19894. It
//      belongs to the registry slot 37h movie screen and raises the lock as part
//      of pushing interface 2Ch, the one id the lock exempts.
//   3. 00E19698, the process-lifetime `_PleaseWait` screen. Game::OnInit builds
//      it at 004E3DBB and only the two shutdown paths clear it, which is why
//      004C41BD may dereference it without a null test.
//
// What is already reconstructed elsewhere and is NOT repeated here: the lock
// itself (`FrontEndInterfaceLock`, `front_end_request_rejected_00683e90` in
// bsp/frontend_managers.hpp), its four-way decision
// (`classify_interface_request_00683e90` in bsp/main_menu_screens_runtime.hpp),
// and the whole of 0068ACA0 (bsp/ingame_interface.hpp).
//
// Names are hypotheses, not recovered symbols. Nothing here is binary
// compatible with the original.
namespace bsp {

// ---------------------------------------------------------------------------
// 1. 005B6960, the mission-end blackout
// ---------------------------------------------------------------------------

// Registry slot 33h, manager offset +A4h, constructor 005BA7B0, vtable
// 00CF0ED8, E8h bytes. Its register virtual 005BB130 loads `GUI_blackout`,
// `GUI_subtitle` and `GUI_narrative` (docs/HUD_SCREEN_PAGES.md) and binds the
// `Blackout_Icon` widget into +BCh at 005BB983.
inline constexpr int kHudNarrativeScreenRegistrySlot = 0x33;
inline constexpr std::uint16_t kHudNarrativeScreenManagerOffset = 0x00A4;
inline constexpr std::uint32_t kHudNarrativeScreenVtable = 0x00CF0ED8u;
inline constexpr std::size_t kHudNarrativeScreenSizeBytes = 0xE8;
inline constexpr std::uint16_t kHudNarrativeBlackoutIconOffset = 0x00BC;
inline constexpr std::string_view kHudNarrativeBlackoutPage = "GUI_blackout";   // 00CF113C, 0Ch chars, loaded into this+20h at 005BB1AC
inline constexpr std::string_view kHudNarrativeBlackoutWidget = "Blackout_Icon"; // 00CF106C, 0Dh chars, looked up in that page at 005BB97E

// The widget virtuals 005B6960 drives. +34h is the set-visibility slot named in
// docs/FRONTEND_PROMPT_SCREEN.md; +50h is the set-colour slot named in
// docs/GUI_TEXT_WIDGET.md and docs/LOADING_SCREEN_ELEMENTS.md.
inline constexpr std::uint16_t kGuiWidgetSetVisibleVirtual = 0x0034;
inline constexpr std::uint16_t kGuiWidgetSetColourVirtual = 0x0050;

// The four floats 005B6960 writes at 005B6978..005B6998: three zeroes and the
// constant at 00D7A24C, which is 1.0f. Opaque black.
struct BlackoutFillColour {
    float red{0.0f};
    float green{0.0f};
    float blue{0.0f};
    float alpha{1.0f};
};

// One method per native call site 005B6960 makes.
struct BlackoutOverlayHost {
    virtual ~BlackoutOverlayHost() = default;
    // Widget vtable +34h at 005B6973, __thiscall(widget, int), the immediate 1.
    virtual void blackout_icon_set_visible(bool visible) = 0;
    // Widget vtable +50h at 005B69A8, __thiscall(widget, const float(&)[4]).
    virtual void blackout_icon_set_colour(const BlackoutFillColour& colour) = 0;
};

// 005B6960, __thiscall(this = the slot 33h screen), RET, no stack arguments.
// 004C40F0 reaches it only as the tail jump at 004C42EF, with ECX reloaded from
// *(00E198C4 + A4h) - the same screen the preceding block forced visible.
void blackout_mission_overlay_005b6960(BlackoutOverlayHost& host) noexcept;

// ---------------------------------------------------------------------------
// 2. 005CD1A0, the only routine that engages the interface lock
// ---------------------------------------------------------------------------

// Registry slot 37h, manager offset +9Ch, constructor 005CC120, vtable
// 00CF17A8, 38h bytes, page `GUI_movie`, raised by interface id 2Ch
// (docs/HUD_SCREEN_PAGES.md and docs/IN_MISSION_INTERFACE_MANAGER.md). Interface
// 2Ch is `kMovieCameraNewInterface`, the single id the lock lets through.
inline constexpr int kHudMovieScreenRegistrySlot = 0x37;
inline constexpr std::uint16_t kHudMovieScreenManagerOffset = 0x009C;
inline constexpr std::uint32_t kHudMovieScreenVtable = 0x00CF17A8u;
inline constexpr std::size_t kHudMovieScreenSizeBytes = 0x38;
inline constexpr std::string_view kHudMovieScreenPage = "GUI_movie";
inline constexpr std::size_t kMovieCameraSizeBytes = 0x570; // operator new inside 005CC170

// 005CD1A9 and 005CD1B5. ECX selects the stream, EDX carries the seed
// (docs for 00BD2FD0 BSP_RandomThreads_Seed). Both constants are literal, so the
// cinematic replays deterministically.
struct RandomStreamSeed {
    int stream{0};
    std::uint32_t seed{0};
};
inline constexpr std::array<RandomStreamSeed, 2> kMovieInterfaceRandomSeeds{{
    {1, 12345u}, // 005CD1A4 ECX=1, 005CD1A9 EDX=3039h
    {0, 54321u}, // 005CD1BA ECX=0, 005CD1B5 EDX=D431h
}};

// 005CD1E7 / 005CD1E5, pushed for 00A933F0 on the input manager 004BEC00
// returns, not for the getter itself: the decompiler attributes both pushes to
// the getter, the listing does not.
inline constexpr int kMovieInputContextId = 0x1E;
inline constexpr int kMovieInputContextLevel = 5;

// 005CD21F, 0077C470 on the unit at 00E188D8.
inline constexpr int kMovieSessionMessageId = 0x1FF;

// The screen's own two fields, both inside the 38h object.
struct HudMovieScreenState {
    // +1Ch. The 570h-byte movie camera 005CC170 allocates on first use.
    // Modelled as an opaque handle; 0 is "not built".
    std::uint32_t camera{0};
    // +20h, written 1 at 005CD229. 005CD240 runs the engage only while clear,
    // so the engage happens once per screen instance.
    bool engaged{false};
};

// Everything 005CD1A0 reads from outside the screen. Each field names the site.
struct MovieInterfaceEngageInputs {
    // 005CD1D3, [00E188A8] != 0: the game object exists.
    bool game_present{false};
    // 005CD1DC, game+1FE4h != 0: the local view mode, per docs/GAME_RENDER_FRAME.md.
    bool local_view_mode_active{false};
    // 005CD202, [00E188D8] != 0: the player-controlled unit exists.
    bool player_unit_present{false};
    // 005CD20E, 00927F30 BSP_UnitInstance_IsLocalPlayerRole(unit, 0).
    bool unit_is_local_player_role{false};
};

// The pure rule the packet was asked for. 005CD1E9 is the only write of 1 to
// 00E19894 in the image; it is guarded by exactly these two reads. Nothing about
// the pushed interface id, the screen state or the unit takes part.
constexpr bool movie_interface_engages_lock(bool game_present,
    bool local_view_mode_active) noexcept {
    return game_present && local_view_mode_active;
}

// What one call to 005CD1A0 did, so a caller can check the path without
// instrumenting the host.
struct MovieInterfaceEngageResult {
    bool pushed_interface{false};        // 005CD1CE, always
    bool engaged_lock{false};            // 005CD1E9
    bool raised_input_context{false};    // 005CD1F6
    bool sent_session_message{false};    // 005CD224
};

// One method per native call site 005CD1A0 makes that this reconstruction does
// not own.
struct MovieInterfaceHost {
    virtual ~MovieInterfaceHost() = default;
    // 00BD2FD0 BSP_RandomThreads_Seed, ECX = stream, EDX = seed. Called twice.
    virtual void seed_random_stream(int stream, std::uint32_t seed) = 0;
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest on the manager at
    // 00E198C4, __thiscall(manager, id, payload). The payload is the unit the
    // caller handed in, which both callers read from 00E188D8.
    virtual void push_interface_request(int interface_id, const void* payload) = 0;
    // 004BEC00 BSP_InputManager_GetSingleton then 00A933F0(id, level) on it.
    virtual void set_input_context_level(int context_id, int level) = 0;
    // 005CC170. Builds the 570h-byte camera into the screen's +1Ch when that is
    // still null and returns what the field should hold afterwards. It runs on
    // every call, not only on the engage path.
    virtual std::uint32_t ensure_movie_camera(std::uint32_t existing_camera) = 0;
    // 0077C470 on the unit at 00E188D8, __thiscall(unit, id, 0).
    virtual void send_unit_session_message(int message_id) = 0;
};

// 005CD1A0, __thiscall(this = the slot 37h screen, void* unit), RET 4.
// `lock` is the shared 00E19894 byte; the routine only ever raises it.
MovieInterfaceEngageResult engage_movie_interface_005cd1a0(HudMovieScreenState& screen,
    FrontEndInterfaceLock& lock, const MovieInterfaceEngageInputs& inputs,
    const void* unit_payload, MovieInterfaceHost& host);

// 005CD240, __thiscall(this) -> the camera at +1Ch, RET. Engages first when the
// screen has not engaged yet, then runs 005CC170 again and returns +1Ch.
std::uint32_t movie_screen_camera_005cd240(HudMovieScreenState& screen,
    FrontEndInterfaceLock& lock, const MovieInterfaceEngageInputs& inputs,
    const void* unit_payload, MovieInterfaceHost& host);

// The complete writer set of 00E19894, from the image-wide xref sweep. Exactly
// one entry engages the lock; the other five clear it. `front_end_managers.hpp`
// lists four of the clears in a comment, so this is the closed form.
struct InterfaceLockWriter {
    std::uint32_t address{0};
    bool engages{false};
};
inline constexpr std::array<InterfaceLockWriter, 6> kInterfaceLockWriters{{
    {0x005CD1E9u, true},  // 005CD1A0, the movie interface engage
    {0x00683EA7u, false}, // 00683E90, an id below 20h passing through
    {0x00684636u, false}, // 00684600, the same test inlined
    {0x004BC493u, false}, // 004BC410
    {0x004D2C5Fu, false}, // 004D2BB0 BSP_Game_DestroyWorld, gated on 00E198C4
    {0x004DAB5Du, false}, // 004DA780 BSP_Game_TeardownSessionState, same gate
}};

// ---------------------------------------------------------------------------
// 3. 00E19698, the `_PleaseWait` screen
// ---------------------------------------------------------------------------

// operator new(14h) at 004E3DB9, constructor 0060D290, vtable 00CF4AB8, and the
// register virtual +10h runs immediately at 004E3DEB. The ten-slot front-end
// screen vtable is the one docs/HUD_SCREEN_PAGES.md tabulates.
inline constexpr std::uint32_t kPleaseWaitScreenGlobal = 0x00E19698u;
inline constexpr std::size_t kPleaseWaitScreenSizeBytes = 0x14;
inline constexpr std::uint32_t kPleaseWaitScreenVtable = 0x00CF4AB8u;
// 0060D2B0, the vtable +00h registry-slot accessor, `mov eax, 53h; ret`.
inline constexpr int kPleaseWaitScreenRegistrySlot = 0x53;
inline constexpr std::string_view kPleaseWaitScreenPage = "_PleaseWait";     // 00CF4AF0, 0Bh chars
inline constexpr std::string_view kPleaseWaitScreenWidget = "Background_Icon"; // 00CF4AE0, 0Fh chars
// 00D7A260 is -1.0f. Both the register virtual and the enter virtual write it.
inline constexpr float kPleaseWaitProgressUnknown = -1.0f;

// The whole 14h-byte object. +4h and +5h are the base front-end screen bytes
// docs/FRONTEND_STATE_MACHINE.md names: +4h is what the caller wants, +5h is
// what the registry has applied.
struct PleaseWaitScreen {
    bool requested{false};                          // +04h
    bool applied{false};                            // +05h
    std::uint32_t page{0};                          // +08h, 00AA5840 result
    std::uint32_t background_icon{0};               // +0Ch, 00AA7E00 result
    float progress{kPleaseWaitProgressUnknown};     // +10h
};

// 0060D2F0's counter source: [01090AB0], whose virtual +20h returns a pointer to
// two consecutive 64-bit counters that the routine reads with FILD/FILD/FDIVP.
struct PleaseWaitProgressSource {
    virtual ~PleaseWaitProgressSource() = default;
    virtual void read_counters(std::int64_t& completed, std::int64_t& total) = 0;
};

// 0060D2F0, the update virtual (+20h), __thiscall(this, float), RET 4. The float
// argument is never read. Leaves `progress` alone while it is still the -1.0f
// sentinel, which is the state both the register and the enter virtual install.
void update_please_wait_screen_0060d2f0(PleaseWaitScreen& screen,
    PleaseWaitProgressSource& source) noexcept;

// The enter (+18h, 0060D390) and exit (+1Ch, 0060D2D0) virtuals. Both call
// 00A94C50 with a single bool in CL, which the exit derives from a global byte
// at 00F889C0; only the exit touches the widget.
struct PleaseWaitLifecycleHost {
    virtual ~PleaseWaitLifecycleHost() = default;
    // 00A94C50, __fastcall(bool). 0060D393 passes false; 0060D2DA passes
    // ([00F889C0] == 0).
    virtual void set_global_flag_00a94c50(bool value) = 0;
    // 004BEC00 then 00A92C40(0.0f) on the input manager, enter only.
    virtual void input_manager_00a92c40(float value) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate then 00AA0F70() and 00AA0E00(0),
    // enter only.
    virtual void gui_manager_00aa0f70() = 0;
    virtual void gui_manager_00aa0e00(int value) = 0;
    // The `Background_Icon` widget's set-visibility virtual +34h, exit only.
    virtual void background_icon_set_visible(bool visible) = 0;
};

void enter_please_wait_screen_0060d390(PleaseWaitScreen& screen,
    PleaseWaitLifecycleHost& host);
void exit_please_wait_screen_0060d2d0(PleaseWaitScreen& screen,
    bool global_byte_00f889c0_set, PleaseWaitLifecycleHost& host);

// Why 004C41BD may dereference 00E19698 without a null test. The pointer is
// written exactly three times in the image: once with the constructed object at
// 004E3DDB inside Game::OnInit, and null at 004CCD75 and 004DD077, both of which
// are shutdown paths that destroy the object through its vtable +0Ch first and
// null-check before doing so. Between OnInit and shutdown the global is never
// null, and fifteen further sites besides 004C41BD read or write `+4h` through
// it without a guard.
inline constexpr std::uint32_t kPleaseWaitScreenConstructSite = 0x004E3DDBu;
inline constexpr std::array<std::uint32_t, 2> kPleaseWaitScreenClearSites{{
    0x004CCD75u, 0x004DD077u,
}};

}  // namespace bsp
