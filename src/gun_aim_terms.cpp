#include "bsp/gun_aim_terms.hpp"

#include <cmath>

// Packet cc9_gun_aim_terms. docs/GUN_AIM_TERMS.md.

namespace bsp {

namespace {
// 00419260 BSP_Vector2f_ReciprocalLength.
float reciprocal_length(float x, float y) noexcept {
    const float len = std::sqrt(x * x + y * y);
    return len > 0.0f ? 1.0f / len : 0.0f;
}
}  // namespace

void gun_aim_wander_init_009fa620(GunAimWander& w, const float draws[4]) noexcept {
    const float rp = reciprocal_length(draws[0], draws[1]);
    w.pos[0] = draws[0] * rp;   // 009FA695
    w.pos[1] = draws[1] * rp;   // 009FA69C
    const float rv = reciprocal_length(draws[2], draws[3]);
    w.vel[0] = w.accel * draws[2] * rv;   // 009FA71E
    w.vel[1] = w.accel * draws[3] * rv;   // 009FA725
}

void gun_aim_wander_step_009fa7e0(GunAimWander& w, float dt, float rx, float ry) noexcept {
    float h = dt * 0.5f;   // 00D7A280
    const float r = reciprocal_length(rx, ry);
    w.vel[0] += w.accel * r * rx;
    w.vel[1] += w.accel * r * ry;
    w.pos[0] += h * w.vel[0];
    w.pos[1] += h * w.vel[1];
    const double sq = static_cast<double>(w.pos[0]) * w.pos[0]
        + static_cast<double>(w.pos[1]) * w.pos[1];
    float len = 0.0f;
    bool bounced = false;
    if (sq > 1e-10) {   // 00CE3820
        len = std::sqrt(static_cast<float>(sq));
        if (len > 1.0f) {   // 00D7A24C: turn the velocity back toward the centre
            w.vel[0] = -0.0f - w.pos[0];
            w.vel[1] = -0.0f - w.pos[1];
            bounced = true;
        }
    }
    if (!bounced && len < w.radius) {
        w.pos[0] += w.vel[0] * h * 3.0f;   // 00D7A2B0
        w.pos[1] += w.vel[1] * h * 3.0f;
        const float n = std::sqrt(w.pos[0] * w.pos[0] + w.pos[1] * w.pos[1]);
        if (n > 1.0f) {
            w.pos[0] /= n;
            w.pos[1] /= n;
        }
    }
    w.out[0] = w.scale * w.pos[0] * 0.8f;   // 00CE3D40 (double 0.8)
    w.out[1] = w.scale * w.pos[1];
}

float gun_aim_damped_cap_009f9fc0(float e, float e_prev, float dt, float spd,
                                  float accel) noexcept {
    if (dt == 0.0f || accel == 0.0f) return 2.0f;
    const float rate = (e_prev - e) / dt;
    if (rate == 0.0f) return 2.0f;
    const float t = e / rate;
    if (!(t > 0.0f)) return 2.0f;
    return 0.35f * t / (spd / accel);   // 00D04690
}

}  // namespace bsp
