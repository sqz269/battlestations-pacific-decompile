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

float plane_death_timer_c10_007bbfa0(PlaneDeathMode mode, float delayed_draw) noexcept {
    switch (mode) {
    case PlaneDeathMode::Explosion: return -1.0f;        // 00D7A260
    case PlaneDeathMode::ExplosionDelayed: return delayed_draw;
    case PlaneDeathMode::PowerLost:
    case PlaneDeathMode::SpinLeft:
    case PlaneDeathMode::SpinRight: return 1000.0f;      // 00CE3804
    case PlaneDeathMode::None: break;
    }
    return -1.0f;
}

PlaneDeathFlags plane_death_flags_007d0b80(PlaneDeathMode mode, float timer_c10) noexcept {
    PlaneDeathFlags out;
    switch (mode) {
    case PlaneDeathMode::PowerLost:
        out.powerlost_c39 = true;   // 007D0BEB 66h -> 007D12D6, 007D12E8
        break;
    case PlaneDeathMode::ExplosionDelayed:
    case PlaneDeathMode::Explosion:
        // 007D0BF9-007D0C2C: kind 63h with unit+C10h > 0 and the free-flight
        // gate takes the same arm as 66h; otherwise 007D0C37's arm kills.
        if (timer_c10 > 0.0f) {
            out.powerlost_c39 = true;
        } else {
            out.killed_now = true;  // 007D0CFD BSP_MissionEntity_Kill(unit, 1)
        }
        break;
    case PlaneDeathMode::SpinLeft:
    case PlaneDeathMode::SpinRight:
        out.spinning_c36 = true;    // kind 67h arm
        break;
    case PlaneDeathMode::None:
        break;
    }
    return out;
}

PlaneDeathStepResult plane_death_step_007caf10(const PlaneDeathStepInputs& in) noexcept {
    PlaneDeathStepResult out;
    out.dead_timer_c3c = in.dead_timer_c3c;
    // 007CAF4x: unit+C3Ch += step while unit+5Dh (or +5Eh) is set.
    if (in.dead_5d) out.dead_timer_c3c += in.step;
    // 0 < unit+C10h < unit+C3Ch and the explosion budget: raise "explosion".
    if (0.0f < in.timer_c10 && in.timer_c10 < out.dead_timer_c3c && in.explosion_budget_ok) {
        out.explode = true;
        return out;
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
