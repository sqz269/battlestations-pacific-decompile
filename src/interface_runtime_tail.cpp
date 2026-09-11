#include "bsp/interface_runtime_tail.hpp"

// Packet cc_interface_runtime. Every routine below is read from the listing, not
// from the decompiler: 005B6960 and 005CD1A0 both have register inputs the
// pseudocode misattributes. docs/INTERFACE_RUNTIME_TAIL.md records which.
namespace bsp {

// ---------------------------------------------------------------------------
// 005B6960
// ---------------------------------------------------------------------------

// 005B6960: SUB ESP,10h / PUSH ESI / MOV ESI,ECX, so `this` is the only input
// and there are no stack parameters. The body is two virtual calls on the same
// child, re-read from this+BCh before each one.
void blackout_mission_overlay_005b6960(BlackoutOverlayHost& host) noexcept {
    // 005B6966..005B6973: child = *(this + BCh); child->vtable[34h](1).
    host.blackout_icon_set_visible(true);

    // 005B6975..005B69A8. XORPS XMM0,XMM0 then three MOVSS of that zero into
    // [ESP+4], [ESP+8] and [ESP+0Ch], then the constant at 00D7A24C (1.0f) into
    // [ESP+10h]; LEA EDX,[ESP+4] is the argument. Four floats, not three.
    const BlackoutFillColour colour{};
    host.blackout_icon_set_colour(colour);
}

// ---------------------------------------------------------------------------
// 005CD1A0 and 005CD240
// ---------------------------------------------------------------------------

MovieInterfaceEngageResult engage_movie_interface_005cd1a0(HudMovieScreenState& screen,
    FrontEndInterfaceLock& lock, const MovieInterfaceEngageInputs& inputs,
    const void* unit_payload, MovieInterfaceHost& host) {
    MovieInterfaceEngageResult result{};

    // 005CD1A4..005CD1C0. Two literal reseeds before anything else runs.
    for (const RandomStreamSeed& seed : kMovieInterfaceRandomSeeds) {
        host.seed_random_stream(seed.stream, seed.seed);
    }

    // 005CD1C1..005CD1CE. Unconditional, and it happens before the lock is
    // raised, so this push is never the one the lock could have rejected.
    host.push_interface_request(kMovieCameraNewInterface, unit_payload);
    result.pushed_interface = true;

    // 005CD1D3..005CD1F6. The lock and the input-context level share one gate.
    if (movie_interface_engages_lock(inputs.game_present, inputs.local_view_mode_active)) {
        // 005CD1E9, MOV byte ptr [00E19894],BL with BL still 1 from 005CD1A4.
        lock.engaged = true;
        result.engaged_lock = true;
        // 005CD1E5/005CD1E7 push 5 and 1Eh for 00A933F0, not for the getter at
        // 005CD1EF; the decompiler hangs both on 004BEC00.
        host.set_input_context_level(kMovieInputContextId, kMovieInputContextLevel);
        result.raised_input_context = true;
    }

    // 005CD1FB. Runs on both paths.
    screen.camera = host.ensure_movie_camera(screen.camera);

    // 005CD202..005CD224. Two independent reads of 00E188D8; the listing reloads
    // the global rather than keeping it.
    if (inputs.player_unit_present && inputs.unit_is_local_player_role) {
        host.send_unit_session_message(kMovieSessionMessageId);
        result.sent_session_message = true;
    }

    // 005CD229, MOV byte ptr [ESI + 20h],BL.
    screen.engaged = true;
    return result;
}

std::uint32_t movie_screen_camera_005cd240(HudMovieScreenState& screen,
    FrontEndInterfaceLock& lock, const MovieInterfaceEngageInputs& inputs,
    const void* unit_payload, MovieInterfaceHost& host) {
    // 005CD243. The engage is skipped once the screen has engaged, which is what
    // makes 005CD1A0 a once-per-instance routine.
    if (!screen.engaged) {
        (void)engage_movie_interface_005cd1a0(screen, lock, inputs, unit_payload, host);
    }
    // 005CD256, then 005CD25B returns this+1Ch.
    screen.camera = host.ensure_movie_camera(screen.camera);
    return screen.camera;
}

// ---------------------------------------------------------------------------
// 0060D2F0, 0060D390, 0060D2D0
// ---------------------------------------------------------------------------

void update_please_wait_screen_0060d2f0(PleaseWaitScreen& screen,
    PleaseWaitProgressSource& source) noexcept {
    // 0060D2F6..0060D306: UCOMISS against 00D7A260 (-1.0f) with the LAHF /
    // TEST AH,44h / JP pair, which takes the jump on equal or unordered. The
    // sentinel and a NaN both skip the update.
    const bool ordered_and_different = screen.progress < kPleaseWaitProgressUnknown ||
        screen.progress > kPleaseWaitProgressUnknown;
    if (!ordered_and_different) {
        return;
    }

    // 0060D308..0060D321. FILD of two 64-bit counters and FDIVP; the numerator
    // is the first, the denominator the second.
    std::int64_t completed = 0;
    std::int64_t total = 0;
    source.read_counters(completed, total);
    if (total == 0) {
        // The listing divides unconditionally and would raise the x87 divide
        // exception here. The reconstruction refuses rather than inventing a
        // result; see the doc's Uncertainties.
        return;
    }
    screen.progress = static_cast<float>(static_cast<double>(completed) /
        static_cast<double>(total));
}

void enter_please_wait_screen_0060d390(PleaseWaitScreen& screen,
    PleaseWaitLifecycleHost& host) {
    // 0060D393. XOR CL,CL before the call: the argument is the low byte of ECX,
    // so `this` is not the receiver here despite MOV ESI,ECX two bytes earlier.
    host.set_global_flag_00a94c50(false);
    // 0060D39A..0060D3A7. FLDZ pushed as the single float argument.
    host.input_manager_00a92c40(0.0f);
    // 0060D3AC. The screen re-arms its own sentinel on every enter.
    screen.progress = kPleaseWaitProgressUnknown;
    // 0060D3B9..0060D3CE. Two separate 004C12B0 calls, one per helper.
    host.gui_manager_00aa0f70();
    host.gui_manager_00aa0e00(0);
}

void exit_please_wait_screen_0060d2d0(PleaseWaitScreen& screen,
    bool global_byte_00f889c0_set, PleaseWaitLifecycleHost& host) {
    // 0060D2D0..0060D2DD. SETE CL off CMP byte ptr [00F889C0],0, so the flag is
    // raised exactly when that byte is clear - the inverse of the enter call.
    host.set_global_flag_00a94c50(!global_byte_00f889c0_set);
    // 0060D2E2..0060D2EC. this+0Ch, the Background_Icon widget, hidden.
    host.background_icon_set_visible(false);
    (void)screen;
}

}  // namespace bsp
