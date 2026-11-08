#include "bsp/plane_death_modes.hpp"

// Packet cc9_plane_death_modes. docs/PLANE_DEATH_MODES.md.

namespace bsp {

const char* plane_death_mode_name(PlaneDeathMode mode) noexcept {
    switch (mode) {
    case PlaneDeathMode::Explosion: return "explosion";
    case PlaneDeathMode::ExplosionDelayed: return "explosion_delayed";
    case PlaneDeathMode::PowerLost: return "powerlost";
    case PlaneDeathMode::SpinLeft: return "leftspinning";
    case PlaneDeathMode::SpinRight: return "rightspinning";
    case PlaneDeathMode::None: break;
    }
    return "none";
}

PlaneDeathMode plane_death_mode_007ca8a0(const PlaneDeathModeInputs& in) noexcept {
    // 007CA8E5-007CA8FB: COMISS 0.0, [unit+370h]; JC returns while health > 0.
    if (0.0f < in.health_370) return PlaneDeathMode::None;
    // 007CA8C6-007CA8F7, three FADDs into [ESP+10h], [ESP+14h], [ESP+18h].
    const float c1 = in.chance_explosion;
    const float c2 = in.chance_explosion_delayed + c1;
    const float c3 = in.chance_powerloss + c2;
    // 007CA91D: CMP byte [ESI+0DFCh], 0.
    if (in.rammed_dfc) return PlaneDeathMode::Explosion;
    // 007CA941-007CA993: unit+800h selects the spin side; the compare is the
    // draw against +18h alone, not against a running sum.
    if (in.spin_side_800 == 0 && in.draw < in.chance_spinning) return PlaneDeathMode::SpinLeft;
    if (in.spin_side_800 == 1 && in.draw < in.chance_spinning) return PlaneDeathMode::SpinRight;
    // 007CA9C9-007CAA7E.
    if (in.draw < c1) return PlaneDeathMode::Explosion;
    if (in.draw < c2) return PlaneDeathMode::ExplosionDelayed;
    if (in.draw < c3) return PlaneDeathMode::PowerLost;
    return PlaneDeathMode::ExplosionDelayed;
}

PlaneDeathStepResult plane_death_step_007caf10(const PlaneDeathStepInputs& in,
                                               const PlaneDeathEngineCalls& engines) {
    PlaneDeathStepResult out;
    out.dead_timer_c3c = in.dead_timer_c3c;
    out.engine_fire_timer_c14 = in.engine_fire_timer_c14;
    // 007CAF2B-007CAF41: unit+C3Ch += step while unit+5Dh (or +5Eh) is set
    // (FLD / FADD / FSTP float).
    if (in.dead_5d) out.dead_timer_c3c += in.step;
    float timer_c10 = in.timer_c10;
    if (!in.session_client_1fe4) {
        // Goal 5 chain 654: the engine block, 007CAF64-007CB07E (bsp.py show
        // 007caf10 --asm; docs/GUN_AIM_TERMS.md section 3). 007CAF7A-007CAF8B:
        // COMISS C14h, 0 / JC, so a negative or NaN C14h skips it.
        const float c14 = out.engine_fire_timer_c14;
        const float c3c = out.dead_timer_c3c;
        if (!in.controller_byte_94 && c14 >= 0.0f) {
            // 007CAF91-007CAFA9: FADD ST0,ST0 doubles C3Ch; FCOMIP / JBE.
            if (c3c + c3c > c14) engines.stop_all_engines();
            if (in.engine_count_140 > 0) {  // 007CAFB1-007CAFBE, CMP / JLE
                // 007CAFD8-007CAFE2: C3Ch > C14h (FCOMI / JA). Otherwise
                // 007CAFE4-007CB00F: C10h > 0 (COMISS / JBE) and C3Ch - C10h >
                // -C14h * 0.6 (FSUB float; FCHS; FMUL double 00CEFF98, which is
                // 0.6f widened, so the 24-bit product is the float product;
                // FCOMIP / JBE). Unordered compares do not fire.
                bool fire = c3c > c14;
                if (!fire && timer_c10 > 0.0f) fire = (c3c - timer_c10) > (-c14) * 0.6f;
                if (fire) {
                    out.engine_fire_timer_c14 = -1.0f;  // 007CB017-007CB028, 00D7A260
                    if (!engines.any_engine_burning()) {
                        timer_c10 = engines.raise_engine_fire();  // 007CB039-007CB060
                    }
                    engines.stop_all_engines();  // 007CB073-007CB075
                }
            }
        }
        // 007CB084-007CB0F8: 0 < unit+C10h < unit+C3Ch and the explosion
        // budget: raise "explosion".
        if (0.0f < timer_c10 && timer_c10 < out.dead_timer_c3c && in.explosion_budget_ok) {
            out.explode = true;
            return out;
        }
    }
    // The C39h/C36h block: in free flight only the throttle (C39h) and the air
    // brake are zeroed, live and latched; the pilot keeps steering.
    if ((in.powerlost_c39 || in.spinning_c36) && in.free_flight_gate) {
        out.zero_throttle = in.powerlost_c39;
        out.zero_air_brake = true;
    }
    return out;
}

}  // namespace bsp
