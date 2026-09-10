#pragma once
// The blocking-screen fast path of the game update BSP_Game_OnMove (004E4A40).
// Addresses: 00689CC0, 004DB220, 00AA4F80, 00AA0E00, 00AA0E50,
//            00689D90, 00689C60, 00689E80, 00689C00, 00A91020, 00A90180, 00AA7EF0.
// Every name below is a hypothesis, not a recovered symbol. Evidence and the
// call-by-call recovery are in docs/GAME_BLOCKING_SCREEN.md. Nothing here is
// binary compatible with the original: the vtables, the pooled strings and the
// checked-iterator containers are not reproduced.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The screen object published in DAT_00E198BC
// ---------------------------------------------------------------------------

// Constructed by 00689D90 with operator new(0x4C) from BSP_Game_OnInitTitle at
// 004C9B92, destroyed by 00689C60, which clears the global. The constructor
// builds the GUI layout named below through the manager factory 00AA5840 and
// the activate virtual plays the movie below, so this is the front-end attract
// screen: the idle timeout that takes over the frame when nobody is playing.
inline constexpr std::size_t kAttractScreenSizeBytes = 0x4C;
inline constexpr std::uint32_t kAttractScreenVtable = 0x00CF78E8u;
inline constexpr std::string_view kAttractScreenLayoutName = "FE_attract";      // 00CF78FC
inline constexpr std::string_view kAttractMoviePath = "movies/PacificTheme.bik"; // 00CF7908

// DAT_00CE3D60, the reload value the constructor stores at +44h and both the
// update and the deactivate virtual restore.
inline constexpr float kAttractIdleSeconds = 45.0f;
// DAT_00D7A24C, the upper bound the update applies to the frame delta before
// subtracting it, so a long hitch costs at most one second of idle time.
inline constexpr float kAttractDeltaClampSeconds = 1.0f;

// The recovered part of the 0x4C-byte object. +00h is the vtable, +04h..+38h
// belong to the base class the constructor 00684E10 fills, and +40h is the
// opaque GUI layout handle 00AA5840 returns, released by 00AA31F0 in the
// destructor. Only the three fields the update touches are modelled.
struct AttractScreenState {
    bool active{false};                        // +3Ch, set by base activate 00684700,
                                               //       cleared by 00689C00
    float idle_countdown{kAttractIdleSeconds}; // +44h
    bool enabled{true};                        // +48h, constructor stores 1
};

// The three game states at *(00E188A8)+5D4h in which 00689CC0 refuses to raise
// the screen even after the countdown expires (00689D71..00689D7E). Their
// meaning is not established; 004E3AA0 destroys the screen entirely when it
// enters state 3, so these are additional in-session guards.
inline constexpr std::array<int, 3> kAttractSuppressedGameStates{0x0D, 0x10, 0x11};
bool attract_suppressed_in_game_state(int game_state) noexcept;

// Integration boundary for 00689CC0. One method per native call site, in call
// order. There are no default implementations: nothing here stands in for
// unrecovered game behaviour.
struct AttractScreenHost {
    virtual ~AttractScreenHost() = default;

    // 00A91020, ECX = the input device table DAT_00F8BBF4. Walks the checked
    // vector at +6Ch (_Myfirst +70h, _Mylast +74h) and returns true as soon as
    // one device's virtual +2Ch does.
    virtual bool any_dynamic_device_button_down() = 0;

    // 00A90180, same table, the eight joystick slots at +44h..+60h, the same
    // virtual +2Ch. Device virtual +2Ch is 00A93F60, which polls buttons 0..3Bh
    // through virtual +20h, so both queries mean "any button is down".
    virtual bool any_joystick_slot_button_down() = 0;

    // 00425D10 then its byte +5. Only read while the screen is down.
    virtual bool scene_context_blocks_attract() = 0;

    // *(00E188A8)+5D4h. Only read once the countdown has gone negative.
    virtual int game_state() = 0;

    // Screen virtual +8h -> 00689E80: base activate 00684700 sets +3Ch, the
    // layout at +40h becomes visible through its virtual +34h, and the movie
    // player DAT_00E18D48 starts kAttractMoviePath through 004F8A20.
    virtual void activate_attract_screen() = 0;

    // Screen virtual +0Ch -> 00689C00: base deactivate 00683AA0, the layout is
    // hidden, +3Ch is cleared, the movie player is stopped and +44h is reloaded
    // with kAttractIdleSeconds. Model that reload in the state you pass in.
    virtual void deactivate_attract_screen() = 0;

    // DAT_00E198C8, the 0x44-byte front-end object BSP_Game_OnInitTitle builds
    // beside the attract screen. Null until that runs.
    virtual bool front_end_menu_present() = 0;
    // Its virtual +8h, reached only on the frame the screen is dismissed.
    virtual void front_end_menu_on_attract_dismissed() = 0;
};

// 00689CC0. Native __thiscall, ECX = the screen, one float stack argument,
// RET 4, no return value. Both device queries are evaluated every call: the
// native code ORs their results, it does not short-circuit.
void update_attract_screen_00689cc0(AttractScreenState& state, AttractScreenHost& host,
    float raw_delta);

// ---------------------------------------------------------------------------
// 00AA4F80, the GUI manager per-frame update
// ---------------------------------------------------------------------------

// The 0x88-byte cGuiManager of docs/APP_INIT_FONTS_GUI.md. Byte +70h is held
// at the caller's value for the whole walk and cleared on exit.
struct GuiManagerUpdateState {
    bool in_update{false}; // +70h
};

// An opaque cGuiScreen. The manager's screen list and the layout handles the
// factories return are only partly recovered, so they stay pointers.
using GuiScreenHandle = void*;

struct GuiManagerUpdateHost {
    virtual ~GuiManagerUpdateHost() = default;

    // 00AA3910, ECX = the manager. Reads the cursor device vector at
    // DAT_00F8BBF4+94h, so this is the pointer hit-test pass. Runs only when
    // the flag argument is false.
    virtual void run_pointer_input_pass() = 0;

    // 00AA0F70 BSP_GuiManager_ResetScreens: hl_FrameBox at +74h and
    // hlCircle_FrameBox at +78h are moved to (-1, -1, 0) and hidden.
    virtual void reset_highlight_frames() = 0;

    // 004D35D0 copies the manager's screen vector (_Myfirst +18h, _Mylast +1Ch)
    // into a temporary that is freed after the walk, so screens added or
    // removed during the walk are not seen by it.
    virtual std::vector<GuiScreenHandle> snapshot_screens() = 0;

    // Screen virtual +38h, no arguments, returns a byte.
    virtual bool screen_wants_update(GuiScreenHandle screen) = 0;

    // 00AA7EF0, ECX = the screen: true when any of the +8Ch entries in the
    // array at +88h is non-null.
    virtual bool screen_has_live_entries(GuiScreenHandle screen) = 0;

    // Screen virtual +40h, one float stack argument.
    virtual void screen_update(GuiScreenHandle screen, float seconds) = 0;
};

// 00AA4F80. Native __thiscall, ECX = the manager, a float and a byte on the
// stack, RET 8. Both OnMove call sites pass false for the flag.
void gui_manager_update_00aa4f80(GuiManagerUpdateState& state, GuiManagerUpdateHost& host,
    float seconds, bool flag);

// ---------------------------------------------------------------------------
// 00AA0E00 and 00AA0E50, the pointer enable/disable pair
// ---------------------------------------------------------------------------

// Manager byte +48h, zeroed by the constructor 00AA5D70. Its readers are not
// identified, so only the writer is modelled.
struct GuiManagerPointerState {
    bool enabled{false}; // +48h
};

struct GuiPointerHost {
    virtual ~GuiPointerHost() = default;
    // Manager +50h is MousePtrFE_Icon, the front-end mouse pointer child of the
    // _Mouse group (docs/APP_INIT_FONTS_GUI.md, resource 4). Its virtual +34h
    // is the visibility setter both routines tail-call.
    virtual void set_front_end_pointer_visible(bool visible) = 0;
};

// 00AA0E00. __thiscall, ECX = the manager, one byte stack argument, RET 4.
// Stores the argument at +48h and, only when it is false, hides the pointer.
void gui_manager_set_enabled_00aa0e00(GuiManagerPointerState& state, GuiPointerHost& host,
    bool enabled);

// 00AA0E50. __thiscall, ECX = the manager, one byte stack argument forwarded by
// a bare JMP to the pointer's virtual +34h, which cleans it. The four call
// sites all push an argument (004E4C38, 00565081, 004DAA9D, 004C6E1C), so the
// decompiler's zero-argument signature is wrong. It never touches +48h.
void gui_manager_set_pointer_visible_00aa0e50(GuiPointerHost& host, bool visible);

// ---------------------------------------------------------------------------
// 004DB220, the return to the title screen
// ---------------------------------------------------------------------------

struct TitleResetHost {
    virtual ~TitleResetHost() = default;
    // 004DB190: sets DAT_00E08874, tears down the session objects DAT_00E198B4,
    // DAT_00E198B8 and DAT_00E198AC through their deleting destructors and
    // drains the state-request ring.
    virtual void release_session_objects() = 0;
    // 004CCCC0, ECX = the game: releases the in-mission singletons DAT_00E18678,
    // DAT_00E1867C, game+19C8h and the rest of that chain.
    virtual void release_mission_objects() = 0;
    // game+2180h = 0.
    virtual void clear_game_flag_2180h() = 0;
    // 004C9A70 BSP_Game_OnInitTitle, reached by a tail JMP. It writes
    // game+5D4h = 2 and lazily builds both DAT_00E198BC and DAT_00E198C8.
    virtual void on_init_title() = 0;
};

// 004DB220. __thiscall, ECX = the game, no stack arguments, no return value.
void reset_to_title_004db220(TitleResetHost& host);

// ---------------------------------------------------------------------------
// The frame itself, 004E4B2A..004E4B8C
// ---------------------------------------------------------------------------

struct BlockingScreenFrameHost {
    virtual ~BlockingScreenFrameHost() = default;
    // DAT_00E198BC, read at 004E4B29.
    virtual bool attract_screen_present() = 0;
    // 00689CC0 with the raw delta, ECX = DAT_00E198BC.
    virtual void update_attract_screen(float raw_delta) = 0;
    // The global is read again at 004E4B40 and its +3Ch tested at 004E4B46.
    virtual bool attract_screen_active() = 0;
    // 004C6C30 BSP_Game_TryBeginRenderFrame, ECX = the game.
    virtual bool try_begin_render_frame() = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate.
    virtual void* gui_manager() = 0;
    // 00AA4F80 with the raw delta and a false flag.
    virtual void gui_manager_update(void* manager, float seconds, bool flag) = 0;
    // 004CA440 BSP_Game_Render, ECX = the game.
    virtual void render() = 0;
    // 004CA1F0 BSP_Game_FinishRenderFrame, ECX = the game.
    virtual void finish_render_frame() = 0;
};

// The whole gate plus the render kick. Returns true when the native code took
// the RET 4 at 004E4B8C, that is when the frame was consumed and the entire
// simulation was skipped; false when OnMove fell through to 004E4B8F and
// carried on with the session polls. The delta is passed on unscaled: this path
// never reaches the frame-control scaling at the tail of OnMove.
bool run_blocking_screen_frame(BlockingScreenFrameHost& host, float raw_delta);
}
