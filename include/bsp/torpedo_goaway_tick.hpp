#pragma once

namespace bsp {
// Semantic interfaces, not native object layouts or binary replacements.
// Names are hypotheses. Native addresses, ABI and limits:
// docs/TORPEDO_AFTER_THE_DROP.md sections 3.1-3.5 and
// docs/TORPEDO_FLY_TO_SOLVER.md.

// ---------------------------------------------------------------------------
// 009D0C10 BSP_BotStateTorpedoGoAway_UpdateGeometry, body 009D0C10-009D0D87,
// `void __thiscall(this)`, no stack argument. It is called from the enter
// (009D0E3A) and first in every tick (009D0F46), and its only store is
// state+18h.
// ---------------------------------------------------------------------------
namespace torpedo_goaway_tick {
inline constexpr float kTurnClampLo = -0.5235988f;   // 00CEC728, FLD dword
inline constexpr float kTurnClampHi = 0.5235988f;    // 00CEC724, MOVSS
inline constexpr float kProbeNudge = 0.69813174f;    // 00D20CC0, FMUL qword
inline constexpr float kBreakOffSlack = 100.0f;      // 00D7A220, FSUB qword
inline constexpr float kReSeedGuard = 6.0f;          // 00CE6628, FADD qword
inline constexpr float kWindowJitterLo = 3.0f;       // 00CE3854, FLD dword
inline constexpr float kWindowJitterHi = 6.0f;       // 00CE6630, FLD dword
inline constexpr float kHalf = 0.5f;                 // 00D7A280, FMUL qword
inline constexpr float kLowAltitudeSplit = 20.0f;    // 00CE3930, COMISS
inline constexpr float kClimbOutAltitude = 1000.0f;  // 00CE3804, FLD dword
inline constexpr float kRollClampLo = -1.2f;         // 00D05EA4, FLD dword
inline constexpr float kRollClampHi = 1.2f;          // 00CE3814, MOVSS
// 009FB800's second argument at all three sites is the literal FLD1.
inline constexpr float kPitchReference = 1.0f;
// The enter tail, 009D0E3A-009D0F04.
inline constexpr float kClimbJitterLo = 50.0f;       // 00CEB4D4, FLD dword
inline constexpr float kClimbJitterHi = 100.0f;      // 00CE3D08, FLD dword
inline constexpr float kHighOffset = 30.0f;          // 00CE7630, FADD qword
inline constexpr float kHighCap = 50.0f;             // 00CE3938, FLD qword
}  // namespace torpedo_goaway_tick

struct TorpedoGoAwayGeometryInputs {
    // unit->vtable[50h] at 009D0CDE and again at 009D0D75. 0074E260 is
    // `FLD [ECX+0C6Ch]; RET`, so this is unit+C6Ch, the latched heading.
    float unit_heading_c6c = 0.0f;
    // 009FD570's return at 009D0C54, a compass heading in [0, 2pi).
    float break_off_bearing = 0.0f;
    // 007F0280's three out-slots at 009D0CB7. The caller zeroes all three at
    // 009D0C96-009D0CAA, so an unmodelled probe leaves the term at exactly 0
    // and the 009D0D50 nudge - which needs a strict sign - never fires.
    float probe[3] = {0.0f, 0.0f, 0.0f};
};

// state+18h, the heading both arms of the tick publish.
float torpedo_goaway_heading_009d0c10(
    const TorpedoGoAwayGeometryInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D0D90-009D0F04, the enter. Its head (through 009D0E3A) is already
// `torpedo_goaway_enter_009d0d90` in bsp/torpedo_task_arm.hpp and produces
// +24h and +2Ch; this is the tail, which produces the four fields the tick
// reads and 009D0E3A's call to the geometry update.
// ---------------------------------------------------------------------------
struct TorpedoGoAwayEnterTailInputs {
    bool has_ordnance_132 = false;   // approach+132h, 009D0E42
    float alt_floor_74 = 0.0f;       // approach+74h
    float alt_margin_78 = 0.0f;      // approach+78h
    // [[approach+0Ch]+394h]: 009D0E7C is MOV ECX,[EAX+0Ch] / FLD [ECX+394h], and
    // 009F9CE0 sets approach+0Ch = unit+9D4h, the PILOT CONTROL BLOCK, so this is
    // Pilot/Torpedo/CruisingAlt (500 in this installation), not a squadron field as
    // the first reading named it. When the ordnance byte is clear this is the ONLY
    // producer of the climb altitude.
    float pilot_cruising_alt_394 = 0.0f;
    bool has_pilot_cruising_alt_394 = false;
    // BSP_Random_UniformFloatRange(50.0, 100.0) at 009D0E71.
    float climb_jitter = torpedo_goaway_tick::kClimbJitterLo;
    // BSP_Random_UniformFloatRange(row+10h, row+14h) at 009D0EA3, the same
    // PilotBotParameters pair the tick re-seeds from at 009D1000.
    float window_delay_row_lo = 0.0f;
    float window_delay_row_hi = 0.0f;
    float window_delay_draw = 0.0f;
};

// The goaway state object, task+6D8h. Only the fields 009D0F10 reads.
struct TorpedoGoAwayRuntime {
    float heading_18 = 0.0f;         // 009D0C10's store
    float climb_altitude_1c = 0.0f;  // 009D0E88
    float high_threshold_20 = 0.0f;  // 009D0EEB / 009D0EFB
    float break_off_distance_24 = 0.0f;  // 009D0E37, from the enter head
    float window_delay_28 = 0.0f;    // 009D0EAB, then 009D0F5E / 009D1000
    float side_2c = 1.0f;            // 009D0DBC, from the enter head
    float window_elapsed_30 = 0.0f;  // 009D0EB6, then 009D0F81
    float window_length_34 = 0.0f;   // 009D0EB1, then 009D0FDE
    // False when the ordnance byte was clear at the enter and the host has no
    // squadron block: 009D0E7F's leg is then a hole, not a stand-in, and the
    // tick must not issue an altitude it invented.
    bool climb_altitude_known = false;
};

void torpedo_goaway_enter_tail_009d0e3a(const TorpedoGoAwayEnterTailInputs& in,
                                        TorpedoGoAwayRuntime& state) noexcept;

// ---------------------------------------------------------------------------
// 009D0F10 BSP_BotStateTorpedoGoAway_Tick, body 009D0F10-009D1210, `RET 4`.
// ---------------------------------------------------------------------------
struct TorpedoGoAwayTickInputs {
    float unit_altitude = 0.0f;   // unit world y, 009D0F2F
    float range_90 = 0.0f;        // approach+90h, 009D0F4E
    float elapsed_134 = 0.0f;     // approach+134h, 009D0F84
    // The geometry update's result for this tick. 009D0F46 runs it before
    // anything else, so a binder that leaves it stale commands last tick's
    // heading.
    float heading_18 = 0.0f;
    // BSP_Random_UniformFloatRange(3.0, 6.0) at 009D0FD7 and
    // UniformFloatRange(row+10h, row+14h) at 009D1000, both drawn only on the
    // re-seed.
    float window_jitter_draw = torpedo_goaway_tick::kWindowJitterLo;
    float window_delay_draw = 0.0f;
};

struct TorpedoGoAwayTickResult {
    // Every arm writes these five, 009D1040-009D1078 and 009D1160-009D1190.
    float throttle_278 = 1.0f;
    int throttle_mode_27c = 1;
    float airbrake_2a8 = 0.0f;
    int airbrake_mode_2ac = 1;
    int flag_2d8 = 0;

    // 009FB800(altitude, 1.0). `altitude_known` is false only on the hole in
    // TorpedoGoAwayRuntime::climb_altitude_known.
    bool commands_altitude = false;
    bool altitude_known = false;
    float altitude = 0.0f;

    // cmd+2C0h / cmd+2CCh. Mode 2 is the heading arm; mode 1 leaves the
    // heading alone and lets the roll target through (0099E2xx).
    bool commands_heading = false;
    float heading_2c0 = 0.0f;
    int heading_mode_2cc = 1;

    // cmd+2C4h, the bank target. Only the two arms that set mode 1 write it.
    bool commands_roll = false;
    float roll_2c4 = 0.0f;

    // Reported so a run can say which of the four arms it took.
    bool window_open = false;
    bool high = false;
    bool reseeded = false;
    int arm = 0;  // 1 window/low, 2 window/high, 3 post/high, 4 post/low
};

TorpedoGoAwayTickResult torpedo_goaway_tick_009d0f10(
    TorpedoGoAwayRuntime& state, const TorpedoGoAwayTickInputs& in,
    float dt) noexcept;

}  // namespace bsp
