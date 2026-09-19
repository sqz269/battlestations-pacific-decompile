#include "bsp/torpedo_goaway_tick.hpp"

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

float clamp_to(float v, float lo, float hi) noexcept {
    // 009D0C97-009D0D27 and 009D10B1-009D10DD are the same shape: compare
    // against the low bound first, then the high, both with JBE so an exact
    // bound keeps the value.
    if (lo > v) return lo;
    if (v > hi) return hi;
    return v;
}

}  // namespace

float torpedo_goaway_heading_009d0c10(
    const TorpedoGoAwayGeometryInputs& in) noexcept {
    // 009D0CE0-009D0CF3: the error is bearing MINUS the current heading, in
    // that order, and 00438B10 returns it in (-pi, pi].
    const float err = wrapped_angle_subtract_00438b10(in.break_off_bearing,
                                                      in.unit_heading_c6c);
    float turn = clamp_to(err, torpedo_goaway_tick::kTurnClampLo,
                          torpedo_goaway_tick::kTurnClampHi);

    // 009D0CC1-009D0CD6: probe = -p[0] * p[1] * p[2]. The caller zeroes all
    // three slots before 007F0280 (009D0C96-009D0CAA), so on a host with no
    // terrain probe this is exactly 0 and the sign tests below are both false.
    const float probe = -in.probe[0] * in.probe[1] * in.probe[2];

    // 009D0D2A-009D0D4E, both clauses, with the strict sign tests the listing
    // uses: a zero probe cannot satisfy either.
    const bool push = (turn > 0.0f && probe < 0.0f) ||
                      (turn < 0.0f && probe > 0.0f);
    if (push) {
        turn = static_cast<float>(
            static_cast<double>(turn) +
            static_cast<double>(probe) *
                static_cast<double>(torpedo_goaway_tick::kProbeNudge));
    }
    // 009D0D7B: the store is the aircraft's own heading plus the turn.
    return wrapped_angle_add_00438aa0(in.unit_heading_c6c, turn);
}

void torpedo_goaway_enter_tail_009d0e3a(const TorpedoGoAwayEnterTailInputs& in,
                                        TorpedoGoAwayRuntime& state) noexcept {
    // 009D0E42: the ordnance byte picks which producer fills the climb
    // altitude. The first leg is the release band plus a 50-to-100 m draw; the
    // second is the squadron ceiling, which this reconstruction reports as
    // absent rather than substituting a number the image never uses.
    const float band = in.alt_margin_78 + in.alt_floor_74;
    if (in.has_ordnance_132) {
        state.climb_altitude_1c = in.climb_jitter + band;
        state.climb_altitude_known = true;
    } else if (in.has_squadron_394) {
        state.climb_altitude_1c = in.squadron_alt_limit_394;
        state.climb_altitude_known = true;
    } else {
        state.climb_altitude_1c = 0.0f;
        state.climb_altitude_known = false;
    }

    // 009D0EA3 / 009D0EAB.
    state.window_delay_28 = in.window_delay_draw;
    // 009D0EB1 / 009D0EB6.
    state.window_length_34 = 0.0f;
    state.window_elapsed_30 = 0.0f;

    // 009D0EBB-009D0EFB: min(band + 30, 50). It is a MINIMUM, not a maximum -
    // the JBE at 009D0EE1 takes the computed value and falls through to the
    // 50.0 constant - so the `high` test the tick runs is never above 50 m.
    const float h = static_cast<float>(
        static_cast<double>(band) +
        static_cast<double>(torpedo_goaway_tick::kHighOffset));
    state.high_threshold_20 =
        (h > torpedo_goaway_tick::kHighCap) ? torpedo_goaway_tick::kHighCap : h;
}

TorpedoGoAwayTickResult torpedo_goaway_tick_009d0f10(
    TorpedoGoAwayRuntime& state, const TorpedoGoAwayTickInputs& in,
    float dt) noexcept {
    TorpedoGoAwayTickResult out;

    // 009D0F2F-009D0F42, before anything else.
    const bool high = in.unit_altitude > state.high_threshold_20;
    out.high = high;

    // 009D0F46 is the geometry update; the caller has already run it and hands
    // its result in, because it needs the fly-to solver and a world query.
    state.heading_18 = in.heading_18;

    // 009D0F56-009D0F64: the delay only runs down while the aircraft is still
    // well inside the break-off ring.
    if (static_cast<double>(in.range_90) <
        static_cast<double>(state.break_off_distance_24) -
            static_cast<double>(torpedo_goaway_tick::kBreakOffSlack)) {
        state.window_delay_28 -= dt;
    }
    // 009D0F6E / 009D0F81.
    state.window_elapsed_30 += dt;

    // 009D0F84-009D0FA6: a fresh approach (its own clock still under a second)
    // whose window has been shut for more than six seconds forces a re-seed.
    if (in.elapsed_134 < 1.0f &&
        static_cast<double>(state.window_elapsed_30) >
            static_cast<double>(state.window_length_34) +
                static_cast<double>(torpedo_goaway_tick::kReSeedGuard)) {
        state.window_delay_28 = -1.0f;
    }

    // 009D0FB2-009D1025.
    if (state.window_delay_28 < 0.0f && high) {
        out.reseeded = true;
        state.window_length_34 = in.window_jitter_draw;
        state.window_delay_28 = in.window_delay_draw + state.window_length_34;
        state.window_elapsed_30 = static_cast<float>(
            -(static_cast<double>(state.window_length_34) *
              static_cast<double>(torpedo_goaway_tick::kHalf)));
    }

    // Every arm writes these, 009D1040 and 009D1160.
    out.throttle_278 = 1.0f;
    out.throttle_mode_27c = 1;
    out.airbrake_2a8 = 0.0f;
    out.airbrake_mode_2ac = 1;
    out.flag_2d8 = 0;

    out.window_open = state.window_elapsed_30 < state.window_length_34;
    out.commands_altitude = true;
    out.altitude_known = state.climb_altitude_known;

    if (out.window_open) {  // 009D1032 JBE
        if (in.unit_altitude > torpedo_goaway_tick::kLowAltitudeSplit) {
            // 009D1096-009D10F4. Hold the climb altitude and roll into the
            // break-off, harder the longer the window has run.
            out.arm = 2;
            out.altitude = state.climb_altitude_1c;
            const float roll = static_cast<float>(
                2.0 * static_cast<double>(state.side_2c) *
                static_cast<double>(state.window_elapsed_30));
            out.commands_roll = true;
            out.roll_2c4 = clamp_to(roll, torpedo_goaway_tick::kRollClampLo,
                                    torpedo_goaway_tick::kRollClampHi);
            out.heading_mode_2cc = 1;
        } else {
            // 009D10F6-009D1112. Below 20 m the image stops rolling and asks
            // for a thousand metres on the break-off heading. This is the only
            // climb command in the whole torpedo chain.
            out.arm = 1;
            out.altitude = torpedo_goaway_tick::kClimbOutAltitude;
            out.altitude_known = true;  // a literal, not the +1Ch hole
            out.commands_heading = true;
            out.heading_2c0 = state.heading_18;
            out.heading_mode_2cc = 2;
        }
    } else {
        // 009D1156-009D120E. The altitude command is issued before the split,
        // so both post-window arms ask for +1Ch.
        out.altitude = state.climb_altitude_1c;
        if (high) {
            out.arm = 3;
            out.commands_heading = true;
            out.heading_2c0 = state.heading_18;
            out.heading_mode_2cc = 2;
        } else {
            out.arm = 4;
            out.commands_roll = true;
            out.roll_2c4 = 0.0f;
            out.heading_mode_2cc = 1;
        }
    }
    return out;
}

}  // namespace bsp
