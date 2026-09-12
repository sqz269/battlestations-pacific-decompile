#include "bsp/submarine_model.hpp"

#include <cmath>

// Reconstruction of the submarine unit's depth, air and crush-depth model.
// docs/SUBMARINE_MODEL.md carries the evidence and the coverage table; every
// name is a hypothesis except the eight the save schema at 00853E10 recovers.

namespace bsp {
namespace {

// 00415510 BSP_Math_MinFloatByRef and 00415550 BSP_Math_MaxFloatByRef, the two
// helpers the native calls instead of writing the comparison inline. 00936DC0
// reaches 00415510 for the seabed clamp.
float max_float(float a, float b) { return a < b ? b : a; }

}  // namespace

// ---------------------------------------------------------------------------
// Depth bands
// ---------------------------------------------------------------------------

std::array<float, kSubDepthBandCount> submarine_build_depth_bands_00853630(
    const SubmarineClassDepths& keys) noexcept {
    const float class_keys[kSubDepthBandCount] = {
        0.0f,  // +80Ch is the PeriscopeWave effect handle; the i<1 test never reads it
        keys.periscope_depth, keys.swim_depth_2, keys.swim_depth_3,
    };
    std::array<float, kSubDepthBandCount> bands{};
    for (int i = 0; i < kSubDepthBandCount; ++i) {
        // 00853A90 is the i<=0 test (CMP EDI,EBX with EBX zero, JLE) and
        // 00853A9F the class-key sign test; either one takes the default table.
        if (i < 1 || class_keys[i] < 0.0f) {
            bands[static_cast<std::size_t>(i)] = kSubDefaultBandDepths[static_cast<std::size_t>(i)];
        } else {
            // 00853AA4..00853AAC: the positive Lua metres become a negative
            // world Y through MOVAPS of DAT_00D7A208 then SUBSS.
            bands[static_cast<std::size_t>(i)] = kSubDepthSignOrigin - class_keys[i];
        }
    }
    return bands;
}

SubmarineDepthBand submarine_nearest_depth_band_00853630(
    const std::array<float, kSubDepthBandCount>& bands, float hull_world_y) noexcept {
    // 00853B77: the incumbent is band 0 with the error |hullY|, which is the
    // same as |hullY - bands[0]| only because bands[0] is always 0.0.
    float best = std::fabs(hull_world_y);
    SubmarineDepthBand best_band = SubmarineDepthBand::surface;
    for (int i = 1; i < kSubDepthBandCount; ++i) {
        const float error = std::fabs(hull_world_y - bands[static_cast<std::size_t>(i)]);
        // 00853B9E is a strict <, so a tie keeps the shallower band.
        if (error < best) {
            best = error;
            best_band = static_cast<SubmarineDepthBand>(i);
        }
    }
    return best_band;
}

// ---------------------------------------------------------------------------
// The depth command
// ---------------------------------------------------------------------------

SubmarineDepthBand submarine_clamp_depth_command_008528b0(int requested,
                                                          bool kamikaze_class) noexcept {
    int level = requested;
    if (level < 0) {                       // 008528CC
        level = 0;
    } else if (level > kSubDepthBandCount - 1) {  // 008528D3 CMP with 3
        level = kSubDepthBandCount - 1;
    }
    if (kamikaze_class) {                  // 008528EC/008528FB, class +510h/+514h
        level = static_cast<int>(SubmarineDepthBand::periscope);
    }
    return static_cast<SubmarineDepthBand>(level);
}

SubmarineDepthBand submarine_step_depth_command_00852c60(SubmarineDepthBand current,
                                                         bool deeper) noexcept {
    const int level = static_cast<int>(current);
    if (deeper) {
        // 00852CA9: the deeper arm refuses to step past band 3 and leaves the
        // level alone rather than clamping through 008528B0.
        return level >= kSubDepthBandCount - 1 ? current
                                               : static_cast<SubmarineDepthBand>(level + 1);
    }
    // 00852C97: JLE, so band 0 is also a no-op.
    return level <= 0 ? current : static_cast<SubmarineDepthBand>(level - 1);
}

// ---------------------------------------------------------------------------
// Air supply
// ---------------------------------------------------------------------------

float submarine_air_breathing_line_00855250(float periscope_band_y) noexcept {
    const float sum = periscope_band_y + kSubAirSurfaceMargin;  // 0085525C
    // 00855270 FCOMIP of 0.0 against the sum, 00855274 JBE keeps the sum.
    return sum < 0.0f ? kSubAirBreathingLineFallback : sum;
}

SubmarineAirStepResult submarine_step_air_00855250(const SubmarineAirState& state,
                                                   float dt,
                                                   float hull_world_y,
                                                   float periscope_band_y,
                                                   const SubmarineAirRates& rates,
                                                   const SubmarineDepthSettings& settings,
                                                   bool already_dead) noexcept {
    SubmarineAirStepResult out;
    out.state = state;

    const float line = submarine_air_breathing_line_00855250(periscope_band_y);
    // 008552A4: hullY <= line is the submerged branch.
    out.submerged = !(hull_world_y > line);

    if (out.submerged) {
        // 008552CB: unlimitedAir skips the whole consumption block, warnings
        // included, so a scripted boat never even reaches the latch test below
        // through this arm.
        if (!state.unlimited_air) {
            const float previous = state.air;
            out.state.air = previous - dt / rates.run_out_time;  // 00855308
            const float need = settings.air_need_limit;
            const float warn = settings.air_warning_limit;
            // 00855319..00855375. The critical edge is the else of
            // "air > need || previous <= need", so it fires exactly on a
            // downward crossing of the need limit and suppresses the low edge.
            if (out.state.air <= need && previous > need) {
                out.warning = SubmarineAirWarning::critical;
            } else if (out.state.air <= warn && previous > warn) {
                out.warning = SubmarineAirWarning::low;
            }
        }
    } else {
        out.state.air = state.air + dt / rates.reload_time;  // 008552AA
    }

    // 0085537A: clamp high, then the drown test on the low side.
    if (out.state.air > kSubAirFull) {
        out.state.air = kSubAirFull;
    } else if (out.state.air < 0.0f && !already_dead) {
        // The store precedes the indirect call, so the renormalised value
        // survives whatever vtable[70h] does to the unit.
        out.state.air = 0.0f;
        out.drowned = true;
    }

    // 008553B4..0085540E, the needAir hysteresis. The native returns straight
    // after arming, so the disarm test is skipped on the arming step.
    if (!out.state.need_air) {
        if (out.state.air < settings.air_need_limit) {
            out.state.need_air = true;
            return out;
        }
        return out;
    }
    if (out.state.air > settings.air_enough_limit) {
        out.state.need_air = false;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Crush depth
// ---------------------------------------------------------------------------

SubmarineCrushStepResult submarine_step_crush_008551c0(
    float accumulator, float dt, float hull_world_y,
    const SubmarineDepthSettings& settings) noexcept {
    SubmarineCrushStepResult out;
    // 008551C6: the sum is stored back before the test, so the field carries
    // the un-pulsed remainder between pulses.
    out.accumulator = accumulator + dt;
    if (!(out.accumulator > kSubCrushTickSeconds)) {  // 008551DD COMISS, strict >
        return out;
    }
    out.pulsed = true;
    // 00855206: the limit is the global setting negated, never a class key.
    if (hull_world_y < -settings.damage_depth) {
        // 0085522F: the damage scales with the accumulated interval, so the
        // rate is frame-rate independent.
        out.damage = settings.depth_damage * out.accumulator;
        out.report = true;
    }
    out.accumulator = 0.0f;  // 00855243
    return out;
}

// ---------------------------------------------------------------------------
// The band the physics actually chases
// ---------------------------------------------------------------------------

SubmarineDepthBand submarine_effective_depth_band_00936dc0(
    const SubmarineEffectiveBandInputs& in) noexcept {
    SubmarineDepthBand band = in.commanded;
    // 00936E2B: needAir or the catapult rule surfaces the boat, but a kamikaze
    // class is exempt and stays where 008528B0 pinned it.
    if ((in.need_air || in.catapult_wants_surface) && !in.kamikaze_class) {
        band = SubmarineDepthBand::surface;
    }
    // 00936E6D: a dead boat is driven to the deepest band, which is how a wreck
    // sinks. This wins over the surface override.
    if (in.dead) {
        band = SubmarineDepthBand::max_depth;
    }
    return band;
}

SubmarineDepthTarget submarine_depth_target_00936dc0(float band_y, float clearance_y,
                                                     bool clearance_valid) noexcept {
    SubmarineDepthTarget out;
    out.target_y = band_y;   // 00936E9E, bands[effective]
    out.gain = kSubAirFull;  // DAT_00D7A24C, the unclamped gain is 1.0
    // 00936EC6: the clamp needs the +740h owner to be a submarine class as well
    // as an ordered depth below the scan output.
    if (clearance_valid && band_y < clearance_y) {
        out.target_y = max_float(band_y, clearance_y);  // 00415510's caller
        out.gain = kSubClearanceClampGain;              // DAT_00CE380C
        out.clamped_by_seabed = true;
    }
    return out;
}

float submarine_accumulate_clearance_00855420(float accumulated_y, float sample_y,
                                              float class_height) noexcept {
    // 0085580C..0085583C: the candidate is the provider height raised by the
    // boat's own height and a three-metre margin, and the accumulator keeps the
    // maximum over the footprint ring, i.e. the shallowest floor.
    const float candidate = sample_y + class_height + kSubClearanceMargin;
    return max_float(accumulated_y, candidate);
}

// ---------------------------------------------------------------------------
// The periscope
// ---------------------------------------------------------------------------

bool submarine_periscope_should_auto_raise_00854650(bool auto_raise_enabled,
                                                    SubmarineDepthBand commanded,
                                                    SubmarinePeriscopeState state,
                                                    float hull_world_y,
                                                    float periscope_band_y) noexcept {
    if (state == SubmarinePeriscopeState::broken) {  // 00854E44 took the repair arm
        return false;
    }
    if (!auto_raise_enabled) {  // 00854ED7, the +1235h gate
        return false;
    }
    if (commanded != SubmarineDepthBand::periscope) {  // 00854EDF, depthLevel == 1
        return false;
    }
    // 00854EF6..00854F46, two strict tests around the band. 00854F10 is D8 E9,
    // FSUBR ST(0),ST(1), so the first bound is bandY - 2.5 and not 2.5 - bandY;
    // the second reads the hull Y again through
    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed at +4h. Both are JBE
    // skips, so the window is open at both ends.
    return hull_world_y > periscope_band_y - kSubPeriscopeAutoRaiseWindow &&
           periscope_band_y + kSubPeriscopeAutoRaiseWindow > hull_world_y;
}

bool submarine_periscope_is_out_00855057(float mast_local_y, float periscope_rest_y,
                                         float periscope_move_range) noexcept {
    // 00855039..00855051: the threshold is the full travel less one metre, and
    // the test is a strict >= through JB on the inverted comparison.
    const float threshold = periscope_move_range + periscope_rest_y - kSubPeriscopeOutMargin;
    return !(mast_local_y < threshold);
}

SubmarinePeriscopeRepairStep submarine_step_periscope_repair_00854650(
    const SubmarinePeriscopeRepair& repair, float dt, bool repair_boosted) noexcept {
    SubmarinePeriscopeRepairStep out;
    out.repair = repair;
    // 00854E56: the multiplier applies only when unit+A44h is 1.
    const float step = repair_boosted ? dt * kSubFailureRepairMultiplierInstalled : dt;
    out.repair.clock_fast = repair.clock_fast + step * 2.0f;  // 00854E8F FADD ST,ST
    out.repair.clock = repair.clock + step;                   // 00854E9B
    // 00854EAF: the deadline is absolute, so this is a comparison and not a
    // countdown. JB on the inverted compare makes completion inclusive.
    if (!(out.repair.clock < out.repair.deadline)) {
        out.completed = true;
        out.repair.deadline = 0.0f;  // 0092BEC0's store
    }
    return out;
}

SubmarinePeriscopeRepair submarine_break_periscope_009373c0(
    float clock_now, float periscope_repair_time) noexcept {
    SubmarinePeriscopeRepair out;
    out.clock_fast = 0.0f;                                  // 009373E9-region store
    out.clock = clock_now;                                  // DAT_00F876A4 snapshot
    out.deadline = periscope_repair_time + clock_now;        // settings +4C4h
    return out;
}

SensorCategory submarine_periscope_sensor_state(bool periscope_out) noexcept {
    // 00852C4C: SETNE plus 2, so the byte picks 3 over 2.
    return periscope_out ? SensorCategory::periscope_out : SensorCategory::periscope_in;
}

// ---------------------------------------------------------------------------
// The motion tick tail
// ---------------------------------------------------------------------------

void submarine_run_motion_tick_tail_00855420(SubmarineMotionTickHost& host,
                                             SubmarineStepState& state, float dt,
                                             float periscope_band_y,
                                             const SubmarineAirRates& rates,
                                             const SubmarineDepthSettings& settings) {
    // 0085591C: 00855250(unit, dt), ECX = EBP = the unit.
    const float air_hull_y = host.refresh_and_read_hull_world_y();
    const SubmarineAirStepResult air = submarine_step_air_00855250(
        state.air, dt, air_hull_y, periscope_band_y, rates, settings, state.dead);
    state.air = air.state;
    switch (air.warning) {
        case SubmarineAirWarning::low:
            host.report_air_low();
            break;
        case SubmarineAirWarning::critical:
            host.report_air_critical();
            break;
        case SubmarineAirWarning::none:
            break;
    }
    if (air.drowned) {
        host.destroy_and_broadcast(1);  // vtable[70h] with the literal 1
    }

    // 0085592B: 008551C0(unit, dt), the same ECX and the same dt. Its pose
    // refresh is inside the once-a-second branch (008551E9), not at the top, so
    // the hull Y is read only on a pulse.
    const float pending = state.crush_accumulator + dt;
    if (pending > kSubCrushTickSeconds) {
        const float crush_hull_y = host.refresh_and_read_hull_world_y();
        const SubmarineCrushStepResult crush =
            submarine_step_crush_008551c0(state.crush_accumulator, dt, crush_hull_y, settings);
        state.crush_accumulator = crush.accumulator;
        if (crush.report) {
            host.add_damage(crush.damage);  // vtable[1ACh] precedes the report
            host.report_depth_damage();
        }
    } else {
        state.crush_accumulator = pending;  // 008551C6 stores the sum regardless
    }
}

}  // namespace bsp
