#pragma once

// Packet cc9_gun_aim_terms. The dogfight gun's aim distortion (009FA7E0) and
// the stick damping 009F9FC0 applies when gun+4Ah is set, as pure rules.
// docs/GUN_AIM_TERMS.md. Names are hypotheses, not recovered symbols.

namespace bsp {

// The 2-D wander object at dogfight-gun+4h (BSP_BotTaskGun_Construct 009FAAD0):
// +0h accel 0.5 (00CE3800), +4h scale 0.02 (00CE9BAC), +8h radius 0,
// +0Ch/+10h position, +14h/+18h velocity, +1Ch/+20h output (gun+20h/+24h).
struct GunAimWander {
    float accel{0.5f};
    float scale{0.02f};
    float radius{0.0f};
    float pos[2]{0.0f, 0.0f};
    float vel[2]{0.0f, 0.0f};
    float out[2]{0.0f, 0.0f};
};

// 009FA620: pos = unit(U(0.1, 1.0), U(-1, 1)), vel = accel * unit(U(-1, 1),
// U(-1, 1)), four draws on 00BD2F10 stream 1 in that order.
void gun_aim_wander_init_009fa620(GunAimWander& w, const float draws[4]) noexcept;

// 009FA7E0 (__thiscall(wander, float dt)): one step with two stream-1 draws
// (rx, ry) in [-1, 1]. h = dt * 0.5 (00D7A280); v += accel * unit(rx, ry);
// p += h * v; when |p| > 1 (00D7A24C) v = -p and the step ends; else when
// |p| < radius, p += 3.0 (00D7A2B0) * h * v and p is clamped to the unit disc.
// out = (scale * p.x * 0.8 (00CE3D40), scale * p.y).
void gun_aim_wander_step_009fa7e0(GunAimWander& w, float dt, float rx, float ry) noexcept;

// 009F9FC0's damped cap, taken only when gun+4Ah (the fine aim ran on the last
// tick) is set: per axis, with the previous tick's angle at gun+54h/+58h,
//   rate = (prev - e) / dt,  t = e / rate,
//   if t > 0:  cap = 0.35 (00D04690) * t / (Spd / Accel)
// and the stick magnitude becomes min(min(|8e| / (Spd^2 / Accel), 1), cap).
// Returns a value >= 1 (no effect) when the error is not closing (t <= 0).
float gun_aim_damped_cap_009f9fc0(float e, float e_prev, float dt, float spd,
                                  float accel) noexcept;

}  // namespace bsp
