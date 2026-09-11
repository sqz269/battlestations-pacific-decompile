#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/frontend_states.hpp"

// What pumps and gates the front-end screens once the main-menu shell rests at
// game state 5 (packet cc_main_menu_screens). Evidence and uncertainties:
// docs/MAIN_MENU_SCREENS_RUNTIME.md.
//
// docs/MAIN_MENU_PATH.md established the boundary: GGame::OnMove takes its
// front-end branch only for game+5D4h of 1, 2 or 4, and the main-menu shell
// rests at 5. Everything that keeps the menu alive from that instant runs on the
// in-mission side of OnMove. Two pieces carry it, and only one of them was still
// missing a body:
//
//   - The frame level, 004E53B4..004E548A: the non-simulating fallback calls
//     004C40F0, then 004E5442 services the managers through 006840F0, then the
//     drain loop at 004E5460 repeats 00776230 / 004C40F0 / 006840F0 while a
//     request was serviced. That loop is already reconstructed as
//     bsp::run_render_tail in include/bsp/render_tail.hpp and is NOT repeated
//     here; this header supplies the body of the one host method it left
//     opaque, RenderTailHost::update_without_simulation.
//
//   - 004C40F0 itself, BSP_Game_UpdateInterfaceOnly. Reconstructed below. It is
//     the routine that reaches 004F8830 at 004C4165, which is the only per-frame
//     pump the main menu gets at state 5.
//
// The manager activate/deactivate walk (00684700 / 00683AA0) and the interface
// request lock (00683E90, byte 00E19894) are already reconstructed in
// include/bsp/frontend_managers.hpp. This header does not redefine them; it adds
// only the four-way classification of the lock decision that the existing bool
// cannot express, and records the corrections in the doc.
//
// Names are hypotheses, not recovered symbols.
namespace bsp {

// ---------------------------------------------------------------------------
// 004C40F0 BSP_Game_UpdateInterfaceOnly
// __fastcall(GGame* ECX), no stack arguments, RET at 004C42F8. Sole caller
// BSP_Game_OnMove 004E4A40, from three sites: the non-pause in-mission branch
// at 004E5259, the non-simulating fallback at 004E53B6 (the one state 5 takes)
// and the drain loop at 004E5469.
// ---------------------------------------------------------------------------

// Which of the two top-level arms 004C412A selects.
enum class InterfaceOnlyArm {
    // 004C4205. Taken while the system UI (the console guide) is up now or was
    // up last frame. Walks the in-mission manager's three overlay screens and
    // never touches the front-end registry.
    SystemUiOverlay,
    // 004C4130. Everything else, including every normal main-menu frame.
    FrontEndPump,
};

// The five screen objects 004C40F0 can reach, named by how it addresses them.
// None of them is a front-end registry slot index: the routine holds each one
// as a pointer, which is why they do not go through FrontEndScreenHost.
enum class InterfaceOnlyScreenRef {
    // The 00425D10 singleton cached at 00E18E6C, the shared menu command and
    // confirmation-dialog screen (bsp::MenuCommandScreen).
    MenuCommand,
    // *(00E19698), dereferenced at 004C41BD with no null test.
    GamepadPrompt,
    // *(00E198C4 + C8h), registry slot 51h per docs/IN_MISSION_INTERFACE_MANAGER.md.
    MissionOverlayC8,
    // *(00E198C4 + 9Ch), registry slot 37h.
    MissionOverlay9C,
    // *(00E198C4 + A4h), registry slot 33h.
    MissionOverlayA4,
};

// The three sub-objects of the in-mission interface manager at 00E198C4 that
// the system-UI arm walks, in the order 004C4212 / 004C4245 / 004C4278 reads
// them. The manager pointer is re-read from the global before each one and the
// null test is done only once, at 004C420A.
inline constexpr std::array<int, 3> kInterfaceOnlyOverlayOffsets{{0xC8, 0x9C, 0xA4}};

inline constexpr std::size_t kInterfaceOnlyOverlayCount = 3;

// Fields of the game object (00E188A8) that 004C40F0 reads or writes.
struct InterfaceOnlyGameFields {
    // +719Eh. A one-shot suppression byte: while it is set the system-UI arm is
    // refused for that frame. 004C4118 clears it unconditionally, before the
    // arm is taken, so it always costs exactly one frame.
    bool suppress_system_ui_arm{false};
    // +21F0h, the time-scaled delta. Every 004F71F0 call in the body uses this
    // one; only the 004F8830 pump at 004C4165 can use the other.
    float scaled_delta{0.0f};
    // +21ECh, the clamped delta before the held-action multipliers
    // (docs/GAME_FRAME_CONTROL.md).
    float unclamped_delta{0.0f};
    // +635h, the "still simulate" companion of the cinematic flag +634h. When it
    // is set the pump is driven from +21ECh instead, because +634h forces +21F0h
    // to 0.0f and the interface would otherwise freeze.
    bool still_simulate{false};
    // +624h, the session termination reason (docs/GAME_SESSION_POLLS.md). Non-zero
    // arms the tail block at 004C42A6.
    std::int32_t termination_reason{0};
};

// The globals 004C40F0 consults, and the screen state it mutates.
struct InterfaceOnlyWorld {
    // 00E188AE, the system-UI byte 004CEB40 maintains.
    bool system_ui_raised{false};
    // 00E18B34, the previous-frame latch of the byte above. 004C411E copies
    // 00E188AE into it on every call, on both arms.
    bool system_ui_previous{false};
    // 00E198C4 != 0. Null for the whole front end, which is why a state-5 frame
    // can never take the system-UI arm's body or the tail block.
    bool in_mission_manager_present{false};
    // The three overlays, indexed as kInterfaceOnlyOverlayOffsets. `present` is
    // the pointer at that offset being non-null; it is tested every frame.
    std::array<bool, kInterfaceOnlyOverlayCount> overlay_present{};
    std::array<FrontEndScreen, kInterfaceOnlyOverlayCount> overlay{};
    // The 00425D10 singleton. Only `base` and `modal_dialog_active` are read.
    MenuCommandScreen menu{};
    // *(00E19698). The routine assumes it is non-null.
    FrontEndScreen gamepad_prompt{};
};

// 004C40F5..004C412A. True when the system-UI arm is taken. The suppression byte
// wins over both halves of the guide test.
bool interface_only_takes_system_ui_arm(
    bool system_ui_raised, bool system_ui_previous, bool suppress_arm) noexcept;

// 004C413D..004C4157. Which delta the 004F8830 pump is driven with. This choice
// exists only at that one call site.
float interface_only_pump_delta(const InterfaceOnlyGameFields& game) noexcept;

// One method per native call site 004C40F0 reaches. No default implementations.
struct InterfaceOnlyHost {
    virtual ~InterfaceOnlyHost() = default;
    // 004F8830 BSP_FrontEndScreens_Pump, __stdcall(float), at 004C4165. The
    // three-pass walk over the 95-slot registry; this is the call that makes the
    // main menu visible and keeps it updated.
    virtual void run_screen_pump_004f8830(float seconds) = 0;
    // 004F71F0 BSP_FrontEndScreen_Update, __thiscall(screen, float). Sites
    // 004C4194 (menu command), 004C41DB (gamepad prompt), 004C4234 / 004C4267 /
    // 004C429A (the three overlays).
    virtual void screen_update_004f71f0(InterfaceOnlyScreenRef ref, float seconds) = 0;
    // 004B6E50 BSP_FrontEndScreen_Close, __thiscall(screen). Sites 004C423B,
    // 004C426E, 004C42A1, all on the system-UI arm.
    virtual void screen_close_004b6e50(InterfaceOnlyScreenRef ref) = 0;
    // Screen vtable +1Ch, the exit virtual. Sites 004C41AE and 004C41F1, where
    // 004B6E50 is inlined rather than called.
    virtual void screen_exit_virtual(InterfaceOnlyScreenRef ref) = 0;
    // Screen vtable +18h, the enter virtual, at 004C42DB only.
    virtual void screen_enter_virtual(InterfaceOnlyScreenRef ref) = 0;
    // 004F83B0 BSP_FrontEndScreen_CommitVisibility, __thiscall(screen). Sites
    // 004C41B8, 004C41FB and 004C42CF.
    virtual void screen_commit_004f83b0(InterfaceOnlyScreenRef ref) = 0;
    // 005B6960, the tail jump at 004C42EF with ECX = *(00E198C4+A4h). Reads the
    // screen's +BCh child, calls that child's vtable +34h with 1 and then writes
    // a zeroed three-float vector and the constant at 00D7A24C. Its effect is
    // not established; it is a host call, not a reconstruction.
    virtual void mission_overlay_finalize_005b6960() = 0;
};

// What the call did, so a caller can check the path without instrumenting the
// host.
struct InterfaceOnlyResult {
    InterfaceOnlyArm arm{InterfaceOnlyArm::FrontEndPump};
    // The 004C4165 pump ran. False on every system-UI frame and on every frame
    // where the menu command screen has a modal dialog up.
    bool screen_pump_ran{false};
    // The delta handed to 004F8830; meaningful only when screen_pump_ran.
    float pump_delta{0.0f};
    // The tail block at 004C42A6 ran to its tail call.
    bool overlay_forced_visible{false};
};

// 004C40F0 in full. Mutates `game.suppress_system_ui_arm` (cleared),
// `world.system_ui_previous` (latched) and the flag bytes of whichever screens
// the taken arm closes or forces visible.
InterfaceOnlyResult run_interface_only_update_004c40f0(
    InterfaceOnlyWorld& world, InterfaceOnlyGameFields& game, InterfaceOnlyHost& host);

// ---------------------------------------------------------------------------
// The state-5 frame, as a description rather than a second implementation
// ---------------------------------------------------------------------------

// game+5D4h once 004E4279 writes it. The shell rests here; docs/MAIN_MENU_PATH.md
// records the value, this names the resting state for the runtime.
inline constexpr int kGameStateMainMenuResting = 5;

// The steps of one state-5 frame, in the order OnMove runs them, starting after
// the simulation gate at 004E50B0 has failed its `game+5D4h == 0Dh` test. The
// bodies live in bsp::run_render_tail; this enum exists so the doc and the
// executable can name the same sites.
enum class MainMenuFrameStep {
    NonSimulatingFallback,  // 004E53B4: 004C40F0, the pump that draws the menu
    ServicePendingRequests, // 004E5442: 006840F0(&pending)
    ClearPumpSentinel,      // 004E544C: 00E18CDC = 0
    DrainPeerQueues,        // 004E5462: 00776230, ECX = game+1EF0h
    DrainInterfaceUpdate,   // 004E5469: 004C40F0 again
    DrainServiceRequests,   // 004E5477: 006840F0(&pending) again
    DrainClearPumpSentinel, // 004E5481: 00E18CDC = 0
};

// 004E5460..004E5488 repeats the last four steps while 006840F0 reports work.
// This is why the main menu appears on the same frame the shell enters it: the
// shell's push is serviced at 004E5442, the out byte goes true, and the drain
// loop runs 004C40F0 at 004E5469 over the screen set the service pass just
// published. Without the loop the menu would wait a frame.
inline constexpr std::array<MainMenuFrameStep, 4> kMainMenuFrameDrainBody{{
    MainMenuFrameStep::DrainPeerQueues,
    MainMenuFrameStep::DrainInterfaceUpdate,
    MainMenuFrameStep::DrainServiceRequests,
    MainMenuFrameStep::DrainClearPumpSentinel,
}};

// ---------------------------------------------------------------------------
// 00683E90, the interface request lock, refined
// ---------------------------------------------------------------------------

// The single site that engages the byte at 00E19894: 005CD1E9 stores EBX, which
// 005CD1A4 set to 1, immediately after 005CD1CE pushes interface request 2Ch on
// the in-mission manager and only when a session is live (game+1FE4h != 0).
// Every other writer clears it: 004DAB5D, 004D2C5F, 004BC493 and 00683EA7.
// The lock therefore means "a movie-camera interface owns the in-session UI".
inline constexpr std::uint32_t kInterfaceLockEngageSite = 0x005cd1e9u;

// The four distinct outcomes of 00683E90. bsp::front_end_request_rejected_00683e90
// in include/bsp/frontend_managers.hpp returns only the reject bit; this splits
// the three passing arms, because two of them differ in what they do to the lock.
enum class InterfaceLockOutcome {
    // 00683E97. The byte was clear, so nothing is gated and nothing changes.
    PassedLockClear,
    // 00683EA0. Id 2Ch passes with the lock left engaged.
    PassedExempt,
    // 00683EA7. Any id below 20h passes and clears the byte on the way through.
    // Every front-end interface, the main menu's id 1 included, lands here.
    PassedAndReleased,
    // 00683EB3. Any other id at or above 20h is dropped.
    Rejected,
};

// 00683E90, __stdcall(int interfaceId), RET 4. Pure classification; the caller
// applies the lock change. Consistent with front_end_request_rejected_00683e90,
// which is Rejected alone.
InterfaceLockOutcome classify_interface_request_00683e90(
    bool lock_engaged, int interface_id) noexcept;

// True when the outcome itself cleared the byte, which only 00683EA7 does.
// PassedLockClear leaves the lock clear because it already was.
bool interface_lock_released_by(InterfaceLockOutcome outcome) noexcept;
}
