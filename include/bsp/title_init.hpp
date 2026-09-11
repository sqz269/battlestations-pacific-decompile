#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/frame_clock.hpp"
#include "bsp/frontend_states.hpp"
#include "bsp/game_entry.hpp"

// GGame::OnInitTitle (004c9a70), the title-screen bring-up, and the logo
// sequence that hands control to it. Evidence, calling conventions, RET sizes
// and uncertainties: docs/GAME_TITLE_INIT.md.
//
// Reconstructed here: the ordered bring-up 004c9a70, the front-end frame layout
// set selector 00518250, the logo skip rule inside 00685170 and the logo
// advance 00685070. Host methods identify native call sites; recovered movie
// and profile-reset bindings live in startup_frontend.hpp. Remaining services
// require explicit implementations.
namespace bsp {

// ---------------------------------------------------------------------------
// Front-end frame layout sets (00518250 on the 164h singleton at 00E18D80)
// ---------------------------------------------------------------------------

// Three parallel name tables in .rdata, each indexed by set then by slot. The
// element counts are the loop trip counts inside 00518250 and the strides the
// native code adds to the set index: 34h for the first table (13 pointers),
// 8h for the other two (2 pointers each).
inline constexpr int kFrontEndFrameSetCount = 5;
inline constexpr int kFrontEndFrameBackdropSlots = 13; // this+0Ch, names 00E08500
inline constexpr int kFrontEndFramePanelSlots = 2; // this+110h, names 00E08604
inline constexpr int kFrontEndFrameTitleSlots = 2; // this+138h, names 00E0862C

// The set index the caller selects. Only 0 is established by a caller in this
// packet: GGame::OnInitTitle pushes 0 at 004c9b7d. The other four are named
// from the layouts their rows hold, which is a hypothesis about intent, not a
// recovered enumeration.
enum class FrontEndFrameSet : int {
    Title = 0, // FE_frame + FE_frame_title
    Shell = 1, // the same two layouts
    MainMenu = 2, // adds FE_options_bg (backdrop 6) and FE_main (backdrop 12)
    Pause = 3, // GUI_pause + GUI_pause_title
    None = 4, // every slot null; selecting it only releases
};

// The static tables read at 00E08500, 00E08604 and 00E0862C. A null slot loads
// nothing and leaves the corresponding handle null.
struct FrontEndFrameLayoutNames {
    const char* backdrop[kFrontEndFrameSetCount][kFrontEndFrameBackdropSlots];
    const char* panel[kFrontEndFrameSetCount][kFrontEndFramePanelSlots];
    const char* title[kFrontEndFrameSetCount][kFrontEndFrameTitleSlots];
};

// The tables as they stand in the image. Every backdrop slot is null except set
// 2 slots 6 and 12; both panel tables and both title tables are populated for
// sets 0..3 in their first slot only.
const FrontEndFrameLayoutNames& front_end_frame_layout_names() noexcept;

// The handle arrays inside the 164h singleton. Layout handles are opaque here:
// the reconstruction never dereferences one, it only passes it back to the
// host. active_set is this+160h, which constructor 00517d10 does not write.
struct FrontEndFrameLayouts {
    void* backdrop[kFrontEndFrameSetCount][kFrontEndFrameBackdropSlots]{};
    void* panel[kFrontEndFrameSetCount][kFrontEndFramePanelSlots]{};
    void* title[kFrontEndFrameSetCount][kFrontEndFrameTitleSlots]{};
    int active_set{-1}; // +160h, uninitialised in the native object
};

// One method per native call site the selector reaches.
struct FrontEndFrameLayoutHost {
    virtual ~FrontEndFrameLayoutHost() = default;
    // 004c12b0 BSP_GuiManager_GetOrCreate then 00aa5840(&name, 1, 1). The name
    // is built as a pooled native string from the .rdata literal; the length is
    // measured by an inline strlen at 00518380.
    virtual void* gui_layout_acquire(const char* name) = 0;
    // The word at layout+4h, read at 0051828a before every release decision.
    virtual std::int32_t gui_layout_refcount(void* layout) = 0;
    // 004c12b0 then 00aa31f0(layout): taken when the refcount word is exactly 1.
    virtual void gui_manager_release_layout(void* layout) = 0;
    // InterlockedDecrement(layout+4h) and, at zero, the layout's virtual +0h.
    virtual void gui_layout_release_ref(void* layout) = 0;
};

// 00518250. __thiscall, ECX = the 164h singleton, two stack arguments
// (int set, char commit), RET 8. When commit is set it first releases every
// handle that belongs to a set other than the requested one, then loads the
// requested set's missing handles, then stores the set in +160h. The whole body
// is skipped when the requested set already equals +160h.
void select_front_end_frame_set(FrontEndFrameLayouts& layouts, FrontEndFrameLayoutHost& host,
    int set, bool commit);

// ---------------------------------------------------------------------------
// Logo sequence (the 80h object at 00E198A4, constructor 006851e0)
// ---------------------------------------------------------------------------

// Members the advance and the skip poll touch. The entry vector at +44h/+48h
// holds 8-byte pooled-string records; only its element count and the address of
// element i are used here, so entries is modelled as an array of names. The
// delay vector at +54h/+58h holds one float per entry.
struct LogoSequenceState {
    const char* const* entries{nullptr}; // +44h, element stride 8
    std::size_t entry_count{0}; // (+48h - +44h) / 8
    const float* delays{nullptr}; // +54h, element stride 4
    std::size_t delay_count{0}; // (+58h - +54h) / 4
    std::uint32_t next_index{0}; // +60h, the index of the entry to play next
    ClockTimestamp entry_started{}; // +68h..+77h, latched from the clock
    float entry_delay{0.0F}; // +78h, delays[current index]
};

// The rule compiled at 006851a3..006851bf: FCOMIP of elapsed against the delay
// followed by JBE, so the skip needs a strict elapsed > delay, and then the
// edge-triggered menu action query 004d92b0(game, 4Ah) must also answer true.
// Both terms are required; the poll cannot advance on time alone.
bool logo_skip_allowed(float elapsed_seconds, float entry_delay, bool skip_action_pressed) noexcept;

// The two exits of 00685070.
enum class LogoAdvanceOutcome {
    EntryStarted, // 006850ae: an entry was played and the timer relatched
    SequenceFinished, // 00685091: the object deleted itself and re-entered the title
};

// One method per native call site the advance reaches. The movie player is the
// 34h front-end screen at 00E18D48 that BSP_Game_BeginStartupSequence builds.
struct LogoSequenceHost {
    virtual ~LogoSequenceHost() = default;
    // 004f8970 with ECX = 00E18D48 and the trampoline 00685060, which re-enters
    // this same advance with ECX = 00E198A4 when the movie ends. Re-armed on
    // every entry, before the movie is started.
    virtual void install_movie_completion_callback() = 0;
    // 004f8a20 with ECX = 00E18D48: (pooled name, prefer shrink-wide,
    // local GUI Z, loop byte). The logo passes 1, 0.0f, 0; the attract
    // screen passes 1.0f and 1 in the last two slots. Z is not audio volume.
    virtual void movie_play(const char* name, int prefer_shrink_wide,
        float local_z, int loop) = 0;
    // 00E18D48 bytes +4h and +5h = 1, then 004f83b0, then its virtual +18h.
    // Marks the movie screen wanted and active, commits it and enters it.
    virtual void movie_screen_enter() = 0;
    // Clock 01090AB0 virtual +20h, the same accessor the skip poll samples.
    virtual ClockTimestamp now() = 0;
    // The object's own virtual +0h with 1: the scalar deleting destructor.
    virtual void destroy_self() = 0;
    // 004dd5b0 BSP_Game_OnInitOnce(game, 0), the soft re-entry that runs
    // GGame::OnInitTitle and leaves game+5D4h at 2.
    virtual void reenter_title() = 0;
    // Calls at 006850D7 and 00685154. The CRT helper can return; a returning
    // binding must leave the relevant state vector valid for the following
    // access. Native reloads the pointer without repeating the bounds check.
    virtual void invalid_parameter_noinfo_00bf6713() = 0;
};

// 00685070. __thiscall, ECX = the logo object, no stack arguments, RET 0. The
// index at +60h is post-incremented: the entry played is the value on entry and
// +60h is left pointing at the next one. Both bounds checks the native code
// makes against the entry and delay vectors call the CRT handler at their
// original positions. The delay check follows movie entry and the timer store;
// it is not a sequence-finished condition. A null entry vector has count zero.
LogoAdvanceOutcome logo_advance_or_finish(LogoSequenceState& state, LogoSequenceHost& host);

// 00685170. __thiscall, ECX = the logo object, no stack arguments, RET 0; it
// tail-jumps into the advance rather than calling it. Returns true when the
// skip fired, and then writes what the advance did through outcome, which may
// be null. elapsed is 00530890 of now against state.entry_started reduced by
// timestamp_seconds_x87, the same pair the native poll uses.
bool logo_poll_skip(LogoSequenceState& state, LogoSequenceHost& host, const ClockTimestamp& now,
    bool skip_action_pressed, LogoAdvanceOutcome* outcome);

// The input action index pushed to 004d92b0 at 006851b6. The action table entry
// is not identified, so this is a raw index rather than a named control.
inline constexpr int kLogoSkipInputAction = 0x4A;

// ---------------------------------------------------------------------------
// GGame::OnInitTitle (004c9a70)
// ---------------------------------------------------------------------------

// The globals and game fields the bring-up writes, in the order it writes them.
// game is the GGame object mirrored at 00E188A8.
struct TitleInitState {
    GameStartupState game_state{GameStartupState::kLogoSequence}; // game+5D4h
    bool skip_title{false}; // 00E198CC, set by the skipTitle switch or by .scn
    void* attract_screen{nullptr}; // 00E198BC, the 4Ch object
    void* title_screen{nullptr}; // 00E198C8, the 44h object
    std::uint32_t front_end_slot{0}; // 00E198B0, cleared unconditionally
    void* mission_player_records{nullptr}; // game+21A0h, null outside a mission
    FrontEndScreen movie_screen{}; // 00E18D48 +4h/+5h
};

// The two string literals the bring-up builds as pooled native strings.
inline constexpr const char* kTitleInitScopeLabel = "GGame::OnInitTitle"; // 00CE7680
inline constexpr const char* kTitleInitAtlas = "interface/textures/allbutingame.ats"; // 00CE765C

// One method per native call site GGame::OnInitTitle reaches, in call order.
struct TitleInitHost {
    virtual ~TitleInitHost() = default;
    // 00be0a30 BSP_FileBlock_Construct with the scope label, closed by
    // 00bdcb30 BSP_FileBlock_Destroy on the way out. The label is a profiler
    // and file-tracking scope, not a file that is opened.
    virtual void open_named_block(const char* label) = 0;
    virtual void close_named_block() = 0;
    // 007fdb20 with ECX = game+650h: resets the player profile block.
    virtual void reset_player_profile() = 0;
    // 00af0060 BSP_TextureAtlas_Load with ECX = the atlas manager 00F8C26C.
    virtual void load_texture_atlas(const char* path) = 0;
    // 004c1ac0 (the lazy 164h singleton getter) then 00518250 with (0, 1).
    virtual void select_front_end_frame_set(FrontEndFrameSet set, bool commit) = 0;
    // operator new(4Ch) then 00689d90 BSP_AttractScreen_Construct. A failed
    // allocation stores null and the native code still dereferences it.
    virtual void* create_attract_screen() = 0;
    virtual void attract_screen_init(void* screen) = 0; // its virtual +4h
    // operator new(44h) then 0068d760 BSP_TitleScreen_Construct.
    virtual void* create_title_screen() = 0;
    // Its virtual +4h, 0068d8d0: builds the 18h FE_initial front-end screen
    // into title+40h, registers it, marks it wanted and active, commits it and
    // enters it. With skip_title set that routine instead enqueues state 4.
    virtual void title_screen_activate(void* screen) = 0;
    // 0068d8a0 BSP_TitleScreen_Skip: enqueues state 4 directly.
    virtual void title_screen_skip() = 0;
    // 00916980 with ECX = game+21A0h: resets eight 284h-byte per-slot records
    // and clears the keyed tree at +1488h.
    virtual void reset_mission_player_records(void* records) = 0;
    // 0067c8f0: true when the sign-in state is coherent, that is the platform
    // manager exists, it reports a signed-in user, and the invitee slot equals
    // the active user slot.
    virtual bool sign_in_state_valid() = 0;
    // 00a40020 BSP_XenonSystemManager_ResetSignInState, ECX = 00F8ABE8.
    virtual void reset_sign_in_state() = 0;
    // 0067c970: re-creates the primary input binding and sets 00E1987C.
    virtual void rebind_primary_input() = 0;
    // 00E18D48 virtual +1Ch, the front-end screen exit.
    virtual void movie_screen_exit() = 0;
    // 004f83b0 on 00E18D48: pushes the cleared +5h byte into every child the
    // screen's virtual +24h reports, through each child's virtual +34h.
    virtual void movie_screen_commit() = 0;
};

// 004c9a70. __thiscall, ECX = the GGame object, no stack arguments, RET 0 (no
// stack cleanup); the body is wrapped in an SEH scope with handler 00C6546E.
// Callers: BSP_Game_BeginStartupSequence 004e5753, BSP_Game_OnInitOnce
// 004dd5b0, BSP_Game_ResetToTitle 004db220 (a tail jump) and
// BSP_Game_PollPlatformSessionEvents 004db290.
void run_title_init(TitleInitState& state, TitleInitHost& host);

} // namespace bsp
