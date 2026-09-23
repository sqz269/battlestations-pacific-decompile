#pragma once

// Packet cc9_aimdive_response. The aimdive tick's tail, 009C5DB8-009C6080,
// after the pitch and roll commands that dive_bomb_aimdive_steer_009c5c9f
// binds: a YAW command aimed at the aim point in the aircraft's own frame,
// and the THROTTLE and AIR BRAKE commands. docs/AIMDIVE_RESPONSE.md.
//
// Original ABI: inline in 009C58D0 (__thiscall(state, float dt), RET 4); no
// separate routine. Everything here is a value projection of that listing.
// The commands land in the pilot command buffer at [unit+18h]:
//   cmd+284h yaw (+288h = 1, +2D4h = 0)       009C5E5D/009C5E3F/009C5E63
//   cmd+278h throttle (+27Ch = 1)             009C5F37 or 009C605F, 009C606A
//   cmd+2A8h air brake (+2ACh = 1, +2D8h = 0) 009C6071/009C6079/009C6080

namespace bsp {

namespace dive_bomb_tail_constant {
// 009C5E57 FMUL double [00CE3DB0]: the yaw gain on x/|z|.
inline constexpr double kYawGain = 8.0;                       // 00CE3DB0
// 009C5EA8 FLD double [00CE3830]: |bank| past a right angle folds the angle.
inline constexpr double kHalfPi = 1.5707963705062866;         // 00CE3830
// 009C5EBC FSUBR double [00CE3D28].
inline constexpr double kPi = 3.1415927410125732;             // 00CE3D28
// 009C5F0F FADD double [00CE3938]: the pull-out margin's fixed part, metres.
inline constexpr double kPullOutMarginBase = 50.0;            // 00CE3938
// 009C5F2F: the throttle below the release floor.
inline constexpr float kFloorThrottle = 0.05000000074505806f; // 00CE7638
// 009C5FDC FDIV double [00D1F3F8]: the height band the power ramp spans.
inline constexpr double kPowerHeightBand = 120.0;             // 00D1F3F8
// 009C5FEA / 009C6006: the clamp of that ramp.
inline constexpr float kPowerRampMin = 0.009999999776482582f; // 00D7A238
inline constexpr float kPowerRampMax = 1.0f;                  // 00D7A24C
}  // namespace dive_bomb_tail_constant

struct DiveBombAimDiveTailInputs {
    // The aim point (approach->vtable[0], 009C5DF1) carried into the
    // aircraft's frame by the inverse 00B63D50 builds at unit+110h
    // (009C5DE0) and 004142E0 (009C5E01): [ESP+40h] x, [ESP+48h] z.
    float aim_local_x = 0.0f;
    float aim_local_z = 0.0f;
    float pitch_c64 = 0.0f;          // pose+C64h, 009C5E6F
    float bank_c68 = 0.0f;           // pose+C68h, 009C5E86
    float speed_vtable38 = 0.0f;     // unit->vtable[38h], 009C5EDE
    float pitch_spd_1ac = 0.0f;      // (approach+8h)->+1ACh, class PitchSpd, 009C5EE4
    float height_above_aim_14 = 0.0f;  // [ESP+14h], 009C59D6
    float release_alt_a8 = 0.0f;     // approach+A8h, 009C5F20
    float pitch_command = 0.0f;      // [ESP+10h], the cmd+29Ch value, 009C5D02
    // (approach+14h)->+44h..+54h, the PilotBot row viewed 0Ch in:
    // robot_config.hpp's dive_bomb_*_ctrl_050.._05c and aim_pitch_ratio_060.
    float max_power_44 = 0.0f;
    float min_power_48 = 0.0f;
    float max_brake_4c = 0.0f;
    float min_brake_50 = 0.0f;
    float aim_pitch_ratio_54 = 0.0f;
};

struct DiveBombAimDiveTail {
    float yaw_284 = 0.0f;
    float throttle_278 = 0.0f;
    float air_brake_2a8 = 0.0f;
    // Census only.
    float pull_out_margin = 0.0f;   // [ESP+20h] at 009C5F15, 0 when not nose-down
    bool below_floor = false;       // the 009C5F2A arm
};

DiveBombAimDiveTail dive_bomb_aimdive_tail_009c5db8(
    const DiveBombAimDiveTailInputs& in) noexcept;

}  // namespace bsp
