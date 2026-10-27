#include "bsp/plane_wanderer.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"

#include <cmath>
#include <cstdint>

// docs/PLANE_WANDERER.md. The listing is x87 throughout; values kept on the FPU stack
// between stores are carried here in double and rounded to float at each store the
// listing makes (LABELLED: extended precision is not reproduced).

namespace bsp {
namespace {

// kPc24PlaneTasksCc13Bound (pc24w1-sweep3-4): the ON arms below round every x87 op at
// PC24 (bsp/cc13_x87_model.hpp) and store to float wherever the listing stores.
namespace pc24 = cc13x87;
float f32(double v) { return static_cast<float>(v); }

float length3_0042b2f0(const float v[3]) {
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 0042B2F9-0042B309: (y*y + x*x) + z*z, each op rounded, one FSTP float.
        const float s = f32(pc24::x87_add24(pc24::x87_add24(pc24::x87_mul24(v[1], v[1]),
                                                            pc24::x87_mul24(v[0], v[0])),
                                            pc24::x87_mul24(v[2], v[2])));
        // 0042B30C-0042B33C: sqrt only when s > 1e-10 [00CE3820] (FCOMI/JBE), else +0,
        // NaN included.
        float len = 0.0f;
        if (static_cast<double>(s) > 1.0e-10) len = f32(std::sqrt(static_cast<double>(s)));
        return len;
    } else {
        const double s = static_cast<double>(v[0]) * v[0] + static_cast<double>(v[1]) * v[1] +
                         static_cast<double>(v[2]) * v[2];
        return static_cast<float>(std::sqrt(s));
    }
}

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x).
float interp_00419010(float x0, float y0, float x1, float y1, float x) {
    if (x1 == x0) return y0;
    const float v = y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
    const float lo = y0 < y1 ? y0 : y1;
    const float hi = y0 < y1 ? y1 : y0;
    return v < lo ? lo : (v > hi ? hi : v);
}

// The shared "v -= (v / len) * min(k, len)" decay of 007BE414-007BE49D, 007BE657-007BE6EB.
void decay_toward_zero(float v[3], float len, float k) {
    const float m = k > len ? len : k;
    for (int i = 0; i < 3; ++i) {
        const float unit = static_cast<float>(static_cast<double>(v[i]) / len);
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE46E-007BE49D / 007BE6BC-007BE6EB: unit * m is stored before the FSUB.
            v[i] = f32(pc24::x87_sub24(v[i], f32(pc24::x87_mul24(unit, m))));
        } else {
            v[i] = static_cast<float>(static_cast<double>(v[i]) - static_cast<double>(unit) * m);
        }
    }
}

void zero3(float v[3]) {
    v[0] = 0.0f;   // [00F87574..00F8757C], the zero vector
    v[1] = 0.0f;
    v[2] = 0.0f;
}

}  // namespace

void plane_wanderer_construct_007c4560(PlaneWandererState& s, const PlaneWandererDraw& draw) {
    zero3(s.offset);
    zero3(s.speed);
    zero3(s.accel);
    s.roll = 0.0f;
    s.roll_target = 0.0f;
    s.mul = 1.0f;
    s.enabled = true;
    s.timer = draw(0.5f, 1.5f);   // 007C4616, ECX = 1 (007C457D)
    s.constructed = true;
}

void plane_wanderer_random_direction_00439950(float out[3], const PlaneWandererDraw& draw) {
    const float z = draw(-1.0f, 1.0f);                       // 0043996D
    const float phi = draw(0.0f, 6.2831854820251465f);       // 0043998D, [00CE3D9C]
    float one_minus_zz;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 0043999A FMUL ST0, 0043999E FSUBRP (DE E1): 1 - z*z, each rounded at PC24.
        one_minus_zz = f32(pc24::x87_sub24(1.0, pc24::x87_mul24(z, z)));
    } else {
        one_minus_zz = static_cast<float>(1.0 - static_cast<double>(z) * z);
    }
    const float r = static_cast<float>(std::sqrt(static_cast<double>(one_minus_zz)));   // 004399A8 sqrt
    // 004399BD FCOS / 004399C7 FSIN are each stored as floats (004399BF, 004399C9)
    // before the products with r (004399E2 FMULP, 004399EA FMUL).
    // kPlaneWandererFloatTrigBound false keeps the older double products.
    if constexpr (kPlaneWandererFloatTrigBound) {
        const float c = static_cast<float>(std::cos(static_cast<double>(phi)));
        const float s = static_cast<float>(std::sin(static_cast<double>(phi)));
        out[0] = static_cast<float>(static_cast<double>(r) * c);
        out[1] = static_cast<float>(static_cast<double>(r) * s);
    } else {
        out[0] = static_cast<float>(static_cast<double>(r) * std::cos(static_cast<double>(phi)));
        out[1] = static_cast<float>(static_cast<double>(r) * std::sin(static_cast<double>(phi)));
    }
    out[2] = z;
}

bool plane_wanderer_fixed_step_007be060(PlaneWandererState& s, const PlaneWandererTuning& t,
                                        const PlaneWandererInputs& in, float dt,
                                        const PlaneWandererDraw& draw) {
    // 007BE0B8-007BE0C2: |accel|.
    float len = length3_0042b2f0(s.accel);
    const bool active = (s.enabled || in.player_slot_arm) && in.airborne;   // 007BE0C6-007BE0FF
    bool ran_active = false;
    if (active) {
        ran_active = true;
        // 007BE105-007BE11D: the timer.
        const float timer = s.timer - dt;
        s.timer = timer;
        if (0.0f > timer) {
            const float accel_mul = in.large ? 1.0f : t.small_accel_mul;   // 007BE126-007BE141
            const float time_mul = in.large ? 1.0f : t.small_time_mul;     // 007BE14A-007BE165
            s.timer = static_cast<float>(
                static_cast<double>(draw(t.time_range_1, t.time_range_2)) * time_mul);   // 007BE186
            // 007BE196-007BE1DD: d = 00439950() - offset / OffsetMax.
            float r[3];
            plane_wanderer_random_direction_00439950(r, draw);
            float d[3];
            for (int i = 0; i < 3; ++i) {
                const float q = static_cast<float>(static_cast<double>(s.offset[i]) / t.offset_max);
                d[i] = r[i] - q;
            }
            // 007BE1E1-007BE224: longer than 1.5 (00CE380C) is scaled to 1.5 (00CE3D78).
            const float dl = length3_0042b2f0(d);
            if (dl > 1.5f) {
                const float k = static_cast<float>(1.5 / static_cast<double>(dl));
                for (int i = 0; i < 3; ++i) d[i] = d[i] * k;
            }
            // 007BE22C-007BE279: accel += d * (ChangeMul * accel_mul).
            const float c = static_cast<float>(static_cast<double>(t.change_mul) * accel_mul);
            for (int i = 0; i < 3; ++i) {
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // 007BE24A-007BE279: d * c is stored before the FADD.
                    s.accel[i] = f32(pc24::x87_add24(s.accel[i], f32(pc24::x87_mul24(d[i], c))));
                } else {
                    s.accel[i] = static_cast<float>(static_cast<double>(s.accel[i]) +
                                                    static_cast<double>(d[i]) * c);
                }
            }
            // 007BE27C-007BE2B6: clamp to AccelMax (0042AE50).
            len = length3_0042b2f0(s.accel);
            if (len > t.accel_max) {
                const float k = static_cast<float>(static_cast<double>(t.accel_max) / len);
                for (int i = 0; i < 3; ++i) s.accel[i] = s.accel[i] * k;
                len = t.accel_max;
            }
            // 007BE2BC-007BE32F: the roll kick.
            const float u = draw(0.0f, 1.0f);
            const float chance = static_cast<float>(static_cast<double>(s.mul) * t.roll_change_chance);
            const float w = chance > u ? interp_00419010(0.0f, 1.0f, chance, 0.2f, u)
                                       : interp_00419010(chance, 0.2f, 0.8f, 0.0f, u);
            if (w > 0.0f) {
                // 007BE347-007BE373: the sign is the parity of _ftol2(timer x 9999.0)
                // (00D05A88, 00BF7420 CVTTSD2SI): odd -> +1.0, even -> -1.0.
                const double scaled = static_cast<double>(s.timer) * 9999.0;
                const std::int32_t n = static_cast<std::int32_t>(scaled);
                const float sign = (n % 2 != 0) ? 1.0f : -1.0f;
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // 007BE375-007BE38C: (max * w) * sign + target, each op rounded.
                    s.roll_target = f32(pc24::x87_add24(
                        pc24::x87_mul24(pc24::x87_mul24(t.roll_change_max, w), sign), s.roll_target));
                } else {
                    s.roll_target = static_cast<float>(
                        static_cast<double>(t.roll_change_max) * w * sign + s.roll_target);
                }
            }
        }
    } else if (!(len >= 0.1f)) {
        // 007BE3CE-007BE411: below 0.1 (00D7A3A0) the offset and the roll are cleared.
        zero3(s.offset);
        s.roll = 0.0f;
        return false;
    }
    // 007BE38F-007BE49D: accel decays toward zero by (AccelMax / AccelDecayTime) x dt.
    if (0.001 > static_cast<double>(len)) {
        zero3(s.accel);
    } else {
        float k;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE414-007BE424: the ratio (a float at 00E0B020) times dt.
            k = f32(pc24::x87_mul24(f32(pc24::x87_div24(t.accel_max, t.accel_decay_time)), dt));
        } else {
            k = static_cast<float>(
                static_cast<double>(t.accel_max / t.accel_decay_time) * dt);
        }
        decay_toward_zero(s.accel, len, k);
    }
    // 007BE4A0-007BE501: speed += accel x dt x mul.
    for (int i = 0; i < 3; ++i) {
        const float a = static_cast<float>(static_cast<double>(s.accel[i]) * dt);
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE4D2-007BE501: a * mul is stored before the FADD.
            s.speed[i] = f32(pc24::x87_add24(s.speed[i], f32(pc24::x87_mul24(a, s.mul))));
        } else {
            s.speed[i] = static_cast<float>(static_cast<double>(s.speed[i]) +
                                            static_cast<double>(a) * s.mul);
        }
    }
    // 007BE504-007BE54F: clamp to SpeedMax.
    float slen = length3_0042b2f0(s.speed);
    if (slen > t.speed_max) {
        const float k = static_cast<float>(static_cast<double>(t.speed_max) / slen);
        for (int i = 0; i < 3; ++i) s.speed[i] = s.speed[i] * k;
        slen = t.speed_max;
    }
    // 007BE551-007BE5F3: f = interp(SR1 -> 0, SR2 -> 1, speed) x interp(15 -> 0, 45 -> 1,
    // plane+9B4h) (00CE5380, 00CE3D60), / SmallPlaneDecalMul for a small plane.
    float f = interp_00419010(t.speed_range_1, 0.0f, t.speed_range_2, 1.0f, in.plane_speed);
    f = interp_00419010(15.0f, 0.0f, 45.0f, 1.0f, in.height_9b4) * f;
    if (!in.large) f = static_cast<float>(static_cast<double>(f) / t.small_decal_mul);
    // 007BE5F3-007BE614: g = 1.5 / (f + 0.5) when enabled, else 3.0 (00D7A280, 00CE3D78,
    // 00D7A2B0).
    float g = 3.0f;
    if (s.enabled) {
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE5FE-007BE614: FADD then FDIVR, each rounded, one FSTP float.
            g = f32(pc24::x87_div24(1.5, pc24::x87_add24(0.5, f)));
        } else {
            g = static_cast<float>(1.5 / (0.5 + static_cast<double>(f)));
        }
    }
    // 007BE618-007BE6EB: speed decays by (SpeedMax / SpeedDecayTime) x dt x mul x g.
    if (0.001 > static_cast<double>(slen)) {
        zero3(s.speed);
    } else {
        float k;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE657-007BE672: ((ratio * dt) * mul) * g, the ratio a float at 00E0B028.
            k = f32(pc24::x87_mul24(pc24::x87_mul24(pc24::x87_mul24(
                    f32(pc24::x87_div24(t.speed_max, t.speed_decay_time)), dt), s.mul), g));
        } else {
            k = static_cast<float>(
                static_cast<double>(t.speed_max / t.speed_decay_time) * dt * s.mul * g);
        }
        decay_toward_zero(s.speed, slen, k);
    }
    // 007BE6EE-007BE741: offset += speed x f x dt.
    for (int i = 0; i < 3; ++i) {
        const float w = static_cast<float>(static_cast<double>(f) * s.speed[i]);
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE712-007BE741: w * dt is stored before the FADD.
            s.offset[i] = f32(pc24::x87_add24(f32(pc24::x87_mul24(w, dt)), s.offset[i]));
        } else {
            s.offset[i] = static_cast<float>(static_cast<double>(w) * dt + s.offset[i]);
        }
    }
    // 007BE744-007BE77C: |offset|, / SmallPlaneOffsetMul for a small plane.
    float olen = length3_0042b2f0(s.offset);
    if (!in.large) olen = static_cast<float>(static_cast<double>(olen) / t.small_offset_mul);
    if (0.001 > static_cast<double>(olen)) {
        zero3(s.offset);                                              // 007BE78A
    } else {
        // 007BE7B7-007BE8B3: r = interp(0.7 OffsetMax -> 1, 3 OffsetMax -> 5, |offset|) x g
        // x (OffsetMax / OffsetDecayTime); offset -= offset / |offset| x min(r, |offset|) x dt.
        const float om = t.offset_max;
        const float x0 = static_cast<float>(static_cast<double>(om) * 0.699999988079071);
        const float x1 = static_cast<float>(3.0 * static_cast<double>(om));
        float r = static_cast<float>(static_cast<double>(interp_00419010(x0, 1.0f, x1, 5.0f, olen)) * g);
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE806-007BE810: r times the ratio (a float at 00E0B02C).
            r = f32(pc24::x87_mul24(r, f32(pc24::x87_div24(om, t.offset_decay_time))));
        } else {
            r = static_cast<float>(static_cast<double>(r) * (om / t.offset_decay_time));
        }
        const float m = r > olen ? olen : r;
        const float step = static_cast<float>(static_cast<double>(m) * dt);
        for (int i = 0; i < 3; ++i) {
            const float unit = static_cast<float>(static_cast<double>(s.offset[i]) / olen);
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // 007BE888-007BE8B3: unit * step is stored before the FSUB.
                s.offset[i] = f32(pc24::x87_sub24(s.offset[i], f32(pc24::x87_mul24(unit, step))));
            } else {
                s.offset[i] = static_cast<float>(static_cast<double>(s.offset[i]) -
                                                 static_cast<double>(unit) * step);
            }
        }
    }
    s.mul = 1.0f;                                                     // 007BE8C3
    if (in.local_player_role_1) s.roll_target = 0.0f;                 // 007BE8C8-007BE8D4
    // 007BE8D9-007BE916: roll moves toward the target at RollChangeSpeed.
    if (s.roll != s.roll_target) {
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007BE8FF-007BE90D: roll + ((target - roll) * dt) * speed, each op rounded.
            s.roll = f32(pc24::x87_add24(s.roll, pc24::x87_mul24(pc24::x87_mul24(
                pc24::x87_sub24(s.roll_target, s.roll), dt), t.roll_change_speed)));
        } else {
            s.roll = static_cast<float>(static_cast<double>(s.roll) +
                (static_cast<double>(s.roll_target) - s.roll) * dt * t.roll_change_speed);
        }
    }
    // 007BE916-007BE994: the target decays toward 0 by RollChangeDecay x dt
    // (x SmallPlaneRollDecayMul for a small plane).
    float dec = static_cast<float>(static_cast<double>(t.roll_change_decay) * dt);
    if (!in.large) dec = static_cast<float>(static_cast<double>(t.small_roll_decay_mul) * dec);
    if (s.roll_target > 0.0f) {
        const float v = s.roll_target - dec;
        s.roll_target = 0.0f > v ? 0.0f : v;
    } else {
        const float v = s.roll_target + dec;
        s.roll_target = v > 0.0f ? 0.0f : v;
    }
    return ran_active;
}

}  // namespace bsp
