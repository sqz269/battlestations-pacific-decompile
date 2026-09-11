#pragma once
#include <cstdint>

#include "bsp/frontend_states.hpp"

// The front-end screen registry as a state machine: how a screen enters the
// fixed slot array at 00E18B60, how the pair of flag bytes at screen+4h/+5h
// moves through its four states, and the ordered path that carries a cold boot
// from GGame::OnInitTitle into the press-start page. Evidence and uncertainties:
// docs/FRONTEND_STATE_MACHINE.md.
//
// This header owns the registry lifecycle (004f7180, 004f71d0, 004f71a0, the
// empty base virtuals at 004f7570..004f75d0) and the title handover 0068d8d0.
// It deliberately does not restate work that already exists:
//   - the pump 004f8830 and the dispatch 004e4b9d..004e4d2c: bsp/frontend_states.hpp
//   - the visibility commit 004f83b0 and the +4h writer 004f7620: bsp/frontend_screen_sets.hpp
//   - GGame::OnInitTitle 004c9a70 and the logo sequence: bsp/title_init.hpp
//   - the press-start body 0067cfb0 and its sign-in flow: bsp/press_start_screen.hpp
namespace bsp {

// ---------------------------------------------------------------------------
// The base screen vtable, 00CEAE54
// ---------------------------------------------------------------------------

// Address of the vtable 004f7180 installs. Ten slots were read from the image;
// the two cells that follow (00CEAE7C, 00CEAE80) point outside the 004f7570..
// 004f75e0 block this class occupies and are not claimed as members.
inline constexpr std::uint32_t kFrontEndScreenBaseVtable = 0x00ceae54u;

// | Slot | Target   | Body                                   | Role             |
// | +00h | 00bf698e | __purecall                             | screen id        |
// | +04h | 004f7570 | xor al,al; ret                         | self-managed     |
// | +08h | 004f7580 | xor al,al; ret                         | unnamed predicate|
// | +0Ch | 004f75e0 | scalar deleting destructor             | destroy          |
// | +10h | 004f71d0 | see register_front_end_screen          | register         |
// | +14h | 004f7590 | ret                                    | unnamed, empty   |
// | +18h | 004f75a0 | ret                                    | enter            |
// | +1Ch | 004f75b0 | ret                                    | exit             |
// | +20h | 004f75c0 | ret 4                                  | update(float)    |
// | +24h | 004f75d0 | ret 4                                  | collect children |
//
// Slots +18h, +1Ch, +20h and +24h are a single RET each, so the base class does
// nothing on enter, exit, update or collect and every observable screen
// behaviour is an override. Their RET sizes give the argument counts: enter and
// exit take none, update and collect take one stack argument each.
bool front_end_screen_self_managed_004f7570() noexcept;
bool front_end_screen_predicate_004f7580() noexcept;

// ---------------------------------------------------------------------------
// Registry lifecycle
// ---------------------------------------------------------------------------

// Bounds of the pointer array, read from the loop guards in 004f71a0
// (CMP EAX,0xe18cdc / JL) and 004f8830. The first slot is 00E18B60 and the last
// is 00E18CD8, which is kFrontEndScreenSlotCount == 95 slots of four bytes.
inline constexpr std::uint32_t kFrontEndScreenTableFirst = 0x00e18b60u;
inline constexpr std::uint32_t kFrontEndScreenTableEnd = 0x00e18cdcu; // exclusive

// 004f7180. __thiscall, ECX = the screen, no stack arguments, RET 0, returns
// this in EAX. Installs kFrontEndScreenBaseVtable and clears +4h and +5h, so a
// screen is born neither wanted nor active and is not yet in the registry.
FrontEndScreen& construct_front_end_screen_004f7180(FrontEndScreen& screen) noexcept;

// 004f71d0. __thiscall, ECX = the screen, no stack arguments, RET 0. The whole
// body is id = this->vtable[0](); *(00E18B60 + id*4) = this. The screen id is
// therefore the slot index, it comes from the leaf type's pure virtual, and the
// store is unchecked: a leaf that returns an id outside 0..94 writes past the
// array. Registering twice with the same id silently replaces the occupant, and
// a slot is never reference counted.
void register_front_end_screen_004f71d0(FrontEndScreenTable& table, FrontEndScreen& screen,
    int screen_id) noexcept;

// 004f71a0. __thiscall, ECX = the screen, no stack arguments, RET 0. Restores
// the base vtable, then walks all 95 slots and nulls every one that holds this.
// It scans rather than using the id, so a screen that registered under several
// ids is removed from all of them. The flag bytes are not touched, so a screen
// destroyed while active leaves no exit call behind: 004b6e50 is what clears
// them (bsp::close_front_end_screen).
void unregister_front_end_screen_004f71a0(FrontEndScreenTable& table,
    const FrontEndScreen& screen) noexcept;

// ---------------------------------------------------------------------------
// The two flag bytes as a state machine
// ---------------------------------------------------------------------------

// (wanted, active) has four combinations and 004f8830 acts on three of them,
// one per pass. The names are this reconstruction's; the native carries no
// enumeration.
enum class FrontEndScreenPhase {
    Hidden, // +4h 0, +5h 0: no pass touches it
    Entering, // +4h 1, +5h 0: pass B sets active, commits, then calls enter
    Shown, // +4h 1, +5h 1: pass C calls update(raw_delta)
    Exiting, // +4h 0, +5h 1: pass A calls exit, then clears active and commits
};

FrontEndScreenPhase front_end_screen_phase(const FrontEndScreen& screen) noexcept;

// Which pump pass acts on a screen in this phase. The pump makes three full
// sweeps of the registry in this order, so every exit in the frame completes
// before any enter runs, and every enter completes before any update runs.
enum class FrontEndScreenPass {
    None,
    Exit, // 004f8890..004f88c5
    Enter, // 004f88d0..004f88ff
    Update, // 004f8906..004f8930
};

FrontEndScreenPass front_end_screen_pass(FrontEndScreenPhase phase) noexcept;

// The commit order differs between the two edges and it is load bearing, so it
// is recorded rather than folded into one helper.
//
//   Exit  004f88a2: exit virtual +1Ch first; then, only if +5h survived that
//         call, +5h = 0 and 004f83b0. A screen whose exit virtual already ran
//         004b6e50 on itself is therefore not committed twice.
//   Enter 004f88e2: +5h = 1 and 004f83b0 first, then the enter virtual +18h, so
//         the enter body observes itself as already visible.
inline constexpr bool kFrontEndExitCommitsAfterVirtual = true;
inline constexpr bool kFrontEndEnterCommitsBeforeVirtual = true;

// ---------------------------------------------------------------------------
// The title handover, 0068d8d0
// ---------------------------------------------------------------------------

// Registry slot the press-start screen occupies, from
// bsp::kPressStartScreenId in bsp/press_start_screen.hpp (0x5C). Repeated here
// only as a comment anchor: 0x5C is 92, which is inside the 95 slot array.

// The 0x44 byte title object's vtable is 00CF7A98; +4h is 0068d8d0 and +8h is
// 0068d850. The activate routine is the only place a front-end screen is made
// visible without going through the pump's enter pass.
struct TitleHandoverState {
    bool skip_title{false}; // 00E198CC, the skipTitle switch
    FrontEndScreen* press_start{nullptr}; // title+40h, the 0x18 byte screen
    bool state_request_pending{false}; // game+5E8h != 0
    bool requests_held{true}; // game+5ECh
};

// One method per native call site 0068d8d0 and 0068d850 reach, in call order.
struct TitleHandoverHost {
    virtual ~TitleHandoverHost() = default;
    // 0068d8f8, 00bd3450 with ECX = *0109cecc: resets the storage operation's
    // availability. Only on the skip path.
    virtual void reset_storage_availability() = 0;
    // 0068d90e, 004d7920 BSP_Game_RequestState with ECX = the game and the
    // state value on the stack. The skip path pushes 4.
    virtual void request_game_state(int state) = 0;
    // 0068d931, operator new(0x18) then 0067c840
    // BSP_PressStartScreen_Construct. A null allocation is stored and then
    // dereferenced by the native code; the reconstruction returns early instead
    // and says so in its comment.
    virtual FrontEndScreen* create_press_start_screen() = 0;
    // 0068d966, the new screen's vtable +10h. For the press-start leaf that is
    // 0067ca80, which calls 004f71d0 and then loads the FE_initial layout.
    virtual void screen_register(FrontEndScreen& screen) = 0;
    virtual void screen_commit(FrontEndScreen& screen) = 0; // 0068d975, 004f83b0
    virtual void screen_enter(FrontEndScreen& screen) = 0; // 0068d981, vtable +18h
    // 0068d866, the singleton at 00F8BBF4 through its vtable +4h with the raw
    // delta. That object is built at 00a908f6 and is still unidentified.
    virtual void update_title_owner(float raw_delta) = 0;
    // 0068d873, 004f71f0 on title+40h, which forwards to that screen's vtable
    // +20h with the same raw delta.
    virtual void update_press_start_screen(float raw_delta) = 0;
};

// 0068d8d0, title vtable +4h. __thiscall, ECX = the 0x44 title object, no stack
// arguments, RET 0, SEH handler 00C7E0CB. With skipTitle set it resets storage
// availability and, only when the request ring is empty, enqueues state 4 and
// clears the hold byte; otherwise it builds the press-start screen, registers
// it, marks it wanted and active, commits and enters it.
void activate_title_screen_0068d8d0(TitleHandoverState& state, FrontEndScreenTable& table,
    TitleHandoverHost& host);

// 0068d850, title vtable +8h. __thiscall(float raw_delta), ECX = the title
// object, RET 4. Both calls take the raw delta, not the scaled one.
void update_title_screen_0068d850(const TitleHandoverState& state, TitleHandoverHost& host,
    float raw_delta);

// ---------------------------------------------------------------------------
// The cold-boot path into the first interactive page
// ---------------------------------------------------------------------------

// The ordered steps a cold boot takes from the end of Init to a press-start
// page that responds to input. Each step names the native routine that performs
// it; the executable reaches them through the modules listed at the top of this
// header, not through one entry point, because no native routine spans them.
enum class FrontEndBootStep {
    // GGame::OnInit leaves game+5D4h at 3 (bsp::kGameStateFrontEndInit) and the
    // startup sequence 004e5540 calls GGame::OnInitTitle, which writes 2.
    TitleInit, // 004c9a70, bsp::run_title_init
    // Title init's last front-end act, vtable +4h of the 0x44 object.
    TitleActivate, // 0068d8d0, activate_title_screen_0068d8d0
    // From here the application frame drives it: state 2 updates the title
    // object and then the shared tail pumps the registry.
    FrameTitleUpdate, // 004e4c24 then 0068d850
    FramePump, // 004e4c9b then 004f8830, bsp::run_front_end_screen_pump
    // Pass C reaches the press-start screen's own update.
    PressStartUpdate, // 0067cfb0, bsp/press_start_screen.hpp
    // Every press-start exit funnels through the title skip, which enqueues 4.
    TitleSkip, // 0068d8a0 then 004d7920
    // The drain pops 4 and runs the front-end shell entry, which destroys the
    // title object, loads the main-menu resource sets and settles the state at
    // 5 (bsp::kGameStateFrontEndShellReady).
    FrontEndShellEntry, // 004e4430 then 004e4000, bsp/frontend_entry.hpp
};

// Returns the step that follows, or the same step while the machine waits. The
// rule is the native one: the frame steps repeat until the press-start screen
// requests a state, and a request already queued blocks a second one
// (0068d903 and the identical guard at 0068d8bc).
FrontEndBootStep next_front_end_boot_step(FrontEndBootStep step, bool press_start_accepted,
    bool state_request_pending) noexcept;

} // namespace bsp
