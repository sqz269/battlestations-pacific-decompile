// Packet cc13-steergate (docs/CC13_STEER_GATE.md). Hypothesis names; the addresses and
// arithmetic are from the disk bytes of battlestationspacific.exe.

#include "bsp/cc13_steer_gate.hpp"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "bsp/cc13_seg68.hpp"
#include "bsp/native_camera_projection.hpp"

namespace bsp {

namespace {

// The two helpers 009FABE0 calls. 009FA620 is the GunHost's third member and
// 009FABE0 never reaches it.
struct SteerGateGunHost final : cc13_seg68::GunHost {
    void reseed_aim_wander_009fa620(GunAimWander&) override {
        std::abort();   // not a callee of 009FABE0
    }
    // 009FAC38-009FAC41: the float pushed as the stack DWORD, ST0 result.
    float tangent_00412e20(float angle) override {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &angle, sizeof bits);
        return native_projection_tangent_00412e20(bits);
    }
    // 00419440 BSP_Vector3f_Length(ECX = v), ST0, stored as float at 009FAC50.
    float vector_length_00419440(const std::array<float, 3>& v) override {
        const float xx = v[0] * v[0];
        const float yy = v[1] * v[1];
        const float zz = v[2] * v[2];
        const float sum = (yy + xx) + zz;
        return static_cast<float>(std::sqrt(static_cast<double>(sum)));
    }
};

}  // namespace

std::array<float, 3> bot_commanded_direction_009fabe0(float heading, float pitch) {
    SteerGateGunHost host;
    cc13_seg68::BotTaskGun gun;
    cc13_seg68::bot_set_direction_from_heading_pitch_009fabe0(host, gun, heading, pitch);
    return gun.direction_68;
}

float steer_gate_one_minus_cos_009fcb96(const float direction_68[3], const float lead_unit[3]) {
    const float yy = direction_68[1] * lead_unit[1];     // 009FCB96-009FCB99
    const float xx = direction_68[0] * lead_unit[0];     // 009FCB9D-009FCBA0
    const float s = yy + xx;                             // 009FCBA4 FADDP
    const float zz = direction_68[2] * lead_unit[2];     // 009FCBA6-009FCBA9
    const float dot = s + zz;                            // 009FCBAD, FSTP float 009FCBAF
    return 1.0f - dot;                                   // 009FCBB7 FLD1 / FSUBRP, FSTP float
}

}  // namespace bsp
