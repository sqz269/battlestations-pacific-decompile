#pragma once

// Packet cc13-fire: the fire envelope of 009FC7C0 (BSP_BotTaskGun_Tick,
// __thiscall(gun = task+314h, float dt), RET 4). docs/CC13_FIRE_ENVELOPE.md.
// The pieces the host's dogfight_gun_tick_009fc7c0 (src/dogfight_task.cpp)
// leaves out, read from the listing:
//   A  009FCDBC 007BA760(ECX = own unit): unit+C3Ah or unit+5Dh set skips the
//      fire arm (src/cc13_seg47.cpp reconstructs the body).
//   B  009FCDC5 007B96D0 runs only when the envelope holds and A passed, so
//      007DEDB0's 1 s evaluation clock (+80h, advanced by 007E2010) is only
//      consumed on those ticks.
//   C  009FCDD3-009FCE01: when 007B96D0 is true and the steer ran this tick
//      (+49h), +4Ah = 0, the local "steered" flag is cleared, the stick
//      vertical gets +0.25 (00D7A348, double) and 009F9FC0 runs again; the
//      tail then skips the +54h/+58h store (009FCE2D-009FCE44).
//   D  009FCA06/009FCAC2: with an auto target (+74h) or the strafe point
//      (+28h > 0) the hold timer +4Ch is set to -1.0 (00D7A260) before the
//      envelope, so the hold arm never passes a found target.
//   E  009FCA16-009FCAC0: with no auto target, +28h <= 0 and +4Ch > 0, +28h =
//      0.1 (00D7A2F0) and the lead point is own position (00427EB0) plus the
//      squadron leader's ([[unit+9D4h]+3D0h], else the unit) forward axis
//      +94h..+9Ch times +34h; the hold arm then passes the envelope.
// Precision: the main thread runs x87 at PC24, so every product and sum below
// rounds to a float mantissa; that is plain float arithmetic. The 53-bit
// variant (Pc53 = true) is kept for the record.
// Draws: none. 007BA760, 007DEDB0, 0042B2F0 and 00414C60 take no stream draw.
// The only coupling is indirect: fewer or more +4Bh edges change how many
// 00BD2F10 burst draws (the host's "#dfgb" key) the tail takes.

#include <cmath>

#include "bsp/dogfight_task.hpp"

namespace bsp {

// Main switch: A, B and C. OFF keeps the host's tick byte-identical.
inline constexpr bool kDogfightFireEnvelopeBound = false;
// Sub-switch, read only with the main switch ON: D and E, the hold timer arm.
inline constexpr bool kDogfightFireHoldArmBound = false;

inline constexpr float kGunReaimVerticalStep_00d7a348 = 0.25f;  // double 0.25
inline constexpr float kGunHoldSearchArm_00d7a2f0 = 0.1f;       // float 0.1
inline constexpr double kGunLengthCutoff_00ce3820 = 1e-10;       // double
inline constexpr double kGunDontShootDivisor_00d049a8 = static_cast<double>(1.8f);

// 007BA760, __thiscall(unit), body 007BA760-007BA778: CMP [ECX+C3Ah],0 /
// JNZ -> 1; CMP [ECX+5Dh],0 / JNZ -> 1; else 0.
inline bool unit_gun_fire_blocked_007ba760(bool death_c3a, bool simulate_5d) noexcept {
    return death_c3a || simulate_5d;
}

// 0042B2F0: (x*x + y*y) + z*z stored as a float (0042B309), sqrt
// (00BF7030) when above 1e-10, else 0; the result is stored as a float.
template <bool Pc53>
inline float gun_length3_0042b2f0(const float v[3]) noexcept {
    float sum;
    if constexpr (Pc53) {
        sum = static_cast<float>((static_cast<double>(v[0]) * v[0] +
                                  static_cast<double>(v[1]) * v[1]) +
                                 static_cast<double>(v[2]) * v[2]);
    } else {
        sum = (v[0] * v[0] + v[1] * v[1]) + v[2] * v[2];
    }
    if (!(static_cast<double>(sum) > kGunLengthCutoff_00ce3820)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

// 00414C60: x*x + y*y, the same cutoff and store.
template <bool Pc53>
inline float gun_length2_00414c60(const float v[3]) noexcept {
    float sum;
    if constexpr (Pc53) {
        sum = static_cast<float>(static_cast<double>(v[0]) * v[0] +
                                 static_cast<double>(v[1]) * v[1]);
    } else {
        sum = v[0] * v[0] + v[1] * v[1];
    }
    if (!(static_cast<double>(sum) > kGunLengthCutoff_00ce3820)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

struct GunFireEnvelopeInputs {
    float lead_local[3] = {0.0f, 0.0f, 0.0f};  // the lead in the unit frame
    float hold_4c = 0.0f;           // after the tick's decrement and D
    float search_range_30 = 1500.0f;
    float shoot_distance_34 = 850.0f;
    float lateral_cap_38 = 0.09f;
    float dont_shoot_3c = 0.0f;
    bool auto_target_74 = false;    // the /1.8 at 009FCC1E
    float k08 = 0.0f;               // +08h
    float k0c = 0.0f;               // +0Ch (0 from the constructor)
};
struct GunFireEnvelope {
    bool in_range = false;  // 009FCB3D d > 1 and 009FCB6A far > d
    bool envelope = false;  // BL at 009FCDB6
    float d = 0.0f;
    float lateral = 0.0f;
};

// 009FCB25-009FCCA0. The host's lead is already in the unit frame, so d is
// taken from it (the image takes d on the world difference and L on the
// 00414E10 transform; for a rigid frame the two agree). Labelled.
template <bool Pc53>
inline GunFireEnvelope gun_fire_envelope_009fcb25(const GunFireEnvelopeInputs& in) noexcept {
    GunFireEnvelope r;
    const float d = gun_length3_0042b2f0<Pc53>(in.lead_local);
    r.d = d;
    if (!(d > 1.0f)) return r;                                     // 009FCB3D
    // 009FCB4F: +34h + 200.0 (00CE4D70) stored as a float; 00415550 max.
    const float near_far = static_cast<float>(static_cast<double>(in.shoot_distance_34) + 200.0);
    const float far_limit = in.search_range_30 < near_far ? near_far : in.search_range_30;
    if (!(far_limit > d)) return r;                                // 009FCB6A
    r.in_range = true;
    const float lat = gun_length2_00414c60<Pc53>(in.lead_local);
    r.lateral = lat;
    const float z = in.lead_local[2];
    float a = in.dont_shoot_3c;                                    // 009FCC0D
    if (in.auto_target_74) {
        a = static_cast<float>(static_cast<double>(a) / kGunDontShootDivisor_00d049a8);
    }
    if (in.hold_4c > 0.0f) {                                       // 009FCC33
        r.envelope = true;
        return r;
    }
    // 009FCC35-009FCC48: (+38h * d) <= L, or unordered, fails (JBE).
    const bool cap_pass = Pc53
        ? static_cast<double>(lat) < static_cast<double>(in.lateral_cap_38) * d
        : lat < in.lateral_cap_38 * d;
    if (!cap_pass) return r;
    if (!(z > 1.0f)) return r;                                     // 009FCC57 [00D7A24C]
    if (!(z < in.shoot_distance_34)) return r;                     // 009FCC64 JBE
    if (lat > a) {                                                 // 009FCC70
        r.envelope = true;
        return r;
    }
    // 009FCC72-009FCC8A: k = float(+0Ch * +08h); L > 2 * k * d.
    const float k = in.k0c * in.k08;
    const bool second = Pc53 ? static_cast<double>(lat) > 2.0 * (static_cast<double>(k) * d)
                             : lat > 2.0f * (k * d);
    r.envelope = second;
    return r;
}

// 009FC7C9-009FC80E: +50h -= dt; +4Ch -= dt while +4Ch >= 0; +48h = 0.
inline void gun_tick_clocks_009fc7c9(DogfightGunState& st, float dt) noexcept {
    st.burst_timer_50 -= dt;
    if (st.hold_4c >= 0.0f) st.hold_4c -= dt;
    st.fire_48 = false;
}

// 009FCDB6-009FCE1F: the fire arm after A and B. Returns true on a fire.
inline bool gun_fire_arm_009fcdb6(DogfightGunState& st, bool envelope, bool blocked,
                                  bool friendly_busy) noexcept {
    if (!envelope || blocked || friendly_busy) return false;
    if (!(st.burst_4b || st.burst_timer_50 < 0.0f)) return false;  // 009FCE05-009FCE10
    st.fire_48 = true;                                             // 009FCE14
    ++st.fire_ticks;
    return true;
}

// 009FCE4F-009FCED9: the burst clock, as dogfight_gun_tick_009fc7c0 has it.
inline void gun_burst_clock_009fce4f(DogfightGunState& st, float burst_draw,
                                     float delay_draw) noexcept {
    if (!st.burst_4b) {
        if (st.burst_timer_50 < 0.0f && st.fire_48) {
            st.burst_4b = true;
            ++st.bursts;
            st.burst_timer_50 = burst_draw;
        }
    } else if (st.burst_timer_50 < 0.0f) {
        st.burst_4b = false;
        st.burst_timer_50 = delay_draw;
    }
}

}  // namespace bsp
