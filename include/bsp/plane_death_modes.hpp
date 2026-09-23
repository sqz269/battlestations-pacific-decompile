#pragma once

// Packet cc9_plane_death_modes. How the image ends a shot-down aircraft, as
// pure rules. docs/PLANE_DEATH_MODES.md. Every name here is a hypothesis, not a
// recovered symbol; nothing is ABI-compatible or game-validated.

namespace bsp {

// The mode 007CA8A0 raises through the plane's named-effect slot 007BBFA0.
enum class PlaneDeathMode : int {
    None = 0,
    Explosion = 1,         // "explosion" (00D05A18)
    ExplosionDelayed = 2,  // "explosion_delayed" (00CE54C4)
    PowerLost = 3,         // "powerlost" (00D059F8)
    SpinLeft = 4,          // "leftspinning" (00D059DC)
    SpinRight = 5,         // "rightspinning" (00D059CC)
};

const char* plane_death_mode_name(PlaneDeathMode mode) noexcept;

// 007CA8A0, the plane's vtable[1B0h] (00D19D28 + 1B0h), which 00877B90
// BSP_UnitInstance_SetHealth calls at 00877C40 on every health change.
struct PlaneDeathModeInputs {
    float health_370{0.0f};        // unit+370h; above zero returns with no mode
    float draw{0.0f};              // 00BD2F10(stream 1, 0.0, 1.0) at 007CA914
    bool rammed_dfc{false};        // unit+DFCh, set by 007BBCF0 when hit+4 is a unit
    int spin_side_800{-1};         // unit+800h, -1 from 007CFDB4 unless a hull segment was hit
    float chance_explosion{0.0f};          // tuning +10h DeathModeChances/Explosion
    float chance_explosion_delayed{0.0f};  // tuning +14h .../Explosion_delayed
    float chance_spinning{0.0f};           // tuning +18h .../Spinning
    float chance_powerloss{0.0f};          // tuning +1Ch .../Powerloss
};

// The listing's order (007CA8C1-007CAA9D):
//   c1 = +10h, c2 = c1 + +14h, c3 = c2 + +1Ch; +20h Explodetoparts is not read.
//   unit+DFCh set                  -> Explosion
//   side 0 and draw < +18h         -> SpinLeft
//   side 1 and draw < +18h         -> SpinRight
//   draw < c1                      -> Explosion
//   draw < c2                      -> ExplosionDelayed
//   draw < c3                      -> PowerLost
//   otherwise                      -> ExplosionDelayed
PlaneDeathMode plane_death_mode_007ca8a0(const PlaneDeathModeInputs& in) noexcept;

// 007BBFA0: the explosion timer unit+C10h each mode stores before it routes
// its session message. Explosion -1.0f (00D7A260, immediate); ExplosionDelayed
// the draw 00BD2F10(stream 0, [00E18710], [00E1870C]); PowerLost and the two
// spins 1000.0f (00CE3804). `delayed_draw` is used only for ExplosionDelayed.
float plane_death_timer_c10_007bbfa0(PlaneDeathMode mode, float delayed_draw) noexcept;

// 007D0B80's arms for the mode's message: which of unit+C39h (power lost) and
// unit+C36h (spinning) the handler sets. Every arm reaches the tail 007D12FC,
// which sets unit+C3Ah and calls vtable[70h](1), so the aircraft is dead
// (unit+5Dh) from the message on. An immediate Explosion also calls
// BSP_MissionEntity_Kill(unit, 1) at 007D0CFD.
struct PlaneDeathFlags {
    bool powerlost_c39{false};
    bool spinning_c36{false};
    bool killed_now{false};
};
PlaneDeathFlags plane_death_flags_007d0b80(PlaneDeathMode mode, float timer_c10) noexcept;

// 007CAF10's death terms, run from the free-flight arm at 007CC322 (and from
// the other arms' vtable[1ECh] call).
struct PlaneDeathStepInputs {
    bool dead_5d{false};
    bool powerlost_c39{false};
    bool spinning_c36{false};
    bool free_flight_gate{true};   // (unit+72Ch)->vtable[38h]()
    float timer_c10{-1.0f};
    float dead_timer_c3c{0.0f};
    float step{0.0f};
    bool explosion_budget_ok{true};  // [00E186E8] <= [00E1873C] MaxExplosionNum
};
struct PlaneDeathStepResult {
    float dead_timer_c3c{0.0f};
    bool explode{false};            // raise "explosion": timer -> -1, Kill(1)
    bool zero_throttle{false};      // unit+9F0h and +BBCh = 0
    bool zero_air_brake{false};     // unit+9F4h and +BC0h = 0
};
PlaneDeathStepResult plane_death_step_007caf10(const PlaneDeathStepInputs& in) noexcept;

}  // namespace bsp
