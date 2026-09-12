#include "bsp/mission_blackout.hpp"

// Packet cc_mission_blackout. Evidence and uncertainty: docs/MISSION_BLACKOUT.md.
// Every branch below is transcribed from the listing of 008D1340, 005B9BA0 and
// 005B9800; the two float expressions come from the x87 sequences, not from the
// decompiler's algebra.
namespace bsp {

// 005B98AB..005B98D5. FLD +C0h / FSTP scratch / FLD +C4h / FLD scratch / FLD ST0 /
// FSUBP ST2 / FLD ST2 / FMULP ST2 / FLD ST3 / FDIVP ST2 / FADDP / FSTP +C0h.
// Reading the stack through: `level + step * (target - level) / remaining`, with
// every intermediate in the x87 register stack and only the result narrowed to a
// float32. `double` is the closest portable stand-in for that width.
float mission_blackout_blend_005b98ab(float level, float target, float remaining,
    float step) noexcept {
    const double current = static_cast<double>(level);
    const double delta = static_cast<double>(target) - current;
    const double advance = (static_cast<double>(step) * delta)
        / static_cast<double>(remaining);
    return static_cast<float>(current + advance);
}

// 008D142F..008D1612, the argument decode. The order matters: the default level
// is the constant at 00D7A24C, argument 4 may replace it, and only then does the
// `enable` boolean of argument 1 force it to zero (008D1602..008D160C). So
// `Blackout(false, cb, d)` fades *out* of black whatever argument 4 said.
MissionBlackoutArgs mission_blackout_decode_008d1340(const MissionBlackoutLuaCall& call,
    float configured_duration) {
    MissionBlackoutArgs args;

    // 008D1481..008D14DD: argument 2 is copied into the local NativeString only
    // when the frame has one. With no argument 2 the string stays empty, and an
    // empty name is what 005B9800 treats as "no callback".
    if (call.argument_count > 1) args.callback = call.callback;

    // 008D14E2 reads *(00432650() + E0h) before the count test, so a frame with
    // no argument 3 gets the configured duration.
    float duration = configured_duration;
    if (call.argument_count > 2) {
        if (call.duration_is_boolean) {
            // 008D1557..008D1587: true means "no fade", false means "the
            // configured duration", re-read from the same field at 008D156B.
            duration = call.duration_boolean ? 0.0f : configured_duration;
        } else {
            duration = call.duration_number;  // 008D1598
        }
    }

    float level = kMissionBlackoutDefaultLevel;  // 00D7A24C at 008D15B2
    if (call.argument_count > 3) level = call.level_number;  // 008D15E8
    if (!call.enable) level = 0.0f;                          // 008D1602

    args.duration = duration;
    args.level = level;
    return args;
}

// 005B9800, __thiscall(screen, float step), RET 4.
MissionBlackoutStep mission_blackout_update_005b9800(MissionBlackoutFade& fade,
    float step, MissionBlackoutHost& host) {
    MissionBlackoutStep record;

    const float remaining = fade.remaining;  // 005B981E, read once into a local
    if (!(remaining > kMissionBlackoutZero)) {
        // 005B98E0, the idle and completion arm. The level is snapped to the
        // target every pass, which is why a finished fade keeps painting.
        fade.level = fade.target;  // 005B98E0 / 005B98EC
        // 005B98E7..005B990D compares +CCh against the empty literal at 00CE3A0C
        // through 00449AF0, the case-insensitive inequality. An empty name is the
        // only value that stops the callback, so `Blackout(x, "", d)` is silent.
        if (!fade.callback.empty()) {
            // 005B9933..005B9969: copy the name out (00426060), clear the field
            // (0041E350 with the empty literal) and only then call. Clearing
            // first is what lets the callback arm the next fade from inside
            // itself, which is exactly what `luaIngameMovieBOStart` does.
            record.callback_fired = true;
            record.callback = fade.callback;
            fade.callback.clear();
            host.mission_lua_call_named_00887e50(record.callback);
        }
    } else if (step < remaining) {
        // 005B983E FCOMI / JC: strictly less than, so a step equal to the
        // remaining time takes the completion arm instead.
        fade.level = mission_blackout_blend_005b98ab(fade.level, fade.target,
            remaining, step);
        fade.remaining = remaining - step;  // 005B98D3 FSUBP
    } else {
        // 005B9842, the arm that lands on the target. It does not call back; the
        // next pass finds remaining == 0 and takes the arm above, so the callback
        // is always one update behind the visual end of the fade.
        record.reached_target = true;
        fade.remaining = kMissionBlackoutZero;  // 005B9851
        fade.level = fade.target;               // 005B9859
        if (fade.target > kMissionBlackoutZero) {
            void* const unit = host.local_player_unit_00e188d8();  // 005B9867
            if (unit != nullptr && !host.interface_request_pending_005b66d0()) {
                host.ingame_interface_store_1c_00644220(0);  // 005B988E
                host.push_interface_request_004cc460(
                    kMissionBlackoutCompletionInterfaceRequest, unit);  // 005B98A1
            }
        }
    }

    // 005B9995, the paint tail. It runs on every arm, including the idle one.
    if (!(fade.level > kMissionBlackoutZero)) {
        host.blackout_icon_set_visible(false);  // 005B9A34/005B9A3B, the immediate 0
    } else {
        host.blackout_icon_set_visible(true);  // 005B99B0..005B99B7, the immediate 1
        BlackoutFillColour colour;
        host.blackout_icon_get_colour(colour);  // 005B99C9
        float alpha = fade.level;               // 005B99D8 writes it into the +Ch slot
        if (host.game_session_kind_1fe4h() != 0) {
            // 005B99F4..005B99FE: FLD the level, FMUL the double 0.5 at 00D7A280,
            // FSTP back into the alpha slot. The halving happens before the
            // branch, so it applies whatever the 0.75f test says.
            alpha = static_cast<float>(static_cast<double>(fade.level)
                * kMissionBlackoutSessionAlphaScale);
            // 005B99ED sets the flags the JBE at 005B9A02 reads; the x87 ops
            // between them leave EFLAGS alone.
            if (fade.level > kMissionBlackoutSessionForceShowLevel) {
                host.force_show_please_wait_screen_00e19698();  // 005B9A04..005B9A20
            }
        }
        colour.alpha = alpha;
        host.blackout_icon_set_colour(colour);  // 005B9A22..005B9A3B
    }

    record.still_fading = fade.remaining > kMissionBlackoutZero;  // 005B9A45
    return record;
}

// 005B9BA0, __thiscall(screen, float level, float duration, const NativeString*),
// RET 0Ch.
MissionBlackoutStep mission_blackout_arm_005b9ba0(MissionBlackoutFade& fade,
    const MissionBlackoutArgs& args, MissionBlackoutHost& host) {
    // 005B9BA0..005B9BB9: FLD the float duration, FADD the *double* 1.0e-4 at
    // 00D7A268, FSTP as a float32 into +C8h. The epsilon is why a zero duration
    // still arms a positive remaining.
    fade.remaining = static_cast<float>(static_cast<double>(args.duration)
        + kMissionBlackoutArmEpsilon);
    fade.target = args.level;  // 005B9BC7, MOVSS

    // 005B9BC5 compares `this + CCh` with the incoming string pointer and skips
    // the copy when they are the same object. That guard is a self-assignment
    // test with no observable effect here; the copy itself is 0041DD40 + memcpy.
    fade.callback = args.callback;

    // 005B9BF5..005B9C01: one immediate update with the float 1.0e-4 at 00CE3C68.
    MissionBlackoutStep record = mission_blackout_update_005b9800(fade,
        kMissionBlackoutArmStep, host);

    // 005B9C06..005B9C36: only when `game+1FE4h` is exactly 1. The message carries
    // the same (level, duration) the caller passed, not the epsilon-adjusted
    // remaining.
    if (host.game_session_kind_1fe4h() == 1) {
        host.session_broadcast_blackout_0076d310(args.level, args.duration);
    }
    return record;
}

// 008D1340, the binding body once the Lua marshalling is lifted out.
MissionBlackoutStep mission_blackout_binding_008d1340(const MissionBlackoutLuaCall& call,
    float configured_duration, MissionBlackoutFade& fade, MissionBlackoutHost& host) {
    const MissionBlackoutArgs args = mission_blackout_decode_008d1340(call,
        configured_duration);
    return mission_blackout_arm_005b9ba0(fade, args, host);  // 008D1635
}

}  // namespace bsp
